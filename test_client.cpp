#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

class TestClient {
public:
    TestClient(const std::string& host, int port) : host_(host), port_(port), socket_fd_(-1) {}
    
    ~TestClient() {
        disconnect();
    }
    
    bool connect() {
        socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd_ == -1) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }
        
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_);
        
        if (inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << "Invalid address" << std::endl;
            return false;
        }
        
        if (::connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Connection failed" << std::endl;
            return false;
        }
        
        std::cout << "Connected to " << host_ << ":" << port_ << std::endl;
        return true;
    }
    
    void sendRequest(const std::string& message) {
        if (socket_fd_ == -1) return;
        
        std::string request = "GET / HTTP/1.1\r\n"
                            "Host: " + host_ + "\r\n"
                            "User-Agent: TestClient/1.0\r\n"
                            "Connection: close\r\n"
                            "\r\n"
                            + message;
        
        ssize_t bytes_sent = send(socket_fd_, request.c_str(), request.length(), 0);
        if (bytes_sent < 0) {
            std::cerr << "Failed to send request" << std::endl;
            return;
        }
        
        std::cout << "Sent: " << message << std::endl;
    }
    
    std::string receiveResponse() {
        if (socket_fd_ == -1) return "";
        
        char buffer[4096];
        ssize_t bytes_received = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            return "";
        }
        
        buffer[bytes_received] = '\0';
        return std::string(buffer);
    }
    
    void disconnect() {
        if (socket_fd_ != -1) {
            close(socket_fd_);
            socket_fd_ = -1;
            std::cout << "Disconnected" << std::endl;
        }
    }
    
private:
    std::string host_;
    int port_;
    int socket_fd_;
};

void testSingleConnection() {
    std::cout << "\n=== Testing Single Connection ===" << std::endl;
    
    TestClient client("127.0.0.1", 8080);
    if (!client.connect()) {
        return;
    }
    
    client.sendRequest("Hello from single client!");
    std::string response = client.receiveResponse();
    std::cout << "Response:\n" << response << std::endl;
    
    client.disconnect();
}

void testMultipleConnections() {
    std::cout << "\n=== Testing Multiple Connections ===" << std::endl;
    
    const int num_clients = 5;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_clients; ++i) {
        threads.emplace_back([i]() {
            TestClient client("127.0.0.1", 8080);
            if (client.connect()) {
                std::string message = "Hello from client " + std::to_string(i);
                client.sendRequest(message);
                std::string response = client.receiveResponse();
                std::cout << "Client " << i << " received response" << std::endl;
                
                // Keep connection alive for a bit
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                client.disconnect();
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "All clients completed" << std::endl;
}

void testConcurrentConnections() {
    std::cout << "\n=== Testing Concurrent Connections ===" << std::endl;
    
    const int num_clients = 10;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_clients; ++i) {
        threads.emplace_back([i]() {
            TestClient client("127.0.0.1", 8080);
            if (client.connect()) {
                for (int j = 0; j < 3; ++j) {
                    std::string message = "Request " + std::to_string(j) + " from client " + std::to_string(i);
                    client.sendRequest(message);
                    std::string response = client.receiveResponse();
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
                client.disconnect();
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "All concurrent clients completed" << std::endl;
}

int main() {
    std::cout << "Network Server Test Client" << std::endl;
    std::cout << "Make sure the server is running on port 8080" << std::endl;
    std::cout << "Press Enter to start tests..." << std::endl;
    std::cin.get();
    
    testSingleConnection();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    testMultipleConnections();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    testConcurrentConnections();
    
    std::cout << "\nAll tests completed!" << std::endl;
    return 0;
}
