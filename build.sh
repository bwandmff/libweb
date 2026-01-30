#!/bin/bash

# libweb - WebSocket & HTTP Server Build Script

set -e

echo "Building libweb server..."

# Check if libwebsockets is installed
if ! pkg-config --exists libwebsockets; then
    echo "Error: libwebsockets development library not found!"
    echo ""
    echo "Install on Ubuntu/Debian:"
    echo "  sudo apt-get install libwebsockets-dev"
    echo ""
    echo "Install on CentOS/RHEL/Fedora:"
    echo "  sudo dnf install libwebsockets-devel"
    echo ""
    exit 1
fi

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake ..

# Build the project
echo "Compiling..."
make -j$(nproc)

echo ""
echo "Build completed successfully!"
echo "Executable: build/libweb_server"
echo ""
echo "To run the server:"
echo "  ./build/libweb_server"
echo ""
echo "Then visit http://localhost:8080 in your browser"