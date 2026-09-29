#!/usr/bin/env bash
# verify-image.sh -- prove the self-contained CONF68K disk is actually usable.
#
# Two separate questions, because they fail separately:
#
#   1. Did every file land, byte for byte? Each manifest entry is copied back
#      OFF the image and compared with its source. A directory listing would
#      only prove a NAME is present; this proves the bytes are.
#   2. Does the suite RUN from the image and report the expected verdicts?
#      All 115, checked against DOCS/expected-rbf -- an image is an RBF device,
#      so the record-locking tests must give real verdicts here rather than
#      the SKIPs a host directory produces.
#
# A disk that lists correctly and cannot run is exactly the failure this is
# built to catch.
set -uo pipefail

REPO="$(cd "$(dirname "$0")/../.." && pwd)"
SUITE="$REPO/test/68k-conformance"
OUT="$REPO/build/selfhost68k"
IMG="$OUT/conf68k.dsk"
MANIFEST="$OUT/manifest.txt"
EXE="$REPO/os9exec"
TIMEOUT=$(command -v gtimeout || command -v timeout)
WORK="$OUT/readback"

[ -f "$IMG" ]      || { echo "no image -- run tools/selfhost68k/build-image.sh" >&2; exit 2; }
[ -f "$MANIFEST" ] || { echo "no manifest -- rebuild the image" >&2; exit 2; }

# An image older than the suite is not a broken image, it is yesterday's
# image, and the two fail in ways that look identical: adding CONF68K t103 on
# 2026-09-21 produced five MISMATCH lines and "image contents are wrong",
# which reads as corruption. Say which it is instead. The check names the
# newest offender so the reason is visible, and it is the operator's move
# rather than ours -- rebuilding here would hide a suite edit nobody meant to
# make. (`find -newer` rather than a timestamp comparison: it needs no stat
# format, which differs between BSD and GNU.)
stale=$(find "$SUITE" -type f -newer "$IMG" \
        -not -path "*/RESULTS/*" -not -path "*/SCRATCH/*" \
        -not -path "*/REBUILT/*" -print 2>/dev/null | head -3)
if [ -n "$stale" ]; then
    echo "the image is OLDER than the suite -- it would be verified against files it does not contain:" >&2
    echo "$stale" | sed "s|^$REPO/|  |" >&2
    echo "run tools/selfhost68k/build-image.sh first" >&2
    exit 2
fi

rm -rf "$WORK"; mkdir -p "$WORK"
bad=0; n=0

echo "== 1. every file readable off the image, byte for byte =="
while read -r onimage bytes; do
    n=$((n+1))
    rel=${onimage#/h7/}
    out="$WORK/$(echo "$rel" | tr '/' '_')"
    $TIMEOUT 60 env OS9H7="$IMG" OS9H6="$WORK" "$EXE" \
        -r icopy "$onimage" "/h6/$(basename "$out")" >/dev/null 2>&1
    if ! cmp -s "$out" "$SUITE/$rel"; then
        echo "  MISMATCH  $onimage"
        bad=$((bad+1))
    fi
done < "$MANIFEST"
echo "  $((n-bad))/$n files verified"
[ "$bad" -eq 0 ] || { echo "image contents are wrong"; exit 1; }

echo
echo "== 2. the suite runs from the image =="
# cd into the image's directory: OS9DISK naming an image resolves the boot
# program relative to the host working directory (a real defect, recorded in
# ROADMAP-68k.md). tools/conformance.sh works around it the same way.
printf 'RUN prebuilt\r' > "$SUITE/RESULTS/report"
mods=$(ls "$SUITE/CMDS" | grep -vE '^(tally|mark|run|load|cio)$' | sort)
for m in $mods; do
    line=$( cd "$OUT" && $TIMEOUT 60 env OS9DISK="$IMG" "$EXE" -r "/dd/CMDS/$m" \
            </dev/null 2>&1 | tr '\r' '\n' | grep -a '^RESULT ' )
    [ -n "$line" ] && printf '%s\r' "$line" >> "$SUITE/RESULTS/report"
done

pass=$(tr '\r' '\n' < "$SUITE/RESULTS/report" | grep -c ' PASS ')
fail=$(tr '\r' '\n' < "$SUITE/RESULTS/report" | grep -c ' FAIL ')
skip=$(tr '\r' '\n' < "$SUITE/RESULTS/report" | grep -c ' SKIP ')
erro=$(tr '\r' '\n' < "$SUITE/RESULTS/report" | grep -c ' ERROR ')
echo "  PASS=$pass FAIL=$fail SKIP=$skip ERROR=$erro"

# What the image is expected to do is whatever DOCS/expected-rbf records, not
# "everything passes".  That stopped being the same thing when t49 arrived: it
# measures the `load` utility, which is built from C and therefore carries the
# vendor's runtime, so it is deliberately kept OFF a disk that gets handed to
# somebody -- and t49 correctly reports SKIP there.  Three conditions, each
# catching something a plain count would miss:
#
#   no FAIL, no ERROR            the obvious one
#   pass + skip == every line    every test reported; silence is not a pass
#   pass >= the recorded passes  a PASS that became a SKIP is a regression,
#                                even though the reverse is not
exp=$(tr '\r' '\n' < "$SUITE/DOCS/expected-rbf" | grep -c '^RESULT ')
exp_pass=$(tr '\r' '\n' < "$SUITE/DOCS/expected-rbf" | grep -c ' PASS ')
if [ "$fail" -ne 0 ] || [ "$erro" -ne 0 ] \
   || [ $((pass + skip)) -ne "$exp" ] || [ "$pass" -lt "$exp_pass" ]; then
    echo "  expected $exp results, at least $exp_pass of them PASS, no FAIL or ERROR"
    echo "  -- see $SUITE/RESULTS/report"
    exit 1
fi
echo
echo "SELF-CONTAINED DISK OK -- $n files, $pass PASS + $skip SKIP of $exp tests from the image"
