# Block-Centric Interval Analysis Pass - Detailed Implementation Plan

## 1. Objective and Scope

Implement an LLVM Function pass that performs forward abstract interpretation over integer values and prints final per-basic-block `OUT` states as interval tables.

This plan is aligned with your SRS/HLD and the existing assignment workflow (`custom-dataflow` style integration + shell runners). It is organized into execution stages with explicit exit criteria.

---

## 2. Non-Negotiable Requirements (Assignment Contract)

1. Architecture must be block-centric with explicit `IN` and `OUT` state per `BasicBlock`.
2. Analysis must be forward dataflow.
3. Merge at CFG joins must use interval join:
   - `[a,b] join [c,d] = [min(a,c), max(b,d)]`
4. Loop headers must apply widening during fixpoint iteration.
5. One post-fixpoint narrowing pass over loop headers must be run.
6. Final output must print block-wise `OUT` state in table form.
7. Values equivalent to bottom/unreachable must not be printed.
8. Keep existing debug logging behavior (do not remove existing debug logging statements).
9. Integrate with LLVM pass pipeline and existing assignment scripts.

---

## 3. Target File Touchpoints

Expected implementation/edit locations:

1. `llvm/include/llvm/Analysis/CustomIntervalAnalysis.h` ( Create new and attach with pass manager )
2. `llvm/lib/Analysis/CustomIntervalAnalysis.cpp` ( create )
3. `llvm/lib/Analysis/CMakeLists.txt` (only if not already wired)
4. `llvm/lib/Passes/PassBuilder.cpp` (only if not already wired)
5. `llvm/lib/Passes/PassRegistry.def` (only if not already wired)
6. `run_single_dataflow.sh` (if adding interval mode flag)
7. `run_batch_dataflow.sh` (if adding interval mode flag)
8. `DATAFLOW_README.md` (if assignment expects doc update)

---

## 4. Detailed Stage Plan

## Stage 0 - Baseline and Branch Hygiene

### Goal
Confirm clean starting point and preserve reproducibility.

### Tasks
1. Verify current branch and status.
2. Build once before changes to ensure baseline health.
3. Run existing pass mode once using current scripts.
4. Capture baseline output artifact path.

### Exit Criteria
1. Baseline build succeeds.
2. Baseline runner scripts execute successfully.

---

## Stage 1 - Formalize Abstract Domain Types

### Goal
Introduce robust interval and per-block state data structures.

### Tasks
1. Add an `Interval` type in `CustomIntervalAnalysis.cpp` (or header if needed):
   - `LowerBound` and `UpperBound` with infinity support.
   - Helper constructors: `top()`, `constant(v)`, `bottom-marker` behavior via map absence.
2. Add interval utilities:
   - `isTop()`, `isConstant()`, `toString()`, equality.
   - `join(Interval, Interval)`.
   - arithmetic transfer helpers for `add`, `sub`, optional `mul`.
3. Define aliases:
   - `using BlockState = DenseMap<Value*, Interval>`
   - `DenseMap<BasicBlock*, BlockState> INMap, OUTMap`
4. Define safe bound arithmetic behavior for infinities.

### Implementation Notes
1. Use map-absence as bottom to match your SRS.
2. Do not force-populate every value in every state; keep sparse representation.
3. Ensure deterministic printing order by collecting keys in a sortable container at print time.

### Exit Criteria
1. Compiles with no warnings in modified files.
2. Unit-level helper checks via debug prints show correct interval formatting and joins.

---

## Stage 2 - Domain Discovery and Initialization

### Goal
Build initial analysis universe and initialize entry conditions.

### Tasks
1. Discover all integer-typed values in function:
   - Function arguments with integer type.
   - Integer-producing instructions.
2. Seed `IN[Entry]` with arguments as top (`[-INF, INF]`).
3. Keep other maps empty initially (implicit bottom).
4. Add helper:
   - `Interval getValueInterval(Value *V, const BlockState &State)`
   - Return constant interval for `ConstantInt`.
   - Return stored state for tracked values.
   - Return top for unknown external integer values only when semantically required.

### Exit Criteria
1. Entry arguments appear as top in entry processing logs.
2. No crash on constants, globals, or unsupported operand classes.

---

## Stage 3 - Transfer Function (Intra-Block Semantics)

### Goal
Implement instruction-wise forward state update.

### Tasks
1. Implement `BlockState transferBlock(BasicBlock *BB, const BlockState &In)`:
   - Start with `Current = In`.
   - Iterate instructions in lexical order.
   - For each supported arithmetic op, compute result interval from operand intervals.
   - Write result interval to `Current[&I]`.
2. Supported instructions (minimum):
   - `add`, `sub` on integer scalars.
