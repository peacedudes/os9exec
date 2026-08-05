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
#   tools/conformance.sh 68k --noshell  run each test as its own boot program,
#                                       needing no shell and no Microware SDK
#                                       (used automatically when h0/ is absent)
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
         t08seekeof t09bpnum t10exists t11rewind t12size t13evread
         t14evsignl t15evset t16evsetr t17evinfo t18evwaitr
         t19lock t20read t21defr t22wrel t23whol t24rels t25eofl t26zrel
         t27shar t28exts t29self t30delo t31zrdr
         t32ticks t33ctrl t34julian t35cent t36crchi t37crclo tally mark)

# ---------------------------------------------------------------- comparison

# compare <suite-dir> <report-file> [expected-basename]
# Reads RESULT lines from the report, checks them against the suite's
# expected/known-divergences, prints a verdict per anomaly.  Returns 1 if
# anything is news.
#
# The expected file is per DEVICE TYPE, not per suite.  It stopped being one
# file when the record-locking tests arrived: those need real RBF mechanics
# underneath, so they report a verdict on an image and SKIP on a host-native
# directory, and a single expected file would have to call one of the two
# wrong.  Recording both is what keeps SKIP meaningful -- a test that started
# skipping on the image, where it is supposed to run, is then news instead of
# being indistinguishable from its normal behaviour on the other device.
compare() {
    local dir="$1" report="$2" rc=0
    local exp="$dir/DOCS/${3:-expected}" kd="$dir/DOCS/known-divergences"

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

    # A verdict is news when it differs from what was recorded, whatever the
    # two verdicts are -- including a FAIL that became a PASS, which is how
    # fixing a known divergence gets noticed instead of quietly closing a
    # gap nobody is watching.
    #
    # SKIP matching SKIP is the one non-PASS that needs no entry in
    # known-divergences, because it is not a disagreement with the manual: it
    # is a test reporting that this device cannot exercise its claim at all.
    # The record-locking tests do exactly that on a host-native directory.
    # Requiring an explanation for those would put nine permanent entries in a
    # file whose whole job is to stay short enough to read.
    local id verdict want nskip=0
    for id in $ids_exp; do
        verdict=$(awk -v i="$id" '$2==i {print $3}' "$n_got" | tail -1)
        want=$(awk -v i="$id" '$2==i {print $3}' "$n_exp" | tail -1)
        if [ -z "$verdict" ]; then
            printf '  MISSING %s  no RESULT line -- the test died or never ran\n' "$id"
            rc=1
        elif [ "$verdict" != "$want" ]; then
            if grep -qx "$id" "$n_kd" 2>/dev/null; then
                printf '  CHANGED %s  recorded %s, now %s\n' "$id" "$want" "$verdict"
            else
                printf '  NEW     %s  recorded %s, now %s\n' "$id" "$want" "$verdict"
                awk -v i="$id" '$2==i' "$n_got" | sed 's/^/          /'
            fi
            rc=1
        elif [ "$verdict" = PASS ]; then
            :
        elif [ "$verdict" = SKIP ]; then
            nskip=$((nskip + 1))
        elif grep -qx "$id" "$n_kd" 2>/dev/null; then
            printf '  KNOWN   %s  %s, as recorded\n' "$id" "$verdict"
        else
            printf '  NEW     %s  recorded as %s, and nothing explains it\n' "$id" "$verdict"
            awk -v i="$id" '$2==i' "$n_got" | sed 's/^/          /'
            rc=1
        fi
    done
    [ "$nskip" -gt 0 ] && printf '  %d skipped -- this device cannot exercise those claims\n' "$nskip"
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
    # l68 recreates these through the host filesystem as 0644, dropping the
    # execute bit.  That matters on the way OUT: the shipped tree is copied to
    # an OS-9 disk by host tools, and a host mode bit is what becomes the OS-9
    # `e` attribute.  Without this, every --build silently degrades the tree
    # it just rebuilt.  (Module contents are byte-identical across rebuilds --
    # verified -- so this mode flip was the only drift.)
    chmod 755 "$dir"/CMDS/*
    echo "  ${#MODULES[@]} modules built"
}

# Run each test as its own boot program -- no shell, no Microware SDK.
#
# This is the mode that makes the suite runnable by anyone who clones the
# repo. The normal path needs /dd/CMDS/shell to execute runall, and that
# shell is Microware's: it lives in h0/, which is a licensed OS-9 system disk
# outside this repository. Without this mode a recipient of the GitHub tree
# -- or CI -- could not run the 68k suite at all, only read it.
#
# Every module in CMDS/ is our own code, and os9exec can boot a module
# directly, so pointing OS9DISK at the suite itself runs each test with the
# suite as /dd. Relative paths still resolve: the top-level process takes its
# data directory from OS9DISK and its execution directory from OS9DISK/CMDS,
# which is exactly what runall arranges by hand with chd and chx.
#
# It is also a real check in its own right -- it proves no test depends on
# the shell for anything beyond being started.
# Build and populate an RBF image with NO shell and no Microware software.
#
# This is what lets CI check the record-locking claims (t19-t31), which SKIP on
# a host-native directory because there is no lock mechanism under it. It used
# to need a shell for `mount -k`, which is why --rbf was a local-only gate --
# but os9exec runs its own internal commands AS THE BOOT PROGRAM, so the whole
# build is just four invocations of the emulator with no OS-9 system at all.
#
# `mount -k` writes to <startPath>/<name>, and startPath is the emulator's
# working directory -- hence the subshell cd. The name has to be h0..hz.
# Echoes the image path on success; returns non-zero having said why.
build_rbf_image_noshell() {
    local dir="$1" work="$2" img="$work/h7" m
    ( cd "$work" && $TIMEOUT 60 "$REPO/os9exec" -r mount -k=800k h7 ) >/dev/null 2>&1
    [ -f "$img" ] || { echo "  could not create $img" >&2; return 1; }

    for m in CMDS SCRATCH RESULTS; do
        $TIMEOUT 60 env OS9H7="$img" "$REPO/os9exec" -r makdir "/h7/$m" >/dev/null 2>&1
    done
    for m in "${MODULES[@]}"; do
        $TIMEOUT 60 env OS9H7="$img" OS9H8="$dir" "$REPO/os9exec" \
            -r copy -n "/h8/CMDS/$m" "/h7/CMDS/$m" >/dev/null 2>&1
    done

    # Prove the copy actually landed rather than trusting four silent runs --
    # every one of those redirects to /dev/null, so a total failure would
    # otherwise reach the tests as "everything SKIPped", which reads like a
    # device limitation instead of a broken build.
    if ! $TIMEOUT 60 env OS9H7="$img" "$REPO/os9exec" -r dir /h7/CMDS </dev/null 2>&1 \
         | tr '\r' '\n' | grep -aq 'tally'; then
        echo "  image built but CMDS/ is empty -- populate step failed" >&2
        return 1
    fi
    echo "$img"
}

run_68k_noshell() {
    local use_rbf="${1:-no}" dir="$REPO/test/68k-conformance" rc=0 m out
    local disk="$dir" work="" img=""

    if [ "$use_rbf" = yes ]; then
        work=$(mktemp -d)
        img=$(build_rbf_image_noshell "$dir" "$work") || { rm -rf "$work"; return 1; }
        disk="$img"
    fi

    echo "== CONF68K on os9exec (no shell, no SDK -- each test as its own boot program$([ "$use_rbf" = yes ] && echo ', RBF image')) =="
    printf 'RUN prebuilt\r' > "$dir/RESULTS/report"
    for m in "${MODULES[@]}"; do
        case "$m" in tally|mark) continue ;; esac
        out=$($TIMEOUT 60 env OS9DISK="$disk" "$REPO/os9exec" -r "/dd/CMDS/$m" </dev/null 2>&1 \
              | tr '\r' '\n' | grep -a '^RESULT ')
        [ -n "$out" ] && printf '%s\r' "$out" >> "$dir/RESULTS/report"
    done
    # tally is a module too, so it reads the report the same way it would on
    # a real system rather than being reimplemented here in shell. Always run
    # it against the SUITE DIRECTORY, never the image: the report it counts is
    # the host-side one this loop just wrote, and pointing /dd at the image
    # instead had it read the image's own empty RESULTS/ and print nothing at
    # all -- a missing totals line, with every test having passed.
    $TIMEOUT 60 env OS9DISK="$dir" "$REPO/os9exec" -r /dd/CMDS/tally </dev/null 2>&1 \
        | tr '\r' '\n' | grep -a 'CONF68K totals' | sed 's/^/  /'
    compare "$dir" "$dir/RESULTS/report" \
        "$([ "$use_rbf" = yes ] && echo expected-rbf || echo expected)" || rc=1
    [ -n "$work" ] && rm -rf "$work"
    return $rc
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
    compare "$dir" "$dir/RESULTS/report" \
        "$([ "$use_rbf" = yes ] && echo expected-rbf || echo expected)" || rc=1
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
use_rbf=no; do_build=no; no_shell=no
for a in "$@"; do
    case "$a" in
        --rbf)     use_rbf=yes ;;
        --noshell) no_shell=yes ;;
        --build) do_build=yes ;;
        *) echo "conformance: unknown option $a" >&2; exit 2 ;;
    esac
done

overall=0
case "$which" in
    68k)  [ "$do_build" = yes ] && { build_68k "$REPO/test/68k-conformance" || exit 1; }
          # No h0 means no Microware shell to run runall with. Fall back
          # rather than fail: a fresh clone has no system disk, and the
          # suite does not actually need one.
          if [ ! -d "$REPO/h0/CMDS" ] && [ "$no_shell" = no ]; then
              echo "note: no h0/CMDS system disk found -- using --noshell"
              no_shell=yes
          fi
          if [ "$no_shell" = yes ]; then run_68k_noshell "$use_rbf" || overall=1
          else run_68k "$use_rbf" || overall=1; fi ;;
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
