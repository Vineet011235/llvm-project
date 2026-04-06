# Expected Output: if_if_else

## Key expectations
- Nested branch narrowing should be visible:
  - `%if.then`: `%argc = [3, INF]` from `%argc > 2`
  - In `%if.else` branch, `%cmp1` (`%argc > 1`) splits to:
    - `%if.then2`: `%argc = {2}`
    - `%if.else3`: `%argc = [-INF, 1]`
- Phi precision:
  - `%x.1` at `%if.end` = `{6} U {8}`
  - `%x.0` at `%if.end4` = `{4} U {6} U {8}`
- Transfer correctness at exit:
  - `%add = %x.0 + 50 = {54} U {56} U {58}`
- Non-updated variables should remain unchanged across blocks (SSA integrity).
