# High-Performance TCP Server with Memory Optimization

A high-performance C++ TCP server that uses epoll for I/O multiplexing, thread pool for handling multiple persistent connections, and advanced memory management to prevent fragmentation.

## Features

- **epoll-based I/O multiplexing**: Efficient handling of thousands of concurrent connections
- **Thread pool**: Multi-threaded message processing
- **Non-blocking I/O**: Edge-triggered epoll for optimal performance
- **Memory pool system**: Pre-allocated buffers to prevent memory fragmentation
- **Atomic memory tracking**: Real-time monitoring of memory usage
- **Persistent TCP connections**: Long-lived connections with message framing
- **Message-based protocol**: Simple newline-delimited message format
- **Signal handling**: Graceful shutdown on SIGINT/SIGTERM
- **Connection management**: Automatic cleanup of disconnected clients
- **Broadcast messaging**: Send messages to all connected clients
- **Activity tracking**: Monitor connection activity and cleanup inactive connections
- **Memory optimization**: Zero-copy message handling and buffer reuse

## Project Structure

```
gcctest/
├── include/
│   ├── NetworkServer.h      # Main server class
│   ├── ConnectionHandler.h  # Individual connection handling
│   ├── ThreadPool.h         # Thread pool implementation
│   ├── MessageBuffer.h      # Memory pool and buffer management
│   └── BufferConfig.h       # Memory configuration and tracking
├── src/
│   ├── main.cpp             # Application entry point
│   ├── NetworkServer.cpp    # Server implementation
│   ├── ConnectionHandler.cpp # Connection handling logic
│   ├── MessageBuffer.cpp    # Memory pool implementation
│   └── BufferConfig.cpp     # Memory tracking implementation
├── test/
│   ├── test_client.cpp      # Linux test client
│   ├── test_client_windows.cpp # Windows test client
│   ├── build_windows.bat   # Windows build script
│   └── README.md           # Test client documentation
├── examples/
│   └── memory_optimization_example.cpp # Memory optimization demo
├── docs/
│   └── MEMORY_OPTIMIZATION.md # Detailed memory optimization guide
├── settings.config         # Server configuration
├── Makefile               # Build configuration
├── build_test_client.bat  # Windows test client builder
└── README.md              # This file
```

## Requirements

- **GCC 7.0+** (C++17 support required)
- **CMake 3.16+**
- **Linux** (epoll is Linux-specific)
- **pthread** library

## Building

### Using CMake (Recommended)

**Linux/Unix:**
```bash
# Using build script (recommended)
./build_cmake.sh

# Or manually:
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```

