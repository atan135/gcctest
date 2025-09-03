#include "NetworkServer.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    // Default values
    int port = 8080;
    int max_connections = 1000;
    int thread_count = 4;
    
    // Parse command line arguments
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (const std::exception& e) {
            std::cerr << "Invalid port number: " << argv[1] << std::endl;
            return 1;
        }
    }
    
    if (argc > 2) {
        try {
            max_connections = std::stoi(argv[2]);
        } catch (const std::exception& e) {
            std::cerr << "Invalid max connections: " << argv[2] << std::endl;
            return 1;
        }
    }
    
    if (argc > 3) {
        try {
            thread_count = std::stoi(argv[3]);
        } catch (const std::exception& e) {
            std::cerr << "Invalid thread count: " << argv[3] << std::endl;
            return 1;
        }
    }
    
    std::cout << "Starting Network Server..." << std::endl;
    std::cout << "Port: " << port << std::endl;
    std::cout << "Max connections: " << max_connections << std::endl;
    std::cout << "Thread count: " << thread_count << std::endl;
    std::cout << "Usage: " << argv[0] << " [port] [max_connections] [thread_count]" << std::endl;
    std::cout << "Press Ctrl+C to stop the server" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    try {
        NetworkServer server(port, max_connections, thread_count);
        
        if (!server.start()) {
            std::cerr << "Failed to start server" << std::endl;
            return 1;
        }
        
        // Run the server
        server.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Server shutdown complete" << std::endl;
    return 0;
}
