# Dogfood report: 68k syscall live-verification batch 11 (2026-07-20)

Continues batch 10. `F$UnLoad`, `F$SUser`, `F$Sleep` — `batch11-01.a`.

## Confirmed cleanly

- **`F$UnLoad`**: loaded a real module, unloaded it by name with a
  deliberately wrong `d0.w` (type/language) to confirm that declared
  input is ignored (like `F$CpyMem`'s PID, `OS9_F_UnLoad`'s C body
  never reads `rp->d[0]` — it looks the module up by name alone). Then
  confirmed the module was genuinely gone, not just link-counted down,
  via a follow-up `F$Link` on the same name correctly failing `E$MNF`.
- **`F$SUser`**: no permission check in `OS9_F_SUser` at all — any
  process can set its own identity to anything, no superuser
  restriction. Confirmed safe to test freely regardless, since the
  change only touches the *calling process's* own descriptor fields
  and dies with that process at `F$Exit` — never leaks into the
  interactive shell that launched it. **Important asymmetry found and
  avoided**: `F$ID`'s output packs group/user into a *single word*
  (`group<<8|user`, one byte each, confirmed via `BpB=8`), but
  `F$SUser`'s input reads `hiword(d1)=group`/`loword(d1)=user` as two
  full 16-bit fields. Feeding `F$ID`'s own output straight into
  `F$SUser` would silently set the wrong identity instead of restoring
  it — caught during test design, before ever running it. Used
  explicit, self-chosen values on both sides instead (group=99,
  user=42), verified the change via `F$ID`'s own packing formula.
- **`F$Sleep`**: `d0.l=1` (documented no-op) and a short positive tick
  count (5, ~50ms) both returned promptly. `d0.l=0` (indefinite sleep
  until signaled) deliberately never tested — nothing in this harness
  could safely signal it, and it would hang the REPL. The function's
  own internal comment header ("Output: d0.w=terminating child
  process' ID...") is stale/wrong, copy-pasted from `F$Wait`'s shape —
  the real code writes no output registers at all; the skill documents
  the actual behavior, not the misleading comment.

## Test-construction bugs, all caught before or during the first live run

- **`F$Load`'s access-mode byte**: first draft used `moveq #1,d0` by
  analogy with other calls' "mode 1" conventions, and it failed
  `E$PNNF`. Reading `OS9_F_Load`'s source explains why: `exedir` (which
  directory it searches) is `IsExec(mode) || mode==0 || mode==0x80`,
  and `IsExec` checks bit 2 (`poExec=$04`), not bit 0 — `mode=1` has
  neither bit set, so it searched the *data* directory instead of
  *exec*, where the file actually was. `batch2-01.a`'s already-working
  test used `clr.l d0` (mode=0) for exactly this reason; matched that
  instead of re-deriving it.
- **`l68 -o=<name>` sets a module's real internal name at link time,
  overriding the source's own `psect` declaration — confirmed live,
  not assumed.** `childprg68k.a`'s `psect` names the module
  `childprg68k`, but after `l68 -o=child68kf childprg68k.r`, `mdir`
  showed it registered as `child68kf`, and `F$UnLoad("childprg68k")`
  correctly failed to find it. Every batch this session that links a
  test to a "fresh name to dodge the stale-overwrite gotcha" has been
  silently renaming that module's own directory identity away from
  what its source declares. Harmless for programs run directly by file
  name (the shell resolves those by path, not by module-directory
  name), but critical for anything doing a name-based `F$Load`/
  `F$Link`/`F$UnLoad` on a *different* file, which is exactly this
  batch's Stage 1 — fixed by using two separate name strings (the file
  name for `F$Load`, the module's real registered name for
  `F$UnLoad`/`F$Link`), not the assumption that they'd match.
- **A leftover resident module from an earlier failed run broke a
  rerun's assertion.** The first live attempt failed at `F$UnLoad`
  (root cause above), leaving `child68kf` still resident afterward.
  Rerunning against the same live instance would have had `F$Load`
  silently reuse the existing entry (bumping its link count rather
  than genuinely reloading), which would have made the "F$Link should
  now fail" assertion at the end of Stage 1 pass for the wrong reason.
  Restarted the REPL for a clean module directory before the real run,
  rather than trust a possibly-polluted one.

## Running total

48 of the 68k syscall reference's calls are now `Live`-confirmed (45 +
3 this batch).

## Files

- `test/68k-live-verification/batch11-01.a`
