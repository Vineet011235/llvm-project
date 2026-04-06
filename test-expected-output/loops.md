# Expected Output: loops

## Key expectations
- Loop header is `%for.cond` with `phi` variables `%i.0`, `%x.0`, `%y.0`.
- Widening behavior should force growth to INF where appropriate:
  - `%y.0`, `%x.0`, `%add1`, `%add2` should carry upper bound `INF`.
- Branch narrowing from loop guard (`%i.0 < 10`):
  - Loop body receives `%i.0 = [0, 9]`
  - Loop exit (`%for.end`) receives `%i.0 = {10}`
- Transfer correctness examples:
  - `%inc = %i.0 + 1 => [1, 10]`
  - `%add3 = %y.0 + %x.0 => [204, INF]`
- No endless finite growth (e.g., `[0, 1000]`) should appear for monotone increasing loop-carried values.
