@echo off
rem ---------------------------------------------------------------------------
rem  Author: Iruha
rem  Date: 2026-09-04
rem  Description: Offline mode - launcher for the SERV_IRUHADEV_OFFLINE client.
rem
rem  The canonical copy of this file is in the repo at
rem  X2Lib\Offline\start_offline.bat; it is deployed next to X2_offline.exe in
rem  the game data directory.
rem
rem  Since SERV_IRUHADEV_NO_PATCHER_TOKEN this script is NO LONGER REQUIRED -
rem  X2_offline.exe can be started directly. It is still the better way to
rem  start the game, for the one reason it always was:
rem
rem    The working directory MUST be the game data directory. X2Main mounts the
rem    .kom archives through a "./" prefix, and the offline server writes
rem    els_db.sql and both log files there. Double-clicking the exe in Explorer
rem    happens to be fine - Explorer starts a process in the exe's own
rem    directory - but a desktop or Start-menu shortcut with a different
rem    "Start in" field, or a launch from a terminal sitting somewhere else, is
rem    not: the client finds no content, or quietly starts a second, empty save
rem    file somewhere surprising. cd /d "%~dp0" makes that impossible.
rem
rem  The token below is PATCHER_RUN_ONLY from KTDXLIB\OnlyGlobal\Always_US.h.
rem  It is now redundant - X2\X2.cpp supplies the same constant itself - and is
rem  still passed so that this script also works against a build with
rem  SERV_IRUHADEV_NO_PATCHER_TOKEN commented out. Before that flag, a bare
rem  launch returned 0 out of WinMain with no window, no message and no log,
rem  which is what sent anyone who tried it looking for a crash.
rem ---------------------------------------------------------------------------

cd /d "%~dp0"

set EXE=X2_offline.exe
set TOKEN=pxk19slammsu286nfha02kpqnf729ck

if not exist "%EXE%" (
	echo.
	echo   %EXE% is not in this directory:
	echo     %CD%
	echo.
	echo   Build it and deploy it - see OFFLINE_MODE_PLAN.md, "Verification":
	echo     msbuild X2\X2_2010.vcxproj /p:Configuration=US_SERVICE /p:Platform=Win32 /p:SolutionDir=[Trunk]\
	echo     copy X2\US_SERVICE\x2.exe "%CD%\%EXE%"
	echo.
	pause
	exit /b 1
)

rem A save file is created on first launch, so its absence is normal on a fresh
rem copy of the game directory. Said out loud so that it cannot be mistaken for
rem a save that has gone missing.
if not exist "els_db.sql" (
	echo No els_db.sql here yet - a new save is created during login, and
	echo offline_server.log will say so. Expect an empty character select.
	echo.
)

start "" "%EXE%" %TOKEN%
