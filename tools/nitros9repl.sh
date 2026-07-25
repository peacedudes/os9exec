#!/usr/bin/env bash
# nitros9repl.sh — tmux REPL helper for NitrOS-9 (6809) under XRoar + DriveWire
#
# Lets a Claude instance (or a human) drive a live NitrOS-9 shell through
# tmux, one command at a time, seeing only the output produced by that
# command — the 6809 sibling of tools/os9repl.sh.
#
# How it works: drivewire-cli (drpitre/drivewire, stock upstream main)
# listens on the becker port; XRoar's CoCo3 connects to it; NitrOS-9's
# startup runs `inetd&`, which reads /DD/SYS/inetd.conf and asks the server
# to `tcp listen 6811`. Connecting to that port makes inetd join the
# accepted connection to a fresh virtual serial channel and fork `login`
# onto it. The REPL talks to that port with nc, answering the login prompt
# with $NITROS9REPL_USER (default USER1, no password on the stock disk).
#
# Both halves are stock: NitrOS-9's own inetd/lib/net.as speaks the DriveWire
# `tcp listen`/`tcp join` command protocol, and upstream DriveWire implements
# the host side of it. Nothing here needs a patched DriveWire build.
#
# Usage:
#   ./tools/nitros9repl.sh start          boot server + XRoar, wait for the inetd shell
#   ./tools/nitros9repl.sh send <cmd>     send one command, wait for prompt, print new output
#   ./tools/nitros9repl.sh key <keys...>  send raw keystrokes (no Enter); special: Enter Space C-x
#   ./tools/nitros9repl.sh snap [label]   print labeled snapshot of the channel pane
#   ./tools/nitros9repl.sh peek           print current full channel pane
#   ./tools/nitros9repl.sh connect        interactive session in YOUR terminal (Ctrl-C to
#                                          detach; that ends the forked shell)
#   ./tools/nitros9repl.sh server         show the DriveWire server's protocol log pane
#   ./tools/nitros9repl.sh stop           kill the session (server + XRoar)
#   ./tools/nitros9repl.sh restart        stop + start
#
# Recognised prompt ("ready for next input"):
#   {N|NN}path:     the EOU Shell+ prompt on the session channel, e.g. {N|07}/DD:
#
# Notes:
#   - OS-9 wants CR line endings; the channel pane runs `stty -icrnl -icanon`
#     so Enter passes through as a bare CR immediately, and an unbuffered
#     perl stage converts OS-9's CR-only output to normal lines.
#   - inetd turns the guest's echo on (PD.EKO) for the session it forks, so
#     the guest echoes what you type and the channel pane does not; `send`
#     output therefore starts at the prompt row, which shows the command.
#     (`connect` relies on the same guest-side echo.)
#   - Each TCP connection is its own session: inetd forks a new login per
#     accepted connection, so disconnecting ends that session and reconnecting
#     logs in again (new pid). Nothing is replayed.
#   - Logging in is not just tidiness. The session's user number, execution
#     directory and data directory all come from that user's /DD/SYS/password
#     entry, so a logged-in session owns what it creates. A bare `shell` in
#     inetd.conf runs as the unauthenticated boot identity and leaves files a
#     later session cannot rewrite (E$FNA, 214).
#   - `connect` displaces the scripted pane's nc; the next `send` reconnects,
#     which under inetd means a new login rather than the same session.
#   - Escape ($1B) is OS-9's default SCF end-of-file character, so the shell
#     exits normally when it reads one. Under inetd that is recoverable
#     without a restart: the next `send` opens a new connection and inetd
#     forks another shell. `connect` still filters Esc/arrow keys so a human
#     can't send EOF by accident; to send one deliberately, use: key Escape
#
# Copyright notice (disk image content):
#   Anything observable through this REPL depends on what is in your disk
#   image.  NitrOS-9 is open source, but do not use this tool to extract or
#   redistribute copyrighted content from other disk images.

