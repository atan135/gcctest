# API Reference

## Overview

This document provides detailed API reference for the high-performance TCP server with memory optimization features.

## Core Classes

### NetworkServer

Main server class that manages connections and handles I/O events.

#### Constructor
```cpp
NetworkServer(int port, int max_connections = 1000, int thread_count = 4);
```

#### Methods
```cpp
// Server lifecycle
bool start();                    // Start the server
void stop();                     // Stop the server
void run();                      // Run the main event loop

// Message handling
void setMessageHandler(std::function<void(const std::string&, ConnectionHandler*)> handler);
void broadcastMessage(const std::string& message);
void sendToClient(int client_fd, const std::string& message);

// Connection management
size_t getConnectionCount() const;
void cleanupInactiveConnections(int timeout_seconds = 300);
```

### ConnectionHandler

Handles individual client connections with memory-efficient message processing.

#### Constructor
```cpp
ConnectionHandler(int client_fd, const std::string& client_ip, int client_port);
```

#### Methods
```cpp
// Main handling methods
void handleRead();               // Handle incoming data
void handleWrite();              // Handle outgoing data
void processMessages();          // Process complete messages

// Message handling (memory-optimized)
void sendMessage(const std::string& message);
void sendMessage(const char* data, size_t length);
void sendMessage(const MessageBuffer& buffer);
bool hasMessagesToSend() const;

// Connection management
bool isConnected() const;
void close();
void setDisconnected();

// Getters
int getClientFd() const;
std::string getClientInfo() const;
std::chrono::steady_clock::time_point getLastActivity() const;

// Callback
std::function<void(const std::string&, ConnectionHandler*)> onMessageReceived;
```

## Memory Management Classes

### MessageBufferPool

Pre-allocates and manages a pool of message buffers to prevent memory fragmentation.

#### Constructor
```cpp
MessageBufferPool(size_t buffer_size = BufferConfig::MEDIUM_MESSAGE_SIZE);
```

#### Methods
```cpp
std::unique_ptr<MessageBuffer> acquire();     // Get buffer from pool
void release(std::unique_ptr<MessageBuffer> buffer);  // Return buffer to pool
size_t getPoolSize() const;                   // Get current pool size
size_t getActiveBuffers() const;              // Get number of active buffers
```

### MessageBuffer

Fixed-size memory buffer with efficient operations and offset tracking.

#### Constructor
```cpp
MessageBuffer(size_t capacity);
```

#### Methods
```cpp
// Buffer operations
bool append(const char* data, size_t length);
bool append(const std::string& data);
bool append(const MessageBuffer& other);

// Send operations
ssize_t sendPartial(int socket_fd, size_t offset = 0);
bool isComplete() const;
bool isEmpty() const;

// Getters
const char* data() const;
size_t size() const;
size_t capacity() const;
size_t remaining() const;
size_t getOffset() const;

// Buffer management
void reset();
std::unique_ptr<MessageBuffer> splitAt(size_t position);
```

### MessageQueue

Memory-efficient queue using pre-allocated buffers.

#### Constructor
```cpp
MessageQueue();
```

#### Methods
```cpp
// Queue operations
bool enqueue(const char* data, size_t length);
bool enqueue(const std::string& message);
MessageBuffer* front();
void pop();
bool empty() const;
size_t size() const;
void clear();
```

## Configuration Classes

### BufferConfig

Configuration constants for memory management.

#### Constants
```cpp
// Buffer sizes
static constexpr size_t SMALL_MESSAGE_SIZE = 256;    // Chat, commands
static constexpr size_t MEDIUM_MESSAGE_SIZE = 1024;  // Game state updates
static constexpr size_t LARGE_MESSAGE_SIZE = 4096;   // Large data transfers
static constexpr size_t MAX_MESSAGE_SIZE = 16384;    // Maximum allowed message

// Pool sizes
static constexpr size_t SMALL_POOL_SIZE = 100;
static constexpr size_t MEDIUM_POOL_SIZE = 50;
static constexpr size_t LARGE_POOL_SIZE = 20;

// Memory limits
static constexpr size_t MAX_TOTAL_MEMORY_MB = 100;  // 100MB limit
static constexpr size_t CLEANUP_INTERVAL_SECONDS = 30;
```

