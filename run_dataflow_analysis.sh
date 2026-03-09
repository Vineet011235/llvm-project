#!/usr/bin/env bash
set -euo pipefail

# ==============================
# Dataflow Analysis Test Runner
# ==============================
# This script compiles C/C++ files to LLVM IR and runs various dataflow analyses:
# - Live Variables Analysis
# - Reaching Definitions Analysis  
# - Available Expressions Analysis
# - Anticipable Expressions Analysis

# ==============================
# Defaults (can be overridden)
# ==============================
LLVM_BUILD_DIR="${LLVM_BUILD_DIR:-$(pwd)/build}"
CLANG_BIN="${CLANG_BIN:-$LLVM_BUILD_DIR/bin/clang}"
OPT_BIN="${OPT_BIN:-$LLVM_BUILD_DIR/bin/opt}"

OUT_DIR="./test-result"  # Centralized results directory
INPUT_SRC=""
INPUT_DIR=""
CFLAGS="-O0 -g"
ANALYSES="all"  # all | live | reach | avail | antic

# ==============================
# Usage
# ==============================
usage() {
  cat << EOF
Usage: $0 [options]

Options:
  -i <file>        Input C/C++ file
  -d <dir>         Input directory (recursively processes all .c/.cc/.cpp files)
  -a <analyses>    Analyses to run: all|live|reach|avail|antic (default: all)
  -h               Show this help

Analyses:
  live   - Live Variables Analysis (-df-live)
  reach  - Reaching Definitions Analysis (-df-reach)
  avail  - Available Expressions Analysis (-df-avail)
  antic  - Anticipable Expressions Analysis (-df-antic)
  all    - Run all analyses

Environment overrides:
  LLVM_BUILD_DIR   Path to llvm-project/build
  CLANG_BIN        Path to clang
  OPT_BIN          Path to opt

Examples:
  $0 -i test/simple/if-else/if-else.cc       # Run all analyses on specific file
  $0 -d test/simple -a live                  # Run live analysis recursively on all files in dir
  $0 -i example.c -a "live reach"            # Run specific analyses
  $0 -d test                                 # Run all analyses on all files in test/ (recursive)
  
Output:
  All results are stored in: ./test-result/dataflow/
  Organized by relative path from project root
EOF
  exit 1
}

# ==============================
# Parse arguments
# ==============================
while getopts "i:d:a:h" opt; do
  case "$opt" in
    i) INPUT_SRC="$OPTARG" ;;
    d) INPUT_DIR="$OPTARG" ;;
    a) ANALYSES="$OPTARG" ;;
    h) usage ;;
    *) usage ;;
  esac
done

if [[ -z "$INPUT_SRC" && -z "$INPUT_DIR" ]]; then
  echo "Error: Must specify either -i <file> or -d <directory>"
  usage
fi