REPO="$(cd "$(dirname "$0")/.." && pwd)"
NITROS9="${NITROS9_DIR:-$REPO/../os9/nitros9}"
# Directory holding the disk image XRoar boots. Overridable so a test harness
# can run against a throwaway CLONE of the golden master rather than the master
# itself: the .ide file is edited IN PLACE by the guest, so any run that writes
# would otherwise mutate the shared boot disk permanently.
# A clone directory needs 68IDE.ide and hdblba.rom; -rompath is absolute.
DISKDIR="${NITROS9REPL_DISKDIR:-$NITROS9/disk-images/eou_ide-v0.3-6809-xroar-dw-becker}"
SESSION="${NITROS9REPL_SESSION:-nitros9repl}"
TIMEOUT=${NITROS9REPL_TIMEOUT:-20}      # seconds per command
BOOT_TIMEOUT=${NITROS9REPL_BOOT_TIMEOUT:-120}
KEY_DELAY=${NITROS9REPL_KEY_DELAY:-0.3}
BECKER_PORT=${NITROS9REPL_BECKER_PORT:-65504}
# Port the guest's inetd asks the DriveWire host to listen on. Must match the
# first field of the active line in the disk's /DD/SYS/inetd.conf.
CHAN_PORT=${NITROS9REPL_CHAN_PORT:-6811}
# Credentials for the `login` that inetd.conf spawns. Must name an entry in the
# disk's /DD/SYS/password; USER1 there has an empty password field, so login
# never asks for one and LOGIN_PASSWORD goes unused.
LOGIN_USER=${NITROS9REPL_USER:-USER1}
LOGIN_PASSWORD=${NITROS9REPL_PASSWORD:-}
# The server's protocol log, mirrored to a file as well as its tmux pane. If
# drivewire-cli ever exits unexpectedly its window closes and the pane's
# scrollback goes with it -- taking the only record of what it was doing. The
# file survives that, so `server` can still show the run's final moments.
SERVER_LOG="${NITROS9REPL_SERVER_LOG:-${TMPDIR:-/tmp}/nitros9repl-$SESSION.log}"

# drivewire-cli binary: env override, else newest Debug build in DerivedData.
find_cli() {
    if [ -n "$DWCLI" ] && [ -x "$DWCLI" ]; then printf '%s' "$DWCLI"; return; fi
    ls -t "$HOME"/Library/Developer/Xcode/DerivedData/DriveWire-*/Build/Products/Debug/drivewire-cli 2>/dev/null | head -1
}

# ── low-level helpers ─────────────────────────────────────────────────────────

alive() {
    tmux has-session -t "$SESSION" 2>/dev/null
}

# drivewire-cli exiting closes only its own window, leaving the session (and
# so `alive`) intact -- ask about the window when the server is the question.
server_alive() {
    tmux list-windows -t "$SESSION" -F '#W' 2>/dev/null | grep -qx server
}

# The channel pane (window "chan") is the one the REPL reads and types into.
pane() {
    tmux capture-pane -t "$SESSION:chan" -p -S -500 2>/dev/null || true
}

# Last non-blank pane row, trailing blanks stripped. Every prompt this script
# gates on is unterminated, so it is always the final row.
last_line() {
    pane | grep -v '^[[:space:]]*$' | tail -1 | sed 's/[[:space:]]*$//'
}

at_prompt() {
    # EOU Shell+ prompt: {N|07}/DD:  (device, pid and current dir all vary --
    # inetd opens /N and the multiplexer hands out whichever channel is free)
    printf '%s' "$(last_line)" | grep -qE '^\{N[0-9]*\|[0-9A-Fa-f]+\}[^ ]*:$'
}

at_login_prompt()    { printf '%s' "$(last_line)" | grep -q 'User name?:$'; }
at_password_prompt() { printf '%s' "$(last_line)" | grep -q 'Password:$'; }

