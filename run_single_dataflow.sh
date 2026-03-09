#!/usr/bin/env bash
set -euo pipefail

# ==============================
# Single File Dataflow Analysis Runner
# ==============================
# This script compiles a single C/C++ file to LLVM IR and runs dataflow analyses.

# ==============================
# Defaults
# ==============================
LLVM_BUILD_DIR="${LLVM_BUILD_DIR:-$(pwd)/build}"
CLANG_BIN="${CLANG_BIN:-$LLVM_BUILD_DIR/bin/clang}"
OPT_BIN="${OPT_BIN:-$LLVM_BUILD_DIR/bin/opt}"

INPUT_FILE=""
OUTPUT_DIR=""
CFLAGS="-Xclang -disable-O0-optnone -O0 -g"
ANALYSES="all"  # all | live | reach | avail | antic
DEBUG_MODE=false

# ==============================
# Usage
# ==============================
usage() {
  cat << EOF
Usage: $0 [options]

Options:
  -i <file>        Input C/C++ file (default: debug.cc in current directory)
  -o <dir>         Output directory (default: beside input file as <name>-dataflow/)
  -a <analyses>    Analyses to run: all|live|reach|avail|antic (default: all)
  -d               Enable debug mode (generate .log file with detailed output)
  -h               Show this help

Analyses:
  live   - Live Variables Analysis
  reach  - Reaching Definitions Analysis
  avail  - Available Expressions Analysis
  antic  - Anticipable Expressions Analysis
  all    - Run all analyses

Environment overrides:
  LLVM_BUILD_DIR   Path to llvm-project/build (default: ./build)
  CLANG_BIN        Path to clang
  OPT_BIN          Path to opt

Examples:
  $0                                              # Run on default debug.cc file
  $0 -d                                           # Run on debug.cc with debug mode
  $0 -i test/simple/if-else.cc                    # Specify input file
  $0 -i example.c -o ./my-results                 # Custom output location
  $0 -i test.c -a "live reach"                    # Run specific analyses
  $0 -i test.c -d                                 # Enable debug mode with .log file
  
Output:
  - LLVM IR file (.ll)
  - Analysis results (.md files)
  - Summary index (README.md)
EOF
  exit 1
}

# ==============================
# Parse arguments
# ==============================
while getopts "i:o:a:dh" opt; do
  case "$opt" in
    i) INPUT_FILE="$OPTARG" ;;
    o) OUTPUT_DIR="$OPTARG" ;;
    a) ANALYSES="$OPTARG" ;;
    d) DEBUG_MODE=true ;;
    h) usage ;;
    *) usage ;;
  esac
done

if [[ -z "$INPUT_FILE" ]]; then
  # Default mode: use debug.cc in current directory
  INPUT_FILE="debug.cc"
  echo "No input file specified, using default: $INPUT_FILE"
fi

if [[ ! -f "$INPUT_FILE" ]]; then
  echo "Error: File not found: $INPUT_FILE"
  if [[ "$INPUT_FILE" == "debug.cc" ]]; then
    echo "Tip: Create debug.cc or specify an input file with -i <file>"
  fi
  exit 1
fi

# ==============================
# Determine output directory
# ==============================
if [[ -z "$OUTPUT_DIR" ]]; then
  # Default: beside the input file
  INPUT_DIR=$(dirname "$INPUT_FILE")
  INPUT_BASENAME=$(basename "$INPUT_FILE")
  INPUT_NAME="${INPUT_BASENAME%.*}"
  OUTPUT_DIR="$INPUT_DIR/${INPUT_NAME}-dataflow"
fi

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Setup debug log if debug mode enabled
if [[ "$DEBUG_MODE" == true ]]; then
  LOG_FILE="$OUTPUT_DIR/debug.log"
  exec > >(tee -a "$LOG_FILE") 2>&1
  echo "=================================="
  echo "Debug mode enabled"
  echo "Log file: $LOG_FILE"
  echo "Timestamp: $(date '+%Y-%m-%d %H:%M:%S')"
  echo "=================================="
  echo ""
fi

# ==============================
# Check LLVM binaries
# ==============================
if [[ ! -f "$CLANG_BIN" ]]; then
  echo "Error: clang not found at $CLANG_BIN"
  echo "Please set LLVM_BUILD_DIR or CLANG_BIN environment variable"
  exit 1
fi

if [[ ! -f "$OPT_BIN" ]]; then
  echo "Error: opt not found at $OPT_BIN"
  echo "Please set LLVM_BUILD_DIR or OPT_BIN environment variable"
  exit 1
fi

# ==============================
# Main analysis
# ==============================
BASENAME=$(basename "$INPUT_FILE")
NAME="${BASENAME%.*}"

echo "Processing: $INPUT_FILE"

# Step 1: Compile to LLVM IR
IR_FILE="$OUTPUT_DIR/$NAME.ll"

if [[ "$INPUT_FILE" == *.cpp || "$INPUT_FILE" == *.cc ]]; then
  "$CLANG_BIN" $CFLAGS -S -emit-llvm -x c++ "$INPUT_FILE" -o "$IR_FILE" 2>/dev/null
else
  "$CLANG_BIN" $CFLAGS -S -emit-llvm "$INPUT_FILE" -o "$IR_FILE" 2>/dev/null
fi

# Step 2: Run dataflow analyses
# Determine which analyses to run
RUN_LIVE=false
RUN_REACH=false
RUN_AVAIL=false
RUN_ANTIC=false

if [[ "$ANALYSES" == "all" ]]; then
  RUN_LIVE=true
  RUN_REACH=true
  RUN_AVAIL=true
  RUN_ANTIC=true
