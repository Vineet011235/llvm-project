# Assignment Report: CFG and Dominator Tree Analysis Pass

## 1. Pass Usage and CLI Options

**Pass Name**: `cfg-dom-analysis`

**Command Line Options**:
- `--result-dir=<dir>` - Output directory (must exist)
- `--show-cfg` - Generate CFG DOT files
- `--show-dom-tree` - Generate dominator tree DOT files
- `--show-report` - Generate verification report (vs LLVM's implementation)

**Usage**:
```bash
opt -passes=cfg-dom-analysis --result-dir=./output --show-cfg --show-dom-tree --show-report -disable-output input.ll
```

**Note**: CFG and DOMTree are always constructed; flags control output generation only.

---

## 2. Scripts Usage

### `myRun.sh` - Single File Analysis
Analyzes a single C/C++ source file with detailed output.

```bash
./myRun.sh -i <file> -m <cfg|dom|verify> -o <output_dir>
```

Modes: `cfg` (CFG only), `dom` (CFG + DOM tree), `verify` (all + verification report)

### `testRun.sh` - Batch Testing
Runs analysis on all test files in `testing/test/` recursively.

```bash
./testRun.sh -m <cfg|dom|verify>
```

**Output Structure**: `<output_dir>/{ir/, dot/, svg/}` plus `VERIFICATION_REPORT.md` (verify mode)

**Verification Summary**: In verify mode, automatically displays consolidated results showing total functions verified, pass/fail counts, and match percentage against LLVM's DominatorTree

---

## 3. Code Logic and Implementation

### 3.1 CFG Construction - Custom BasicBlock Wrapper

Custom `MyBasicBlock` class wraps LLVM's BasicBlock, storing:
- Reference to original BasicBlock
- Unique name and instruction list
- Terminator instruction
- Successor/predecessor lists
- Dominator sets and immediate dominator

**Rationale**: Enables from-scratch dominator analysis without LLVM's built-in APIs.

### 3.2 Edge Construction via Terminator Instructions

Edges created by examining terminator types:
- **BranchInst**: Edges to all successor blocks
- **SwitchInst**: Edges to default case + all case labels
- **ReturnInst**: No successors

### 3.3 Predecessors and Successors

Forward edges populate successor lists; backward edges populate predecessor lists for bidirectional traversal.

### 3.4 Dominator Computation using Reverse Post-Order

**Iterative dataflow algorithm with RPO**:
1. Compute Reverse Post-Order via DFS traversal
2. Initialize: Entry dominates itself; others dominated by all nodes
3. Iterate until convergence: Dom(n) = {n} ∪ (∩ Dom(p) for all predecessors p)

**RPO Advantage**: Near-topological ordering ensures faster convergence.

### 3.5 Immediate Dominator and DOM Tree Construction

**IDom Calculation**: For each node, the immediate dominator is the dominator with the largest dominator set (closest to the node).

**DOM Tree**: Adjacency list where each block is a child of its immediate dominator, forming parent-child domination relationships.

---

## 4. Handling Unreachable Code

### Problem
Unreachable blocks cannot be reached from entry via any control flow path (dead code, eliminated branches, hand-written IR).

### Implementation

**Reachability**: DFS from entry identifies reachable blocks  
**CFG**: All blocks included; edges only between reachable blocks; unreachable blocks as isolated nodes  
**Dominators**: Only reachable blocks get initialized dominator sets; unreachable get empty sets  
**IDom**: Reachable blocks computed normally; unreachable blocks get `nullptr`  
**DOM Tree**: Only non-null IDoms create parent edges; unreachable blocks isolated

### Visualization

**CFG**: Unreachable blocks shown as isolated nodes (no edges)  
**DOM Tree**: Unreachable blocks shown without parent connections

### Verification
100% match with LLVM's DominatorTree for all test cases including unreachable blocks.

### Test Cases
See `unreachable_tests/` directory with hand-written IR (`test_unreachable.ll`). C/C++ code doesn't produce unreachable IR blocks (Clang eliminates at AST level).

---

## Summary

Complete from-scratch CFG and dominator tree analysis demonstrating compiler optimization algorithms with verification against LLVM's implementation.

**Key Features**:
- ✅ Custom CFG without LLVM's built-in APIs
- ✅ Dominator calculation with RPO iteration
- ✅ Correct unreachable block handling
- ✅ DOT/SVG visualizations
- ✅ 100% LLVM verification match

