#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: ./run_single_interval.sh -i <input.{ll,c,cpp}> [-o <output-file>] [-b <build-dir>] [-k] [-r]
                              [-s|-S] [-f|-F] [-l|-L] [-a <artifact-dir>]

Options:
  -i <file>   Input source (.ll, .c, .cpp)
  -o <file>   Output file for pass output (default: <input>_interval.out)
  -b <dir>    Build directory (default: ./build)
  -k          Keep intermediate .ll when compiling from C/C++
  -r          Force rebuild opt/clang with ninja before running
  -a <dir>    Artifact directory (writes original.ir, after-prepass.ir, interval.out, result.md)

Pre-Pass Running Flags:
  -s          Enable sroa pre-pass (default: enabled)
  -S          Disable sroa pre-pass
  -f          Enable simplifycfg pre-pass (default: enabled)
  -F          Disable simplifycfg pre-pass
  -l          Enable loop-simplify pre-pass (default: enabled)
  -L          Disable loop-simplify pre-pass

Notes:
  - cmake is used only for first-time build directory configuration
  - ninja is used for building binaries
  -h          Show this help message
EOF
}

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
LLVM_SRC_DIR="$ROOT_DIR/llvm"
INPUT_FILE=""
OUTPUT_FILE=""
KEEP_IR=0
FORCE_REBUILD=0
PRE_SROA=1
PRE_SIMPLIFYCFG=1
PRE_LOOP_SIMPLIFY=1
ARTIFACT_DIR=""

ensure_build() {
  local jobs="${JOBS:-$(nproc)}"

  if [[ ! -d "$LLVM_SRC_DIR" ]]; then
    echo "error: expected LLVM source at $LLVM_SRC_DIR"
    exit 1
  fi

  mkdir -p "$BUILD_DIR"

  if [[ ! -f "$BUILD_DIR/build.ninja" ]]; then
    cmake -S "$LLVM_SRC_DIR" -B "$BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE=Release
  fi

  if [[ "$FORCE_REBUILD" -eq 1 || ! -x "$BUILD_DIR/bin/opt" || ! -x "$BUILD_DIR/bin/clang" ]]; then
    ninja -C "$BUILD_DIR" -j "$jobs" opt clang
  fi
}

while getopts ":i:o:b:a:krsSfFlLh" opt; do
  case "$opt" in
    i) INPUT_FILE="$OPTARG" ;;
    o) OUTPUT_FILE="$OPTARG" ;;
    b) BUILD_DIR="$OPTARG" ;;
    a) ARTIFACT_DIR="$OPTARG" ;;
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

if [[ -z "$INPUT_FILE" ]]; then
  echo "error: -i is required"
  usage
  exit 1
fi

if [[ ! -f "$INPUT_FILE" ]]; then
  echo "error: input file not found: $INPUT_FILE"
  exit 1
fi

ensure_build

OPT_BIN="$BUILD_DIR/bin/opt"
CLANG_BIN="$BUILD_DIR/bin/clang"

WORK_IR="$INPUT_FILE"
TEMP_IR=""
EXT="${INPUT_FILE##*.}"

if [[ "$EXT" == "c" || "$EXT" == "cpp" || "$EXT" == "cc" || "$EXT" == "cxx" ]]; then
  TEMP_IR="${INPUT_FILE%.*}.interval.tmp.ll"
  "$CLANG_BIN" -S -emit-llvm -O0 -Xclang -disable-O0-optnone "$INPUT_FILE" -o "$TEMP_IR"
  WORK_IR="$TEMP_IR"
fi

if [[ -z "$OUTPUT_FILE" ]]; then
  BASE_NAME="$(basename "${INPUT_FILE%.*}")"
  OUTPUT_FILE="$ROOT_DIR/${BASE_NAME}_interval.out"
fi

declare -a PRE_PIPELINE=()
if [[ "$PRE_SROA" -eq 1 ]]; then
  PRE_PIPELINE+=("sroa")
fi
if [[ "$PRE_SIMPLIFYCFG" -eq 1 ]]; then
  PRE_PIPELINE+=("simplifycfg")
fi
if [[ "$PRE_LOOP_SIMPLIFY" -eq 1 ]]; then
  PRE_PIPELINE+=("loop-simplify")
fi

PRE_PASS_PIPELINE="$(IFS=,; echo "${PRE_PIPELINE[*]}")"
PASS_PIPELINE="$PRE_PASS_PIPELINE"
if [[ -n "$PASS_PIPELINE" ]]; then
  PASS_PIPELINE+=",interval-analysis"
else
  PASS_PIPELINE="interval-analysis"
fi

echo "Pre-Pass Running:"
echo "  sroa: $([[ "$PRE_SROA" -eq 1 ]] && echo enabled || echo disabled)"
echo "  simplifycfg: $([[ "$PRE_SIMPLIFYCFG" -eq 1 ]] && echo enabled || echo disabled)"
echo "  loop-simplify: $([[ "$PRE_LOOP_SIMPLIFY" -eq 1 ]] && echo enabled || echo disabled)"
echo "Pipeline: $PASS_PIPELINE"

if [[ -n "$ARTIFACT_DIR" ]]; then
  mkdir -p "$ARTIFACT_DIR"

  ORIGINAL_IR="$ARTIFACT_DIR/original.ir"
  PREPASS_IR="$ARTIFACT_DIR/after-prepass.ir"
  INTERVAL_OUT="$ARTIFACT_DIR/interval.out"
  REPORT_MD="$ARTIFACT_DIR/result.md"

  cp "$WORK_IR" "$ORIGINAL_IR"

  if [[ -n "$PRE_PASS_PIPELINE" ]]; then
    "$OPT_BIN" -S -passes="$PRE_PASS_PIPELINE" "$ORIGINAL_IR" -o "$PREPASS_IR"
  else
    cp "$ORIGINAL_IR" "$PREPASS_IR"
  fi

  "$OPT_BIN" -disable-output -passes="interval-analysis" "$PREPASS_IR" > "$INTERVAL_OUT"

  {
    echo "# Interval Analysis Report"
    echo
    echo '```text'
    echo "+------------------------------------------------------------------+"
    echo "|                    INTERVAL ANALYSIS REPORT                      |"
    echo "+------------------------------------------------------------------+"
    echo "| Test File   : $INPUT_FILE"
    echo "| Pre-Passes  : ${PRE_PASS_PIPELINE:-none}"
    echo "| Main Pass   : interval-analysis"
    echo "+------------------------------------------------------------------+"
    echo '```'
    echo
    echo "## Generated Artifacts"
    echo
    echo "- original.ir"
    echo "- after-prepass.ir"
    echo "- interval.out"
    echo
    echo "## Final Interval Output"
    echo
    echo '```text'
    cat "$INTERVAL_OUT"
    echo '```'
  } > "$REPORT_MD"

  if [[ "$INTERVAL_OUT" != "$OUTPUT_FILE" ]]; then
    cp "$INTERVAL_OUT" "$OUTPUT_FILE"
  fi
  echo "Artifact directory: $ARTIFACT_DIR"
  echo "Markdown report: $REPORT_MD"
else
  "$OPT_BIN" -disable-output -passes="$PASS_PIPELINE" "$WORK_IR" > "$OUTPUT_FILE"
fi

echo "Interval analysis output: $OUTPUT_FILE"

if [[ -n "$TEMP_IR" && "$KEEP_IR" -eq 0 ]]; then
  rm -f "$TEMP_IR"
fi
