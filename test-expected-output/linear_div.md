# Expected Output: linear_div

## Key expectations
- Single-block analysis with precise constants where operands are constants.
- Division expectations:
  - `100 / 2 = {50}`
  - `100 / 1 = {100}`
  - `100 / unknown input => [-INF, INF]`.
- Input-dependent arithmetic (`%add2`, `%sub4`, `%mul6`, `%div8`) should remain conservative.
- No branch refinement or loop widening is expected in this testcase.
