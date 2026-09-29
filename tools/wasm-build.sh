#!/usr/bin/env bash
# wasm-build.sh -- build os9exec for WebAssembly and prove it runs OS-9 code.
#
# Measured 2026-09-21: the whole CONF68K suite runs under this build and
# reports PASS=102 FAIL=0 SKIP=1, which is byte for byte what the native
# emulator reports from the same image. The 68020 core needed NO changes: it
# is portable C with no assembly and no JIT, and the three things that usually
# break such code under wasm were already proven clean here -- alignment
# (tools/ubsan-sweep.sh), byte order (the s390x leg) and pointer width, since
# wasm32 is ILP32 exactly like the i386 leg.
#
# Only two source changes were needed, both compile-time no-ops off-target and
# both already in the tree:
#   * __EMSCRIPTEN__ alongside `linux` in os9main_incl_precomp.h -- emcc does
#     not define `linux`, so the platform flag never got set.
#   * main() takes `environ` under Emscripten, which calls main with two
#     arguments and leaves the third holding rubbish.
#
# What this build deliberately does NOT have: sockets (a browser has none
# without a proxy), /tN ptys, and host directories. It boots from an RBF disk
# IMAGE preloaded into Emscripten's in-memory filesystem, which is why the
# self-contained CONF68K disk is the natural thing to run.
#
# Needs emsdk on PATH:  source /path/to/emsdk/emsdk_env.sh
set -uo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${1:-$REPO/build/wasm}"
IMG="${OS9IMAGE:-$REPO/build/selfhost68k/conf68k.dsk}"

command -v emcc >/dev/null || {
    echo "no emcc -- source your emsdk_env.sh first" >&2; exit 2; }
[ -f "$IMG" ] || {
    echo "no disk image at $IMG -- run tools/selfhost68k/build-image.sh" >&2; exit 2; }

mkdir -p "$OUT"
cp "$IMG" "$OUT/conf68k.dsk"

# os9exec finds its devices through the environment, and node's process.env
# does not reach Emscripten's getenv on its own. Hand over just the OS9* ones.
cat > "$OUT/pre.js" <<'JS'
Module.preRun = Module.preRun || [];
Module.preRun.push(function () {
  for (var k in process.env) if (k.indexOf('OS9') === 0) ENV[k] = process.env[k];
});
JS

# The same source list the native build uses, so the two cannot drift.
# -B because a plain `make -n` on an already-built tree prints nothing to
# parse, and an empty list would otherwise link a binary out of no sources.
SRCS=$(cd "$REPO" && make -n -B 2>/dev/null | grep -aoE '[A-Za-z0-9_./]+\.c' | sort -u)
[ -n "$SRCS" ] || { echo "could not read the source list from the makefile" >&2; exit 1; }

# ASYNCIFY is what lets the blocking idle wait yield to the host event loop
# without restructuring it. The alternative (pthreads in a Worker) is faster
# but needs COOP/COEP headers, which GitHub Pages will not serve.
( cd "$REPO" && emcc $SRCS -o "$OUT/os9exec.js" \
    -O2 -w -fcommon -D_FILE_OFFSET_BITS=64 \
    -DTERMINAL_CONSOLE -DINT_CMD -DRAM_SUPPORT -DREUSE_MEM \
    -ISource/OS9exec_core -ISource/OS9exec_core/os9defs \
    -ISource/Platforms/LINUX -ISource/Platforms \
    -ISource/OS9AppEmu/UAE68emulator -ISource/OS9AppEmu \
    -sASYNCIFY -sALLOW_MEMORY_GROWTH -sEXIT_RUNTIME=1 \
    -sINITIAL_MEMORY=268435456 -sSTACK_SIZE=8388608 \
    -sEXPORTED_RUNTIME_METHODS=ENV,FS \
    --pre-js "$OUT/pre.js" --preload-file "$OUT/conf68k.dsk"@/conf68k.dsk ) || exit 1

echo "built $OUT/os9exec.js ($(wc -c < "$OUT/os9exec.wasm") bytes of wasm)"

command -v node >/dev/null || { echo "no node -- built but not verified"; exit 0; }

# Prove it runs OS-9 code rather than merely starting. Anything less than the
# whole suite would not distinguish "boots" from "works".
pass=0; fail=0; skip=0; none=0
for m in $(ls "$REPO/test/68k-conformance/CMDS" | grep -avE '^(tally|mark|run|load|cio)$' | sort); do
    line=$( cd "$OUT" && OS9DISK=/conf68k.dsk node os9exec.js -r "/dd/CMDS/$m" 2>/dev/null \
            | tr '\r' '\n' | grep -a '^RESULT ' | head -1 )
    case "$line" in
        *PASS*) pass=$((pass+1)) ;;
        *FAIL*) fail=$((fail+1)); echo "  $line" ;;
        *SKIP*) skip=$((skip+1)) ;;
        *)      none=$((none+1)); echo "  no RESULT from $m" ;;
    esac
done
echo "WASM CONF68K: PASS=$pass FAIL=$fail SKIP=$skip NO-RESULT=$none"
# The browser build signs on with the original authors' names whatever it runs:
# a page is an interactive session even when it starts bash rather than the
# shell, and it showed none (a terminal still prints it for shell/sh only).
banner=$( cd "$OUT" && OS9DISK=/conf68k.dsk node os9exec.js -r /dd/CMDS/t01open 2>&1 | tr '\r' '\n' )
case "$banner" in
    *"Lukas Zeller / Beat Forster"*) echo "WASM sign-on: the authors' banner is shown" ;;
    *) echo "WASM sign-on: NO authors' banner for a program other than shell"; exit 1 ;;
esac
case "$banner" in   # and names its platform, where it said '?' and "Unknown System"
    *"Platform: 'Browser - wasm32' (wasm32)"*) echo "WASM sign-on: the platform is named" ;;
    *) echo "WASM sign-on: the platform is not named"; exit 1 ;;
esac
# No test at all is not a pass: an empty CMDS (the suite not built) ran nothing.
[ "$pass" -gt 0 ] || { echo "no conformance test ran -- is test/68k-conformance/CMDS built?"; exit 1; }
[ "$fail" -eq 0 ] && [ "$none" -eq 0 ] || exit 1
echo "WASM OK -- the emulator runs OS-9/68000 code in WebAssembly"