else
  [[ "$ANALYSES" == *"live"* ]] && RUN_LIVE=true
  [[ "$ANALYSES" == *"reach"* ]] && RUN_REACH=true
  [[ "$ANALYSES" == *"avail"* ]] && RUN_AVAIL=true
  [[ "$ANALYSES" == *"antic"* ]] && RUN_ANTIC=true
fi

# Live Variables Analysis
if [[ "$RUN_LIVE" == true ]]; then
  LIVE_OUT="$OUTPUT_DIR/live-variables.md"
  {
    echo "# Live Variables Analysis Results"
    echo ""
    echo "- **Test:** \`$BASENAME\`"
    echo "- **File:** \`$INPUT_FILE\`"
    echo "- **Generated:** $(date '+%Y-%m-%d %H:%M:%S')"
    echo ""
    echo "---"
    echo ""
    echo "## Source Code"
    echo ""
    echo '```cpp'
    cat "$INPUT_FILE"
    echo ""
    echo '```'
    echo ""
    echo "---"
    echo ""
    echo "## Analysis Output"
    echo ""
    echo '```'
    "$OPT_BIN" -passes="custom-dataflow" -df-live -disable-output "$IR_FILE" 2>&1 || echo "Analysis completed with warnings"
    echo '```'
  } > "$LIVE_OUT"
fi

# Reaching Definitions Analysis
if [[ "$RUN_REACH" == true ]]; then
  REACH_OUT="$OUTPUT_DIR/reaching-definitions.md"
  {
    echo "# Reaching Definitions Analysis Results"
    echo ""
    echo "- **Test:** \`$BASENAME\`"
    echo "- **File:** \`$INPUT_FILE\`"
    echo "- **Generated:** $(date '+%Y-%m-%d %H:%M:%S')"
    echo ""
    echo "---"
    echo ""
    echo "## Source Code"
    echo ""
    echo '```cpp'
    cat "$INPUT_FILE"
    echo ""
    echo '```'
    echo ""
    echo "---"
    echo ""
    echo "## Analysis Output"
    echo ""
    echo '```'
    "$OPT_BIN" -passes="custom-dataflow" -df-reach -disable-output "$IR_FILE" 2>&1 || echo "Analysis completed with warnings"
    echo '```'
  } > "$REACH_OUT"
fi

# Available Expressions Analysis
if [[ "$RUN_AVAIL" == true ]]; then
  AVAIL_OUT="$OUTPUT_DIR/available-expressions.md"
  {
    echo "# Available Expressions Analysis Results"
    echo ""
    echo "- **Test:** \`$BASENAME\`"
    echo "- **File:** \`$INPUT_FILE\`"
    echo "- **Generated:** $(date '+%Y-%m-%d %H:%M:%S')"
    echo ""
    echo "---"
    echo ""
    echo "## Source Code"
    echo ""
    echo '```cpp'
    cat "$INPUT_FILE"
    echo ""
    echo '```'
    echo ""
    echo "---"
    echo ""
    echo "## Analysis Output"
    echo ""
    echo '```'
    "$OPT_BIN" -passes="custom-dataflow" -df-avail -disable-output "$IR_FILE" 2>&1 || echo "Analysis completed with warnings"
    echo '```'
  } > "$AVAIL_OUT"
fi

# Anticipable Expressions Analysis
if [[ "$RUN_ANTIC" == true ]]; then
  ANTIC_OUT="$OUTPUT_DIR/anticipable-expressions.md"
  {
    echo "# Anticipable Expressions Analysis Results"
    echo ""
    echo "- **Test:** \`$BASENAME\`"
    echo "- **File:** \`$INPUT_FILE\`"
    echo "- **Generated:** $(date '+%Y-%m-%d %H:%M:%S')"
    echo ""
    echo "---"
    echo ""
    echo "## Source Code"
    echo ""
    echo '```cpp'
    cat "$INPUT_FILE"
    echo ""
    echo '```'
    echo ""
    echo "---"
    echo ""
    echo "## Analysis Output"
    echo ""
    echo '```'
    "$OPT_BIN" -passes="custom-dataflow" -df-antic -disable-output "$IR_FILE" 2>&1 || echo "Analysis completed with warnings"
    echo '```'
  } > "$ANTIC_OUT"
fi

# Step 3: Create summary
SUMMARY="$OUTPUT_DIR/README.md"
{
  echo "# Dataflow Analysis Results: $BASENAME"
  echo ""
  echo "- **Source File:** \`$BASENAME\`"
  echo "- **Full Path:** \`$INPUT_FILE\`"
  echo "- **Output Directory:** \`$OUTPUT_DIR\`"
  echo "- **Generated:** $(date '+%Y-%m-%d %H:%M:%S')"
  echo ""
  echo "## Files"
  echo ""
  echo "- **LLVM IR:** [\`$NAME.ll\`](./$NAME.ll)"
  echo ""
  echo "## Analysis Results"
  echo ""
  [[ "$RUN_LIVE" == true ]] && echo "- [Live Variables Analysis](./live-variables.md)"
  [[ "$RUN_REACH" == true ]] && echo "- [Reaching Definitions Analysis](./reaching-definitions.md)"
  [[ "$RUN_AVAIL" == true ]] && echo "- [Available Expressions Analysis](./available-expressions.md)"
  [[ "$RUN_ANTIC" == true ]] && echo "- [Anticipable Expressions Analysis](./anticipable-expressions.md)"
} > "$SUMMARY"

echo "✓ Complete → $OUTPUT_DIR"
