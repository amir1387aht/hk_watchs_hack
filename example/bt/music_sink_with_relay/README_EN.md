# BT Music Sink Relay Example

Source code path: example/bt/music_sink_with_relay

{#Platform_music_sink}
## Supported Platforms
<!-- Which development boards and chip platforms are supported -->
+ eh-lb52J

## Overview
<!-- Example introduction -->
This example demonstrates a music_sink-based implementation that requires two boards to download this project, enabling simultaneous music playback from a mobile phone on both devices.

## Example Usage
<!-- Instructions on how to use the example, such as which hardware pins to connect to observe waveforms, compilation and flashing can reference related documentation.
For rt_device examples, the configuration switches used in this example also need to be listed, for example, PWM examples use PWM1, which needs to be enabled in the onchip menu -->

The example consists of two parts:
1. One part involves operations between the device and mobile phone, same as music_sink. The project enables Bluetooth Inquiry scan and page scan on startup, allowing A2DP source devices like mobile phones to discover this device and initiate connections. After connection, mobile phone music can be played.
2. The other part involves operations between two boards. The two boards need to complete pairing to achieve music forwarding. Only one board can connect to the mobile phone at a time. The board not connected to the mobile phone will receive forwarded music information.

2.1 The board connected to the mobile phone will be configured as relay src role, otherwise you can set the role through the finsh command music set_role x (1 is relay sink; 2 is relay src). But ensure that relay src is the board connected to the mobile phone.
If pairing is in progress or already completed, setting the role will fail; you can check the current role through music get_role. Only src and sink can perform pairing.

2.2 After the roles of both boards are determined, use music pair to perform pairing. Successful pairing will show the log "A2DP relay paired! Role is x"

2.3 Use music unpair to cancel pairing between the two boards.

2.4 Before the two boards are paired, you can set the audio channels of the two boards through music set_chnl x (0 represents src left channel/sink right channel, 1 represents src right channel/sink left channel, 2 represents both sides play left channel).

3. The default Bluetooth name of this device is sifli_music_sink.

### Hardware Requirements
Before running this example, you need to prepare:
+ Two development boards supported by this example ([Supported Platforms](#Platform_music_sink)).
+ Speakers.

### menuconfig Configuration

1. Enable AUDIO CODEC and AUDIO PROC:
    - Path: On-chip Peripheral RTOS Drivers
    - Enable: Enable Audio Process driver
        - Macro switch: `CONFIG_BSP_ENABLE_AUD_PRC`
        - Description: Enable Audio process device, mainly used for audio data processing (including resampling, volume adjustment, etc.)
    - Enable: Enable Audio codec driver
        - Macro switch: `CONFIG_BSP_ENABLE_AUD_CODEC`
        - Description: Enable Audio codec device, mainly used for DAC conversion
2. Enable AUDIO(`AUDIO`):
    - Path: Sifli middleware
    - Enable: Enable Audio
        - Description: Enable audio configuration options
3. Enable AUDIO MANAGER(`AUDIO_USING_MANAGER`):
    - Path: Sifli middleware → Enable Audio
    - Enable: Enable audio manager
        - Macro switch: `CONFIG_AUDIO_USING_MANAGER`
        - Description: Use audio manager module for audio process handling
4. Enable Bluetooth(`BLUETOOTH`):
    - Path: Sifli middleware → Bluetooth
    - Enable: Enable bluetooth
        - Macro switch: `CONFIG_BLUETOOTH`
        - Description: Enable Bluetooth functionality
5. Enable A2DP SNK and AVRCP:
    - Path: Sifli middleware → Bluetooth → Bluetooth service → Classic BT service
    - Enable: Enable BT finsh (Optional)
        - Macro switch: `CONFIG_BT_FINSH`
        - Description: Enable finsh command line for Bluetooth control
    - Enable: Manually select profiles
        - Macro switch: `CONFIG_BT_PROFILE_CUSTOMIZE`
        - Description: Manually select profiles to enable
    - Enable: Enable A2DP
        - Macro switch: `CONFIG_CFG_AV`
        - Description: Enable A2DP
    - Enable: Enable A2DP sink profile
        - Macro switch: `CONFIG_CFG_AV_SNK`
        - Description: Enable A2DP SINK ROLE
    - Enable: Enable AVRCP
        - Macro switch: `CONFIG_CFG_AVRCP`
        - Description: Enable AVRCP profile
6. Enable BT connection manager:
    - Path: Sifli middleware → Bluetooth → Bluetooth service → Classic BT service
    - Enable: Enable BT connection manager
        - Macro switch: `CONFIG_BSP_BT_CONNECTION_MANAGER`
        - Description: Use connection manager module to manage BT connections
7. Enable NVDS:
    - Path: Sifli middleware → Bluetooth → Bluetooth service → Common service
    - Enable: Enable NVDS synchronous
        - Macro switch: `CONFIG_BSP_BLE_NVDS_SYNC`
        - Description: Bluetooth NVDS synchronization. When Bluetooth is configured to HCPU, BLE NVDS can be accessed synchronously, enable this option; when Bluetooth is configured to LCPU, this option needs to be disabled

### Compilation and Flashing
Switch to the example project directory and run the scons command to compile:
```c
> scons --board=eh-lb525 -j32
```
Switch to the example `project/build_xx` directory and run `uart_download.bat`, select the port as prompted to download:
```c
$ ./uart_download.bat

     Uart Download

please input the serial port num:5
```
For detailed compilation and download steps, please refer to the relevant introduction in [Quick Start](/quickstart/get-started.md).

## Expected Results
<!-- Describe the example execution results, such as which LEDs will light up, what logs will be printed, so users can determine if the example is running normally. Results can be explained step by step combined with code -->
After the example starts:
Mobile phone music forwarding can be achieved with synchronized sound between forwarding devices without stuttering.

## Exception Diagnosis

## Reference Documentation
<!-- For rt_device examples, RT-Thread official website documentation provides detailed explanations, web links can be added here, for example, refer to RT-Thread's [RTC Documentation](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/device/rtc/rtc) -->

## Update History
|Version |Date   |Release Notes |
|:---|:---|:---|
|0.0.1 |01/2025 |Initial version |
| | | |
| | | |
