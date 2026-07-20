# Dogfood report: hand-written OS-9/68000 device driver (2026-07-18)

Task: write a minimal SCF-class device driver module (Init/Read/Write/
GetStat/SetStat/Term, in-memory loopback FIFO), assemble/link it with the
real `r68`/`l68` toolchain, pair it with a hand-written device
descriptor, install it, and exercise it from a real test program via
`tools/os9repl.sh` — the "highest-value next step" the `os9-dev`
`VERIFICATION-BACKLOG.md` had flagged and never done. Artifacts:
`dogfood-driver-loopback.a` (driver), `dogfood-driver-loopback-desc.a`
(descriptor), `dogfood-driver-test.a` (test program) — each file's own
header comment carries the same verification-status detail condensed
here.

## Bottom line: a real, precisely-located wall, not a partial success

**The driver was never invoked. Not once, at any entry point.** This
isn't "couldn't get the register convention right" — it's one level
earlier: os9exec's I/O subsystem does not execute loaded driver-module
machine code for *any* device, custom or otherwise. Two independent
from-source investigations this session (cross-checked against each
other) and three independent live experiments confirmed this:

- **Source-level:** `Source/OS9exec_core/icalls.c`'s `OS9_I_Attach` is
  an explicit dummy — its own comment says "Is only a dummy. Always
  returns 0xFF00FF00 for the device table entry" — it never allocates
  driver static storage or calls Init. `Source/OS9exec_core/filestuff.c`
  dispatches every I$Read/I$Write/I$GetStt/I$SetStt through a fixed C
  function-pointer table (`fmgr_op[]`) wired at startup
  (`init_fmgrs()`) to hardcoded handler sets per a 13-entry internal
  `ptype_typ` enum, decided by `IO_Type()`'s string-prefix matching —
  never by loading and executing an installed module's own code.
  `grep -rin iniz Source/` returns **zero hits**: the `iniz` command
  the shell ships (`/h0/CMDS/iniz`) has no emulator-side implementation
  behind it at all. A follow-up check of `/vmod` (mentioned in a
  `pBadMode` comment as an SCF special case) confirmed it's a separate,
  unrelated hardcoded shim for a legacy Mac Classic picture-display
  protocol, gated out entirely on non-classic-Mac builds — not a real
  driver-loading mechanism either.
- **Live, this session:** even after the descriptor module was
  assembled, linked, and `load`ed resident under the *exact* device
  name `DOGFOOD` (confirmed via `mdir`/`dump` of its own `M$Name`
  string), `dir /DOGFOOD` still failed with `Module Not Found`
  (`E$MNF`, `000:221`) — the same error as before it was loaded at all.
  `iniz DOGFOOD` and `iniz /DOGFOOD` both silently no-op: no error, no
  `devs` entry ever appears (checked before/after every step; `devs`
  stayed an empty table for the entire session, even though the shell
  itself is actively running I/O through `/term`). A real test program
  (`dogfood-driver-test.a`) built and run live printed exactly
  `open /DOGFOOD failed` — confirming the failure at the actual
  `I$Open` syscall/carry-flag level, not a compile or link problem.

Given that, "was Init actually called" (the task's suggested first
debugging step) was answered before a debugger session was even
needed: no static storage is ever allocated for this driver by
os9exec, so there is nothing for Init to have written into, and no way
for a live inspection to show anything but silence. I did not
end up running the OS-9 `debug` command in the way the task
anticipated (breakpoint on Init, step through a live call) — there was
no live call reaching it to break on, and `debug`/`F$DFork` only apply
to `Prgrm`-type modules being run, not to a kernel-invoked driver
jump-table entry. `ident`/`dump`/`mdir` (also part of the documented
"Inspection tools" set) did the actual load-bearing verification work
instead, and did it conclusively.

**What *did* get built and confirmed real, independent of the dispatch
question:** all three modules assemble with 0 errors, link with 0
errors, and are genuine, CRC-valid, correctly-typed OS-9/68k modules —
`ident` confirms `68000 Dev Drv, Object Code, Sharable, System State
Process` for the driver (Ty/La `$E01`, Attr/Rev `$A001`) and
`68000 Dev Descr, Object Code, Sharable, System State Process` for the
descriptor (`$F01`/`$A001`). The driver's jump table was verified
byte-correct by dumping the linked module and matching each `dc.w`
entry against the real disassembled position of its routine (Init's
table word was `$004A`, and Init's first real opcode — `48e7 8080`,
`movem.l d0/a0,-(a7)` — was confirmed sitting at file offset `$4A`).
That's a genuine, non-trivial `Live` result in its own right: a
hand-assembled `Drivr`-type module's header, jump table, and code all
came out self-consistent and correctly linked, which the skill had
never been tested against before this session.

## Did the skill's documented entry-point calling convention match what was observed?

**Not testable — and that's itself the finding.** No entry point was
ever invoked, so there was nothing to compare a register dump against.
What I *can* report precisely:

