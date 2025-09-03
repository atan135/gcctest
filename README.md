# Network Server with epoll and Multithreading

A high-performance C++ network server that uses epoll for I/O multiplexing and a thread pool for handling multiple concurrent connections.

## Features

- **epoll-based I/O multiplexing**: Efficient handling of thousands of concurrent connections
- **Thread pool**: Multi-threaded request processing
- **Non-blocking I/O**: Edge-triggered epoll for optimal performance
- **HTTP-like responses**: Simple request/response handling
- **Signal handling**: Graceful shutdown on SIGINT/SIGTERM
- **Connection management**: Automatic cleanup of disconnected clients

## Project Structure

```
gcctest/
├── include/
│   ├── NetworkServer.h      # Main server class
│   ├── ConnectionHandler.h  # Individual connection handling
│   └── ThreadPool.h         # Thread pool implementation
├── src/
│   ├── main.cpp             # Application entry point
│   ├── NetworkServer.cpp    # Server implementation
│   └── ConnectionHandler.cpp # Connection handling logic
├── CMakeLists.txt           # Build configuration
└── README.md               # This file
```

## Requirements

- **GCC 7.0+** (C++17 support required)
- **CMake 3.16+**
- **Linux** (epoll is Linux-specific)
- **pthread** library

## Building

### Using CMake (Recommended)

```bash
# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make

# The executable will be created as 'NetworkServer'
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

#### Using curl
```bash
curl http://localhost:8080
```

#### Using telnet
```bash
telnet localhost 8080
# Type any message and press Enter
```

#### Using netcat
```bash
echo "Hello Server" | nc localhost 8080
```

#### Load testing with Apache Bench
```bash
ab -n 1000 -c 100 http://localhost:8080/
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

### Connection Handling
- Each connection is managed by a `ConnectionHandler`
- Automatic cleanup on disconnection
- HTTP-like response format

## Performance Considerations

- **epoll**: Can handle 10,000+ concurrent connections efficiently
- **Thread Pool**: Prevents thread creation overhead
- **Non-blocking I/O**: No blocking operations in the main loop
- **Edge-triggered**: Minimizes system calls

## Signal Handling

The server responds to:
- **SIGINT** (Ctrl+C): Graceful shutdown
- **SIGTERM**: Graceful shutdown

## Error Handling

- Comprehensive error checking for system calls
- Graceful handling of client disconnections
- Proper resource cleanup

## Example Output

```
Starting Network Server...
Port: 8080
Max connections: 1000
Thread count: 4
Usage: ./NetworkServer [port] [max_connections] [thread_count]
Press Ctrl+C to stop the server
----------------------------------------
Server started on port 8080
Max connections: 1000
Thread pool size: 4
New connection from 127.0.0.1:54321
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

## License

This project is provided as an educational example for learning about high-performance network programming in C++.
