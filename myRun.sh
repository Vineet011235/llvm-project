#!/usr/bin/env bash
set -euo pipefail

# ==============================
# Defaults (can be overridden)
# ==============================
LLVM_BUILD_DIR="${LLVM_BUILD_DIR:-$(pwd)/build}"
CLANG_BIN="${CLANG_BIN:-$LLVM_BUILD_DIR/bin/clang}"
OPT_BIN="${OPT_BIN:-$LLVM_BUILD_DIR/bin/opt}"

# Flags (independent options, can be combined)
SHOW_CFG=false
SHOW_DOM_TREE=false
SHOW_VIEW_DOM=false
SHOW_VERIFY=false
USER_SPECIFIED_FLAGS=false       # Track if user specified any flags

OUT_DIR=""                       # Output directory for IR and DOT files (auto-generated if empty)
OUT_DIR_EXPLICIT=false           # Track if user specified output directory
INPUT_SRC=""
CFLAGS="-O0 -Xclang -disable-O0-optnone -g" 
# Using O0 with optnone disabled to preserve unreachable code while allowing pass execution
PASS_NAME="cfg-dom-analysis"
DEFAULT_TEST_FILE="example.c"    # Default test file if none provided

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
  echo "  -c               Show CFG (control flow graph)"
  echo "  -d               Show dominator tree visualization"
  echo "  -v               Generate detailed dominators text file (view-dom)"
  echo "  -V               Verify against LLVM's implementation"
  echo "  -o <dir>         Output directory (default: <input_dir>/<input_name>_results)"
  echo "  -h               Show this help message"
  echo
  echo "Default: If no flags specified, enables -c -d -v (all except verify)"
  echo "Flags can be combined: -cd (CFG + DOM tree), -cdv (all displays), -cV (CFG + verify)"
  echo
  echo "Output: Creates ir/, dot/, svg/, and txt/ subdirectories with analysis results."
  exit 0
}

# ==============================
# Parse arguments
# ==============================
while getopts "cdvVo:i:h" opt; do
  case "$opt" in
    c) SHOW_CFG=true; USER_SPECIFIED_FLAGS=true ;;
    d) SHOW_DOM_TREE=true; USER_SPECIFIED_FLAGS=true ;;
    v) SHOW_VIEW_DOM=true; USER_SPECIFIED_FLAGS=true ;;
    V) SHOW_VERIFY=true; USER_SPECIFIED_FLAGS=true ;;
    o) OUT_DIR="$OPTARG"; OUT_DIR_EXPLICIT=true ;;
    i) INPUT_SRC="$OPTARG" ;;
    h) usage ;;
    *) usage ;;
  esac
done

# If no flags specified, enable all except verify by default
if [[ "$USER_SPECIFIED_FLAGS" == false ]]; then
  SHOW_CFG=true
  SHOW_DOM_TREE=true
  SHOW_VIEW_DOM=true
  SHOW_VERIFY=false
fi

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
mkdir -p "$OUT_DIR"/{ir,dot,svg,txt}

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
ENABLED_FLAGS=""
[[ "$SHOW_CFG" == true ]] && ENABLED_FLAGS="${ENABLED_FLAGS}cfg "
[[ "$SHOW_DOM_TREE" == true ]] && ENABLED_FLAGS="${ENABLED_FLAGS}dom-tree "
[[ "$SHOW_VIEW_DOM" == true ]] && ENABLED_FLAGS="${ENABLED_FLAGS}view-dom "
[[ "$SHOW_VERIFY" == true ]] && ENABLED_FLAGS="${ENABLED_FLAGS}verify "

echo "[2/3] Running CFGDomAnalysis pass with: $ENABLED_FLAGS"

# Build pass arguments
PASS_ARGS="-passes=$PASS_NAME"
NEED_SEPARATE_VIEWDOM_RUN=false

# Determine result directory based on what's enabled
if [[ "$SHOW_CFG" == true ]] || [[ "$SHOW_DOM_TREE" == true ]] || [[ "$SHOW_VERIFY" == true ]]; then
  RESULT_DIR_FLAG="--result-dir=$OUT_DIR/dot"
  PASS_ARGS="$PASS_ARGS $RESULT_DIR_FLAG"
fi

# Add flags based on what's enabled
if [[ "$SHOW_CFG" == true ]]; then
  PASS_ARGS="$PASS_ARGS --show-cfg"
fi

if [[ "$SHOW_DOM_TREE" == true ]]; then
  PASS_ARGS="$PASS_ARGS --show-dom-tree"
fi

if [[ "$SHOW_VIEW_DOM" == true ]]; then
  mkdir -p "$OUT_DIR/txt"
  RESULT_DIR_TXT="--result-dir=$OUT_DIR/txt"
  # Override result-dir for dominators text output
  # If other flags are set, they use dot dir; dominators use txt dir
  if [[ "$SHOW_CFG" == true ]] || [[ "$SHOW_DOM_TREE" == true ]] || [[ "$SHOW_VERIFY" == true ]]; then
    # Need to run dominators separately to use txt dir
    PASS_ARGS_VIEWDOM="-passes=$PASS_NAME $RESULT_DIR_TXT --show-dominators"
    NEED_SEPARATE_VIEWDOM_RUN=true
  else
    # Only view-dom, use txt dir directly
    PASS_ARGS="$PASS_ARGS --show-dominators $RESULT_DIR_TXT"
    NEED_SEPARATE_VIEWDOM_RUN=false
  fi
fi

if [[ "$SHOW_VERIFY" == true ]]; then
  PASS_ARGS="$PASS_ARGS --show-report"
  echo ""
  echo "=== Verifying Dominator Tree ==="
fi

# Run the pass
if [[ "$SHOW_VERIFY" == true ]]; then
  $OPT_BIN $PASS_ARGS -disable-output "$IR_FILE" 2>&1 | tee "$OUT_DIR/verification_results.txt"
else
  $OPT_BIN $PASS_ARGS -disable-output "$IR_FILE"
fi

# Run separate pass for view-dom if needed (when combined with other flags)
if [[ "${NEED_SEPARATE_VIEWDOM_RUN:-false}" == true ]]; then
  $OPT_BIN $PASS_ARGS_VIEWDOM -disable-output "$IR_FILE"
fi

if [[ "$SHOW_VIEW_DOM" == true ]]; then
  echo ""
  echo "Dominator analysis text files generated in: $OUT_DIR/txt"
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
if [[ "$SHOW_CFG" == true ]] || [[ "$SHOW_DOM_TREE" == true ]] || [[ "$SHOW_VERIFY" == true ]]; then
  echo "DOT files:  $OUT_DIR/dot"
  echo "SVG files:  $OUT_DIR/svg"
fi
if [[ "$SHOW_VIEW_DOM" == true ]]; then
  echo "TXT files:  $OUT_DIR/txt"
fi

