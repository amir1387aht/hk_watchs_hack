set SIFLI_SDK=%cd%

set TOOLS_ROOT=%cd%/tools

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

dir
if "%2"=="" goto :BUILD
if "%2"=="--resource" goto :RESOURCE

set WIDTH=%2
set HEIGHT=%3
set TYPE=%4
set FONT=%5

if "%6"=="--resource" goto :RESOURCE

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
goto :BUILD

:RESOURCE
call scons resource --no_cc
if %errorlevel% neq 0 exit /b %errorlevel%

:BUILD

call scons -j8 --target=vs2017 -s
if %errorlevel% neq 0 exit /b %errorlevel%

call devenv project.sln /build
if %errorlevel% neq 0 exit /b %errorlevel%

date /t && time /t

cd %SIFLI_SDK%
