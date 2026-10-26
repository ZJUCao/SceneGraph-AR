@echo off
rem Build (if needed) and run SceneGraph AR on Windows.
rem Double-click this file, or run "run.bat [program] [source]" in a terminal:
rem   program  calibrate, ar or features (default: ar once a calibration exists, otherwise calibrate)
rem   source   camera index (default 0) or a video file
setlocal
rem Resolve a video file argument before changing directory; camera indices are kept as is
set "program=%~1"
set "source=%~2"
set "source_path=%~f2"
cd /d "%~dp0"
if defined source (
    echo %source%| findstr /r /x "[0-9][0-9]*" >nul || set "source=%source_path%"
)

for %%t in (cmake git) do (
    where %%t >nul 2>nul || (
        echo [run] %%t not found. See "Requirements" in README.md.
        goto fail
    )
)

rem CMake finds Visual Studio on its own; check here for a clearer message
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "VSPATH="
if exist "%VSWHERE%" (
    for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSPATH=%%i"
)
if not defined VSPATH (
    echo [run] Visual Studio with the "Desktop development with C++" workload not found.
    echo [run] See "Requirements" in README.md.
    goto fail
)

if "%program%"=="" (
    if exist "build\bin\data\intrinsics.yml" (set "program=ar") else (set "program=calibrate")
)
if /i "%program%"=="calibrate" (
    set "exe=Calibrate"
) else if /i "%program%"=="ar" (
    set "exe=AugmentedReality"
) else if /i "%program%"=="features" (
    set "exe=HarrisFeatures"
) else (
    echo [run] Unknown program "%program%". Usage: run.bat [calibrate^|ar^|features] [camera-index^|video-file]
    goto fail
)

if not exist "build\CMakeCache.txt" (
    cmake --preset msvc || goto fail
)
cmake --build --preset msvc -j %NUMBER_OF_PROCESSORS% || goto fail

if /i "%exe%"=="Calibrate" (
    echo [run] Show the 9x6 checkerboard from different angles: S saves a view, C calibrates after 5+ views, Q quits.
) else if /i "%exe%"=="AugmentedReality" (
    echo [run] Point the camera at the checkerboard: 1/2/3 toggle axes/object/scene, S saves a screenshot, Q quits.
)
if defined source (
    "build\bin\%exe%.exe" "%source%"
) else (
    "build\bin\%exe%.exe"
)
set "code=%errorlevel%"
if not "%code%"=="0" (
    echo [run] %exe% exited with code %code%.
    goto fail
)
exit /b 0

:fail
pause
exit /b 1
