@echo off
REM ===============================================
REM Test Client Configuration
REM ===============================================
REM This file contains default server settings for testing
REM Modify these values to match your server configuration
REM ===============================================

REM Server Configuration
set DEFAULT_SERVER_IP=127.0.0.1
set DEFAULT_PORT=8080

REM Test Configuration
set DEFAULT_MODE=interactive
set DEFAULT_TIMEOUT=30

REM Advanced Settings
set ENABLE_PING_TEST=true
set ENABLE_CONNECTIVITY_CHECK=true
set VERBOSE_OUTPUT=true

REM ===============================================
REM Do not modify below this line
REM ===============================================

REM Export variables for use by other batch files
call :export_vars

goto :eof

:export_vars
REM This function exports the configuration variables
REM so they can be used by other batch files
set CONFIG_LOADED=true
goto :eof
