# Interval Analysis Report

```text
+------------------------------------------------------------------+
|                    INTERVAL ANALYSIS REPORT                      |
+------------------------------------------------------------------+
| Test File   : /home/decompiler/llvm/llvm-project/tests/compound_conditions.cc
| Pre-Passes  : mem2reg,loop-simplify
| Main Pass   : interval-analysis
+------------------------------------------------------------------+
```

## Generated Artifacts

- original.ir
- after-prepass.ir

## Final Interval Output

```text
+--------------------------------------+
|  Basic Block: %entry                 |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %argc    | [-INF, INF] | [-INF, INF] |
| %cmp     | -           | [-INF, INF] |

+--------------------------------------+
|  Basic Block: %land.lhs.true         |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %argc    | [-INF, 9]   | [-INF, 9]   |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %cmp1    | -           | [-INF, INF] |

+--------------------------------------+
|  Basic Block: %if.then               |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %add     | -           | [2, 10]     |
| %argc    | [1, 9]      | [1, 9]      |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %cmp1    | [-INF, INF] | [-INF, INF] |

+----------------------------------------------------------+
|  Basic Block: %if.else                                   |
+----------------------------------------------------------+
| Variable | BoxIn                 | BoxOut                |
| -------- | --------------------- | --------------------- |
| %argc    | [-INF, 0] U [10, INF] | [-INF, 0] U [10, INF] |
| %cmp     | [-INF, INF]           | [-INF, INF]           |
| %cmp1    | [-INF, INF]           | [-INF, INF]           |
| %sub     | -                     | [-INF, -1] U [9, INF] |

+----------------------------------------------------------+
|  Basic Block: %if.end                                    |
+----------------------------------------------------------+
| Variable | BoxIn                 | BoxOut                |
| -------- | --------------------- | --------------------- |
| %add     | [2, 10]               | [2, 10]               |
| %argc    | [-INF, INF]           | [-INF, INF]           |
| %cmp     | [-INF, INF]           | [-INF, INF]           |
| %cmp1    | [-INF, INF]           | [-INF, INF]           |
| %cmp2    | -                     | [-INF, INF]           |
| %conv    | -                     | [-INF, INF]           |
| %sub     | [-INF, -1] U [9, INF] | [-INF, -1] U [9, INF] |
| %xor     | -                     | [-INF, INF]           |
| %y.0     | -                     | [-INF, -1] U [2, INF] |

```
