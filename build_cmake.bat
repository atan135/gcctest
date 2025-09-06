@echo off
REM ========================================
REM CMake Build Script for Windows
REM ========================================

echo Building with CMake...
echo.

REM Check if cmake is available
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: CMake not found!
    echo Please install CMake and add it to your PATH
    echo Download from: https://cmake.org/download/
    pause
    exit /b 1
)

echo CMake found:
cmake --version | findstr "cmake"
echo.

REM Create build directory if it doesn't exist
if not exist "build" (
    mkdir build
    echo Created build directory
)

REM Change to build directory
cd build

REM Configure the project
echo Configuring project...
cmake .. -G "MinGW Makefiles"

if %errorlevel% neq 0 (
    echo.
    echo ERROR: CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

REM Build the project
echo.
echo Building project...
cmake --build .

if %errorlevel% neq 0 (
    echo.
    echo ERROR: Build failed!
    cd ..
    pause
    exit /b 1
)

echo.
echo ========================================
echo BUILD SUCCESSFUL!
echo ========================================
echo.
echo Executables created in build\ directory:
dir *.exe 2>nul
echo.
echo To run:
echo   build\NetworkServer.exe
echo   build\TestClient.exe
if exist TestClientWindows.exe (
    echo   build\TestClientWindows.exe
)
echo.

cd ..
pause
