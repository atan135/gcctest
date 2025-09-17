#include "ConnectionHandler.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <cerrno>

ConnectionHandler::ConnectionHandler(int client_fd, const std::string& client_ip, int client_port)
    : client_fd_(client_fd), client_ip_(client_ip), client_port_(client_port), 
      connected_(true), last_activity_(std::chrono::steady_clock::now()) {
    // Pre-allocate temporary buffer for common operations
    temp_buffer_ = std::make_unique<MessageBuffer>(MAX_MESSAGE_SIZE);
}

ConnectionHandler::~ConnectionHandler() {
    close();
}

void ConnectionHandler::handleRead() {
    if (!connected_) return;
    
    try {
        bool data_received = false;
        
        // Keep reading until no more data available (EAGAIN)
        // This is crucial for edge-triggered epoll
        while (true) {
            char buffer[4096];
            ssize_t bytes_received = recv(client_fd_, buffer, sizeof(buffer), 0);
            
            if (bytes_received <= 0) {
                if (bytes_received == 0) {
                    // Client disconnected gracefully
                    std::cout << "Client " << getClientInfo() << " disconnected gracefully" << std::endl;
                    handleDisconnection();
                    return;
                } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // No more data available - this is normal for non-blocking sockets
                    break;
                } else {
                    // Real error occurred
                    std::cerr << "Error receiving data from " << getClientInfo() << ": " << strerror(errno) << std::endl;
                    handleDisconnection();
                    return;
                }
            }
            
            // Append received data to read buffer
            read_buffer_.append(buffer, bytes_received);
            data_received = true;
            
            // If we received less than buffer size, likely no more data
            if (bytes_received < static_cast<ssize_t>(sizeof(buffer))) {
                break;
            }
        }
        
        // Only update activity and process if we actually received data
        if (data_received) {
            updateActivity();
            processIncomingData();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error handling read from " << getClientInfo() 
                  << ": " << e.what() << std::endl;
        handleDisconnection();
    }
}

void ConnectionHandler::handleWrite() {
    if (!connected_ || !hasMessagesToSend()) return;
    
    try {
        while (!send_queue_.empty()) {
            MessageBuffer* buffer = send_queue_.front();
            if (!buffer) break;
            
            ssize_t bytes_sent = buffer->sendPartial(client_fd_, buffer->getOffset());
            
            if (bytes_sent < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // Socket buffer full, try again later
                    break;
                } else {
                    std::cerr << "Error sending data to " << getClientInfo() 
                              << ": " << strerror(errno) << std::endl;
                    handleDisconnection();
                    return;
                }
            } else if (bytes_sent == 0) {
                // No data sent, try again later
                break;
            } else if (buffer->isComplete()) {
                // Complete message sent
                send_queue_.pop();
                updateActivity();
            }
            // Partial send - buffer will be retried next time
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error handling write to " << getClientInfo() 
                  << ": " << e.what() << std::endl;
        handleDisconnection();
    }
}

void ConnectionHandler::processMessages() {
    if (!connected_) return;
    
    // Extract complete messages from read buffer
    extractMessages();
}

void ConnectionHandler::sendMessage(const std::string& message) {
    if (!connected_) return;
    
    // Use pre-allocated buffer to avoid string allocations
    temp_buffer_->reset();
    temp_buffer_->append(message);
    temp_buffer_->append(MESSAGE_DELIMITER);
    
    send_queue_.enqueue(temp_buffer_->data(), temp_buffer_->size());
}

void ConnectionHandler::sendMessage(const char* data, size_t length) {
    if (!connected_) return;
    
    // Use pre-allocated buffer to avoid string allocations
    temp_buffer_->reset();
    temp_buffer_->append(data, length);
    temp_buffer_->append(MESSAGE_DELIMITER);
    
    send_queue_.enqueue(temp_buffer_->data(), temp_buffer_->size());
}

void ConnectionHandler::sendMessage(const MessageBuffer& buffer) {
    if (!connected_) return;
    
    // Direct enqueue without additional formatting
    send_queue_.enqueue(buffer.data(), buffer.size());
}

bool ConnectionHandler::hasMessagesToSend() const {
    return !send_queue_.empty();
}

bool ConnectionHandler::isConnected() const {
    return connected_;
}

void ConnectionHandler::close() {
    if (connected_) {
        // Clear send queue to free memory
        send_queue_.clear();
        
        ::close(client_fd_);
        connected_ = false;
        std::cout << "Closed connection to " << getClientInfo() << std::endl;
    }
}

void ConnectionHandler::setDisconnected() {
    connected_ = false;
}

std::string ConnectionHandler::getClientInfo() const {
    return client_ip_ + ":" + std::to_string(client_port_);
}

void ConnectionHandler::updateActivity() {
    last_activity_ = std::chrono::steady_clock::now();
}

void ConnectionHandler::processIncomingData() {
    // Check if read buffer is getting too large
    if (read_buffer_.size() > MAX_MESSAGE_SIZE * 10) {
        std::cerr << "Read buffer too large for " << getClientInfo() 
                  << ", disconnecting" << std::endl;
        handleDisconnection();
        return;
    }
    
    // Extract complete messages
    extractMessages();
}

void ConnectionHandler::extractMessages() {
    size_t pos = 0;
    
    while ((pos = read_buffer_.find(MESSAGE_DELIMITER)) != std::string::npos) {
        std::string message = read_buffer_.substr(0, pos);
        read_buffer_.erase(0, pos + 1); // Remove message and delimiter
        
        // Skip empty messages
        if (message.empty()) continue;
        
        // Call message handler if set
        if (onMessageReceived) {
            onMessageReceived(message, this);
        } else {
            // Default echo behavior
            std::string response = "Echo: " + message;
            sendMessage(response);
        }
    }
}

void ConnectionHandler::handleDisconnection() {
    connected_ = false;
    std::cout << "Connection lost: " << getClientInfo() << std::endl;
}

std::string ConnectionHandler::formatMessage(const std::string& message) {
    // Simple message framing: message + delimiter
    return message + MESSAGE_DELIMITER;
}
