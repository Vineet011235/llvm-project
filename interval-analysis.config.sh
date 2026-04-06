# Default interval-analysis settings for run_single_interval.sh and run_batch_interval.sh.
# Edit this file to change the default pass enablement or pipeline order.

# Default pre-pass enablement.
# 0 means disabled, 1 means enabled.
INTERVAL_DEFAULT_PRE_SROA=0
INTERVAL_DEFAULT_PRE_MEM2REG=1
INTERVAL_DEFAULT_PRE_SIMPLIFYCFG=0
INTERVAL_DEFAULT_PRE_LOOP_SIMPLIFY=1

# Default pre-pass execution order.
INTERVAL_PREPASS_ORDER=("sroa" "mem2reg" "simplifycfg" "loop-simplify")
