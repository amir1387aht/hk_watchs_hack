@echo off

set DS=Thinker_DS
set FS=Thinker_FS
set IPC=Thinker_IPC
set HAL=Thinker_HAL
set UART=Thinker_UART
set RPMSG=Thinker_RPMSG
set BLINK_RTT=Thinker_BLINK
set BLINK_NO_OS=Thinker_BLINK
set HELLO_WORLD_RTT=Thinker_HELLO
set HELLO_WORLD_NO_OS=Thinker_HELLO


set PARAM=%1

if "%PARAM%"=="DS" (
    set TEST_WORKSPACE=%DS%
) else if "%PARAM%"=="FS" (
    set TEST_WORKSPACE=%FS%
) else if "%PARAM%"=="IPC" (
    set TEST_WORKSPACE=%IPC%
) else if "%PARAM%"=="HAL" (
    set TEST_WORKSPACE=%HAL%
) else if "%PARAM%"=="UART" (
    set TEST_WORKSPACE=%UART%
) else if "%PARAM%"=="RPMSG" (
    set TEST_WORKSPACE=%RPMSG%
) else if "%PARAM%"=="BLINK_RTT" (
    set TEST_WORKSPACE=%BLINK_RTT%
) else if "%PARAM%"=="BLINK_NO_OS" (
    set TEST_WORKSPACE=%BLINK_NO_OS%
) else if "%PARAM%"=="HELLO_WORLD_RTT" (
    set TEST_WORKSPACE=%HELLO_WORLD_RTT%
) else if "%PARAM%"=="HELLO_WORLD_NO_OS" (
    set TEST_WORKSPACE=%HELLO_WORLD_NO_OS%
) else (
    echo Î´Öª²ÎÊý: %PARAM%
    exit /b 1
)
echo ²½Öè1£¬%PARAM% ±àÒë/ÏÂÔØ/²âÊÔ£º--------------------------------------

set RESULT_DIR="%WORKSPACE%_Test\Result\%BUILD_ID%_%BUILD_USER_ID%_%W_BRANCH_NAME:~7,50%_%GIT_COMMIT:~0,8%\%PARAM%"
set BAK_DIR="Z:\CITEST_SDK\%JOB_BASE_NAME%\%BUILD_ID%_%W_BRANCH_NAME:~7,50%_%GIT_COMMIT:~0,8%\%PARAM%\"
echo.
echo  -------------- Print Test info --------------
echo RESULT_DIR -- %RESULT_DIR%
echo TEST_DIR -- %TEST_DIR%
echo DownloadTool=%DownloadTool%
echo PRJ_NAME -- %PRJ_NAME%
echo UART_PORT-- %UART_PORT%
echo RELAY_PORT -- %RELAY_PORT%
echo RTYPE -- %RTYPE%
echo RCHAN -- %RCHAN%
echo BCHAN -- %BCHAN%
echo DEVICE_TYPE -- %DEVICE_TYPE%
echo W_BRANCH_NAME=%W_BRANCH_NAME%


echo.
echo ²½Öè1.1£¬%PARAM% ±àÒë£º--------------------------------------

setlocal EnableDelayedExpansion

set root_path=%cd%
set compile_path=%2
call set_env.bat %3
cd %compile_path%


echo,-----------Build %PRJ_NAME% start------------
call scons --board=%PRJ_NAME% -j8
if !errorlevel! neq 0 goto BUILD_FAIL
echo,-----------Build project[!PRJ_NAME!] end------------

  


:BUILD_PASS
echo #########        #          ######        ######
echo #       #       # #        #             #
echo #       #      #   #      #             #
echo #########     #     #     #             #
echo #            #########     ###### #      ########
echo #           #         #            #             #
echo #          #           #           #             #
echo #         #             #   #######      ########
SET BUILD_RESULT=0
mkdir %RESULT_DIR%
goto BUILD_END

:BUILD_FAIL
echo Build project[%PRJ_NAME%] fail.
echo #########        #         #########    #
echo #               # #            #        #
echo #              #   #           #        #
echo #########     #     #          #        #         
echo #            #########         #        #
echo #           #         #        #        #
echo #          #           #       #        #
echo #         #             #  ##########   ##########
SET BUILD_RESULT=1
goto BUILD_END

