# BLE File Transfer Example
Source code path: example/ble/file_transfer
(Platform_file_rec)=
## Supported Platforms
<!-- Which boards and chip platforms are supported -->
All platforms
## Overview
<!-- Brief introduction of the example -->
This example demonstrates how to implement a GAP peripheral role on this platform to receive files sent from a mobile device.
## Example Usage
<!-- Instructions on how to use the example, such as connecting hardware pins to observe waveforms, compilation and downloading can reference related documents.
For rt_device examples, also need to list the configuration switches used in this example, such as PWM example uses PWM1, need to enable PWM1 in onchip menu -->
1. The Finsh commands for this example can be displayed by entering `diss help` to show command usage.
2. When operating as a peripheral device, it will start advertising on boot with the name format: SIFLI_APP-xx-xx-xx-xx-xx-xx, where xx represents the device's Bluetooth address. You can connect using a mobile BLE app.
3. The sifli ble serial transport protocol can be used to send files from the mobile device to the target device, supporting one or multiple files in a single transfer.
### Hardware Requirements
Before running this example, you need to prepare:
+ A development board supported by this example ([Supported Platforms](#Platform_file_rec)).
+ A mobile device.
### menuconfig Configuration
1. Enable Bluetooth(`BLUETOOTH`):
    - Path: Sifli middleware → Bluetooth
    - Enable: Enable bluetooth
        - Macro switch: `CONFIG_BLUETOOTH`
        - Description: Enable Bluetooth functionality
2. Enable GAP, GATT Client, BLE connection manager:
    - Path: Sifli middleware → Bluetooth → Bluetooth service → BLE service
    - Enable: Enable BLE GAP central role
        - Macro switch: `CONFIG_BLE_GAP_CENTRAL`
        - Description: Switch for BLE CENTRAL (central device). When enabled, it provides scanning and active connection initiation with peripherals.
    - Enable: Enable BLE GATT client
        - Macro switch: `CONFIG_BLE_GATT_CLIENT`
        - Description: Switch for GATT CLIENT. When enabled, it can actively search and discover services, read/write data, and receive notifications.
    - Enable: Enable BLE connection manager
        - Macro switch: `CONFIG_BSP_BLE_CONNECTION_MANAGER`
        - Description: Provides BLE connection control management, including multi-connection management, BLE pairing, link connection parameter updates, etc.
3. Enable NVDS:
    - Path: Sifli middleware → Bluetooth → Bluetooth service → Common service
    - Enable: Enable NVDS synchronous
        - Macro switch: `CONFIG_BSP_BLE_NVDS_SYNC`
        - Description: Bluetooth NVDS synchronization. When Bluetooth is configured to HCPU, BLE NVDS can be accessed synchronously, so enable this option; when Bluetooth is configured to LCPU, this option needs to be disabled
4. Enable transmission related macros:
    - Path: Sifli middleware → Bluetooth → Bluetooth service → BLE service
    - Enable: Enable BLE serial transmission
        - Macro switch: `CONFIG_BSP_BLE_SERIAL_TRANSMISSION`
        - Description: Serial transmission protocol. When enabled, it provides a simple serial transmission protocol that allows users to transmit content to the development board via BLE.
    - Enable: Enable watch face downloaded via BLE
        - Macro switch: `CONFIG_BSP_BLE_WATCH_FACE`
        - Description: Watch face transmission protocol. Depends on serial transmission. When enabled, files can be transferred to the development board via BLE using the encapsulated mobile SDK.
        
### Compilation and Flashing
Navigate to the example's project/common directory and run the scons command to compile:
```c
> scons --board=eh-lb525 -j32
```
Navigate to the example's `project/common/build_xx` directory and run `uart_download.bat`, then select the port as prompted to download:
```c
$ ./uart_download.bat
     Uart Download
please input the serial port num:5
```
For detailed compilation and downloading steps, please refer to the relevant sections in [Quick Start](/quickstart/get-started.md).
## Expected Results
<!-- Explain the expected outcome of the example, such as which LEDs will light up, what logs will be printed, to help users determine if the example is running normally. Results can be explained step by step in conjunction with code -->
After the example starts:
1. It can be discovered and connected by a mobile BLE app.
2. Compress the files to be transferred into a single .zip file and transfer this file to your mobile device.
3. Use the SIFLI BLE app to search for and connect to the device, then on the watchface interface, select custom file and choose the file to transfer it to the device.
4. Mobile app operation steps:
![APP1](./assets/app1.jpg)
![APP2](./assets/app2.jpg)
![APP3](./assets/app3.jpg)
![APP4](./assets/app4.jpg)
5. Transfer interface:
Set file type to 3 (custom file)
Check the withByteAlign option
Future development can define different processing for various file types
## Mobile SDK Integration
https://github.com/OpenSiFli/SiFli_OTA_APP
SiFli-SDK file transfer
## Troubleshooting
When the development board returns an error, check the ble_watchface_status_id_t enumeration in bf0_sibles_watchface.h
## Reference Documentation
<!-- For rt_device examples, RT-Thread official documentation provides detailed explanations, you can add web links here, for example, refer to RT-Thread's [RTC documentation](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/device/rtc/rtc) -->
## Update History
| Version | Date    | Release Notes |
|:--------|:--------|:--------------|
| 0.0.1   | 07/2025 | Initial version |
|         |         |               |
|         |         |               |
No newline at end of right file.