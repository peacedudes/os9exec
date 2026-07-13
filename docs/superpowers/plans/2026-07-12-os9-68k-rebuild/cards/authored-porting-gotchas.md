# Authored GOTCHA/FACT cards — porting Unix C to OS-9/68k

Written directly by Claude (not extracted by a subagent), from a real port
carried out live in `os9exec`: building GNU fileutils 3.13 `ls` with the
gcc2 (2.5.6) toolchain on `h0`, from first compile through a fully correct
`ls -l`. Every claim below was hit, diagnosed and then fixed against a
running system — none is inferred from a manual.

`verify: VERIFIED` per the design spec's honesty ledger (line 56:
`VERIFIED | from-manual | authored`; line 170: upgrade to `VERIFIED` when
confirmed against the emulator/REPL). These are live-confirmed behaviour,
not manual extraction and not synthesis.

The os9lib headers referenced here are the POSIX-compatibility layer from
the public Microware archive (`h0/DEFS/os9lib/`), not proprietary source.

--- CARD ---
id:        os9lib-declares-stat-but-does-not-implement-it
type:      GOTCHA
target:    68k
verify:    VERIFIED
topic:     c-library
claim:     os9lib's `<stat.h>` declares `stat()` and `fstat()` (`extern stat (); extern fstat ();`) but the library contains no implementation of either. Any Unix code ported to OS-9 that calls `stat()` must supply its own. The header gives no hint of this — the failure appears only at link time, as `Symbol 'stat' unresolved`.
context:   Confirmed by compiling a program whose only external call is `stat()` and linking it without any stub: `l68` reports the symbol unresolved. This is a trap precisely because the declaration's presence makes the function look available; a porter naturally assumes the library provides what its header promises, and only discovers otherwise after everything else already links.
source:    authored
--- END ---

