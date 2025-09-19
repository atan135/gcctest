@echo off
REM ===============================================
REM Test Client Runner for Windows
REM ===============================================
REM This batch file runs the TestClient.exe with configurable server settings
REM
REM Usage:
REM   run_test_client.bat [server_ip] [port] [options]
REM
REM Examples:
REM   run_test_client.bat 127.0.0.1 8080
REM   run_test_client.bat 192.168.1.100 9000
REM   run_test_client.bat localhost 8080 --quiet
REM   run_test_client.bat --help
REM   run_test_client.bat 121.40.159.168 8080
REM ===============================================

setlocal enabledelayedexpansion

REM Default values
set DEFAULT_SERVER_IP=127.0.0.1
set DEFAULT_PORT=8080
set DEFAULT_MODE=interactive

REM Check if TestClient.exe exists
if not exist "TestClient.exe" (
    echo ERROR: TestClient.exe not found!
    echo Please build the test client first by running:
    echo   build_windows.bat
    echo   or
    echo   build_windows_advanced.bat
    echo.
    pause
    exit /b 1
)

REM Parse command line arguments
set SERVER_IP=%DEFAULT_SERVER_IP%
set PORT=%DEFAULT_PORT%
set MODE=%DEFAULT_MODE%
set QUIET_FLAG=

:parse_args
if "%~1"=="" goto :run_client
if "%~1"=="--help" goto :show_help
if "%~1"=="-h" goto :show_help
if "%~1"=="--quiet" (
    set QUIET_FLAG=--quiet
    set MODE=quiet
    shift
    goto :parse_args
)
if "%~1"=="-q" (
    set QUIET_FLAG=--quiet
    set MODE=quiet
    shift
    goto :parse_args
)
if "%~1"=="--interactive" (
    set QUIET_FLAG=
    set MODE=interactive
    shift
    goto :parse_args
)
if "%~1"=="-i" (
    set QUIET_FLAG=
    set MODE=interactive
    shift
    goto :parse_args
)

REM First argument is server IP
if "%SERVER_IP%"=="%DEFAULT_SERVER_IP%" (
    set SERVER_IP=%~1
    shift
    goto :parse_args
)

REM Second argument is port
if "%PORT%"=="%DEFAULT_PORT%" (
    set PORT=%~1
    shift
    goto :parse_args
)

REM Unknown argument
echo ERROR: Unknown argument: %~1
echo.
goto :show_help

:show_help
echo.
echo Test Client Runner - Usage:
echo.
echo   run_test_client.bat [server_ip] [port] [options]
echo.
echo Arguments:
echo   server_ip    IP address of the server (default: %DEFAULT_SERVER_IP%)
echo   port         Port number of the server (default: %DEFAULT_PORT%)
echo.
echo Options:
echo   --help, -h           Show this help message
echo   --quiet, -q          Run in quiet mode (no interactive prompts)
echo   --interactive, -i    Run in interactive mode (default)
echo.
echo Examples:
echo   run_test_client.bat
echo   run_test_client.bat 192.168.1.100 9000
echo   run_test_client.bat localhost 8080 --quiet
echo   run_test_client.bat 127.0.0.1 8080 --interactive
echo.
echo Default behavior:
echo   - Server IP: %DEFAULT_SERVER_IP%
echo   - Port: %DEFAULT_PORT%
echo   - Mode: %DEFAULT_MODE%
echo.
pause
exit /b 0

:run_client
REM Display configuration
echo ===============================================
echo Test Client Runner
echo ===============================================
echo Server IP: %SERVER_IP%
echo Port: %PORT%
echo Mode: %MODE%
echo ===============================================
echo.

REM Check if server is reachable (optional ping test)
echo Testing server connectivity...
ping -n 1 -w 1000 %SERVER_IP% >nul 2>&1
if %errorlevel%==0 (
    echo Server %SERVER_IP% is reachable.
) else (
    echo WARNING: Server %SERVER_IP% may not be reachable.
    echo Please ensure the server is running and accessible.
)
echo.

REM Run the test client
echo Starting TestClient.exe...
echo Command: TestClient.exe %SERVER_IP% %PORT% %QUIET_FLAG%
echo.

TestClient.exe %SERVER_IP% %PORT% %QUIET_FLAG%

REM Check exit code
if %errorlevel%==0 (
    echo.
    echo ===============================================
    echo Test completed successfully!
    echo ===============================================
) else (
    echo.
    echo ===============================================
    echo Test failed with error code: %errorlevel%
    echo ===============================================
)

echo.
echo Press any key to exit...
pause >nul
exit /b %errorlevel%
