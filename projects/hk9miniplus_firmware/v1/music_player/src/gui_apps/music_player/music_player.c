#include "./music_player.h"
#include "gui_app_fwk.h"
#include <rtdevice.h>
#include <string.h>
#include <ctype.h>

// ======================= Internal config =======================
#ifndef MUSIC_PLAYER_MAX_PLAYLIST
    #define MUSIC_PLAYER_MAX_PLAYLIST 128
#endif

// Timer period (ms). Drift-free logic corrects jitter anyway.
#ifndef MUSIC_PLAYER_TIMER_PERIOD_MS
    #define MUSIC_PLAYER_TIMER_PERIOD_MS 100
#endif

// ===================== Internal state (static) ==================
static uint32_t last_tick_ms = 0; // snapshot when we last processed
static uint32_t accum_ms = 0;     // leftover ms carried between callbacks

static music_info_t music_state = {.title = "Unknown Track",
                                   .artist = "Unknown Artist",
                                   .duration_sec = 180,
                                   .position_sec = 0,
                                   .state = MUSIC_STATE_STOPPED};

static lv_timer_t *music_timer = NULL;

static music_track_t playlist[MUSIC_PLAYER_MAX_PLAYLIST];
static size_t playlist_count = 0;
static size_t current_index = SIZE_MAX; // SIZE_MAX == no selection

static uint8_t volume_percent = 70; // 0..100
static music_repeat_mode_t repeat_mode = MUSIC_REPEAT_NONE;
static bool shuffle_enabled = false;

// ===================== Internal utilities ======================
static void set_timer_status(bool enable);
static void update_song_data(void);
static bool load_current_index_to_state(bool reset_position);
static bool advance_next_index(bool from_auto_advance);
static bool advance_prev_index(void);

static int str_icmp(const char *a, const char *b)
{
    if (!a || !b)
        return (a == b) ? 0 : (a ? 1 : -1);
    while (*a && *b)
    {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb)
            return ca - cb;
        ++a;
        ++b;
    }
    return (int)((unsigned char)*a) - (int)((unsigned char)*b);
}

static uint32_t clampu32(uint32_t v, uint32_t lo, uint32_t hi)
{
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

// =================== Core drift-free progression =================
static void update_song_data(void)
{
    if (music_state.state != MUSIC_STATE_PLAYING)
        return;

    // elapsed ms since last callback (handles tick wraparound)
    uint32_t elapsed = lv_tick_elaps(last_tick_ms);
    last_tick_ms = lv_tick_get();

    accum_ms += elapsed;

    // Advance by whole seconds only; keep leftover milliseconds
    while (accum_ms >= 1000)
    {
        accum_ms -= 1000;

        if (music_state.position_sec < music_state.duration_sec)
        {
            music_state.position_sec++;
        }

        // Reached end? auto-advance according to playlist policy
        if (music_state.position_sec >= music_state.duration_sec)
        {
            // Auto-next. If it fails (no next), we stop.
            if (!music_player_next())
            {
                music_player_stop();
            }
            break;
        }
    }
}

static void update_song_data_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    update_song_data();
}

static void ensure_timer_created(void)
{
    if (!music_timer)
    {
        music_timer = lv_timer_create(update_song_data_timer_cb,
                                      MUSIC_PLAYER_TIMER_PERIOD_MS, NULL);
    }
}

static void set_timer_status(bool enable)
{
    ensure_timer_created();

    if (enable)
    {
        // Refresh baseline so we don't "catch up" instantly on resume.
        // NOTE: we DO NOT touch accum_ms here; we preserve fractional progress.
        last_tick_ms = lv_tick_get();
        lv_timer_reset(music_timer);
        lv_timer_resume(music_timer);
    }
    else
    {
        lv_timer_pause(music_timer);
    }
}

// Load playlist[current_index] into music_state; optionally reset pos.
static bool load_current_index_to_state(bool reset_position)
{
    if (current_index == SIZE_MAX || current_index >= playlist_count)
        return false;
    const music_track_t *t = &playlist[current_index];

    strncpy(music_state.title, t->title, sizeof(music_state.title) - 1);
    music_state.title[sizeof(music_state.title) - 1] = '\0';
    strncpy(music_state.artist, t->artist, sizeof(music_state.artist) - 1);
    music_state.artist[sizeof(music_state.artist) - 1] = '\0';

    music_state.duration_sec = t->duration_sec;

    if (reset_position)
    {
        music_state.position_sec = 0;
        accum_ms = 0; // new track -> no fractional carryover
    }
    return true;
}

