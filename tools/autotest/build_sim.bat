@echo off
set SIFLI_SDK=%cd%

set TOOLS_ROOT=%cd%\tools\sifli_develop\Butterfli

date /t && time /t

cd %TOOLS_ROOT%

if "%1"=="watch\sifli\project\simulator" goto sifli_build

::if "%1"=="sifli" goto sifli
::if "%1"=="kct" goto kct


:sifli_build
echo sifli_build start
Butterfli.exe --group ec-lb551_v1.0_LB55SPI17801 --simu 1 --hide 1 
goto simu


:simu
cd %SIFLI_SDK%

cd %1

call devenv project.sln /build
if %errorlevel% neq 0 exit /b %errorlevel%

date /t && time /t

cd %SIFLI_SDK%
