#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

echo "=== Building GeSoftphone Ubuntu .deb Package via Docker ==="

if ! command -v docker &> /dev/null; then
    echo "Error: docker command not found in PATH."
    exit 1
fi

if ! docker info &> /dev/null; then
    echo "Docker daemon is not running. Attempting to start Docker Desktop..."
    open -a Docker 2>/dev/null || true
    echo "Waiting for Docker daemon to become responsive..."
    attempts=0
    while ! docker info &> /dev/null; do
        sleep 3
        attempts=$((attempts + 1))
        if [ $attempts -gt 25 ]; then
            echo "Error: Docker daemon did not start in time. Please start Docker Desktop manually."
            exit 1
        fi
    done
fi

IMAGE_TAG="gesoftphone-builder:ubuntu22.04"

echo "1. Building Docker environment image..."
docker build -t "${IMAGE_TAG}" -f "${SCRIPT_DIR}/Dockerfile.ubuntu" "${ROOT_DIR}"

echo "2. Compiling and packaging inside container..."
docker run --rm -v "${ROOT_DIR}:/workspace" "${IMAGE_TAG}"

echo "============================================="
echo " 🎉 Ubuntu Build & Packaging Complete!"
echo " Debian package created at:"
ls -lh "${ROOT_DIR}/build_linux/"*.deb 2>/dev/null || true
echo "============================================="
