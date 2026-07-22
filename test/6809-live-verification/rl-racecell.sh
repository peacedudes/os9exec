#!/usr/bin/env bash
# rl-racecell.sh <marker> [racer]  — one guarded lost-update measurement.
#
# Runs the two-racer RMW cell (default racer: rlrace3; pass rlrace3o for the
# observation variant) against whatever RBF the booted image carries, and
# prints the rlchk2 line.  A run is TRUSTED only when it reports done=2 —
# and note that on stock RBF the done marker itself can be lost-updated
# (both racers write done=1), which is evidence of the bug, not noise.
#
# Needs: R = path to tools/nitros9repl.sh, a booted logged-in REPL already
# chd/chx'd into a workdir holding the packed modules rlinit, rlchk2 and the
# racer (see tools/nitros9-upload-b09.sh), and a quiet host — the loss is
# timing-dependent and host load smears it.
set -u
: "${R:?set R to tools/nitros9repl.sh}"
n="$1"
racer="${2:-rlrace3}"
"$R" send "del rl.dat" >/dev/null 2>&1
"$R" send "del bg" >/dev/null 2>&1
"$R" send "del log1" >/dev/null 2>&1
"$R" send "del log2" >/dev/null 2>&1
"$R" send "rlinit" >/dev/null 2>&1
initchk=$("$R" peek 2>&1 | grep -v '^[[:space:]]*$' | tail -3)
case "$initchk" in *"rl.dat created"*) ;; *) echo "CELL$n: INIT FAILED"; exit 1;; esac
"$R" send "echo MARK$n" >/dev/null 2>&1
"$R" send "$racer >>>bg&" >/dev/null 2>&1
NITROS9REPL_TIMEOUT=150 "$R" send "$racer" >/dev/null 2>&1
i=0
while [ $i -lt 60 ]; do
  cnt=$("$R" peek 2>&1 | sed -n "/MARK$n/,\$p" | grep -c 'finished 200')
  [ "$cnt" -ge 2 ] && break
  sleep 2; i=$((i+1))
done
sleep 2
"$R" send "rlchk2" >/dev/null 2>&1
res=$("$R" peek 2>&1 | grep 'chk2: counter' | tail -1 | sed 's/.*chk2/chk2/')
case "$res" in
  *"done=2"*) echo "CELL$n: $res" ;;
  *) sleep 6; "$R" send "rlchk2" >/dev/null 2>&1
     res=$("$R" peek 2>&1 | grep 'chk2: counter' | tail -1 | sed 's/.*chk2/chk2/')
     echo "CELL$n(late): $res" ;;
esac
