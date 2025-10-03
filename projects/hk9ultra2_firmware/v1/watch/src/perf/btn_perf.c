#include "./btn_perf.h"

static button_entry_t buttons[MAX_BUTTONS];
static int button_count = 0;

int perf_button_add_trigger(int32_t handle, button_trigger_cb_t cb)
{
    for (int i = 0; i < button_count; i++)
    {
        if (buttons[i].handle == handle && cb)
        {
            if (buttons[i].trigger_count < MAX_TRIGGERS_PER_BUTTON)
            {
                buttons[i].triggers[buttons[i].trigger_count++] = cb;
                return 0;
            }
            return -1;
        }
    }
    return -1;
}

static void call_triggers(int32_t handle, button_action_t event)
{
    for (int i = 0; i < button_count; i++)
    {
        if (buttons[i].handle == handle)
        {
            for (int j = 0; j < buttons[i].trigger_count; j++)
            {
                if (buttons[i].triggers[j])
                {
                    buttons[i].triggers[j](event);
                }
            }
            break;
        }
    }
}

static void button_event_handler(int32_t pin, button_action_t action)
{
    int32_t handle = -1;
    for (int i = 0; i < button_count; i++)
    {
        if (buttons[i].pin == (uint32_t)pin)
        {
            handle = buttons[i].handle;
            break;
        }
    }

    call_triggers(handle, action);
}

int32_t perf_button_register(uint32_t pin, uint8_t active_poll)
{
    if (button_count >= MAX_BUTTONS)
        return -1;

    button_cfg_t cfg;
    cfg.pin = pin;
    cfg.active_state = active_poll;
    cfg.mode = PIN_MODE_INPUT;
    cfg.button_handler = button_event_handler;

    int32_t id = button_init(&cfg);
    if (id < 0)
        return -1;

    if (SF_EOK != button_enable(id))
        return -1;

    buttons[button_count].pin = pin;
    buttons[button_count].handle = id;
    buttons[button_count].trigger_count = 0;
    button_count++;

    return id;
}