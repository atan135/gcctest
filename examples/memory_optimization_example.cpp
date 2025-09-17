#include "../include/NetworkServer.h"
#include "../include/MessageBuffer.h"
#include "../include/BufferConfig.h"
#include <iostream>
#include <chrono>
#include <thread>

// Example showing memory-efficient message handling
class MemoryOptimizedServer {
private:
    NetworkServer server_;
    std::atomic<size_t> messages_sent_{0};
    std::atomic<size_t> memory_usage_{0};

public:
    MemoryOptimizedServer(int port, int max_connections, int thread_count)
        : server_(port, max_connections, thread_count) {
        
        // Set up memory-efficient message handler
        server_.setMessageHandler([this](const std::string& message, ConnectionHandler* handler) {
            handleMessage(message, handler);
        });
    }

    void handleMessage(const std::string& message, ConnectionHandler* handler) {
        // Example: Echo with memory tracking
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Use pre-allocated buffer for response
        MessageBuffer response_buffer(BufferConfig::MEDIUM_MESSAGE_SIZE);
        response_buffer.append("Echo: ");
        response_buffer.append(message);
        
        // Send using memory-efficient method
        handler->sendMessage(response_buffer);
        
        // Track memory usage
        memory_usage_.store(MemoryTracker::getInstance().getCurrentUsage());
        messages_sent_.fetch_add(1);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        
        if (messages_sent_.load() % 1000 == 0) {
            std::cout << "Processed " << messages_sent_.load() 
                      << " messages, Memory: " << (memory_usage_.load() / 1024) << " KB"
                      << ", Avg processing time: " << duration.count() << " μs" << std::endl;
        }
    }

    void start() {
        if (server_.start()) {
            std::cout << "Memory-optimized server started" << std::endl;
            std::cout << "Buffer sizes - Small: " << BufferConfig::SMALL_MESSAGE_SIZE
                      << ", Medium: " << BufferConfig::MEDIUM_MESSAGE_SIZE
                      << ", Large: " << BufferConfig::LARGE_MESSAGE_SIZE << std::endl;
            
            // Start memory monitoring thread
            std::thread memory_monitor([this]() {
                while (true) {
                    std::this_thread::sleep_for(std::chrono::seconds(10));
                    
                    auto& tracker = MemoryTracker::getInstance();
                    std::cout << "Memory Stats - Current: " << (tracker.getCurrentUsage() / 1024) << " KB"
                              << ", Peak: " << (tracker.getPeakUsage() / 1024) << " KB"
                              << ", Messages sent: " << messages_sent_.load() << std::endl;
                    
                    if (tracker.isMemoryLimitExceeded()) {
                        std::cout << "WARNING: Memory limit exceeded!" << std::endl;
                    }
                }
            });
            memory_monitor.detach();
            
            server_.run();
        }
    }
};

int main() {
    try {
        MemoryOptimizedServer server(8080, 1000, 4);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
