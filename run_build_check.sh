#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LLVM_SRC_DIR="$ROOT_DIR/llvm"
BUILD_DIR="$ROOT_DIR/build"
JOBS="${JOBS:-$(nproc)}"

if [[ ! -d "$LLVM_SRC_DIR" ]]; then
  echo "error: expected LLVM source at $LLVM_SRC_DIR"
  exit 1
fi

mkdir -p "$BUILD_DIR"

if [[ ! -f "$BUILD_DIR/build.ninja" ]]; then
  cmake -S "$LLVM_SRC_DIR" -B "$BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE=Release
fi

ninja -C "$BUILD_DIR" -j "$JOBS" opt clang

echo "Build check successful."