**Windows:**
```bash
# Using build script (recommended)
build_cmake.bat

# Or manually:
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

**Executables created:**
- `build/NetworkServer` (or `NetworkServer.exe` on Windows)
- `build/TestClient` (Linux test client)
- `build/TestClientWindows.exe` (Windows test client, if on Windows)

### Using Make (Linux/Unix)

```bash
make all          # Build both server and test client
make clean        # Clean build files
make run          # Build and run server
make test         # Build and run test client
make test-windows # Build Windows test client (requires MinGW)
make debug        # Build with debug symbols
make help         # Show available targets
```

### Manual Compilation

```bash
g++ -std=c++17 -O2 -Wall -Wextra \
    -I include \
    src/*.cpp \
    -o NetworkServer \
    -lpthread
```

## Running

### Basic Usage

```bash
# Run with default settings (port 8080, 1000 max connections, 4 threads)
./NetworkServer

# Run with custom port
./NetworkServer 9000

# Run with custom port and max connections
./NetworkServer 9000 2000

# Run with custom port, max connections, and thread count
./NetworkServer 9000 2000 8
```

### Command Line Arguments

1. **Port** (default: 8080): Server listening port
2. **Max Connections** (default: 1000): Maximum number of concurrent connections
3. **Thread Count** (default: 4): Number of worker threads in the pool

### Testing the Server

You can test the server using various methods:

#### Using the included Test Client

**Linux/Unix:**
```bash
make test          # Build and run Linux test client
```

**Windows:**
```bash
# Option 1: From project root
build_test_client.bat

# Option 2: From test directory  
cd test
build_windows.bat

# Then run
test\TestClient.exe
```

#### Using telnet
```bash
telnet localhost 8080
# Type any message and press Enter
# Messages should end with newline character
```

#### Using netcat
```bash
echo "Hello Server" | nc localhost 8080
```

#### Custom TCP client
Connect to the server and send newline-delimited messages:
```
Client: "Hello World\n"
Server: "Server received: Hello World\n"
```

## Architecture

### epoll Event Loop
- Main thread runs the epoll event loop
- Handles new connections and I/O events
- Uses edge-triggered mode for optimal performance

### Thread Pool
- Worker threads process client requests
- Prevents blocking the main event loop
- Configurable thread count based on CPU cores

### Memory Management System
- **MessageBufferPool**: Pre-allocates and reuses memory buffers
- **MessageBuffer**: Fixed-size memory blocks with offset tracking
- **MemoryTracker**: Real-time memory usage monitoring
- **Zero-copy operations**: Direct memory operations without string allocations

### Connection Handling
- Each connection is managed by a `ConnectionHandler`
- Memory-efficient message queuing using pre-allocated buffers
- Automatic cleanup on disconnection
- Message-based protocol with newline delimiters
- Persistent connections with activity tracking
- Thread-safe message queuing for sending

## Performance Considerations

- **epoll**: Can handle 10,000+ concurrent connections efficiently
- **Thread Pool**: Prevents thread creation overhead
- **Non-blocking I/O**: No blocking operations in the main loop
- **Edge-triggered**: Minimizes system calls
- **Memory Pools**: Eliminates memory fragmentation and allocation overhead
- **Zero-copy Operations**: Direct memory operations without string allocations
- **Atomic Operations**: Lock-free memory tracking and statistics
- **Buffer Reuse**: Pre-allocated buffers reduce garbage collection pressure

## Signal Handling

The server responds to:
- **SIGINT** (Ctrl+C): Graceful shutdown
- **SIGTERM**: Graceful shutdown

## Memory Optimization

### Memory Pool System
The server uses a sophisticated memory management system to prevent fragmentation:

- **Pre-allocated Buffers**: Fixed-size memory blocks (256B, 1KB, 4KB, 16KB)
- **Buffer Reuse**: Pools of reusable buffers eliminate allocation overhead
- **Zero-copy Operations**: Direct memory operations without string allocations
- **Memory Tracking**: Real-time monitoring of memory usage and limits

### Configuration
```cpp
// Buffer sizes for different message types
SMALL_MESSAGE_SIZE = 256;    // Chat, commands
MEDIUM_MESSAGE_SIZE = 1024;  // Game state updates  
LARGE_MESSAGE_SIZE = 4096;   // Large data transfers
MAX_MESSAGE_SIZE = 16384;    // Maximum allowed message

// Memory limits
MAX_TOTAL_MEMORY_MB = 100;   // 100MB memory limit
```

### Memory Monitoring
```cpp
// Get current memory usage
auto& tracker = MemoryTracker::getInstance();
std::cout << "Current memory: " << (tracker.getCurrentUsage() / 1024) << " KB" << std::endl;
std::cout << "Peak memory: " << (tracker.getPeakUsage() / 1024) << " KB" << std::endl;
```

## Error Handling

- Comprehensive error checking for system calls
- Graceful handling of client disconnections
- Proper resource cleanup
- Memory limit enforcement and monitoring

## Example Output

```
Starting Network Server...
Port: 8080
Max connections: 1000
Thread count: 4
Configuration loaded from settings.config
Edit settings.config to modify server parameters
Press Ctrl+C to stop the server
----------------------------------------
Server started on port 8080
Max connections: 1000
Thread pool size: 4
Buffer sizes - Small: 256, Medium: 1024, Large: 4096
TCP Server is running. Clients can send messages ending with '\n'
Use TestClient to connect and send messages
New connection from 127.0.0.1:54321
Received from 127.0.0.1:54321: Hello World
Memory Stats - Current: 1024 KB, Peak: 1024 KB, Messages sent: 1
Cleaning up connection: 127.0.0.1:54321
```

## Troubleshooting

### Common Issues

1. **Permission denied**: Make sure the port is not in use by another process
2. **Address already in use**: Change the port number
3. **Too many open files**: Increase system limits with `ulimit -n`

### System Limits

```bash
# Check current limits
ulimit -n

# Increase file descriptor limit
ulimit -n 65536
```

## Examples and Documentation

### Memory Optimization Example
See `examples/memory_optimization_example.cpp` for a complete example showing:
- Memory-efficient message handling
- Real-time memory monitoring
- Performance benchmarking
- Memory usage tracking

### Detailed Documentation
- `docs/MEMORY_OPTIMIZATION.md`: Comprehensive guide to memory optimization techniques
- Memory fragmentation prevention strategies
- Performance optimization best practices
- Configuration and tuning guidelines

### Building Examples
```bash
# Build with examples
mkdir build && cd build
cmake .. -DBUILD_EXAMPLES=ON
make

# Run memory optimization example
./examples/memory_optimization_example
```

## Advanced Features

### Memory Pool Configuration
```cpp
// Custom buffer pool sizes
MessageBufferPool small_pool(BufferConfig::SMALL_MESSAGE_SIZE);
MessageBufferPool medium_pool(BufferConfig::MEDIUM_MESSAGE_SIZE);
MessageBufferPool large_pool(BufferConfig::LARGE_MESSAGE_SIZE);
```

### Memory Monitoring
```cpp
// Track memory usage in your application
auto& tracker = MemoryTracker::getInstance();
if (tracker.isMemoryLimitExceeded()) {
    // Handle memory limit exceeded
}
```

### Custom Message Handling
```cpp
// Use memory-efficient message handling
void handleMessage(const std::string& message, ConnectionHandler* handler) {
    MessageBuffer response(BufferConfig::MEDIUM_MESSAGE_SIZE);
    response.append("Response: ");
    response.append(message);
    handler->sendMessage(response);
}
```

## License

This project is provided as an educational example for learning about high-performance network programming in C++.
