#!/bin/bash

# Build script for Network Server

echo "Building Network Server..."

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
echo "Building..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Executable: build/NetworkServer"
    echo ""
    echo "To run the server:"
    echo "  cd build && ./NetworkServer"
    echo ""
    echo "Or with custom parameters:"
    echo "  cd build && ./NetworkServer 8080 1000 4"
else
    echo "Build failed!"
    exit 1
fi
