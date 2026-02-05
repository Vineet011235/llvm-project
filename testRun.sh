#!/usr/bin/env bash
set -euo pipefail

# ==============================
# Configuration
# ==============================
LLVM_BUILD_DIR="${LLVM_BUILD_DIR:-$(pwd)/build}"
CLANG_BIN="${CLANG_BIN:-$LLVM_BUILD_DIR/bin/clang}"
OPT_BIN="${OPT_BIN:-$LLVM_BUILD_DIR/bin/opt}"

# Flags (independent options, can be combined)
SHOW_CFG=false
SHOW_DOM_TREE=false
SHOW_VIEW_DOM=false
SHOW_VERIFY=false
USER_SPECIFIED_FLAGS=false

TESTING_DIR="./testing"
TEST_DIR="$TESTING_DIR/test"
RESULTS_DIR="$TESTING_DIR/results"
CFLAGS="-O0 -Xclang -disable-O0-optnone -g"
PASS_NAME="cfg-dom-analysis"

# ==============================
# Usage
# ==============================
usage() {
  echo "Usage: $0 [options]"
  echo
  echo "Batch testing of CFG/DOM analysis on all test files in testing/test/"
  echo
  echo "Options:"
  echo "  -c               Show CFG (control flow graph) for all tests"
  echo "  -d               Show dominator tree visualization for all tests"
  echo "  -v               Generate detailed dominators text file for all tests"
  echo "  -V               Verify all tests against LLVM's implementation"
  echo "  -h               Show this help message"
  echo
  echo "Default: If no flags specified, enables -c -d -v (all except verify)"
  echo "Flags can be combined: -cd (CFG + DOM tree), -cdv (all displays), -cV (CFG + verify)"
  echo
  echo "Output: Results saved to ./testing/results/"
  exit 0
}

# ==============================
# Parse arguments
# ==============================
while getopts "cdvVh" opt; do
  case "$opt" in
    c) SHOW_CFG=true; USER_SPECIFIED_FLAGS=true ;;
    d) SHOW_DOM_TREE=true; USER_SPECIFIED_FLAGS=true ;;
    v) SHOW_VIEW_DOM=true; USER_SPECIFIED_FLAGS=true ;;
    V) SHOW_VERIFY=true; USER_SPECIFIED_FLAGS=true ;;
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

# ==============================
# Check and build if needed
# ==============================
check_and_build() {
  local need_build=false

  if [[ ! -d "$LLVM_BUILD_DIR" ]] || \
     [[ ! -f "$CLANG_BIN" ]] || \
     [[ ! -f "$OPT_BIN" ]]; then
    need_build=true
  elif [[ -f "llvm/include/llvm/Analysis/CFGDomAnalysis.h" ]] && \
       [[ "llvm/include/llvm/Analysis/CFGDomAnalysis.h" -nt "$OPT_BIN" ]]; then
    need_build=true
  elif [[ -f "llvm/lib/Analysis/CFGDomAnalysis.cpp" ]] && \
       [[ "llvm/lib/Analysis/CFGDomAnalysis.cpp" -nt "$OPT_BIN" ]]; then
    need_build=true
  fi

  if [[ "$need_build" == true ]]; then
    echo "========================================="
    echo "Building LLVM (source files changed)..."
    echo "========================================="
    
    if [[ ! -d "$LLVM_BUILD_DIR" ]]; then
      mkdir -p "$LLVM_BUILD_DIR"
      cmake -S llvm -B "$LLVM_BUILD_DIR" \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DLLVM_ENABLE_PROJECTS="clang" \
        -DLLVM_TARGETS_TO_BUILD="X86" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON >/dev/null 2>&1
    fi
    
    if ! cmake --build "$LLVM_BUILD_DIR" --target opt clang -j$(nproc); then
      echo "Error: Build failed" >&2
      exit 1
    fi
    echo "Build completed!"
    echo
  else
    echo "[Info] Binaries are up-to-date, skipping build"
    echo
  fi
}

check_and_build

