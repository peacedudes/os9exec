#!/usr/bin/env bash
# asan-args-sweep.sh -- feed over-long arguments to every internal command, and
# to os9exec's own command line and environment, under AddressSanitizer.
#
# Internal commands are host C that take their arguments raw: guest system
# calls arrive bounded (icalls.c truncates names to OS9PATHLEN), internal
# commands and the host command line never did. The first run of this sweep
# (2026-09-19) found eight stack overflows that way -- move, rename, mount,
# idbg -o, and the program name itself -- each of which killed the emulator
# outright on a 450-character argument.
#
#   tools/asan-args-sweep.sh            sweep the committed tree
#   tools/asan-args-sweep.sh --dirty    sweep the WORKING tree instead of HEAD
#
# Every case boots the command directly (`os9exec -r <cmd> <args>`), so no
# system disk is needed. Builds in a scratch export, never in the working tree
# (project memory `feedback_never-clean-the-dogfood-tree`).
set -uo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
WORK=${TMPDIR:-/tmp}/os9-asanargs-$$
DIRTY=no
[ "${1:-}" = "--dirty" ] && DIRTY=yes

trap 'rm -rf "$WORK"' EXIT
mkdir -p "$WORK/logs" || { echo "could not make a scratch tree"; exit 1; }

if [ "$DIRTY" = yes ]; then
    ( cd "$REPO" && COPYFILE_DISABLE=1 tar -cf - Source GNUmakefile tools ) \
        | tar -x -C "$WORK" || { echo "could not copy the working tree"; exit 1; }
else
    ( cd "$REPO" && git archive HEAD ) | tar -x -C "$WORK" \
        || { echo "could not export HEAD"; exit 1; }
fi

# The UAE core's known UB is excluded (see tools/ubsan-ignore.txt); what is
# left to report is os9exec's own.
SAN="-fsanitize=address,undefined -fsanitize-ignorelist=$WORK/tools/ubsan-ignore.txt"
cd "$WORK" || exit 1

echo "== building with $SAN =="
make -B -j4 CC="cc $SAN -g -O1 -fno-omit-frame-pointer" >/dev/null 2>&1 \
  || { echo "  build FAILED"; exit 1; }
[ -x ./os9exec ] || { echo "  no binary was produced"; exit 1; }

# PROVE THE SANITIZER IS ARMED before believing a clean run: a build where
# -fsanitize did not apply reports nothing and looks exactly like a clean one.
n=$(nm -u ./os9exec 2>/dev/null | grep -c "__asan")
[ "${n:-0}" -gt 0 ] || { echo "  the binary has NO __asan symbols -- the sanitizer did not apply"; exit 1; }
cat > "$WORK/armed.c" <<'EOF'
#include <string.h>
#include <stdio.h>
int main(int argc, char **argv) {
    char small[16];
    strcpy(small, argc > 1 ? argv[1] : "");   /* overflows on a long argument */
    printf("%s\n", small);
    return 0;
}
EOF
cc $SAN -g -O0 -o "$WORK/armed" "$WORK/armed.c" 2>/dev/null \
  || { echo "  could not build the positive control"; exit 1; }
ctl=$("$WORK/armed" "this argument is far longer than sixteen bytes" 2>&1)
case "$ctl" in
    *"stack-buffer-overflow"*) ;;
    *) echo "  the positive control did NOT report -- this sweep could not fail"
       exit 1 ;;
esac
echo "  sanitizer armed ($n __asan symbols; positive control reports)"

L=$(printf 'X%.0s' $(seq 1 450))
M=$(printf 'Y%.0s' $(seq 1 250))
H=$(printf 'Z%.0s' $(seq 1 5000))

# One emulator per case, bounded, in a fresh scratch working directory that
# holds a file and a directory for the commands to act on. A watchdog rather
# than `timeout`: not every host this runs on has one.
cases=0 found=0
run() {
    cases=$((cases+1))
    local cwd="$WORK/cwd$cases" pid t=0
    mkdir -p "$cwd/dir" && : > "$cwd/src"
    ( cd "$cwd" && env ASAN_OPTIONS="detect_leaks=0,log_path=$WORK/logs/a$cases" \
                           UBSAN_OPTIONS="print_stacktrace=1,log_path=$WORK/logs/u$cases" \
                           "$@"; : ) < /dev/null > "$WORK/out$cases" 2>&1 &
    pid=$!
    while kill -0 $pid 2>/dev/null && [ $t -lt 40 ]; do sleep 0.5; t=$((t+1)); done
    # $pid is the subshell; the emulator is its child. Stop the CHILD first:
    # signalling only the subshell left a hung emulator orphaned and spinning
    # (2026-09-19: 21 of them, six cores, for three hours).
    if kill -0 $pid 2>/dev/null; then
        pkill -TERM -P $pid 2>/dev/null
        kill -TERM $pid 2>/dev/null
    fi
    wait $pid   # the trailing `:` keeps bash from exec-ing, so the subshell reaps an
                # aborted case itself and its "Abort trap" lands in out$cases, not here
    if ls "$WORK/logs/"[au]$cases.* >/dev/null 2>&1; then
        found=$((found+1))
        echo "  REPORT from: $(echo "$*" | sed "s|$WORK/||g" | cut -c1-90)"
        sed -nE '/ERROR: AddressSanitizer|runtime error/,/^$/p' "$WORK/logs/"[au]$cases.* \
            | grep -E "ERROR|runtime error|WRITE|READ|#[0-4] " | head -6 | sed 's/^/    /'
    fi
    rm -rf "$cwd"
}

E="$WORK/os9exec"
for c in rename move mv icopy imakdir mount unmount systime iprocs imdir ipaths \
         imem ihit iunused idevs idbg dhelp; do
    run "$E" -r $c "$L"
    run "$E" -r $c "$L" "$L"
    run "$E" -r $c src "$L"
    run "$E" -r $c "$L" src
    run "$E" -r $c "dir/$L"
    run "$E" -r $c "-$L"
    run "$E" -r $c "$M/$M"
    run "$E" -r $c src "dir/$M/$M"
done
for c in "mount -k=100K" "mount -r=100" "idbg -o"; do
    run "$E" -r $c "$L"
    run "$E" -r $c "$M/$M"
done
run "$E" -r "$L"
run "$E" -r "$L/$L"
run "$E" -r "$H"
run "$E" -r -d "$L" idevs
run "$E" -r -M"$L" idevs
run "$E" -r -q"$L" idevs
run env OS9H5="$L" "$E" -r idevs
run env OS9H5="$WORK/$H" "$E" -r idevs
run env OS9DISK="$WORK/$L" "$E" -r idevs
run env OS9T="$L" "$E" -r idevs

echo "  $cases cases, $found with sanitizer reports"
[ $found -eq 0 ] && echo "ASAN ARGS SWEEP OK -- no overflow from any over-long argument"
[ $found -eq 0 ]
