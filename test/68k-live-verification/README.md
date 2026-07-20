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
