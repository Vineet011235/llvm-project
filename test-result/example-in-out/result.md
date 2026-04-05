# Interval Analysis Report

```text
+------------------------------------------------------------------+
|                    INTERVAL ANALYSIS REPORT                      |
+------------------------------------------------------------------+
| Test File   : example.cc
| Pre-Passes  : none
| Main Pass   : interval-analysis
+------------------------------------------------------------------+
```

## Generated Artifacts

- original.ir
- after-prepass.ir

## Final Interval Output

```text
+--------------------------------+
|  Basic Block: %entry           |
+--------------------------------+
| Variable | BoxIn | BoxOut      |
| -------- | ----- | ----------- |
| %add     | -     | [-INF, INF] |
| %cmp     | -     | [-INF, INF] |
| %0       | -     | [-INF, INF] |
| %1       | -     | [-INF, INF] |

+--------------------------------------+
|  Basic Block: %if.then               |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %add     | [-INF, INF] | [-INF, INF] |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %0       | [-INF, INF] | [-INF, INF] |
| %1       | [-INF, INF] | [-INF, INF] |

+--------------------------------------+
|  Basic Block: %if.else               |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %add     | [-INF, INF] | [-INF, INF] |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %0       | [-INF, INF] | [-INF, INF] |
| %1       | [-INF, INF] | [-INF, INF] |

+--------------------------------------+
|  Basic Block: %if.end                |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %add     | [-INF, INF] | [-INF, INF] |
| %add1    | -           | [-INF, INF] |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %sub     | -           | [-INF, INF] |
| %0       | [-INF, INF] | [-INF, INF] |
| %1       | [-INF, INF] | [-INF, INF] |
| %2       | -           | [-INF, INF] |
| %3       | -           | [-INF, INF] |
| %4       | -           | [-INF, INF] |
| %5       | -           | [-INF, INF] |

```
