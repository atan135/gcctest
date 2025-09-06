@echo off
REM ========================================
REM Windows Test Client Build Script
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

REM Check if test directory exists
if not exist "test" (
    echo ERROR: test directory not found!
    echo Please make sure you're running this from the project root directory.
    pause
    exit /b 1
)

REM Check if source file exists
if not exist "test\test_client_windows.cpp" (
    echo ERROR: test\test_client_windows.cpp not found!
    pause
    exit /b 1
)

REM Clean previous build
echo Cleaning previous build...
if exist "test\TestClient.exe" (
    del "test\TestClient.exe"
    echo Removed old TestClient.exe
)
if exist "test\test_client_windows.o" (
    del "test\test_client_windows.o"
    echo Removed old object file
)
echo.

REM Change to test directory and compile
echo Compiling test client...
echo Command: cd test ^&^& g++ -std=c++17 -Wall -Wextra -O2 -o TestClient.exe test_client_windows.cpp -lws2_32
echo.

cd test
g++ -std=c++17 -Wall -Wextra -O2 -o TestClient.exe test_client_windows.cpp -lws2_32
set compile_result=%errorlevel%
cd ..

REM Check if compilation was successful
if %compile_result% neq 0 (
    echo.
    echo ERROR: Compilation failed!
    echo Please check the error messages above.
    pause
    exit /b 1
)

REM Check if executable was created
if not exist "test\TestClient.exe" (
    echo.
    echo ERROR: test\TestClient.exe was not created!
    pause
    exit /b 1
)

echo.
echo ========================================
echo BUILD SUCCESSFUL!
echo ========================================
echo.
echo TestClient.exe has been created in test\ directory.
echo File size: 
dir "test\TestClient.exe" | findstr "TestClient.exe"
echo.
echo To run the test client:
echo   cd test
echo   .\TestClient.exe
echo.
echo Or from root directory:
echo   test\TestClient.exe
echo.
echo To edit server IP/port, modify test\test_client_windows.cpp:
echo   - Line ~258: std::string server_ip = "YOUR_IP_HERE";
echo   - Line ~259: int port = YOUR_PORT_HERE;
echo.
echo Then run this build script again to recompile.
echo.
pause
