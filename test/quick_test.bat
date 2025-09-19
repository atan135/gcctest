@echo off
REM ===============================================
REM Quick Test Client Runner
REM ===============================================
REM Simple batch file for quick testing with common configurations
REM ===============================================

setlocal

REM Check if TestClient.exe exists
if not exist "TestClient.exe" (
    echo ERROR: TestClient.exe not found!
    echo Please build the test client first.
    pause
    exit /b 1
)

echo ===============================================
echo Quick Test Client Runner
echo ===============================================
echo.
echo Select test configuration:
echo.
echo 1. Local server (127.0.0.1:8080)
echo 2. Local server quiet mode (127.0.0.1:8080 --quiet)
echo 3. Custom server (enter IP and port)
echo 4. Show help
echo 5. Exit
echo.

set /p choice="Enter your choice (1-5): "

if "%choice%"=="1" goto :local_server
if "%choice%"=="2" goto :local_quiet
if "%choice%"=="3" goto :custom_server
if "%choice%"=="4" goto :show_help
if "%choice%"=="5" goto :exit
goto :invalid_choice

:local_server
echo.
echo Running test with local server (127.0.0.1:8080)...
echo.
TestClient.exe 127.0.0.1 8080
goto :end

:local_quiet
echo.
echo Running test with local server in quiet mode (127.0.0.1:8080 --quiet)...
echo.
TestClient.exe 127.0.0.1 8080 --quiet
goto :end

:custom_server
echo.
set /p custom_ip="Enter server IP (default: 127.0.0.1): "
if "%custom_ip%"=="" set custom_ip=127.0.0.1

set /p custom_port="Enter server port (default: 8080): "
if "%custom_port%"=="" set custom_port=8080

echo.
set /p quiet_mode="Run in quiet mode? (y/n, default: n): "
if /i "%quiet_mode%"=="y" (
    echo Running test with %custom_ip%:%custom_port% in quiet mode...
    echo.
    TestClient.exe %custom_ip% %custom_port% --quiet
) else (
    echo Running test with %custom_ip%:%custom_port%...
    echo.
    TestClient.exe %custom_ip% %custom_port%
)
goto :end

:show_help
echo.
TestClient.exe --help
echo.
pause
goto :start

:invalid_choice
echo.
echo Invalid choice. Please enter 1-5.
echo.
pause
goto :start

:end
echo.
echo Test completed. Press any key to exit...
pause >nul
exit /b %errorlevel%

:exit
echo.
echo Goodbye!
exit /b 0
