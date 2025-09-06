@echo off
REM ========================================
REM Advanced Windows Build Script for TCP Test Client
REM ========================================

setlocal enabledelayedexpansion

echo ========================================
echo TCP Test Client - Windows Build Script
echo ========================================
echo.

REM Parse command line arguments
set "BUILD_TYPE=release"
set "CLEAN_FIRST=no"
set "RUN_AFTER=no"

:parse_args
if "%~1"=="" goto :done_parsing
if /i "%~1"=="debug" set "BUILD_TYPE=debug"
if /i "%~1"=="release" set "BUILD_TYPE=release"
if /i "%~1"=="clean" set "CLEAN_FIRST=yes"
if /i "%~1"=="run" set "RUN_AFTER=yes"
if /i "%~1"=="help" goto :show_help
shift
goto :parse_args

:done_parsing

REM Show build configuration
echo Build Configuration:
echo - Build Type: %BUILD_TYPE%
echo - Clean First: %CLEAN_FIRST%
echo - Run After Build: %RUN_AFTER%
echo.

REM Check if g++ is available
where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: g++ compiler not found!
    echo Please install MinGW-w64 or MSYS2 and add it to your PATH
    echo Download from: https://www.mingw-w64.org/
    pause
    exit /b 1
)

echo Compiler Information:
g++ --version | findstr "g++"
echo.

REM Clean if requested
if /i "%CLEAN_FIRST%"=="yes" (
    echo Cleaning build files...
    if exist TestClient.exe del TestClient.exe
    if exist TestClient_debug.exe del TestClient_debug.exe
    if exist *.o del *.o
    echo Clean completed.
    echo.
)

REM Set compiler flags based on build type
if /i "%BUILD_TYPE%"=="debug" (
    set "CXXFLAGS=-std=c++17 -Wall -Wextra -g -DDEBUG"
    set "OUTPUT=TestClient_debug.exe"
    echo Building DEBUG version...
) else (
    set "CXXFLAGS=-std=c++17 -Wall -Wextra -O2 -DNDEBUG"
    set "OUTPUT=TestClient.exe"
    echo Building RELEASE version...
)

REM Show compile command
echo.
echo Compile Command:
echo g++ !CXXFLAGS! -o !OUTPUT! test_client_windows.cpp -lws2_32
echo.

REM Compile
g++ !CXXFLAGS! -o !OUTPUT! test_client_windows.cpp -lws2_32

REM Check compilation result
if %errorlevel% neq 0 (
    echo.
    echo ========================================
    echo BUILD FAILED!
    echo ========================================
    echo Please check the error messages above.
    pause
    exit /b 1
)

if not exist !OUTPUT! (
    echo.
    echo ERROR: !OUTPUT! was not created!
    pause
    exit /b 1
)

echo.
echo ========================================
echo BUILD SUCCESSFUL!
echo ========================================
echo.
echo Created: !OUTPUT!
echo File Info:
dir !OUTPUT! | findstr "!OUTPUT!"
echo.

REM Show usage instructions
echo Usage Instructions:
echo   .\!OUTPUT!
echo.
echo Configuration:
echo   Edit test_client_windows.cpp lines ~258-259:
echo   - std::string server_ip = "YOUR_LINUX_SERVER_IP";
echo   - int port = YOUR_PORT_NUMBER;
echo.

REM Run if requested
if /i "%RUN_AFTER%"=="yes" (
    echo Running test client...
    echo.
    !OUTPUT!
)

goto :end

:show_help
echo.
echo Usage: build_windows_advanced.bat [options]
echo.
echo Options:
echo   debug     - Build debug version with symbols
echo   release   - Build optimized release version (default)
echo   clean     - Clean previous build files first
echo   run       - Run the test client after successful build
echo   help      - Show this help message
echo.
echo Examples:
echo   build_windows_advanced.bat
echo   build_windows_advanced.bat debug
echo   build_windows_advanced.bat release clean
echo   build_windows_advanced.bat debug clean run
echo.
pause
exit /b 0

:end
echo Build script completed.
pause
