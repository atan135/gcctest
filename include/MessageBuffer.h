#pragma once

#include <memory>
#include <vector>
#include <cstring>
#include <atomic>
#include <mutex>
#include "BufferConfig.h"

// Forward declarations
class MessageBuffer;

// Memory pool for message buffers to avoid fragmentation
class MessageBufferPool {
public:
    static constexpr size_t DEFAULT_BUFFER_SIZE = BufferConfig::MEDIUM_MESSAGE_SIZE;
    static constexpr size_t MAX_POOL_SIZE = BufferConfig::MEDIUM_POOL_SIZE;
    
    MessageBufferPool(size_t buffer_size = DEFAULT_BUFFER_SIZE);
    ~MessageBufferPool();
    
    // Get a buffer from the pool
    std::unique_ptr<MessageBuffer> acquire();
    
    // Return a buffer to the pool
    void release(std::unique_ptr<MessageBuffer> buffer);
    
    // Get pool statistics
    size_t getPoolSize() const { return pool_size_.load(); }
    size_t getActiveBuffers() const { return active_buffers_.load(); }

private:
    size_t buffer_size_;
    std::atomic<size_t> pool_size_;
    std::atomic<size_t> active_buffers_;
    std::vector<std::unique_ptr<MessageBuffer>> available_buffers_;
    std::mutex pool_mutex_;
};

// Efficient message buffer with pre-allocated memory
class MessageBuffer {
public:
    MessageBuffer(size_t capacity);
    ~MessageBuffer();
    
    // Non-copyable, movable
    MessageBuffer(const MessageBuffer&) = delete;
    MessageBuffer& operator=(const MessageBuffer&) = delete;
    MessageBuffer(MessageBuffer&&) = default;
    MessageBuffer& operator=(MessageBuffer&&) = default;
    
    // Buffer operations
    bool append(const char* data, size_t length);
    bool append(const std::string& data);
    bool append(const MessageBuffer& other);
    
    // Send operations
    ssize_t sendPartial(int socket_fd, size_t offset = 0);
    bool isComplete() const { return offset_ >= size_; }
    bool isEmpty() const { return size_ == 0; }
    
    // Getters
    const char* data() const { return buffer_.get(); }
    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    size_t remaining() const { return capacity_ - size_; }
    size_t getOffset() const { return offset_; }
    
    // Reset for reuse
    void reset();
    
    // Split buffer at position (for partial sends)
    std::unique_ptr<MessageBuffer> splitAt(size_t position);

private:
    std::unique_ptr<char[]> buffer_;
    size_t capacity_;
    size_t size_;
    size_t offset_;  // For partial sends
};

// Efficient message queue using pre-allocated buffers
class MessageQueue {
public:
    MessageQueue();
    ~MessageQueue() = default;
    
    // Add message to queue
    bool enqueue(const char* data, size_t length);
    bool enqueue(const std::string& message);
    
    // Get next message for sending
    MessageBuffer* front();
    void pop();
    bool empty() const;
    size_t size() const;
    
    // Clear all messages
    void clear();

private:
    std::vector<std::unique_ptr<MessageBuffer>> messages_;
    mutable std::mutex queue_mutex_;
    MessageBufferPool buffer_pool_;
};
