# Interval Analysis Report

```text
+------------------------------------------------------------------+
|                    INTERVAL ANALYSIS REPORT                      |
+------------------------------------------------------------------+
| Test File   : /home/decompiler/llvm/llvm-project/tests/linear.cc
| Pre-Passes  : mem2reg,loop-simplify
| Main Pass   : interval-analysis
+------------------------------------------------------------------+
```

## Generated Artifacts

- original.ir
- after-prepass.ir

## Final Interval Output

```text
+---------------------------------+
|  Basic Block: %entry            |
+---------------------------------+
| Variable  | BoxIn | BoxOut      |
| --------- | ----- | ----------- |
| @_ZSt3cin | -     | [-INF, INF] |
| %add      | -     | {101}       |
| %add1     | -     | {101}       |
| %add2     | -     | [-INF, INF] |
| %c        | -     | [-INF, INF] |
| %div      | -     | {50}        |
| %div7     | -     | {100}       |
| %div8     | -     | [-INF, INF] |
| %mul      | -     | {200}       |
| %mul5     | -     | {100}       |
| %mul6     | -     | [-INF, INF] |
| %sub      | -     | {0}         |
| %sub3     | -     | {99}        |
| %sub4     | -     | [-INF, INF] |
| %0        | -     | [-INF, INF] |
| %1        | -     | [-INF, INF] |
| %2        | -     | [-INF, INF] |
| %3        | -     | [-INF, INF] |

```