# Wait for a usable shell prompt, answering inetd's `login` on the way if the
# disk's inetd.conf spawns one. Each connection gets its own login, so this
# runs on every (re)connect, not just at boot. It is also correct against a
# conf line that spawns a bare `shell`: the login branches simply never fire.
#
# Answers at most once each, so a rejected name or password surfaces as a
# timeout with the pane dumped rather than an endless retry loop.
wait_prompt() {
    local limit_s="${1:-$TIMEOUT}"
    local i=0 limit=$(( limit_s * 7 )) sent_user="" sent_pass=""
    while [ $i -lt $limit ]; do
        alive || { printf '[session exited]\n' >&2; return 1; }
        at_prompt && return 0
        if [ -z "$sent_user" ] && at_login_prompt; then
            tmux send-keys -t "$SESSION:chan" -l -- "$LOGIN_USER"
            tmux send-keys -t "$SESSION:chan" Enter
            sent_user=1
        elif [ -n "$sent_user" ] && [ -z "$sent_pass" ] && at_password_prompt; then
            tmux send-keys -t "$SESSION:chan" -l -- "$LOGIN_PASSWORD"
            tmux send-keys -t "$SESSION:chan" Enter
            sent_pass=1
        fi
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

# NOTE: tmux send-keys -l -- "$string" (a single already-quoted bash argv
# element) delivers every byte of $string to the pane literally -- tmux does
# not re-split or re-parse it for its own ';'-as-command-separator syntax
# (that syntax only applies to a *bare*, unquoted ';' token typed directly on
# a tmux command line). A previous version of this helper replaced ';' with
# '\;' on the theory that tmux needed it escaped; live-verified (2026-07-18,
# via the 6809 REPL) that this was wrong and actively harmful: it typed a
# literal backslash into the guest before every semicolon in any BASIC09
# line containing one (e.g. `PRINT #2, "x="; x` arrived on disk as
# `PRINT #2, "x="\; x`), which is any PRINT statement using ';' as a
# separator -- a common, previously-unnoticed source of corrupted typed
# source. Kept as a no-op (not removed outright) so call sites don't need to
# change and any future re-add of real escaping has one place to put it.
tmux_escape() {
    printf '%s' "$1"
}

send_one_key() {
    local k="$1"
    case "$k" in
        Enter|Return|enter)  tmux send-keys -t "$SESSION:chan" "Enter" ;;
        Space|space)         tmux send-keys -t "$SESSION:chan" " " ;;
        # Escape is OS-9's default SCF end-of-file character: sending one to
        # the session is a deliberate EOF (the shell exits on it; the next
        # `send` reconnects and inetd forks a new one).
        Escape|ESC|escape)   tmux send-keys -t "$SESSION:chan" "Escape" ;;
        C-*)                 tmux send-keys -t "$SESSION:chan" "$k" ;;
        *)                   tmux send-keys -t "$SESSION:chan" -l -- "$(tmux_escape "$k")" ;;
    esac
}

# The session pane the REPL types into and reads from.
#   -icrnl -icanon: Enter reaches OS-9 as a bare CR, unbuffered
#   -echo: no local echo. inetd turns the guest's own echo ON (PD.EKO) for
#          the session it forks, so local echo too would double every keystroke.
#   retry loop: the port does not exist until the guest has booted far enough
#          for inetd to run and ask the host to listen on it, so a single nc
#          at start would just be refused. Retry until one connects; nc's
#          "Connection refused" goes to /dev/null so it never reaches the pane
#          and confuses at_prompt/delta.
#   perl: converts OS-9 line ends to NL *unbuffered* (tr's line-buffered
#         stdout would hold back the trailing prompt). inetd also turns auto
#         line feed ON (PD.ALF), so the guest sends CR LF; dropping LF and
#         mapping CR to NL yields exactly one newline per line and is safe
#         across sysread chunk boundaries (a plain tr would double-space).
open_chan_window() {
    tmux new-window -t "$SESSION" -n chan \
        "stty -icrnl -icanon -echo; while ! nc 127.0.0.1 $CHAN_PORT 2>/dev/null; do sleep 0.5; done | perl -e '\$|=1; while (sysread(STDIN,\$b,4096)) { \$b =~ tr/\\n//d; \$b =~ tr/\\r/\\n/; print \$b }'"
}

