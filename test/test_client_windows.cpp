#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>

using namespace std;

class TestClient {
public:
    TestClient(const std::string& host, int port) : host_(host), port_(port) {
#ifdef _WIN32
        socket_fd_ = INVALID_SOCKET;
        // Initialize Winsock
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
        }
#else
        socket_fd_ = -1;
#endif
    }
    
    ~TestClient() {
        disconnect();
#ifdef _WIN32
        WSACleanup();
#endif
    }
    
    bool connect() {
#ifdef _WIN32
        socket_fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (socket_fd_ == INVALID_SOCKET) {
            std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
            return false;
        }
#else
        socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd_ == -1) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }
#endif
        
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_);
        
#ifdef _WIN32
        server_addr.sin_addr.s_addr = inet_addr(host_.c_str());
        if (server_addr.sin_addr.s_addr == INADDR_NONE) {
#else
        if (inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr) <= 0) {
#endif
            std::cerr << "Invalid address" << std::endl;
            return false;
        }
        
#ifdef _WIN32
        if (::connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            std::cerr << "Connection failed: " << WSAGetLastError() << std::endl;
            return false;
        }
#else
        if (::connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Connection failed" << std::endl;
            return false;
        }
#endif
        
        std::cout << "Connected to " << host_ << ":" << port_ << std::endl;
        return true;
    }
    
    void sendMessage(const std::string& message) {
#ifdef _WIN32
        if (socket_fd_ == INVALID_SOCKET) return;
#else
        if (socket_fd_ == -1) return;
#endif
        
        // Add newline delimiter for TCP message framing
        std::string tcp_message = message + "\n";
        
#ifdef _WIN32
        int bytes_sent = send(socket_fd_, tcp_message.c_str(), (int)tcp_message.length(), 0);
        if (bytes_sent == SOCKET_ERROR) {
            std::cerr << "Failed to send message: " << WSAGetLastError() << std::endl;
            return;
        }
#else
        ssize_t bytes_sent = send(socket_fd_, tcp_message.c_str(), tcp_message.length(), 0);
        if (bytes_sent < 0) {
            std::cerr << "Failed to send message" << std::endl;
            return;
        }
#endif
        
        std::cout << "Sent: " << message << std::endl;
    }
    
    std::string receiveMessage(int timeout_ms = 5000) {
#ifdef _WIN32
        if (socket_fd_ == INVALID_SOCKET) return "";
#else
        if (socket_fd_ == -1) return "";
#endif
        
        // Set receive timeout
#ifdef _WIN32
        DWORD timeout = timeout_ms;
        setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
#else
        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
        
        char buffer[4096];
#ifdef _WIN32
        int bytes_received = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            if (WSAGetLastError() == WSAETIMEDOUT) {
                std::cout << "Receive timeout - no response from server" << std::endl;
            }
            return "";
        }
#else
        ssize_t bytes_received = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cout << "Receive timeout - no response from server" << std::endl;
            }
            return "";
        }
#endif
        
        buffer[bytes_received] = '\0';
        std::string message(buffer);
        
        // Remove newline delimiter if present
        if (!message.empty() && message.back() == '\n') {
            message.pop_back();
        }
        
        return message;
    }
    
    // Alternative: Receive with retry logic
    std::string receiveMessageWithRetry(int max_retries = 5, int retry_delay_ms = 200) {
        for (int retry = 0; retry < max_retries; ++retry) {
            std::string response = receiveMessage(1000); // 1 second timeout per try
            if (!response.empty()) {
                return response;
            }
            
            if (retry < max_retries - 1) {
                std::cout << "Retry " << (retry + 1) << "/" << max_retries << " - waiting for response..." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(retry_delay_ms));
            }
        }
        return "";
    }
    
    // Improved: Send and receive in one operation
    std::string sendAndReceive(const std::string& message, int timeout_ms = 3000) {
        sendMessage(message);
        
        // Give server time to process
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        return receiveMessage(timeout_ms);
    }
    
    void disconnect() {
#ifdef _WIN32
        if (socket_fd_ != INVALID_SOCKET) {
            closesocket(socket_fd_);
            socket_fd_ = INVALID_SOCKET;
            std::cout << "Disconnected" << std::endl;
        }
#else
        if (socket_fd_ != -1) {
            close(socket_fd_);
            socket_fd_ = -1;
            std::cout << "Disconnected" << std::endl;
        }
#endif
    }
    
private:
    std::string host_;
    int port_;
#ifdef _WIN32
    SOCKET socket_fd_;
#else
    int socket_fd_;
#endif
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
        
        // Wait a bit longer for server to process
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::string response = client.receiveMessage(3000); // 3 second timeout
        if (!response.empty()) {
            std::cout << "Response: " << response << std::endl;
        } else {
            std::cout << "No response received for message " << (i + 1) << std::endl;
        }
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
    
    std::cout << "TCP Server Test Client (Windows Compatible)" << std::endl;
    std::cout << "Connecting to " << server_ip << ":" << port << std::endl;
    std::cout << "Make sure the TCP server is running on " << server_ip << ":" << port << std::endl;
    
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

