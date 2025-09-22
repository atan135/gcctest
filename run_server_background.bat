@echo off
REM ===============================================
REM NetworkServer Background Runner for Windows
REM ===============================================
REM This batch file runs the NetworkServer in background mode
REM
REM Usage:
REM   run_server_background.bat [options]
REM
REM Options:
REM   --help, -h           Show this help message
REM   --stop, -s           Stop running background server
REM   --status             Check if server is running
REM   --restart, -r        Restart the server
REM ===============================================

setlocal enabledelayedexpansion

REM Configuration
set SERVER_EXECUTABLE=NetworkServer.exe
set PID_FILE=networkserver.pid
set LOG_FILE=networkserver.log

REM Check if NetworkServer.exe exists
if not exist "%SERVER_EXECUTABLE%" (
    echo ERROR: %SERVER_EXECUTABLE% not found!
    echo Please build the server first by running:
    echo   build_cmake.bat
    echo   or
    echo   build.sh
    echo.
    pause
    exit /b 1
)

REM Function to check if server is running
:is_server_running
if exist "%PID_FILE%" (
    set /p SERVER_PID=<"%PID_FILE%"
    tasklist /FI "PID eq !SERVER_PID!" 2>nul | find "!SERVER_PID!" >nul
    if !errorlevel!==0 (
        exit /b 0
    ) else (
        del "%PID_FILE%" 2>nul
        exit /b 1
    )
) else (
    exit /b 1
)

REM Function to start server
:start_server
call :is_server_running
if !errorlevel!==0 (
    echo WARNING: Server is already running (PID: !SERVER_PID!)
    exit /b 1
)

echo Starting NetworkServer in background...

REM Start server in background
start /B "" "%SERVER_EXECUTABLE%" > "%LOG_FILE%" 2>&1

REM Get the PID of the started process
for /f "tokens=2" %%i in ('tasklist /FI "IMAGENAME eq %SERVER_EXECUTABLE%" /FO CSV ^| find "%SERVER_EXECUTABLE%"') do (
    set SERVER_PID=%%i
    set SERVER_PID=!SERVER_PID:"=!
)

REM Save PID to file
echo !SERVER_PID! > "%PID_FILE%"

REM Give server a moment to start
timeout /t 2 /nobreak >nul

REM Check if server started successfully
call :is_server_running
if !errorlevel!==0 (
    echo SUCCESS: Server started successfully!
    echo PID: !SERVER_PID!
    echo Log file: %LOG_FILE%
    echo To stop server: %0 --stop
    echo To view logs: type %LOG_FILE%
) else (
    echo ERROR: Failed to start server!
    echo Check log file: %LOG_FILE%
    del "%PID_FILE%" 2>nul
    exit /b 1
)
exit /b 0

REM Function to stop server
:stop_server
call :is_server_running
if !errorlevel!==1 (
    echo WARNING: Server is not running
    exit /b 1
)

echo Stopping server (PID: !SERVER_PID!)...

REM Try to stop the process
taskkill /PID !SERVER_PID! /F >nul 2>&1

REM Clean up
del "%PID_FILE%" 2>nul

REM Check if stopped
call :is_server_running
if !errorlevel!==1 (
    echo SUCCESS: Server stopped successfully
    exit /b 0
) else (
    echo ERROR: Failed to stop server!
    exit /b 1
)

REM Function to show server status
:show_status
call :is_server_running
if !errorlevel!==0 (
    echo SUCCESS: Server is running (PID: !SERVER_PID!)
    echo Log file: %LOG_FILE%
    echo To stop: %0 --stop
    echo To view logs: type %LOG_FILE%
) else (
    echo WARNING: Server is not running
)
exit /b 0

REM Function to restart server
:restart_server
echo Restarting server...
call :stop_server
timeout /t 1 /nobreak >nul
call :start_server
exit /b 0

REM Function to show help
:show_help
echo NetworkServer Background Runner for Windows
echo.
echo Usage: %0 [options]
echo.
echo Options:
echo   --help, -h           Show this help message
echo   --stop, -s           Stop running background server
echo   --status             Check if server is running
echo   --restart, -r        Restart the server
echo.
echo Examples:
echo   %0                   # Start server in background
echo   %0 --stop            # Stop server
echo   %0 --status          # Check server status
echo   %0 --restart         # Restart server
echo.
echo Files:
echo   %PID_FILE%           # Process ID file
echo   %LOG_FILE%           # Server log file
exit /b 0

REM Main script logic
if "%~1"=="" goto :start_server
if "%~1"=="--help" goto :show_help
if "%~1"=="-h" goto :show_help
if "%~1"=="--stop" goto :stop_server
if "%~1"=="-s" goto :stop_server
if "%~1"=="--status" goto :show_status
if "%~1"=="--restart" goto :restart_server
if "%~1"=="-r" goto :restart_server

echo ERROR: Unknown option: %~1
echo.
goto :show_help
