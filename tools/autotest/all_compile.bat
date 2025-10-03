@echo off
setlocal EnableDelayedExpansion
set DAY=%date:~0,4%-%date:~5,2%-%date:~8,2%
set h=%time:~0,2%
set h=%h: =0%
set HMS=%h%-%time:~3,2%-%time:~6,2%
set DT=%DAY%-%HMS%
:: Create a folder to keep the results of Jenkins  builds
set root_Dir=%cd%
set bk_dir=%cd%\FailLog
::DIR
call set_env.bat
SET PROJECTS_Length=26


SET PROJECTS[0].Name=example\ble\ancs_dualcore\project\eh-lb555\lcpu
SET PROJECTS[1].Name=example\ble\ancs_dualcore\project\eh-lb555\hcpu
SET PROJECTS[2].Name=example\ble\ancs_dualcore\project\eh-ss6600_551\lcpu
SET PROJECTS[3].Name=example\ble\ancs_dualcore\project\eh-ss6600_551\hcpu

SET PROJECTS[4].Name=example\security\hcpu\project\eh-lb525

SET PROJECTS[5].Name=example\watch_demo\project\ec-lb583_v11
SET PROJECTS[6].Name=example\watch_demo\project\ec-lb587_v11
SET PROJECTS[7].Name=example\watch_demo\project\eh-lb561
SET PROJECTS[8].Name=example\watch_demo\project\eh-lb563
SET PROJECTS[9].Name=example\watch_demo\project\eh-lb555
SET PROJECTS[10].Name=example\watch_demo\project\eh-ss6600_551
SET PROJECTS[11].Name=example\watch_demo\project\eh-lb523
SET PROJECTS[12].Name=example\watch_demo\project\eh-lb525

SET PROJECTS[13].Name=test\drivers\project\ec-lb563\hcpu
SET PROJECTS[14].Name=test\drivers\project\ec-lb58xxxxxx001\v11_583
SET PROJECTS[15].Name=test\drivers\project\wvt\project\ec-lb563
SET PROJECTS[16].Name=test\drivers\project\wvt\project\ec-lb58x_general
SET PROJECTS[17].Name=example\watch_demo\project\simulator
SET PROJECTS[18].Name=example\watch_demo\project\eh-ss6600a8
SET PROJECTS[19].Name=example\watch_demo\project\ec-lb551
SET PROJECTS[20].Name=example\watch_demo\project\ec-lb555
SET PROJECTS[21].Name=example\lvgl_v8_demos\simulator
SET PROJECTS[22].Name=example\lvgl_v8_examples\simulator
SET PROJECTS[23].Name=example\lvgl_v9_demos\simulator
SET PROJECTS[24].Name=example\lvgl_v9_examples\simulator
SET PROJECTS[25].Name=example\lvgl_v8_media\simulator

set NORMAL_RESULT=0
REM ##################  LOOP_START  #######################
SET PROJECTS_Index=0
echo ***************FAILED PROJECTS***************> failproject.txt
:LoopStart
IF %PROJECTS_Index% EQU %PROJECTS_Length% GOTO NORMAL_BUILD_END
SET PROJECTS_Current.Name=0

FOR /F "usebackq delims==. tokens=1-3" %%I IN (`SET PROJECTS[!PROJECTS_Index!]`) DO (
  SET PROJECTS_Current.%%J=%%K
)

echo,-----------Build project[!PROJECTS_Current.Name!] start------------
    set Currentprj=!PROJECTS_Current.Name:\=_!
    ::echo !PROJECTS_Index! 
    ::echo !PROJECTS_Current.Name!
    tools\autotest\PrjCompile.exe scons !PROJECTS_Current.Name! %Currentprj%.txt 1
    if !errorlevel! neq 0 (
    echo [!PROJECTS_Current.Name!]>> failproject.txt
    xcopy %Currentprj%.txt FailLog\  /y
    del %Currentprj%.txt
    set NORMAL_RESULT=1) else (
    del %Currentprj%.txt
    )
echo,-----------Build project[!PROJECTS_Current.Name!] end------------
    cls

SET /A PROJECTS_Index=!PROJECTS_Index! + 1
GOTO LoopStart
REM ##################  LOOP_END  #######################

:NORMAL_BUILD_END
set /a PROJECTS_CNT=%PROJECTS_Length%-1
echo ---------------BUILD PROJECTS -----------------
FOR /l %%i IN (0,1,%PROJECTS_CNT%) DO (
  echo, !PROJECTS[%%i].Name!
)
ECHO NORMAL_BUILD_RESULT: %NORMAL_RESULT%
ECHO -------------------------------
ECHO COMMON PROJECT START COMPILE
ECHO -------------------------------


::eh-lb551_hcpu eh-lb555_hcpu eh-lb563_hcpu eh-lb561_hcpu
::eh-lb523_hcpu eh-lb525_hcpu ec-lb583_hcpu ec-lb587_hcpu
::eh-ss6500_hcpu eh-lb520_hcpu

