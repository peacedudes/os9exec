# Dogfood report: hand-written OS-9/68000 file manager (2026-07-18)

Task: write a minimal custom file manager module (`FlMgr`, type `$0D`) per
`os9-systems-dev`'s `file-managers.md` 13-entry Create/Open/MakDir/ChgDir/
Delete/Seek/Read/Write/ReadLn/WriteLn/GetStat/SetStat/Close dispatch table,
pair it with a device descriptor, install it, and try to exercise it —
specifically to answer the open question the earlier device-driver dogfood
session left on the table: real OS-9 dispatches a file manager
*differently* from a driver (I$ calls go to the file manager first, which
then calls the driver), so it was genuinely unknown whether `os9exec`
handles that layer the same broken way, differently, or correctly.

Artifacts: `dogfood-filemgr-dogfm.a` (file manager), `dogfood-filemgr-desc.a`
(device descriptor), `dogfood-filemgr-test.a` (test program) — each file's
own header carries the same verification-status detail condensed here.

## Bottom line: same wall as the driver session, and *provably* the same cause

**Not just empirically similar — literally the same code path.** The
earlier driver session established *that* `os9exec` never invokes a
driver's code, via `I$Attach` being a dummy and I/O dispatch running
through a fixed C table. This session went one level deeper and found the
single function that decides both drivers' and file managers' fate:

- `Source/OS9exec_core/icalls.c`'s `OS9_I_OpenCreate` (the shared body
  behind `I$Open`/`I$Create`) calls `IO_Type()` **exactly once** to
  classify the path string into a fixed `ptype_typ` enum (hardcoded
  prefix/name matching — `/pipe`, `/tty`, `/lp`, `/vmod`, then a fallback
  to `OS9_Device()` for RBF/SCSI/host-directory matching), then hands that
  type straight to `usrpath_open()`, which indexes `fmgr_op[type]`
  (`filestuff.c`'s fixed dispatch table, populated once at startup by
  `init_fmgrs()`). **There is no code anywhere in this chain — `IO_Type`,
  `OS9_Device`, or `OS9_I_OpenCreate` — that ever loads a device-descriptor
  module or reads its `M$FMgr` field.** A custom device name never even
  reaches the point where a descriptor's file-manager selection could
  matter: path classification is fully decided before any module is
  consulted, for drivers *and* file managers alike, via the identical
  function and the identical table.