// Compute next index (with repeat/shuffle policy), set it, load track, and
// start playing. Returns true if we switched to another track.
static bool advance_next_index(bool from_auto_advance)
{
    if (playlist_count == 0)
        return false;

    if (current_index == SIZE_MAX)
    {
        current_index = 0;
    }
    else
    {
        if (repeat_mode == MUSIC_REPEAT_ONE && from_auto_advance)
        {
            // stay on the same index
        }
        else if (shuffle_enabled && playlist_count > 1)
        {
            // NOTE: Basic pseudo-shuffle: pick next different index.
            // Replace with your RNG of choice for better distribution.
            size_t next = current_index;
            next = (next + 1 + (last_tick_ms % (playlist_count - 1))) %
                   playlist_count;
            if (next == current_index)
                next = (next + 1) % playlist_count;
            current_index = next;
        }
        else
        {
            // Linear next
            if (current_index + 1 < playlist_count)
            {
                current_index++;
            }
            else
            {
                if (repeat_mode == MUSIC_REPEAT_ALL)
                    current_index = 0;
                else
                    return false; // no next
            }
        }
    }

    (void)load_current_index_to_state(true);

    // auto-advance always starts playing
    music_state.state = MUSIC_STATE_PLAYING;
    set_timer_status(true);

    rt_kprintf("[Music] Next: %s - %s (vol=%u%%, idx=%u/%u)\n",
               music_state.title, music_state.artist, (unsigned)volume_percent,
               (unsigned)current_index, (unsigned)playlist_count);

    return true;
}

static bool advance_prev_index(void)
{
    if (playlist_count == 0)
        return false;
    if (current_index == SIZE_MAX)
        return false;

    if (shuffle_enabled && playlist_count > 1)
    {
        // Simple "previous" for shuffle: step back linearly (you may keep a
        // history stack if you like)
        if (current_index > 0)
            current_index--;
        else
            current_index =
                (repeat_mode == MUSIC_REPEAT_ALL) ? (playlist_count - 1) : 0;
    }
    else
    {
        if (current_index > 0)
            current_index--;
        else
        {
            if (repeat_mode == MUSIC_REPEAT_ALL)
                current_index = playlist_count - 1;
            else
                return false; // no prev
        }
    }

    (void)load_current_index_to_state(true);
    music_state.state = MUSIC_STATE_PLAYING;
    set_timer_status(true);

    rt_kprintf("[Music] Prev: %s - %s (vol=%u%%, idx=%u/%u)\n",
               music_state.title, music_state.artist, (unsigned)volume_percent,
               (unsigned)current_index, (unsigned)playlist_count);
    return true;
}

// ========================= Public API ===========================
// Query
const music_info_t *music_player_get_info(void)
{
    return &music_state;
}
uint8_t music_player_get_volume(void)
{
    return volume_percent;
}
size_t music_player_playlist_count(void)
{
    return playlist_count;
}
size_t music_player_get_index(void)
{
    return current_index;
}
music_repeat_mode_t music_player_get_repeat_mode(void)
{
    return repeat_mode;
}
bool music_player_get_shuffle(void)
{
    return shuffle_enabled;
}

// Playback
void music_player_play(void)
{
    if (music_state.state != MUSIC_STATE_PLAYING)
    {
        music_state.state = MUSIC_STATE_PLAYING;

        // If nothing selected but we have a playlist, start at first track
        if (current_index == SIZE_MAX && playlist_count > 0)
        {
            current_index = 0;
            (void)load_current_index_to_state(true);
        }

        // DO NOT reset accum_ms here; preserve fractional progress on resume
        last_tick_ms = lv_tick_get();

        set_timer_status(true);
        rt_kprintf("[Music] Playing: %s - %s (pos=%u s, +%u ms, vol=%u%%)\n",
                   music_state.title, music_state.artist,
                   (unsigned)music_state.position_sec, (unsigned)accum_ms,
                   (unsigned)volume_percent);
    }
}

void music_player_pause(void)
{
    if (music_state.state == MUSIC_STATE_PLAYING)
    {
        music_state.state = MUSIC_STATE_PAUSED;
        set_timer_status(false);
        rt_kprintf("[Music] Paused at %u s (+%u ms)\n",
                   (unsigned)music_state.position_sec, (unsigned)accum_ms);
    }
}

void music_player_stop(void)
{
    music_state.state = MUSIC_STATE_STOPPED;
    music_state.position_sec = 0;
    accum_ms = 0; // clear fractional progress
    set_timer_status(false);
    rt_kprintf("[Music] Stopped\n");
}

void music_player_toggle(void)
{
    if (music_state.state == MUSIC_STATE_PLAYING)
        music_player_pause();
    else
        music_player_play();
}

// Single-track setter (does not touch playlist)
void music_player_set_track(const char *title, const char *artist,
                            uint32_t duration_sec)
{
    strncpy(music_state.title, title ? title : "Unknown Track",
            sizeof(music_state.title) - 1);
    music_state.title[sizeof(music_state.title) - 1] = '\0';

    strncpy(music_state.artist, artist ? artist : "Unknown Artist",
            sizeof(music_state.artist) - 1);
    music_state.artist[sizeof(music_state.artist) - 1] = '\0';

    music_state.duration_sec = duration_sec ? duration_sec : 1;
    music_state.position_sec = 0;
    music_state.state = MUSIC_STATE_STOPPED;

    accum_ms = 0;
    set_timer_status(false);

    // Not part of playlist
    current_index = SIZE_MAX;

    rt_kprintf("[Music] Track Set (single): %s - %s (%u sec)\n",
               music_state.title, music_state.artist,
               (unsigned)music_state.duration_sec);
}

