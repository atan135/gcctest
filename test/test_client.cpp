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

void testSingleConnection(const std::string& server_ip, int port) {
    std::cout << "\n=== Testing Single Connection ===" << std::endl;
    
    TestClient client(server_ip, port);
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

void testMultipleConnections(const std::string& server_ip, int port) {
    std::cout << "\n=== Testing Multiple Connections ===" << std::endl;
    
    const int num_clients = 5;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_clients; ++i) {
        threads.emplace_back([i, server_ip, port]() {
            TestClient client(server_ip, port);
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

void testConcurrentConnections(const std::string& server_ip, int port) {
    std::cout << "\n=== Testing Concurrent Connections ===" << std::endl;
    
    const int num_clients = 10;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_clients; ++i) {
        threads.emplace_back([i, server_ip, port]() {
            TestClient client(server_ip, port);
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

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " <server_ip> <port> [options]" << std::endl;
    std::cout << "  server_ip: IP address of the server (e.g., 127.0.0.1, 192.168.1.100)" << std::endl;
    std::cout << "  port:      Port number of the server (e.g., 8080, 9000)" << std::endl;
    std::cout << "  options:" << std::endl;
    std::cout << "    --help, -h:    Show this help message" << std::endl;
    std::cout << "    --quiet, -q:   Run tests without interactive prompts" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << " 127.0.0.1 8080" << std::endl;
    std::cout << "  " << program_name << " 192.168.1.100 9000 --quiet" << std::endl;
    std::cout << "  " << program_name << " localhost 8080" << std::endl;
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    std::string server_ip;
    int port = 0;
    bool quiet_mode = false;
    
    // Check for help
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        }
        if (arg == "--quiet" || arg == "-q") {
            quiet_mode = true;
        }
    }
    
    // Parse server IP and port
    if (argc < 3) {
        std::cerr << "Error: Missing required arguments" << std::endl;
        std::cerr << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    server_ip = argv[1];
    
    try {
        port = std::stoi(argv[2]);
        if (port <= 0 || port > 65535) {
            throw std::invalid_argument("Port must be between 1 and 65535");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: Invalid port number '" << argv[2] << "': " << e.what() << std::endl;
        std::cerr << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    std::cout << "TCP Server Test Client" << std::endl;
    std::cout << "Connecting to " << server_ip << ":" << port << std::endl;
    std::cout << "This client will test persistent TCP connections with message framing" << std::endl;
    
    if (!quiet_mode) {
        std::cout << "Press Enter to start tests..." << std::endl;
        std::cin.get();
    } else {
        std::cout << "Running in quiet mode..." << std::endl;
    }
    
    try {
        testSingleConnection(server_ip, port);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        testMultipleConnections(server_ip, port);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        testConcurrentConnections(server_ip, port);
        
        std::cout << "\nAll TCP tests completed successfully!" << std::endl;
        
        if (!quiet_mode) {
            std::cout << "Press Enter to exit..." << std::endl;
            std::cin.get();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error during testing: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
