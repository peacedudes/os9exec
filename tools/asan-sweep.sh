#!/usr/bin/env bash
# asan-sweep.sh -- run the conformance suite under the ADDRESS sanitizer.
#
# Covers the class tools/ubsan-sweep.sh does not. That one is
# -fsanitize=alignment,undefined: it finds misaligned access and undefined
# behaviour, and says nothing at all about the heap. A buffer overrun, a
# use-after-free or a stack-buffer-overflow passes every gate this project
# had -- which matters here more than in most C, because os9exec hands out
# host pointers into an emulated arena and writes 68k structures through
# them.
#
# Added 2026-09-21 after a full run of the 337-test suite under ASan came
# back clean: worth having as a standing check rather than a thing somebody
# once did by hand. Existing ASan use in the tree is narrower on purpose --
# asan-args-sweep.sh fuzzes the command line, fuzz-module.sh and
# fuzz-rbf-image.sh fuzz their inputs -- and none of them runs the suite.
#
#   tools/asan-sweep.sh            both conformance legs, committed tree
#   tools/asan-sweep.sh --dirty    sweep the WORKING tree instead of HEAD
#
# Builds in a scratch export, never in the working tree: a sanitizer binary
# must not become the ./os9exec another session is dogfooding (project memory
# `feedback_never-clean-the-dogfood-tree`).
set -uo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
WORK=${TMPDIR:-/tmp}/os9-asan-$$
DIRTY=no
[ "${1:-}" = "--dirty" ] && DIRTY=yes

trap 'rm -rf "$WORK"' EXIT
mkdir -p "$WORK" || { echo "could not make a scratch tree"; exit 1; }

if [ "$DIRTY" = yes ]; then
    ( cd "$REPO" && COPYFILE_DISABLE=1 tar -cf - Source GNUmakefile tools test ) \
        | tar -x -C "$WORK" || { echo "could not copy the working tree"; exit 1; }
else
    ( cd "$REPO" && git archive HEAD ) | tar -x -C "$WORK" \
        || { echo "could not export HEAD"; exit 1; }
fi

SAN="-fsanitize=address -fsanitize-ignorelist=$WORK/tools/ubsan-ignore.txt"
cd "$WORK" || exit 1

echo "== building with $SAN =="
make -B -j4 CC="cc $SAN -g -O1 -fno-omit-frame-pointer" >/dev/null 2>&1 \
  || { echo "  build FAILED"; exit 1; }
[ -x ./os9exec ] || { echo "  no binary was produced"; exit 1; }

# PROVE THE SANITIZER IS ARMED before believing a clean run. A build where
# -fsanitize quietly did not apply reports nothing and looks exactly like a
# clean one -- the vacuous check this project keeps finding. Two independent
# confirmations, same as the ubsan sweep: the runtime's symbols are linked
# in, and a real heap overflow actually reports.
n=$(nm -u ./os9exec 2>/dev/null | grep -c "__asan")
[ "${n:-0}" -gt 0 ] || { echo "  the binary has NO __asan symbols -- the sanitizer did not apply"; exit 1; }

# -O0 and a value that is really used: at -O1 the compiler can fold the read
# away and the control then reports nothing, failing a sweep that was fine.
cat > "$WORK/armed.c" <<'CEOF'
#include <stdio.h>
#include <stdlib.h>
int main(void) {
    char *buf = malloc(16);
    buf[16] = 'x';                      /* one past the end, on purpose */
    printf("%d\n", buf[16]);
    return 0;
}
CEOF
cc $SAN -g -O0 -o "$WORK/armed" "$WORK/armed.c" 2>/dev/null \
  || { echo "  could not build the positive control"; exit 1; }
ctl=$("$WORK/armed" 2>&1)
case "$ctl" in
    *"heap-buffer-overflow"*) ;;
    *) echo "  the positive control did NOT report -- this sweep could not fail"
       echo "  it printed: ${ctl:-<nothing>}"
       exit 1 ;;
esac
echo "  sanitizer armed ($n __asan symbols; positive control reports)"

# detect_leaks=0: LeakSanitizer is a different question from memory safety and
# is not supported on macOS/arm64 at all, so asking for it would fail the
# sweep on the machine it is most often run from. log_path keeps reports out
# of the suite's own pipelines, which filter and tail past anything on stderr.
rc=0
for mode in "" "--rbf"; do
    out="$WORK/report$(echo "$mode" | tr -d ' -')"
    ASAN_OPTIONS="detect_leaks=0,log_path=$out" \
        ./tools/conformance.sh 68k --noshell $mode > "$WORK/conf.out" 2>&1
    st=$?
    grep -E "CONF68K totals|CONFORMANCE" "$WORK/conf.out" | sed 's/^/  /'
    [ $st -eq 0 ] || rc=1
    # Content, not existence. ASan's log_path leaves a file per PROCESS, and
    # the suite starts hundreds of them: on a clean run those files are there
    # and empty, so testing `ls` alone reported "SANITIZER REPORTS:" with
    # nothing under it and failed a sweep that had found nothing. Measured
    # 2026-09-21, first clean run of this tool.
    # shellcheck disable=SC2086
    if grep -lq "ERROR: AddressSanitizer" $out.* 2>/dev/null; then
        echo "  SANITIZER REPORTS:"
        grep -h -A 20 "ERROR: AddressSanitizer" $out.* | sed 's/^/    /'
        rc=1
    else
        echo "  no sanitizer reports"
    fi
done

# The conformance legs above run 68k MODULES: they exercise the syscall
# surface hard, and a lot of host-side code not at all. Measured while writing
# this, by injecting a one-past-the-end write into DirHasEntries (file_rbf.c,
# reached only by deldir): both legs reported OK. A sweep that cannot see a
# real overflow is the vacuous check this project keeps finding, so the
# 337-test suite runs too -- it is what covers deldir, pipes, /tN and the
# networking paths. It needs the system disk, so it is skipped, LOUDLY, when
# there is none rather than quietly narrowing what this sweep claims.
if [ -n "${OS9DISK:-}" ]; then
    out="$WORK/reportsuite"
    echo "== the full test suite =="
    ASAN_OPTIONS="detect_leaks=0,log_path=$out" \
        swift run --package-path test OS9Tests > "$WORK/suite.out" 2>&1
    st=$?
    grep -aE "^FAIL:|Results" "$WORK/suite.out" | sed 's/^/  /'
    [ $st -eq 0 ] || rc=1
    # Content, not existence. ASan's log_path leaves a file per PROCESS, and
    # the suite starts hundreds of them: on a clean run those files are there
    # and empty, so testing `ls` alone reported "SANITIZER REPORTS:" with
    # nothing under it and failed a sweep that had found nothing. Measured
    # 2026-09-21, first clean run of this tool.
    # shellcheck disable=SC2086
    if grep -lq "ERROR: AddressSanitizer" $out.* 2>/dev/null; then
        echo "  SANITIZER REPORTS:"
        grep -h -A 20 "ERROR: AddressSanitizer" $out.* | sed 's/^/    /'
        rc=1
    else
        echo "  no sanitizer reports"
    fi
else
    echo "== the full test suite: SKIPPED, no OS9DISK =="
    echo "   the conformance legs alone do NOT cover deldir, pipes, /tN or"
    echo "   networking; set OS9DISK to sweep those too"
fi

[ $rc -eq 0 ] && echo "ASAN SWEEP OK -- no heap or stack memory errors"
exit $rc
