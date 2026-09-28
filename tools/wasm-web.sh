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
# LICENSING: the disk is copied into build/web (disk.gz, or os9exec.data
# for a directory). build/ is
# gitignored, so a licensed system disk stays on this machine -- but do not
# publish a build/web made from one. The CONF68K image is ours to ship.
#
# Differences from tools/wasm-build.sh (node): keys come from the page, not
# stdin; and the idle wait and a busy guest both yield to the page
# (Asyncify). The system tick is on, as everywhere else: a browser has no
# SIGALRM, so the emulation loop reads the clock itself (os9_tick.c), and a
# program that makes no system calls no longer freezes the page.

REPO=$(cd "$(dirname "$0")/.." && pwd)
OUT="$REPO/build/web"
DISK="${1:-$REPO/build/selfhost68k/conf68k.dsk}"
shift 2>/dev/null
BOOT=("$@"); [ ${#BOOT[@]} -gt 0 ] || BOOT=(shell)

command -v emcc >/dev/null || { echo "no emcc -- brew install emscripten" >&2; exit 2; }
[ -e "$DISK" ] || { echo "no disk at $DISK" >&2; exit 2; }

rm -rf "$OUT"; mkdir -p "$OUT"   # no stale disk from an earlier build

# An image goes out gzipped, as disk.gz, and the page inflates it
# (DecompressionStream): one file, and a fraction of the download -- the
# web-only freeware image is 119 MB, over GitHub's 100 MB file limit, and
# 34 MB gzipped. A directory (a local system disk) is preloaded as is.
if [ -d "$DISK" ]; then
    DD=/dd;     PRELOAD=(--preload-file "$DISK@$DD")
else
    DD=/dd.img; PRELOAD=()
    gzip -9 -c "$DISK" > "$OUT/disk.gz" || exit 1
fi

# The console goes to the page byte by byte: OS-9 ends lines in CR, and
# Emscripten's default stdout only flushes on LF.
cat > "$OUT/pre.js" <<JS
Module.preRun = Module.preRun || [];
Module.preRun.push(function () {
  ENV.OS9DISK = '$DD';
  ENV.OS9H0   = '$DD';   // the freeware disk must also be /h0 (its GAMES and termcap say so)
  var out = function (c) { if (c !== null) Module.os9out(c); };
  FS.init(function () { return null; }, out, out);
  if ('$DD' !== '/dd.img') return;
  addRunDependency('disk.gz');   // main() waits until the disk is in place
  fetch('disk.gz')
    .then(function (r) {
      if (!r.ok) throw new Error('disk.gz: HTTP ' + r.status);
      return new Response(r.body.pipeThrough(new DecompressionStream('gzip'))).arrayBuffer();
    })
    .then(function (buf) {
      FS.writeFile('$DD', new Uint8Array(buf));
      removeRunDependency('disk.gz');
    })
    .catch(function (e) { Module.os9status('could not load the disk: ' + e.message); });
});

// /h1 lives in the browser's own storage (IndexedDB, via Emscripten's IDBFS),
// so what a program writes there outlives a reload. It is loaded before main()
// and becomes /h1 as OS9H1 does natively; the page attaches, saves and detaches
// it through Module.os9h1.
// A sync asked for while one is running is not dropped: it runs again when
// that one ends, and every caller's <done> waits for the sync that covers its
// own write -- attaching reloads the page from <done>, so answering early lost
// the disk it had just been given.
var H1 = '/keep/h1.dsk', H1NAME = '/keep/h1.name', syncing = false, again = false, waiters = [];
function keepSync(done) {
  if (done) waiters.push(done);
  if (syncing) { again = true; return; }
  syncing = true;
  FS.syncfs(false, function (err) {
    syncing = false;
    if (again) { again = false; keepSync(); return; }
    var ready = waiters; waiters = [];
    ready.forEach(function (f) { f(err); });
  });
}
Module.preRun.push(function () {
  FS.mkdir('/keep');
  FS.mount(IDBFS, {}, '/keep');
  addRunDependency('keep');
  FS.syncfs(true, function () {
    if (FS.analyzePath(H1).exists) ENV.OS9H1 = H1;
    removeRunDependency('keep');
  });
  var last = 0;       // write /h1 back a few seconds after it changes
  function syncIfChanged() {
    if (!FS.analyzePath(H1).exists) return;
    var m = FS.stat(H1).mtime.getTime();
    if (m !== last) { last = m; keepSync(); }
  }
  setInterval(syncIfChanged, 3000);
  // and at once when the page is hidden or closed, rather than losing the
  // last few seconds of writes (hidden is the one a phone reliably sends)
  document.addEventListener('visibilitychange', function () {
    if (document.visibilityState === 'hidden') syncIfChanged();
  });
  window.addEventListener('pagehide', syncIfChanged);
  if (navigator.storage && navigator.storage.persist) navigator.storage.persist();
});
Module.os9h1 = {
  name: function () {
    return FS.analyzePath(H1).exists
      ? (FS.analyzePath(H1NAME).exists ? FS.readFile(H1NAME, { encoding: 'utf8' }) : 'h1.dsk') : null;
  },
  attach: function (name, bytes, done) {
    FS.writeFile(H1, bytes); FS.writeFile(H1NAME, name); keepSync(done);
  },
  image: function () { return FS.readFile(H1); },
  detach: function (done) {
    if (FS.analyzePath(H1).exists) FS.unlink(H1);
    if (FS.analyzePath(H1NAME).exists) FS.unlink(H1NAME);
    keepSync(done);
  },
};
JS

args=''; for a in "${BOOT[@]}"; do args="$args${args:+, }\"$a\""; done
sed -e "s|OS9_ARGUMENTS|[$args]|" \
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
    -sEXPORTED_RUNTIME_METHODS=ENV,FS -sENVIRONMENT=web -lidbfs.js \
    --pre-js "$OUT/pre.js" "${PRELOAD[@]}" ) || exit 1

rm -f "$OUT/pre.js"
disk=$(cat "$OUT/disk.gz" "$OUT/os9exec.data" 2>/dev/null | wc -c)
echo "built $OUT ($(wc -c < "$OUT/os9exec.wasm") bytes of wasm, $disk bytes of disk)"
echo "serve:  python3 -m http.server -d $OUT 8000   then open http://localhost:8000"
