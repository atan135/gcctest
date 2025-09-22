#!/bin/bash
# ===============================================
# NetworkServer Background Runner
# ===============================================
# This script runs the NetworkServer in background mode with proper signal handling
#
# Usage:
#   ./run_server_background.sh [options]
#
# Options:
#   --help, -h           Show this help message
#   --stop, -s           Stop running background server
#   --status             Check if server is running
#   --restart, -r        Restart the server
#
# Examples:
#   ./run_server_background.sh
#   ./run_server_background.sh --stop
#   ./run_server_background.sh --status
#   ./run_server_background.sh --restart
# ===============================================

# Configuration
SERVER_EXECUTABLE="./NetworkServer"
PID_FILE="networkserver.pid"
LOG_FILE="networkserver.log"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

# Function to check if server is running
is_server_running() {
    if [ -f "$PID_FILE" ]; then
        local pid=$(cat "$PID_FILE")
        if ps -p "$pid" > /dev/null 2>&1; then
            return 0
        else
            # PID file exists but process is dead
            rm -f "$PID_FILE"
            return 1
        fi
    else
        return 1
    fi
}

# Function to get server PID
get_server_pid() {
    if [ -f "$PID_FILE" ]; then
        cat "$PID_FILE"
    else
        echo ""
    fi
}

# Function to start server
start_server() {
    if is_server_running; then
        local pid=$(get_server_pid)
        print_status $YELLOW "Server is already running (PID: $pid)"
        return 1
    fi
    
    if [ ! -f "$SERVER_EXECUTABLE" ]; then
        print_status $RED "Error: $SERVER_EXECUTABLE not found!"
        print_status $YELLOW "Please build the server first:"
        print_status $YELLOW "  make"
        print_status $YELLOW "  or"
        print_status $YELLOW "  ./build.sh"
        return 1
    fi
    
    print_status $BLUE "Starting NetworkServer in background..."
    
    # Start server in background and capture PID
    nohup "$SERVER_EXECUTABLE" > "$LOG_FILE" 2>&1 &
    local pid=$!
    
    # Save PID to file
    echo "$pid" > "$PID_FILE"
    
    # Give server a moment to start
    sleep 2
    
    if is_server_running; then
        print_status $GREEN "Server started successfully!"
        print_status $GREEN "PID: $pid"
        print_status $GREEN "Log file: $LOG_FILE"
        print_status $YELLOW "To stop server: $0 --stop"
        print_status $YELLOW "To view logs: tail -f $LOG_FILE"
    else
        print_status $RED "Failed to start server!"
        print_status $YELLOW "Check log file: $LOG_FILE"
        rm -f "$PID_FILE"
        return 1
    fi
}

# Function to stop server
stop_server() {
    if ! is_server_running; then
        print_status $YELLOW "Server is not running"
        return 1
    fi
    
    local pid=$(get_server_pid)
    print_status $BLUE "Stopping server (PID: $pid)..."
    
    # Try graceful shutdown first
    if kill -SIGUSR1 "$pid" 2>/dev/null; then
        # Wait for graceful shutdown
        local count=0
        while [ $count -lt 10 ] && is_server_running; do
            sleep 1
            count=$((count + 1))
        done
        
        if is_server_running; then
            print_status $YELLOW "Graceful shutdown timed out, forcing stop..."
            kill -9 "$pid" 2>/dev/null
        fi
    else
        # Force kill if SIGUSR1 failed
        kill -9 "$pid" 2>/dev/null
    fi
    
    # Clean up
    rm -f "$PID_FILE"
    
    if is_server_running; then
        print_status $RED "Failed to stop server!"
        return 1
    else
        print_status $GREEN "Server stopped successfully"
        return 0
    fi
}

# Function to show server status
show_status() {
    if is_server_running; then
        local pid=$(get_server_pid)
        print_status $GREEN "Server is running (PID: $pid)"
        print_status $BLUE "Log file: $LOG_FILE"
        print_status $YELLOW "To stop: $0 --stop"
        print_status $YELLOW "To view logs: tail -f $LOG_FILE"
    else
        print_status $RED "Server is not running"
    fi
}

# Function to restart server
restart_server() {
    print_status $BLUE "Restarting server..."
    stop_server
    sleep 1
    start_server
}

# Function to show help
show_help() {
    echo "NetworkServer Background Runner"
    echo ""
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  --help, -h           Show this help message"
    echo "  --stop, -s           Stop running background server"
    echo "  --status             Check if server is running"
    echo "  --restart, -r        Restart the server"
    echo ""
    echo "Examples:"
    echo "  $0                   # Start server in background"
    echo "  $0 --stop            # Stop server"
    echo "  $0 --status          # Check server status"
    echo "  $0 --restart         # Restart server"
    echo ""
    echo "Files:"
    echo "  $PID_FILE            # Process ID file"
    echo "  $LOG_FILE            # Server log file"
}

# Main script logic
case "${1:-}" in
    --help|-h)
        show_help
        ;;
    --stop|-s)
        stop_server
        ;;
    --status)
        show_status
        ;;
    --restart|-r)
        restart_server
        ;;
    "")
        start_server
        ;;
    *)
        print_status $RED "Unknown option: $1"
        echo ""
        show_help
        exit 1
        ;;
esac
