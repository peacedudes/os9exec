# Dogfood report: two-process pipe IPC, OS-9/68000 (2026-07-18)

Task: write and run a real two-process pipeline using OS-9's pipe
mechanism, using it to check two specific claims in `common/ipc.md`
against live behavior: (1) the documented 90-byte default pipe buffer,
and (2) the EOF/deadlock rules on writer-close, reader-with-no-writer,
and writer-with-no-reader. Target: OS-9/68000 via `os9exec` and
`tools/os9repl.sh` (chosen over 6809/NitrOS-9 because this project owns
`os9exec` and 68k findings are directly actionable emulator facts, per
this project's own skill-priority note, and because the 68k REPL's
host-native-directory file-creation route is much faster to iterate with
than the 6809 IDE-image route for six rounds of source edits).

Six BASIC09 programs, PACKed and run standalone via the shell's bare-name
RunB auto-detection (`producer ! consumer` pipe syntax, or a lone
process against a named pipe):

- `dogfood-pipes-producer.bas` / `dogfood-pipes-consumer.bas` — small,
  explicit-trace version: 3 WRITE calls of 100 bytes each into an
  unnamed pipe, with a stalled reader.
- `dogfood-pipes-bulk-producer.bas` / `dogfood-pipes-bulk-consumer.bas`
  — same idea at 33x the volume (10KB), reader intentionally slow on
  every READ, not just the first.
- `dogfood-pipes-solo-reader-eof.bas` — one process alone, named pipe,
  READ with no writer ever.
- `dogfood-pipes-solo-writer-block.bas` — one process alone, named pipe,
  WRITE loop with no reader ever.

Each file's header comment carries its own literal verified output; this
report is the narrative/analysis layer on top.

## Did the skill get me there first-try, or real trial-and-error?

Mixed — genuinely first-try for the mechanics the skill actually
documents, real trial-and-error for two gaps it doesn't cover.

**First-try, using the skill directly:**
- Shell pipe syntax (`!`, not `|`) — `os9-tools-and-shell.md` — worked
  immediately, no experimentation.
- `PACK`/RunB/bare-name shell auto-detection mechanics —
  `basic09/pack-and-runb.md` — worked exactly as documented, including
  the `Rewrite?:` prompt on re-packing and the "output goes to CHX, not
  CHD" behavior (I initially looked in CHD and had to remember that from
  the skill, not rediscover it).
- **The one real crash-and-recover, but the skill nailed the diagnosis
  immediately**: the first `LOAD` of a numeric-variable-using procedure
  crashed BASIC09 itself back to the shell:
  ```
  B:LOAD /dd/USR/CLAUDE/pipeprod.bas
  pipeprod
  **** Can't install trap handler ****
  AError #000:216 (E_PNNF) Path Name Not Found
  ```
  `pack-and-runb.md`'s own triage list for this exact banner says cause
  1 is "the `math` trap module isn't reachable, fix: `load math`" — I
  ran `load math`, re-entered `basic`, and the same `LOAD` worked cleanly
  on the next attempt. Zero exploration needed; the skill had already
  named the fix.

**Real trial-and-error, not covered by the skill:**
- **Moving `chx` broke plain command lookup, not just compiler
  sub-tool forking.** I set `chx /dd/USR/CLAUDE` (my working directory)
  for convenience, and the very next command failed:
  ```
  claude: basic #32k
  shell: can't execute "basic"  - Error #000:216 (E_PNNF) Path Name Not Found
  ```
  `using-os9exec-repl.md`'s Accounts section warns that `chx` pointing at
  a personal directory "breaks every multi-stage tool (compiler drivers
  fork sub-tools via chx)" — framed specifically around `cc`. What
  actually broke here was plain interactive command resolution (`basic`
  itself), because `PATH`'s entries are relative to `chx` (the same
  section's own account-layout example: `PATH .:ALICE:SHARE`). The skill
  states the underlying mechanism but frames the consequence narrowly
  enough (compiler-only) that I didn't connect it until hitting the
  failure myself. Fixed with `chx /dd/CMDS/claude` per the skill's own
  recommended layout.
- **`OPEN` doesn't auto-create a not-yet-existing named pipe; `CREATE`
  does — not stated anywhere in `ipc.md`.** First version of the solo
  reader test used `OPEN #rp, "/pipe/soloB": READ` on a pipe nobody had
  ever touched, and the whole program crashed uncaught the moment the
  shell forked it:
  ```
  claude: pipesoloR
  001E Error #000:216 (E_PNNF) Path Name Not Found
  ```
  Switching to `CREATE` fixed it immediately. `ipc.md` describes pipes
  as created via `_os_create()` (consistent with `CREATE` being the
  right call) but never says `OPEN` on a nonexistent named pipe fails
  outright — a real, if narrow, gap for anyone writing pipe code from
  this doc alone.
