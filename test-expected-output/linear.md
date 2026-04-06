# Expected Output: linear

## Key expectations
- Single-block analysis (`%entry` only), no control-flow joins.
- Constant arithmetic should be exact:
  - `%add`/`%add1` = `{101}`
  - `%sub` = `{0}`, `%sub3` = `{99}`
  - `%mul` = `{200}`, `%mul5` = `{100}`
  - `%div` = `{50}`, `%div7` = `{100}`
- Values depending on loaded user input remain conservative (TOP-like):
  - `%0`/`%1`/`%2`/`%3`, `%add2`, `%sub4`, `%mul6`, `%div8` should stay `[-INF, INF]`.
- No unsound narrowing should appear for `%c`-driven expressions.
