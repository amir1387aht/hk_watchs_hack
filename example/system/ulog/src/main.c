#include "rtthread.h"
#include "bf0_hal.h"
#include "drv_io.h"
#include "stdio.h"
#include "string.h"
#include "board.h"
#include "rtconfig.h"


#define LOG_TAG              "example"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>

/* Common functions for RT-Thread based platform -----------------------------------------------*/
/**
  * @brief  Initialize board default configuration.
  * @param  None
  * @retval None
  */
void HAL_MspInit(void)
{
    // __asm("B .");        /*For debugging purpose*/
    BSP_IO_Init();
}

/* User code start from here --------------------------------------------------------*/


int ulog_test(int argc, char **argv)
{
    static int count = 0;
    static uint8_t buf[128];
    int i = 0;
    while (count < 50)
    {
        for (i = 0; i < sizeof(buf); i ++)
        {
            buf[i] = i;
        }


        /* output different level log by LOG_X API */
        LOG_D("LOG_D(%d): RT-Thread is an open source IoT operating system from China.", count);
        LOG_I("LOG_I(%d): RT-Thread is an open source IoT operating system from China.", count);
        LOG_W("LOG_W(%d): RT-Thread is an open source IoT operating system from China.", count);
        LOG_E("LOG_E(%d): RT-Thread is an open source IoT operating system from China.", count);
        LOG_RAW("LOG_RAW(%d): RT-Thread is an open source IoT operating system from China.", count);
        ulog_hexdump("buf_dump_test", 16, buf, sizeof(buf));
        rt_thread_mdelay(10);

        count++;
    }

    return 0;
}

MSH_CMD_EXPORT(ulog_test, ulog test.);




/**
  * @brief  Main program
  * @param  None
  * @retval 0 if success, otherwise failure number
  */
int main(void)
{
    while (1)
    {
        rt_thread_mdelay(5000);
    }
    return 0;
}

