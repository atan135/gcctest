# Performance Guide

## Overview

This guide provides comprehensive performance optimization strategies and benchmarking techniques for the high-performance TCP server with memory optimization.

## Performance Metrics

### Key Performance Indicators (KPIs)

1. **Throughput**: Messages per second (MPS)
2. **Latency**: Average response time (microseconds)
3. **Memory Usage**: Peak and current memory consumption
4. **CPU Usage**: CPU utilization percentage
5. **Connection Capacity**: Maximum concurrent connections
6. **Memory Fragmentation**: Memory fragmentation percentage

### Benchmarking Tools

```cpp
// Performance measurement utilities
class PerformanceMonitor {
public:
    void startMeasurement(const std::string& name);
    void endMeasurement(const std::string& name);
    void printStatistics();
    
private:
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> start_times_;
    std::unordered_map<std::string, std::vector<double>> measurements_;
};
```

## Memory Optimization Strategies

### 1. Buffer Pool Sizing

#### Optimal Pool Sizes
```cpp
// Based on message patterns
constexpr size_t CHAT_POOL_SIZE = 200;      // High frequency, small messages
constexpr size_t GAME_STATE_POOL_SIZE = 50;  // Medium frequency, medium messages
constexpr size_t DATA_TRANSFER_POOL_SIZE = 20; // Low frequency, large messages
```

#### Dynamic Pool Sizing
```cpp
class AdaptivePoolManager {
public:
    void adjustPoolSizes() {
        // Monitor usage patterns and adjust pool sizes
        if (small_pool_usage_ > 0.8) {
            increasePoolSize(small_pool_);
        }
        if (large_pool_usage_ < 0.2) {
            decreasePoolSize(large_pool_);
        }
    }
};
```

### 2. Memory Allocation Patterns

#### Pre-allocation Strategy
```cpp
// Pre-allocate buffers for common operations
class ConnectionHandler {
private:
    std::unique_ptr<MessageBuffer> temp_buffer_;
    std::unique_ptr<MessageBuffer> response_buffer_;
    
public:
    ConnectionHandler() {
        temp_buffer_ = std::make_unique<MessageBuffer>(BufferConfig::MEDIUM_MESSAGE_SIZE);
        response_buffer_ = std::make_unique<MessageBuffer>(BufferConfig::MEDIUM_MESSAGE_SIZE);
    }
};
```

#### Memory Pool Hierarchy
```cpp
class HierarchicalPoolManager {
private:
    MessageBufferPool small_pool_{BufferConfig::SMALL_MESSAGE_SIZE};
    MessageBufferPool medium_pool_{BufferConfig::MEDIUM_MESSAGE_SIZE};
    MessageBufferPool large_pool_{BufferConfig::LARGE_MESSAGE_SIZE};
    
public:
    std::unique_ptr<MessageBuffer> acquireBuffer(size_t required_size) {
        if (required_size <= BufferConfig::SMALL_MESSAGE_SIZE) {
            return small_pool_.acquire();
        } else if (required_size <= BufferConfig::MEDIUM_MESSAGE_SIZE) {
            return medium_pool_.acquire();
        } else {
            return large_pool_.acquire();
        }
    }
};
```

### 3. Zero-Copy Operations

#### Direct Memory Operations
```cpp
// Avoid string operations in hot paths
void sendMessage(const char* data, size_t length) {
    temp_buffer_->reset();
    temp_buffer_->append(data, length);  // Direct memcpy
    temp_buffer_->append(MESSAGE_DELIMITER);
    send_queue_.enqueue(temp_buffer_->data(), temp_buffer_->size());
}
```

#### Buffer Reuse
```cpp
class BufferRecycler {
public:
    void recycleBuffer(std::unique_ptr<MessageBuffer> buffer) {
        buffer->reset();
        available_buffers_.push_back(std::move(buffer));
    }
    
private:
    std::vector<std::unique_ptr<MessageBuffer>> available_buffers_;
};
```

## Network Optimization

### 1. epoll Configuration

#### Optimal epoll Settings
```cpp
// Edge-triggered mode for maximum performance
event.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP;

// Optimal timeout for event loop
int num_events = epoll_wait(epoll_fd_, events, MAX_EVENTS, 1000); // 1 second timeout
```

#### Connection Limits
```cpp
// System-level optimizations
void optimizeSystemLimits() {
    // Increase file descriptor limit
    struct rlimit rlim;
    rlim.rlim_cur = 65536;
    rlim.rlim_max = 65536;
    setrlimit(RLIMIT_NOFILE, &rlim);
    
    // Optimize TCP settings
    int tcp_keepalive = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &tcp_keepalive, sizeof(tcp_keepalive));
}
```

### 2. Thread Pool Optimization

#### Optimal Thread Count
```cpp
// Calculate optimal thread count
size_t getOptimalThreadCount() {
    size_t cpu_cores = std::thread::hardware_concurrency();
    
    // For I/O bound workloads: 2 * CPU cores
    // For CPU bound workloads: CPU cores
    // For mixed workloads: 1.5 * CPU cores
    return static_cast<size_t>(cpu_cores * 1.5);
}
```

#### Work Stealing
```cpp
class WorkStealingThreadPool {
public:
    void enqueue(std::function<void()> task) {
        // Distribute work across threads
        size_t thread_id = getNextThreadId();
        threads_[thread_id].enqueue(std::move(task));
    }
    
private:
    std::atomic<size_t> next_thread_{0};
    std::vector<ThreadPool> threads_;
};
```

## Benchmarking Implementation

### 1. Throughput Testing

