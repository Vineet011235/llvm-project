# Interval Analysis Report

```text
+------------------------------------------------------------------+
|                    INTERVAL ANALYSIS REPORT                      |
+------------------------------------------------------------------+
| Test File   : /home/decompiler/llvm/llvm-project/tests/basic/box_overflow.cc
| Pre-Passes  : none
| Main Pass   : interval-analysis
+------------------------------------------------------------------+
```

## Generated Artifacts

- original.ir
- after-prepass.ir

## Final Interval Output

```text
+----------------------------------------------------------------------+
|  Basic Block: %entry                                                 |
+----------------------------------------------------------------------+
| Value | Box         |
| ----- | ----------- |
| %add  | [-INF, INF] |
| %conv | [-INF, INF] |
| %0    | [-INF, INF] |
| %1    | [-INF, INF] |

```
