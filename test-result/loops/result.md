# Interval Analysis Report

```text
+------------------------------------------------------------------+
|                    INTERVAL ANALYSIS REPORT                      |
+------------------------------------------------------------------+
| Test File   : /home/decompiler/llvm/llvm-project/tests/loops.cc
| Pre-Passes  : mem2reg,loop-simplify
| Main Pass   : interval-analysis
+------------------------------------------------------------------+
```

## Generated Artifacts

- original.ir
- after-prepass.ir

## Final Interval Output

```text
+---------------------------+
|  Basic Block: %entry      |
+---------------------------+
| Variable | BoxIn | BoxOut |
| -------- | ----- | ------ |
| %add     | -     | {104}  |

+--------------------------------------+
|  Basic Block: %for.cond              |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %add     | {104}       | {104}       |
| %add1    | [114, INF]  | [114, INF]  |
| %add2    | [100, INF]  | [100, INF]  |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %i.0     | [0, 9]      | [0, 10]     |
| %inc     | [1, 10]     | [1, 10]     |
| %x.0     | [100, INF]  | [100, INF]  |
| %y.0     | [104, INF]  | [104, INF]  |

+--------------------------------------+
|  Basic Block: %for.body              |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %add     | {104}       | {104}       |
| %add1    | [114, INF]  | [114, INF]  |
| %add2    | [100, INF]  | [100, INF]  |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %i.0     | [0, 9]      | [0, 9]      |
| %inc     | [1, 10]     | [1, 10]     |
| %x.0     | [100, INF]  | [100, INF]  |
| %y.0     | [104, INF]  | [104, INF]  |

+--------------------------------------+
|  Basic Block: %for.inc               |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %add     | {104}       | {104}       |
| %add1    | [114, INF]  | [114, INF]  |
| %add2    | [100, INF]  | [100, INF]  |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %i.0     | [0, 9]      | [0, 9]      |
| %inc     | [1, 10]     | [1, 10]     |
| %x.0     | [100, INF]  | [100, INF]  |
| %y.0     | [104, INF]  | [104, INF]  |

+--------------------------------------+
|  Basic Block: %for.end               |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %add     | {104}       | {104}       |
| %add1    | [114, INF]  | [114, INF]  |
| %add2    | [100, INF]  | [100, INF]  |
| %add3    | -           | [204, INF]  |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %i.0     | {10}        | {10}        |
| %inc     | [1, 10]     | [1, 10]     |
| %x.0     | [100, INF]  | [100, INF]  |
| %y.0     | [104, INF]  | [104, INF]  |

```