- **INIT/READ/WRITE/GETSTAT/SETSTAT/TERM (SCF).**
  `device-drivers.md` states plainly under "SCF (Character Device)
  Driver Specifics": *"unlike RBF's complete Driver Static Storage
  offset table above, SCF's fields below (`V_PAUS`, `V_XON`, `V_BUSY`,
  `V_WAKE`) are named in prose only — no offsets, sizes, or total
  static-storage size are documented anywhere in this skill."* That
  claim held up exactly as stated — I could find no SCF register or
  static-storage convention anywhere in either skill, for any of the
  six entry points, and had to invent my own static-storage layout
  from scratch (documented as such in the driver's own header
  comment). This never got tested because nothing ever called in.
- **READ/WRITE (RBF, cited by analogy only).** `device-drivers.md`
  documents RBF's convention as `d0.l`=sector count, `d2.l`=starting
  LSN, `(a1)`=path descriptor, `(a2)`=static storage, `(a4)`=process
  descriptor, `(a5)`=caller's register stack, `(a6)`=system globals,
  and explicitly flags: *"READ/WRITE are the only entry points with a
  documented register convention anywhere in this skill... a skeleton
  for [INIT/TERM/GETSTAT/SETSTAT] can only be written by unconfirmed
  analogy... not from attested content."* I used only the one register
  this pattern shares with every other documented driver/file-manager
  entry point, `(a2)`=static storage, as the sole trusted assumption in
  my driver code — and even that was never actually exercised.
