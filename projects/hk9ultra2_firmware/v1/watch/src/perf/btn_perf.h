#ifndef __PERF_BUTTON_H__
#define __PERF_BUTTON_H__

#include <button.h>
#include <rtthread.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
    #define MAX_BUTTONS 3
    #define MAX_TRIGGERS_PER_BUTTON 1

    typedef void (*button_trigger_cb_t)(button_action_t event);

    typedef struct
    {
        uint32_t pin;
        int32_t handle;
        button_trigger_cb_t triggers[MAX_TRIGGERS_PER_BUTTON];
        int trigger_count;
    } button_entry_t;

    int32_t perf_button_register(uint32_t pin, uint8_t active_poll);
    int perf_button_add_trigger(int32_t handle, button_trigger_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif /* __PERF_BUTTON_H__ */