#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>

using namespace std;

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
    
    void sendMessage(const std::string& message) {
        if (socket_fd_ == -1) return;
        
        // Add newline delimiter for TCP message framing
        std::string tcp_message = message + "\n";
        
        ssize_t bytes_sent = send(socket_fd_, tcp_message.c_str(), tcp_message.length(), 0);
        if (bytes_sent < 0) {
            std::cerr << "Failed to send message" << std::endl;
            return;
        }
        
        std::cout << "Sent: " << message << std::endl;
    }
    
    std::string receiveMessage() {
        if (socket_fd_ == -1) return "";
        
        char buffer[4096];
        ssize_t bytes_received = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            return "";
        }
        
        buffer[bytes_received] = '\0';
        std::string message(buffer);
        
        // Remove newline delimiter if present
        if (!message.empty() && message.back() == '\n') {
            message.pop_back();
        }
        
        return message;
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
    
    // Send multiple messages to test persistent connection
    for (int i = 0; i < 3; ++i) {
        std::string message = "Hello message " + std::to_string(i + 1);
        client.sendMessage(message);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::string response = client.receiveMessage();
        std::cout << "Response: " << response << std::endl;
    }
    
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
                client.sendMessage(message);
                std::string response = client.receiveMessage();
                std::cout << "Client " << i << " received: " << response << std::endl;
                
                // Keep connection alive and send more messages
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                client.sendMessage("Second message from client " + std::to_string(i));
                response = client.receiveMessage();
                std::cout << "Client " << i << " received: " << response << std::endl;
                
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
                    client.sendMessage(message);
                    std::string response = client.receiveMessage();
                    std::cout << "Client " << i << " got: " << response << std::endl;
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
    std::cout << "TCP Server Test Client" << std::endl;
    std::cout << "Make sure the TCP server is running on port 8080" << std::endl;
    std::cout << "This client will test persistent TCP connections with message framing" << std::endl;
    std::cout << "Press Enter to start tests..." << std::endl;
    std::cin.get();
    
    testSingleConnection();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    testMultipleConnections();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    testConcurrentConnections();
    
    std::cout << "\nAll TCP tests completed!" << std::endl;
    return 0;
}