3. Recommended support (if time permits and assignment allows):
   - `mul` with min/max over bound products.
   - `sext/zext/trunc` conservative conversions.
   - `phi` handling by normal block merge (avoid re-merging in transfer unless needed).
4. Unsupported integer-producing instructions:
   - Assign top conservatively.
5. Non-integer instructions:
   - Ignore.

### Exit Criteria
1. Straight-line test function yields expected constant intervals.
2. Unsupported operations never produce unsound narrow intervals.

---

## Stage 4 - CFG Meet and Worklist Engine (RPO)

### Goal
Implement convergent forward analysis loop with efficient ordering.

### Tasks
1. Build RPO order of function basic blocks.
2. Initialize worklist as `SetVector<BasicBlock*>` in RPO.
3. Main loop:
   - Pop block.
   - Compute `NewIn` by joining all predecessor `OUT` states.
   - For entry block, include seeded entry state.
   - Apply widening if block is loop header.
   - Compute `NewOut = transferBlock(BB, NewIn)`.
   - If `NewOut != OUT[BB]`, update and enqueue successors.
4. Use `LoopInfo` query for loop-header detection.

### Edge Cases
1. Entry with zero predecessors.
2. Unreachable blocks remain empty and should not print data rows.
3. Blocks with one predecessor can fast-path copy/merge.

### Exit Criteria
1. Analysis reaches fixpoint on loop and non-loop CFGs.
2. Iteration count remains reasonable on provided tests.

---

## Stage 5 - Widening at Loop Headers

### Goal
Guarantee termination for monotonic growth loops.

### Tasks
1. Implement `widenInterval(oldI, newI)`:
   - Lower: if `new.low < old.low` then `-INF` else `old.low`.
   - Upper: if `new.high > old.high` then `+INF` else `old.high`.
2. Implement `widenState(OldIn, NewIn)` key-wise over tracked values.
3. Apply widening only when current block is a loop header and old `IN` exists.
4. Add debug logs for each widened variable under `LLVM_DEBUG`.

### Exit Criteria
1. Induction-variable loop no longer requires unbounded iterations.
2. Widening events are visible with debug flag.

---

## Stage 6 - Narrowing Phase

### Goal
Recover precision lost by widening.

### Tasks
1. After worklist fixpoint, perform one narrowing sweep over loop headers.
2. Recompute merged `IN` using normal join (no widening).
3. Recompute `OUT` via transfer.
4. Update states if improved precision and optionally propagate once to successors.
5. Keep narrowing bounded (one pass as per your design).

### Exit Criteria
1. Some previously infinite bounds reduce to finite where safe.
2. No oscillation introduced.

---

## Stage 7 - Output Formatting (Grading-Safe)

### Goal
Emit exactly the required final report format.

### Tasks
1. Iterate basic blocks in function order.
2. Print section header:
   - `### Basic Block: %<name>`
3. Print markdown table header exactly once per block.
4. Print only variables present in `OUT[BB]` (omit bottom/unreachable).
5. Print intervals as:
   - `[-INF, INF]`, `[c, c]`, `[l, u]`
6. Preserve existing extra logging behavior separately via `LLVM_DEBUG`.

### Determinism Rules
1. Sort rows by stable key:
   - Prefer named values first by name.
   - Then unnamed values by printed IR operand string.
2. Ensure no nondeterministic `DenseMap` traversal in output.

### Exit Criteria
1. Output matches required structure on sample functions.
2. Repeated runs produce identical output ordering.

---

## Stage 8 - CLI and Mode Wiring

### Goal
Expose interval analysis mode in the existing custom-dataflow framework.

### Tasks
1. Add `cl::opt<bool>` switch in `CustomIntervalAnalysis.cpp` (for example `-df-interval`).
2. Extend dispatcher in `run(Function&, FAM)` to call interval routine.
3. If scripts enumerate analysis names, add interval option:
   - `run_single_dataflow.sh`
   - `run_batch_dataflow.sh`
4. Keep backward compatibility with existing modes.

### Exit Criteria
1. `opt -passes="custom-dataflow" -df-interval` executes correctly.
2. Script wrappers can run interval mode via their `-a` argument.

---

## Stage 9 - Debug and Observability

### Goal
Keep deep diagnostics without polluting grading output.

### Tasks
1. Ensure `#define DEBUG_TYPE "interval-analysis"` (or compatible existing debug type).
2. Add `LLVM_DEBUG` traces for:
   - Block processing start.
   - `IN` and `OUT` snapshots.
   - Widening and narrowing changes.
   - Worklist re-enqueue events.
3. Keep standard pass output minimal and strictly assignment-compliant.

