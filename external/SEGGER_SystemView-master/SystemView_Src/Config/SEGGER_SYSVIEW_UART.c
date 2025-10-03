#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "SEGGER_RTT.h"
#include "SEGGER_SYSVIEW.h"
#include <rthw.h>


/*** (systemview uart recorder sample) ----------------------------------------------------------*/
#define _SERVER_HELLO_SIZE (4)
#define _TARGET_HELLO_SIZE (4)

// "Hello" message expected by SysView: [ 'S', 'V', <PROTOCOL_MAJOR>, <PROTOCOL_MINOR> ]
static uint8_t _abHelloMsg[_TARGET_HELLO_SIZE] = \
    {'S', 'V', (SEGGER_SYSVIEW_VERSION / 10000), 
    (SEGGER_SYSVIEW_VERSION / 1000) % 10}; 

/**
 * @brief systemview信息
 * 
 */
static struct SVInfo_t
{
    uint16_t uart_open_flag; // UART open flag
    rt_device_t uart_device;
    struct rt_semaphore tx_sem;
    rt_thread_t tx_thread;

    uint8_t reset_request;  //Recived STOP cmd

    rt_size_t Total_tx_size;   //Systemview reuqire tx size

    uint8_t NumBytesHelloRcvd;  // 接收hello info计数
    uint8_t NumBytesHelloSent;  // 发送hello info计数
    int ChannelID;

    uint32_t NumBytesTxBuf;     //TX buffer total bytes
} _SVInfo;
/**
 * @brief This function starts and initializes a SystemView session
 * 
 */
static void _StartSysView(void)
{
    int r;

    r = SEGGER_SYSVIEW_IsStarted();
    if (r == 0)
    {
        SEGGER_SYSVIEW_Start();
    }
}

/**
 * @brief This function is called when the UART receives data.
 * 
 * @param Data: 串口原始数据
 */
static void _cbOnRx(uint8_t Data)
{
    if (_SVInfo.NumBytesHelloRcvd < _SERVER_HELLO_SIZE)
    {
        if (_SVInfo.NumBytesHelloRcvd < 2)
        {   
            //  recv 'SV' to start
            if (Data != _abHelloMsg[_SVInfo.NumBytesHelloRcvd])
            {
                return;
            }
        }
        _SVInfo.NumBytesHelloRcvd++;

        if (_SVInfo.NumBytesHelloRcvd == _SERVER_HELLO_SIZE - 1)
        {
            _SVInfo.NumBytesHelloSent = 0;
            _StartSysView();
        }
    }
    else 
    {
        // Write data into corresponding RTT buffer 
        // for application to read and handle accordingly
        SEGGER_RTT_WriteDownBuffer(_SVInfo.ChannelID, &Data, 1);
        if (SEGGER_SYSVIEW_IsStarted() == 0)
        {
            _SVInfo.reset_request = 1;
        }
    }
}


static void sysview_tx_thread_entry(void *parameter)
{
    while (1)
    {
        const uint8_t *pBuf;
        if(_SVInfo.Total_tx_size > 0)
        {
            rt_size_t tx_size;

            if (_SVInfo.NumBytesHelloSent < _TARGET_HELLO_SIZE)
            { // Not all bytes of <Hello> message sent to SysView yet?
                pBuf = &_abHelloMsg[_SVInfo.NumBytesHelloSent];

                tx_size = rt_device_write(_SVInfo.uart_device, 0, pBuf, _TARGET_HELLO_SIZE - _SVInfo.NumBytesHelloSent);

                _SVInfo.NumBytesHelloSent += tx_size;
            }
            else
            {
                pBuf = SEGGER_RTT_GetUpBufferLock(_SVInfo.ChannelID, &_SVInfo.NumBytesTxBuf);

                RT_ASSERT(_SVInfo.NumBytesTxBuf < 4*1024*1024);
                if(_SVInfo.NumBytesTxBuf > 0)
                {
                    tx_size = rt_device_write(_SVInfo.uart_device, 0, pBuf, _SVInfo.NumBytesTxBuf);
                }
                else
                {
                    tx_size = 0;
                }

                RT_ASSERT(tx_size == _SVInfo.NumBytesTxBuf);
            }

            /* Update tx size */
            rt_base_t level;
            level = rt_hw_interrupt_disable();
            if(0 == tx_size) _SVInfo.Total_tx_size = 0; // reset tx size
            else _SVInfo.Total_tx_size -= tx_size;
            rt_hw_interrupt_enable(level);

            if(tx_size > 0) 
            {
                if(_SVInfo.uart_open_flag & RT_DEVICE_FLAG_DMA_TX)  rt_sem_take(&_SVInfo.tx_sem,RT_WAITING_FOREVER);

                if(_SVInfo.NumBytesTxBuf > 0)
                {
                    SEGGER_RTT_DropUpBufferLock(_SVInfo.ChannelID, _SVInfo.NumBytesTxBuf);
                    _SVInfo.NumBytesTxBuf = 0;
                }
            }
        }
        else
        {
            //Response the STOP request after all tx done
            if(_SVInfo.reset_request)
            {
                _SVInfo.NumBytesHelloRcvd = 0;
                _SVInfo.reset_request = 0;
            }
            rt_thread_mdelay(10);
        }



    }
}
/*** (uart hardware) ----------------------------------------------------------*/

