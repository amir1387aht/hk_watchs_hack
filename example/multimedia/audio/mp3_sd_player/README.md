# MP3 本地音乐播放器

源码路径：example/multimedia/audio/mp3_sd_player

## 支持的平台
<!-- 支持哪些板子和芯片平台 -->
+ sf32lb52-lchspi-ulp

## 概述
<!-- 例程简介 -->
本例程将演示播放在 SD 卡上`music`目录下的的 MP3 或者 wav 音频文件，并提供了一个 shell 接口用于修改播放音量。

## 例程的使用
<!-- 说明如何使用例程，比如连接哪些硬件管脚观察波形，编译和烧写可以引用相关文档。
对于 rt_device 的例程，还需要把本例程用到的配置开关列出来，比如 PWM 例程用到了 PWM1，需要在 onchip 菜单里使能 PWM1 -->

### 硬件需求
运行该例程前，需要准备：
+ 一块本例程支持的开发板（[支持的平台](quick_start)）。
+ 喇叭。

```{warning}
需要注意的是，在`sf32lb52-lchspi-ulp`平台上需要外接电池，否则长按按键会无法检测到。
```

### menuconfig 配置

1. 本例程需要读写文件，所以需要用到文件系统，配置`FAT`文件系统：
![RT_USING_DFS_ELMFAT](./assets/mc_fat.png)
1. 本例程使用 SPI1 作为 SD 卡的接口，所以需要使能 SPI1：
![RT_USING_SPI1](./assets/mc_spi1.png)
1. 本例程中的 SD 卡使用 SPI 进行驱动，所以需要使能`MSD`驱动
![RT_USING_MSD](./assets/mc_msd.png)
1. 使能 AUDIO CODEC 和 AUDIO PROC：
![AUDIO CODEC & PROC](./assets/mc_audcodec_audprc.png)
1. 使能 AUDIO(`AUDIO`)：
![AUDIO](./assets/mc_audio.png)
1. 使能 AUDIO MANAGER.(`AUDIO_USING_MANAGER`)
![AUDIO_USING_MANAGER](./assets/mc_audio_manager.png)
1. (`AUDIO_LOCAL_MUSIC`)
![AUDIO_LOCAL_MUSIC](./assets/mc_local_music.png)

### 编译和烧录

切换到例程 project 目录，运行 scons 命令执行编译：

```bash
scons --board=sf32lb52-lchspi-ulp -j32
```

切换到例程`project/build_xx`目录，运行`uart_download.bat`，按提示选择端口即可进行下载：

```bash
$ ./uart_download.bat

     Uart Download

please input the serial port num:5
```

关于编译、下载的详细步骤，请参考 [快速上手](quick_start) 的相关介绍。

## 例程的预期结果
<!-- 说明例程运行结果，比如哪几个灯会亮，会打印哪些 log，以便用户判断例程是否正常运行，运行结果可以结合代码分步骤说明 -->
例程运行之后，会播放 SD 卡`music`目录下的 MP3 或者 wav 文件。上电之后默认处于停止播放状态。当列表中的音乐播放完毕后，会自动从第一首开始循环播放。

对于`sf32lb52-lchspi-ulp`平台，长按`KEY1`按键即可开始播放，播放中长按`KEY1`会停止播放，下次再次开始的时候会从第一首歌开始。短按`KEY1`按键可以暂停/继续播放，短按`KEY2`按键可以切换下一首音乐。

例程也提供了一个 shell 接口，可以通过`volume`命令获取或设置音量。输入`volume`命令可以查看当前音量，输入`volume set <value>`可以设置音量，`<value>`的范围是 0-15。

!![shell](./assets/mc_volume_shell.png)

## 异常诊断

## 参考文档
<!-- 对于 rt_device 的示例，rt-thread 官网文档提供的较详细说明，可以在这里添加网页链接，例如，参考 RT-Thread 的 [RTC 文档](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/device/rtc/rtc) -->

## 更新记录
|版本 |日期   |发布说明 |
|:---|:---|:---|
|0.0.1 |6/2025 |初始版本 |
| | | |
