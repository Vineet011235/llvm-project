# Expected Output: compound_conditions

## Key expectations
- Compound condition narrowing from (`%argc < 10`) && (`%argc > 0`):
  - `%if.then` should receive `%argc = [1, 9]`
  - `%if.else` should represent complement paths: `[-INF, 0] U [10, INF]`
- Transfer correctness:
  - `%add = %argc + 1` in `%if.then` => `[2, 10]`
  - `%sub = %argc - 1` in `%if.else` => `[-INF, -1] U [9, INF]`
- Join correctness at `%if.end`:
  - `%y.0 = [-INF, -1] U [2, INF]`
- Pointer-based compare and xor may remain conservative if not modeled precisely.
