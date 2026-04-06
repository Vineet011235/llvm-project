# Expected Output: if_else

## Key expectations
- Branch refinement on `%argc` from condition `%argc > 1`:
  - True edge (`%if.then`): `%argc = [2, INF]`
  - False edge (`%if.else`): `%argc = [-INF, 1]`
- Transfer correctness:
  - `%add` in `%if.then` = `[102, INF]`
  - `%sub` in `%if.else` = `[-INF, -199]`
- Join at `%if.end` through `phi` (`%x.0`):
  - `%x.0 = [-INF, -199] U [102, INF]`
  - `%add1 = %x.0 + 50 = [-INF, -149] U [152, INF]`
- No impossible-path poisoning should appear.
