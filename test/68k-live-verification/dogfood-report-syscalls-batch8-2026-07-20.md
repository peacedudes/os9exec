# Dogfood report: 68k syscall live-verification batch 8 (2026-07-20)

Continues batch 7. `F$TLink($21)` — the riskiest call in the whole audit,
since it actively sets the program counter into a hand-built module's
init routine. `batch8-01.a`.

## The test

Staged deliberately: Stage 1 calls the already-proven-safe `F$SetCRC` on
the in-memory module buffer (zero jump risk) to validate the header
before ever risking the real call. Stage 2 writes the CRC-corrected
bytes to a real file. Stage 3 is the actual `F$TLink` install, followed
immediately by `F$TLink` remove.

`psect` cannot generate `mod_trap`'s extra 8-byte header extension (only
the standard 72-byte `mod_exec` shape, confirmed by the earlier driver
dogfood session), so the whole 80-byte header plus a hand-written
init/term routine was built from raw `dc.w`/`dc.l`/`dc.b` directives.

## Three real bugs found in the test module itself, all caught before
## ever risking a wild jump

1. **`trapinit` couldn't be a bare `rts`.** Reading `OS9_F_TLink`'s
   source (`fcalls.c`) shows it pushes `[caller's A6][dummy 0][saved
   PC]` onto the stack and points A7 at the *caller's A6* value, then
   jumps PC into `_mtrapinit`. A bare `rts` there pops caller's A6 as if
   it were a return address — a wild jump, exactly the failure mode the
   file's own risk-assessment comment worried about, just one level
   removed from where I'd been looking. Fixed: `move.l (a7)+,a6` /
   `addq.l #4,a7` / `rts` to properly unwind before returning.

2. **A misaligned `dc.b` string flipped the header from valid to
   E$BMID.** Widening `fname` from a bare relative filename to a full
   absolute path (needed to fix a separate bug, see below) made its
   length odd, which shifted every following label — including the
   whole `trapmod` header — onto an odd address. `F$SetCRC` started
   failing with `E$BMID` (bad sync) immediately after that edit, with
   *no other change*. This assembler has no `even` directive (`*** error
   - bad mnemonic ***`); the fix is a manual `dc.b 0,0` pad to force the
   next label back onto an even boundary. Worth remembering for any
   future hand-built-header test: string literals of odd length
   anywhere before a header silently break it.

3. **`(a0)` for `F$TLink` must be a module *name*, not a filename.**
   `OS9_F_TLink`'s doc comment says "Module name pointer", and
   `install_traphandler` resolves it via `link_load`, the same
   name-search mechanism `F$Link`/`F$Load` use (exec-directory search,
   `exedir=true` mode) — not a bare relative filename in the *data*
   directory, which is where `I$Create` had written the file. First
   attempt failed with `E$PNNF` (216) for exactly this reason. Fixed by
   passing the file's full absolute path instead of a bare name, which
   resolves correctly regardless of exec-vs-data-directory ambiguity.

## A real `os9exec` bug found — and fixed, not just logged

