#include "./vibrator_perf.h"

#define VIB_PAD PAD_PA20             /* physical pad to mux */
#define VIB_GPIO_FUNC (GPIO_A0 + 20) /* GPIO function selector for PA20 */
#define VIB_LOGICAL_PIN 20           /* number used by the "pin" device */

#define VIB_PULL PIN_PULLUP /* safe default pull */
#define VIB_PUSH_PULL 1     /* push-pull enable flag in HAL_PIN_Set */

static rt_device_t s_pin_dev = RT_NULL;
static rt_bool_t s_inited = RT_FALSE;

static void vib_write(rt_base_t level)
{
    struct rt_device_pin_status st = {0};
    st.pin = VIB_LOGICAL_PIN;
    st.status = (level != 0) ? 1 : 0;
    (void)rt_device_write(s_pin_dev, 0, &st, sizeof(st));
}

void vibrator_init(void)
{
    if (s_inited)
        return;

    /* 1) Pin mux: route PAD to GPIO function. */
    HAL_PIN_Set(VIB_PAD, VIB_GPIO_FUNC, VIB_PULL, VIB_PUSH_PULL);

    /* 2) Open the "pin" device (once). */
    s_pin_dev = rt_device_find("pin");
    if (!s_pin_dev)
    {
        rt_kprintf("[vibrator] ERROR: device 'pin' not found\n");
        return;
    }
    if (rt_device_open(s_pin_dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("[vibrator] ERROR: open 'pin' failed\n");
        return;
    }

    /* 3) Configure PA20 as OUTPUT and default LOW (off). */
    struct rt_device_pin_mode m = {0};
    m.pin = VIB_LOGICAL_PIN;
    m.mode = PIN_MODE_OUTPUT;
    (void)rt_device_control(s_pin_dev, 0, &m);

    vib_write(0); /* off */

    s_inited = RT_TRUE;
    rt_kprintf("[vibrator] GPIO ready on PAD_PA20 (pin=%d)\n", VIB_LOGICAL_PIN);
}

void vibrator_on(void)
{
    if (!s_inited)
        vibrator_init();
    if (!s_pin_dev)
        return;
    vib_write(1);
}

void vibrator_off(void)
{
    if (!s_inited || !s_pin_dev)
        return;
    vib_write(0);
}

void vibrator_buzz(uint16_t ms)
{
    if (!s_inited)
        vibrator_init();
    if (!s_pin_dev)
        return;
    vibrator_on();
    rt_thread_mdelay(ms);
    vibrator_off();
}

void vibrator_burst_pattern(const uint16_t *bursts, size_t count)
{
    if (!bursts || count == 0)
        return;
    if (!s_inited)
        vibrator_init();
    if (!s_pin_dev)
        return;

    for (size_t i = 0; i < count; ++i)
    {
        if ((i % 2) == 0)
            vibrator_on(); /* even index: ON duration */
        else
            vibrator_off(); /* odd index : OFF duration */
        rt_thread_mdelay(bursts[i]);
    }
    vibrator_off();
}