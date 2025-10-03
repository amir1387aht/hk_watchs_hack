# sensor示例
Sensor（传感器）是物联网重要的一部分，“Sensor 之于物联网”就相当于“眼睛之于人类”。人类如果没有了眼睛就看不到这大千的花花世界，对于物联网来说也是一样。已经有大量的 Sensor 被开发出来供开发者选择了，如：加速度计(Accelerometer)、磁力计(Magnetometer)、陀螺仪(Gyroscope)、气压计(Barometer/pressure)、湿度计(Humidometer)等。

源码路径：SiFli-SDK\example\rt_device\sensor

## 支持的平台
* sf32lb52-lchspi-ulp

## 传感器类型与介绍
* LTR303传感器、MMC56X3传感器、LSM6DS传感器
* SiFli-SDK 目前基于 RT-Thread，因此外设方面API与RT-Thread 保持一致。详细的API使用说明可以参考 [RT-Thread sensor API](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/device/sensor/sensor) 文档。在接下来的示例中，我们会逐步讲解每个API的使用方法和如何添加新的sensor。

## 例程的使用
### 编译和烧录
切换到例程project目录，运行scons命令执行编译：
```
scons --board=sf32lb52-lchspi-ulp -j8
```
执行烧写命令：
```
build_sf32lb52-lchspi-ulp_hcpu\uart_download.bat
```
按提示选择端口即可进行下载：
```none
please input the serial port num:6
```

#### 例程输出结果展示:
* 烧写完成串口打印各传感器数据
![alt text](assets/sensor.png)

#### sensor配置流程
* 默认情况已经进行了配置，用户不必自行进行修改
* 在menuconfig中打开相应传感器和`I2C3`

```
menuconfig --board=sf32lb52-lchspi-ulp 
```
![alt text](assets/menuconfig_sensor.png)
![alt text](assets/I2C3.png)

## 硬件连接
我们查看一下立创黄山派的原理图，发现各传感器都是使用`PA_39_I2C3_SDA` 与`PA_40_I2C3_SLC`进行数据的接收与发送,外设总线使用的是`I2C3`。
![alt text](assets/sensor_1.png)

## 硬件配置
* 对于sf32lb52-lchspi-ulp开发板,可以配置到任意带有PA*_I2C_UART功能的IO输出I2C的SDA,SCLK波形，I2C名称不做强制要求，可以根据需求配置（I2C1~I2C4）,这里以I2C3为例。
* 传感器通过I2C协议进行通信，具体连接引脚为 PA40（SCL 时钟线）和 PA39（SDA 数据线）
* 通过HAL_PIN_Set() 函数把IO引脚配置为I2C模式，设置为上拉模式，最后一个参数为hcpu/lcpu选择, 1:选择hcpu,0:选择lcpu 
```c
static void board_io_init(void)
{
    HAL_PIN_Set(PAD_PA40, I2C3_SCL, PIN_PULLUP, 1);
    HAL_PIN_Set(PAD_PA39, I2C3_SDA, PIN_PULLUP, 1);
}
```

## 如何添加一个新的传感器
### 添加一个sensor驱动
* 在SiFli-SDK\customer\peripherals\sensor文件夹下，传感器名称创建文件夹，用于传感器的器的驱动代码（例如磁力计SiFli-SDK\customer\peripherals\sensor\MMC56x3）
* 在文件夹下分别创建sensor_nane和sensor_memsic_(sensor_name)对应的.c和.h文件（例如磁力计MMC56x3）
* sensor_name.c文件中编写传感器的需的驱动代码，包括：传感器的初始化代码、I2C通信接口的管理、传感器寄存器的配置、传感器数据的采集和处理、传感器工作模式的控制·（根据传感器手册编写）
* sensor_memsic_(sensor_name).c文件中编写传感器的接口定义，包括：传感器的初始化接口、传感器数据获取接口、传感器工作模式的接口
* 具体操作步骤可以参考[Sensor添加指南](https://docs.sifli.com/projects/sdk/latest/sf32lb52x/app_note/sensor.html)

### app功能代码
* 1、首先确认sensor与板子的连接引脚，通过函数board_io_init进行IO引脚初始化。
* 2、添加传感器的初始化函数，比如MMC5603传感器的初始化函数为rt_hw_mmc56x3_init。
* 3、打开并配置传感器设备，先寻找设备(添加哪个sensor就在menuconfig中使能对应的传感器)，在打开sensor设备进行设置。
* 4、添加传感器数据获取函数，比如MMC5603传感器的数据获取函数为rt_device_read接口函数进行数据的读取。
* 5、最后将读取到的数据进行打印

## 异常诊断
* 如果数据读取是否或者异常跳变，检查rt_device_control是否正确设置电源模式，存在电磁干扰
