#!/usr/bin/env bash
# case.sh <contender.s> <label> -- one EOF-lock contention case, on a FRESH boot.
#
# A fresh boot per case is deliberate: an earlier attempt reused a guest whose
# previous holder had been Ctrl-E'd, and a contender then blocked for 950s+
# with no way to tell whether the intended holder or the stale one was holding
# it. Booting per case makes exactly one holder exist by construction.
set -uo pipefail
# Repo root, derived rather than hardcoded: this file used to cd to one
# developer's absolute path, so it broke for every clone and would have
# broken again the moment the checkout moved.
cd "$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
export NITROS9REPL_DISKDIR=$HOME/.cache/nitros9repl/eof-probe
export NITROS9REPL_EXTRA_XROAR="-no-ratelimit -ram 2048"
R=./tools/nitros9repl.sh
SCRIPT=$1; LABEL=$2

spell() { echo "$1" | sed 's/./& /g'; }

$R stop >/dev/null 2>&1
$R start >/dev/null 2>&1
$R send 'chd /dd/EOFPROBE'       >/dev/null 2>&1
$R send 'del eof.dat'            >/dev/null 2>&1
$R send 'basic09 #32k <einit.s'  >/dev/null 2>&1

$R key b a s i c 0 9 ' ' '#' 3 2 k ' ' '<' e h o l d . s ' ' '&' Enter >/dev/null 2>&1
held=0
for i in $(seq 1 30); do
    $R peek 2>/dev/null | grep -q 'A: PUT at EOF' && { held=1; break; }
    perl -e 'select undef,undef,undef,1'
done
[ "$held" -eq 1 ] || { echo "VOID $LABEL: holder never wrote at EOF"; exit 2; }
$R peek 2>/dev/null | grep -q 'A: closed' && { echo "VOID $LABEL: holder already released"; exit 2; }

START=$(date +%s)
$R key b a s i c 0 9 ' ' '#' 3 2 k ' ' '<' $(spell "$SCRIPT") Enter >/dev/null 2>&1

# Did the contender return BEFORE the holder released?
ret_before_close=""
for i in $(seq 1 150); do
    pane=$($R peek 2>/dev/null)
    if printf '%s' "$pane" | grep -q 'B: PUT returned'; then
        printf '%s' "$pane" | grep -q 'A: closing' && ret_before_close=no || ret_before_close=yes
        break
    fi
    perl -e 'select undef,undef,undef,2'
done
ELAPSED=$(( $(date +%s) - START ))

echo "===== $LABEL"
$R peek 2>/dev/null | grep -E 'A: |B: ' | sed 's/^B://' | grep -vE '^\s*$'
case "$ret_before_close" in
    yes) echo ">>> contender returned WITHOUT waiting for the holder  (NOT blocked)  ${ELAPSED}s" ;;
    no)  echo ">>> contender returned only AFTER the holder released   (BLOCKED)     ${ELAPSED}s" ;;
    *)   echo ">>> contender never returned within the window          (STUCK)       ${ELAPSED}s" ;;
esac
