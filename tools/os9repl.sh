#!/usr/bin/env bash
# os9repl.sh — tmux REPL helper for os9exec
#
# Lets a Claude instance (or a human) drive os9exec interactively through
# tmux, one command at a time, seeing only the output produced by that
# command rather than the full scrollback.
#
# Usage:
#   ./tools/os9repl.sh start              boot os9exec via /dd/startup (preloads the
#                                          toolchain, then tsmon -> `User name?:`)
#   ./tools/os9repl.sh send <cmd>         send one command, wait for prompt, print new output
#                                          (right after start, send the bare account
#                                          name, e.g. `send dog` -- tsmon already
#                                          invoked login, so no `login <user>` verb)
#   ./tools/os9repl.sh key <keys...>      send raw keystrokes (no Enter); special: Escape Enter Up Down Left Right
#   ./tools/os9repl.sh snap [label]       print labeled snapshot of current pane
#   ./tools/os9repl.sh vi <file> <seq>    run vi on file, execute key sequence, show screen at each step
#                                          seq: space-separated keys; use Escape, Enter, Up, Down, Left, Right
#                                          e.g.: vi /dd/foo.txt "i a b c Enter Escape : q ! Enter"
#   ./tools/os9repl.sh peek               print current full pane (useful after a crash)
#   ./tools/os9repl.sh stop               kill the session
#   ./tools/os9repl.sh restart            stop + make + start in one step
#
# Recognised prompts (all mean "ready for next input"):
#   $              OS-9 shell
#   for hlp)       debugwait() loop (ends every debug-prompt line)
#   User name?:    tsmon's login prompt, right after boot
#   Password:      only shown by accounts with a non-empty password
#
# Copyright notice (disk image content):
#   Anything that Claude or another automated tool can observe through this
#   REPL — directory listings, file contents, program output — depends on
#   what is in your disk image.  OS-9 software (binaries, libraries, shell
#   scripts) may be copyright Microware Systems Corporation or other parties.
#   Do not use this tool to extract, reproduce, or redistribute copyrighted
#   content without appropriate authorisation.

REPO="$(cd "$(dirname "$0")/.." && pwd)"
SESSION="${OS9REPL_SESSION:-os9exec}"  # override with OS9REPL_SESSION=<name> to avoid colliding with a concurrent caller
TIMEOUT=${OS9REPL_TIMEOUT:-20}  # seconds per command (override with OS9REPL_TIMEOUT=60)
KEY_DELAY=${OS9REPL_KEY_DELAY:-0.3}   # seconds between keystrokes in vi mode

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
    printf '%s' "$last" | grep -qE 'for hlp\)|^dbg:|^dis:|^tra:|^(su|claude|dog):$' && return 0
    # tsmon's login sequence (boot now goes through /dd/startup -> tsmon ->
    # login, not straight to a shell $ prompt) -- see cmd_start.
    printf '%s' "$last" | grep -qE '^User name\?:$|^Password:$' && return 0
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

# Escape a literal string for tmux send-keys -l: even in literal mode, tmux's
# own command parser still treats a bare ';' as a command separator (verified
# empirically — '-l --' alone does NOT protect it), so it must be backslash-escaped.
tmux_escape() {
    printf '%s' "${1//;/\\;}"
}

# Send one key token — single character or named key — without Enter.
# Named keys: Escape, Enter, Up, Down, Left, Right, BSpace, Tab, Space
send_one_key() {
    local k="$1"
    case "$k" in
        Escape|ESC|escape)   tmux send-keys -t "$SESSION" "Escape" ;;
        Enter|Return|enter)  tmux send-keys -t "$SESSION" "Enter" ;;
        Up|up)               tmux send-keys -t "$SESSION" "Up" ;;
        Down|down)           tmux send-keys -t "$SESSION" "Down" ;;
        Left|left)           tmux send-keys -t "$SESSION" "Left" ;;
        Right|right)         tmux send-keys -t "$SESSION" "Right" ;;
        BSpace|Backspace|BS) tmux send-keys -t "$SESSION" "BSpace" ;;
        Tab|tab)             tmux send-keys -t "$SESSION" "Tab" ;;
        Space|space)         tmux send-keys -t "$SESSION" " " ;;
        C-*)                 tmux send-keys -t "$SESSION" "$k" ;;
        *)                   tmux send-keys -t "$SESSION" -l -- "$(tmux_escape "$k")" ;;
    esac
}

# ── subcommands ───────────────────────────────────────────────────────────────