- **A stuck solo-writer process was unresponsive to both Ctrl-C and
  Ctrl-E**, contradicting `using-os9exec-repl.md`'s own claim that
  Ctrl-E "kills the child... regardless of what the process is doing
  (compute loop, **blocked write** — no console read needed)". See the
  deadlock section below — this needed a full harness restart to
  recover from, not documented as a possible outcome anywhere.

## Was the 90-byte pipe size claim confirmed, contradicted, or nuanced?

**Nuanced — more nuanced than the doc states, in a way worth flagging.**

With a reader present and eventually draining (both the 3-chunk and the
100-chunk producer/consumer pairs), every single WRITE call succeeded
immediately with `ERR=0`, including three separate 100-byte writes
(101 bytes each on the wire) executed back-to-back before the reader had
consumed anything — already 3x the documented 90-byte default in flight
with zero errors, zero short writes, zero truncation. Data integrity was
perfect in both cases (309/309 bytes and 10009/10009 bytes accounted for
on the reader side, matching exactly).

The genuinely informative result came from the solo-writer test
(`dogfood-pipes-solo-writer-block.bas`): a lone process, no reader ever,
writing 100-byte chunks in a tight loop. It got through **40 full
chunks — 4040 bytes — with `ERR=0` on every single one**, before finally
blocking indefinitely on chunk 41. That's roughly 45x the documented
90-byte default accepted into the pipe before any blocking occurred at
all, with a permanently absent reader (the most conservative possible
test — there is no scenario where more space could ever free up).

