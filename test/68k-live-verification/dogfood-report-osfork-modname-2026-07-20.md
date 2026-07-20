# Dogfood report: os9fork() modname/datasize (2026-07-20)

Resolves the two "known gap" items already flagged in
`c/os9-clib-reference.md`'s `os9fork()` row. Small, self-contained
follow-up after the 68k syscall audit finished.

## `modname`: bare names resolve, confirmed live

`os9fork("childprg68k", 0, 0, 1, 1, 4096)` — no leading `/` — forked
correctly (real PID returned, `wait()` reported status 77, matching
`childprg68k.a`'s own `F$Exit(77)`). Bare-name resolution works the
same way `F$Fork` itself already does (already proven in batch2 with a
raw assembly call).

**First attempt failed, but not for the reason under test.** The
target module has to actually be present in the exec directory the
running program resolves at runtime — `cc`'s own build requires
`chx /h0/CMDS` (to find its sub-tools), and the compiled program
inherits that same `chx`, so `childprg68k` had to be linked into
`/h0/CMDS` specifically, not left in the test's working directory
where earlier assembly batches kept it. Once it was, the bare name
resolved cleanly. A pure directory-placement mistake, not a real
resolution-rule ambiguity — worth recording so a future session
doesn't mistake it for one.

**Separately, `cc <file>.c` where the filename contains a hyphen
produces a module the shell can't invoke by typing that name** — it
splits at the hyphen and treats the remainder as arguments
(`dogfood-osfork-modname` → tried to run `dogfood` with args
`-osfork-modname`). Renamed to `osforktest.c` (no hyphen) and moved on;
not chased further as a general rule, just a naming trap to avoid for
any `cc`-built module meant to be typed as a shell command.

## `datasize`: source-confirmed, not just live-guessed

`os9exec`'s `F$Fork` implementation (`OS9_F_Fork` → `procstuff.c:1468`)
calls `prepData(newpid, theModule, memplus+paramsiz, &memsiz, &mp)` —
the exact same `prepData()` already read in detail for the `F$TLink`
batch. Its formula:

```
memsz = module's own _mdata + module's own _mstack
      + EXTRAEMUSTACK + memplusall + (memplus + paramsiz)
      rounded up to 16 bytes
```

`memplus` here is `F$Fork`'s `d1.l`, which is exactly what
`os9fork()`'s `datasize` parameter maps to. So `datasize` is **headroom
added on top of** whatever the compiled module's own header already
declares needing (its own static data plus its own stack requirement)
— not a replacement absolute total, and not something that needs to
account for the C runtime's baseline needs from scratch. The earlier
"a 4096-byte guess worked" observation is explained: 4096 is a
reasonable *extra* cushion regardless of what a given program's own
compiled-in requirement already is, which is why it "just worked" for
more than one program without ever being derived from a documented
rule.

## Files

- `test/68k-live-verification/dogfood-osfork-modname.c` — kept as a
  worked example (matches this project's `dogfood-c-line-counter.c`
  convention); the deployed OS-9-side artifacts (both bare-name and
  hyphenated build attempts, plus the temporary `childprg68k` copy in
  `/h0/CMDS`) were cleaned up after confirming the finding, since
  `/h0/CMDS` is the real shared system command directory, not a
  sandbox.
