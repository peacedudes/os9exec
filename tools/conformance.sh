#!/usr/bin/env bash
# conformance.sh -- run a guest conformance suite and CHECK its results.
#
# Both suites could already be run by hand, and the 6809 one could be run by
# tools/selfhost6809/run-on-nitros9.sh.  Neither could FAIL: the guest wrote
# RESULT lines, a human read them, and nothing compared them to anything.  A
# suite whose results are never checked is a suite that cannot report a
# regression, which is most of the value of having it.
#
# This is that check.  For each suite:
#
#   DOCS/expected           the results we have actually recorded, one
#                           RESULT line per test (real observations, not an
#                           invented ideal)
#   DOCS/known-divergences  test IDs already investigated, with reasons
#
# A run is OK when every test in DOCS/expected reported PASS, except for the
# IDs in known-divergences, which must report the verdict recorded in
# expected.  Anything else is news and exits non-zero:
#
#   NEW      a test that should pass did not, and nobody has explained it
#   CHANGED  a known divergence behaves differently than recorded
#   MISSING  a test in expected produced no RESULT line at all -- it died,
#            or never ran.  Silence is not a pass.
#   EXTRA    a RESULT line for a test not in expected: the suite grew and
#            expected was not updated
#
# Usage:
#   tools/conformance.sh 68k            run the 68k suite on os9exec
#   tools/conformance.sh 68k --rbf      ... from a freshly built RBF image
#                                       instead of a host-native directory
#   tools/conformance.sh 68k --build    rebuild the modules from SRC first
#   tools/conformance.sh 6809           run the 6809 suite under XRoar
#   tools/conformance.sh all            both
#
# --rbf matters more than it looks.  A host-native directory is a shim with
# no real RBF underneath it, so a claim about file-system behaviour can pass
# there and fail on a real image, or the reverse.  t10 did exactly that: it
# passed on an RBF image and failed on a host directory, which is how the
# host-native I$Create defect was found.  When in doubt, run both.
set -uo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
TIMEOUT=$(command -v gtimeout || command -v timeout)
[ -n "$TIMEOUT" ] || { echo "conformance: need gtimeout or timeout on PATH" >&2; exit 2; }

MODULES=(t01open t02eof t03bmode t04mode0 t05mode0rd t06short t07extend
         t08seekeof t09bpnum t10exists t11rewind t12size tally mark)

# ---------------------------------------------------------------- comparison

# compare <suite-dir> <report-file>
# Reads RESULT lines from the report, checks them against the suite's
# expected/known-divergences, prints a verdict per anomaly.  Returns 1 if
# anything is news.
compare() {
    local dir="$1" report="$2" rc=0
    local exp="$dir/DOCS/expected" kd="$dir/DOCS/known-divergences"

    [ -f "$exp" ] || { echo "  no DOCS/expected -- nothing to check against" >&2; return 2; }

    # CR-only files: normalise before any line-oriented tool touches them.
    # Without this every one of these greps sees a single enormous line and
    # silently matches nothing, which reads exactly like a clean run.
    local n_exp n_got n_kd
    n_exp=$(mktemp); n_got=$(mktemp); n_kd=$(mktemp)
    tr '\r' '\n' < "$exp" | grep '^RESULT ' > "$n_exp"
    tr '\r' '\n' < "$report" | grep '^RESULT ' > "$n_got"
    if [ -f "$kd" ]; then tr '\r' '\n' < "$kd" | awk '/^t[0-9]/ {print $1}' | sort -u > "$n_kd"; fi

    local ids_exp ids_got
    ids_exp=$(awk '{print $2}' "$n_exp" | sort -u)
    ids_got=$(awk '{print $2}' "$n_got" | sort -u)

    local id verdict want
    for id in $ids_exp; do
        verdict=$(awk -v i="$id" '$2==i {print $3}' "$n_got" | tail -1)
        want=$(awk -v i="$id" '$2==i {print $3}' "$n_exp" | tail -1)
        if [ -z "$verdict" ]; then
            printf '  MISSING %s  no RESULT line -- the test died or never ran\n' "$id"
            rc=1
        elif grep -qx "$id" "$n_kd" 2>/dev/null; then
            if [ "$verdict" = "$want" ]; then
                printf '  KNOWN   %s  %s, as recorded\n' "$id" "$verdict"
            else
                printf '  CHANGED %s  recorded %s, now %s\n' "$id" "$want" "$verdict"
                rc=1
            fi
        elif [ "$verdict" != PASS ]; then
            printf '  NEW     %s  %s\n' "$id" "$verdict"
            awk -v i="$id" '$2==i' "$n_got" | sed 's/^/          /'
            rc=1
        fi
    done
    for id in $ids_got; do
        grep -qx "$id" <<<"$ids_exp" || {
            printf '  EXTRA   %s  reported but not in DOCS/expected\n' "$id"; rc=1; }
    done

    printf '  %d of %d tests reported; %s\n' \
        "$(wc -l < "$n_got" | tr -d ' ')" "$(wc -l < "$n_exp" | tr -d ' ')" \
        "$([ $rc -eq 0 ] && echo 'no news' || echo 'see above')"
    rm -f "$n_exp" "$n_got" "$n_kd"
    return $rc
}

# ------------------------------------------------------------------ 68k arm

