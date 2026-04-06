# Interval Analysis Report

```text
+------------------------------------------------------------------+
|                    INTERVAL ANALYSIS REPORT                      |
+------------------------------------------------------------------+
| Test File   : /home/decompiler/llvm/llvm-project/tests/if_else.cc
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
|  Basic Block: %if.then               |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %add     | -           | [102, INF]  |
| %argc    | [2, INF]    | [2, INF]    |
| %cmp     | [-INF, INF] | [-INF, INF] |

+---------------------------------------+
|  Basic Block: %if.else                |
+---------------------------------------+
| Variable | BoxIn       | BoxOut       |
| -------- | ----------- | ------------ |
| %argc    | [-INF, 1]   | [-INF, 1]    |
| %cmp     | [-INF, INF] | [-INF, INF]  |
| %sub     | -           | [-INF, -199] |

+-----------------------------------------------------+
|  Basic Block: %if.end                               |
+-----------------------------------------------------+
| Variable | BoxIn        | BoxOut                    |
| -------- | ------------ | ------------------------- |
| %add     | [102, INF]   | [102, INF]                |
| %add1    | -            | [-INF, -149] U [152, INF] |
| %argc    | [-INF, INF]  | [-INF, INF]               |
| %cmp     | [-INF, INF]  | [-INF, INF]               |
| %sub     | [-INF, -199] | [-INF, -199]              |
| %x.0     | -            | [-INF, -199] U [102, INF] |

```
