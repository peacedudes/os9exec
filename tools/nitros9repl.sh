#!/usr/bin/env bash
# nitros9repl.sh — tmux REPL helper for NitrOS-9 (6809) under XRoar + DriveWire
#
# Lets a Claude instance (or a human) drive a live NitrOS-9 shell through
# tmux, one command at a time, seeing only the output produced by that
# command — the 6809 sibling of tools/os9repl.sh.
#
# How it works: drivewire-cli (drpitre/drivewire, branch virtual-serial)
# listens on the becker port; XRoar's CoCo3 connects to it; NitrOS-9's
# startup runs `shell <>>>/n1&`, whose virtual serial channel the server
# bridges to a local TCP port. The REPL talks to that port with nc.
#
# Usage:
#   ./tools/nitros9repl.sh start          boot server + XRoar, wait for the /N1 shell
#   ./tools/nitros9repl.sh send <cmd>     send one command, wait for prompt, print new output
#   ./tools/nitros9repl.sh key <keys...>  send raw keystrokes (no Enter); special: Enter Space C-x
#   ./tools/nitros9repl.sh snap [label]   print labeled snapshot of the channel pane
#   ./tools/nitros9repl.sh peek           print current full channel pane
#   ./tools/nitros9repl.sh connect        interactive session in YOUR terminal (Ctrl-C to
#                                          detach; the OS-9 session survives disconnects)
#   ./tools/nitros9repl.sh server         show the DriveWire server's protocol log pane
#   ./tools/nitros9repl.sh stop           kill the session (server + XRoar)
#   ./tools/nitros9repl.sh restart        stop + start
#
# Recognised prompt ("ready for next input"):
#   {N1|NN}path:    the EOU Shell+ prompt on /N1, e.g. {N1|07}/DD:
#
# Notes:
#   - OS-9 wants CR line endings; the channel pane runs `stty -icrnl -icanon`
#     so Enter passes through as a bare CR immediately, and an unbuffered
#     perl stage converts OS-9's CR-only output to normal lines.
#   - Neither the guest nor the channel pane echoes input; `send` output
#     therefore starts at the prompt row the command was typed on.
#     (`connect` keeps local echo on so humans can see their typing.)
#   - A disconnected client does not close the guest's shell; reconnecting
#     resumes the same session (backlogged output is replayed).
#   - One client per channel: `connect` displaces the scripted pane's nc;
#     the next `send` recreates it automatically.
#   - Escape ($1B) is OS-9's default SCF end-of-file character, so the /n1
#     shell exits normally when it reads one. Nothing respawns it (startup
#     launches it once), so after an EOF a full `restart` is needed.
#     `connect` filters Esc/arrow keys so a human can't send EOF by
#     accident; to send one deliberately, use: key Escape
#
# Copyright notice (disk image content):
#   Anything observable through this REPL depends on what is in your disk
#   image.  NitrOS-9 is open source, but do not use this tool to extract or
#   redistribute copyrighted content from other disk images.

REPO="$(cd "$(dirname "$0")/.." && pwd)"
NITROS9="${NITROS9_DIR:-$REPO/../os9/nitros9}"
DISKDIR="$NITROS9/disk-images/eou_ide-v0.3-6809-xroar-dw-becker"
SESSION="${NITROS9REPL_SESSION:-nitros9repl}"
TIMEOUT=${NITROS9REPL_TIMEOUT:-20}      # seconds per command
BOOT_TIMEOUT=${NITROS9REPL_BOOT_TIMEOUT:-120}
KEY_DELAY=${NITROS9REPL_KEY_DELAY:-0.3}
BECKER_PORT=${NITROS9REPL_BECKER_PORT:-65504}
CHAN_PORT=${NITROS9REPL_CHAN_PORT:-6811}   # /N1 = wire channel 1 = base 6810 + 1

# drivewire-cli binary: env override, else newest Debug build in DerivedData.
find_cli() {
    if [ -n "$DWCLI" ] && [ -x "$DWCLI" ]; then printf '%s' "$DWCLI"; return; fi
    ls -t "$HOME"/Library/Developer/Xcode/DerivedData/DriveWire-*/Build/Products/Debug/drivewire-cli 2>/dev/null | head -1
}

# ── low-level helpers ─────────────────────────────────────────────────────────

alive() {
    tmux has-session -t "$SESSION" 2>/dev/null
}

# The channel pane (window "chan") is the one the REPL reads and types into.
pane() {
    tmux capture-pane -t "$SESSION:chan" -p -S -500 2>/dev/null || true
}

at_prompt() {
    local last
    last=$(pane | grep -v '^[[:space:]]*$' | tail -1 | sed 's/[[:space:]]*$//')
    # EOU Shell+ prompt: {N1|07}/DD:  (pid and current dir vary)
    printf '%s' "$last" | grep -qE '^\{N1\|[0-9A-Fa-f]+\}[^ ]*:$'
}

