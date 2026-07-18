# Dogfood notes: building/installing GFX+GFX2 on the live NitrOS-9 test disk (2026-07-18)

Companion to `dogfood-gfx2-goset-palette-test.bas` and
`dogfood-report-gfx2-2026-07-18.md`. This file is the mechanical
how-it-was-done record (host-side tooling, patches applied to the
third-party source, and the exact ToolShed/asm commands); the report file
has the narrative summary and answers to GOSET/PALETTE.

Source used (already in-tree, community-maintained, third-party, not
duplicated here): `os9/nitros9/source/3rdparty/packages/basic09/gfx.asm`
and `gfx2.asm`.

## Getting the source onto the disk image (host-side, no boot needed)

The two source files (25,602 / 90,007 bytes) are far too large for
`nitros9repl.sh`'s `tee`/`key`-typed-line-by-line route. Used DrPitre's
ToolShed (`os9` CLI, already built at
`os9/nitros9/tools/toolshed/build/unix/os9/os9` from an earlier session --
see the `feature-xroar-autoboot-and-drivewire-fixes` memory) to inject
them directly into the disk image, host-side, with XRoar stopped:

```sh
DISKDIR=os9/nitros9/disk-images/eou_ide-v0.3-6809-xroar-dw-becker
TOOLSHED=os9/nitros9/tools/toolshed/build/unix/os9/os9
# extract the real RBF filesystem out of the XRoar-specific .ide container
dd if="$DISKDIR/68IDE.ide" of="$DISKDIR/partition.img" bs=512 skip=632
$TOOLSHED makdir "$DISKDIR/partition.img,CLAUDE/GFX2SRC"
$TOOLSHED del  "$DISKDIR/partition.img,CLAUDE/GFX2SRC/gfx.a"   # if re-copying
$TOOLSHED copy -l "$SCRATCH/gfx.asm"  "$DISKDIR/partition.img,CLAUDE/GFX2SRC/gfx.a"
$TOOLSHED copy -l "$SCRATCH/gfx2.asm" "$DISKDIR/partition.img,CLAUDE/GFX2SRC/gfx2.a"
dd if="$DISKDIR/partition.img" of="$DISKDIR/68IDE.ide" bs=512 seek=632 conv=notrunc
```

`SCRATCH` above is a locally-patched copy of the two files (see "Source
patches" below) -- the in-tree originals were never modified.

**Real ToolShed bug found and worked around, `Live`:** `os9 copy -r`
(the documented "rewrite if exists" flag) updates a file's declared
size correctly but can leave some of its earlier data blocks holding
stale bytes from whatever *previously* occupied those same segments --
confirmed directly: after overwriting `gfx.a` in place with `-r`, its
`fstat`-reported size was correct (25,174 bytes, matching the patched
host file exactly) but `list`ing it back showed the file's opening
~700 bytes replaced by an unrelated stretch of GFX2's own dispatch table
(`FuncTbl`/`fdb L03AE`-style entries, `"Mouse"`/`"PutGC"`/`"DWSet"` names
that only exist in gfx2.asm) mixed in with GFX's real content further
down -- a leftover of an earlier, since-superseded draft. **Workaround
that resolved it cleanly, `Live`:** `os9 del` the target file first,
*then* a plain `os9 copy` (no `-r`) to force fresh segment allocation,
rather than overwriting in place. Re-verified after: `fstat` size and
`list` content matched the host source byte-for-byte, `asm` assembled
cleanly. Not filed upstream as a ToolShed issue (out of scope for this
task), but worth remembering if a future session hits inexplicable
"my source doesn't match what I wrote" symptoms after an `-r` overwrite.

The disk's `/DD/DEFS` directory already had `os9.d`, `scf.d`,
`cocovtio.d`, and `defsfile` present (the modern lwasm-oriented DEFS
files alongside the classic Microware ones) -- no missing `use` targets,
so nothing needed to be resolved by searching the disk further. `asm`'s
`USE` resolves relative to the *current data directory* (`chd`), not the
source file's own location, so `chd /dd/defs` before assembling (with
the source files living elsewhere, in `/dd/CLAUDE/GFX2SRC/`) was enough
to satisfy all three bare-name `use` directives.

## Source patches needed to assemble under `asm`/MIA (not `rma`, which hangs)

Both files were written for the modern `nitros9project/nitros9` build
(`lwasm`, driven via `rules.mak`/`makefile` in the same source
directory), not for the classic Microware `asm`/MIA on this EOU disk.
Three genuine, mechanical (behavior-preserving) porting fixes were
needed -- all found by iterating real `asm` error output, not by
guessing:

1. **Comment leader**: every full-line comment in both files starts with
   `;` (confirmed: zero mid-line `;` occurrences in either file, i.e. no
   ambiguous cases) -- `lwasm` accepts `;` as a comment leader, `asm`/MIA
   does not (only `*` in column 1, per `6809/assembly-and-tools.md`'s own
   directive table). Fix: `sed -E 's/^;+/*/'` on both files. Purely
   cosmetic, verified line-count-identical before/after.

