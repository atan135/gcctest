#!/bin/bash
# ========================================
# CMake Build Script for Linux/Unix
# ========================================

echo "Building with CMake..."
echo

# Check if cmake is available
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found!"
    echo "Please install CMake:"
    echo "  Ubuntu/Debian: sudo apt install cmake"
    echo "  CentOS/RHEL: sudo yum install cmake"
    echo "  Arch: sudo pacman -S cmake"
    exit 1
fi

echo "CMake found:"
cmake --version | head -1
echo

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
    echo "Created build directory"
fi

# Change to build directory
cd build

# Configure the project
echo "Configuring project..."
cmake ..

if [ $? -ne 0 ]; then
    echo
    echo "ERROR: CMake configuration failed!"
    cd ..
    exit 1
fi

# Build the project
echo
echo "Building project..."
cmake --build . -j$(nproc)

if [ $? -ne 0 ]; then
    echo
    echo "ERROR: Build failed!"
    cd ..
    exit 1
fi

echo
echo "========================================"
echo "BUILD SUCCESSFUL!"
echo "========================================"
echo
echo "Executables created in build/ directory:"
ls -la *.exe 2>/dev/null || ls -la NetworkServer TestClient 2>/dev/null
echo
echo "To run:"
echo "  ./build/NetworkServer"
echo "  ./build/TestClient"
echo

cd ..
echo "Build completed successfully!"