cmd_start() {
    tmux kill-session -t "$SESSION" 2>/dev/null || true
    # OS9STOP lets any account run the `stop`/`shutdown` internal command, not
    # just group-0 super-users -- so an agent driving a session that logs in as
    # a plain account (or gets stuck in a tsmon login loop) can always exit.
    #
    # Boot through /dd/startup (shell is the boot program, /dd/startup is its
    # procedure-file argument -- NOT a bootable target on its own, that fails
    # with E_FNA) rather than launching straight into /dd/CMDS/shell. startup
    # preloads the full toolchain (cio/csl/math, r68/l68/o68/runb, cc/cpp/c68/
    # gcc2/cccp2/cc2, common file utilities) as memory-resident modules, which
    # F$Fork's bare-name lookups resolve via F$Link regardless of any
    # account's chx -- so `cc` forking `cpp` etc. just works without a chx
    # workaround. Manually loading only `math cio` here (the old approach)
    # left cpp/c68/r68/l68 unloaded, which looks like a chx/PATH problem
    # (`cc: cannot execute the pre-processor`) but isn't one -- the fix is
    # more preloading, not chx surgery. If something you need still isn't
    # preloaded, add it to /dd/startup's `load` lines, don't patch chx.
    # OS9DISK names the system disk on the HOST side; /dd is what that disk
    # IS inside OS-9 -- so every path below says /dd. It used to say
    # /h0/startup, which worked only because os9exec ALSO maps an h0 directory
    # sitting beside the binary to /h0: the same directory was mounted twice
    # and the boot line depended on the working directory being $REPO. From
    # anywhere else /h0/startup is "can't open" while /dd/startup is right.
    # The disk itself is the operator's choice, never a repo-relative guess.
    tmux new-session -d -s "$SESSION" -c "$REPO" -x 220 -y 60 "OS9STOP=1 OS9DISK='$OS9DISK' ./os9exec $EXTRA_ARGS shell /dd/startup"
    printf '[starting os9exec...]\n'
    # startup ends in `tsmon /term`, which waits for a keypress before
    # showing `User name?:` -- wait for that banner, then send one.
    local i=0 limit=$(( TIMEOUT * 7 ))
    while [ $i -lt $limit ]; do
        alive || { printf '[os9exec exited]\n' >&2; return 1; }
        pane | grep -qE 'OS-9 Ready -- modules loaded' && break
        sleep 0.15
        i=$(( i + 1 ))
    done
    tmux send-keys -t "$SESSION" Enter
    wait_prompt   # waits for tsmon's `User name?:` (at_prompt now recognizes it)
    printf '[ready -- log in: send <username>, e.g. `send dog`]\n'
    pane | grep -v '^[[:space:]]*$' | tail -8
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
    tmux send-keys -t "$SESSION" -l -- "$(tmux_escape "$cmd")"
    tmux send-keys -t "$SESSION" Enter
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

# Send raw keystrokes (no Enter appended), then show pane.
cmd_key() {
    if ! alive; then
        printf '[session not running — use: start]\n' >&2
        return 1
    fi
    for k in "$@"; do
        send_one_key "$k"
    done
    sleep "$KEY_DELAY"
    pane
}

# Print a labeled snapshot of the current pane.
cmd_snap() {
    local label="${1:-snap}"
    printf '\n── %s ────────────────────────────────\n' "$label"
    pane
    printf '────────────────────────────────────────\n'
}

# Run vi on a file and walk through a space-separated key sequence,
# printing the rendered pane after each keystroke.
cmd_vi() {
    local file="$1"; shift
    local seq=("$@")

    if ! alive; then
        printf '[session not running — use: start]\n' >&2
        return 1
    fi
    wait_prompt || return 1

    printf '[launching VI %s]\n' "$file"
    tmux send-keys -t "$SESSION" -l -- "$(tmux_escape "VI $file")"
    tmux send-keys -t "$SESSION" Enter
    sleep 1.5          # wait for vi to draw initial screen
    cmd_snap "initial screen"

    local step=0
    for k in "${seq[@]}"; do
        step=$(( step + 1 ))
        printf '\n[step %d: sending "%s"]\n' "$step" "$k"
        send_one_key "$k"
        sleep "$KEY_DELAY"
        cmd_snap "after $k"
        # If vi has returned to shell prompt, stop early
        at_prompt && { printf '[vi exited — done]\n'; return 0; }
    done

    printf '\n[sequence done — current screen:]\n'
    pane
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
    start)   shift; EXTRA_ARGS="$*"; cmd_start ;;
    send)    shift; cmd_send "$@" ;;
    key)     shift; cmd_key "$@" ;;
    snap)    shift; cmd_snap "$@" ;;
    vi)      shift; cmd_vi "$@" ;;
    peek)    cmd_peek ;;
    stop)    cmd_stop ;;
    restart) cmd_restart ;;
    *)
        printf 'Usage: os9repl.sh {start|send <cmd>|key <keys...>|snap [label]|vi <file> <keys...>|peek|stop|restart}\n'
        printf '\n'
        printf '  start            launch ./os9exec shell in a tmux session\n'
        printf '  send CMD         send command, wait for prompt, print new output\n'
        printf '  key K [K...]     send raw keystrokes (no Enter); special: Escape Enter Up Down Left Right\n'
        printf '  snap [label]     print labeled snapshot of current pane\n'
        printf '  vi FILE K [K...] launch vi on FILE, walk key sequence, snapshot after each key\n'
        printf '  peek             show full current pane (useful after a crash)\n'
        printf '  stop             kill the tmux session\n'
        printf '  restart          stop + make + start in one step\n'
        ;;
esac
