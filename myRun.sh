#!/usr/bin/env bash
set -euo pipefail

# ==============================
# Defaults (can be overridden)
# ==============================
LLVM_BUILD_DIR="${LLVM_BUILD_DIR:-$(pwd)/build}"
CLANG_BIN="${CLANG_BIN:-$LLVM_BUILD_DIR/bin/clang}"
OPT_BIN="${OPT_BIN:-$LLVM_BUILD_DIR/bin/opt}"

MODE="verify"                       # cfg | dom | verify
OUT_DIR=""                     # Output directory for IR and DOT files (auto-generated if empty)
OUT_DIR_EXPLICIT=false           # Track if user specified output directory
INPUT_SRC=""
CFLAGS="-O0 -Xclang -disable-O0-optnone -g" 
# Using O0 with optnone disabled to preserve unreachable code while allowing pass execution
PASS_NAME="cfg-dom-analysis"
DEFAULT_TEST_FILE="example.c"  # Default test file if none provided

# ==============================
# Usage
# ==============================
usage() {
  echo "Usage: $0 [options]"
  echo
  echo "Compiles C/C++ to LLVM IR, runs CFG/DOM analysis, and generates visualizations."
  echo
  echo "Options:"
  echo "  -i <file>        Input C/C++ source file (default: example.c)"
  echo "  -m <mode>        Analysis mode: 'cfg' (control flow), 'dom' (dominators), or 'verify' (verify against LLVM) (default: dom)"
  echo "  -o <dir>         Output directory (default: <input_dir>/<input_name>_results)"
  echo "  -h               Show this help message"
  echo
  echo "Output: Creates ir/, dot/, and svg/ subdirectories with analysis results."
  exit 0
}

# ==============================
# Parse arguments
# ==============================
while getopts "m:o:i:h" opt; do
  case "$opt" in
    m) MODE="$OPTARG" ;;
    o) OUT_DIR="$OPTARG"; OUT_DIR_EXPLICIT=true ;;
    i) INPUT_SRC="$OPTARG" ;;
    h) usage ;;
    *) usage ;;
  esac
done

if [[ -z "$INPUT_SRC" ]]; then
  INPUT_SRC="$DEFAULT_TEST_FILE"
fi

# Auto-generate output directory name based on input file if not specified
if [[ "$OUT_DIR_EXPLICIT" == false ]]; then
  INPUT_DIR=$(dirname "$INPUT_SRC")
  BASENAME=$(basename "$INPUT_SRC")
  NAME="${BASENAME%.*}"
  OUT_DIR="${INPUT_DIR}/${NAME}_results"
fi

if [[ "$MODE" != "cfg" && "$MODE" != "dom" && "$MODE" != "verify" ]]; then
  echo "Error: mode must be 'cfg', 'dom', or 'verify'"
  exit 1
fi

# ==============================
# Check and build LLVM if needed
# ==============================
check_and_build() {
  local need_build=false
  local reason=""

  # Check if build directory exists
  if [[ ! -d "$LLVM_BUILD_DIR" ]]; then
    need_build=true
    reason="Build directory not found"
  # Check if clang binary exists
  elif [[ ! -f "$CLANG_BIN" ]]; then
    need_build=true
    reason="clang binary not found"
  # Check if opt binary exists
  elif [[ ! -f "$OPT_BIN" ]]; then
    need_build=true
    reason="opt binary not found"
  # Check if CFGDomAnalysis.h is newer than opt binary (outdated check)
  elif [[ -f "llvm/include/llvm/Analysis/CFGDomAnalysis.h" ]] && \
       [[ "llvm/include/llvm/Analysis/CFGDomAnalysis.h" -nt "$OPT_BIN" ]]; then
    need_build=true
    reason="Source files newer than binaries"
  # Check if the pass implementation is newer
  elif [[ -f "llvm/lib/Analysis/CFGDomAnalysis.cpp" ]] && \
       [[ "llvm/lib/Analysis/CFGDomAnalysis.cpp" -nt "$OPT_BIN" ]]; then
    need_build=true
    reason="Source files newer than binaries"
  fi

  if [[ "$need_build" == true ]]; then
    echo "========================================="
    echo "Build required: $reason"
    echo "========================================="
    
    # Create build directory if it doesn't exist
    if [[ ! -d "$LLVM_BUILD_DIR" ]]; then
      echo "Creating build directory..."
      mkdir -p "$LLVM_BUILD_DIR"
      
      echo "Running CMake configuration..."
      cmake -S llvm -B "$LLVM_BUILD_DIR" \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DLLVM_ENABLE_PROJECTS="clang" \
        -DLLVM_TARGETS_TO_BUILD="X86" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    fi
    
    echo "Building LLVM (this may take a while)..."
    cmake --build "$LLVM_BUILD_DIR" --target opt clang -j$(nproc)
    
    echo "Build completed successfully!"
    echo "========================================="
    echo
  else
    echo "[Info] Build is up-to-date, skipping build step"
  fi
}

