# A working RMA + RLINK multi-file build (6809, live NitrOS-9)

The first end-to-end `rma` → `rlink` → run demonstrated in this project
(2026-07-30). It closes the long-standing "`rma` hangs indefinitely" roadmap
item: `rma` does not hang, and the RLINK/PSECT multi-file path works.

`main.a` is the mainline PSECT and calls `helper`, which it never defines —
that is how RMA records an external reference. `sub.a` defines `helper` in a
separate file. Linked together they print `RMA-RLINK-OK`.

## The recipe that works

```
login                      (empty user name = user 0; see below)
chd /dd/RLTEST
copy /dd/CMDS/rma   /dd/RLTEST/rma
copy /dd/CMDS/rlink /dd/RLTEST/rlink
chx /dd/RLTEST
rma main.a -o=main.r
rma sub.a  -o=sub.r
rlink main.r sub.r -o=prog
attr prog e pe pr
prog                       ->  RMA-RLINK-OK
```

## The four things that made it look broken

Each of these produces silence or a misleading error, and together they are a
sufficient explanation for the old "it hangs / it does nothing" reports.

1. **`rma` writes no object file unless you pass `-o=`.** Without it the
   assembler runs, reports nothing, exits cleanly and leaves no output. A
   listing (`-l`) shows a clean assembly with object bytes on every line while
   still producing no ROF. This is the single biggest trap.
2. **Output goes to the execution directory (`chx`), not the data directory.**
   With `chx` on `/dd/CMDS`, an ordinary user cannot write there and the run
   silently produces nothing, anywhere.
3. **`/dd/CMDS/rma` has no public read** (`--e--ewr`, owner 0.0). An ordinary
   user can execute it by bare name but cannot load it by pathlist:
   `/dd/CMDS/rma <file>` prints **nothing at all**, while `copy` on the same
   file says `Error #214` outright. Copy the binaries somewhere you own, or be
   user 0.
4. **A label is local unless it carries a trailing colon.** `helper` exports
   nothing; `helper:` does. Without the colon `rma` assembles both files
   without complaint and `rlink` fails with `linker fatal: unresolved
   references` — which is a real, accurate diagnostic, and the first sign this
   toolchain was working correctly all along.

## What the toolchain actually is

`rma` identifies itself in listings as **Microware OS-9 RMA - V1.1** — the
genuine Relocating Macro Assembler. Note it prefixes its *error* messages with
`asm:` (`rma` with no argument prints `asm: no input file`), which does not
mean you ran `asm`. The listing marks external references with `=` in the
object-code column, exactly as the manual describes:

```
00004 0000=17fffd     start    lbsr  helper
```

`rma`, `rma_orig` and `rma.6809` are byte-identical (module `rma`, 20,143
bytes, CRC `$F83DD9`). `rma.6309` is a **different** module — name `r63`,
23,591 bytes, CRC `$E86F73` — not an alias. All of them plus `rlink` verify
CRC Good.

Files here are LF for host storage; OS-9 needs CR-only (`tr '\n' '\r'`).
