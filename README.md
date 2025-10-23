# hk_watchs_hack
This respository contains SDK and Project file for reverse engineering of hk watchs, watch the video [here]()

# Install SDK
You should Install the sdk and config it, you can reach [here](https://docs.sifli.com/projects/sdk/latest/sf32lb52x/quickstart/install/script/windows.html)

# Compile
Open the ``` projects\hk10ultra3_firmware\v1\watch\project ``` path with SiFli-SDK Enviroment and enter ``` scons --board=eh-lb525 -j8 ```, Project will compile

# Menu Config
Follow steps as before and enter ``` scons --menuconfig --board=eh-lb525 ```

# Flash
Connect Uart Converter that explained in video and enter ``` build_eh-lb525_hcpu/uart_download.bat ``` and select your COM port