--- CARD ---
id:        gs-gfd-returns-the-file-descriptor-sector
type:      FACT
target:    68k
verify:    VERIFIED
topic:     disk-io
claim:     `_gs_gfd(path, buffer, count)` — the C library wrapper for the SS_FD getstat — fills the caller's buffer with the file's *file descriptor sector*, which carries all of the file's real metadata: attribute byte, owner, date last modified, link count, size, and creation date. `<direct.h>` describes the layout as `struct fildes { char fd_att, fd_own[2], fd_date[5], fd_link, fd_fsize[4], fd_dcr[3]; ... }`.
context:   This is the answer to "how do I stat a file on OS-9", given that the library's own `stat()` does not exist (see [[os9lib-declares-stat-but-does-not-implement-it]]). Verified to work not only on real RBF disk images but also on os9exec's *host-native* directory devices — the emulator synthesises a faithful FD sector for host files, so a `stat()` built on `_gs_gfd` returns correct sizes and timestamps for them too (checked field-by-field against the host's own `ls -l`).
source:    authored
--- END ---

--- CARD ---
id:        opening-a-directory-needs-the-directory-bit
type:      GOTCHA
target:    all
verify:    VERIFIED
topic:     io
claim:     A directory cannot be opened as an ordinary file. `open(path, S_IREAD)` on a directory fails; the directory bit must be set in the access mode — `open(path, S_IFDIR | S_IREAD)` (0x80 in the mode byte). The same applies to `fopen()`, which offers no way to set that bit and therefore cannot open a directory at all.
context:   Two consequences worth internalising. First, this is why straightforward Unix code that does `fopen(dir, "r")` to list a directory simply does not work on OS-9. Second, the failure is *useful*: since a plain `open()` fails on a directory and succeeds on a file, trying the plain open first and falling back to the dir-bit open is a reliable way to discover which one you have, without needing a separate stat.
source:    authored
--- END ---

--- CARD ---
id:        os9-attribute-byte-bit-layout
type:      FACT
target:    all
verify:    VERIFIED
topic:     disk-io
claim:     The OS-9 file attribute byte (`fd_att`, the first byte of the file descriptor sector) is: 0x01 owner read, 0x02 owner write, 0x04 owner execute, 0x08 public read, 0x10 public write, 0x20 public execute, 0x40 sharable, 0x80 directory. It maps bit-for-bit onto os9lib `<stat.h>`'s `S_IREAD`/`S_IWRITE`/`S_IEXEC`/`S_IOREAD`/`S_IOWRITE`/`S_IOEXEC`/`S_ISHARE`/`S_IFDIR`.
context:   Because the mapping is exact, a `stat()` built on `_gs_gfd` can assign `st_mode = fd_att` directly and get correct permissions and directory-ness in one move. Note the two access classes are *owner* and *public* — there is no group class (see [[os9-has-no-group-permission-class]]). Verified live: a file with owner read/write/execute plus public read/execute reads back as `att=0x2f`, a directory as `att=0xbf`.
source:    authored
--- END ---

--- CARD ---
id:        os9lib-s-ifmt-is-too-narrow-for-s-isreg
type:      GOTCHA
target:    68k
verify:    VERIFIED
topic:     c-library
claim:     os9lib's `<stat.h>` defines `S_IFMT` as `0x0080` — a *single bit* — while `S_IFREG` is `0x0100`. The stock macro `S_ISREG(m)` is therefore `((m & 0x0080) == 0x0100)`, which can never be true for any value of m. Regular files are silently misclassified, typically falling through to `S_ISCHR(m)` (`(m & 0x0080) == 0x0000`, true whenever the directory bit is clear) and being reported as character devices.
context:   This is the single nastiest header trap found in the whole port. Unix code that decodes `st_mode` with the standard macros compiles clean, links clean, runs, and quietly prints `c` for every regular file — GNU `ls -l` did exactly this. The `S_IFMT` a porter actually needs is `0x0380` (wide enough to span S_IFDIR 0x0080, S_IFREG 0x0100 and S_IFIFO 0x0200). Beware that a project may already carry a private fix: GNU fileutils' `system.h` redefines `S_IFMT` to `0x0380` correctly, but `filemode.c` includes `<sys/stat.h>` rather than `system.h` and so never sees it — so the same program can hold both the correct and the broken definition, in different translation units.
source:    authored
--- END ---

--- CARD ---
id:        os9-has-no-group-permission-class
type:      GOTCHA
target:    all
verify:    VERIFIED
topic:     c-library
claim:     OS-9 has only two permission classes, owner and public — there is no group class. os9lib's `<stat.h>` acknowledges this by deliberately aliasing the group macros onto the public ones: `S_IRGRP` and `S_IROTH` are both `0x0008`, `S_IWGRP` and `S_IWOTH` both `0x0010`, and so on. Unix code that extracts permission triples with the octal masks `0700`/`0070`/`0007` misses OS-9's permission bits (`0x01`–`0x20`) entirely and renders every permission as absent.
context:   The aliasing is a useful hint about intent: when rendering a Unix-style 10-character mode string, mirroring the public bits into the group position is what the header itself already implies. Symptom to recognise: an `ls -l` whose type character is right but whose permissions are all dashes means the *permission* decode is using Unix masks, independently of whatever is wrong with the type decode.
source:    authored
--- END ---

--- CARD ---
id:        os9-file-dates-are-local-time-not-utc
type:      GOTCHA
target:    all
verify:    VERIFIED
topic:     c-library
claim:     OS-9 stores file dates as *local wall-clock* time (the FD sector's 5-byte `fd_date`: year−1900, month, day, hour, minute — no seconds). A Unix `time_t` is UTC. Converting the stored fields as though they were UTC and then printing them back through `localtime()` shifts every timestamp by the local UTC offset — a flat, silent whole-timezone error.
context:   The obvious fix, `mktime()` (which interprets a `struct tm` as local time), is **not** available: os9lib's `<time.h>` declares it only in its non-GCC branch, so under gcc2 it is not declared. A zone-agnostic workaround that needs no new library symbol: convert the fields naively, pass the result to `localtime()`, see how far the returned fields moved from the originals, and add that difference back. Correct in any zone; can be an hour out for a timestamp landing inside a DST transition, which minute-resolution local dates cannot disambiguate anyway.
source:    authored
--- END ---

--- CARD ---
id:        os9lib-ctype-is-pre-ansi-no-isgraph
type:      GOTCHA
target:    68k
verify:    VERIFIED
topic:     c-library
claim:     os9lib's `<ctype.h>` is K&R-era and is missing ANSI-era additions. It defines `isalpha`, `isupper`, `islower`, `isdigit`, `isxdigit`, `isspace`, `ispunct`, `isalnum`, `isprint`, `iscntrl` and `isascii` as macros over a `_chcodes[]` table — but has no `isgraph`. A call to `isgraph()` therefore degrades to an implicit function call and fails at link time as an unresolved symbol.
context:   Treat this as an instance of the general rule rather than a one-off: os9lib predates ANSI C, so assume any function added by the ANSI standard may simply be absent, and expect to discover it at link time rather than compile time. (`isgraph` is trivially supplied as `isascii(c) && isprint(c) && c != ' '` — note that os9lib's `isprint` already excludes space, since space is classified `_WHITE` rather than `_PUNCT`.)
source:    authored
--- END ---

--- CARD ---
id:        l68-error-taxonomy-corrupt-object-vs-link-failure
type:      GOTCHA
target:    68k
verify:    VERIFIED
topic:     c-compiler
claim:     `l68`'s two early errors say very different things, and the distinction is diagnostic. `file 'x.r' is not a relocatable module` means the object did not *parse* at all — the file is corrupt or is not a ROF. `no root psect found` means the object parsed perfectly well and the linker simply found no entry point in it. Running `l68` on a *single* object file is therefore a cheap integrity check: a healthy object reports `no root psect found`; a corrupt one reports `not a relocatable module`.
context:   Worth reaching for early, because a corrupt object masquerades convincingly as a linker bug. In this project a single bad `.r` file — one of eleven, with a perfectly plausible ROF header and a creation timestamp one second apart from its known-good siblings — was blamed across two sessions on a `l68` size limit and on a toolchain-generation mismatch, when the actual fix was to recompile that one file. General lesson: when exactly one input out of many is rejected, rebuild it before theorising about the tool.
source:    authored
--- END ---

--- CARD ---
id:        rof-header-carries-a-decodable-build-timestamp
type:      FACT
target:    68k
verify:    VERIFIED
topic:     c-compiler
claim:     A ROF relocatable object (`.r`) begins with the sync longword `$DEADFACE`, and bytes 12..17 of the header hold its creation date as six bytes: year−1900, month, day, hour, minute, second. The module's code size sits at offset 0x1C and its name is a NUL-terminated string starting at offset 0x38.
context:   Directly useful for forensics on a suspect object, without needing any tool: decoding the timestamp of two `.r` files is enough to prove or disprove "these were built by different generations of the compiler" — a theory that can otherwise absorb a lot of effort. In this project two objects whose build dates differed by one second settled that question immediately.
source:    authored
--- END ---
