# 6809 live-verification tests

Source/command sequences behind `Live`-tagged claims in the `os9-dev` and
`os9-systems-dev` skills (`~/.claude/skills/`), run against real NitrOS-9
via `tools/nitros9repl.sh`. Each file records what was actually sent and
observed, so a claim can be re-checked later without re-deriving the test
from scratch — proof of the verification work, not just a convenience.

Naming: `<topic>.a` for 6809 assembly test programs, `<topic>.bas` for
BASIC09 procedures, `<topic>.txt` for plain REPL command/output
transcripts (no separate source file needed — e.g. testing shell syntax
directly). Each file's header comment/first line names the skill file and
claim it backs.

These are not automated regression tests (no runner replays them
automatically) — rerunning one means driving `nitros9repl.sh` by hand or
via an agent, same as when it was first written. Automating that is a
possible future step if this proves worth it, not done yet.

`gfx2-scripts/` is a subdirectory rather than a set of `.bas` files because
those are whole BASIC09 *session* transcripts, not procedures — see its own
README.

A batch of files here was recovered from the disk image itself on 2026-07-29,
when the accumulated scratch on `/DD` was swept: they had only ever existed on
the guest. Recovered under a descriptive name, with content unchanged apart
from CR→LF: `basic09-date-string.bas`, `basic09-printusing-b8.bas`,
`falarm-basic.a`, `falarm-cycle.a`, `process-descriptor-dump.a`,
`rl-create-update-mode.bas`, the five `dogfood-eoflock-*-{81byte,partial,4x}`
variants, and all of `gfx2-scripts/`. They carry no header comment naming the
claim they back, unlike everything else here — the comments were stripped when
they were typed into the guest over the REPL channel, and were never on the
disk to recover.
