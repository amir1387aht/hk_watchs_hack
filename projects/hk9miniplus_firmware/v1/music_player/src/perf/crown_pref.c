#include "./crown_pref.h"

// --- GPIO definitions ---
#define GPIO33_PAD PAD_PA33
#define GPIO33_FUNC (GPIO_A0 + 33)
#define GPIO33_LOGICAL 33

#define GPIO32_PAD PAD_PA32
#define GPIO32_FUNC (GPIO_A0 + 32)
#define GPIO32_LOGICAL 32

#define GPIO_PULL PIN_PULLUP
#define GPIO_PUSH 1

// --- Filter config ---
#define FILTER_DELAY_MS 13 // check stable state

// --- Callbacks ---
static gpio_callback_t callback_scroll_up = RT_NULL;
static gpio_callback_t callback_scroll_down = RT_NULL;

// --- Work timer ---
static rt_timer_t filter_timer = RT_NULL;
static volatile int pending_event = 0; // 1=up, -1=down, 0=none

static void filter_timer_cb(void *parameter)
{
    int up = rt_pin_read(GPIO32_LOGICAL) == 1;
    int down = rt_pin_read(GPIO33_LOGICAL) == 1;

    if (pending_event == 1 && up && callback_scroll_down)
    {
        callback_scroll_down(GPIO32_LOGICAL, 1);
    }
    else if (pending_event == -1 && down && callback_scroll_up)
    {
        callback_scroll_up(GPIO33_LOGICAL, 1);
    }

    pending_event = 0; // reset
}

// IRQ handler: just mark event and start debounce timer
static void irq_handler(void *args)
{
    rt_uint32_t pin = (rt_uint32_t)args;
    rt_int16_t value = rt_pin_read(pin);

    if (value == 0)
        return; // ignore release

    if (pin == GPIO32_LOGICAL)
        pending_event = 1; // UP candidate
    else if (pin == GPIO33_LOGICAL)
        pending_event = -1; // DOWN candidate

    // restart timer
    if (filter_timer)
        rt_timer_start(filter_timer);
}

// Initialize GPIO pins with IRQ
int crown_init(void)
{
    // Create filter timer (one-shot)
    filter_timer = rt_timer_create("crownflt", filter_timer_cb, RT_NULL,
                                   rt_tick_from_millisecond(FILTER_DELAY_MS),
                                   RT_TIMER_FLAG_ONE_SHOT);
    if (!filter_timer)
    {
        rt_kprintf("Failed to create crown filter timer\n");
        return -1;
    }

    // --- GPIO33 ---
    HAL_PIN_Set(GPIO33_PAD, GPIO33_FUNC, GPIO_PULL, GPIO_PUSH);
    rt_pin_mode(GPIO33_LOGICAL, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(GPIO33_LOGICAL, PIN_IRQ_MODE_RISING_FALLING, irq_handler,
                      (void *)GPIO33_LOGICAL);
    rt_pin_irq_enable(GPIO33_LOGICAL, PIN_IRQ_ENABLE);

    // --- GPIO32 ---
    HAL_PIN_Set(GPIO32_PAD, GPIO32_FUNC, GPIO_PULL, GPIO_PUSH);
    rt_pin_mode(GPIO32_LOGICAL, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(GPIO32_LOGICAL, PIN_IRQ_MODE_RISING_FALLING, irq_handler,
                      (void *)GPIO32_LOGICAL);
    rt_pin_irq_enable(GPIO32_LOGICAL, PIN_IRQ_ENABLE);

    return 0;
}

void crown_scroll_up_cb(gpio_callback_t cb)
{
    callback_scroll_up = cb;
}

void crown_scroll_down_cb(gpio_callback_t cb)
{
    callback_scroll_down = cb;
}