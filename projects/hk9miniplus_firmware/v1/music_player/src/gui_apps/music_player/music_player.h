#ifndef MUSIC_PLAYER_APP_H
#define MUSIC_PLAYER_APP_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "littlevgl2rtt.h"
#include "lv_ext_resource_manager.h"
#include "lvgl.h"
#include "lvsf.h"
#include "gui_app_fwk.h"
#include <rtdevice.h>
#include <string.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C"
{
#endif
    // ---------- Playback & playlist types ----------
    typedef enum
    {
        MUSIC_STATE_STOPPED,
        MUSIC_STATE_PLAYING,
        MUSIC_STATE_PAUSED
    } music_state_t;

    typedef enum
    {
        MUSIC_REPEAT_NONE = 0,
        MUSIC_REPEAT_ALL = 1,
        MUSIC_REPEAT_ONE = 2
    } music_repeat_mode_t;

    typedef struct
    {
        char title[64];
        char artist[64];
        uint32_t duration_sec; // total track length, in seconds
    } music_track_t;

    // Current playback info (read-only view)
    typedef struct
    {
        char title[64];
        char artist[64];
        uint32_t duration_sec; // total track length, in seconds
        uint32_t position_sec; // current whole-second position (drift-free)
        music_state_t state;
    } music_info_t;

    // ---------- Query ----------
    /** Get current music playback info (do not modify the returned struct). */
    const music_info_t *music_player_get_info(void);

    /** Get volume in [0..100]. */
    uint8_t music_player_get_volume(void);

    /** Get playlist size (number of tracks). */
    size_t music_player_playlist_count(void);

    /** Get current playlist index (0-based). Returns SIZE_MAX if no track
     * selected. */
    size_t music_player_get_index(void);

    /** Get current repeat mode. */
    music_repeat_mode_t music_player_get_repeat_mode(void);

    // ---------- Playback control ----------
    /** Start or resume playback (preserves fractional progress when resuming).
     */
    void music_player_play(void);

    /** Pause playback (keeps whole-second and fractional progress). */
    void music_player_pause(void);

    /** Stop playback and reset position to 0 (clears fractional progress). */
    void music_player_stop(void);

    /** Toggle between play and pause. */
    void music_player_toggle(void);

    // ---------- Track selection ----------
    /** Set a new single track and stop (position=0). Does NOT modify playlist.
     */
    void music_player_set_track(const char *title, const char *artist,
                                uint32_t duration_sec);

    /** Play a specific playlist index (0-based). Returns true on success. */
    bool music_player_play_index(size_t index);

    /** Play the first track whose title matches (case-insensitive). Returns
     * true on success. */
    bool music_player_play_by_title(const char *title);

    /** Next track (auto-wrap depends on repeat mode). Returns true if switched.
     */
    bool music_player_next(void);

    /** Previous track (auto-wrap depends on repeat mode). Returns true if
     * switched. */
    bool music_player_prev(void);

    // ---------- Playlist management ----------
    /** Replace playlist with an array of tracks. Returns number loaded. */
    size_t music_player_set_playlist(const music_track_t *tracks, size_t count);

    /** Clear the playlist entirely and stop. */
    void music_player_clear_playlist(void);

    /** Append one track. Returns new count on success, 0 on failure. */
    size_t music_player_add_track(const char *title, const char *artist,
                                  uint32_t duration_sec);

    // ---------- Options ----------
    /** Set volume in [0..100] (clamped). */
    void music_player_set_volume(uint8_t volume);

    /** Set repeat mode. */
    void music_player_set_repeat_mode(music_repeat_mode_t mode);

    /** Enable/disable shuffle */
    void music_player_set_shuffle(bool enabled);

    /** Get shuffle state. */
    bool music_player_get_shuffle(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MUSIC_PLAYER_APP_H