os9exec_shell() {   # feed stdin to a throwaway os9exec shell
    local extra_h8="$1"; shift
    (cd "${1:-$REPO}" && $TIMEOUT 300 env OS9STOP=1 \
        OS9DISK="$REPO/h0" OS9H8="$extra_h8" "$REPO/os9exec" -r /dd/CMDS/shell 2>&1) \
        | tr '\r' '\n'
}

build_68k() {
    local dir="$1" n
    echo "building modules from SRC/"
    rm -f "$dir"/SCRATCH/*.r
    { echo "chd /h8/SRC"
      for n in "${MODULES[@]}"; do
          echo "r68 $n.a -o=/h8/SCRATCH/$n.r"
          echo "l68 /h8/SCRATCH/$n.r -o=/h8/CMDS/$n"
      done
      echo stop
    } | os9exec_shell "$dir" > /tmp/conf68k-build.log
    # r68's "destination in short branch range" is advisory; only a nonzero
    # error count means the module is not there.
    if grep -aq 'Errors: 0000[1-9]' /tmp/conf68k-build.log; then
        echo "  assembly errors -- see /tmp/conf68k-build.log" >&2; return 1
    fi
    rm -f "$dir"/SCRATCH/*.r
    for n in "${MODULES[@]}"; do
        [ -s "$dir/CMDS/$n" ] || { echo "  $n did not build" >&2; return 1; }
    done
    echo "  ${#MODULES[@]} modules built"
}

run_68k() {
    local use_rbf="$1" dir="$REPO/test/68k-conformance" rc=0
    echo "== CONF68K on os9exec ($([ "$use_rbf" = yes ] && echo 'RBF image' || echo 'host-native directory')) =="
    rm -f "$dir/RESULTS/report"

    if [ "$use_rbf" = yes ]; then
        # Build a blank RBF image inside the guest and copy the suite onto
        # it, so the tests run against real RBF mechanics rather than the
        # host-directory shim.
        local work n; work=$(mktemp -d)
        { echo "mount -k=500k h7"
          echo "makdir /h7/CMDS"; echo "makdir /h7/SCRATCH"; echo "makdir /h7/RESULTS"
          for n in "${MODULES[@]}"; do echo "copy -n /h8/CMDS/$n /h7/CMDS/$n"; done
          echo "copy -n /h8/runall /h7/runall"
          echo "chd /h7"; echo "chx /h7/CMDS"; echo "runall"
          # copy by ABSOLUTE path: chx now points at the suite's own CMDS,
          # so a bare "copy" is no longer findable.  This is the same wart
          # the suite's readme warns operators about, and it bit this script
          # first -- which is the argument for the warning being there.
          echo "/dd/CMDS/copy -n /h7/RESULTS/report /h8/RESULTS/report"
          echo stop
        } | os9exec_shell "$dir" "$work" > /tmp/conf68k-run.log
        rm -rf "$work"
    else
        printf 'chd /h8\nchx /h8/CMDS\nrunall\nstop\n' \
            | os9exec_shell "$dir" > /tmp/conf68k-run.log
    fi

    grep -a 'CONF68K totals' /tmp/conf68k-run.log | sed 's/^/  /'
    if [ ! -s "$dir/RESULTS/report" ]; then
        echo "  no RESULTS/report was produced -- see /tmp/conf68k-run.log" >&2
        return 1
    fi
    compare "$dir" "$dir/RESULTS/report" || rc=1
    return $rc
}

# ----------------------------------------------------------------- 6809 arm

run_6809() {
    local dir="$REPO/test/6809-conformance" rc=0
    echo "== CONF6809 on NitrOS-9 under XRoar =="
    local runner="$REPO/tools/selfhost6809/run-on-nitros9.sh"
    [ -x "$runner" ] || { echo "  $runner not executable" >&2; return 2; }
    local out; out=$(mktemp)
    if ! "$runner" > "$out" 2>&1; then
        echo "  runner failed -- output follows" >&2
        tail -20 "$out" | sed 's/^/  /' >&2
        rm -f "$out"; return 1
    fi
    grep -a 'CONF6809 totals' "$out" | sed 's/^/  /'
    compare "$dir" "$out" || rc=1
    rm -f "$out"
    return $rc
}

# --------------------------------------------------------------------- main

which=${1:-all}; shift 2>/dev/null || true
use_rbf=no; do_build=no
for a in "$@"; do
    case "$a" in
        --rbf)   use_rbf=yes ;;
        --build) do_build=yes ;;
        *) echo "conformance: unknown option $a" >&2; exit 2 ;;
    esac
done

overall=0
case "$which" in
    68k)  [ "$do_build" = yes ] && { build_68k "$REPO/test/68k-conformance" || exit 1; }
          run_68k "$use_rbf" || overall=1 ;;
    6809) run_6809 || overall=1 ;;
    all)  [ "$do_build" = yes ] && { build_68k "$REPO/test/68k-conformance" || exit 1; }
          run_68k "$use_rbf" || overall=1
          run_6809 || overall=1 ;;
    *) sed -n '2,40p' "$0" >&2; exit 2 ;;
esac

echo
if [ $overall -eq 0 ]; then echo "CONFORMANCE OK -- no unexplained divergence"
else echo "CONFORMANCE: divergence reported above"; fi
exit $overall