### Exit Criteria
1. `-debug-only=interval-analysis` shows detailed evolution.
2. Normal run remains clean and grader-safe.

---

## Stage 10 - Validation Matrix

### Goal
Prove correctness, soundness, and formatting compliance.

### Required Test Classes
1. Straight-line constants.
2. Branch merge with different bounds.
3. Single loop increasing induction variable.
4. Nested loops.
5. Unreachable block.
6. Mixed supported/unsupported integer ops.
7. Function with integer arguments only.
8. Function with no integer values.

### For Each Test, Verify
1. Soundness: inferred interval over-approximates concrete values.
2. Termination: fixpoint reached quickly.
3. Widening observed on loops.
4. Narrowing improves at least one case where applicable.
5. Final markdown table structure is exactly valid.

### Exit Criteria
1. All test classes pass manually reviewed expectations.
2. No crashes or assertions in debug/release builds.

---

## Stage 11 - Performance and Robustness Hardening

### Goal
Avoid pathological slowdowns and fragile behavior.

### Tasks
1. Avoid full-state deep copies where possible (move/swap when safe).
2. Minimize repeated operand-to-interval lookups via local refs.
3. Keep sparse states; do not materialize top for every value globally.
4. Verify no infinite re-enqueue loops due to unstable comparisons.
5. Add guard for very large integer constants if using fixed-width arithmetic.

### Exit Criteria
1. Batch runner completes in acceptable time.
2. No memory blow-up on medium-sized test corpus.

---

## Stage 12 - Final Submission Pack

### Goal
Prepare final deliverables with reproducibility.

### Deliverables
1. Updated pass source/header.
2. Any required script updates.
3. Test outputs for representative cases.
4. Short report section:
   - Domain/lattice.
   - Transfer rules.
   - Widening/narrowing policy.
   - Complexity and limitations.

### Final Pre-Submission Checklist
1. Build clean from scratch.
2. Interval mode runs from both direct `opt` and runner scripts.
3. Output format exactly matches assignment expectation.
4. Existing analyses remain unaffected.
5. Debug logs preserved and gated behind debug flag.

---

## 5. Pseudocode Blueprint (Drop-In Reference)

```cpp
runIntervalAnalysis(Function &F, FunctionAnalysisManager &FAM) {
  auto &LI = FAM.getResult<LoopAnalysis>(F);

  DenseMap<BasicBlock*, BlockState> IN, OUT;
  SmallVector<BasicBlock*, 16> RPO = computeRPO(F);
  SetVector<BasicBlock*> Worklist;
  for (BasicBlock *BB : RPO) Worklist.insert(BB);

  seedEntryArgumentsTop(F, IN);

  while (!Worklist.empty()) {
    BasicBlock *BB = Worklist.pop_back_val();

    BlockState NewIn = mergePredecessorOutStates(BB, OUT, IN, F);
    if (isLoopHeader(BB, LI))
      NewIn = widenState(IN[BB], NewIn);

    IN[BB] = NewIn;
    BlockState NewOut = transferBlock(BB, IN[BB]);

    if (NewOut != OUT[BB]) {
      OUT[BB] = std::move(NewOut);
      for (BasicBlock *Succ : successors(BB))
        Worklist.insert(Succ);
    }
  }

  runSingleNarrowingSweepOnHeaders(F, LI, IN, OUT);
  printPerBlockOutTables(F, OUT);
}
```

---

## 6. Known Conservative Choices and Limits

1. No path-sensitive branch condition refinement unless explicitly implemented.
2. Unsupported integer instructions are mapped to top for soundness.
3. Pointer and floating-point values are out of scope.
4. One-pass narrowing balances precision and runtime.

---

## 7. Suggested Day-by-Day Execution (Practical Timeline)

1. Day 1: Stages 1-2 (domain + init), compile-stable.
2. Day 2: Stages 3-4 (transfer + worklist), non-loop correctness.
3. Day 3: Stages 5-6 (widen + narrow), loop correctness.
4. Day 4: Stages 7-9 (output + CLI + debug), script integration.
5. Day 5: Stages 10-12 (validation, hardening, submission pack).

---

## 8. Fast Command Checklist

```bash
# Configure/build (example)
cmake -S llvm -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build opt clang

# Single run (direct opt)
opt -passes="custom-dataflow" -df-interval < input.ll > /dev/null

# Debug trace
opt -passes="custom-dataflow" -df-interval -debug-only=interval-analysis < input.ll > /dev/null

# Scripted single/batch (if wired)
./run_single_dataflow.sh -i sample.c -a interval
./run_batch_dataflow.sh -d tests -a interval
```

This plan is intended to be followed sequentially. Do not skip stage exit criteria; each gate prevents hard-to-debug downstream failures.