SET Dir_Length=24
SET Dir[0]=example\ble\ams\project\common
SET Dir[1]=example\ble\central_and_peripheral\project\common
SET Dir[2]=example\ble\hid\project\common
SET Dir[3]=example\ble\peripheral\project\common
SET Dir[4]=example\bt\test_example\project\common
SET Dir[5]=example\hal_example\project\common
SET Dir[6]=example\multicore\data_service\common\hcpu
SET Dir[7]=example\multicore\ipc_queue\common\hcpu
SET Dir[8]=example\rt_driver\project\common
SET Dir[9]=example\uart\project\common
SET Dir[10]=example\pm\ble\common\hcpu
SET Dir[11]=example\pm\bt\project\common\hcpu
SET Dir[12]=example\pm\coremark\common\hcpu
SET Dir[13]=example\lvgl_v8_demos\project
SET Dir[14]=example\lvgl_v8_examples\project
SET Dir[15]=example\lvgl_v9_demos\project
SET Dir[16]=example\lvgl_v9_examples\project
SET Dir[17]=example\blink\rtt\project
SET Dir[18]=example\blink\no-os\project
SET Dir[19]=example\file_system\project
SET Dir[20]=example\hello_world\no-os\project
SET Dir[21]=example\hello_world\rtt\project
SET Dir[22]=example\lvgl_v8_media\project
SET Dir[23]=example\multicore\rpmsg-lite\project\hcpu


set /a Dir_CNT=%Dir_Length%-1
set COMMON_RESULT=0
::echo ***************FAILED PROJECTS***************> failproject.txt
for /l %%n in (0,1,%Dir_CNT%) do (
    cd %root_Dir%
    echo !Dir[%%n]!
    set "commands=eh-lb563_hcpu eh-lb561_hcpu eh-lb525_hcpu eh-lb523_hcpu eh-lb551_hcpu eh-lb555_hcpu ec-lb587_hcpu ec-lb583_hcpu em-lb525 em-lb566 em-lb567 em-lb587"
    if !Dir[%%n]!==example\bt\test_example\project\common (
        set "commands=eh-lb563_hcpu eh-lb561_hcpu eh-lb525_hcpu eh-lb523_hcpu ec-lb587_hcpu ec-lb583_hcpu "
    )
    if !Dir[%%n]!==example\pm\bt\project\common\hcpu (
        set "commands=eh-lb563_hcpu eh-lb561_hcpu eh-lb525_hcpu eh-lb523_hcpu ec-lb587_hcpu ec-lb583_hcpu em-lb525 em-lb566 em-lb567 em-lb587"
    )
    if !Dir[%%n]!==example\multicore\data_service\common\hcpu (
        set "commands=eh-lb563_hcpu eh-lb561_hcpu eh-lb551_hcpu eh-lb555_hcpu ec-lb587_hcpu ec-lb583_hcpu em-lb566 em-lb567 em-lb587"
    )
    if !Dir[%%n]!==example\multicore\ipc_queue\common\hcpu (
        set "commands=eh-lb563_hcpu eh-lb561_hcpu eh-lb551_hcpu eh-lb555_hcpu ec-lb587_hcpu ec-lb583_hcpu em-lb566 em-lb567 em-lb587"
    )
    if !Dir[%%n]!==example\pm\ble\common\hcpu (
        set "commands=eh-lb551_hcpu eh-lb555_hcpu"
    )
    if !Dir[%%n]!==example\multicore\rpmsg-lite\project\hcpu (
        set "commands=eh-lb563_hcpu eh-lb561_hcpu eh-lb551_hcpu eh-lb555_hcpu ec-lb587_hcpu ec-lb583_hcpu em-lb566 em-lb567 em-lb587"
    )
    cd !Dir[%%n]!
    for %%c in (!commands!) do (
    echo - %%c -
    set Currentprj=!Dir[%%n]:\=_!
    set logfile=!Currentprj!_%%c
    call scons --board=%%c -j8 1>!logfile!.log 2>&1
    if !errorlevel! neq 0 (
    echo [!logfile!]>> %root_Dir%\failproject.txt
    xcopy !logfile!.log %bk_dir%\>nul  /y
    set COMMON_RESULT=1)
)
)

:COMMON_BUILD_END
ECHO COMMON_BUILD_RESULT: %COMMON_RESULT%
ECHO -------------------------------
ECHO COMMON PROJECT COMPILE END
ECHO -------------------------------

:BUILD_END
cd %root_Dir%
IF %NORMAL_RESULT%%COMMON_RESULT%==00 (
    SET RESULT=0
) ELSE (
    SET RESULT=1
)

if %RESULT% equ 0 (
echo.
echo #########        #          ######        ######
echo #       #       # #        #             #
echo #       #      #   #      #             #
echo #########     #     #     #             #
echo #            #########     ########      ########
echo #           #         #            #             #
echo #          #           #           #             #
echo #         #             #   #######      ########
echo.
exit /b %COMMON_RESULT%
) ELSE (
echo.
echo #########        #         #########    #
echo #               # #            #        #
echo #              #   #           #        #
echo #########     #     #          #        #
echo #            #########         #        #
echo #           #         #        #        #
echo #          #           #       #        #
echo #         #             #  ##########   ##########
echo.
xcopy %bk_dir%\*.* ..\sdk-all-ci-results\%BUILD_ID%\>nul  /e /y
xcopy %bk_dir%\*.* Z:\sdk-all-ci-results\%DT%_FailLog\>nul  /e /y
type failproject.txt 
exit /b %RESULT%
)

