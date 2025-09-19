# Test Client

This directory contains test clients for the TCP Network Server.

## Files

- `test_client.cpp` - Linux/Unix test client (uses POSIX sockets)
- `test_client_windows.cpp` - Windows test client (uses Winsock)
- `build_windows.bat` - Simple Windows build script (run from this directory)
- `build_windows_advanced.bat` - Advanced Windows build script with options
- `TestClient.exe` - Compiled Windows executable (if built)

### Windows Batch Files for Easy Testing

- `run_test_client.bat` - Main test runner with full command line support
- `quick_test.bat` - Interactive menu for quick testing
- `run_tests.bat` - Comprehensive test runner with multiple test modes
- `test_config.bat` - Configuration file for default server settings

## Building

### Linux/Unix (from project root)
```bash
make test          # Build and run Linux test client
make test-windows  # Build Windows test client (requires MinGW)
```

### Windows

#### Option 1: From project root
```bash
build_test_client.bat
```

#### Option 2: From test directory
```bash
cd test
build_windows.bat
```

#### Option 3: Manual compilation
```bash
cd test
g++ -std=c++17 -Wall -Wextra -O2 -o TestClient.exe test_client_windows.cpp -lws2_32
```

## Configuration

Edit the server IP and port in the source files:

**For Windows (`test_client_windows.cpp`):**
```cpp
// Line ~258-259
std::string server_ip = "192.168.1.100";  // Your Linux server IP
int port = 8080;                          // Your server port
```

**For Linux (`test_client.cpp`):**
```cpp
// Line ~99
TestClient client("127.0.0.1", 8080);
```

## Usage

### Running the Test Client

1. **Start your server** (on Linux or wherever it's running)
2. **Build the test client**
3. **Run the test client** with command line arguments:

#### Windows Usage
```bash
# Basic usage (server IP and port required)
TestClient.exe 127.0.0.1 8080

# Connect to remote server
TestClient.exe 192.168.1.100 9000

# Run in quiet mode (no interactive prompts)
TestClient.exe 127.0.0.1 8080 --quiet

# Show help
TestClient.exe --help
```

#### Linux/Unix Usage
```bash
# Basic usage (server IP and port required)
./TestClient 127.0.0.1 8080

# Connect to remote server
./TestClient 192.168.1.100 9000

# Run in quiet mode (no interactive prompts)
./TestClient 127.0.0.1 8080 --quiet

# Show help
./TestClient --help
```

#### Command Line Arguments
- `<server_ip>`: IP address of the server (required)
- `<port>`: Port number of the server (required)
- `--help, -h`: Show help message
- `--quiet, -q`: Run tests without interactive prompts

### Windows Batch Files

#### Quick Test (Interactive Menu)
```bash
# Run interactive menu
quick_test.bat
```
This provides a simple menu to select common test configurations.

#### Main Test Runner
```bash
# Basic usage (uses defaults from test_config.bat)
run_test_client.bat

# Custom server and port
run_test_client.bat 192.168.1.100 9000

# Quiet mode
run_test_client.bat 127.0.0.1 8080 --quiet

# Show help
run_test_client.bat --help
```

#### Comprehensive Test Runner
```bash
# Run all tests
run_tests.bat

# Run specific test mode
run_tests.bat --mode single
run_tests.bat --mode multiple
run_tests.bat --mode concurrent

# Custom server settings
run_tests.bat --server 192.168.1.100 --port 9000

# Quiet mode with specific test
run_tests.bat --mode single --quiet
```

#### Configuration File
Edit `test_config.bat` to set default server settings:
```batch
REM Server Configuration
set DEFAULT_SERVER_IP=127.0.0.1
set DEFAULT_PORT=8080

REM Test Configuration
set DEFAULT_MODE=interactive
set ENABLE_PING_TEST=true
```

### Test Scenarios

The test client runs three test scenarios:

1. **Single Connection Test**
   - Connects once
   - Sends 3 messages
   - Receives responses
   - Disconnects

2. **Multiple Connections Test**
   - Creates 5 concurrent connections
   - Each sends 2 messages
   - Tests multi-client handling

3. **Concurrent Stress Test**
   - Creates 10 concurrent connections
   - Each sends 3 messages rapidly
   - Tests server under load

### Expected Output

```
=== Testing Single Connection ===
Connected to 192.168.1.100:8080
Sent: Hello message 1
Response: Server received: Hello message 1
...

=== Testing Multiple Connections ===
Connected to 192.168.1.100:8080
Client 0 received: Server received: Hello from client 0
...

=== Testing Concurrent Connections ===
Client 0 got: Server received: Request 0 from client 0
...

All TCP tests completed!
```

## Troubleshooting

### Connection Issues
- Ensure server is running and accessible
- Check firewall settings
- Verify IP address and port
- Test network connectivity: `ping <server_ip>`

### Compilation Issues
- **Windows**: Install MinGW-w64 or MSYS2
- **Linux**: Install build-essential: `sudo apt install build-essential`
- Ensure g++ supports C++17

### Runtime Issues
- Check server logs for connection attempts
- Verify message format (must end with `\n`)
- Check for port conflicts
