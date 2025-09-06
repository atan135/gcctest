@echo off
REM ========================================
REM Windows Build Script for TCP Test Client
REM ========================================

echo Building TCP Test Client for Windows...
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

echo Compiler found: 
g++ --version | findstr "g++"
echo.

REM Clean previous build
echo Cleaning previous build...
if exist TestClient.exe (
    del TestClient.exe
    echo Removed old TestClient.exe
)
if exist test_client_windows.o (
    del test_client_windows.o
    echo Removed old object file
)
echo.

REM Compile the test client
echo Compiling test client...
echo Command: g++ -std=c++17 -Wall -Wextra -O2 -o TestClient.exe test_client_windows.cpp -lws2_32
echo.

g++ -std=c++17 -Wall -Wextra -O2 -o TestClient.exe test_client_windows.cpp -lws2_32

REM Check if compilation was successful
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Compilation failed!
    echo Please check the error messages above.
    pause
    exit /b 1
)

REM Check if executable was created
if not exist TestClient.exe (
    echo.
    echo ERROR: TestClient.exe was not created!
    pause
    exit /b 1
)

echo.
echo ========================================
echo BUILD SUCCESSFUL!
echo ========================================
echo.
echo TestClient.exe has been created successfully.
echo File size: 
dir TestClient.exe | findstr "TestClient.exe"
echo.
echo To run the test client:
echo   .\TestClient.exe
echo.
echo To edit server IP/port, modify test_client_windows.cpp:
echo   - Line ~258: std::string server_ip = "YOUR_IP_HERE";
echo   - Line ~259: int port = YOUR_PORT_HERE;
echo.
echo Then run this build script again to recompile.
echo.
pause
