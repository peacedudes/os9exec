#!/usr/bin/env bash
# verify.sh -- one button. Runs every gate this project has and prints ONE
# verdict at the end.
#
# It exists because the gates were eight separate commands with no summary, and
# the predictable happened twice in one week: `make test-linux` sat at 11 red
# for however long nobody ran it, and the RBF conformance leg had never passed
# in CI once since the day it was added. Neither was a hard problem. Both were
# invisible because running everything meant remembering everything.
#
#   tools/verify.sh            the local gates (macOS/host + docker)
#   tools/verify.sh --vms      those, plus the UTM virtual machines
#   tools/verify.sh --quick    skip the slow ones (no docker, no VMs)
#
# Every stage prints its own headline result, and the exit status is non-zero
# if ANY stage failed -- so this is usable as a pre-commit gate, not just as
# something to read.
#
# It deliberately does NOT `make clean`, and never rebuilds ./os9exec into the
# repo root except once at the start: that binary is dogfooded, and a sweep
# that leaves the tree bare for ten minutes is worse than no sweep.
set -uo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO"

want_docker=yes; want_vms=no
for a in "$@"; do
    case "$a" in
        --vms)   want_vms=yes ;;
        --quick) want_docker=no ;;
        -h|--help) sed -n '2,20p' "$0"; exit 0 ;;
        *) echo "verify: unknown option '$a'" >&2; exit 2 ;;
    esac
done

PASSES=(); FAILS=(); SKIPS=()

# stage <name> <command...>
# Runs a gate, keeps its output in a log, prints one line either way. The log
# path is printed on failure -- a summary that cannot be followed back to the
# actual output is a summary nobody trusts.
stage() {
    local name="$1"; shift
    # mktemp needs its X's at the END of the template on BSD/macOS -- with a
    # suffix after them the name is taken LITERALLY, so every stage shared one
    # file and a failure pointed at the wrong log. Found by reading it.
    local log; log=$(mktemp "/tmp/verify-XXXXXX")
    printf '  %-46s ' "$name"
    if "$@" >"$log" 2>&1; then
        printf 'ok\n'
        PASSES+=("$name")
        rm -f "$log"
    else
        printf 'FAILED   (%s)\n' "$log"
        FAILS+=("$name")
    fi
}

skip() { printf '  %-46s skipped (%s)\n' "$1" "$2"; SKIPS+=("$1"); }

echo "== os9exec verify =="
echo "   $(uname -s) $(uname -m), $(git rev-parse --short HEAD 2>/dev/null || echo '?')"
echo

echo "-- build and static checks"
stage "native build (warning-free)"      make -B
stage "build the self-contained disk"     tools/selfhost68k/build-image.sh
stage "warning sweep, 4 toolchains, -O2" tools/verify-warnings.sh

echo "-- the emulator, on this machine"
stage "integration suite (tick on)"      make test
stage "integration suite (tick off)"     make test-notick
stage "CONF68K, host-native directory"   tools/conformance.sh 68k
stage "CONF68K, real RBF image"          tools/conformance.sh 68k --noshell --rbf
stage "live-verification corpus"         make live-verify
stage "RBF integrity hammer (gate)"      make hammer
stage "self-contained CONF68K disk"      tools/selfhost68k/verify-image.sh

if [ "$want_docker" = yes ]; then
    echo "-- other operating systems"
    if docker info >/dev/null 2>&1; then
        stage "integration suite on Linux (docker)" make test-linux
    else
        skip "integration suite on Linux (docker)" "docker not running"
    fi
else
    skip "integration suite on Linux (docker)" "--quick"
fi

if [ "$want_vms" = yes ]; then
    echo "-- real machines (UTM)"
    if command -v utmctl >/dev/null 2>&1; then
        stage "Windows 11 ARM64 (UTM VM)" tools/verify-winvm.sh
    else
        skip "Windows 11 ARM64 (UTM VM)" "utmctl not installed"
    fi
else
    skip "UTM virtual machines" "pass --vms to include"
fi

echo
echo "== ${#PASSES[@]} passed, ${#FAILS[@]} failed, ${#SKIPS[@]} skipped =="
if [ "${#FAILS[@]}" -gt 0 ]; then
    printf '   FAILED: %s\n' "${FAILS[@]}"
    exit 1
fi
echo "   all gates green"
