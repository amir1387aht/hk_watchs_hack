# 双核综合例程
源码路径：example/get-started/dualcore

## 支持的平台
例程可以运行在以下开发板
* sf32lb52-lcd_n16r8
* eh-lb561
* ec-lb587
* eh-lb523

## 概述
本例程演示如何使用双核开发应用，项目已支持蓝牙和低功耗，基于LVGL v8构建用户界面（参考了`multimedia/lvgl/watch`），大小核运行各自的程序（SF32LB52系列芯片小核为蓝牙专用，不运行用户程序）。

## 目录结构
```
.
└── hmi_demo
    ├── project
    │   ├── hcpu    // HCPU 工程目录
    │   └── lcpu    // LCPU 工程目录
    └── src
        ├── hcpu    // HCPU 代码目录
        └── lcpu    // LCPU 代码目录
```        

## 例程的使用
### 编译和烧录

切换到`project/hcpu`目录，运行scons命令编译：
```
scons --board=sf32lb52-lcd_n16r8 -j32
```

运行`build_sf32lb52-lcd_n16r8_hcpu\uart_download.bat`，按提示选择端口即可进行下载：


在HCPU工程目录下执行`scons`命令会自动编译LCPU的工程，下载脚本会下载包括小核在内的所有的固件。

### 代码解析
#### 大核

大核的`main`函数在`src/hcpu/main.c`中，完成蓝牙初始化
```c
int main(void)
{
    int count = 0;

    app_env_t *env = ble_app_get_env();
    env->mb_handle = rt_mb_create("app", 8, RT_IPC_FLAG_FIFO);
    sifli_ble_enable();
    env->time_handle  = rt_timer_create("app", app_timeout_handler,  NULL,
                                        rt_tick_from_millisecond(BLE_APP_TIMEOUT_INTERVAL), RT_TIMER_FLAG_SOFT_TIMER);
#ifdef SF32LB52X
    env->rc10k_time_handle  = rt_timer_create("rc10", rc10k_timeout_handler,  NULL,
                              rt_tick_from_millisecond(15 * 1000), RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER); // 15s
    rt_timer_start(env->rc10k_time_handle);
#endif

    while (1)
    {
        uint32_t value;
        int ret;
        rt_mb_recv(env->mb_handle, (rt_uint32_t *)&value, RT_WAITING_FOREVER);
        if (value == BLE_POWER_ON_IND)
        {
            env->is_power_on = 1;
            env->conn_para.mtu = 23; /* Default value. */
            ble_app_service_init();
            /* First enable connectable adv then enable non-connectable. */
            ble_app_advertising_start();
            LOG_I("receive BLE power on!\r\n");
        }
    }
    return RT_EOK;
}
```

图形的初始化由`src/hcpu/gui_apps/watch_demo.c`中的`app_watch_init`触发，他被注册为`APP`级别的[自动初始化函数](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/basic/basic?id=rt-thread-%e8%87%aa%e5%8a%a8%e5%88%9d%e5%a7%8b%e5%8c%96%e6%9c%ba%e5%88%b6)，执行时间早于`main.c`中的`main`函数，在`app_watch_init`中创建了`app_watch`线程，图形的初始化实际是在`app_watch`线程完成。

```c
INIT_APP_EXPORT(app_watch_init);
```

#### 小核
小核的`main`函数在`src/lcpu/main.c`中，如下

```c
int main(void)
{
    if (HAL_LXT_DISABLED())
    {
        rc10k_time_handle  = rt_timer_create("rc10", rc10k_timeout_handler,  NULL,
                                            rt_tick_from_millisecond(15 * 1000), RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER); // 15s
        RT_ASSERT(rc10k_time_handle);
        rt_timer_start(rc10k_time_handle);
    }

    while (1)
    {
        rt_kprintf("main loop\n");
        rt_thread_mdelay(3000);
    }
    return RT_EOK;
}
```

#### 按键
如果按键Key1接在小核的管脚上，则由小核处理按键事件，再通过data service将按键事件转发给大核。
小核的按键初始化代码在`src/lcpu/main.c`的`init_pin`函数中，同时小核工程配置`proj.conf`使能了`CONFIG_BUTTON_SERVICE_ENABLED`，也就是注册了按键服务，大核会订阅该服务以接收按键事件。

````{note}
由于大部分板子小核的配置文件（即板子lcpu目录下的`board.conf`里没有定义按键的管脚，因此在lcpu工程目录下针对每一款支持的板子，都添加一个`proj.conf`用于定义按键的管脚编号，例如在`project/lcpu/eh-lb561_lcpu`目录下的`proj.conf`有如下的KEY1的配置参数，该配置针对eh-lb561这块开发板有效
```c
CONFIG_BSP_USING_KEY1=y
CONFIG_BSP_KEY1_PIN=128
CONFIG_BSP_KEY1_ACTIVE_HIGH=y
```

````

大核的按键初始化函数是`src/hcpu/gui_apps/watch_demo.c`中的`init_pin`，可以看到只有在按键对应的GPIO管脚为PA的管脚时，才会在大核程序中初始化按键，否则就只是大约按键的数据服务。
```c
#if (SLEEP_CTRL_PIN < GPIO1_PIN_NUM)
    button_cfg_t cfg;
#if defined(BSP_USING_PM) && !defined(SF32LB52X)
    int8_t wakeup_pin;
    uint16_t gpio_pin;
    GPIO_TypeDef *gpio;
#endif /* BSP_USING_PM && !SF32LB52X */
    
    cfg.pin = SLEEP_CTRL_PIN;
    cfg.active_state = BUTTON_ACTIVE_POL;
    cfg.mode = PIN_MODE_INPUT;
    cfg.button_handler = button_event_handler;
    int32_t id = button_init(&cfg);
    RT_ASSERT(id >= 0);
    RT_ASSERT(SF_EOK == button_enable(id));
    key1_button_handle = id;

#endif /* SLEEP_CTRL_PIN < GPIO1_PIN_NUM */

    button_handle = datac_open();
    RT_ASSERT(DATA_CLIENT_INVALID_HANDLE != button_handle);
    datac_subscribe(button_handle, "btn0", button_service_callback, SLEEP_CTRL_PIN);    
```



对于SF32LB52系列的芯片，小核为蓝牙专用核，不运行用户程序，按键接在了大核的管脚上，由大核直接处理GPIO中断，但仍旧可以通过订阅按键服务的方式统一处理按键事件，这时按键服务也位于大核。

```{note}
SF32LB52系列需要使能`DATA_SVC_MBOX_THREAD_DISABLED`关闭跨核服务
```

## 例程的预期结果
上电后出现蜂窝界面，点击时钟图标可以打开表盘，如果10秒没有操作界面或者按键，会自动灭屏，此时大核睡眠进入低功耗模式，按键(Key1)唤醒大核并亮屏。

## 异常诊断

  
## 参考文档