# ==============================
# Verify test directory exists
# ==============================
if [[ ! -d "$TEST_DIR" ]]; then
  echo "Error: Test directory '$TEST_DIR' not found" >&2
  exit 1
fi

# ==============================
# Process each test file
# ==============================
# Build enabled flags display string
ENABLED_FLAGS=""
[[ "$SHOW_CFG" == true ]] && ENABLED_FLAGS="${ENABLED_FLAGS}cfg "
[[ "$SHOW_DOM_TREE" == true ]] && ENABLED_FLAGS="${ENABLED_FLAGS}dom-tree "
[[ "$SHOW_VIEW_DOM" == true ]] && ENABLED_FLAGS="${ENABLED_FLAGS}view-dom "
[[ "$SHOW_VERIFY" == true ]] && ENABLED_FLAGS="${ENABLED_FLAGS}verify "

# Find all .c and .cpp files recursively
mapfile -t TEST_FILES < <(find "$TEST_DIR" -type f \( -name "*.c" -o -name "*.cpp" \) | sort)

if [[ ${#TEST_FILES[@]} -eq 0 ]]; then
  echo "Error: No test files found in '$TEST_DIR'" >&2
  exit 1
fi

echo "========================================="
echo "Running CFG/DOM Analysis with: $ENABLED_FLAGS"
echo "Test files: ${#TEST_FILES[@]} (recursive scan)"
echo "=========================================="
echo

TOTAL_FILES=${#TEST_FILES[@]}
CURRENT=0
SUCCESS_COUNT=0
FAIL_COUNT=0

for test_file in "${TEST_FILES[@]}"; do
  ((CURRENT+=1))
  BASENAME=$(basename "$test_file")
  NAME="${BASENAME%.*}"
  
  # Calculate relative path from TEST_DIR to maintain directory structure
  REL_PATH=$(dirname "${test_file#$TEST_DIR/}")


  # Handle case where file is directly in TEST_DIR (REL_PATH would be ".")
  if [[ "$REL_PATH" == "." ]]; then
    OUT_DIR="$RESULTS_DIR/$NAME"
    DISPLAY_NAME="$NAME"
  else
    OUT_DIR="$RESULTS_DIR/$REL_PATH/$NAME"
    DISPLAY_NAME="$REL_PATH/$NAME"
  fi
  
  # Create result directories maintaining structure
  mkdir -p "$OUT_DIR"/{ir,dot,svg,txt}
  
  IR_FILE="$OUT_DIR/ir/$NAME.ll"
  
  # Step 1: Compile to LLVM IR
  echo -ne "\\r[${CURRENT}/${TOTAL_FILES}] Testing: $DISPLAY_NAME...                                        "
  COMPILE_OUTPUT=$("$CLANG_BIN" $CFLAGS -S -emit-llvm "$test_file" -o "$IR_FILE" 2>&1)
  COMPILE_EXIT=$?
  if [[ $COMPILE_EXIT -ne 0 ]] || echo "$COMPILE_OUTPUT" | grep -qi "error"; then
    ((FAIL_COUNT+=1))
    continue
  fi
  if [[ ! -f "$IR_FILE" ]]; then
    ((FAIL_COUNT+=1))
    continue
  fi
  
  # Step 2: Run pass
  PASS_ARGS="-passes=$PASS_NAME"
  
  # Determine result directory and add flags
  if [[ "$SHOW_CFG" == true ]] || [[ "$SHOW_DOM_TREE" == true ]] || [[ "$SHOW_VERIFY" == true ]]; then
    RESULT_DIR_FLAG="--result-dir=$OUT_DIR/dot"
    PASS_ARGS="$PASS_ARGS $RESULT_DIR_FLAG"
  fi
  
  # Add individual flags
  [[ "$SHOW_CFG" == true ]] && PASS_ARGS="$PASS_ARGS --show-cfg"
  [[ "$SHOW_DOM_TREE" == true ]] && PASS_ARGS="$PASS_ARGS --show-dom-tree"
  [[ "$SHOW_VERIFY" == true ]] && PASS_ARGS="$PASS_ARGS --show-report"
  
  # Handle view-dom flag
  NEED_SEPARATE_VIEWDOM_RUN=false
  if [[ "$SHOW_VIEW_DOM" == true ]]; then
    if [[ "$SHOW_CFG" == true ]] || [[ "$SHOW_DOM_TREE" == true ]] || [[ "$SHOW_VERIFY" == true ]]; then
      # Will run separately with txt result dir
      NEED_SEPARATE_VIEWDOM_RUN=true
    else
      # Only view-dom, add to main pass with txt dir
      RESULT_DIR_TXT="--result-dir=$OUT_DIR/txt"
      PASS_ARGS="$PASS_ARGS --show-dominators $RESULT_DIR_TXT"
    fi
  fi
  
  # Run the main pass with cfg/dom-tree/verify flags
  PASS_OUTPUT=$($OPT_BIN $PASS_ARGS -disable-output "$IR_FILE" 2>&1)
  PASS_EXIT=$?
  if [[ $PASS_EXIT -ne 0 ]]; then
    ((FAIL_COUNT+=1))
    continue
  fi
  
  # Run separate pass for view-dom if needed (when combined with other flags)
  if [[ "$NEED_SEPARATE_VIEWDOM_RUN" == true ]]; then
    RESULT_DIR_TXT="--result-dir=$OUT_DIR/txt"
    PASS_ARGS_VIEWDOM="-passes=$PASS_NAME $RESULT_DIR_TXT --show-dominators"
    PASS_OUTPUT=$($OPT_BIN $PASS_ARGS_VIEWDOM -disable-output "$IR_FILE" 2>&1)
    PASS_EXIT=$?
    if [[ $PASS_EXIT -ne 0 ]]; then
      ((FAIL_COUNT+=1))
      continue
    fi
  fi
  
  # Step 3: Generate SVG visualizations
  if command -v dot >/dev/null 2>&1; then
    shopt -s nullglob
    DOT_FILES=("$OUT_DIR"/dot/*.dot)
    shopt -u nullglob
    
    if [[ ${#DOT_FILES[@]} -gt 0 ]]; then
      for dot_file in "${DOT_FILES[@]}"; do
        DOT_BASENAME=$(basename "$dot_file" .dot)
        dot -Tsvg "$dot_file" -o "$OUT_DIR/svg/${DOT_BASENAME}.svg" 2>/dev/null
      done
    fi
  fi
  
  ((SUCCESS_COUNT+=1))
done

echo -e "\\r[${TOTAL_FILES}/${TOTAL_FILES}] Complete                                                      "
echo
echo "=========================================="
echo "Testing Complete"
echo "Total files tested: $TOTAL_FILES"
echo "Successful: $SUCCESS_COUNT"
echo "Failed: $FAIL_COUNT"
echo "Results directory: $RESULTS_DIR"
echo "========================================="
# Display verification summary if verify is enabled
if [[ "$SHOW_VERIFY" == true ]]; then
  echo
  python3 << 'PYEOF'
import re
from pathlib import Path

results_dir = Path("./testing/results")
reports = sorted(results_dir.rglob("VERIFICATION_REPORT.md"))

total_passed = 0
total_failed = 0
total_functions = 0
all_passed = True

for report in reports:
    content = report.read_text()
    passed = len(re.findall(r'Status.*PASSED', content))
    failed = len(re.findall(r'Status.*FAILED', content))
    
    if passed + failed > 0:
        total_passed += passed
        total_failed += failed
        total_functions += passed + failed
        if failed > 0:
            all_passed = False

if total_functions > 0:
    print("=" * 60)
    print("      DOMINATOR TREE VERIFICATION SUMMARY")
    print("=" * 60)
    print(f"Total Functions Verified:  {total_functions}")
    print(f"Functions Passed:          {total_passed} ({100*total_passed//total_functions}%)")
    print(f"Functions Failed:          {total_failed}")
    print("-" * 60)
    if all_passed:
        print("✅ All functions match LLVM's DominatorTree implementation")
    else:
        print("⚠️  Some functions have mismatches - check reports for details")
    print("=" * 60)
PYEOF
fi