check_and_build

# ==============================
# Setup output layout
# ==============================
mkdir -p "$OUT_DIR"/{ir,dot,svg}

BASENAME=$(basename "$INPUT_SRC")
NAME="${BASENAME%.*}"

IR_FILE="$OUT_DIR/ir/$NAME.ll"

# ==============================
# Step 1: Compile to LLVM IR
# ==============================
echo "[1/3] Compiling $INPUT_SRC → LLVM IR"
"$CLANG_BIN" $CFLAGS -S -emit-llvm "$INPUT_SRC" -o "$IR_FILE"

# ==============================
# Step 2: Run CFG pass
# ==============================
echo "[2/3] Running CFGDomAnalysis pass preferences: mode=$MODE"

# New CLI flags
RESULT_DIR_FLAG="--result-dir=$OUT_DIR/dot"
SHOW_CFG_FLAG="--show-cfg"
SHOW_DOM_TREE_FLAG="--show-dom-tree"
SHOW_REPORT_FLAG="--show-report"

if [[ "$MODE" == "cfg" ]]; then
  "$OPT_BIN" \
    -passes="$PASS_NAME" \
    $RESULT_DIR_FLAG \
    $SHOW_CFG_FLAG \
    -disable-output \
    "$IR_FILE"
elif [[ "$MODE" == "verify" ]]; then
  echo "\n=== Verifying Dominator Tree ==="
  "$OPT_BIN" \
    -passes="$PASS_NAME" \
    $RESULT_DIR_FLAG \
    $SHOW_CFG_FLAG \
    $SHOW_DOM_TREE_FLAG \
    $SHOW_REPORT_FLAG \
    -disable-output \
    "$IR_FILE" 2>&1 | tee "$OUT_DIR/verification_results.txt"
else
  # dom mode
  "$OPT_BIN" \
    -passes="$PASS_NAME" \
    $RESULT_DIR_FLAG \
    $SHOW_CFG_FLAG \
    $SHOW_DOM_TREE_FLAG \
    -disable-output \
    "$IR_FILE"
fi

# ==============================
# Step 3: Visualization (optional)
# ==============================
if command -v dot >/dev/null 2>&1; then
  echo "[3/3] Generating SVGs from DOT files"
  shopt -s nullglob
  DOT_FILES=("$OUT_DIR"/dot/*.dot)
  shopt -u nullglob
  if [[ ${#DOT_FILES[@]} -eq 0 ]]; then
    echo "No DOT files generated (no functions or CFG emission disabled)"
  else
    for f in "${DOT_FILES[@]}"; do
      BASENAME=$(basename "$f" .dot)
      dot -Tsvg "$f" -o "$OUT_DIR/svg/${BASENAME}.svg"
    done
  fi
else
  echo "[3/3] graphviz not found; skipping SVG generation"
fi

echo
echo "Done."
echo "IR files:   $OUT_DIR/ir"
echo "DOT files:  $OUT_DIR/dot"
echo "SVG files:  $OUT_DIR/svg"
