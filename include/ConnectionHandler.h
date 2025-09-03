#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <memory>
#include <functional>
#include <queue>
#include <mutex>
#include <chrono>

class ConnectionHandler {
public:
    ConnectionHandler(int client_fd, const std::string& client_ip, int client_port);
    ~ConnectionHandler();
    
    // Main handling methods
    void handleRead();
    void handleWrite();
    void processMessages();
    
    // Message handling
    void sendMessage(const std::string& message);
    void sendMessage(const char* data, size_t length);
    bool hasMessagesToSend() const;
    
    // Connection management
    bool isConnected() const;
    void close();
    void setDisconnected();
    
    // Getters
    int getClientFd() const { return client_fd_; }
    std::string getClientInfo() const;
    std::chrono::steady_clock::time_point getLastActivity() const { return last_activity_; }
    
    // Message processing callback
    std::function<void(const std::string&, ConnectionHandler*)> onMessageReceived;

private:
    int client_fd_;
    std::string client_ip_;
    int client_port_;
    bool connected_;
    std::chrono::steady_clock::time_point last_activity_;
    
    // Message buffers
    std::string read_buffer_;
    std::queue<std::string> send_queue_;
    mutable std::mutex send_queue_mutex_;
    
    // Message framing
    static const size_t MAX_MESSAGE_SIZE = 4096;
    static const char MESSAGE_DELIMITER = '\n';
    
    // Helper methods
    void updateActivity();
    void processIncomingData();
    void extractMessages();
    void handleDisconnection();
    std::string formatMessage(const std::string& message);
};