wait_prompt() {
    local limit_s="${1:-$TIMEOUT}"
    local i=0 limit=$(( limit_s * 7 ))
    while [ $i -lt $limit ]; do
        alive || { printf '[session exited]\n' >&2; return 1; }
        at_prompt && return 0
        sleep 0.15
        i=$(( i + 1 ))
    done
    printf '[TIMEOUT waiting for prompt]\n' >&2
    pane >&2
    return 1
}

# Print pane content from the first line where 'after' diverges from
# 'before'. (Not just appended lines: OS-9 programs that emit no leading CR
# append their first output to the prompt row itself, e.g. `date`.)
delta() {
    local before="$1" after="$2"
    awk 'NR==FNR { b[NR]=$0; nb=NR; next }
         { a[FNR]=$0; na=FNR; if (!p && (FNR>nb || $0!=b[FNR])) p=FNR }
         END {
             if (!p) exit
             for (i=p; i<=na; i++) if (a[i] ~ /[^ \t]/) { if (!first) first=i; last=i }
             if (first) for (i=first; i<=last; i++) print a[i]
         }' <(printf '%s\n' "$before") <(printf '%s\n' "$after")
}

# Escape a literal string for tmux send-keys -l (';' needs a backslash).
tmux_escape() {
    printf '%s' "${1//;/\\;}"
}

send_one_key() {
    local k="$1"
    case "$k" in
        Enter|Return|enter)  tmux send-keys -t "$SESSION:chan" "Enter" ;;
        Space|space)         tmux send-keys -t "$SESSION:chan" " " ;;
        # Escape is OS-9's default SCF end-of-file character: sending one to
        # a program reading /n1 is a deliberate EOF (the /n1 shell itself
        # exits on it, and only `restart` brings it back).
        Escape|ESC|escape)   tmux send-keys -t "$SESSION:chan" "Escape" ;;
        C-*)                 tmux send-keys -t "$SESSION:chan" "$k" ;;
        *)                   tmux send-keys -t "$SESSION:chan" -l -- "$(tmux_escape "$k")" ;;
    esac
}

# The /N1 bridge pane the REPL types into and reads from.
#   -icrnl -icanon: Enter reaches OS-9 as a bare CR, unbuffered
#   -echo: no local echo (the guest doesn't echo either; delta starts at
#          the prompt row)
#   perl: converts OS-9's CR-only output to NL *unbuffered* (tr's
#         line-buffered stdout would hold back the trailing prompt);
#         the pane pty's default onlcr then renders NL as CRNL.
open_chan_window() {
    tmux new-window -t "$SESSION" -n chan \
        "stty -icrnl -icanon -echo; nc 127.0.0.1 $CHAN_PORT | perl -e '\$|=1; while (sysread(STDIN,\$b,4096)) { \$b =~ tr/\\r/\\n/; print \$b }'"
}

# The server allows one client per channel: a `connect` displaces the chan
# pane's nc and its window closes. Recreate it on demand so `send` keeps
# working afterward.
ensure_chan() {
    tmux list-windows -t "$SESSION" -F '#W' 2>/dev/null | grep -qx chan && return 0
    open_chan_window
    sleep 0.5
    # A fresh pane is blank (the previous client consumed the last prompt);
    # a bare Enter elicits a new one from the shell.
    at_prompt || tmux send-keys -t "$SESSION:chan" Enter
}

# ── subcommands ───────────────────────────────────────────────────────────────

cmd_start() {
    local cli
    cli=$(find_cli)
    if [ -z "$cli" ]; then
        printf '[drivewire-cli not found — build it, or set DWCLI=/path/to/drivewire-cli]\n' >&2
        return 1
    fi
    if [ ! -f "$DISKDIR/68IDE.ide" ]; then
        printf '[disk image not found: %s]\n' "$DISKDIR/68IDE.ide" >&2
        return 1
    fi
    tmux kill-session -t "$SESSION" 2>/dev/null || true
    if lsof -nP -iTCP:"$BECKER_PORT" -sTCP:LISTEN >/dev/null 2>&1; then
        printf '[port %s already in use — a stale server or another instance? lsof -nP -i :%s]\n' \
            "$BECKER_PORT" "$BECKER_PORT" >&2
        return 1
    fi

    # Window 0 "server": the DriveWire host, verbose protocol log.
    tmux new-session -d -s "$SESSION" -n server -x 220 -y 60 \
        "'$cli' --becker-port $BECKER_PORT --channel-port-base $(( CHAN_PORT - 1 )) --verbose"
    printf '[starting drivewire-cli...]\n'
    local i=0
    while [ $i -lt 35 ]; do
        tmux capture-pane -t "$SESSION:server" -p 2>/dev/null | grep -q 'DriveWire listening' && break
        sleep 0.15; i=$(( i + 1 ))
    done

    # Window 1 "xroar": headless CoCo3 boot straight into NitrOS-9.
    # NITROS9REPL_GUI=1 shows the emulator window instead (focus-stealing!).
    # NITROS9REPL_EXTRA_XROAR passes through extra XRoar flags, e.g.
    # NITROS9REPL_EXTRA_XROAR=-no-ratelimit for full-speed (non-real-time) runs.
    local ui="-ui null -ao null"
    [ -n "$NITROS9REPL_GUI" ] && ui=""
    tmux new-window -t "$SESSION" -n xroar -c "$DISKDIR" \
        "xroar -rompath '$NITROS9/roms' -machine coco3 -tv-input rgb -machine-cart ide \
         -cart-rom ./hdblba.rom -load-hd0 68IDE.ide -cart-becker \
         -becker-port $BECKER_PORT -type 'DOS 0\\r\\r' $ui $NITROS9REPL_EXTRA_XROAR"
    printf '[booting NitrOS-9 under XRoar (up to %ss)...]\n' "$BOOT_TIMEOUT"

    # Window 2 "chan": the /N1 bridge.
    open_chan_window

    if wait_prompt "$BOOT_TIMEOUT"; then
        printf '[ready — {N1|..} shell prompt below]\n'
        pane | grep -v '^[[:space:]]*$' | tail -4
    else
        printf '[boot did not reach the /N1 prompt — check `snap` and the server window]\n' >&2
        printf '[known transient: XRoar sometimes boots stuck ("bad data read" + frozen);\n' >&2
        printf ' a plain `restart` usually clears it]\n' >&2
        return 1
    fi
}