```cpp
class ThroughputBenchmark {
public:
    void runBenchmark(int duration_seconds) {
        auto start_time = std::chrono::steady_clock::now();
        auto end_time = start_time + std::chrono::seconds(duration_seconds);
        
        size_t messages_sent = 0;
        while (std::chrono::steady_clock::now() < end_time) {
            // Send test message
            sendTestMessage();
            messages_sent++;
        }
        
        auto actual_duration = std::chrono::steady_clock::now() - start_time;
        double throughput = messages_sent / std::chrono::duration<double>(actual_duration).count();
        
        std::cout << "Throughput: " << throughput << " messages/second" << std::endl;
    }
};
```

### 2. Latency Testing

```cpp
class LatencyBenchmark {
public:
    void measureLatency() {
        auto start = std::chrono::high_resolution_clock::now();
        
        // Perform operation
        performOperation();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        latency_measurements_.push_back(duration.count());
    }
    
    void printLatencyStatistics() {
        std::sort(latency_measurements_.begin(), latency_measurements_.end());
        
        double p50 = latency_measurements_[latency_measurements_.size() * 0.5];
        double p95 = latency_measurements_[latency_measurements_.size() * 0.95];
        double p99 = latency_measurements_[latency_measurements_.size() * 0.99];
        
        std::cout << "Latency (μs): P50=" << p50 << ", P95=" << p95 << ", P99=" << p99 << std::endl;
    }
};
```

### 3. Memory Usage Testing

```cpp
class MemoryBenchmark {
public:
    void runMemoryTest() {
        auto& tracker = MemoryTracker::getInstance();
        
        // Test memory allocation patterns
        for (int i = 0; i < 10000; ++i) {
            auto buffer = buffer_pool_.acquire();
            buffer->append("Test message " + std::to_string(i));
            send_queue_.enqueue(buffer->data(), buffer->size());
        }
        
        std::cout << "Peak memory: " << (tracker.getPeakUsage() / 1024) << " KB" << std::endl;
        std::cout << "Current memory: " << (tracker.getCurrentUsage() / 1024) << " KB" << std::endl;
    }
};
```

## Performance Tuning

### 1. Compiler Optimizations

#### Release Build Flags
```bash
g++ -std=c++17 -O3 -DNDEBUG -march=native -mtune=native \
    -flto -fwhole-program -fno-exceptions \
    -Wall -Wextra
```

#### Profile-Guided Optimization
```bash
# First pass: Generate profile data
g++ -std=c++17 -O2 -fprofile-generate -o server
./server --benchmark-mode

# Second pass: Use profile data
g++ -std=c++17 -O3 -fprofile-use -o server
```

### 2. System Tuning

#### TCP Buffer Sizes
```cpp
void optimizeTCPBuffers(int socket_fd) {
    int send_buffer_size = 1024 * 1024;  // 1MB
    int recv_buffer_size = 1024 * 1024;  // 1MB
    
    setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, &send_buffer_size, sizeof(send_buffer_size));
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &recv_buffer_size, sizeof(recv_buffer_size));
}
```

#### CPU Affinity
```cpp
void setCPUAffinity(int thread_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(thread_id % std::thread::hardware_concurrency(), &cpuset);
    
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}
```

### 3. Memory Tuning

#### NUMA Awareness
```cpp
class NUMAManager {
public:
    void allocateOnNode(void* ptr, size_t size, int node) {
        // Allocate memory on specific NUMA node
        numa_alloc_onnode(size, node);
    }
};
```

#### Memory Prefetching
```cpp
void prefetchData(const void* ptr) {
    __builtin_prefetch(ptr, 0, 3);  // Read, high temporal locality
}
```

## Monitoring and Profiling

### 1. Real-time Monitoring

```cpp
class PerformanceMonitor {
public:
    void startMonitoring() {
        monitoring_thread_ = std::thread([this]() {
            while (running_) {
                collectMetrics();
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
    }
    
private:
    void collectMetrics() {
        auto& tracker = MemoryTracker::getInstance();
        
        std::cout << "Memory: " << (tracker.getCurrentUsage() / 1024) << " KB" << std::endl;
        std::cout << "Connections: " << getConnectionCount() << std::endl;
        std::cout << "Messages/sec: " << calculateThroughput() << std::endl;
    }
};
```

### 2. Profiling Tools

#### Valgrind
```bash
valgrind --tool=massif --pages-as-heap=yes ./server
```

#### perf
```bash
perf record -g ./server
perf report
```

#### AddressSanitizer
```bash
g++ -fsanitize=address -g -o server
./server
```

## Performance Targets

### Typical Performance Expectations

| Metric | Target | Notes |
|--------|--------|-------|
| Throughput | 100,000+ MPS | Small messages (256B) |
| Latency | < 100μs | P95 response time |
| Memory | < 100MB | Peak usage for 1000 connections |
| CPU | < 80% | Under normal load |
| Connections | 10,000+ | Concurrent connections |

### Scaling Guidelines

- **Linear scaling** up to 4 CPU cores
- **Memory usage** scales linearly with connections
- **Throughput** plateaus around 8-16 threads
- **Latency** increases with connection count

## Troubleshooting Performance Issues

### Common Performance Problems

1. **Memory Fragmentation**: Use memory pools
2. **Lock Contention**: Use lock-free data structures
3. **Cache Misses**: Optimize data layout
4. **System Calls**: Batch operations
5. **Context Switches**: Optimize thread count

### Performance Debugging

```cpp
// Enable performance debugging
#ifdef PERFORMANCE_DEBUG
    #define PERF_START(name) performance_monitor_.startMeasurement(name)
    #define PERF_END(name) performance_monitor_.endMeasurement(name)
#else
    #define PERF_START(name)
    #define PERF_END(name)
#endif
```

This comprehensive performance guide provides the tools and techniques needed to optimize your TCP server for maximum performance and efficiency.
