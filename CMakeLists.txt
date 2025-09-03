cmake_minimum_required(VERSION 3.16)
project(NetworkServer VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG")

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/NetworkServer.cpp
    src/ConnectionHandler.cpp
)

# Header files
set(HEADERS
    include/NetworkServer.h
    include/ConnectionHandler.h
    include/ThreadPool.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    Threads::Threads
    pthread
)

# Create test client executable
add_executable(TestClient test_client.cpp)
target_link_libraries(TestClient pthread)

# Install targets
install(TARGETS ${PROJECT_NAME} TestClient DESTINATION bin)
