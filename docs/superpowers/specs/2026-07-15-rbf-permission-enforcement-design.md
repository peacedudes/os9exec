# RBF Permission Enforcement — Design

## Problem

Every file on an RBF disk image is created owned `0.0` and no code path ever
checks the attribute bits against the caller's identity — even a file with
every permission bit cleared stays fully readable/writable/deletable, as
long as the accessing process isn't blocked some other way. `is_super()`
(`procstuff.c:220`) and `E_PERMIT` exist but the only file-related caller of
either is the `stop` command. This is tracked in `ROADMAP.md` under "File
permissions and ownership don't work."

Root cause, confirmed by reading the code (not assumed):

1. `Create_FD` (`file_rbf.c:2621`) clears the whole FD sector, then sets
   only `FD_ATT`/link-count/size — `FD_OWN` (FD sector offset `$01`, 2
   bytes) is never written, so it stays `0.0`.
2. Nothing in `file_rbf.c` ever compares a caller's `pd._group`/`pd._user`
   against a file's owner or attribute bits. `pRopen`, `pRdelete`,
   `pRmakdir`, `pRsetatt` all proceed unconditionally.

A third open question from the ROADMAP — whether `login` genuinely drops
privilege via `F$SUser` — is **already answered**: prior live REPL testing
(recorded in the `os9-dev` skill's `using-os9exec-repl.md`) confirmed a
logged-in non-super account shows a distinct `Grp.Usr` via `procs`. Not
re-verified here as a research question, but the enforcement tests below
exercise it in practice.

## Scope

- **RBF disk images only.** Host-native directories (`os9exec`'s directory
  shim) have no real permission bits to enforce and must keep behaving
  exactly as they do today — untouched by this change.
- Ownership stamping **and** enforcement, landed together (they're only
  meaningful in combination — enforcement without real ownership is inert,
  and there's already a pinned test written for the combined behavior).
- Directory-traversal permission checks (not just target-file checks) are
  in scope, matching genuine RBF as closely as possible.
- No `chown`. Grepping the whole SS dispatch table turned up no
  `SS_Own`/ownership-change SetStat code anywhere in this codebase, and
  real OS-9 doesn't support changing ownership post-creation either.
  Ownership is set once, at creation, and never changes.

## Permission model (matches the documented RBF model, not Unix)

The `os9-systems-dev` skill's file-managers reference, sourced from the
Disk File Organization manual, states the real rule directly: *"Every file
open checks access permission on every directory in the pathlist plus the
target file itself — no read permission on a directory means nothing under
it is reachable."* This is **read-only** gating traversal — OS-9 has no
Unix-style read-vs-execute split for directories. A directory's execute bit
is otherwise unused; the target file's own execute bit only matters when
the *target itself* is opened in execute mode. This was deliberately chosen
over an added Unix-style directory-execute check, since fidelity to real
RBF is the priority here.

`FD_ATT` bit layout (already documented, unchanged): `0x01` owner-read,
`0x02` owner-write, `0x04` owner-execute, `0x08` public-read, `0x10`
public-write, `0x20` public-execute, `0x80` directory. No group class —
only owner and public.

## Design

### 1. Owner stamping

Add `Get_FDOwn`/`Set_FDOwn` accessors next to the existing `FDAtt`/`FDSize`
family in `file_rbf.c`, operating on `FD_OWN` (offset `$01`, 2 bytes) via
the existing `GET_OS9W`/`SET_OS9W` macros. Pack the owner word the same way
this codebase already packs group.user elsewhere (`fcalls.c:702`:
`os9_word(pd->_group)<<BpB | os9_word(pd->_user)`). Ownership is decided by
a full 16-bit match on this packed word (group and user both matching
exactly) — self-consistent, since the same formula both stamps the owner
word at creation and rebuilds the caller's word for comparison (see
`CallerOwner` in `file_rbf.c`).

- `Create_FD` gains an owner-word parameter, stamped from the *creating*
  process's `pd._group`/`pd._user`.
- `CreateNewFile` (regular file/dir creation via `I$Create`) and
  `pRmakdir` (real directory creation) both thread the owner word through
  at their respective call sites.

### 2. Permission primitive

One new static helper in `file_rbf.c`:

```
Boolean has_perm( ushort pid, byte att, ushort ownerWord, byte want )
```

`want` is one of the three bit-pairs (owner-bit, public-bit) for
read/write/execute. Logic:

- `is_super(pid)` → always true. This is the key blast-radius control:
  every existing test runs as super-user, so **zero existing test
  behavior changes** — only a test that explicitly `login`s as a non-super
  account can ever hit a `false` from this helper.
- Otherwise: compare the caller's full 16-bit packed group.user word
  against `ownerWord` (both group and user must match exactly — see
  `IsOwner`/`CallerOwner`). This is self-consistent with §1: the owner
  word is stamped and compared using the identical packing formula, so a
  full-word match is the correct (and only sensible) comparison. Match →
  test the owner bit. No match → test the public bit.

### 3. Enforcement wiring

All in `pRopen`'s directory-walk loop, since `pRdelete` and `pRmakdir`
both resolve paths through `pRopen` already — one choke point covers all
four operations:

- **Every directory searched** (root, and each intermediate directory
  descended into before the final path component) requires **read**
  permission — the traversal check.
- **The final target** is checked against the caller's actually-requested
  access mode (the R/W/E bits already present in `*modeP`), mapped to the
  matching owner/public bit pair. An execute-mode open only tests the
  execute bit — never read — so loading/running a program only requires
  execute permission on the file itself, matching how `F$Load` is
  documented to work.
- **Create** (new file, or `makdir`) additionally requires **write**
  permission on the parent directory when the target entry isn't found.
  Since read is already required to search that same directory, a
  write-only-no-read directory can't be created into either — read is a
  prerequisite for reaching it at all, same as real OS-9.
- **Delete** (`pRdelete`, which opens-then-removes) requires write
  permission on the parent directory, in addition to whatever the
  open-for-delete mode already checks on the target.
- `pRsetatt` (the `attr` command's SetStat, `SS_Attr`) requires the caller
  be the **owner or super-user**. Nobody else may flip permission bits on
  a file they don't own.

### 4. Errors

Permission-denied returns `E_FNA` (File Not Accessible) — the documented
real-OS-9 code for "open attempted without correct access permissions,"
already used by `pRopen`'s existing wrong-type error paths, so it composes
naturally rather than introducing a new error class. `pRsetatt`'s
owner-only check also returns `E_FNA`.

## Testing

Flip the pinned `fs: KNOWN-LIMITATION permissions not enforced` test
(`test/Sources/OS9Tests/main.swift:696`) into a real enforcement test:

- `login` as a non-super account with a blank password (`dog` or `claude`
  — avoids the interactive-password hang noted in the REPL skill doc).
- Create a file as that account, strip all bits, confirm that same
  process can no longer read/write/delete it.
- Confirm a *different* non-super uid is blocked from a file it doesn't
  own, even with some bits set (owner-only bits, tested from the
  non-owning account).
- Confirm super-user still bypasses every check unconditionally (today's
  behavior, preserved).
- Confirm a host-native device (not RBF) is completely unaffected —
  permission stripping there should remain a no-op, same as today.

Re-run the full existing suite (`swift run --package-path test
OS9Tests`) to confirm the super-user bypass keeps every pre-existing test
green, since none of them log in as a restricted account today.

## Out of scope / explicitly deferred

- `chown` / ownership changes after creation — no mechanism exists in this
  codebase or in real OS-9; not adding one.
- Record-locking interaction with permissions — orthogonal, untouched.
- Host-native directory permission emulation — explicitly not real RBF,
  stays exactly as-is.
