REM Install python package in \\ftpserver\Software2\regression\gitroot\wvt_v10\utilities\python packages\graphical module(s)\colorama-0.3.9
REM Install serial port driver in \\ftpserver\Software2\ganghe\LCUS-2型双路USB 继电器\驱动
REM Add D:\Users\regression\gitroot\rt-thread\env\env\tools\Python27\Scripts to PATH

set RTT_ROOT=..\..\..\rt-thread\rt-thread\
set BSP_ROOT=.
net use z: \\ftpserver\Software2\regression
for /F "tokens=2-4 delims=/ " %%i in ('date /t') do set _date=%%i%%j%%k
for /F "tokens=1-2 delims=:/" %%i in ('time /t') do set _time=%%i%%j
set tag=compile_%_date%%_time%
set GIT_ROOT=Z:\gitroot
Z:
cd %GIT_ROOT%\bt
mkdir %GIT_ROOT%\logs\%tag%
git checkout .
git pull
set logfile=%GIT_ROOT%\logs\%tag%
git log --oneline -n1 > %logfile%\last_commit.txt

cd %GIT_ROOT%\bt\rt_bsp\emb_bsp_m0
call scons --target=mdk5 -s --warn=no-all
uv4 -b project.uvprojx -r -l %logfile%\emb_bsp_m0.txt
echo emb_bsp_m0 %ERRORLEVEL% >> %logfile%\last_commit.txt

cd %GIT_ROOT%\bt\rt_bsp\emb_bsp_m33
call scons --target=mdk5 -s --warn=no-all
call uv4 -b project.uvprojx -r -l %logfile%\emb_bsp_m33.txt
echo emb_bsp_m33 %ERRORLEVEL% >> %logfile%\last_commit.txt

cd %GIT_ROOT%\bt\rt_bsp\emb_boot_m33
call scons --target=mdk5 -s --warn=no-all
call uv4 -b project.uvprojx -r -l %logfile%\emb_boot_m33.txt
echo emb_boot_m33 %ERRORLEVEL% >> %logfile%\last_commit.txt

cd %GIT_ROOT%\bt\rt_bsp\bsp_m0
call scons --target=mdk5 -s --warn=no-all
call uv4 -b project.uvprojx -r -l %logfile%\bsp_m0.txt
echo bsp_m0 %ERRORLEVEL% >> %logfile%\last_commit.txt

cd %GIT_ROOT%\bt\rt_bsp\bsp_m33
call scons --target=mdk5 -s --warn=no-all
call uv4 -b project.uvprojx -r -l %logfile%\bsp_m33.txt
echo bsp_m33 %ERRORLEVEL% >> %logfile%\last_commit.txt

cd %GIT_ROOT%\bt\rt_bsp\emwin_m0
call scons --target=mdk5 -s --warn=no-all
call uv4 -b project.uvprojx -r -l %logfile%\emwin_m0.txt
echo emwin_m0 %ERRORLEVEL% >> %logfile%\last_commit.txt

cd %GIT_ROOT%\bt\rt_bsp\emwin_m33
call scons --target=mdk5 -s --warn=no-all
call uv4 -b project.uvprojx -r -l %logfile%\emwin_m33.txt
echo emwin_m33 %ERRORLEVEL% >> %logfile%\last_commit.txt

cd %GIT_ROOT%\bt\rt_bsp\febsp_m0
call scons --target=mdk5 -s --warn=no-all
call uv4 -b project.uvprojx -r -l %logfile%\febsp_m0.txt
echo febsp_m0 %ERRORLEVEL% >> %logfile%\last_commit.txt

cd %GIT_ROOT%\bt\rt_bsp\gfx_bsp_m33
call scons --target=mdk5 -s --warn=no-all
call uv4 -b project.uvprojx -r -l %logfile%\gfx_bsp_m33.txt
echo gfx_bsp_m33 %ERRORLEVEL% >> %logfile%\last_commit.txt

cd %GIT_ROOT%\bt\rt_bsp\lvgl_m33
call scons --target=mdk5 -s --warn=no-all
call uv4 -b project.uvprojx -r -l %logfile%\lvgl_m33.txt
echo lvgl_m33 %ERRORLEVEL% >> %logfile%\last_commit.txt

timeout /t 30
d:
