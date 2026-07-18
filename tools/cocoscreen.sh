#!/usr/bin/env bash
# cocoscreen.sh — look at, and type into, the CoCo screen under XRoar
#
# The DriveWire REPL (tools/nitros9repl.sh) is a text channel: it can drive a
# NitrOS-9 shell but cannot observe a *screen*, which is why the 6809 graphics
# notes in the os9-dev skill are almost entirely unverified.  This tool closes
# that gap.  Together the two give a full loop: send commands over the /N1
# REPL, press keys with `key`/`type` here, and see the result with `shot`.
#
# Usage:
#   ./tools/cocoscreen.sh shot [file]     capture the screen to a PNG (prints the path)
#   ./tools/cocoscreen.sh key <name>...   press keys, e.g. `key clear`, `key enter`
#   ./tools/cocoscreen.sh type <string>   type a literal string (no trailing Enter)
#   ./tools/cocoscreen.sh clear           press CoCo CLEAR — cycles to the next screen
#   ./tools/cocoscreen.sh next            press CLEAR until the display actually changes
#   ./tools/cocoscreen.sh cycle [n]       press CLEAR n times, capturing each screen
#   ./tools/cocoscreen.sh place [x y]     move XRoar's window (default 0 0, upper-left)
#   ./tools/cocoscreen.sh measure FILE      report ink bounding box / line positions in a PNG
#   ./tools/cocoscreen.sh winid           print XRoar's window ID and geometry
#   ./tools/cocoscreen.sh pid             print XRoar's process ID
#
# Requires XRoar running WITH A WINDOW.  The REPL is headless by default, so
# start it as:  NITROS9REPL_GUI=1 ./tools/nitros9repl.sh start
#
# How it works, and the two traps worth knowing:
#
#   Capture uses `screencapture -l <CGWindowID>`, which grabs XRoar's window
#   and nothing else — never the desktop, so other windows on screen stay
#   private.  It renders through XRoar's own video path, so every GIME mode,
#   palette and window comes out right with no video decoding on this side.
#   XRoar's window ID changes on every restart, so it is re-queried each call.
#
#   Key injection uses a HID-level CGEvent posted straight to XRoar's pid.
#   AppleScript System Events does NOT work here — both `key code` and
#   `keystroke` reach the process but arrive as the wrong key (they showed up
#   as stray 'p' characters in the guest).
#
#   XRoar must be the FRONTMOST application to receive injected keys; a
#   postToPid to an unfocused XRoar is silently dropped (measured: identical
#   screen before and after, then the same key working once focused).  So
#   `key`/`type`/`clear` raise the window first.  That does steal focus, so
#   don't run them while someone is typing elsewhere.  `shot` does NOT need
#   focus and never raises the window.
#
# The CoCo CLEAR key is the host backtick; under NitrOS-9 it cycles between
# the screens of the active windows, and is the only way to bring a window's
# screen to the front from outside that window — a SELECT issued from another
# process is deferred until the next CLEAR.
#
# The Swift helpers in tools/cocoscreen/ are compiled on first use and cached
# outside the repo, keyed by source mtime; nothing is built into the tree.

set -euo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
SRCDIR="$REPO/tools/cocoscreen"
CACHE="${TMPDIR:-/tmp}/cocoscreen-build"
SHOTDIR="${COCOSCREEN_SHOTDIR:-${TMPDIR:-/tmp}/cocoscreen-shots}"

die() { printf '%s\n' "$*" >&2; exit 1; }

# Compile a helper if the cached binary is missing or older than its source.
build() {
    local name="$1" src="$SRCDIR/$1.swift" bin="$CACHE/$1"
    [ -f "$src" ] || die "cocoscreen: missing source $src"
    mkdir -p "$CACHE"
    if [ ! -x "$bin" ] || [ "$src" -nt "$bin" ]; then
        swiftc -O -o "$bin" "$src" >&2 || die "cocoscreen: failed to build $name"
    fi
    printf '%s' "$bin"
}

xroar_pid() {
    pgrep -f 'xroar .*-machine coco3' | head -1
}

# XRoar's emulator display window; winid.swift prints it first.
xroar_winid() {
    local out
    out=$("$(build winid)" xroar 2>/dev/null | head -1) \
        || die "cocoscreen: no XRoar window found — start the REPL with NITROS9REPL_GUI=1"
    [ -n "$out" ] || die "cocoscreen: no XRoar window found — start the REPL with NITROS9REPL_GUI=1"
    printf '%s' "${out%%$'\t'*}"
}

