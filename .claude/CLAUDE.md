# OS9exec project rules

OS9exec runs real OS-9/68k binaries on a modern host: a 68000 emulator plus a
reimplementation of the OS-9 kernel. `README.md` is the human-facing tour; this
file is what you need to be useful in the first five minutes.

## Getting running

```sh
export OS9DISK=$HOME/Developer/os9/play/oskBoot   # the OS-9 system disk
make                      # build ./os9exec (warning-clean, or it is a bug)
make test                 # the integration suite -- expect 192/0, ~1 min
./os9exec shell           # a live OS-9 shell
```

**`OS9DISK` names the system disk, and there is no `h0` in this repo.** The
symlinks that used to sit here are gone: there are two system disks in play --
a licensed one and the freeware one -- each has to stand alone (its own
`termcap`, `SYS/errmsg`, the works), and **freeware must be `/h0` for
historical reasons** (its `GAMES` and termcap expect that name). Nothing in
the tree may assume which disk is mounted, so everything reads `$OS9DISK`.

The rule, in one line: **host side names the disk with `$OS9DISK`; guest side
says `/dd`.** A repo-relative path is neither, and is what was removed.

Unset is handled honestly rather than guessed: `tools/conformance.sh` drops to
`--noshell` (which needs no system disk at all) and says so; the Swift harness
refuses with the line to type. Silently testing the wrong disk would be worse
than not running.

**Do not write to the system disk.** The suite writes everything to a per-run
scratch device (`/h5` -> a private temp dir, plus the emulator's working
directory, so `mount -k` images land there too). Two sessions, or two suites,
can therefore run at once. Before this existed they deleted each other's
fixtures and produced failures indistinguishable from real regressions.

## Other targets

```sh
make test-notick   # same suite with the system tick off (-q); the fallback path
make warnings      # clang + gcc-in-docker + mingw; all three must be 0/0
make test-linux    # the suite against a real Linux build, in Docker
```

`make warnings` prints `NOT BUILT` if a leg produced no binary -- a 0/0 score
from a build that never ran is not a pass. Same for any check you write here:
**make it fail once before believing it.** Three checks in this codebase were
found unable to fail (a verification assembling a deleted file, a guard whose
condition is never true, an assertion listing the wrong directory).

## Things that will bite you

- **The system tick is ON by default** (100Hz, real OS-9 behaviour): processes
  are pre-empted. `-q` (or `-q0`) turns it off, `-q<ms>` retunes it. A system
  call is never cut in half either way -- syscalls run as host C outside the
  emulation loop, which is what guarantees it, not the `regs.s` test that looks
  like it does.
- **Never `pkill -f os9exec`.** Other sessions and the test suite match that
  pattern; you will kill their runs and manufacture failures. Match a path.
- **Several Claude sessions may share this repo.** Expect commits and modified
  files you did not make. Leave them alone; do not revert or "fix" them.
- **The roadmap is `ROADMAP-68k.md` (os9exec/68k) or `ROADMAP-6809.md`** --
  gitignored, local only, never committed. There is no plain `ROADMAP.md`; a
  stale index by that name was deleted on 2026-08-04 after items got appended
  to it that belonged in the 68k file. A roadmap holds OPEN work only: done
  items are deleted (git log and memory keep the history), and anything
  decided against moves to `DECISIONS-68k.md` / `DECISIONS-6809.md`.
- The suite prints a filtered preview on failure that drops `#`/`$` lines, so an
  empty `output:` does NOT mean the run produced none.

## Provenance of os9defs/

The `Source/OS9exec_core/os9defs/` headers are OURS: reconstructed from books
and documentation found online, using the information rather than any Microware
source. Copyrighted facts, not stolen code -- so they are committed.

They are also SECOND-HAND and not guaranteed Microware-compatible (each header
says so at its top). When something disagrees with real OS-9, suspect the
header at least as readily as the code using it. The proprietary Microware
headers that once lived here are gone; no access restrictions apply.
