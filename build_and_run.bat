@echo off
REM Build and run TsunamiSimUI with Qt 6.11.0 MinGW
REM Usage: build_and_run.bat [debug|release]

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=release

set QT_DIR=D:\Qt\6.11.0\mingw_64
set MINGW_DIR=D:\Qt\Tools\mingw1310_64
set NINJA=D:\Qt\Tools\Ninja\ninja.exe
set CMAKE="D:\Program Files\CMake\bin\cmake.exe"
set BUILD_DIR=build-%BUILD_TYPE%

echo === TsunamiSimUI Build (%BUILD_TYPE%) ===
echo Qt:    %QT_DIR%
echo MinGW: %MINGW_DIR%

REM Add MinGW to PATH
set PATH=%MINGW_DIR%\bin;%QT_DIR%\bin;%PATH%

REM Configure
if not exist %BUILD_DIR% (
    echo Configuring...
    %CMAKE% --preset mingw-%BUILD_TYPE%
    if errorlevel 1 (
        echo Configuration failed!
        pause
        exit /b 1
    )
)

REM Build
echo Building...
%CMAKE% --build %BUILD_DIR%
if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

REM Deploy Qt DLLs if not yet done
if not exist "%BUILD_DIR%\Qt6Core.dll" (
    echo Deploying Qt DLLs...
    %QT_DIR%\bin\windeployqt.exe %BUILD_DIR%\TsunamiSimUI.exe
)

REM Run
echo === Running TsunamiSimUI ===
start "" %BUILD_DIR%\TsunamiSimUI.exe
