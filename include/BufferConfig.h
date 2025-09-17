#pragma once

// Configuration for memory buffer optimization
struct BufferConfig {
    // Buffer sizes for different message types
    static constexpr size_t SMALL_MESSAGE_SIZE = 256;    // Chat, commands
    static constexpr size_t MEDIUM_MESSAGE_SIZE = 1024;  // Game state updates
    static constexpr size_t LARGE_MESSAGE_SIZE = 4096;   // Large data transfers
    static constexpr size_t MAX_MESSAGE_SIZE = 16384;    // Maximum allowed message
    
    // Pool configuration
    static constexpr size_t SMALL_POOL_SIZE = 100;
    static constexpr size_t MEDIUM_POOL_SIZE = 50;
    static constexpr size_t LARGE_POOL_SIZE = 20;
    
    // Pre-allocation settings
    static constexpr size_t PREALLOCATED_CONNECTIONS = 100;
    static constexpr size_t READ_BUFFER_RESERVE = 8192;
    
    // Memory management
    static constexpr size_t MAX_TOTAL_MEMORY_MB = 100;  // 100MB limit
    static constexpr size_t CLEANUP_INTERVAL_SECONDS = 30;
};

// Memory usage tracker
class MemoryTracker {
public:
    static MemoryTracker& getInstance();
    
    void allocate(size_t bytes);
    void deallocate(size_t bytes);
    size_t getCurrentUsage() const { return current_usage_.load(); }
    size_t getPeakUsage() const { return peak_usage_.load(); }
    bool isMemoryLimitExceeded() const;
    
    void reset();

private:
    MemoryTracker() = default;
    std::atomic<size_t> current_usage_{0};
    std::atomic<size_t> peak_usage_{0};
    static constexpr size_t MAX_MEMORY_BYTES = BufferConfig::MAX_TOTAL_MEMORY_MB * 1024 * 1024;
};
