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
| %a.0     | [-INF, INF] | [-INF, INF] |
| %add     | [-INF, INF] | [-INF, INF] |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %i.0     | [-INF, INF] | [-INF, INF] |
| %inc     | [-INF, INF] | [-INF, INF] |

+--------------------------------------+
|  Basic Block: %for.body              |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %a.0     | [-INF, INF] | [-INF, INF] |
| %add     | [-INF, INF] | [-INF, INF] |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %i.0     | [-INF, INF] | [-INF, INF] |
| %inc     | [-INF, INF] | [-INF, INF] |

+--------------------------------------+
|  Basic Block: %for.inc               |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %a.0     | [-INF, INF] | [-INF, INF] |
| %add     | [-INF, INF] | [-INF, INF] |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %i.0     | [-INF, INF] | [-INF, INF] |
| %inc     | [-INF, INF] | [-INF, INF] |

+--------------------------------------+
|  Basic Block: %for.end               |
+--------------------------------------+
| Variable | BoxIn       | BoxOut      |
| -------- | ----------- | ----------- |
| %a.0     | [-INF, INF] | [-INF, INF] |
| %add     | [-INF, INF] | [-INF, INF] |
| %cmp     | [-INF, INF] | [-INF, INF] |
| %i.0     | [-INF, INF] | [-INF, INF] |
| %inc     | [-INF, INF] | [-INF, INF] |
| %mul     | -           | [-INF, INF] |

```
