#!/usr/bin/env bash
# os9repl.sh — tmux REPL helper for os9exec
#
# Lets a Claude instance (or a human) drive os9exec interactively through
# tmux, one command at a time, seeing only the output produced by that
# command rather than the full scrollback.
#
# Usage:
#   ./tools/os9repl.sh start          launch ./os9exec shell in a tmux session
#   ./tools/os9repl.sh send <cmd>     send one command, wait for prompt, print new output
#   ./tools/os9repl.sh peek           print current full pane (useful after a crash)
#   ./tools/os9repl.sh stop           kill the session
#   ./tools/os9repl.sh restart        stop + make + start in one step
#
# Recognised prompts (both mean "ready for next input"):
#   $              OS-9 shell
#   for hlp)       debugwait() loop (ends every debug-prompt line)
#
# Copyright notice (disk image content):
#   Anything that Claude or another automated tool can observe through this
#   REPL — directory listings, file contents, program output — depends on
#   what is in your disk image.  OS-9 software (binaries, libraries, shell
#   scripts) may be copyright Microware Systems Corporation or other parties.
#   Do not use this tool to extract, reproduce, or redistribute copyrighted
#   content without appropriate authorisation.

REPO="$(cd "$(dirname "$0")/.." && pwd)"
SESSION="os9exec"
TIMEOUT=${OS9REPL_TIMEOUT:-20}  # seconds per command (override with OS9REPL_TIMEOUT=60)

# ── low-level helpers ─────────────────────────────────────────────────────────

alive() {
    tmux has-session -t "$SESSION" 2>/dev/null
}

pane() {
    tmux capture-pane -t "$SESSION" -p -S -500 2>/dev/null || true
}

at_prompt() {
    local content
    content=$(pane | grep -v '^[[:space:]]*$')
    local last
    last=$(printf '%s\n' "$content" | tail -1 | sed 's/[[:space:]]*$//')
    [ "$last" = '$' ] && return 0
    printf '%s' "$last" | grep -qE 'for hlp\)|^dbg:|^dis:|^tra:' && return 0
    # Also check last 5 lines (trace output may follow the prompt on same/next line)
    printf '%s\n' "$content" | tail -5 | grep -qE '(^|\s)dbg:\s*$|(^|\s)dis:\s*$|(^|\s)tra:\s*$' && return 0
    return 1
}

wait_prompt() {
    local i=0 limit=$(( TIMEOUT * 7 ))
    while [ $i -lt $limit ]; do
        alive || { printf '[os9exec exited]\n' >&2; return 1; }
        at_prompt && return 0
        sleep 0.15
        i=$(( i + 1 ))
    done
    printf '[TIMEOUT waiting for prompt]\n' >&2
    pane >&2
    return 1
}

# Print only lines added to 'after' that were not in 'before'.
# Uses line count: tmux capture-pane grows monotonically until it hits the
# history cap, so NR > n_before gives the new content.
# Trims leading/trailing blank lines from the result.
delta() {
    local before="$1" after="$2"
    local n_before
    n_before=$(printf '%s\n' "$before" | wc -l | tr -d ' ')
    local skip=$(( n_before ))
    printf '%s\n' "$after" | awk -v skip="$skip" '
        NR > skip { lines[NR] = $0; if ($0 ~ /[^ \t]/) { if (!first) first=NR; last=NR } }
        END { if (first) for (i=first; i<=last; i++) print lines[i] }
    '
}

# ── subcommands ───────────────────────────────────────────────────────────────

cmd_start() {
    tmux kill-session -t "$SESSION" 2>/dev/null || true
    tmux new-session -d -s "$SESSION" -c "$REPO" -x 220 -y 60 "OS9DISK='$REPO/dd' ./os9exec /dd/CMDS/shell"
    printf '[starting os9exec...]\n'
    if wait_prompt; then
        tmux send-keys -t "$SESSION" "chx /h1/CMDS" Enter
        wait_prompt
        tmux send-keys -t "$SESSION" "shell /dd/startup" Enter
        wait_prompt
        printf '[ready]\n'
        pane | grep -v '^[[:space:]]*$' | tail -5
    fi
}

cmd_send() {
    local cmd="$*"
    if ! alive; then
        printf '[session not running — use: start]\n' >&2
        return 1
    fi
    wait_prompt || return 1
    local before
    before=$(pane)
    tmux send-keys -t "$SESSION" "$cmd" Enter
    sleep 0.1
    local i=0 limit=$(( TIMEOUT * 7 )) after
    while [ $i -lt $limit ]; do
        alive || { printf '[os9exec exited unexpectedly]\n' >&2; return 1; }
        after=$(pane)
        if [ "$after" != "$before" ] && at_prompt; then
            delta "$before" "$after"
            return 0
        fi
        sleep 0.15
        i=$(( i + 1 ))
    done
    printf '[TIMEOUT]\n' >&2
    pane >&2
    return 1
}

cmd_peek() {
    alive || { printf '[no session running]\n' >&2; return 1; }
    pane
}

cmd_stop() {
    tmux kill-session -t "$SESSION" 2>/dev/null || true
    printf '[stopped]\n'
}

cmd_restart() {
    tmux kill-session -t "$SESSION" 2>/dev/null || true
    printf '[rebuilding...]\n'
    make -C "$REPO" 2>&1 | tail -3
    cmd_start
}

# ── dispatch ──────────────────────────────────────────────────────────────────

case "${1:-help}" in
    start)   cmd_start ;;
    send)    shift; cmd_send "$@" ;;
    peek)    cmd_peek ;;
    stop)    cmd_stop ;;
    restart) cmd_restart ;;
    *)
        printf 'Usage: os9repl.sh {start|send <cmd>|peek|stop|restart}\n'
        printf '\n'
        printf '  start      launch ./os9exec shell in a tmux session\n'
        printf '  send CMD   send command, wait for prompt, print new output\n'
        printf '  peek       show full current pane (useful after a crash)\n'
        printf '  stop       kill the tmux session\n'
        printf '  restart    stop + make + start in one step\n'
        ;;
esac