- **IRQ registers, `F$IRQ`, F$Fork's program-entry register table** —
  out of scope for this driver (no interrupts, no forking it directly:
  its module type is `Drivr`, and `module-format.md` confirms
  `F$Fork`ing a non-`Prgrm` module returns `E_NEMOD`, consistent with
  why `debug` couldn't run it directly either) — not exercised, not
  claimed as tested.

So: zero claims from the skill were contradicted, because zero claims
were put to a live test that could contradict them. The skill was
honest in advance that this was the state of things ("no custom driver
or file manager has actually been written and loaded to confirm the
documented calling conventions... treat that specific gap as still
open"); this session closes the *toolchain* half of that gap (module
format, psect syntax, byte layout) but leaves the *register convention*
half exactly as open as it started, for a different reason than
expected — not "the convention was wrong" but "there's no dispatch
path on this platform to test it against at all."

## Was anything in os9-systems-dev wrong, missing, or confusing?

**Wrong (well, misleading for this one specific case) at the skill-suite
level, not within `os9-systems-dev` itself:** `os9-systems-dev`'s own
top-level file states under "Verification": *"os9exec emulates the
kernel side of every mechanism here... and the emulator itself can host
live tests."* That's true for plenty of things this project has
verified before (syscalls, module CRC/parity, BASIC09-calls-assembly,
RBF/host-native permission enforcement) but it is **not true** for the
one mechanism this specific task needed: os9exec does not emulate
driver-module dispatch at all. This is exactly the kind of claim a
reader would reasonably take at face value before attempting this
task, and it cost real time before the two source-investigation forks
settled it conclusively. This is the single most actionable correction
this session surfaced — not a detail inside `device-drivers.md`, but
the skill's own framing of what's testable at all.

**Missing, not wrong:** `device-drivers.md`'s "Known gap" framing
throughout (SCF static storage, INIT/TERM/GETSTAT/SETSTAT registers)
is accurate as far as it goes, but none of it anticipates or warns
about the platform-level gap above. A reader following the skill
exactly as written would reasonably expect *some* live signal to be
obtainable (even just "Init got called, here's its actual register
state") and would have no way to know in advance that the answer is
"none, ever, on this emulator" until they'd built the whole thing and
tried, as I did.

**Confusing, in a way that cost real toolchain time (not a skill
content problem, but adjacent):** device descriptor authoring hits a
concrete gap the skill doesn't flag because it's downstream of
`os9-dev`'s own disclosed `psect` gap, not `os9-systems-dev`'s content:
the one *working*, `Live`-verified `psect` form
(`basic09-vs-68k-differences.md`'s 6-operand shape) turns out to be
usable only for `mod_exec`-shaped module types (`Prgrm`/`Sbrtn`/
`Drivr`, which share the `M$Exec`/`M$Excpt`/`M$Data`/`M$Stack` header
shape) — it silently and unconditionally reserves those same 12 bytes
before the psect body for *every* module type, including `Devic`,
whose real extended header (`M$Port`/`M$Vector`/`M$Mode`/`M$FMgr`/...)
is a completely different shape at those same file offsets. Nothing in
either skill's `module-format.md`/`device-drivers.md`/
`os9-68k-assembly.md` flags that the one working `psect` recipe doesn't
generalize to non-executable module types, and no alternative form
exists that I could find (a shorter, 4-operand `psect` line fails
outright with `*** error - comma expected ***`). This is a genuine gap
in the *toolchain* documentation available to the skill authors, not a
skill mistake per se, but it means `device-drivers.md`'s own Device
Descriptor Fields table (canonical offsets `M$Port`@`$30`,
`M$Mode`@`$37`, etc.) currently cannot be hand-authored byte-accurately
with any assembler syntax this session's search turned up.

## Was anything surprisingly helpful?

- **The prior dogfood session's psect/`I$`-code groundwork transferred
  perfectly, again.** The exact same hand-defined
  `Type_Lang`/`Attr_Rev` pattern and 6-operand `psect` shape from
  `basic09-vs-68k-differences.md` (previously proven for `Prgrm`/
  `Sbrtn`) worked immediately for `Drivr` and (mostly — see the gap
  above) `Devic` too, with zero syntax changes needed beyond swapping
  the type constant. The hand-defined `I$Open`/`I$Read`/`I$Write`/
  `I$Close`/`F$Exit` numeric codes from the earlier
  `dogfood-asm-line-counter.a` session dropped straight into
  `dogfood-driver-test.a` with no rework.
- **`l68`'s error taxonomy stayed diagnostic gold**, exactly as
  `module-format.md` claims: `*** error - comma expected ***` on the
  4-operand `psect` probe pinpointed the exact wrong assumption
  (fewer operands are allowed) on the first try.
- **`ident`/`dump`/`mdir` together were fully sufficient to reverse-
  engineer real linker behavior** (the 12-byte header padding, the
  module-naming-comes-from-`-o=`-not-source finding) without ever
  needing `debug` proper — a genuinely useful escalation path the
  skill's "Inspection tools" section (`module-format.md`) already
  recommends "before deeper debugging," and it held up perfectly here.
- **Two new, concrete toolchain findings this session is confident are
  correct and worth folding into the skills**, beyond the driver-
  dispatch wall itself:
  1. `dc.b 'text'` (single-quoted) fails on `r68` with
     `*** error - value out of range ***` on *every* single-quoted
     string tried, regardless of length/parity; `dc.b "text"`
     (double-quoted) assembles cleanly. Not documented anywhere
     surveyed in either skill.
  2. A linked module's registered name (`M$Name`, what `F$Link`/
     `F$Load`/`mdir` use) comes from `l68 -o=<name>`'s **output file
     argument**, not from the source's `nam`/`psect` name operands —
     confirmed by linking the same object twice under two different
     `-o=` names and watching `mdir`/`dump` report two different
     internal names. This matters for anyone trying to install a
     device descriptor or driver under a specific required name.

## Did I hit any REPL/harness/debugger friction?

Two real ones, both recoverable, neither from `os9exec` itself:

- **Double-`flip -m`ing an already-CR-only file silently collapses it
  to a single line** (all line-ending bytes vanish, not just CR→LF
  confusion) — hit once when I re-ran `flip -m` on a file I'd already
  converted, without checking state first. `using-os9exec-repl.md`
  recommends `flip -t` to check current state before converting but
  doesn't warn that a wrong-direction double-flip is *destructive*
  rather than a harmless no-op; recovered by `flip -u` back to LF and
  re-editing, no data lost since the host file was still readable, but
  this is worth a line in the skill given how easy it is to hit during
  an iterate-edit-reflip loop like this task's.
- **`l68 -o=<name>` writes its output to the execution directory
  (`chx`), not the current data directory** — same "fork lookups use
  `chx`, not `PATH`"/output-defaults-to-`chx` pattern
  `using-os9exec-repl.md` already documents for `cc`'s `-F=` flag, but
  not stated for `l68` directly; cost one confused round of "the
  linker printed no error and no file appeared" before checking
  `/h0/CMDS` (`chx`) and finding it there. Worth a one-line addition
  cross-referencing the existing `cc -F=` note.

No sub-prompt or gate-desync friction this session — every command
ran from the ordinary shell prompt, no `vi`/`debug`/BASIC09 sub-prompts
involved.

## Overall: is os9-systems-dev usable for real driver-writing work?

**For the toolchain/module-format half: yes, and this session
materially improved that gap** — a `Drivr`-type module can now be
hand-assembled, linked, and verified byte-correct from the guidance
available across both skills plus direct experimentation, which was
genuinely unproven before today. **For the actual purpose of writing a
driver that does something on os9exec: no, and no amount of reading
either skill would have gotten a user unstuck**, because the blocker
isn't a documentation gap that better prose could close — it's a real,
confirmed absence of the underlying mechanism in the current emulator.
A user following `os9-systems-dev` exactly as written, in good faith,
building everything correctly, would still end this exercise exactly
where I did: a well-formed module that nothing ever calls. The single
highest-value fix for next time isn't inside `device-drivers.md` at
all — it's correcting the skill-suite-level "the emulator itself can
host live tests" claim to scope it away from driver/file-manager
module dispatch specifically, so the next person doesn't spend a full
session re-discovering the same wall from scratch. This is a genuine
`os9exec` feature gap, not a bug in existing behavior — implementing
real `I$Attach`/driver-module dispatch would be a substantial emulator
feature addition, well beyond this session's scope to fix, and is
noted here for the record rather than attempted.