After fixing all three of the above, `F$TLink` install still failed,
now with `E$BMID` (205) again — but this time from a different place:
`prepData` (`modstuff.c`), which parses the module's init-data (`
_midata`) and init-data-reference (`_midref`) tables. The hand-built
module had set both to `0`, on the assumption that `0` means "no
table" (the convention several *other* header fields use, e.g.
`_mexcpt=0` for "no exception handler"). It doesn't, for these two:
`prepData` unconditionally reads a table's `dOff`/`cnt` fields (or
group-terminator) starting at that offset. An offset of `0` makes it
read the module's *own header bytes* as table content, which almost
always fails the bounds check and returns `E$BMID` — coincidentally the
same error code as the earlier sync-check failure, which is what made
this confusing to diagnose at first. Fixed the test module by adding a
real empty `idata` table (`dOff=0, cnt=0`, 8 bytes) and a real empty
`idref` table (two 0 terminators, 8 bytes — `prepData` scans two
reference groups, code-relative and data-relative), and pointing
`_midata`/`_midref` at them.

That fix alone got `F$TLink` past `prepData` and both install and
remove passed. But retries against the *original* broken module (before
adding the idata/idref tables) had a side effect worth its own writeup:
**`mfree` reported 0 free RAM afterward, and a completely unrelated
follow-up test (`dcb-probe.a`) failed with `E$NORAM`.** Reading
`install_traphandler`'s source explains why:

```c
tp->mid=mid;
tp->trapmodule=(mod_trap *) theModule;   /* set BEFORE prepData runs */
tp->trapentry= TO68K(theModule)+os9_long(theModule->_mexec);
err= prepData(pid,theModule,addmem,&tp->trapmemsz,&trapdata);
if (err) return err;                     /* no rollback on failure! */
tp->trapmem= TO68K(trapdata);
```

`tp->trapmodule`/`tp->mid` are set *before* `prepData` runs (needed to
compute `trapentry`), but if `prepData` then fails, the function returns
the error without resetting them. Two consequences: the trap slot now
looks permanently "installed" (`OS9_F_TLink`'s own already-installed
check, `if (tp->trapmodule!=NULL) return E$ITRAP`, would reject any
future install attempt on that slot for the rest of the process's
life), and the module `install_traphandler` itself just linked via
`link_load` is never unlinked — leaked. At process exit,
`unlink_traphandlers`/`release_traphandler` runs automatically, sees
the stale non-NULL `tp->trapmodule`, and calls
`os9free(pid, FROM68K(tp->trapmem), tp->trapmemsz)` — but `tp->trapmem`
and `tp->trapmemsz` were *also* never set (that only happens after
`prepData` succeeds), so this is `os9free(pid, 0, 0)`, which
`memstuff.c`'s `os9free` doesn't treat as a harmless no-op — it falls
through to the "not found" branch and logs
`# os9free: Block at 0x0 (size=0) not found in pid=N's memory list`.
Repeated retries against the broken module compounded this every time,
which is what exhausted the arena.

**Fixed** in `Source/OS9exec_core/modstuff.c`'s `install_traphandler`:
on `prepData` failure, explicitly `unlink_module(mid)` and reset
`tp->trapmodule`/`tp->trapentry`/`tp->mid` back to NULL/0 before
returning the error. Rebuilt (`make`, clean, no warnings), reverified
end-to-end on a fresh `os9exec` instance: both install and remove now
pass, repeatably (ran the test twice in a row), and `pd` confirms the
harness stays fully responsive afterward.

## A toolchain gotcha reconfirmed with hard proof

While chasing why the on-disk module's name field showed `"batch"+00`
instead of the intended `"batch"+0xB8` (sign-bit terminator), a minimal
isolated probe (`dc.b "AB",$FF,"CD",$EE`, dumped raw) proved the
assembler handles `dc.b "string",byte` exactly as expected — no
auto-NUL-termination, no surprises. That ruled out an assembler quirk,
which meant the *executable itself* had been stale. Confirmed directly:
`del batch801` (the previously-linked output) failed with `E$PNNF`
because — per the already-known `l68 -o=<bare-name>` writes-to-`chx`
gotcha — it isn't visible to `del` from the data directory at all, so
none of this session's several `l68 -o=batch801` relinks had ever
actually been preceded by a successful delete of the old one. `l68 -o=`
almost certainly shares the same "doesn't reliably overwrite an
existing output" bug already documented for `r68 -O=`. **Every earlier
run in this batch that appeared to fail (or pass) against `batch801`
may have been silently exercising a stale build.** Confirmed fixed by
linking to a brand-new name (`batch8f`) instead of reusing `batch801` —
the final, correct test results in this report are all from that build.

## Running total

37 of the 68k syscall reference's detailed calls are now `Live`-confirmed
(36 after batch 7 + `F$TLink` this batch). That's the whole detailed
tier except the deliberately-never-tested `F$RTE`/`F$SysDbg` (killing
the caller / hanging the scripted REPL, respectively — not safe to
automate).

## Files

- `test/68k-live-verification/batch8-01.a` — F$TLink install/remove,
  staged behind F$SetCRC
- `Source/OS9exec_core/modstuff.c` — real fix: `install_traphandler`
  rolls back `tp->trapmodule`/`tp->mid` on `prepData` failure
