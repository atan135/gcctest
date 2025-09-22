#include "NetworkServer.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <csignal>
#include <atomic>
#include <unistd.h>

// Global variables for signal handling
std::atomic<bool> g_shutdown_requested(false);
NetworkServer* g_server_instance = nullptr;

// Signal handler function
void signalHandler(int signal) {
    const char* signal_name = "UNKNOWN";
    
    switch (signal) {
        case SIGINT:
            signal_name = "SIGINT (Ctrl+C)";
            break;
        case SIGTERM:
            signal_name = "SIGTERM";
            break;
        case SIGUSR1:
            signal_name = "SIGUSR1 (Background stop)";
            break;
#ifdef SIGQUIT
        case SIGQUIT:
            signal_name = "SIGQUIT";
            break;
#endif
#ifdef SIGHUP
        case SIGHUP:
            signal_name = "SIGHUP";
            break;
#endif
        default:
            break;
    }
    
    std::cout << "\nReceived signal " << signal << " (" << signal_name << ")" << std::endl;
    std::cout << "Initiating graceful shutdown..." << std::endl;
    
    g_shutdown_requested.store(true);
    
    // If we have a server instance, stop it
    if (g_server_instance) {
        g_server_instance->stop();
    }
}

// Function to register signal handlers
void setupSignalHandlers() {
    std::signal(SIGINT, signalHandler);   // Ctrl+C
    std::signal(SIGTERM, signalHandler);  // Termination request
    
#ifdef SIGQUIT
    std::signal(SIGQUIT, signalHandler);  // Quit signal (Ctrl+\)
#endif

#ifdef SIGHUP
    std::signal(SIGHUP, signalHandler);   // Hangup signal
#endif

    // For background processes, also handle SIGUSR1 for graceful shutdown
    std::signal(SIGUSR1, signalHandler);  // User-defined signal 1

    std::cout << "Signal handlers registered for graceful shutdown" << std::endl;
    std::cout << "For background mode, use 'kill -SIGUSR1 <pid>' to stop server" << std::endl;
}

// Structure to hold configuration settings
struct ServerConfig {
    int port = 8080;
    int max_connections = 1000;
    int thread_count = 4;
};

// Function to read configuration from file
ServerConfig readConfig(const std::string& filename) {
    ServerConfig config;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cout << "Config file '" << filename << "' not found. Using default values." << std::endl;
        return config;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Find the '=' separator
        size_t pos = line.find('=');
        if (pos == std::string::npos) {
            continue; // Skip invalid lines
        }
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        // Parse the configuration values
        try {
            if (key == "port") {
                config.port = std::stoi(value);
            } else if (key == "max_connections") {
                config.max_connections = std::stoi(value);
            } else if (key == "thread_count") {
                config.thread_count = std::stoi(value);
            }
        } catch (const std::exception& e) {
            std::cerr << "Warning: Invalid value for '" << key << "': " << value << std::endl;
        }
    }
    
    file.close();
    std::cout << "Configuration loaded from '" << filename << "'" << std::endl;
    return config;
}

int main() {
    // Setup signal handlers first
    setupSignalHandlers();
    
    // Force stdout to be unbuffered for background execution
    std::cout.setf(std::ios::unitbuf);
    std::cerr.setf(std::ios::unitbuf);
    
    // Load configuration from file
    ServerConfig config = readConfig("settings.config");
    
    // Use config values
    int port = config.port;
    int max_connections = config.max_connections;
    int thread_count = config.thread_count;
    
    // Get process ID for background mode reference
    pid_t pid = getpid();

    std::cout << "Starting Network Server..." << std::endl;
    std::cout << "Process ID: " << pid << std::endl;
    std::cout << "Port: " << port << std::endl;
    std::cout << "Max connections: " << max_connections << std::endl;
    std::cout << "Thread count: " << thread_count << std::endl;
    std::cout << "Configuration loaded from settings.config" << std::endl;
    std::cout << "Edit settings.config to modify server parameters" << std::endl;
    std::cout << "Press Ctrl+C to stop the server (foreground mode)" << std::endl;
    std::cout << "Use 'kill -SIGUSR1 " << pid << "' to stop server (background mode)" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    try {
        NetworkServer server(port, max_connections, thread_count);
        
        // Set global server instance for signal handler
        g_server_instance = &server;
        
        // Set up custom message handler
        server.setMessageHandler([](const std::string& message, ConnectionHandler* handler) {
            std::cout << "Received from " << handler->getClientInfo() 
                      << ": " << message << std::endl;
            
            // Example: Echo the message back
            std::string response = "Server received: " + message;
            handler->sendMessage(response);
            
            // Example: Broadcast to all clients (optional)
            // server.broadcastMessage("Broadcast: " + message);
        });
        
        if (!server.start()) {
            std::cerr << "Failed to start server" << std::endl;
            return 1;
        }
        
        std::cout << "TCP Server is running. Clients can send messages ending with '\\n'" << std::endl;
        std::cout << "Use TestClient to connect and send messages" << std::endl;
        
        // Run the server
        server.run();
        
        // Clear global server instance
        g_server_instance = nullptr;
        
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        g_server_instance = nullptr;
        return 1;
    }
    
    if (g_shutdown_requested.load()) {
        std::cout << "Server shutdown completed gracefully" << std::endl;
    } else {
        std::cout << "Server shutdown complete" << std::endl;
    }
    return 0;
}
