#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUT_DIR="$ROOT_DIR/test-result/cpp_correctness"
INPUT_DIR="$ROOT_DIR/tests/cpp_correctness"

check_re() {
  local file="$1"
  local re="$2"
  local name="$3"
  if grep -Eq "$re" "$file"; then
    echo "[ok] $name"
  else
    echo "[fail] $name"
    echo "  file: $file"
    echo "  expected regex: $re"
    return 1
  fi
}

echo "[info] Running C++ correctness suite through interval analysis"
"$ROOT_DIR/run_batch_interval.sh" -d "$INPUT_DIR" -o "$OUT_DIR" -N -m >/dev/null

FAIL=0

SLT="$OUT_DIR/slt_true_false/result.md"
SGT="$OUT_DIR/sgt_true_false/result.md"
SWAP="$OUT_DIR/swapped_operand_refine/result.md"
UNSIGNED="$OUT_DIR/unsigned_conservative/result.md"
COMPOUND="$OUT_DIR/compound_condition_conservative/result.md"
NESTED="$OUT_DIR/nested_loop_narrowing/result.md"

check_re "$SLT" "%argc[[:space:]]*\| \[-INF, 9\][[:space:]]*\| \[-INF, 9\]" "slt then-edge refinement" || FAIL=1
check_re "$SLT" "%argc[[:space:]]*\| \[10, INF\][[:space:]]*\| \[10, INF\]" "slt else-edge refinement" || FAIL=1

check_re "$SGT" "%argc[[:space:]]*\| \[6, INF\][[:space:]]*\| \[6, INF\]" "sgt then-edge refinement" || FAIL=1
check_re "$SGT" "%argc[[:space:]]*\| \[-INF, 5\][[:space:]]*\| \[-INF, 5\]" "sgt else-edge refinement" || FAIL=1

check_re "$SWAP" "%argc[[:space:]]*\| \[-INF, 6\][[:space:]]*\| \[-INF, 6\]" "swapped predicate true-edge" || FAIL=1
check_re "$SWAP" "%argc[[:space:]]*\| \[7, INF\][[:space:]]*\| \[7, INF\]" "swapped predicate false-edge" || FAIL=1

check_re "$UNSIGNED" "%argc[[:space:]]*\| \[-INF, INF\][[:space:]]*\| \[-INF, INF\]" "unsigned remains conservative" || FAIL=1

check_re "$COMPOUND" "%argc[[:space:]]*\| \[1, 9\][[:space:]]*\| \[1, 9\]" "compound short-circuit then refinement" || FAIL=1
check_re "$COMPOUND" "%argc[[:space:]]*\| \[-INF, 0\] U \[10, INF\][[:space:]]*\| \[-INF, 0\] U \[10, INF\]" "compound else refinement" || FAIL=1

check_re "$NESTED" "%j\.0[[:space:]]*\| \[0, 1\][[:space:]]*\| \[0, 1\]" "nested inner-body bound" || FAIL=1
check_re "$NESTED" "%j\.0[[:space:]]*\| \[2, INF\][[:space:]]*\| \[2, INF\]" "nested inner-exit bound" || FAIL=1
check_re "$NESTED" "%i\.0[[:space:]]*\| \[3, INF\][[:space:]]*\| \[3, INF\]" "nested outer-exit bound" || FAIL=1

if [[ "$FAIL" -ne 0 ]]; then
  echo "[result] FAIL: one or more correctness checks did not match expected output"
  exit 2
fi

echo "[result] PASS: all C++ correctness checks matched expected output"
