#!/bin/bash
set -e

echo "=== Building & Packaging GeSoftphone for Linux (Ubuntu/Debian) ==="

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

# 1. Update and install dependencies on Ubuntu/Debian
if command -v apt-get &> /dev/null; then
    echo "Installing build dependencies via APT..."
    sudo apt-get update
    sudo apt-get install -y \
        build-essential \
        cmake \
        pkg-config \
        qt6-base-dev \
        qt6-tools-dev \
        libpjproject-dev \
        libsqlite3-dev \
        libsqlitecpp-dev \
        libasound2-dev \
        libpulse-dev \
        libssl-dev \
        libopus-dev \
        libopencore-amrnb-dev \
        libopencore-amrwb-dev
fi

BUILD_DIR="${ROOT_DIR}/build_linux"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo "Configuring CMake..."
cmake "${ROOT_DIR}" -DCMAKE_BUILD_TYPE=Release

echo "Compiling GeSoftphone..."
cmake --build . --config Release -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

echo "Packaging .deb with CPack..."
cpack -G DEB

echo "============================================="
echo " 🎉 Linux Build & Packaging Completed Successfully!"
echo " Binary output: ${BUILD_DIR}/gesoftphone"
echo " Debian/Ubuntu Package: $(ls -1 ${BUILD_DIR}/*.deb 2>/dev/null || echo 'Check build_linux/ for .deb')"
echo "============================================="
