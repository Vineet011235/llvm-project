#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: ./run_batch_interval.sh -d <input-dir> [-o <results-dir>] [-b <build-dir>] [-k]

Options:
  -d <dir>    Directory containing .ll, .c, or .cpp files
  -o <dir>    Output directory (default: ./interval_results)
  -b <dir>    Build directory (default: ./build)
  -k          Keep intermediate .ll files for C/C++ inputs
  -h          Show this help message
EOF
}

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
INPUT_DIR=""
OUTPUT_DIR="$ROOT_DIR/interval_results"
KEEP_IR=0

while getopts ":d:o:b:kh" opt; do
  case "$opt" in
    d) INPUT_DIR="$OPTARG" ;;
    o) OUTPUT_DIR="$OPTARG" ;;
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

if [[ -z "$INPUT_DIR" ]]; then
  echo "error: -d is required"
  usage
  exit 1
fi

if [[ ! -d "$INPUT_DIR" ]]; then
  echo "error: input directory not found: $INPUT_DIR"
  exit 1
fi

mkdir -p "$OUTPUT_DIR"

mapfile -t INPUTS < <(find "$INPUT_DIR" -maxdepth 1 -type f \( -name '*.ll' -o -name '*.c' -o -name '*.cpp' -o -name '*.cc' -o -name '*.cxx' \) | sort)

if [[ "${#INPUTS[@]}" -eq 0 ]]; then
  echo "error: no supported input files found in $INPUT_DIR"
  exit 1
fi

SUCCESS=0
FAIL=0

for FILE in "${INPUTS[@]}"; do
  BASE="$(basename "${FILE%.*}")"
  OUT_FILE="$OUTPUT_DIR/${BASE}.interval.out"
  if "$ROOT_DIR/run_single_interval.sh" -i "$FILE" -o "$OUT_FILE" -b "$BUILD_DIR" $([[ "$KEEP_IR" -eq 1 ]] && echo "-k"); then
    echo "[ok] $FILE"
    SUCCESS=$((SUCCESS + 1))
  else
    echo "[fail] $FILE"
    FAIL=$((FAIL + 1))
  fi
done

echo "Batch complete: success=$SUCCESS fail=$FAIL output_dir=$OUTPUT_DIR"

if [[ "$FAIL" -ne 0 ]]; then
  exit 2
fi
