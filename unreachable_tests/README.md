# Unreachable Code Test Cases

Test cases for verifying CFG and DOMTree handling of unreachable basic blocks in LLVM IR.

## Why Direct LLVM IR Testing?

**Clang eliminates dead code at the AST level before generating IR.** Unreachable C/C++ code never becomes LLVM IR blocks. We use hand-written LLVM IR to test our implementation where unreachable blocks actually exist.

## Test Files

**test_unreachable.ll** - Hand-written IR with unreachable blocks:
- `test_with_unreachable`: One unreachable block after conditionals
- `another_test`: Two unreachable blocks with no predecessors

## Running Tests

```bash
# From repository root - modes: cfg, dom, verify
./myRun.sh -i unreachable_tests/test_unreachable.ll -m verify -o unreachable_tests/results
```

## Expected Behavior

**CFG**: All blocks visible; unreachable blocks as isolated nodes (no edges)  
**DOM Tree**: Unreachable blocks as isolated nodes (no parent edges); IDom = `nullptr`  
**Verification**: 100% PASS match against LLVM's DominatorTree

## Results Structure

```
test_unreachable_results/
├── ir/                    # Processed IR
├── dot/                   # CFG (*.dot) and DOM tree (*_dom.dot)
├── svg/                   # Visualizations
└── VERIFICATION_REPORT.md # Detailed results
```

## Implementation

1. **Reachability**: DFS from entry identifies reachable blocks
2. **Dominators**: Only reachable blocks get non-empty dominator sets
3. **IDom**: Unreachable blocks have `nullptr`
4. **Visualization**: Unreachable blocks shown without edges

This ensures correctness while maintaining dead code visibility for analysis.
