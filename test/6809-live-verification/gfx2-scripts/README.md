# GFX / GFX2 driver scripts (6809, live NitrOS-9)

Recovered 2026-07-29 from `/DD/CLAUDE/` on the NitrOS-9 EOU disk image, where
they had been left as loose scratch. They are the fixtures behind two reports
in the parent directory:

- `dogfood-report-gfx2-2026-07-18.md`
- `dogfood-gfx-pixels-2026-07-18.md`

Each file is a **BASIC09 session transcript**, not a `.bas` procedure: it is fed
to a running `basic09` over the REPL channel and drives the editor directly.

```
e <name>          enter the editor on a new procedure
 <BASIC09 lines>   (leading space -- the editor needs it)
q                 leave the editor
run <name>        run it
bye               exit basic09
```

So a script both *writes* its procedure and *runs* it in one pass; there is no
separate source file to install. Send one with the REPL's raw-key path, not the
prompt-gated `send` — `basic09`'s own prompts are not the shell's.

`RUN GFX(...)` / `RUN GFX2(...)` resolve against the `gfx` and `gfx2` modules in
`/DD/CMDS` on that disk. Those were assembled from locally-patched copies of
NitrOS-9's third-party `gfx.asm` / `gfx2.asm`; what the patches were and why
each was needed is `../dogfood-gfx2-install-notes.md`. The patched sources
themselves are not duplicated into this repo — they live on the disk in
`/DD/GFX2SRC` with a host-side copy beside the golden image.

Line endings here are LF (host convention). OS-9 wants CR-only, which is what
the REPL's send path already does — `tr '\n' '\r'` if you transfer one by
another route.

Several scripts are deliberately trivial harness probes rather than graphics
tests (`t1.s`, `h1.s`, `sh1.s`, `et.s`, `q1.s`, `sv1.s`): they check that the
editor-write-run-exit cycle itself survives the channel, which is the thing that
breaks first. Keep them — a graphics failure is only interpretable once that
cycle is known good.
