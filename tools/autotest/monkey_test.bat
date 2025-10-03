@echo off

for /F %%i in ('git rev-parse HEAD') do ( set HASH=%%i)

set DAY=%date:~0,4%-%date:~5,2%-%date:~8,2%
set h=%time:~0,2%
set h=%h: =0%
set HMS=%h%-%time:~3,2%-%time:~6,2%
set DT=%DAY%-%HMS%



set Sources_DIR=D:\Users\gitlab-runner\builds\irjvhm37\0\sw\bt
set RESULT_DIR=D:\Users\gitlab-runner\builds\irjvhm37\0\sw\Result\%HASH%
set TEST_DIR=D:\Users\gitlab-runner\builds\irjvhm37\0\sw\Test
set PRJ_NAME=ec-lb555_LB55DSI13903
set JLINK=59605049
set UART3=com8
::set SWITCH=com18
set RTYPE=1
set RCHAN=3
set BCHAN=4
mkdir %RESULT_DIR%
mkdir %RESULT_DIR%\memory
mkdir %TEST_DIR%\memory
copy /Y \\10.23.20.6\jenkins\jenkins_tool\*.exe %TEST_DIR%
copy /Y \\10.23.20.6\jenkins\jenkins_tool\*.dll %TEST_DIR%
copy /Y \\10.23.20.6\jenkins\jenkins_tool\file\*.*  %TEST_DIR%\file
copy /Y \\10.23.20.6\jenkins\jenkins_tool\monkey\*.*  %TEST_DIR%\monkey
copy /Y \\10.23.20.6\jenkins\jenkins_tool\*.jlink %TEST_DIR%
copy /Y \\10.23.20.6\jenkins\jenkins_tool\save_ram_a0.bat %TEST_DIR%
echo STEP_1 ------build project %PRJ_NAME%----------------------------------------

::set DAY=%date:~0,4%-%date:~5,2%-%date:~8,2%
::mkdir Z:\solution_version\%DAY%\%HASH%\%PRJ_NAME%


cd tools\sifli_develop\Butterfli
dir

Butterfli.exe --group %PRJ_NAME% --binpath %RESULT_DIR% --logpath %RESULT_DIR%\cblog.txt 
if %errorlevel% neq 0 exit /b %errorlevel%


echo STEP_2----download img-----------------------------------------
cd %TEST_DIR%
set cnt=0
:PREBURN
ping 127.0.0.1 -n 1
SwitchReset.exe --type %RTYPE% --port %SWITCH% --baund 9600 --msdelay 1000 --rchan %RCHAN% --bchan %BCHAN% --blevel 1
if %errorlevel% EQU -2 goto PREBURN
if %errorlevel% EQU -1 exit /b %errorlevel%

set /a cnt+=1
if %cnt% gtr 3 exit -1
ImgDownUart.exe --port %UART3% --baund 3000000 --postact 0 --path %RESULT_DIR% --log %RESULT_DIR%\burnlog.txt
if %errorlevel% neq 0 goto PREBURN

:POSTBURN
ping 127.0.0.1 -n 1
SwitchReset.exe --type %RTYPE% --port %SWITCH% --baund 9600 --msdelay 1000 --rchan %RCHAN% --bchan %BCHAN% --blevel 0
if %errorlevel% EQU -2 goto POSTBURN
if %errorlevel% EQU -1 exit /b %errorlevel%

echo STEP_3-------start test-------------------------------------------

call Thinker.exe --exit 1 --hardver %PRJ_NAME% --softver gitlab_runner --logpath %RESULT_DIR%\log
if %errorlevel% neq 0 goto ERR_END

copy /Y CaseInfo.xlsx  %RESULT_DIR%\CaseInfo.xlsx 
exit 0

:ERR_END
echo "TEST FAILED dump memory"
mkdir \\10.23.20.6\jenkins\gitlab_runner_monkey_test\%PRJ_NAME%_%HASH%
set dumpdir=\\10.23.20.6\jenkins\gitlab_runner_monkey_test\%PRJ_NAME%_%HASH%
call save_ram_a0.bat %JLINK% %RESULT_DIR%\memory
copy /Y CaseInfo.xlsx  %RESULT_DIR%\CaseInfo.xlsx
echo D|xcopy %RESULT_DIR% %dumpdir% /E
echo "DUMP MEMORY SUCCESS END"
exit -1