#include "rtthread.h"
#include "bf0_hal.h"

/**
 * @brief  Main program
 * @param  None
 * @retval 0 if success, otherwise failure number
 */
int main(void)
{
    rt_kprintf("cherryusb device cdc acm demo!\n");

    extern void cdc_acm_init(uint8_t busid, uintptr_t base);
    cdc_acm_init(0, (uintptr_t)USBC_BASE);

    while (1)
    {
      extern void cdc_acm_data_send_with_dtr_test(uint8_t busid);
      cdc_acm_data_send_with_dtr_test(0);
      rt_thread_mdelay(1000);
    }
    return 0;
}

