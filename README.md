# hk_watchs_hack
This respository contains SDK and Project file for reverse engineering of hk watchs, video [link]()

# Install SDK
You should Install the sdk and config it, you can see [here](https://docs.sifli.com/projects/sdk/latest/sf32lb52x/quickstart/install/script/windows.html)

# Compile
Open the ``` projects\hk10ultra3_firmware\v1\watch\project ``` path with SiFli-SDK Enviroment and type ``` scons --board=eh-lb525 -j8 ```, Project will compile

# Menu Config
Follow steps as befor and write ``` scons --menuconfig --board=eh-lb525 ```

# Flash
Connect Uart Converter that explained in video and write ``` build_eh-lb525_hcpu/uart_download.bat ``` and select your COM port
