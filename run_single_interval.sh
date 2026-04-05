#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: ./run_single_interval.sh -i <input.{ll,c,cpp}> [-o <output-file>] [-b <build-dir>] [-k]

Options:
  -i <file>   Input source (.ll, .c, .cpp)
  -o <file>   Output file for pass output (default: <input>_interval.out)
  -b <dir>    Build directory (default: ./build)
  -k          Keep intermediate .ll when compiling from C/C++
  -h          Show this help message
EOF
}

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
INPUT_FILE=""
OUTPUT_FILE=""
KEEP_IR=0

while getopts ":i:o:b:kh" opt; do
  case "$opt" in
    i) INPUT_FILE="$OPTARG" ;;
    o) OUTPUT_FILE="$OPTARG" ;;
    b) BUILD_DIR="$OPTARG" ;;
    k) KEEP_IR=1 ;;
    h)
      usage
      exit 0
      ;;
    :) echo "error: option -$OPTARG requires an argument"; usage; exit 1 ;;
    \?) echo "error: invalid option -$OPTARG"; usage; exit 1 ;;
  esac
done

if [[ -z "$INPUT_FILE" ]]; then
  echo "error: -i is required"
  usage
  exit 1
fi

if [[ ! -f "$INPUT_FILE" ]]; then
  echo "error: input file not found: $INPUT_FILE"
  exit 1
fi

OPT_BIN="$BUILD_DIR/bin/opt"
CLANG_BIN="$BUILD_DIR/bin/clang"
if [[ ! -x "$OPT_BIN" ]]; then
  echo "error: opt binary not found at $OPT_BIN"
  echo "hint: run ./run_build_check.sh first"
  exit 1
fi

WORK_IR="$INPUT_FILE"
TEMP_IR=""
EXT="${INPUT_FILE##*.}"

if [[ "$EXT" == "c" || "$EXT" == "cpp" || "$EXT" == "cc" || "$EXT" == "cxx" ]]; then
  if [[ ! -x "$CLANG_BIN" ]]; then
    echo "error: clang binary not found at $CLANG_BIN"
    echo "hint: run ./run_build_check.sh first"
    exit 1
  fi

  TEMP_IR="${INPUT_FILE%.*}.interval.tmp.ll"
  "$CLANG_BIN" -S -emit-llvm -O0 -Xclang -disable-O0-optnone "$INPUT_FILE" -o "$TEMP_IR"
  WORK_IR="$TEMP_IR"
fi

if [[ -z "$OUTPUT_FILE" ]]; then
  BASE_NAME="$(basename "${INPUT_FILE%.*}")"
  OUTPUT_FILE="$ROOT_DIR/${BASE_NAME}_interval.out"
fi

"$OPT_BIN" -disable-output -passes="interval-analysis" "$WORK_IR" > "$OUTPUT_FILE"

echo "Interval analysis output: $OUTPUT_FILE"

if [[ -n "$TEMP_IR" && "$KEEP_IR" -eq 0 ]]; then
  rm -f "$TEMP_IR"
fi