// Playlist operations
size_t music_player_set_playlist(const music_track_t *tracks, size_t count)
{
    if (!tracks || count == 0)
    {
        music_player_clear_playlist();
        return 0;
    }

    size_t n =
        (count > MUSIC_PLAYER_MAX_PLAYLIST) ? MUSIC_PLAYER_MAX_PLAYLIST : count;
    for (size_t i = 0; i < n; ++i)
    {
        strncpy(playlist[i].title,
                tracks[i].title ? tracks[i].title : "Untitled",
                sizeof(playlist[i].title) - 1);
        playlist[i].title[sizeof(playlist[i].title) - 1] = '\0';

        strncpy(playlist[i].artist,
                tracks[i].artist ? tracks[i].artist : "Unknown",
                sizeof(playlist[i].artist) - 1);
        playlist[i].artist[sizeof(playlist[i].artist) - 1] = '\0';

        playlist[i].duration_sec =
            tracks[i].duration_sec ? tracks[i].duration_sec : 1;
    }

    playlist_count = n;
    current_index = (playlist_count > 0) ? 0 : SIZE_MAX;

    // Load first track into state (stopped)
    if (playlist_count > 0)
    {
        (void)load_current_index_to_state(true);
        music_state.state = MUSIC_STATE_STOPPED;
        set_timer_status(false);
    }
    else
    {
        music_player_stop();
    }

    rt_kprintf("[Music] Playlist loaded (%u tracks)\n",
               (unsigned)playlist_count);
    return playlist_count;
}

void music_player_clear_playlist(void)
{
    playlist_count = 0;
    current_index = SIZE_MAX;
    // Stop playback and clear position
    music_player_stop();
    // Leave current title/artist as-is; or clear if you prefer.
}

size_t music_player_add_track(const char *title, const char *artist,
                              uint32_t duration_sec)
{
    if (playlist_count >= MUSIC_PLAYER_MAX_PLAYLIST)
        return 0;

    strncpy(playlist[playlist_count].title, title ? title : "Untitled",
            sizeof(playlist[playlist_count].title) - 1);
    playlist[playlist_count].title[sizeof(playlist[playlist_count].title) - 1] =
        '\0';

    strncpy(playlist[playlist_count].artist, artist ? artist : "Unknown",
            sizeof(playlist[playlist_count].artist) - 1);
    playlist[playlist_count]
        .artist[sizeof(playlist[playlist_count].artist) - 1] = '\0';

    playlist[playlist_count].duration_sec = duration_sec ? duration_sec : 1;

    playlist_count++;
    rt_kprintf("[Music] Added track: %s - %s (%u sec). Count=%u\n",
               title ? title : "Untitled", artist ? artist : "Unknown",
               (unsigned)(duration_sec ? duration_sec : 1),
               (unsigned)playlist_count);
    return playlist_count;
}

// Selection / navigation
bool music_player_play_index(size_t index)
{
    if (index >= playlist_count)
        return false;
    current_index = index;
    (void)load_current_index_to_state(true);
    music_state.state = MUSIC_STATE_PLAYING;
    set_timer_status(true);

    rt_kprintf("[Music] Play index %u: %s - %s\n", (unsigned)index,
               music_state.title, music_state.artist);
    return true;
}

bool music_player_play_by_title(const char *title)
{
    if (!title || playlist_count == 0)
        return false;

    for (size_t i = 0; i < playlist_count; ++i)
    {
        if (str_icmp(playlist[i].title, title) == 0)
        {
            return music_player_play_index(i);
        }
    }
    return false;
}

bool music_player_next(void)
{
    return advance_next_index(false /*manual*/);
}

bool music_player_prev(void)
{
    return advance_prev_index();
}

// Options
void music_player_set_volume(uint8_t volume)
{
    volume_percent = (uint8_t)clampu32(volume, 0, 100);
    // TODO: hook into your audio driver here.
    rt_kprintf("[Music] Volume set to %u%%\n", (unsigned)volume_percent);
}

void music_player_set_repeat_mode(music_repeat_mode_t mode)
{
    if (mode != MUSIC_REPEAT_NONE && mode != MUSIC_REPEAT_ALL &&
        mode != MUSIC_REPEAT_ONE)
        mode = MUSIC_REPEAT_NONE;
    repeat_mode = mode;
    rt_kprintf("[Music] Repeat mode: %s\n",
               (repeat_mode == MUSIC_REPEAT_ONE)   ? "ONE"
               : (repeat_mode == MUSIC_REPEAT_ALL) ? "ALL"
                                                   : "NONE");
}

void music_player_set_shuffle(bool enabled)
{
    shuffle_enabled = enabled;
    rt_kprintf("[Music] Shuffle: %s\n", enabled ? "ON" : "OFF");
}