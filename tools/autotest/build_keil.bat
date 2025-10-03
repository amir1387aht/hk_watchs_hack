REM Install python package in \\ftpserver\Software2\regression\gitroot\wvt_v10\utilities\python packages\graphical module(s)\colorama-0.3.9
REM Install serial port driver in \\ftpserver\Software2\ganghe\LCUS-2型双路USB 继电器\驱动
REM Add D:\Users\regression\gitroot\rt-thread\env\env\tools\Python27\Scripts to PATH
REM echo on

set SIFLI_SDK=%cd%

set TOOLS_ROOT=%cd%/tools

set RTT_EXEC_PATH=C:\GNU MCU Eclipse\ARM Embedded GCC\8.2.1-1.2-20190119-1237\bin
set RTT_CC=gcc
dir
call set_env.bat
cd %1

date /t && time /t

if exist resources (
    cd resources
    call scons --res=%1
    if %errorlevel% neq 0 exit /b %errorlevel%
    cd ..
)

set RTT_EXEC_PATH=C:/Keil_v5
set RTT_CC=keil
set build_result=0
echo "build start" > log.txt

if "%2"=="" goto :GEN_PROJ
if "%2"=="0" goto :BUILD

set WIDTH=%2
set HEIGHT=%3
set TYPE=%4
set FONT=%5
mkdir resource\images
%TOOLS_ROOT%/png2ezip/ezip -dir ..\..\resource\images -cfile %TYPE% %WIDTH%x%HEIGHT% -outdir resource/images

::mkdir ..\..\resource\lang\output
mkdir resource\lang
mkdir resource\font

REM generate lang resource 
%TOOLS_ROOT%/sifli_develop/language/lang_gen.exe ..\..\resource\lang  ..\..\..  resource\lang
::xcopy ..\..\resource\lang\output resource\lang /y
::del /S /Q ..\..\resource\lang\output\*

REM generate font resource
%TOOLS_ROOT%/sifli_develop/font2c/font2c.exe ..\..\resource\fonts\freetype\%FONT%.ttf 
move /Y lvsf_font_%FONT%_ttf.c resource\font

:GEN_PROJ

call scons -j8 --target=mdk5 -s
if %errorlevel% neq 0 exit /b %errorlevel%

:BUILD
start /wait %RTT_EXEC_PATH%\UV4\UV4.exe -j0 -r project.uvprojx -o log.txt

echo error_level=%errorlevel%
set build_result=%errorlevel%
if %build_result% equ 1 set build_result=0

:BUILD_END
    date /t && time /t
    type log.txt
    cd %SIFLI_SDK%
    exit /b %build_result%
