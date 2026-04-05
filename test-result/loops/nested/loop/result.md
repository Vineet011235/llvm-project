# Interval Analysis Report

```text
+------------------------------------------------------------------+
|                    INTERVAL ANALYSIS REPORT                      |
+------------------------------------------------------------------+
| Test File   : /home/decompiler/llvm/llvm-project/tests/loops/nested/loop.cc
| Pre-Passes  : sroa,simplifycfg,loop-simplify
| Main Pass   : interval-analysis
+------------------------------------------------------------------+
```

## Generated Artifacts

- original.ir
- after-prepass.ir
- interval.out

## Final Interval Output

```text
### Basic Block: %entry
| Value | Interval |
|---|---|

### Basic Block: %for.cond
| Value | Interval |
|---|---|
| %a.0 | [-INF, INF] |
| %cmp | [-INF, INF] |
| %i.0 | [-INF, INF] |
| %inc | [-INF, INF] |
| %inc1 | [-INF, INF] |

### Basic Block: %for.body
| Value | Interval |
|---|---|
| %a.0 | [-INF, INF] |
| %cmp | [-INF, INF] |
| %i.0 | [-INF, INF] |
| %inc | [-INF, INF] |
| %inc1 | [-INF, INF] |

### Basic Block: %for.end
| Value | Interval |
|---|---|
| %a.0 | [-INF, INF] |
| %cmp | [-INF, INF] |
| %i.0 | [-INF, INF] |
| %inc | [-INF, INF] |
| %inc1 | [-INF, INF] |

```
