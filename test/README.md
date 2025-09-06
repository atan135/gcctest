# Test Client

This directory contains test clients for the TCP Network Server.

## Files

- `test_client.cpp` - Linux/Unix test client (uses POSIX sockets)
- `test_client_windows.cpp` - Windows test client (uses Winsock)
- `build_windows.bat` - Simple Windows build script (run from this directory)
- `build_windows_advanced.bat` - Advanced Windows build script with options
- `TestClient.exe` - Compiled Windows executable (if built)

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
2. **Configure the IP/port** in the test client source
3. **Build the test client**
4. **Run the test client**:
   ```bash
   # Windows
   TestClient.exe
   
   # Linux
   ./TestClient
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
