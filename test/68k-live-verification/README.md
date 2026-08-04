# 68k live-verification tests

Source/command sequences behind `Live`-tagged claims in the `os9-dev` and
`os9-systems-dev` skills (`~/.claude/skills/`), run against real `os9exec`
via `tools/os9repl.sh` (the 68k sibling of the 6809 harness in
`../6809-live-verification/`, which drives real NitrOS-9 under XRoar
instead — see that directory's README for what's 6809-only).

Naming: `<topic>.a`/`.s` for 68k assembly test programs, `<topic>.c` for
C, `<topic>.bas` for BASIC09, `<topic>.txt` for plain REPL
command/output transcripts. Each file's header comment/first line names
the skill file and claim it backs.

Not automated regression tests — rerunning one means driving
`os9repl.sh` by hand or via an agent, same as when it was first written.

## This is dogfood, not an instrument — and it cannot become one cheaply

98 files, of which **6** have a machine-checkable oracle
(`../live-verification-manifest.json`: `f-id-time-cmpnam`,
`osfork-bare-modname`, `eoflock-deadlock`, `bmode-repro`, `getwd-dirfd`,
`lostupdate-trio`). Everything else is checkable only by a human reading it.
The corpus grows faster than that coverage does, and the question of whether
to convert it into `../68k-conformance/` (CONF68K) was open for a long time.

**Triaged, and the answer is mostly no.** The reason is the oracle, not the
effort:

- **CONF68K's expectations come from Microware manuals.** `DOCS/claims.md`
  cites the page behind every claim, which is what makes a FAIL on somebody
  else's real OS-9/68k hardware *mean* something.
- **These tests' expectations come from `68k/syscall-reference.md` and from
  reading os9exec's own C.** Their headers say so plainly — "register
  conventions cross-checked against os9exec's own `fcalls.c`", "F$SRqCMem
  shares `OS9_F_SRqMem`'s own C body … its color parameter is accepted but
  never read". That is a description of this implementation, not a claim about
  OS-9. Promote it unchanged and a real OS-9 system reports a divergence that
  says nothing.

Two concrete demonstrations that the difference is real, not pedantic:

- `batch12-01.a` expects `E$UNKSVC`/208 from `F$SSpd`, `F$Mem`, `F$SchBit`,
  `F$AllBit`, `F$DelBit`, `F$Trans` and `F$UAcct`, because os9exec routes all
  seven to `OS9_F_UnImp`. Real OS-9 *implements* them. As a conformance test
  this would fail seven ways on correct hardware.
- `batch3-02.a`'s header still states that `F$Send` has no PID-0 broadcast and
  that `F$Icpt` "does not work". **Both findings were retracted** (project
  memory `fsend-pid0-broadcast-already-works`,
  `ficpt-signal-delivery-already-works`); they already work. The file has been
  wrong ever since and nothing noticed, because nothing checks it — which is
  the cost of an uncheckable corpus, stated as concretely as it can be.

### Worth promoting (expectation is documented, not source-derived)

Each still needs its expectation re-derived from a manual page and a
`DOCS/claims.md` citation written — that re-derivation *is* the work:

- `batch9-01.a` — `F$Julian`/`F$Gregor` round-trip. Pure documented
  arithmetic, identical on any conforming system, no device needed.
- `batch1-01.a` — `F$CmpNam` name comparison, and `F$Time`'s documented
  packing.
- `batch3-01.a` — `I$Create`/`I$Delete`/`I$MakDir`/`I$ChgDir`/`I$Seek`/`I$Dup`
  register contracts. Note its own finding that `d1` is a BYTE, not a word,
  for `I$Create`/`I$MakDir` — check that against the manual before trusting
  it, since it was read off the source.
- `batch4-01.a` — `F$CRC`, whose expected value is fixed by the documented
  polynomial rather than by us.

### Not promotable, and that is fine

`dogfood-*.a` (12), `dogfood-*.bas`, `dogfood-*.c` — device drivers, a file
manager, debugger and BASIC09 exercises. These were written to find out
whether os9exec could do a thing at all, and they did their job; several
emulator fixes in `git log` came out of them. They have no expected value to
compare against and are not meant to.

**Rule going forward:** a new file here is dogfood by default and does not
count as verification coverage. If a behaviour is worth guarding, it belongs
in CONF68K with a manual citation, or in the Swift suite
(`../Sources/OS9Tests/main.swift`) if it is about os9exec rather than OS-9 —
that distinction is the whole point.
