#include "BufferConfig.h"
#include <iostream>

MemoryTracker& MemoryTracker::getInstance() {
    static MemoryTracker instance;
    return instance;
}

void MemoryTracker::allocate(size_t bytes) {
    size_t new_usage = current_usage_.fetch_add(bytes) + bytes;
    
    // Update peak usage
    size_t current_peak = peak_usage_.load();
    while (new_usage > current_peak && 
           !peak_usage_.compare_exchange_weak(current_peak, new_usage)) {
        // Retry if another thread updated peak_usage
    }
}

void MemoryTracker::deallocate(size_t bytes) {
    current_usage_.fetch_sub(bytes);
}

bool MemoryTracker::isMemoryLimitExceeded() const {
    return current_usage_.load() > MAX_MEMORY_BYTES;
}

void MemoryTracker::reset() {
    current_usage_.store(0);
    peak_usage_.store(0);
}
