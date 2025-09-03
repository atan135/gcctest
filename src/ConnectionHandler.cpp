#include "ConnectionHandler.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

ConnectionHandler::ConnectionHandler(int client_fd, const std::string& client_ip, int client_port)
    : client_fd_(client_fd), client_ip_(client_ip), client_port_(client_port), connected_(true) {
}

ConnectionHandler::~ConnectionHandler() {
    close();
}

void ConnectionHandler::handle() {
    try {
        std::string request = receiveRequest();
        if (!request.empty()) {
            processRequest(request);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling connection from " << getClientInfo() 
                  << ": " << e.what() << std::endl;
        connected_ = false;
    }
}

std::string ConnectionHandler::receiveRequest() {
    char buffer[4096];
    ssize_t bytes_received = recv(client_fd_, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            std::cout << "Client " << getClientInfo() << " disconnected" << std::endl;
        } else {
            std::cerr << "Error receiving data from " << getClientInfo() << std::endl;
        }
        connected_ = false;
        return "";
    }
    
    buffer[bytes_received] = '\0';
    return std::string(buffer);
}

void ConnectionHandler::sendResponse(const std::string& response) {
    if (!connected_) return;
    
    ssize_t bytes_sent = send(client_fd_, response.c_str(), response.length(), 0);
    if (bytes_sent < 0) {
        std::cerr << "Error sending response to " << getClientInfo() << std::endl;
        connected_ = false;
    }
}

void ConnectionHandler::processRequest(const std::string& request) {
    std::string response = generateResponse(request);
    sendResponse(response);
}

std::string ConnectionHandler::generateResponse(const std::string& request) {
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    // Create HTTP-like response
    std::stringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: text/plain\r\n";
    response << "Content-Length: ";
    
    std::string body = "Hello from server!\n"
                      "Client: " + getClientInfo() + "\n"
                      "Time: " + ss.str() + "\n"
                      "Request: " + request.substr(0, 100) + "\n";
    
    response << body.length() << "\r\n\r\n";
    response << body;
    
    return response.str();
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

std::string ConnectionHandler::getClientInfo() const {
    return client_ip_ + ":" + std::to_string(client_port_);
}
