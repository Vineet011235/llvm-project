# Interval Analysis Report

```text
+------------------------------------------------------------------+
|                    INTERVAL ANALYSIS REPORT                      |
+------------------------------------------------------------------+
| Test File   : example.cc
| Pre-Passes  : mem2reg
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
| %a.0     | -     | {10}   |
| %i.0     | -     | {0}    |

+--------------------------------------+
|  Basic Block: %for.cond              |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %a.0     | [10, INF]   | [10, INF]   |
| %add     | [11, INF]   | [11, INF]   |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %i.0     | [0, 5]      | [0, 5]      |
| %inc     | [1, 5]      | [1, 5]      |

+--------------------------------------+
|  Basic Block: %for.body              |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %a.0     | [10, INF]   | [10, INF]   |
| %add     | [11, INF]   | [11, INF]   |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %i.0     | [0, 4]      | [0, 4]      |
| %inc     | [1, INF]    | [1, INF]    |

+--------------------------------------+
|  Basic Block: %for.inc               |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %a.0     | [10, INF]   | [11, INF]   |
| %add     | [11, INF]   | [11, INF]   |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %i.0     | [0, 4]      | [1, 5]      |
| %inc     | [1, INF]    | [1, 5]      |

+--------------------------------------+
|  Basic Block: %for.end               |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %a.0     | [10, INF]   | [10, INF]   |
| %add     | [11, INF]   | [11, INF]   |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %i.0     | [5, INF]    | [5, INF]    |
| %inc     | [1, INF]    | [1, INF]    |

```
