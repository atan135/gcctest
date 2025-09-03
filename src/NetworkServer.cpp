#include "NetworkServer.h"
#include <iostream>
#include <cstring>
#include <csignal>
#include <sys/epoll.h>

NetworkServer* NetworkServer::instance_ = nullptr;

NetworkServer::NetworkServer(int port, int max_connections, int thread_count)
    : port_(port), max_connections_(max_connections), server_fd_(-1), 
      epoll_fd_(-1), running_(false) {
    
    // Initialize thread pool
    thread_pool_ = std::make_unique<ThreadPool>(thread_count);
    
    // Set up signal handling
    instance_ = this;
    signal(SIGINT, staticSignalHandler);
    signal(SIGTERM, staticSignalHandler);
}

NetworkServer::~NetworkServer() {
    stop();
}

bool NetworkServer::start() {
    if (!setupServer()) {
        std::cerr << "Failed to setup server" << std::endl;
        return false;
    }
    
    if (!setupEpoll()) {
        std::cerr << "Failed to setup epoll" << std::endl;
        return false;
    }
    
    running_ = true;
    std::cout << "Server started on port " << port_ << std::endl;
    std::cout << "Max connections: " << max_connections_ << std::endl;
    std::cout << "Thread pool size: " << thread_pool_->workers.size() << std::endl;
    
    return true;
}

void NetworkServer::stop() {
    if (running_) {
        running_ = false;
        
        // Close all client connections
        for (auto& pair : connections_) {
            pair.second->close();
        }
        connections_.clear();
        
        // Close server socket
        if (server_fd_ != -1) {
            close(server_fd_);
            server_fd_ = -1;
        }
        
        // Close epoll
        if (epoll_fd_ != -1) {
            close(epoll_fd_);
            epoll_fd_ = -1;
        }
        
        std::cout << "Server stopped" << std::endl;
    }
}

void NetworkServer::run() {
    const int MAX_EVENTS = 100;
    struct epoll_event events[MAX_EVENTS];
    
    while (running_) {
        int num_events = epoll_wait(epoll_fd_, events, MAX_EVENTS, 1000); // 1 second timeout
        
        if (num_events == -1) {
            if (errno == EINTR) {
                continue; // Interrupted by signal
            }
            std::cerr << "epoll_wait error: " << strerror(errno) << std::endl;
            break;
        }
        
        for (int i = 0; i < num_events; ++i) {
            int fd = events[i].data.fd;
            uint32_t event_mask = events[i].events;
            
            if (fd == server_fd_) {
                // New connection
                handleNewConnection();
            } else {
                // Client event
                handleClientEvent(fd, event_mask);
            }
        }
    }
}

bool NetworkServer::setupServer() {
    // Create socket
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ == -1) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return false;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        std::cerr << "Failed to set SO_REUSEADDR: " << strerror(errno) << std::endl;
        return false;
    }
    
    // Set non-blocking
    setNonBlocking(server_fd_);
    
    // Bind socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);
    
    if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) == -1) {
        std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
        return false;
    }
    
    // Listen
    if (listen(server_fd_, max_connections_) == -1) {
        std::cerr << "Failed to listen: " << strerror(errno) << std::endl;
        return false;
    }
    
    return true;
}

bool NetworkServer::setupEpoll() {
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ == -1) {
        std::cerr << "Failed to create epoll: " << strerror(errno) << std::endl;
        return false;
    }
    
    // Add server socket to epoll
    struct epoll_event event;
    event.data.fd = server_fd_;
    event.events = EPOLLIN | EPOLLET; // Edge-triggered
    
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_fd_, &event) == -1) {
        std::cerr << "Failed to add server socket to epoll: " << strerror(errno) << std::endl;
        return false;
    }
    
    return true;
}

void NetworkServer::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "Failed to get socket flags: " << strerror(errno) << std::endl;
        return;
    }
    
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "Failed to set socket non-blocking: " << strerror(errno) << std::endl;
    }
}

void NetworkServer::handleNewConnection() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    while (true) {
        int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No more connections
                break;
            }
            std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
            continue;
        }
        
        // Set non-blocking
        setNonBlocking(client_fd);
        
        // Get client info
        std::string client_ip = inet_ntoa(client_addr.sin_addr);
        int client_port = ntohs(client_addr.sin_port);
        
        std::cout << "New connection from " << client_ip << ":" << client_port << std::endl;
        
        // Create connection handler
        auto handler = std::make_unique<ConnectionHandler>(client_fd, client_ip, client_port);
        
        // Add to epoll
        struct epoll_event event;
        event.data.fd = client_fd;
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
        
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &event) == -1) {
            std::cerr << "Failed to add client to epoll: " << strerror(errno) << std::endl;
            close(client_fd);
            continue;
        }
        
        // Store connection
        connections_[client_fd] = std::move(handler);
    }
}

void NetworkServer::handleClientEvent(int client_fd, uint32_t events) {
    auto it = connections_.find(client_fd);
    if (it == connections_.end()) {
        return;
    }
    
    if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
        // Client disconnected or error
        cleanupConnection(client_fd);
        return;
    }
    
    if (events & EPOLLIN) {
        // Data available to read
        auto& handler = it->second;
        
        // Submit to thread pool
        thread_pool_->enqueue([&handler]() {
            handler->handle();
        });
    }
}

void NetworkServer::cleanupConnection(int client_fd) {
    auto it = connections_.find(client_fd);
    if (it != connections_.end()) {
        std::cout << "Cleaning up connection: " << it->second->getClientInfo() << std::endl;
        it->second->close();
        connections_.erase(it);
    }
    
    // Remove from epoll
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, client_fd, nullptr);
    close(client_fd);
}

void NetworkServer::signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    running_ = false;
}

void NetworkServer::staticSignalHandler(int signal) {
    if (instance_) {
        instance_->signalHandler(signal);
    }
}
