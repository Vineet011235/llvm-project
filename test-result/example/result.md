# Interval Analysis Report

```text
+------------------------------------------------------------------+
|                    INTERVAL ANALYSIS REPORT                      |
+------------------------------------------------------------------+
| Test File   : example.cc
| Pre-Passes  : sroa,simplifycfg,loop-simplify
| Main Pass   : interval-analysis
+------------------------------------------------------------------+
```

## Generated Artifacts

- original.ir
- after-prepass.ir
- interval.out

## Final Interval Output

```text
### Basic Block: %entry
| Value | Interval |
|---|---|
| %add | [110, 110] |
| %sub | [-10, -10] |

```
