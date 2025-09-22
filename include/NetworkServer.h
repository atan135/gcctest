#pragma once

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory>
#include <thread>
#include <atomic>
#include <unordered_map>
#include "ThreadPool.h"
#include "ConnectionHandler.h"

class NetworkServer {
public:
    NetworkServer(int port, int max_connections = 1000, int thread_count = 4);
    ~NetworkServer();
    
    bool start();
    void stop();
    void run();
    
    // Message handling
    void setMessageHandler(std::function<void(const std::string&, ConnectionHandler*)> handler);
    void broadcastMessage(const std::string& message);
    void sendToClient(int client_fd, const std::string& message);
    void forceWriteEvent(int client_fd);
    
    // Connection management
    size_t getConnectionCount() const;
    void cleanupInactiveConnections(int timeout_seconds = 300);
    
private:
    int port_;
    int max_connections_;
    int server_fd_;
    int epoll_fd_;
    std::atomic<bool> running_;
    std::unique_ptr<ThreadPool> thread_pool_;
    std::unordered_map<int, std::unique_ptr<ConnectionHandler>> connections_;
    std::function<void(const std::string&, ConnectionHandler*)> message_handler_;
    
    bool setupServer();
    bool setupEpoll();
    void setNonBlocking(int fd);
    void handleNewConnection();
    void handleClientEvent(int client_fd, uint32_t events);
    void cleanupConnection(int client_fd);
};
