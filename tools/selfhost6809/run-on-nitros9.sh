#!/usr/bin/env bash
# run-on-nitros9.sh [image] [--rebuild] -- serve a built 6809 conformance
# image to a live NitrOS-9 guest over DriveWire, run the suite as this
# disk's ordinary non-super login, and print RESULTS/report back exactly as
# retrieved through the guest.
#
# Wraps tools/nitros9repl.sh, adding NITROS9REPL_EXTRA_DWCLI="--disk0
# <image>" so the guest sees the conformance image as /X0 (rbdw driver),
# same recipe as the RBF-over-DriveWire proof this project already did --
# see the os9-dev skill's 6809/using-nitros9-repl.md.
#
# The report is retrieved with `list` over the live channel, never by
# reading the image file on the host: a --disk0 image served over
# DriveWire is not a host-side retrieval route (writes the guest makes
# were not visible in the backing file on the host, checked with two
# independent tools -- see the same skill section). Reading the backing
# file here would silently show a stale or empty report.
#
# This tool is for the maintainer's own live verification, not for the
# recipient of the disk -- they run the three commands in text/readme
# from their own OS-9 shell.
#
# Usage:
#   run-on-nitros9.sh [image] [--rebuild]
#
# image defaults to build/selfhost6809/conf6809.dsk (build-image.sh's
# output). --rebuild additionally runs "rebuild" after "runall" and
# re-fetches the report, so both runs land in one call.
#
# The golden NitrOS-9 .ide this boots may be shared with other sessions
# (it is a real file XRoar edits in place -- see the skill's "Concurrent
# sessions need a private disk clone" note). This script clones it into a
# private scratch directory by default, unless the caller has already set
# NITROS9REPL_DISKDIR, and deletes the clone when done. Set
# NITROS9REPL_KEEP_CLONE=1 to keep it for inspection.
set -uo pipefail

REPO="$(cd "$(dirname "$0")/../.." && pwd)"
REPL="$REPO/tools/nitros9repl.sh"

IMG="$REPO/build/selfhost6809/conf6809.dsk"
DO_REBUILD=0
for arg in "$@"; do
    case "$arg" in
        --rebuild) DO_REBUILD=1 ;;
        *)         IMG="$arg" ;;
    esac
done

[ -f "$IMG" ] || {
    printf 'run-on-nitros9: image not found: %s (run build-image.sh first)\n' "$IMG" >&2
    exit 1
}

# Same golden disk tools/nitros9repl.sh defaults to; only used here to know
# what to clone, not passed to the REPL directly.
NITROS9="${NITROS9_DIR:-$REPO/../os9/nitros9}"
GOLDEN="${NITROS9REPL_GOLDEN_DISKDIR:-$NITROS9/disk-images/eou_ide-v0.3-6809-xroar-dw-becker}"

CLONEDIR=""
cleanup() {
    "$REPL" stop >/dev/null 2>&1 || true
    if [ -n "$CLONEDIR" ] && [ -z "${NITROS9REPL_KEEP_CLONE:-}" ]; then
        rm -rf "$CLONEDIR"
    fi
}
trap cleanup EXIT

if [ -z "${NITROS9REPL_DISKDIR:-}" ]; then
    [ -f "$GOLDEN/68IDE.ide" ] || {
        printf 'run-on-nitros9: golden disk not found: %s\n' "$GOLDEN/68IDE.ide" >&2
        exit 1
    }
    CLONEDIR=$(mktemp -d "${TMPDIR:-/tmp}/conf6809-guest.XXXXXX")
    printf '[cloning golden disk into %s -- the golden .ide is never touched]\n' "$CLONEDIR"
    cp -c "$GOLDEN/68IDE.ide" "$CLONEDIR/68IDE.ide" 2>/dev/null \
        || cp "$GOLDEN/68IDE.ide" "$CLONEDIR/68IDE.ide"
    cp "$GOLDEN/hdblba.rom" "$CLONEDIR/hdblba.rom"
    export NITROS9REPL_DISKDIR="$CLONEDIR"
fi

# Unique per invocation, so a run of this script never collides with another
# session's tmux session, disk clone or ports (see feedback_sequential-
# agents-not-parallel.md and the skill's concurrency notes). Callers running
# more than one of these at once must still pick distinct ports themselves.
export NITROS9REPL_SESSION="${NITROS9REPL_SESSION:-conf6809run-$$}"
export NITROS9REPL_CHAN_PORT="${NITROS9REPL_CHAN_PORT:-6829}"
export NITROS9REPL_BECKER_PORT="${NITROS9REPL_BECKER_PORT:-65529}"
export NITROS9REPL_EXTRA_DWCLI="--disk0 $IMG ${NITROS9REPL_EXTRA_DWCLI:-}"
export NITROS9REPL_EXTRA_XROAR="-no-ratelimit ${NITROS9REPL_EXTRA_XROAR:-}"
# nitros9repl.sh's own default (USER1) is already this disk's ordinary
# non-super login (flat ID 1, not the boot identity's 0) -- left as the
# default rather than overridden here, so the account actually used is
# whatever the caller's environment already says.
LOGIN_USER="${NITROS9REPL_USER:-USER1}"
export NITROS9REPL_USER="$LOGIN_USER"
# runall drives twelve tests plus tally, four of which wait on a helper
# process; `rebuild` additionally runs 5 assemblies and 12 basic09 PACKs in
# one command. Give both far more room than a one-line command needs. Cheap:
# this only lengthens the wait on a genuine hang.
export NITROS9REPL_TIMEOUT="${NITROS9REPL_TIMEOUT:-900}"

printf '[image: %s]\n[session: %s  login: %s (non-super)]\n' "$IMG" "$NITROS9REPL_SESSION" "$LOGIN_USER"

"$REPL" start || exit 1

# The load lines matter, not cosmetic: this account has no PATH, so once
# chx points command lookup at /x0/CMDS, "procs" and RunB's "tally" become
# unfindable unless already resident -- see text/readme and task-4-report.md.
"$REPL" send "chd /x0"       || exit 1
"$REPL" send "load procs"    || exit 1
"$REPL" send "load runb"     || exit 1
"$REPL" send "chx /x0/CMDS"  || exit 1
printf '[running runall -- this drives every test plus tally]\n'
"$REPL" send "runall" || exit 1

printf '\n[RESULTS/report, retrieved through the guest with list]\n'
report=$("$REPL" send "list /x0/RESULTS/report") || exit 1
printf '%s\n' "$report"

if [ "$DO_REBUILD" -eq 1 ]; then
    printf '\n[running rebuild]\n'
    "$REPL" send "rebuild" || exit 1
    printf '\n[RESULTS/report after rebuild, retrieved through the guest with list]\n'
    report=$("$REPL" send "list /x0/RESULTS/report") || exit 1
    printf '%s\n' "$report"
fi
