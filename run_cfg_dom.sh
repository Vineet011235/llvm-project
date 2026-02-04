#!/usr/bin/env bash
set -euo pipefail

# ==============================
# Defaults (can be overridden)
# ==============================
LLVM_BUILD_DIR="${LLVM_BUILD_DIR:-$(pwd)/build}"
CLANG_BIN="${CLANG_BIN:-$LLVM_BUILD_DIR/bin/clang}"
OPT_BIN="${OPT_BIN:-$LLVM_BUILD_DIR/bin/opt}"

MODE="dom"                       # cfg | dom
OUT_DIR="./pass-results"       # Output directory for IR and DOT files
INPUT_SRC=""
CFLAGS="-O1 -g" 
# If O0 is used, the IR will have 'optnone' which prevents CFG emission. O1 or higher is recommended for testing.
PASS_NAME="cfg-dom-analysis"
DEFAULT_TEST_FILE="example.c"  # Default test file if none provided

# ==============================
# Usage
# ==============================
usage() {
  echo "Usage: $0 [options]"
  echo
  echo "Options:"
  echo "  -m <mode>        Mode: cfg | dom   (default: cfg)"
  echo "  -o <out_dir>     Output directory (default: ./pass-results)"
  echo "  -i <file.c/.cpp> Input C/C++ file (default: ./example.c)"
  echo
  echo "Environment overrides:"
  echo "  LLVM_BUILD_DIR   Path to llvm-project/build"
  echo "  CLANG_BIN        Path to clang"
  echo "  OPT_BIN          Path to opt"
  exit 1
}

# ==============================
# Parse arguments
# ==============================
while getopts "m:o:i:h" opt; do
  case "$opt" in
    m) MODE="$OPTARG" ;;
    o) OUT_DIR="$OPTARG" ;;
    i) INPUT_SRC="$OPTARG" ;;
    h) usage ;;
    *) usage ;;
  esac
done

if [[ -z "$INPUT_SRC" ]]; then
  INPUT_SRC="$DEFAULT_TEST_FILE"
fi

if [[ "$MODE" != "cfg" && "$MODE" != "dom" ]]; then
  echo "Error: mode must be 'cfg' or 'dom'"
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

CFG_FLAGS="-cfg-dot-folder=$OUT_DIR/dot"
DOM_ENABLE_FLAG="--emit-dom-tree"  # Flag to enable dominator tree emission in the pass

if [[ "$MODE" == "cfg" ]]; then
  "$OPT_BIN" \
    -passes="$PASS_NAME" \
    $CFG_FLAGS \
    -disable-output \
    "$IR_FILE"
else
  # Placeholder for future dominator extension
  "$OPT_BIN" \
    -passes="$PASS_NAME" \
    $CFG_FLAGS \
    -disable-output \
    $DOM_ENABLE_FLAG \
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
