# Interval Analysis Report

```text
+------------------------------------------------------------------+
|                    INTERVAL ANALYSIS REPORT                      |
+------------------------------------------------------------------+
| Test File   : /home/decompiler/llvm/llvm-project/tests/basic/linear.cc
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
| %add1 | [-INF, INF] |
| %add2 | [-INF, INF] |
| %div  | [-INF, INF] |
| %div7 | [-INF, INF] |
| %div8 | [-INF, INF] |
| %mul  | [-INF, INF] |
| %mul5 | [-INF, INF] |
| %mul6 | [-INF, INF] |
| %sub  | [-INF, INF] |
| %sub3 | [-INF, INF] |
| %sub4 | [-INF, INF] |
| %0    | [-INF, INF] |
| %1    | [-INF, INF] |
| %10   | [-INF, INF] |
| %11   | [-INF, INF] |
| %12   | [-INF, INF] |
| %13   | [-INF, INF] |
| %14   | [-INF, INF] |
| %15   | [-INF, INF] |
| %16   | [-INF, INF] |
| %17   | [-INF, INF] |
| %18   | [-INF, INF] |
| %19   | [-INF, INF] |
| %2    | [-INF, INF] |
| %3    | [-INF, INF] |
| %4    | [-INF, INF] |
| %5    | [-INF, INF] |
| %6    | [-INF, INF] |
| %7    | [-INF, INF] |
| %8    | [-INF, INF] |
| %9    | [-INF, INF] |

```
