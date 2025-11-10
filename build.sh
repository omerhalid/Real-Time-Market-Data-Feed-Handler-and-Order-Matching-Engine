#!/bin/bash

# HFT Market Data Engine Build Script for Linux
# This script builds the project with HFT optimizations

set -e

echo "=========================================="
echo "HFT Market Data Engine - Build Script"
echo "=========================================="
echo ""

# Check if running on Linux
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    echo " WARNING: This project is optimized for Linux."
    echo " Building on non-Linux systems may fail or produce suboptimal results."
    echo ""
fi

# Create build directory
echo " Creating build directory..."
mkdir -p build
cd build

# Configure CMake
echo "  Configuring CMake (Release mode)..."
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build
echo "🔨 Building project..."
make -j$(nproc)

echo ""
echo " Build complete!"
echo ""
echo "To run:"
echo "  sudo ./bin/MarketDataEngine    # With full optimizations (requires root)"
echo "  ./bin/MarketDataEngine        # Without root (some optimizations disabled)"
echo ""
echo "For production deployment, see HFT_OPTIMIZATIONS.md for system configuration."