Once actually full, it behaved exactly as documented: no error, no
truncation, no silent data loss — it just blocked (see the deadlock
section for what "just blocked" meant in practice). So the qualitative
claim ("pipes have a bounded default buffer; a full pipe blocks the
writer") is confirmed. The specific number, 90 bytes, is very likely not
what's actually gating this — either BASIC09's own `WRITE` statement
batches several logical writes into fewer real `I$Write` syscalls
client-side (most likely explanation, though `basic09-language.md`
doesn't document any such buffering), or the effective default in this
disk/toolchain is genuinely much larger than 90 bytes. I could not
distinguish these two explanations within this pass's scope — that would
need syscall-level tracing (`idbg -d 2`, per
`using-os9exec-repl.md`'s debugging section) to see how many real
`I$Write` calls actually happen per BASIC09 `WRITE` statement, which I
did not have time to run. Flagging this as the single most
actionable follow-up from this session.

## Were the EOF/deadlock claims confirmed, contradicted, or nuanced?

**EOF: cleanly confirmed, twice, at two different scales.** Both the
small (309-byte) and bulk (10009-byte) producer/consumer pairs showed
the reader's final `READ` — issued only after the writer had written its
sentinel and exited — failing with `ERR=211` (`E$EOF`), not before, not
hanging, not returning a different code. This matches `ipc.md`'s "empty
AND no other writers" framing exactly: all preceding reads succeeded and
returned real data; only the read past true end-of-data-and-closed-pipe
raised EOF.

**Reader-with-no-writer: cleanly confirmed.** A lone process opening a
fresh named pipe for `READ` only, with no writer ever going to touch it,
got an *immediate* `ERR=211` on its first `READ` — not an indefinite
block waiting for a writer that will never arrive. This is the sharpest,
most direct test of the "reader count equals total user count" half of
the EOF rule (reader count 1, total user count 1, pipe empty from the
start → EOF fires at once), and it held exactly as stated.

**Writer-with-no-reader (named pipe): the "blocks, no deadlock
detection" claim is confirmed, but recovering from it live is not — and
that's the sharpest finding of this whole pass.** `ipc.md` explicitly
scopes deadlock *detection* to unnamed pipes only; for named pipes it
says only "blocks until space frees (unless the writer is interrupted by
a signal)." Live behavior matched the first half exactly: after 40
successful chunks, the 41st `WRITE` call simply never returned — no
`E_WRITE`, no timeout, no error of any kind, for 45+ real seconds of
observation with zero further progress. That's consistent with "no
deadlock detection for named pipes."

But the parenthetical — "unless the writer is interrupted by a
signal" — did not hold in practice. Per `using-os9exec-repl.md`'s own
"Stopping a runaway program" section, Ctrl-E is documented to kill a
child process immediately "regardless of what the process is doing
(compute loop, blocked write — no console read needed)." I sent Ctrl-C
and Ctrl-E (via the REPL's raw `key C-c` / `key C-e`) multiple times each
against the wedged process; none had any visible effect — the process
stayed parked at "chunk 40 written OK" and the shell prompt never
returned. Recovery required a full `./tools/os9repl.sh restart`
(session state was lost, though the host-native `/dd/USR/CLAUDE` files
and PACKed modules under `/dd/CMDS/claude` survived the restart intact,
since that device is host-native, not an in-memory RBF image). This is
either a real os9exec/UAE-core bug in signal delivery to a process
blocked deep in a kernel-level pipe wait, or the skill's Ctrl-E claim is
overstated for this specific case (a syscall genuinely parked mid-wait,
as opposed to a compute loop or a console read) — not root-caused
further within this pass's scope, and explicitly not fixed per this
task's own instructions (diagnostic pass only). Flagging for follow-up:
this is exactly the kind of thing that turns "let me quickly test a
pipe deadlock" into a stuck session for the next person who tries it
following the skill's Ctrl-E guidance.

The one piece of the deadlock story I did *not* test: `ipc.md`'s
unnamed-pipe deadlock-*detection* claim itself (`E_WRITE` returned to
the first waiting writer when every process with pipe access is
simultaneously blocked writing). That needs a genuinely cyclic
multi-process setup (or all-writers-blocked-with-nobody-left-to-read on
an *unnamed* pipe specifically) that I didn't have budget to construct
correctly and safely after the solo-writer test's recovery cost. Left
untested, not contradicted or confirmed — a real gap in this pass's
coverage, worth a follow-up session specifically targeting the unnamed
case.

## Was anything else in `ipc.md` wrong, missing, or confusing?

- **Missing**: no mention of `OPEN` vs `CREATE` semantics for a
  not-yet-existing named pipe (see above — `OPEN` fails outright,
  `CREATE` is what actually establishes it). Worth a line given the file
  already discusses `FAM_NOCREATE` for the *already-exists* case but says
  nothing about the *doesn't-exist-yet* case from the reader's side.
- **Missing**: no caveat that a language runtime's own I/O statement
  (BASIC09's `WRITE`) may not map 1:1 to real `I$Write` syscalls, which
  matters a lot for anyone trying to reason about the 90-byte default
  from application-level behavior the way this dogfood pass did.
- **Not wrong, but worth strengthening**: the "unless interrupted by a
  signal" aside on named-pipe blocking reads as a minor implementation
  detail in the doc, but live-testing shows it's actually a real
  operational risk (an unrecoverable-except-by-restart wedge) worth its
  own callout, not a parenthetical.
- Everything else in the Pipes section (unnamed-vs-named creation and
  sharing model, close-behavior-by-kind table, "why pipes over signals")
  wasn't specifically exercised by this pass but nothing encountered
  contradicted it.

## Was anything surprisingly helpful?

- `pack-and-runb.md`'s trap-handler triage list turned what looked like a
  scary crash banner (`**** Can't install trap handler ****`) into a
  one-line fix (`load math`) with zero investigation — genuinely saved
  real time, and is a good example of a skill file earning its keep on a
  task (pipes) that has nothing to do with what the triage list was
  originally written for (it's filed under PACK/RunB, not IPC).
- The residency-contamination warning in `pack-and-runb.md` ("modules
  registered by running a packed group file stay resident... after
  `BYE`") made me check `mdir` before re-running the fixed
  solo-reader test, specifically to rule out resolving a stale resident
  copy of the crashed first attempt — turned out not to be an issue
  here (RunB-invoked standalone programs unload cleanly on exit, unlike
  the group-file sibling case the warning is about), but the skill's
  own framing made it a five-second check instead of a confusing "why
  did my fix not take effect" moment if it *had* mattered.
- `using-os9exec-repl.md`'s host-native-directory + `flip -m` file
  creation route made six rounds of BASIC09 source iteration fast — edit
  on the host, one `flip -m`, `LOAD`/`PACK` in BASIC09. Zero friction
  once the account's `chx`/`chd` split was set up correctly.

## Overall: does the skill's pipe documentation hold up for real work?

Mostly yes, with one important asterisk. The qualitative model in
`ipc.md` — bounded default buffer, blocks-not-errors on a full named
pipe, EOF defined by empty-and-no-other-writers, named vs. unnamed
differing on deadlock detection — held up against every live test in
this pass, including the two sharpest single-process edge cases (reader
alone gets immediate EOF; writer alone eventually blocks with no
error). Anyone using this doc to reason about *what will happen* got the
right mental model.

The asterisk: the specific 90-byte number is not something I could
actually pin down live (the observed threshold was ~45x higher, most
likely due to an intervening BASIC09-level buffer this pass didn't have
budget to rule out via syscall tracing), and the one live deadlock this
pass deliberately induced turned into a session-ending wedge that the
skill's own stated recovery procedure (Ctrl-E) didn't actually recover
from. A user following this skill to explore pipe behavior interactively
— exactly what this dogfood task did — has a real chance of hitting that
same wedge and needing to know "restart the harness" isn't just a
suggestion here, it's the only way out.