### MemoryTracker

Singleton class for tracking memory usage across the application.

#### Methods
```cpp
static MemoryTracker& getInstance();

void allocate(size_t bytes);                    // Track memory allocation
void deallocate(size_t bytes);                  // Track memory deallocation
size_t getCurrentUsage() const;                 // Get current memory usage
size_t getPeakUsage() const;                    // Get peak memory usage
bool isMemoryLimitExceeded() const;             // Check if memory limit exceeded
void reset();                                   // Reset memory counters
```

## Thread Pool

### ThreadPool

Manages worker threads for processing client requests.

#### Constructor
```cpp
ThreadPool(size_t num_threads);
```

#### Methods
```cpp
template<typename F, typename... Args>
auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

void stop();
size_t size() const;
```

## Usage Examples

### Basic Server Setup

```cpp
#include "NetworkServer.h"

int main() {
    NetworkServer server(8080, 1000, 4);
    
    server.setMessageHandler([](const std::string& message, ConnectionHandler* handler) {
        std::cout << "Received: " << message << std::endl;
        handler->sendMessage("Echo: " + message);
    });
    
    if (server.start()) {
        server.run();
    }
    
    return 0;
}
```

### Memory-Efficient Message Handling

```cpp
#include "MessageBuffer.h"
#include "BufferConfig.h"

void handleMessage(const std::string& message, ConnectionHandler* handler) {
    // Use pre-allocated buffer
    MessageBuffer response(BufferConfig::MEDIUM_MESSAGE_SIZE);
    response.append("Response: ");
    response.append(message);
    
    // Send using memory-efficient method
    handler->sendMessage(response);
}
```

### Memory Monitoring

```cpp
#include "BufferConfig.h"

void monitorMemory() {
    auto& tracker = MemoryTracker::getInstance();
    
    std::cout << "Current memory: " << (tracker.getCurrentUsage() / 1024) << " KB" << std::endl;
    std::cout << "Peak memory: " << (tracker.getPeakUsage() / 1024) << " KB" << std::endl;
    
    if (tracker.isMemoryLimitExceeded()) {
        std::cout << "WARNING: Memory limit exceeded!" << std::endl;
    }
}
```

### Custom Buffer Pool

```cpp
#include "MessageBuffer.h"

// Create custom buffer pools
MessageBufferPool small_pool(BufferConfig::SMALL_MESSAGE_SIZE);
MessageBufferPool large_pool(BufferConfig::LARGE_MESSAGE_SIZE);

// Use pools for different message types
auto small_buffer = small_pool.acquire();
small_buffer->append("Short message");

auto large_buffer = large_pool.acquire();
large_buffer->append("Large data payload...");

// Return buffers to pool
small_pool.release(std::move(small_buffer));
large_pool.release(std::move(large_buffer));
```

## Performance Considerations

### Memory Optimization
- Use appropriate buffer sizes for different message types
- Reuse buffers through the pool system
- Monitor memory usage to prevent leaks
- Use zero-copy operations when possible

### Thread Safety
- All public methods are thread-safe
- Atomic operations used for counters and flags
- Mutex protection for shared data structures
- Lock-free operations where possible

### Error Handling
- All methods return appropriate error codes
- Exceptions thrown for critical errors
- Graceful degradation on resource exhaustion
- Comprehensive logging for debugging

## Compilation Flags

### Recommended Flags
```bash
g++ -std=c++17 -O2 -Wall -Wextra -DNDEBUG
```

### Debug Flags
```bash
g++ -std=c++17 -g -Wall -Wextra -DDEBUG -fsanitize=address
```

### Memory Debugging
```bash
g++ -std=c++17 -g -Wall -Wextra -DDEBUG -fsanitize=memory
```

## Dependencies

### Required
- C++17 compiler (GCC 7.0+ or Clang 5.0+)
- pthread library
- CMake 3.16+

### Optional
- AddressSanitizer for memory debugging
- Valgrind for memory profiling
- perf for performance analysis
