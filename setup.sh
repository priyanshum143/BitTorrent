#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ./setup.sh
#
# This script:
#  - installs system deps (best-effort)
#  - configures + builds the project

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

while [[ $# -gt 0 ]]; do
  case "$1" in
    -h|--help)
      echo "Usage:"
      echo "  ./setup.sh"
      exit 0
      ;;
    *)
      echo "Unknown arg: $1"
      exit 1
      ;;
  esac
done

need_cmd() { command -v "$1" >/dev/null 2>&1; }

install_linux_deps() {
  echo "[setup] Detected Linux. Installing dependencies..."

  if need_cmd apt-get; then
    sudo apt-get update
    sudo apt-get install -y \
      build-essential cmake pkg-config \
      libcurl4-openssl-dev libssl-dev
    return
  fi

  if need_cmd dnf; then
    sudo dnf install -y \
      gcc-c++ cmake pkgconfig \
      libcurl-devel openssl-devel
    return
  fi

  if need_cmd yum; then
    sudo yum install -y \
      gcc-c++ cmake pkgconfig \
      libcurl-devel openssl-devel
    return
  fi

  if need_cmd pacman; then
    sudo pacman -Sy --noconfirm \
      base-devel cmake pkgconf \
      curl openssl
    return
  fi

  echo "[setup] Could not detect a supported Linux package manager."
  echo "        Please install: cmake, a C++17 compiler, libcurl dev, openssl dev."
  exit 1
}

install_macos_deps() {
  echo "[setup] Detected macOS. Installing dependencies..."

  if ! need_cmd brew; then
    echo "[setup] Homebrew not found."
    echo "        Install it from https://brew.sh and re-run."
    exit 1
  fi

  brew update
  brew install cmake curl openssl@3 pkg-config

  # Help CMake find Homebrew OpenSSL/Curl if needed.
  export PKG_CONFIG_PATH="$(brew --prefix)/opt/openssl@3/lib/pkgconfig:$(brew --prefix)/opt/curl/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
}

build_project() {
  echo "[setup] Configuring CMake..."
  cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release

  echo "[setup] Building..."
  cmake --build "${BUILD_DIR}" -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

  echo "[setup] Build complete: ${BUILD_DIR}/bt_main"
}

OS="$(uname -s)"
case "${OS}" in
  Linux)  install_linux_deps ;;
  Darwin) install_macos_deps ;;
  *)
    echo "[setup] Unsupported OS for setup.sh: ${OS}"
    echo "        Use Linux/macOS, or on Windows use setup.ps1 (WSL)."
    exit 1
    ;;
esac

build_project

echo "[setup] Done."
