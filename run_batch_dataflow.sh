#!/usr/bin/env bash
set -euo pipefail

# ==============================
# Batch Dataflow Analysis Runner
# ==============================
# This script recursively finds all C/C++ files in a directory and runs
# dataflow analysis on each using the individual runner script.

# ==============================
# Defaults
# ==============================
LLVM_BUILD_DIR="${LLVM_BUILD_DIR:-$(pwd)/build}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SINGLE_RUNNER="$SCRIPT_DIR/run_single_dataflow.sh"

INPUT_DIR=""
OUTPUT_DIR="./test-result"  # Default centralized location
ANALYSES="all"  # all | live | reach | avail | antic
BUILD_IF_NEEDED=true

# ==============================
# Usage
# ==============================
usage() {
  cat << EOF
Usage: $0 -d <directory> [options]

Required:
  -d <dir>         Input directory (processes all .c/.cc/.cpp files recursively)

Options:
  -o <dir>         Output base directory (default: ./test-result)
  -a <analyses>    Analyses to run: all|live|reach|avail|antic (default: all)
  --no-build       Skip automatic build check/build step
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
  $0 -d test                              # Process all files in test/ directory
  $0 -d test/simple -a live               # Run only live analysis
  $0 -d test -o ./my-results              # Custom output location
  $0 -d test --no-build                   # Skip build check
  
Output Structure:
  Each file will have its results stored in a subdirectory maintaining
  the relative path structure from the input directory.
  
  Example:
    Input: test/simple/if-else.cc
    Output: ./test-result/test/simple/if-else-dataflow/
    
A master index file will be created at: <output-dir>/INDEX.md
EOF
  exit 1
}

# ==============================
# Parse arguments
# ==============================
while [[ $# -gt 0 ]]; do
  case "$1" in
    -d)
      INPUT_DIR="$2"
      shift 2
      ;;
    -o)
      OUTPUT_DIR="$2"
      shift 2
      ;;
    -a)
      ANALYSES="$2"
      shift 2
      ;;
    --no-build)
      BUILD_IF_NEEDED=false
      shift
      ;;
    -h|--help)
      usage
      ;;
    *)
      echo "Error: Unknown option: $1"
      usage
      ;;
  esac
done

if [[ -z "$INPUT_DIR" ]]; then
  echo "Error: Input directory (-d) is required"
  usage
fi

if [[ ! -d "$INPUT_DIR" ]]; then
  echo "Error: Directory not found: $INPUT_DIR"
  exit 1
fi

# ==============================
# Check dependencies
# ==============================
if [[ ! -f "$SINGLE_RUNNER" ]]; then
  echo "Error: Single file runner not found at: $SINGLE_RUNNER"
  echo "Expected: run_single_dataflow.sh in the same directory as this script"
  exit 1
fi

if [[ ! -x "$SINGLE_RUNNER" ]]; then
  echo "Making single runner executable..."
  chmod +x "$SINGLE_RUNNER"
fi

# ==============================
# Check and build LLVM if needed
# ==============================
check_and_build() {
  local need_build=false
  local reason=""
  
  CLANG_BIN="${CLANG_BIN:-$LLVM_BUILD_DIR/bin/clang}"
  OPT_BIN="${OPT_BIN:-$LLVM_BUILD_DIR/bin/opt}"

  if [[ ! -d "$LLVM_BUILD_DIR" ]]; then
    need_build=true
    reason="Build directory not found"
  elif [[ ! -f "$CLANG_BIN" ]]; then
    need_build=true
    reason="clang binary not found"
  elif [[ ! -f "$OPT_BIN" ]]; then
    need_build=true
    reason="opt binary not found"
  elif [[ -f "llvm/lib/Analysis/CustomDataFlow.cpp" ]] && \
       [[ "llvm/lib/Analysis/CustomDataFlow.cpp" -nt "$OPT_BIN" ]]; then
    need_build=true
    reason="CustomDataFlow.cpp is newer than binaries"
  fi

  if [[ "$need_build" == true ]]; then
    echo "========================================="
    echo "Build required: $reason"
    echo "========================================="
    
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

# ==============================
# Main execution
# ==============================
echo "Batch processing: $INPUT_DIR → $OUTPUT_DIR"

# Build check
if [[ "$BUILD_IF_NEEDED" == true ]]; then
  check_and_build
fi

# Find all C/C++ files recursively
mapfile -t FILES < <(find "$INPUT_DIR" -type f \( -name "*.c" -o -name "*.cc" -o -name "*.cpp" \) | sort)

if [[ ${#FILES[@]} -eq 0 ]]; then
  echo "Error: No C/C++ files found in $INPUT_DIR"
  exit 1
fi

echo "Found ${#FILES[@]} files"

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Process each file
PROCESSED=0
FAILED=0
declare -a PROCESSED_FILES
declare -a FAILED_FILES

for file in "${FILES[@]}"; do
  # Get relative path from current directory
  REL_PATH=$(realpath --relative-to=. "$file")
  FILE_DIR=$(dirname "$REL_PATH")
  FILE_BASENAME=$(basename "$file")
  FILE_NAME="${FILE_BASENAME%.*}"
  
  # Create output directory maintaining structure
  FILE_OUTPUT="$OUTPUT_DIR/$FILE_DIR/${FILE_NAME}-dataflow"
  
  # Run the single file analysis
  if "$SINGLE_RUNNER" -i "$file" -o "$FILE_OUTPUT" -a "$ANALYSES" 2>/dev/null; then
    PROCESSED=$((PROCESSED + 1))
    PROCESSED_FILES+=("$file|$FILE_OUTPUT")
  else
    echo "✗ Failed: $file"
    FAILED=$((FAILED + 1))
    FAILED_FILES+=("$file")
  fi
done

# Create master index
MASTER_INDEX="$OUTPUT_DIR/INDEX.md"
{
  echo "# Batch Dataflow Analysis Results"
  echo ""
  echo "- **Project Root:** \`$(pwd)\`"
  echo "- **Input Directory:** \`$INPUT_DIR\`"
  echo "- **Output Directory:** \`$OUTPUT_DIR\`"
  echo "- **Analyses:** $ANALYSES"
  echo "- **Generated:** $(date '+%Y-%m-%d %H:%M:%S')"
  echo ""
  echo "## Summary"
  echo ""
  echo "- **Total Files:** ${#FILES[@]}"
  echo "- **Processed Successfully:** $PROCESSED"
  echo "- **Failed:** $FAILED"
  echo ""
  
  if [[ $PROCESSED -gt 0 ]]; then
    echo "## Processed Files"
    echo ""
    for entry in "${PROCESSED_FILES[@]}"; do
      IFS='|' read -r file output <<< "$entry"
      REL_FILE=$(realpath --relative-to=. "$file")
      REL_OUTPUT=$(realpath --relative-to="$OUTPUT_DIR" "$output")
      echo "- [\`$REL_FILE\`](./$REL_OUTPUT/README.md)"
    done
    echo ""
  fi
  
  if [[ $FAILED -gt 0 ]]; then
    echo "## Failed Files"
    echo ""
    for file in "${FAILED_FILES[@]}"; do
      REL_FILE=$(realpath --relative-to=. "$file")
      echo "- \`$REL_FILE\`"
    done
    echo ""
  fi
  
  echo "---"
  echo ""
  echo "*Generated by run_batch_dataflow.sh*"
} > "$MASTER_INDEX"

echo ""
echo "✓ Processed $PROCESSED/${#FILES[@]} files → $OUTPUT_DIR/INDEX.md"

[[ $FAILED -gt 0 ]] && echo "⚠ $FAILED failed" && exit 1
exit 0
