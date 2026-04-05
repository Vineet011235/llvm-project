#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: ./run_batch_interval.sh [-d <input-dir>] [-o <results-dir>] [-b <build-dir>] [-k] [-r]
                             [-s|-S] [-f|-F] [-l|-L]

Options:
  -d <dir>    Directory containing test files recursively (default: ./tests)
  -o <dir>    Output directory (default: ./test-result)
  -b <dir>    Build directory (default: ./build)
  -k          Keep intermediate .ll files for C/C++ inputs
  -r          Force rebuild opt/clang with ninja before running

Pre-Pass Running Flags:
  -s          Enable sroa pre-pass (default: enabled)
  -S          Disable sroa pre-pass
  -f          Enable simplifycfg pre-pass (default: enabled)
  -F          Disable simplifycfg pre-pass
  -l          Enable loop-simplify pre-pass (default: enabled)
  -L          Disable loop-simplify pre-pass
  -h          Show this help message
EOF
}

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
INPUT_DIR="$ROOT_DIR/tests"
OUTPUT_DIR="$ROOT_DIR/test-result"
KEEP_IR=0
FORCE_REBUILD=0
PRE_SROA=1
PRE_SIMPLIFYCFG=1
PRE_LOOP_SIMPLIFY=1

while getopts ":d:o:b:krsSfFlLh" opt; do
  case "$opt" in
    d) INPUT_DIR="$OPTARG" ;;
    o) OUTPUT_DIR="$OPTARG" ;;
    b) BUILD_DIR="$OPTARG" ;;
    k) KEEP_IR=1 ;;
    r) FORCE_REBUILD=1 ;;
    s) PRE_SROA=1 ;;
    S) PRE_SROA=0 ;;
    f) PRE_SIMPLIFYCFG=1 ;;
    F) PRE_SIMPLIFYCFG=0 ;;
    l) PRE_LOOP_SIMPLIFY=1 ;;
    L) PRE_LOOP_SIMPLIFY=0 ;;
    h)
      usage
      exit 0
      ;;
    :) echo "error: option -$OPTARG requires an argument"; usage; exit 1 ;;
    \?) echo "error: invalid option -$OPTARG"; usage; exit 1 ;;
  esac
done

if [[ ! -d "$INPUT_DIR" ]]; then
  echo "error: input directory not found: $INPUT_DIR"
  exit 1
fi

mkdir -p "$OUTPUT_DIR"

mapfile -t INPUTS < <(find "$INPUT_DIR" -type f \( -name '*.ll' -o -name '*.c' -o -name '*.cpp' -o -name '*.cc' -o -name '*.cxx' \) | sort)

if [[ "${#INPUTS[@]}" -eq 0 ]]; then
  echo "error: no supported input files found in $INPUT_DIR"
  exit 1
fi

SUCCESS=0
FAIL=0

echo "Pre-Pass Running:"
echo "  sroa: $([[ "$PRE_SROA" -eq 1 ]] && echo enabled || echo disabled)"
echo "  simplifycfg: $([[ "$PRE_SIMPLIFYCFG" -eq 1 ]] && echo enabled || echo disabled)"
echo "  loop-simplify: $([[ "$PRE_LOOP_SIMPLIFY" -eq 1 ]] && echo enabled || echo disabled)"

for FILE in "${INPUTS[@]}"; do
  REL_PATH="${FILE#$INPUT_DIR/}"
  REL_NO_EXT="${REL_PATH%.*}"
  OUT_FILE="$OUTPUT_DIR/${REL_NO_EXT}.interval.out"
  mkdir -p "$(dirname "$OUT_FILE")"

  declare -a SINGLE_ARGS
  SINGLE_ARGS=(-i "$FILE" -o "$OUT_FILE" -b "$BUILD_DIR")
  if [[ "$KEEP_IR" -eq 1 ]]; then
    SINGLE_ARGS+=("-k")
  fi
  if [[ "$FORCE_REBUILD" -eq 1 ]]; then
    SINGLE_ARGS+=("-r")
  fi
  if [[ "$PRE_SROA" -eq 0 ]]; then
    SINGLE_ARGS+=("-S")
  fi
  if [[ "$PRE_SIMPLIFYCFG" -eq 0 ]]; then
    SINGLE_ARGS+=("-F")
  fi
  if [[ "$PRE_LOOP_SIMPLIFY" -eq 0 ]]; then
    SINGLE_ARGS+=("-L")
  fi

  if "$ROOT_DIR/run_single_interval.sh" "${SINGLE_ARGS[@]}"; then
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
