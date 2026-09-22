#!/usr/bin/env bash
# wasm-web.sh -- build os9exec for a BROWSER: a page with a terminal, the
# emulator in WebAssembly, and one OS-9 disk preloaded into memory.
#
#   tools/wasm-web.sh [<disk>] [<boot program and args>...]
#
#   <disk>  an RBF image or a directory; default: the self-contained CONF68K
#           image (tools/selfhost68k/build-image.sh). It becomes /dd.
#   boot    default: shell. The freeware disk: /path/osk-freeware.dd bash /dd/SYS/login
#           (it has no shell, and is mounted as /h0 as well, as the `free` alias does)
#
# Then serve build/web and open it:
#   python3 -m http.server -d build/web 8000     ->  http://localhost:8000
#
# LICENSING: the disk is copied into build/web/os9exec.data. build/ is
# gitignored, so a licensed system disk stays on this machine -- but do not
# publish a build/web made from one. The CONF68K image is ours to ship.
#
# Differences from tools/wasm-build.sh (node): keys come from the page, not
# stdin; the idle wait and a busy guest both yield to the page (Asyncify);
# and the system tick is off (-q), since a browser has no SIGALRM.

REPO=$(cd "$(dirname "$0")/.." && pwd)
OUT="$REPO/build/web"
DISK="${1:-$REPO/build/selfhost68k/conf68k.dsk}"
shift 2>/dev/null
BOOT=("$@"); [ ${#BOOT[@]} -gt 0 ] || BOOT=(shell)

command -v emcc >/dev/null || { echo "no emcc -- brew install emscripten" >&2; exit 2; }
[ -e "$DISK" ] || { echo "no disk at $DISK" >&2; exit 2; }

mkdir -p "$OUT"
if [ -d "$DISK" ]; then DD=/dd; else DD=/dd.img; fi

# The console goes to the page byte by byte: OS-9 ends lines in CR, and
# Emscripten's default stdout only flushes on LF.
cat > "$OUT/pre.js" <<JS
Module.preRun = Module.preRun || [];
Module.preRun.push(function () {
  ENV.OS9DISK = '$DD';
  ENV.OS9H0   = '$DD';   // the freeware disk must also be /h0 (its GAMES and termcap say so)
  var out = function (c) { if (c !== null) Module.os9out(c); };
  FS.init(function () { return null; }, out, out);
});
JS

args='"-q"'; for a in "${BOOT[@]}"; do args="$args, \"$a\""; done
sed -e "s|OS9_ARGUMENTS|[$args]|" -e "s|OS9_DISK|'$DD'|" \
    "$REPO/tools/wasm-web/index.html" > "$OUT/index.html"

SRCS=$(cd "$REPO" && make -n -B 2>/dev/null | grep -aoE '[A-Za-z0-9_./]+\.c' | sort -u)
[ -n "$SRCS" ] || { echo "could not read the source list from the makefile" >&2; exit 1; }

( cd "$REPO" && emcc $SRCS -o "$OUT/os9exec.js" \
    -O2 -w -fcommon -D_FILE_OFFSET_BITS=64 \
    -DTERMINAL_CONSOLE -DINT_CMD -DRAM_SUPPORT -DREUSE_MEM \
    -ISource/OS9exec_core -ISource/OS9exec_core/os9defs \
    -ISource/Platforms/LINUX -ISource/Platforms \
    -ISource/OS9AppEmu/UAE68emulator -ISource/OS9AppEmu \
    -sASYNCIFY -sALLOW_MEMORY_GROWTH -sEXIT_RUNTIME=1 \
    -sINITIAL_MEMORY=268435456 -sSTACK_SIZE=8388608 \
    -sEXPORTED_RUNTIME_METHODS=ENV,FS -sENVIRONMENT=web \
    --pre-js "$OUT/pre.js" --preload-file "$DISK@$DD" ) || exit 1

echo "built $OUT ($(wc -c < "$OUT/os9exec.wasm") bytes of wasm, $(wc -c < "$OUT/os9exec.data") bytes of disk)"
echo "serve:  python3 -m http.server -d $OUT 8000   then open http://localhost:8000"
