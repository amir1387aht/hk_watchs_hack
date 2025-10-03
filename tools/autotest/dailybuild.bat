REM Install python package in \\ftpserver\Software2\regression\gitroot\wvt_v10\utilities\python packages\graphical module(s)\colorama-0.3.9
REM Install serial port driver in \\ftpserver\Software2\ganghe\LCUS-2型双路USB 继电器\驱动
REM Add D:\Users\regression\gitroot\rt-thread\env\env\tools\Python27\Scripts to PATH
REM echo on

set SIFLI_SDK=%cd%

set TOOLS_ROOT=%cd%/tools

set ymd=%date:~3,4%%date:~8,2%%date:~11,2%
set h=%time:~0,2%
set h=%h: =0%
set hms=%h%%time:~3,2%%time:~6,2%
md \\10.21.20.6\oa2vdi\Software\regression\dailybuild\%ymd%_%hms%

set RTT_EXEC_PATH=C:\GNU MCU Eclipse\ARM Embedded GCC\8.2.1-1.2-20190119-1237\bin
set RTT_CC=gcc
dir
call set_env.bat
cd %1


if exist resources (
    cd resources
    call scons --res=%1
    if %errorlevel% neq 0 exit /b %errorlevel%
    cd ..
)

set RTT_EXEC_PATH=C:/Keil_v5
set RTT_CC=keil
call scons -j8
if %errorlevel% neq 0 exit /b %errorlevel%

XCOPY %cd%\build\*.*  \\10.21.20.6\oa2vdi\Software\regression\dailybuild\%ymd%_%hms%   /e /s /y
XCOPY %cd%\build\*.*  \\10.21.20.6\oa2vdi\Software\regression\dailybuild\autotest   /e /s /y

if %1 equ boot_loader (
    call scons --ftab=1
    if %errorlevel% neq 0 exit /b %errorlevel%
)

cd %SIFLI_SDK%
