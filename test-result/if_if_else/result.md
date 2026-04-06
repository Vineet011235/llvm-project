# Interval Analysis Report

```text
+------------------------------------------------------------------+
|                    INTERVAL ANALYSIS REPORT                      |
+------------------------------------------------------------------+
| Test File   : /home/decompiler/llvm/llvm-project/tests/if_if_else.cc
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
| %argc    | [3, INF]    | [3, INF]    |
| %cmp     | [-INF, INF] | [-INF, INF] |

+--------------------------------------+
|  Basic Block: %if.else               |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %argc    | [-INF, 2]   | [-INF, 2]   |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %cmp1    | -           | [-INF, INF] |

+--------------------------------------+
|  Basic Block: %if.then2              |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %argc    | {2}         | {2}         |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %cmp1    | [-INF, INF] | [-INF, INF] |

+--------------------------------------+
|  Basic Block: %if.else3              |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %argc    | [-INF, 1]   | [-INF, 1]   |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %cmp1    | [-INF, INF] | [-INF, INF] |

+--------------------------------------+
|  Basic Block: %if.end                |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %argc    | [-INF, 2]   | [-INF, 2]   |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %cmp1    | [-INF, INF] | [-INF, INF] |
| %x.1     | -           | {6} U {8}   |

+---------------------------------------------+
|  Basic Block: %if.end4                      |
+---------------------------------------------+
| Variable | BoxIn       | BoxOut             |
| -------- | ----------- | ------------------ |
| %add     | -           | {54} U {56} U {58} |
| %argc    | [-INF, INF] | [-INF, INF]        |
| %cmp     | [-INF, INF] | [-INF, INF]        |
| %cmp1    | [-INF, INF] | [-INF, INF]        |
| %x.0     | -           | {4} U {6} U {8}    |
| %x.1     | {6} U {8}   | {6} U {8}          |

```
