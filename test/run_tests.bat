@echo off
REM ===============================================
REM Comprehensive Test Runner
REM ===============================================
REM This batch file runs various test scenarios
REM with configurable server settings
REM ===============================================

setlocal enabledelayedexpansion

REM Load configuration
call test_config.bat

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
set TEST_MODE=all
set QUIET_FLAG=

:parse_args
if "%~1"=="" goto :start_tests
if "%~1"=="--help" goto :show_help
if "%~1"=="-h" goto :show_help
if "%~1"=="--server" (
    set SERVER_IP=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="--port" (
    set PORT=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="--quiet" (
    set QUIET_FLAG=--quiet
    shift
    goto :parse_args
)
if "%~1"=="--mode" (
    set TEST_MODE=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="--single" (
    set TEST_MODE=single
    shift
    goto :parse_args
)
if "%~1"=="--multiple" (
    set TEST_MODE=multiple
    shift
    goto :parse_args
)
if "%~1"=="--concurrent" (
    set TEST_MODE=concurrent
    shift
    goto :parse_args
)
if "%~1"=="--all" (
    set TEST_MODE=all
    shift
    goto :parse_args
)

REM Unknown argument
echo ERROR: Unknown argument: %~1
echo.
goto :show_help

:show_help
echo.
echo Comprehensive Test Runner - Usage:
echo.
echo   run_tests.bat [options]
echo.
echo Options:
echo   --help, -h           Show this help message
echo   --server IP          Server IP address (default: %DEFAULT_SERVER_IP%)
echo   --port PORT          Server port (default: %DEFAULT_PORT%)
echo   --quiet              Run in quiet mode
echo   --mode MODE          Test mode: single, multiple, concurrent, all (default: all)
echo   --single             Run only single connection test
echo   --multiple           Run only multiple connections test
echo   --concurrent         Run only concurrent connections test
echo   --all                Run all tests (default)
echo.
echo Examples:
echo   run_tests.bat
echo   run_tests.bat --server 192.168.1.100 --port 9000
echo   run_tests.bat --mode single --quiet
echo   run_tests.bat --server localhost --port 8080 --mode concurrent
echo.
pause
exit /b 0

:start_tests
REM Display configuration
echo ===============================================
echo Comprehensive Test Runner
echo ===============================================
echo Server IP: %SERVER_IP%
echo Port: %PORT%
echo Test Mode: %TEST_MODE%
echo Quiet Mode: %QUIET_FLAG%
echo ===============================================
echo.

REM Check server connectivity
if "%ENABLE_CONNECTIVITY_CHECK%"=="true" (
    echo Testing server connectivity...
    ping -n 1 -w 1000 %SERVER_IP% >nul 2>&1
    if !errorlevel!==0 (
        echo Server %SERVER_IP% is reachable.
    ) else (
        echo WARNING: Server %SERVER_IP% may not be reachable.
        echo Please ensure the server is running and accessible.
    )
    echo.
)

REM Run tests based on mode
if "%TEST_MODE%"=="single" goto :run_single
if "%TEST_MODE%"=="multiple" goto :run_multiple
if "%TEST_MODE%"=="concurrent" goto :run_concurrent
if "%TEST_MODE%"=="all" goto :run_all
goto :invalid_mode

:run_single
echo Running single connection test...
TestClient.exe %SERVER_IP% %PORT% %QUIET_FLAG%
goto :end_tests

:run_multiple
echo Running multiple connections test...
TestClient.exe %SERVER_IP% %PORT% %QUIET_FLAG%
goto :end_tests

:run_concurrent
echo Running concurrent connections test...
TestClient.exe %SERVER_IP% %PORT% %QUIET_FLAG%
goto :end_tests

:run_all
echo Running all tests...
TestClient.exe %SERVER_IP% %PORT% %QUIET_FLAG%
goto :end_tests

:invalid_mode
echo ERROR: Invalid test mode: %TEST_MODE%
echo Valid modes: single, multiple, concurrent, all
goto :show_help

:end_tests
REM Check exit code
if %errorlevel%==0 (
    echo.
    echo ===============================================
    echo All tests completed successfully!
    echo ===============================================
) else (
    echo.
    echo ===============================================
    echo Tests failed with error code: %errorlevel%
    echo ===============================================
)

echo.
echo Press any key to exit...
pause >nul
exit /b %errorlevel%