need_pid() {
    local pid
    pid=$(xroar_pid) || true
    [ -n "$pid" ] || die "cocoscreen: XRoar is not running"
    printf '%s' "$pid"
}

# Injected keys only land when XRoar is frontmost, so every key path raises it.
# NB: keep this on ONE line.  A backslash continuation inside single quotes is
# passed to AppleScript literally and is a syntax error there, not a shell line
# continuation — which fails silently and drops every keystroke.
focus() {
    osascript -e 'tell application "System Events" to tell process "xroar" to set frontmost to true' >/dev/null 2>&1 \
        || die "cocoscreen: could not focus XRoar — grant this terminal
       System Settings > Privacy & Security > Accessibility"
    sleep 0.5
}

cmd_shot() {
    local out="${1:-}"
    if [ -z "$out" ]; then
        mkdir -p "$SHOTDIR"
        out="$SHOTDIR/screen-$(date +%Y%m%d-%H%M%S).png"
    fi
    screencapture -x -o -l "$(xroar_winid)" "$out" \
        || die "cocoscreen: screencapture failed"
    printf '%s\n' "$out"
}

# Park the window at a known spot so captures are reproducible across runs and
# the emulator stays clear of whatever else is on the desktop.  This is the one
# place AppleScript is the right tool: window geometry is an accessibility
# property, not something CGEvent or screencapture can reach.  Needs the
# terminal to hold macOS Accessibility permission.
cmd_place() {
    local x="${1:-0}" y="${2:-0}"
    osascript -e "tell application \"System Events\" to tell process \"xroar\" \
                  to set position of window 1 to {$x, $y}" >/dev/null 2>&1 \
        || die "cocoscreen: could not move the window — grant this terminal
       System Settings > Privacy & Security > Accessibility"
    printf 'moved to %s,%s\n' "$x" "$y"
}

# Press CLEAR until the display actually changes.  Injected keys are dropped
# often enough (dependent on focus timing) that a single press is unreliable;
# retrying against an observed change makes screen switching deterministic,
# which every visual test depends on.
cmd_next() {
    local pid bin before after i
    pid=$(need_pid); bin=$(build sendkey)
    mkdir -p "$SHOTDIR"
    before="$SHOTDIR/.next-before.png"
    after="$SHOTDIR/.next-after.png"
    screencapture -x -o -l "$(xroar_winid)" "$before"
    focus
    for (( i = 1; i <= 8; i++ )); do
        "$bin" "$pid" key clear
        sleep 0.7
        screencapture -x -o -l "$(xroar_winid)" "$after"
        if [ "$(md5 -q "$before")" != "$(md5 -q "$after")" ]; then
            printf 'switched after %d press(es)\n' "$i"
            return 0
        fi
    done
    die "cocoscreen: display did not change after 8 CLEAR presses"
}

cmd_cycle() {
    local count="${1:-4}" pid bin i out
    pid=$(need_pid); bin=$(build sendkey)
    mkdir -p "$SHOTDIR"
    focus
    for (( i = 1; i <= count; i++ )); do
        "$bin" "$pid" key clear
        sleep 0.8
        out="$SHOTDIR/cycle-$i.png"
        screencapture -x -o -l "$(xroar_winid)" "$out"
        printf '%s\t%s\n' "$(md5 -q "$out")" "$out"
    done
}

case "${1:-}" in
    shot)  shift; cmd_shot "${1:-}" ;;
    key)   shift; [ $# -gt 0 ] || die "cocoscreen: key needs a key name"
           focus; "$(build sendkey)" "$(need_pid)" key "$@" ;;
    type)  shift; [ $# -gt 0 ] || die "cocoscreen: type needs a string"
           focus; "$(build sendkey)" "$(need_pid)" type "$@" ;;
    clear) focus; "$(build sendkey)" "$(need_pid)" key clear ;;
    next)  cmd_next ;;
    measure) shift; [ $# -gt 0 ] || die "cocoscreen: measure needs a PNG path"
           "$(build measure)" "$@" ;;
    cycle) shift; cmd_cycle "${1:-4}" ;;
    place) shift; cmd_place "${1:-0}" "${2:-0}" ;;
    winid) "$(build winid)" xroar ;;
    pid)   need_pid; printf '\n' ;;
    *)     sed -n '2,30p' "$0" >&2; exit 1 ;;
esac
