#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: ./run_batch_interval.sh [-d <input-dir>] [-o <results-dir>] [-b <build-dir>] [-k] [-r] [-g]
                             [-N|-A] [-m|-M] [-s|-S] [-f|-F] [-l|-L]

Options:
  -d <dir>    Directory containing test files recursively (default: ./tests)
  -o <dir>    Output directory (default: ./test-result)
  -b <dir>    Build directory (default: ./build)
  -k          Keep intermediate .ll files for C/C++ inputs
  -r          Force rebuild opt/clang with ninja before running
  -g          Enable LLVM debug output for interval analysis

Pre-Pass Running Flags:
  -N          Disable all pre-passes (mem2reg, sroa, simplifycfg, loop-simplify)
  -A          Enable all pre-passes (sroa, mem2reg, simplifycfg, loop-simplify)
  -m          Enable mem2reg pre-pass (default: enabled)
  -M          Disable mem2reg pre-pass
  -s          Enable sroa pre-pass (default: disabled)
  -S          Disable sroa pre-pass
  -f          Enable simplifycfg pre-pass (default: enabled)
  -F          Disable simplifycfg pre-pass
  -l          Enable loop-simplify pre-pass (default: disabled)
  -L          Disable loop-simplify pre-pass
  -h          Show this help message
EOF
}

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
INPUT_DIR="$ROOT_DIR/tests"
OUTPUT_DIR="$ROOT_DIR/test-result"
CONFIG_FILE="$ROOT_DIR/interval-analysis.config.sh"
LOCAL_CONFIG_FILE="$ROOT_DIR/interval-analysis.config.local.sh"
KEEP_IR=0
FORCE_REBUILD=0
DEBUG_MODE=0

if [[ "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

if [[ -f "$CONFIG_FILE" ]]; then
  # shellcheck disable=SC1090
  source "$CONFIG_FILE"
fi

if [[ -f "$LOCAL_CONFIG_FILE" ]]; then
  # shellcheck disable=SC1090
  source "$LOCAL_CONFIG_FILE"
fi

PRE_MEM2REG="${INTERVAL_DEFAULT_PRE_MEM2REG:-1}"
PRE_SROA="${INTERVAL_DEFAULT_PRE_SROA:-0}"
PRE_SIMPLIFYCFG="${INTERVAL_DEFAULT_PRE_SIMPLIFYCFG:-1}"
PRE_LOOP_SIMPLIFY="${INTERVAL_DEFAULT_PRE_LOOP_SIMPLIFY:-0}"

declare -a PREPASS_ORDER=()
if declare -p INTERVAL_PREPASS_ORDER >/dev/null 2>&1; then
  PREPASS_ORDER=("${INTERVAL_PREPASS_ORDER[@]}")
else
  PREPASS_ORDER=("sroa" "mem2reg" "simplifycfg" "loop-simplify")
fi

join_by() {
  local separator="$1"
  shift
  local IFS="$separator"
  echo "$*"
}

while getopts ":d:o:b:krgNAmMsSfFlLh" opt; do
  case "$opt" in
    d) INPUT_DIR="$OPTARG" ;;
    o) OUTPUT_DIR="$OPTARG" ;;
    b) BUILD_DIR="$OPTARG" ;;
    k) KEEP_IR=1 ;;
    r) FORCE_REBUILD=1 ;;
    g) DEBUG_MODE=1 ;;
    N)
      PRE_MEM2REG=0
      PRE_SROA=0
      PRE_SIMPLIFYCFG=0
      PRE_LOOP_SIMPLIFY=0
      ;;
    A)
      PRE_MEM2REG=1
      PRE_SROA=1
      PRE_SIMPLIFYCFG=1
      PRE_LOOP_SIMPLIFY=1
      ;;
    m) PRE_MEM2REG=1 ;;
    M) PRE_MEM2REG=0 ;;
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

echo "Pre-Pass Running (configured order: $(join_by ' -> ' "${PREPASS_ORDER[@]}")):"
echo "  sroa: $([[ "$PRE_SROA" -eq 1 ]] && echo enabled || echo disabled)"
echo "  mem2reg: $([[ "$PRE_MEM2REG" -eq 1 ]] && echo enabled || echo disabled)"
echo "  simplifycfg: $([[ "$PRE_SIMPLIFYCFG" -eq 1 ]] && echo enabled || echo disabled)"
echo "  loop-simplify: $([[ "$PRE_LOOP_SIMPLIFY" -eq 1 ]] && echo enabled || echo disabled)"
echo "  config: $(basename "$CONFIG_FILE")"
echo "  debug: $([[ "$DEBUG_MODE" -eq 1 ]] && echo enabled || echo disabled)"
if [[ "$FORCE_REBUILD" -eq 1 ]]; then
  echo "Build mode: force rebuild enabled (-r)"
else
  echo "Build mode: incremental ninja check (rebuilds if sources changed)"
fi
echo "Per-test execution logs: <artifact-dir>/run.log"

for FILE in "${INPUTS[@]}"; do
  REL_PATH="${FILE#$INPUT_DIR/}"
  REL_NO_EXT="${REL_PATH%.*}"
  ARTIFACT_DIR="$OUTPUT_DIR/${REL_NO_EXT}"
  RUN_LOG="$ARTIFACT_DIR/run.log"
  mkdir -p "$ARTIFACT_DIR"

  declare -a SINGLE_ARGS
  SINGLE_ARGS=(-i "$FILE" -a "$ARTIFACT_DIR" -b "$BUILD_DIR")
  if [[ "$KEEP_IR" -eq 1 ]]; then
    SINGLE_ARGS+=("-k")
  fi
  if [[ "$FORCE_REBUILD" -eq 1 ]]; then
    SINGLE_ARGS+=("-r")
  fi
  if [[ "$DEBUG_MODE" -eq 1 ]]; then
    SINGLE_ARGS+=("-g")
  fi
  if [[ "$PRE_MEM2REG" -eq 0 ]]; then
    SINGLE_ARGS+=("-M")
  fi
  if [[ "$PRE_SROA" -eq 1 ]]; then
    SINGLE_ARGS+=("-s")
  fi
  if [[ "$PRE_SROA" -eq 0 ]]; then
    SINGLE_ARGS+=("-S")
  fi
  if [[ "$PRE_SIMPLIFYCFG" -eq 1 ]]; then
    SINGLE_ARGS+=("-f")
  fi
  if [[ "$PRE_SIMPLIFYCFG" -eq 0 ]]; then
    SINGLE_ARGS+=("-F")
  fi
  if [[ "$PRE_LOOP_SIMPLIFY" -eq 1 ]]; then
    SINGLE_ARGS+=("-l")
  fi
  if [[ "$PRE_LOOP_SIMPLIFY" -eq 0 ]]; then
    SINGLE_ARGS+=("-L")
  fi

  if "$ROOT_DIR/run_single_interval.sh" "${SINGLE_ARGS[@]}" >"$RUN_LOG" 2>&1; then
    echo "[ok] $FILE"
    SUCCESS=$((SUCCESS + 1))
  else
    echo "[fail] $FILE (see $RUN_LOG)"
    FAIL=$((FAIL + 1))
  fi
done

echo "Batch complete: success=$SUCCESS fail=$FAIL output_dir=$OUTPUT_DIR"

if [[ "$FAIL" -ne 0 ]]; then
  exit 2
fi
