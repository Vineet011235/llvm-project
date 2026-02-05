# CFG and Dominator Tree Analysis Pass

## 0. Setup & Patch Application

### Prerequisites
- LLVM Project repository: `git clone https://github.com/llvm/llvm-project.git`
- Files: `assignment.patch` and `patch.sh`

### Applying the Patch

```bash
# 1. Navigate to llvm-project root
cd path/to/llvm-project/

# 2. Copy patch files to root directory
cp /path/to/assignment.patch .
cp /path/to/patch.sh .

# 3. Run patch script
chmod +x patch.sh
./patch.sh
```

The script checks out commit `a34e89fcb5f0`, validates and applies the patch. After completion, use `myRun.sh` and `testRun.sh` scripts for analysis (see Usage section below).

## 1. Usage

### **Pass**: `cfg-dom-analysis`

**CLI Options**:
- `--result-dir=<dir>` - Output directory
- `--show-cfg` - Generate CFG DOT files
- `--show-dom-tree` - Generate dominator tree DOT files
- `--show-dominators` - Generate detailed dominators text file (pre/post-dominators)
- `--show-report` - Generate verification report vs LLVM

### **Scripts**:

**myRun.sh** - Single file analysis:
```bash
./myRun.sh [options]

Options:
  -i <file>    Input C/C++ source file (default: example.c)
  -c           Show CFG (control flow graph)
  -d           Show dominator tree visualization
  -v           Generate detailed dominators text file (view-dom)
  -V           Verify against LLVM's implementation
  -o <dir>     Output directory (default: <input_name>_results)
  -h           Show help message

Default: If no flags specified, runs -c -d -v (all visualizations, no verify)
Flags can be combined: -cd, -cdv, -cV, -cdvV
```

**testRun.sh** - Batch testing:
```bash
./testRun.sh [options]

Options:
  -c           Show CFG for all tests
  -d           Show dominator tree for all tests
  -v           Generate dominators text files for all tests
  -V           Verify all tests against LLVM
  -h           Show help message

Default: If no flags specified, runs -c -d -v (all visualizations)
Results saved to: ./testing/results/
```

**Examples**:
```bash
# Basic usage with default example.c (generates CFG, DOM tree, and text)
./myRun.sh

# Analyze custom file with verification
./myRun.sh -i mycode.c -V

# Only CFG visualization
./myRun.sh -c -i program.c

# Full analysis with custom output directory
./myRun.sh -cdvV -i test.c -o ./my_output

# Run all tests with verification
./testRun.sh -V

# Run tests with only CFG and DOM tree
./testRun.sh -cd
```

## 2. Implementation

### CFG Construction
- Custom `MyBasicBlock` wraps LLVM BasicBlock with terminators, successors/predecessors, and dominator sets
- Edges extracted from terminator instructions (Branch, Switch, Return)

### Pre-Dominator Algorithm
1. Compute Reverse Post-Order via DFS
2. Initialize: Entry = {self}, others = all nodes
3. Iterate: Dom(n) = {n} ∪ (∩ Dom(pred))
4. Calculate immediate dominators (IDom)

### Post-Dominator Algorithm
1. Create virtual exit node connected to all return/unreachable blocks
2. Initialize: Exit = {self}, others = all nodes
3. Iterate: PostDom(n) = {n} ∪ (∩ PostDom(succ))
4. Calculate immediate post-dominators (IPostDom)

**Key Difference**: Post-dominators use successors (backward traversal), pre-dominators use predecessors

## 3. Corner Cases

| Case | Solution |
|------|----------|
| Multiple exits | Virtual exit node aggregates all returns |
| Infinite loops | Blocks only post-dominate themselves |
| Unreachable blocks | Empty dominator sets, nullptr IDom, isolated in DOM tree |
| Early returns | Each treated as exit, connects to virtual exit |
| Self-domination | Every block dominates/post-dominates itself |
| Entry/Exit | Dominated/post-dominated only by themselves, no IDom/IPostDom |

## 4. Verification & Testing

- **100% match** with LLVM's DominatorTree
- Test cases: standard tests (`testing/test/`), unreachable blocks (`unreachable_tests/`), post-dominators (`test_postdom.c`)
- Verify mode generates markdown report with per-function match/mismatch analysis

## 5. Technical Details

- **Complexity**: O(N × E) time, O(N²) space
- **Convergence**: 2-3 iterations typical
- **Algorithm**: Cooper-Harvey-Kennedy dominator algorithm
- **CFG**: Color-coded nodes (entry, exits, branches), labeled edges
- **DOM Tree**: Tree layout with rounded boxes

## 6. Summary

Complete from-scratch CFG and dominator analysis (pre/post) with:
- ✅ Custom CFG without LLVM APIs
- ✅ RPO-based pre-dominator calculation  
- ✅ Virtual exit post-dominator calculation
- ✅ All corner cases handled (unreachable, infinite loops, multiple exits)
- ✅ DOT/SVG visualizations + detailed text output
- ✅ 100% LLVM verification match
- ✅ Four analysis modes (cfg, dom-tree, view-dom, verify)


