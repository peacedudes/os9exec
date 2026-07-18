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
