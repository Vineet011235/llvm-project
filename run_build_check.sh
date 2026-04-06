#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: ./run_build_check.sh [-b <build-dir>] [-j <jobs>] [-h|--help]

Options:
  -b <dir>    Build directory (default: ./build)
  -j <jobs>   Parallel build jobs (default: nproc or JOBS env)
  -h          Show this help message
  --help      Show this help message
EOF
}

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LLVM_SRC_DIR="$ROOT_DIR/llvm"
BUILD_DIR="$ROOT_DIR/build"
JOBS="${JOBS:-$(nproc)}"

if [[ "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

while getopts ":b:j:h" opt; do
  case "$opt" in
    b) BUILD_DIR="$OPTARG" ;;
    j) JOBS="$OPTARG" ;;
    h)
      usage
      exit 0
      ;;
    :) echo "error: option -$OPTARG requires an argument"; usage; exit 1 ;;
    \?) echo "error: invalid option -$OPTARG"; usage; exit 1 ;;
  esac
done

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
