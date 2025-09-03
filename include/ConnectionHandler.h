#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <memory>
#include <functional>

class ConnectionHandler {
public:
    ConnectionHandler(int client_fd, const std::string& client_ip, int client_port);
    ~ConnectionHandler();
    
    void handle();
    void sendResponse(const std::string& response);
    std::string receiveRequest();
    bool isConnected() const;
    void close();
    
    int getClientFd() const { return client_fd_; }
    std::string getClientInfo() const;

private:
    int client_fd_;
    std::string client_ip_;
    int client_port_;
    bool connected_;
    
    void processRequest(const std::string& request);
    std::string generateResponse(const std::string& request);
};
