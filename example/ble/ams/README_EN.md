# BLE AMS Example

Source code path: example/ble/ams

(Platform_ams)=
## Supported Platforms
<!-- Which boards and chip platforms are supported -->
All platforms

## Overview
<!-- Example introduction -->
This example demonstrates how to trigger Apple AMS (Apple Media Service) protocol subscription and simple handling of corresponding events.
AMS is an audio control protocol provided by Apple that allows access to the current iOS device's audio playback status and control of music play, pause, etc.


## Usage Instructions
<!-- Explain how to use the example, such as which hardware pins to connect to observe waveforms, compilation and flashing can reference related documentation.
For rt_device examples, also list the configuration switches used in this example, for example PWM example uses PWM1, need to enable PWM1 in onchip menu -->
1. The example starts advertising upon boot with the name SIFLI_APP-xx-xx-xx-xx-xx-xx, where xx represents the Bluetooth address of the device. This can be obtained using the finsh command "nvds get_mac".
2. Use BLE software (LightBlue, nRF Connect, etc.) on iOS devices (iPhone or iPad) to connect to this device. Note that AMS requires pairing to complete, so you must accept when the pairing dialog appears on the iOS device.
3. Play music on the iOS device. This example has registered AMS Player/queue/track, and you can see related content in the logs.
    1) Related protocol reference: [AMS Official Documentation](https://developer.apple.com/library/archive/documentation/CoreBluetooth/Reference/AppleMediaService_Reference/Specification/Specification.html)


### Hardware Requirements
Before running this example, prepare:
+ One development board supported by this example ([Supported Platforms](#Platform_ams)).
+ iOS device.

### menuconfig Configuration
1. Enable Bluetooth (`BLUETOOTH`):
    - Path: Sifli middleware → Bluetooth
    - Enable: Enable bluetooth
        - Macro switch: `CONFIG_BLUETOOTH`
        - Description: Enable Bluetooth functionality
2. Enable GAP, GATT Client, BLE connection manager, and AMS:
    - Path: Sifli middleware → Bluetooth → Bluetooth service → BLE service
    - Enable: Enable BLE GAP central role
        - Macro switch: `CONFIG_BLE_GAP_CENTRAL`
        - Description: Switch for BLE CENTRAL (central device). When enabled, provides scanning and active connection initiation with peripherals.
    - Enable: Enable BLE GATT client
        - Macro switch: `CONFIG_BLE_GATT_CLIENT`
        - Description: Switch for GATT CLIENT. When enabled, can actively search for and discover services, read/write data, and receive notifications.
    - Enable: Enable BLE connection manager
        - Macro switch: `CONFIG_BSP_BLE_CONNECTION_MANAGER`
        - Description: Provides BLE connection control management, including multi-connection management, BLE pairing, link connection parameter updates, etc.
    - Enable: Enable BLE AMS
        - Macro switch: `CONFIG_BSP_BLE_AMS`
        - Description: Apple Media Service. After registering AMS, provides iOS device playback control, playback synchronization, volume control, etc.
3. Enable AMS in Data service:
    - Path: Sifli middleware → Enable Data service
    - Enable: Enable AMS Service
        - Macro switch: `CONFIG_BSP_USING_AMS_SVC`
        - Description: When using Data Service to register AMS, this option needs to be enabled
4. Enable NVDS:
    - Path: Sifli middleware → Bluetooth → Bluetooth service → Common service
    - Enable: Enable NVDS synchronous
        - Macro switch: `CONFIG_BSP_BLE_NVDS_SYNC`
        - Description: Bluetooth NVDS synchronization. When Bluetooth is configured to HCPU, BLE NVDS can be accessed synchronously, so enable this option; when Bluetooth is configured to LCPU, this option needs to be disabled.

### Compilation and Flashing
Switch to the example project directory and run the scons command to compile:
```c
> scons --board=eh-lb525 -j32
```
Switch to the example `project/build_xx` directory and run `uart_download.bat`, then select the port as prompted to download:
```c
$ ./uart_download.bat

     Uart Download

please input the serial port num:5
```
For detailed compilation and download steps, please refer to the [Quick Start Guide](/quickstart/get-started.md).

## Expected Results
<!-- Describe the expected results of running the example, such as which LEDs will light up, what logs will be printed, to help users determine if the example is running normally. Results can be explained step by step in conjunction with the code -->
After the example starts:
1. It can be connected and successfully paired by BLE software on iOS (such as LightBlue, nRF Connect).
2. When the iOS device plays music, the device displays related information through logs.

## Troubleshooting


## Reference Documentation
<!-- For rt_device examples, RT-Thread official documentation provides detailed explanations. Web links can be added here, for example, refer to RT-Thread's [RTC Documentation](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/device/rtc/rtc) -->

## Update History
|Version |Date   |Release Notes |
|:---|:---|:---|
|0.0.1 |01/2025 |Initial version |
| | | |
| | | |