# The server allows one client per channel: a `connect` displaces the chan
# pane's nc and its window closes. Recreate it on demand so `send` keeps
# working afterward.
ensure_chan() {
    tmux list-windows -t "$SESSION" -F '#W' 2>/dev/null | grep -qx chan && return 0
    open_chan_window
    sleep 0.5
    # No blind Enter to elicit a prompt: inetd forks a fresh login/shell per
    # connection, which announces itself unprompted. An Enter sent at the
    # "User name?:" prompt would just earn a "Who?" and another prompt.
    # wait_prompt (called next by every caller) drives the login.
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
    : > "$SERVER_LOG"
    tmux new-session -d -s "$SESSION" -n server -x 220 -y 60 \
        "'$cli' --tcp-port $BECKER_PORT --verbose 2>&1 | tee '$SERVER_LOG'"
    printf '[starting drivewire-cli...]\n'
    local i=0
    while [ $i -lt 35 ]; do
        tmux capture-pane -t "$SESSION:server" -p 2>/dev/null | grep -q 'listening on port' && break
        sleep 0.15; i=$(( i + 1 ))
    done

    # Window 1 "xroar": headless CoCo3 boot straight into NitrOS-9.
    # NITROS9REPL_GUI=1 shows the emulator window instead (focus-stealing!).
    # NITROS9REPL_EXTRA_XROAR passes through extra XRoar flags, e.g.
    # NITROS9REPL_EXTRA_XROAR=-no-ratelimit for full-speed (non-real-time) runs.
    # GUI mode still wants audio off -- the window is for screenshots
    # (tools/cocoscreen.sh), not for listening to.
    local ui="-ui null -ao null"
    local prev_app=""
    if [ -n "$NITROS9REPL_GUI" ]; then
        ui="-ao null"
        # XRoar's window steals focus the moment it opens. Remember whatever
        # app currently has focus so it can be handed back once the window
        # exists -- otherwise keystrokes meant for another window land on
        # the emulator instead.
        prev_app=$(osascript -e \
            'tell application "System Events" to get name of first process whose frontmost is true' \
            2>/dev/null || true)
    fi
    tmux new-window -t "$SESSION" -n xroar -c "$DISKDIR" \
        "xroar -rompath '$NITROS9/roms' -machine coco3 -tv-input rgb -machine-cart ide \
         -cart-rom ./hdblba.rom -load-hd0 68IDE.ide -cart-becker \
         -becker-port $BECKER_PORT -type 'DOS 0\\r\\r' $ui $NITROS9REPL_EXTRA_XROAR"
    printf '[booting NitrOS-9 under XRoar (up to %ss)...]\n' "$BOOT_TIMEOUT"

    # Window 2 "chan": the inetd session client (retries until inetd listens).
    open_chan_window

    if [ -n "$NITROS9REPL_GUI" ]; then
        # tmux new-window returning doesn't mean XRoar's actual window exists
        # yet (ROM load etc. takes a beat), so retry `place` briefly rather
        # than failing silently on the first attempt.
        local j=0
        while [ $j -lt 20 ]; do
            "$REPO/tools/cocoscreen.sh" place 0 0 >/dev/null 2>&1 && break
            sleep 0.25; j=$(( j + 1 ))
        done
        # Give focus straight back to whatever the user was doing -- see the
        # prev_app comment above.
        if [ -n "$prev_app" ]; then
            osascript -e "tell application \"$prev_app\" to activate" >/dev/null 2>&1 || true
        fi
    fi

    if wait_prompt "$BOOT_TIMEOUT"; then
        printf '[ready — shell prompt below]\n'
        pane | grep -v '^[[:space:]]*$' | tail -4
    else
        printf '[boot did not reach a shell prompt — check `snap` and the server window]\n' >&2
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
    printf '[connecting to the guest shell on port %s — Ctrl-C to detach]\n' "$CHAN_PORT"
    printf '[inetd forks a shell per connection, so detaching ends this one]\n'
    printf '[line-oriented: local editing works; Esc/arrow keys are filtered]\n'
    perl -e '$|=1; while (sysread(STDIN,$b,4096)) { $b =~ tr/\n/\r/; $b =~ tr/\x20-\x7e\r//cd; print $b }' \
        | nc 127.0.0.1 "$CHAN_PORT" \
        | perl -e '$|=1; while (sysread(STDIN,$b,4096)) { $b =~ tr/\n//d; $b =~ tr/\r/\n/; print $b }'
}

# Prefer the log file over the pane: it holds the whole run rather than the
# last few thousand rows, and it still exists if the server died and took its
# window with it. --verbose logs every SERREAD poll, so the interesting lines
# (the tcp listen/join handshake) scroll far out of a short capture.
cmd_server() {
    if [ -s "$SERVER_LOG" ]; then
        cat "$SERVER_LOG"
        # Test the server WINDOW, not the session: drivewire-cli exiting closes
        # only its own window, and the surviving xroar window keeps `alive`
        # true -- so `alive` here would never report the very failure this
        # log exists to catch.
        server_alive || printf '[server is NOT running — log above is from the dead run]\n' >&2
        return 0
    fi
    server_alive || { printf '[server not running and no log at %s]\n' "$SERVER_LOG" >&2; return 1; }
    tmux capture-pane -t "$SESSION:server" -p -S -5000
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
        printf '  start            boot drivewire-cli + XRoar, wait for the inetd shell prompt\n'
        printf '  send CMD         send command, wait for prompt, print new output\n'
        printf '  key K [K...]     send raw keystrokes (no Enter); special: Enter Space C-x\n'
        printf '  snap [label]     print labeled snapshot of the channel pane\n'
        printf '  peek             show full current channel pane\n'
        printf '  connect          interactive session in your own terminal (Ctrl-C detaches)\n'
        printf '  server           show the DriveWire protocol log\n'
        printf '  stop             kill the session (server + XRoar)\n'
        printf '  restart          stop + start\n'
        ;;
esac
