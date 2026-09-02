#!/usr/bin/env bash
# ubsan-sweep.sh -- run the conformance suite under the UB sanitizer.
#
# Chiefly for ALIGNMENT. os9exec reads and writes 68k structures out of a byte
# array, so a host that traps on unaligned access is where such a bug surfaces
# -- and the only machine in the fleet that does is sparc64, whose emulated IDE
# is unreliable enough that a leg on it can take hours or hang outright (see
# project memory `sparc64-one-vcpu-and-the-ata-storm`). `-fsanitize=alignment`
# finds the same class here in minutes, deterministically, and reports the
# exact line. It does NOT replace real big-endian coverage: s390x does that.
#
#   tools/ubsan-sweep.sh            both conformance legs, committed tree
#   tools/ubsan-sweep.sh --dirty    sweep the WORKING tree instead of HEAD
#
# Builds in a scratch export, never in the working tree: a sanitizer binary
# must not become the ./os9exec another session is dogfooding (project memory
# `feedback_never-clean-the-dogfood-tree`).
set -uo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
WORK=${TMPDIR:-/tmp}/os9-ubsan-$$
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

SAN="-fsanitize=alignment,undefined -fsanitize-ignorelist=$WORK/tools/ubsan-ignore.txt"
cd "$WORK" || exit 1

echo "== building with $SAN =="
make -B -j4 CC="cc $SAN -g -O1" >/dev/null 2>&1 \
  || { echo "  build FAILED"; exit 1; }
[ -x ./os9exec ] || { echo "  no binary was produced"; exit 1; }

# PROVE THE SANITIZER IS ARMED before believing a clean run. A build where
# -fsanitize quietly did not apply reports nothing and looks identical to a
# clean one -- the vacuous check this project keeps finding. Two independent
# confirmations: the runtime's symbols are linked in, and a deliberately
# misaligned load actually reports.
n=$(nm -u ./os9exec 2>/dev/null | grep -c "__ubsan")
[ "${n:-0}" -gt 0 ] || { echo "  the binary has NO __ubsan symbols -- the sanitizer did not apply"; exit 1; }

# -O0 and a HEAP buffer whose value is really used, on purpose. Two ways this
# control quietly stops working, both hit while writing it: at -O1 the
# compiler folds a load from a static zeroed buffer away entirely, and marking
# anything `volatile` makes UBSan skip the alignment check altogether. Either
# way it reports nothing and this guard fails a sweep that was fine.
cat > "$WORK/armed.c" <<'EOF'
#include <stdio.h>
#include <stdlib.h>
int main(void) {
    char *buf = malloc(16);
    int *p = (int *)(buf + 1);          /* deliberately misaligned */
    printf("%d\n", *p);
    return 0;
}
EOF
cc $SAN -g -O0 -o "$WORK/armed" "$WORK/armed.c" 2>/dev/null \
  || { echo "  could not build the positive control"; exit 1; }
ctl=$("$WORK/armed" 2>&1)
case "$ctl" in
    *"misaligned address"*) ;;
    *) echo "  the positive control did NOT report -- this sweep could not fail"
       echo "  it printed: ${ctl:-<nothing>}"
       exit 1 ;;
esac
echo "  sanitizer armed ($n __ubsan symbols; positive control reports)"

# log_path keeps reports out of the suite's own pipelines, which filter and
# tail their way past anything on stderr.
rc=0
for mode in "" "--rbf"; do
    out="$WORK/report$(echo "$mode" | tr -d ' -')"
    UBSAN_OPTIONS="print_stacktrace=1,log_path=$out" \
        ./tools/conformance.sh 68k --noshell $mode > "$WORK/conf.out" 2>&1
    st=$?
    grep -E "CONF68K totals|CONFORMANCE" "$WORK/conf.out" | sed 's/^/  /'
    [ $st -eq 0 ] || rc=1
    # shellcheck disable=SC2086
    if ls $out.* >/dev/null 2>&1; then
        echo "  SANITIZER REPORTS:"
        cat $out.* | sed 's/^/    /'
        rc=1
    else
        echo "  no sanitizer reports"
    fi
done

[ $rc -eq 0 ] && echo "UBSAN SWEEP OK -- no misalignment, no undefined behaviour"
exit $rc