static rt_err_t uart_rx_ind(rt_device_t dev, rt_size_t size)
{
    uint8_t buffer[64];
    rt_size_t read_size;

    if (size > 0)
    {
        while(1)
        {
            read_size = rt_device_read(dev, -1, &buffer[0], sizeof(buffer));
            if (read_size <= 0)
            {
                break;
            }
            for(rt_size_t i = 0; i < read_size; i++)
            {
                _cbOnRx(buffer[i]);
            }
        }
    }
    return RT_EOK;
}
static rt_err_t uart_tx_cplt(rt_device_t dev, void *buffer)
{
    rt_sem_release(&_SVInfo.tx_sem);
    return RT_EOK;
}


/*** (APIs) ----------------------------------------------------------*/
/**
 * @brief 串口发送函数
 * 
 * @param ch: channel ID
 * @param tx_size: 发送数据大小
 * @retval : 0
 */
int uart_tx_func(U32 ch, size_t tx_size)
{
    if((_SVInfo.uart_device)&&(tx_size > 0))
    {
        //rt_sem_release(&_SVInfo.tx_sem); Can't use OS semaphore here, because this function is called from ISR

        rt_base_t level;
            /* disable interrupt */
        level = rt_hw_interrupt_disable();
        _SVInfo.Total_tx_size += tx_size;
        rt_hw_interrupt_enable(level);
    }

    return 0;
}


rt_err_t sysview_via_uart(const char *name)
{
    memset(&_SVInfo, 0, sizeof(_SVInfo));
    _SVInfo.ChannelID = 1; // Systemview defalut channel ID
    _SVInfo.NumBytesHelloSent = _TARGET_HELLO_SIZE;

    rt_device_t device = rt_device_find(name);
    if(device)
    {
         _SVInfo.uart_open_flag = RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_DMA_TX;
         rt_err_t err = rt_device_open(device, _SVInfo.uart_open_flag);

        if (err == -RT_EIO)
        {
            _SVInfo.uart_open_flag = RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX;
            err = rt_device_open(device, _SVInfo.uart_open_flag);

            if(err != RT_EOK)
            {
                return err;
            }
        }
        
        _SVInfo.tx_thread = rt_thread_create("sysv_tx", sysview_tx_thread_entry, RT_NULL,  1024, 30, 10);
        
        /* start the thread of g_uart_device */
        if (_SVInfo.tx_thread != RT_NULL)
        {
            rt_sem_init(&_SVInfo.tx_sem, "sysv_tx", 0, RT_IPC_FLAG_FIFO);
            rt_device_set_rx_indicate(device, uart_rx_ind);
            if(_SVInfo.uart_open_flag & RT_DEVICE_FLAG_DMA_TX) rt_device_set_tx_complete(device, uart_tx_cplt);
            rt_thread_startup(_SVInfo.tx_thread);

        }
        else
        {
            rt_device_close(device);
            return -RT_ERROR;
        }

        _SVInfo.uart_device = device;
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}