# ==============================
# Check and build LLVM if needed
# ==============================
check_and_build() {
  local need_build=false
  local reason=""

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
# Run dataflow analysis on a file
# ==============================
run_analysis_on_file() {
  local src_file="$1"
  local basename=$(basename "$src_file")
  local name="${basename%.*}"
  
  # Convert absolute path to relative for better organization
  local rel_path=$(realpath --relative-to=. "$src_file")
  local rel_dir=$(dirname "$rel_path")
  
  echo ""
  echo "========================================="
  echo "Processing: $rel_path"
  echo "========================================="
  
  # Create organized directory structure in central location
  local results_dir="$OUT_DIR/$rel_dir/$name"
  mkdir -p "$results_dir"
  
  local ir_file="$results_dir/$name.ll"
  
  # Step 1: Compile to LLVM IR
  echo "[1/3] Compiling to LLVM IR..."
  if [[ "$src_file" == *.cpp || "$src_file" == *.cc ]]; then
    "$CLANG_BIN" $CFLAGS -S -emit-llvm -x c++ "$src_file" -o "$ir_file"
  else
    "$CLANG_BIN" $CFLAGS -S -emit-llvm "$src_file" -o "$ir_file"
  fi
  
  # Step 2: Run dataflow analyses
  echo "[2/3] Running dataflow analyses..."
  
  local run_live=false run_reach=false run_avail=false run_antic=false
  
  if [[ "$ANALYSES" == "all" ]]; then
    run_live=true run_reach=true run_avail=true run_antic=true
  else
    [[ "$ANALYSES" == *"live"* ]] && run_live=true
    [[ "$ANALYSES" == *"reach"* ]] && run_reach=true
    [[ "$ANALYSES" == *"avail"* ]] && run_avail=true
    [[ "$ANALYSES" == *"antic"* ]] && run_antic=true
  fi
  
  # Live Variables Analysis
  if [[ "$run_live" == true ]]; then
    echo "  → Running Live Variables Analysis..."
    local live_out="$results_dir/live-variables.md"
    {
      echo "# Live Variables Analysis Results"
      echo ""
      echo "- **Test:** \`$basename\`"
      echo "- **Location:** \`$rel_path\`"
      echo "- **Generated:** $(date '+%Y-%m-%d %H:%M:%S')"
      echo ""
      echo "---"
      echo ""
      echo "## Source Code"
      echo ""
      echo '```cpp'
      cat "$src_file"
      echo '```'
      echo ""
      echo "---"
      echo ""
      echo "## Analysis Output"
      echo ""
      echo '```'
      "$OPT_BIN" -passes="custom-dataflow" -df-live -disable-output "$ir_file" 2>&1
      echo '```'
    } > "$live_out"
    echo "     ✓ Results saved to $live_out"
  fi
  
  # Reaching Definitions Analysis
  if [[ "$run_reach" == true ]]; then
    echo "  → Running Reaching Definitions Analysis..."
    local reach_out="$results_dir/reaching-definitions.md"
    {
      echo "# Reaching Definitions Analysis Results"
      echo ""
      echo "- **Test:** \`$basename\`"
      echo "- **Location:** \`$rel_path\`"
      echo "- **Generated:** $(date '+%Y-%m-%d %H:%M:%S')"
      echo ""
      echo "---"
      echo ""
      echo "## Source Code"
      echo ""
      echo '```cpp'
      cat "$src_file"
      echo '```'
      echo ""
      echo "---"
      echo ""
      echo "## Analysis Output"
      echo ""
      echo '```'
      "$OPT_BIN" -passes="custom-dataflow" -df-reach -disable-output "$ir_file" 2>&1
      echo '```'
    } > "$reach_out"
    echo "     ✓ Results saved to $reach_out"
  fi
  
  # Available Expressions Analysis
  if [[ "$run_avail" == true ]]; then
    echo "  → Running Available Expressions Analysis..."
    local avail_out="$results_dir/available-expressions.md"
    {
      echo "# Available Expressions Analysis Results"
      echo ""
      echo "- **Test:** \`$basename\`"
      echo "- **Location:** \`$rel_path\`"
      echo "- **Generated:** $(date '+%Y-%m-%d %H:%M:%S')"
      echo ""
      echo "---"
      echo ""
      echo "## Source Code"
      echo ""
      echo '```cpp'
      cat "$src_file"
      echo '```'
      echo ""
      echo "---"
      echo ""
      echo "## Analysis Output"
      echo ""
      echo '```'
      "$OPT_BIN" -passes="custom-dataflow" -df-avail -disable-output "$ir_file" 2>&1
      echo '```'
    } > "$avail_out"
    echo "     ✓ Results saved to $avail_out"
  fi
  
  # Anticipable Expressions Analysis
  if [[ "$run_antic" == true ]]; then
    echo "  → Running Anticipable Expressions Analysis..."
    local antic_out="$results_dir/anticipable-expressions.md"
    {
      echo "# Anticipable Expressions Analysis Results"
      echo ""
      echo "- **Test:** \`$basename\`"
      echo "- **Location:** \`$rel_path\`"
      echo "- **Generated:** $(date '+%Y-%m-%d %H:%M:%S')"
      echo ""
      echo "---"
      echo ""
      echo "## Source Code"
      echo ""
      echo '```cpp'
      cat "$src_file"
      echo '```'
      echo ""
      echo "---"
      echo ""
      echo "## Analysis Output"
      echo ""
      echo '```'
      "$OPT_BIN" -passes="custom-dataflow" -df-antic -disable-output "$ir_file" 2>&1
      echo '```'
    } > "$antic_out"
    echo "     ✓ Results saved to $antic_out"
  fi
  
  # Step 3: Create summary index
  echo "[3/3] Creating summary..."
  local summary="$results_dir/README.md"
  {
    echo "# Dataflow Analysis Results: $basename"
    echo ""
    echo "- **Source File:** \`$basename\`"
    echo "- **Full Path:** \`$rel_path\`"
    echo "- **Generated:** $(date '+%Y-%m-%d %H:%M:%S')"
    echo ""
    echo "## Files"
    echo ""
    echo "- **LLVM IR:** [\`$name.ll\`](./$name.ll)"
    echo ""
    echo "## Analysis Results"
    echo ""
    [[ "$run_live" == true ]] && echo "- [Live Variables Analysis](./live-variables.md)"
    [[ "$run_reach" == true ]] && echo "- [Reaching Definitions Analysis](./reaching-definitions.md)"
    [[ "$run_avail" == true ]] && echo "- [Available Expressions Analysis](./available-expressions.md)"
    [[ "$run_antic" == true ]] && echo "- [Anticipable Expressions Analysis](./anticipable-expressions.md)"
  } > "$summary"
  
  echo "✓ Processing complete for $basename"
  echo "   Results: $results_dir/"
}

# ==============================
# Main execution
# ==============================
check_and_build

echo ""
echo "========================================="
echo "Dataflow Analysis Test Runner"
echo "========================================="
echo "Output Directory: $OUT_DIR"
echo "Analyses: $ANALYSES"
echo ""

# Process files
if [[ -n "$INPUT_SRC" ]]; then
  # Single file mode
  if [[ ! -f "$INPUT_SRC" ]]; then
    echo "Error: File not found: $INPUT_SRC"
    exit 1
  fi
  run_analysis_on_file "$INPUT_SRC"
  
  # Create simple index for single file
  echo ""
  echo "Creating index..."
  MASTER_INDEX="$OUT_DIR/INDEX.md"
  {
    echo "# Dataflow Analysis Results"
    echo ""
    echo "- **Project Root:** \`$(pwd)\`"
    echo "- **Analyses:** $ANALYSES"
    echo "- **Generated:** $(date '+%Y-%m-%d %H:%M:%S')"
    echo ""
    echo "## Test Files"
    echo ""
    FILE_BASENAME=$(basename "$INPUT_SRC")
    REL_PATH=$(realpath --relative-to=. "$INPUT_SRC")
    REL_DIR=$(dirname "$REL_PATH")
    FILE_NAME="${FILE_BASENAME%.*}"
    echo "- [$REL_PATH](./$REL_DIR/$FILE_NAME/README.md)"
  } > "$MASTER_INDEX"
  echo "Index: $MASTER_INDEX"
else
  # Directory mode
  if [[ ! -d "$INPUT_DIR" ]]; then
    echo "Error: Directory not found: $INPUT_DIR"
    exit 1
  fi
  
  # Find all C/C++ files recursively
  echo "Searching for C/C++ files in $INPUT_DIR (recursively)..."
  mapfile -t FILES < <(find "$INPUT_DIR" -type f \( -name "*.c" -o -name "*.cc" -o -name "*.cpp" \) | sort)
  
  if [[ ${#FILES[@]} -eq 0 ]]; then
    echo "No C/C++ files found in $INPUT_DIR (searched recursively)"
    exit 1
  fi
  
  echo "Found ${#FILES[@]} file(s) to process"
  
  for file in "${FILES[@]}"; do
    run_analysis_on_file "$file"
  done
  
  # Create master index for directory
  echo ""
  echo "Creating master index..."
  MASTER_INDEX="$OUT_DIR/INDEX.md"
  {
    echo "# Dataflow Analysis Results"
    echo ""
    echo "- **Project Root:** \`$(pwd)\`"
    echo "- **Analyses:** $ANALYSES"
    echo "- **Generated:** $(date '+%Y-%m-%d %H:%M:%S')"
    echo ""
    echo "## Test Files"
    echo ""
    for file in "${FILES[@]}"; do
      FILE_BASENAME=$(basename "$file")
      REL_PATH=$(realpath --relative-to=. "$file")
      REL_DIR=$(dirname "$REL_PATH")
      FILE_NAME="${FILE_BASENAME%.*}"
      echo "- [$REL_PATH](./$REL_DIR/$FILE_NAME/README.md)"
    done
  } > "$MASTER_INDEX"
  echo "Master index: $MASTER_INDEX"
fi

echo ""
echo "========================================="
echo "All Done!"
echo "========================================="
echo "Results location: $OUT_DIR"
echo ""
echo "View master index:"
echo "  cat $OUT_DIR/INDEX.md"
echo ""
