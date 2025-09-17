#include "MessageBuffer.h"
#include <iostream>
#include <algorithm>
#include <sys/socket.h>

// MessageBufferPool Implementation
MessageBufferPool::MessageBufferPool(size_t buffer_size) 
    : buffer_size_(buffer_size), pool_size_(0), active_buffers_(0) {
    // Pre-allocate some buffers
    for (int i = 0; i < 10; ++i) {
        available_buffers_.push_back(std::make_unique<MessageBuffer>(buffer_size_));
        pool_size_.fetch_add(1);
    }
}

MessageBufferPool::~MessageBufferPool() {
    available_buffers_.clear();
}

std::unique_ptr<MessageBuffer> MessageBufferPool::acquire() {
    std::lock_guard<std::mutex> lock(pool_mutex_);
    
    if (!available_buffers_.empty()) {
        auto buffer = std::move(available_buffers_.back());
        available_buffers_.pop_back();
        pool_size_.fetch_sub(1);
        active_buffers_.fetch_add(1);
        buffer->reset();
        return buffer;
    }
    
    // Create new buffer if pool is empty
    if (active_buffers_.load() < MAX_POOL_SIZE) {
        active_buffers_.fetch_add(1);
        return std::make_unique<MessageBuffer>(buffer_size_);
    }
    
    return nullptr; // Pool exhausted
}

void MessageBufferPool::release(std::unique_ptr<MessageBuffer> buffer) {
    if (!buffer) return;
    
    std::lock_guard<std::mutex> lock(pool_mutex_);
    
    if (available_buffers_.size() < MAX_POOL_SIZE) {
        buffer->reset();
        available_buffers_.push_back(std::move(buffer));
        pool_size_.fetch_add(1);
    }
    
    active_buffers_.fetch_sub(1);
}

// MessageBuffer Implementation
MessageBuffer::MessageBuffer(size_t capacity) 
    : capacity_(capacity), size_(0), offset_(0) {
    buffer_ = std::make_unique<char[]>(capacity_);
    MemoryTracker::getInstance().allocate(capacity_);
}

bool MessageBuffer::append(const char* data, size_t length) {
    if (size_ + length > capacity_) {
        return false; // Not enough space
    }
    
    std::memcpy(buffer_.get() + size_, data, length);
    size_ += length;
    return true;
}

bool MessageBuffer::append(const std::string& data) {
    return append(data.c_str(), data.length());
}

bool MessageBuffer::append(const MessageBuffer& other) {
    if (size_ + other.size_ > capacity_) {
        return false; // Not enough space
    }
    
    std::memcpy(buffer_.get() + size_, other.buffer_.get(), other.size_);
    size_ += other.size_;
    return true;
}

ssize_t MessageBuffer::sendPartial(int socket_fd, size_t start_offset) {
    if (start_offset >= size_) {
        return 0; // Nothing to send
    }
    
    size_t bytes_to_send = size_ - start_offset;
    ssize_t bytes_sent = send(socket_fd, buffer_.get() + start_offset, bytes_to_send, 0);
    
    if (bytes_sent > 0) {
        offset_ = start_offset + bytes_sent;
    }
    
    return bytes_sent;
}

void MessageBuffer::reset() {
    size_ = 0;
    offset_ = 0;
}

std::unique_ptr<MessageBuffer> MessageBuffer::splitAt(size_t position) {
    if (position >= size_) {
        return nullptr;
    }
    
    auto new_buffer = std::make_unique<MessageBuffer>(capacity_);
    size_t remaining_size = size_ - position;
    
    if (new_buffer->append(buffer_.get() + position, remaining_size)) {
        size_ = position; // Truncate current buffer
        return new_buffer;
    }
    
    return nullptr;
}

// Destructor to track memory deallocation
MessageBuffer::~MessageBuffer() {
    MemoryTracker::getInstance().deallocate(capacity_);
}

// MessageQueue Implementation
MessageQueue::MessageQueue() : buffer_pool_(MessageBufferPool::DEFAULT_BUFFER_SIZE) {
}

bool MessageQueue::enqueue(const char* data, size_t length) {
    auto buffer = buffer_pool_.acquire();
    if (!buffer) {
        return false; // Pool exhausted
    }
    
    if (!buffer->append(data, length)) {
        buffer_pool_.release(std::move(buffer));
        return false; // Message too large
    }
    
    std::lock_guard<std::mutex> lock(queue_mutex_);
    messages_.push_back(std::move(buffer));
    return true;
}

bool MessageQueue::enqueue(const std::string& message) {
    return enqueue(message.c_str(), message.length());
}

MessageBuffer* MessageQueue::front() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (messages_.empty()) {
        return nullptr;
    }
    return messages_.front().get();
}

void MessageQueue::pop() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (!messages_.empty()) {
        auto buffer = std::move(messages_.front());
        messages_.erase(messages_.begin());
        buffer_pool_.release(std::move(buffer));
    }
}

bool MessageQueue::empty() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return messages_.empty();
}

size_t MessageQueue::size() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return messages_.size();
}

void MessageQueue::clear() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    for (auto& buffer : messages_) {
        buffer_pool_.release(std::move(buffer));
    }
    messages_.clear();
}
