#include "ConnectionHandler.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <cstring>

ConnectionHandler::ConnectionHandler(int client_fd, const std::string& client_ip, int client_port)
    : client_fd_(client_fd), client_ip_(client_ip), client_port_(client_port), 
      connected_(true), last_activity_(std::chrono::steady_clock::now()) {
}

ConnectionHandler::~ConnectionHandler() {
    close();
}

void ConnectionHandler::handleRead() {
    if (!connected_) return;
    
    try {
        char buffer[4096];
        ssize_t bytes_received = recv(client_fd_, buffer, sizeof(buffer), 0);
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                std::cout << "Client " << getClientInfo() << " disconnected gracefully" << std::endl;
            } else {
                std::cerr << "Error receiving data from " << getClientInfo() << ": " << strerror(errno) << std::endl;
            }
            handleDisconnection();
            return;
        }
        
        // Append received data to read buffer
        read_buffer_.append(buffer, bytes_received);
        updateActivity();
        
        // Process incoming data
        processIncomingData();
        
    } catch (const std::exception& e) {
        std::cerr << "Error handling read from " << getClientInfo() 
                  << ": " << e.what() << std::endl;
        handleDisconnection();
    }
}

void ConnectionHandler::handleWrite() {
    if (!connected_ || !hasMessagesToSend()) return;
    
    try {
        std::lock_guard<std::mutex> lock(send_queue_mutex_);
        
        while (!send_queue_.empty()) {
            const std::string& message = send_queue_.front();
            ssize_t bytes_sent = send(client_fd_, message.c_str(), message.length(), 0);
            
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
            } else if (bytes_sent < static_cast<ssize_t>(message.length())) {
                // Partial send, remove sent part and keep the rest
                std::string remaining = message.substr(bytes_sent);
                send_queue_.pop();
                send_queue_.push(remaining);
                break;
            } else {
                // Complete message sent
                send_queue_.pop();
                updateActivity();
            }
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
    
    std::string formatted_message = formatMessage(message);
    
    std::lock_guard<std::mutex> lock(send_queue_mutex_);
    send_queue_.push(formatted_message);
}

void ConnectionHandler::sendMessage(const char* data, size_t length) {
    if (!connected_) return;
    
    std::string message(data, length);
    sendMessage(message);
}

bool ConnectionHandler::hasMessagesToSend() const {
    std::lock_guard<std::mutex> lock(send_queue_mutex_);
    return !send_queue_.empty();
}

bool ConnectionHandler::isConnected() const {
    return connected_;
}

void ConnectionHandler::close() {
    if (connected_) {
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
