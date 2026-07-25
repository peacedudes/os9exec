#!/usr/bin/env bash
# b09run.sh — compile and run a BASIC09 procedure on the live NitrOS-9 system
#
# BASIC09 has no immediate mode: you cannot type a statement and see it run.
# Everything must go through its line editor, which makes scripted testing
# painful.  This wraps the whole dance — write a command script on the guest
# with `build`, then feed it to BASIC09 on stdin, which BASIC09 accepts and
# processes exactly as if typed.
#
# Usage:
#   ./tools/b09run.sh <procname> [--bg] < body.txt
#   printf 'PRINT "HI"\n' | ./tools/b09run.sh demo
#
# Body lines are the procedure body ONLY.  Do NOT include `PROCEDURE <name>`
# or the closing `END` — BASIC09's `e <name>` creates that skeleton itself,
# and supplying it again is a duplicate that fails with
# "Error #012 Illegal Statement Construction".
#
# --bg runs the procedure in the background (`&`) and returns immediately.
# Use it for anything that must stay alive to be looked at: a window is torn
# down when its path closes, so a procedure that draws and exits leaves
# nothing to screenshot.  Pair it with a `LOOP`/`ENDLOOP` body, capture with
# tools/cocoscreen.sh, then `kill` the process.  A window belonging to a live
# process is also reachable in the CLEAR cycle, which one owned by nobody
# is not.
#
# Requires a running REPL (tools/nitros9repl.sh) and writes into the guest's
# current data directory.

set -euo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
REPL="$REPO/tools/nitros9repl.sh"

die() { printf '%s\n' "$*" >&2; exit 1; }

[ $# -ge 1 ] || die "usage: b09run.sh <procname> [--bg] < body.txt"
proc="$1"; shift
background=""
[ "${1:-}" = "--bg" ] && background="&"

mapfile -t body || die "b09run: cannot read procedure body from stdin"
[ "${#body[@]}" -gt 0 ] || die "b09run: empty procedure body"

script="$proc.s"

# Driving `build` with raw keystrokes into a channel that is actually sitting
# at a BASIC09 `B:` or debugger `D:` prompt feeds the whole script to the wrong
# reader and wedges the REPL.  So make sure of the prompt first — and recover
# automatically, because unattended test runs otherwise stall on the first
# procedure that happens to drop into the debugger.
session="${NITROS9REPL_SESSION:-nitros9repl}"

channel_tail() {
    tmux capture-pane -t "$session:chan" -p -S -200 2>/dev/null \
        | grep -v '^[[:space:]]*$' | tail -1 | sed 's/[[:space:]]*$//'
}

at_shell_prompt() {
    channel_tail | grep -qE '^\{N1\|[0-9A-Fa-f]+\}[^ ]*:$'
}

# `bye` leaves BASIC09's command prompt.  Nothing reliably leaves the debugger
# `D:` prompt — `q`, `quit`, `kill` are rejected outright, `end`/`stop` are
# accepted but do not return to Ready — so a restart is the only dependable
# way out, and it is what this falls back to.
ensure_shell_prompt() {
    local attempt
    for attempt in 1 2 3; do
        at_shell_prompt && return 0
        "$REPL" key Enter >/dev/null 2>&1
        sleep 1
        at_shell_prompt && return 0
        "$REPL" key bye Enter >/dev/null 2>&1
        sleep 1.5
    done
    at_shell_prompt && return 0

    printf 'b09run: channel wedged at "%s" — restarting the REPL\n' "$(channel_tail)" >&2
    NITROS9REPL_GUI="${NITROS9REPL_GUI:-1}" \
    NITROS9REPL_EXTRA_XROAR="${NITROS9REPL_EXTRA_XROAR:--no-ratelimit}" \
        "$REPL" restart >/dev/null 2>&1
    sleep 2
    [ -n "${B09RUN_CHD:-}" ] && "$REPL" send "chd $B09RUN_CHD" >/dev/null 2>&1
    at_shell_prompt || die "b09run: REPL still not at a shell prompt after restart"
    printf 'b09run: REPL restarted; note any background job was lost\n' >&2
}

ensure_shell_prompt

# `key` (raw keystrokes) rather than `send` (prompt-gated): `build` and the
# BASIC09 editor present their own prompts, which `send` does not recognise.
type_line() {
    "$REPL" key "$1" Enter >/dev/null 2>&1
    sleep 0.3
}

"$REPL" send "del $script" >/dev/null 2>&1 || true
"$REPL" key "build $script" Enter >/dev/null 2>&1
sleep 1

type_line "e $proc"
for line in "${body[@]}"; do
    # Leading space is what makes the editor INSERT the line rather than
    # interpret it as an editor command.
    type_line " $line"
done
type_line "q"
type_line "run $proc"
type_line "bye"
"$REPL" key Enter >/dev/null 2>&1   # blank line terminates `build`
sleep 1

# `build` is driven with raw keystrokes, so nothing reports whether the lines
# actually landed.  If the channel was busy the file ends up empty and BASIC09
# then runs a no-op script and prints a perfectly healthy-looking banner —
# which reads as "the test passed and drew nothing".  Verify instead.
written=$("$REPL" send "list $script" 2>&1 || true)
if ! printf '%s' "$written" | grep -q "run $proc"; then
    die "b09run: script $script did not get written (channel busy?) — got:
$written"
fi

if [ -n "$background" ]; then
    # A backgrounded BASIC09 inherits the session channel for output, and its
    # prompts then
    # interleave with the REPL's own channel and wreck it (the REPL ends up
    # parked at a stray `B:` prompt).  Send both stdout and stderr to /nil.
    # Note OS-9 spells stderr `>>`.  Append is `>+` (`>-` truncates); `>>>`
    # is NOT append, despite looking like it — verified live, it silently
    # does nothing useful.
    "$REPL" send "basic09 #32k <$script >/nil >>/nil&"
else
    "$REPL" send "basic09 #32k <$script"
fi