- **Live confirmation, mirroring the driver session's methodology exactly:**
  `dogfm.a` assembled (0 errors) and linked into a genuine, CRC-valid
  `68000 File Mngr, Object Code, Sharable, System State Process` module
  (`ident`: Ty/La `$D01`, Attr/Rev `$A001` — confirms `FlMgr`=13=`$0D` is
  the correct type code). `dogfmdesc.a` linked into a `68000 Dev Descr`
  module (`$F01`/`$A001`) with `M$FMgr`/`M$PDev` fields verified
  byte-correct by dumping the linked module: `M$FMgr`=`$0054` and
  `M$PDev`=`$005A` point exactly at the `"dogfm\0"` and `"dogloop\0"`
  strings' true file offsets. Both modules `load`ed resident under the
  names `DOGFM`/`DOGFM2` (confirmed via `mdir`). `iniz DOGFM2` silently
  no-ops (`devs` stays an empty table, same as the driver session).
  `dogfmtest.a`'s real `I$Open("/DOGFM2")` printed exactly
  `open /DOGFM2 failed`, and `dir /DOGFM2` failed with the **identical
  error code** the driver session's `/DOGFOOD` hit: `Error #000:221
  (E$MNF) Module Not Found`. No entry point in `dogfm.a` was ever reached
  (the `Open` routine's `PD_FST` magic-byte write, meant to prove live
  invocation via a path-descriptor dump, was never reachable to test).

Given the source-level finding above, this outcome was expected before the
live run started; the live run's only job was to confirm the prediction,
which it did exactly. No extended live experimentation was needed or
attempted beyond this single confirming pass, per the task's own guidance
not to re-prove an already-source-settled point at length.

## Was file-managers.md wrong, missing, or confusing?

**Missing, in the same shape as device-drivers.md's known gap, but not yet
present in this file:** `device-drivers.md` now carries an explicit
verification-status header disclosing that `os9exec` never dispatches to
driver-module code. `file-managers.md` has no equivalent note anywhere —
a reader would reasonably assume (especially given the task's own framing,
"a file manager is dispatched differently in real OS-9") that this file's
Entry Point Table might actually be exercisable on `os9exec`, since the
already-known driver gap doesn't obviously generalize to a different
kernel subsystem without checking. It doesn't generalize by accident — it
generalizes because `fmgr_op[]`/`IO_Type()` is the literal shared
mechanism — but a reader has no way to know that without either reading
`os9exec`'s C source directly (as this session did) or repeating this
session's build-and-test cycle. This is the single most actionable
finding: `file-managers.md` should get a verification-status header
parallel to `device-drivers.md`'s, ideally citing `IO_Type()`/`fmgr_op[]`
directly so the "is this different from drivers?" question doesn't have
to be re-asked and re-answered by the next person.

**Not wrong:** every structural claim this session actually exercised held
up exactly as documented — the 13-entry table's offsets (0/2/4/.../24),
the FlMgr type code (`$0D`, cross-checked against `module_from_book.h`'s
`MT_FILEMAN 13`), the 42-byte path-descriptor header / `PD_FST` boundary,
and "Create synonymous with Open when there's no multi-file support" all
matched cleanly with zero surprises during authoring — this file's content
is solid as far as toolchain/format goes; the gap is purely the
"is any of this reachable on os9exec" framing, same as the driver file.

**Confusing, tangential, unresolved (minor, not chased further):** the
descriptor's psect-header-padding gap (`Devic`-type body landing at file
offset `$3C` instead of the canonical `$30`, already documented in the
driver session) reproduced identically here for `dogfmdesc.a` — expected,
not a new finding. By contrast `dogfm.a` (an `FlMgr`-type, `mod_exec`-shaped
module like `Drivr`) landed its body at `$3C` too, but that *is* the
canonical, correct offset for `mod_exec`-shaped headers (`M$Exec`/
`M$Excpt`/`M$Data` occupy `$30`-`$3B`) — worth noting explicitly that the
padding bug is `Devic`-specific, not universal, in case a future reader
sees "$3C" in two different files and assumes the same gap applies to
both for the same reason.

One small, unverified-and-not-chased loose end: `file-managers.md`'s File
Security section says a file manager module "must be owned by super-user
... or OS-9 refuses to load it," but `ident /h0/CMDS/DOGFM` reported
`Owner: 0.259` (group 0, but user 259, not 0) after a `load` that
succeeded without error and left the module resident per `mdir`. Either
`os9exec`'s `load` doesn't actually enforce the owner-UID-0 half of that
rule (only the attribute bit / group), or the doc's "super-user" means
something looser than UID 0 specifically. Not dug into further — tangential
to the dispatch question this session was scoped to answer, flagged here
for whoever next touches file-security enforcement in `os9exec`.

## Anything surprisingly helpful?

- **The driver session's toolchain findings transferred with zero
  friction**: double-quoted string literals, bare `dc.w` label references
  for the jump table, `l68`'s `-o=` output-naming behavior. The one new
  wrinkle this session hit and had to work around: `l68 -o=<bare-name>`
  did **not** land in `/h0/CMDS` this time (a silent no-op — no file
  written, no error) — this account's `chx` apparently isn't `/h0/CMDS`,
  unlike the driver session's account. Using an explicit full path
  (`l68 -o=/h0/CMDS/DOGFM ...`) worked immediately and reliably. Worth
  treating `using-os9exec-repl.md`'s "`l68 -o=` writes to `chx`" note as
  account-dependent rather than a fixed path, and defaulting to an
  explicit full `-o=` path to sidestep the ambiguity entirely.
- **Reading `icalls.c`+`utilstuff.c` together settled the "different or
  same?" question in one pass**, with a strength of evidence (single
  shared function, byte-identical dispatch table) well beyond what the
  live experiment alone could show — the live run subsequently matched
  the prediction exactly, which is the ideal outcome for a source-first
  approach to this kind of question.

## Overall

Confirmed, from source and live: file-manager dispatch on `os9exec` hits
the *exact same* wall as device-driver dispatch, for the *exact same*
reason (`IO_Type()` decides device class from the path string alone,
`fmgr_op[]` never executes installed module code). This is a genuine
`os9exec` feature gap, not a bug in existing behavior, and not attempted
to fix here per the task's scope. The toolchain/module-format side of
writing a custom `FlMgr`-type module is fully workable and now
`Live`-verified byte-correct, same as the driver session found for
`Drivr`/`Devic` — only the "does the kernel ever call it" half remains
blocked, and now for a source-confirmed reason that generalizes across
both module classes rather than needing to be independently re-verified
per class.