2. **8-char label truncation collisions** (`gfx.asm` only -- `gfx2.asm`
   had zero): `asm`/MIA labels are 1-8 characters (vs. RMA's 1-9, and
   apparently unlimited for `lwasm`); several of `gfx.asm`'s original
   labels are longer and silently truncate to the same first-8-chars,
   producing `***** Error: redefined name`:
   - `StoreAppend2Write`/`StoreAppend1Write` -> both truncate to
     `StoreApp` -- renamed to `StorApp2`/`StorApp1`.
   - `RecoverAndBadFunc`/`RecoverAndRTS` -> both truncate to `RecoverA`
     -- renamed to `RecovBad`/`RecovRts`.
   - `SaveBYTEParam`/`SaveBYTEandRTS` -> both truncate to `SaveBYTE` --
     renamed to `SaveBytP`/`SaveByRT`.
   - Two genuinely-duplicate `ret@` local-style labels (lines 397 and
     453 of the original, in two unrelated subroutines) -- `asm`/MIA has
     no scoped/local-label mechanism for `@`-suffixed names the way
     `lwasm` apparently does; the second occurrence (and its one
     `beq ret@` reference) renamed to `retb@`.
   All renames verified with a Python pass over every label definition
   in the file, checked for any other 8-char-prefix collision (only
   these three pairs existed) and confirmed the new names don't
   themselves collide with anything else in the file.

3. **`H6309` conditional-assembly symbol undefined** (`gfx2.asm` only):
   12 `IFNE H6309`/`IFEQ H6309` blocks gate 6309-specific instruction
   variants; the real build supplies `H6309` via `lwasm -DH6309=1` (or
   omits it for a plain 6809 build, which `lwasm` apparently treats as
   0/false when undefined). `asm`/MIA has no `-D` command-line define
   equivalent and hard-errors `***** Error: undefined name` on an
   `IFNE`/`IFEQ` test against a truly-undefined symbol, which then
   cascades into `***** Error: phasing` for every subsequent forward
   reference (since pass 1/pass 2 addresses stop agreeing once a
   conditional block's inclusion becomes uncertain). Fix: added
   `H6309 set 0` near the top of the file (right after `nam`/`ttl`,
   before the `ifp1`/`use` block), selecting the plain-6809 code path --
   correct for this 6809-only disk. This single addition cleared all of
   the "undefined name"/"phasing" cascade.

4. **Three individual over-length source lines** (`gfx2.asm` only,
   after H6309 was fixed and the cascade cleared): 3 lines (135, 136,
   135 characters) triggered `***** Error: bad instr` on the *next*
   physical line, with a stray fragment like `ow)` or `one)` appearing
   as if it were a bogus label -- `asm`/MIA appears to have an internal
   per-line read limit somewhere between 132 (confirmed OK) and 135
   (confirmed broken) characters, undocumented in any manual mined so
   far for this skill. Fix: moved each offending line's overflow comment
   text to a `*`-led full-line comment on the line above/below, leaving
   the instruction's own trailing comment short. Purely cosmetic,
   zero semantic change; longest line in the file afterward is 132
   characters.

After all four fixes: `asm /dd/CLAUDE/GFX2SRC/gfx.a -O=gfx #32k` and
`asm /dd/CLAUDE/GFX2SRC/gfx2.a -O=gfx2 #48k` both assembled `00000
error(s)` (gfx2 had 1 warning, an addressing-mode hint, not investigated
further). `ident CMDS/gfx` / `ident CMDS/gfx2` both show good CRCs.
`#32k`/`#48k` memory bumps were needed -- default 4K symbol-table area
gives `Symbol Table Full` on files this size (per
`6809/assembly-and-tools.md`'s own documented `#memsize` mechanism).

## A harness gotcha hit again, for the record

The already-documented "a burst of rapid `key`/`send` calls can produce
delayed/bundled output attached to a later unrelated command" gotcha (see
`using-nitros9-repl.md`) reproduced here too: right after two heavy `asm`
invocations back-to-back, a third `asm` command's output initially looked
like 116 fresh errors referencing content that didn't match the file just
assembled. A plain `dir` probe confirmed the channel was actually healthy
and responsive; re-running the same `asm` command in isolation gave a
*different*, reproducible result each of the next several times -- which
in this specific case turned out not to be REPL-bundling noise after all,
but the real ToolShed segment-corruption bug described above (confirmed
by reading the file's own content back with `list`, not just re-running
`asm` and hoping). Point for future sessions: don't assume "looks like
the bundling gotcha" is automatically the explanation -- verify by reading
the actual file/state, since a genuinely different bug can produce a
similar-looking symptom.
