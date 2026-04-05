# Interval Analysis Report

```text
+------------------------------------------------------------------+
|                    INTERVAL ANALYSIS REPORT                      |
+------------------------------------------------------------------+
| Test File   : /home/decompiler/llvm/llvm-project/tests/basic/branch.cc
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
| %add | [101, 101] |
| %cmp | [-INF, INF] |

### Basic Block: %if.then
| Value | Interval |
|---|---|
| %add | [101, 101] |
| %cmp | [-INF, INF] |
| %mul | [404, 404] |

### Basic Block: %if.else
| Value | Interval |
|---|---|
| %add | [101, 101] |
| %cmp | [-INF, INF] |
| %sub | [98, 98] |

### Basic Block: %if.end
| Value | Interval |
|---|---|
| %add | [101, 101] |
| %cmp | [-INF, INF] |
| %mul | [404, 404] |
| %sub | [98, 98] |

```