:BUILD_END
cd %root_path%
if %BUILD_RESULT% equ 0 (
    robocopy  %compile_path%\build_%PRJ_NAME% %RESULT_DIR% *.bin *.hex  *.ini  /S /LEV:3
) else (
    set BUILD_RESULT_DOWNLOAD_RESULT=3
    set BUILD_RESULT_TEST_RESULT=3
    goto ERREND
)



echo ²½Öè1.2£¬%PARAM% ÏÂÔØ£º--------------------------------------
cd %TEST_DIR%
set cnt=0

SET DOWNLOAD_RESULT=0

:PREBURN
MsDelay.exe 2000
SwitchReset.exe --type %RTYPE% --port %RELAY_PORT% --baund 9600 --msdelay 1000 --rchan %RCHAN% --bchan %BCHAN% --blevel 1
if %errorlevel% EQU -2 goto PREBURN
if %errorlevel% EQU -1 (
    SET DOWNLOAD_RESULT=1
    set TEST_RESULT=3
    goto ERREND
)

set /a cnt+=1
if %cnt% gtr 2  (
    SET DOWNLOAD_RESULT=1
    set TEST_RESULT=3
    goto ERREND
)
%DownloadTool%ImgDownUart.exe --port %UART_PORT% --baund 3000000 --func 0 --loadram 1 --postact 1 --device %DEVICE_TYPE% --file %RESULT_DIR%\ImgBurnList.ini --log %RESULT_DIR%\ImgBurn.log
if %errorlevel% neq 0 goto PREBURN

:POSTBURN
MsDelay.exe 2000
SwitchReset.exe --type %RTYPE% --port %RELAY_PORT% --baund 9600 --msdelay 1000 --rchan %RCHAN% --bchan %BCHAN% --blevel 0
::SwitchReset.exe --type %RTYPE% --msdelay 1000 --rchan %RCHAN% --bchan %BCHAN% --blevel 0

if %errorlevel% EQU -2 goto POSTBURN
if %errorlevel% EQU -1 (
    SET DOWNLOAD_RESULT=1
    set TEST_RESULT=3
    goto ERREND
)

echo ²½Öè1.3£¬%PARAM% ²âÊÔ£º--------------------------------------

set TEST_RESULT=0
cd %TEST_DIR%\%TEST_WORKSPACE%
call Thinker.exe --exit 1 --hardver  %PRJ_NAME%  --softver %GIT_COMMIT:~0,8% --logpath %RESULT_DIR%\log
if %errorlevel% NEQ 0 (
    SET TEST_RESULT=1
    goto ERREND
)


:ERREND
copy /Y CaseInfo.xlsx  %RESULT_DIR%\CaseInfo.xlsx 
xcopy %RESULT_DIR% %BAK_DIR% /e /y /q
SwitchReset.exe --type %RTYPE% --port %RELAY_PORT% --baund 9600 --msdelay 1000 --rchan %RCHAN% --bchan %BCHAN% --blevel 0
cd  %WORKSPACE%
echo.
set /a all_result=%BUILD_RESULT%+%DOWNLOAD_RESULT%+%TEST_RESULT%

if %all_result%==9 set STEP_RESULT=2
if %all_result% LSS 9  set STEP_RESULT=1
if %all_result%==0 set STEP_RESULT=0

echo.
echo  -------------- %PARAM% STEP RESULT --------------
echo %PARAM%_BUILD_RESULT -- %BUILD_RESULT%
echo %PARAM%_DOWNLOAD_RESULT -- %DOWNLOAD_RESULT%
echo %PARAM%_TEST_RESULT -- %TEST_RESULT%
echo %PARAM%_STEP_RESULT -- %STEP_RESULT%
echo  -------------- %PARAM% STEP RESULT --------------
echo.
echo  -------------- %PARAM% STEP RESULT -------------- >> properties.properties
echo %PARAM%_BUILD_RESULT=%BUILD_RESULT% >> properties.properties
echo %PARAM%_DOWNLOAD_RESULT=%DOWNLOAD_RESULT% >> properties.properties
echo %PARAM%_TEST_RESULT=%TEST_RESULT% >> properties.properties
echo %PARAM%_STEP_RESULT=%STEP_RESULT% >> properties.properties
echo.>> properties.properties
