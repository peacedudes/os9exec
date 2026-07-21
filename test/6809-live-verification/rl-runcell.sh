#!/usr/bin/env bash
# runcell.sh <sloww|slowu>  -- one follow-the-producer measurement.
#
# Batching these without checking the setup produced garbage: a background
# producer from a previous run kept rl.dat open, so `del` failed, `rlinit`
# failed with 218, the data file was stale, and an orphaned rlfollo was still
# writing into the channel. Counts from that look like results.
#
# So every run here proves its own preconditions: rl.dat really was recreated,
# and the reader's output really is this run's (its records must carry
# write-stamps at or after the moment rlinit finished).
#
# Prerequisites (export before running, or set NITROS9REPL_SESSION etc. so this
# points at YOUR isolated REPL -- see docs/nitros9-rbf-lock-HANDOFF.md):
#   R = path to tools/nitros9repl.sh
# and a booted REPL logged in as USER1 with chd+chx to the working directory
# holding the packed rlinit/rlsloww/rlslowu/rlfollo modules.
set -uo pipefail
: "${R:?set R to the path of tools/nitros9repl.sh}"
export NITROS9REPL_TIMEOUT=${NITROS9REPL_TIMEOUT:-200}
mode="$1"

# The reader can return while the producer is still running and holding
# rl.dat, which makes `del` fail and `rlinit` then fail with 218. Wait the
# producer out rather than measuring against a stale file.
init=""
for attempt in 1 2 3 4 5 6 7 8; do
    "$R" send "del rl.dat" >/dev/null 2>&1
    init=$("$R" send "rlinit" 2>&1)
    printf '%s' "$init" | grep -qi 'error' || break
    sleep 4
done
if printf '%s' "$init" | grep -qi 'error'; then
    echo "SETUP FAILED (rlinit): $(printf '%s' "$init" | grep -i error | head -1)"
    exit 1
fi

start=$(date +%s)
"$R" send "rl$mode >/nil >>/nil&" >/dev/null 2>&1
out=$("$R" send "rlfollo" 2>&1)
elapsed=$(( $(date +%s) - start ))

n=$(printf '%s\n' "$out" | grep -c 'follo: record' || true)
opened=$(printf '%s\n' "$out" | grep -c 'follo: opened v7' || true)
if [ "$opened" -ne 1 ]; then
    echo "AMBIGUOUS: saw $opened 'opened v7' banners -- channel has stray output"
    printf '%s\n' "$out"
    exit 1
fi
echo "mode=$mode records=$n elapsed=${elapsed}s"
printf '%s\n' "$out" | grep 'follo: record' | sed 's/^/    /'
