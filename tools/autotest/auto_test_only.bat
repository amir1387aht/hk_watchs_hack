REM Install python package in \\ftpserver\Software2\regression\gitroot\wvt_v10\utilities\python packages\graphical module(s)\colorama-0.3.9
REM Install serial port driver in \\ftpserver\Software2\ganghe\LCUS-2型双路USB 继电器\驱动
REM Add D:\Users\regression\gitroot\rt-thread\env\env\tools\Python27\Scripts to PATH

@echo off
set RTT_ROOT=..\..\..\rt-thread\rt-thread\
set BSP_ROOT=.
net use z: \\ftpserver\Software2\regression
for /F "tokens=2-4 delims=/ " %%i in ('date /t') do set _date=%%i%%j%%k
for /F "tokens=1-2 delims=:/" %%i in ('time /t') do set _time=%%i%%j
set tag=regression_test_only_%_date%%_time%
set GIT_ROOT=Z:\gitroot
Z:
cd %GIT_ROOT%\bt
mkdir %GIT_ROOT%\logs\%tag%
C:\Keil_v5\ARM\Segger\JLink.exe -Device CORTEX-M33 -SelectEmuBySN 63640316 -CommanderScript %GIT_ROOT%\bt\tools\autotest\jlink_command.txt
cd %GIT_ROOT%\wvt_v10\tool\wvt\src
python wvt.py --hw_reset -l %GIT_ROOT%\wvt_v10\tool\wvt\tc\ble\lists\ll\00_ble_ll_acceptance_tc_list.txt 
cd 
call move %GIT_ROOT%\wvt_v10\tool\wvt\logs %logfile%
REM git push --tags         -- Push tags to server 
REM git tag --delete %tag%
timeout /t 30
d:
