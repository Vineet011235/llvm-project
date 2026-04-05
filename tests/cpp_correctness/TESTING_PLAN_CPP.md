# C++ Interval Analysis Correctness Plan

This suite uses C++ sources (not hand-written IR) and validates critical behavior by matching generated `result.md` output after running the existing pipeline.

## Test Cases

1. `slt_true_false.cc`
- Feature: signed less-than edge refinement (`x < 10`).
- Desired output:
  - true-edge state contains `%argc` in `[-INF, 9]`.
  - false-edge state contains `%argc` in `[10, INF]`.

2. `sgt_true_false.cc`
- Feature: signed greater-than edge refinement (`x > 5`).
- Desired output:
  - true-edge `%argc` in `[6, INF]`.
  - false-edge `%argc` in `[-INF, 5]`.

3. `swapped_operand_refine.cc`
- Feature: swapped-operand predicate handling (`7 > x`).
- Desired output:
  - true-edge `%argc` in `[-INF, 6]`.
  - false-edge `%argc` in `[7, INF]`.

4. `unsigned_conservative.cc`
- Feature: unsupported unsigned predicate remains conservative (`x < 8u`).
- Desired output:
  - `%argc` remains top `[-INF, INF]`.

5. `compound_condition_conservative.cc`
- Feature: short-circuit conjunction (`x < 10 && x > 0`).
- Desired output:
  - then path shows `%argc` in `[1, 9]`.
  - else path shows `%argc` in `[-INF, 0] U [10, INF]`.

6. `nested_loop_narrowing.cc`
- Feature: nested loops with widening/narrowing convergence.
- Desired output:
  - inner body `%j.0` in `[0, 1]`.
  - inner exit `%j.0` in `[2, INF]`.
  - outer exit `%i.0` in `[3, INF]`.

## Simple Check Command

Run all checks in one command:

- `./check_cpp_correctness.sh`

This command:
- runs `run_batch_interval.sh` with `-N -m` on `tests/cpp_correctness`.
- checks expected output patterns in generated markdown reports.
- exits nonzero if any expected pattern is missing.

## Anomaly Signals

- If signed edge checks fail, predicate inversion or edge-target mapping likely regressed.
- If unsigned case refines unexpectedly, conservative behavior may have become unsound.
- If nested-loop exit bounds disappear, widening/narrowing scheduling likely regressed.