cmd_send() {
    local cmd="$*"
    if ! alive; then
        printf '[session not running — use: start]\n' >&2
        return 1
    fi
    ensure_chan
    wait_prompt || return 1
    local before
    before=$(pane)
    tmux send-keys -t "$SESSION:chan" -l -- "$(tmux_escape "$cmd")"
    tmux send-keys -t "$SESSION:chan" Enter
    sleep 0.1
    local i=0 limit=$(( TIMEOUT * 7 )) after
    while [ $i -lt $limit ]; do
        alive || { printf '[session exited unexpectedly]\n' >&2; return 1; }
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

cmd_key() {
    if ! alive; then
        printf '[session not running — use: start]\n' >&2
        return 1
    fi
    ensure_chan
    for k in "$@"; do
        send_one_key "$k"
    done
    sleep "$KEY_DELAY"
    pane
}

cmd_snap() {
    local label="${1:-snap}"
    printf '\n── %s ────────────────────────────────\n' "$label"
    pane
    printf '────────────────────────────────────────\n'
}

cmd_peek() {
    alive || { printf '[no session running]\n' >&2; return 1; }
    pane
}

# Interactive session for a human, in the calling terminal.
#
# The terminal stays in its normal canonical mode (local echo and line
# editing just work); line endings are translated in the pipeline instead:
# NL->CR toward OS-9, CR->NL back. The input stage also drops everything
# but printable ASCII + CR: Escape ($1B) is OS-9's default SCF end-of-file
# character, so a stray Esc or arrow key (ESC [ A) would make the guest
# shell read EOF and exit — normal OS-9 behavior, but since startup only
# launches that shell once, recovering means a full `restart`.
cmd_connect() {
    printf '[connecting to /N1 on port %s — Ctrl-C to detach, session survives]\n' "$CHAN_PORT"
    printf '[line-oriented: local editing works; Esc/arrow keys are filtered]\n'
    perl -e '$|=1; while (sysread(STDIN,$b,4096)) { $b =~ tr/\n/\r/; $b =~ tr/\x20-\x7e\r//cd; print $b }' \
        | nc 127.0.0.1 "$CHAN_PORT" \
        | perl -e '$|=1; while (sysread(STDIN,$b,4096)) { $b =~ tr/\r/\n/; print $b }'
}

cmd_server() {
    alive || { printf '[no session running]\n' >&2; return 1; }
    tmux capture-pane -t "$SESSION:server" -p -S -200
}

cmd_stop() {
    tmux kill-session -t "$SESSION" 2>/dev/null || true
    printf '[stopped]\n'
}

cmd_restart() {
    cmd_stop
    cmd_start
}

# ── dispatch ──────────────────────────────────────────────────────────────────

case "${1:-help}" in
    start)   cmd_start ;;
    send)    shift; cmd_send "$@" ;;
    key)     shift; cmd_key "$@" ;;
    snap)    shift; cmd_snap "$@" ;;
    peek)    cmd_peek ;;
    connect) cmd_connect ;;
    server)  cmd_server ;;
    stop)    cmd_stop ;;
    restart) cmd_restart ;;
    *)
        printf 'Usage: nitros9repl.sh {start|send <cmd>|key <keys...>|snap [label]|peek|connect|server|stop|restart}\n'
        printf '\n'
        printf '  start            boot drivewire-cli + XRoar, wait for the /N1 shell prompt\n'
        printf '  send CMD         send command, wait for prompt, print new output\n'
        printf '  key K [K...]     send raw keystrokes (no Enter); special: Enter Space C-x\n'
        printf '  snap [label]     print labeled snapshot of the channel pane\n'
        printf '  peek             show full current channel pane\n'
        printf '  connect          interactive /N1 session in your own terminal (Ctrl-C detaches)\n'
        printf '  server           show the DriveWire protocol log\n'
        printf '  stop             kill the session (server + XRoar)\n'
        printf '  restart          stop + start\n'
        ;;
esac
