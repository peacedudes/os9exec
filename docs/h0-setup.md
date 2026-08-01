# Building a system disk the tests can pass with

The suites in `test/` are all in git — 443 files. The **disk** they run against is
not, and cannot be: it is a licensed Microware OS-9/68k system disk. This is the
missing half of the setup, and without it a fresh clone can build `os9exec` and
run only the parts that need no disk at all.

`docs/h0-template/` is the part we can ship. Everything else is a copy step.

## What you need

Point `OS9DISK` at a directory (or an RBF image) laid out like this. Inside OS-9
that directory **is `/dd`** — that is what a boot disk is. `/h0` is only a
default alias os9exec adds for a directory sitting beside the binary; don't
build anything that depends on it.

```
<your disk>/
  startup             <- ours, edit freely
  SYS/password        <- ours: the accounts the login tests use
  CMDS/               <- shipped EMPTY; this is the copy step below
  CMDS/CLAUDE/        <- empty; the `claude` account's execution directory
  CMDS/DOG/           <- empty; the `dog` account's execution directory
  USR/CLAUDE/.login   <- ours
  USR/DOG/.login      <- ours
```

Copy `docs/h0-template/` to wherever you keep it, then fill `CMDS/`.

The account directories are not decoration: the suite runs `login claude` 20
times and `login dog` 10 times, and `login` fails outright with `E$PNNF` if the
home directory named in `SYS/password` does not exist. Each `.login` sets `chd`,
`chx`, `PATH` and `TERM` and ends with an `echo`, which is what the REPL's
prompt detection watches for. Verified: a disk built from this template plus the
copy step below boots, logs in as `dog`, and runs `procs`.

## Why so little of SYS/

Measured the same way as the command list — only **`SYS/password`** is referenced
by anything in the suite. Nothing reads `termcap`, `motd`, `MENUS`,
`moded.fields` or `umacs.hlp`; those are data files for utilities the tests do
not run, and most are Microware's or third-party anyway.

**`SYS/errmsg` deserves its own warning.** `h0/SYS/errmsg` is **Microware's
copyrighted file** — the error-code numbers and `E$` names are facts, but the
message prose is expression. Do not copy it into anything shippable. Nothing in
the suite needs it; its absence only means the shell prints bare numbers instead
of sentences. A clean-room replacement written from our own `error-codes.md`
exists at `freeware/SYS/errmsg` (not in this repo). If you want readable errors
on your own disk, copy Microware's from your own OS-9 disk — that is yours to
use, just not to redistribute.

## Exactly which commands to copy

**Measured, not guessed.** `make test` was run with module tracing on
(`os9exec -d 0x0020`, the `dbgModules` mask) and every module the suite looked
for was collected. It searches for **62** modules that exist on our disk:

```
attr    binex   build   cfp     chown   cio     cmp     compress copy
count   csl     cudo    date    dcheck  deiniz  del     deldir  devs
dir     dsave   dump    echo    editmod events  exbin   expand  fixmod
free    grep    help    ident   iniz    l68     list    load    login
makdir  math    mdir    merge   mfree   mkdatmod os9gen padrom  paths
pd      pr      printenv procs  qsort   r68     save    shell   sleep
tar     tee     tmode   touch   tr      tsmon   unlink  what
```

### Where they come from

These live under `OS9/<cpu>/CMDS` on a Microware **OS-9 for 68K SDK** — v1.2 is
the one this project was developed against. Our disk was built by copying
**all of `OS9/68000/CMDS`**
and then **all of `OS9/68020/CMDS` over the top** — the 68020 build replaces
same-named modules and adds none, so the file *set* is the 68000 one and the
*contents* are 68020 where a 68020 build exists. Either layer alone should work
under os9exec, which emulates a 68020/68881; the doubled copy is simply what we
have measured against.

`CPU32/CMDS`, `68040/CMDS` and `68060/CMDS` exist on the SDK too and were not
used here.

The SDK itself is Microware's and is not ours to point you at — but the
**manuals** this project cites throughout are separately archived in public,
and are what every claim in `test/68k-conformance/DOCS/claims.md` is checked
against:

- [OS-9/68K V2.4 Technical Manual](https://www.peripheraltech.com/OS9%20-%2068K%20V2.4%20Technical%20Manual.pdf)
- [OS-9 Operating System User Manual](http://www.icdia.co.uk/microware/77165102.pdf)
- [OS-9 for 68K OEM Installation Manual](https://archive.org/details/os968kOem_v9.9)

Three of the 62 are trap handlers rather than utilities and are the ones most
often missing: **`cio`**, **`csl`**, **`math`**. Without `cio` most archived
OS-9 binaries die with `**** Can't install trap handler ****`; the suite needs
it. `math881` is a drop-in replacement for `math` on a 68881 build and is more
accurate — see the note in `startup`.

To reproduce the measurement after changing the suite:

```sh
OS9DISK=<disk> ./os9exec -d 0x0020 shell < commands >log 2>&1
grep -o "searching module '[a-z0-9_]*'" log | sort -u
```

## The other 156

Our own working `h0/CMDS` holds **218** entries. The suite touches 62 of them.
The rest accumulated. If you are building a disk from scratch, you do not need
them, and starting from the list above rather than from a copy of somebody's
whole `CMDS` is the difference between a disk you can reason about and one you
cannot.

The same goes for the disk root: a long-lived `h0` collects scratch
(`seed.tmp`, `probe`, `quicktest`, half-finished `.bas` files). None of it is
required. The template has none of it.

## What the REPL needs

`tools/os9repl.sh` needs **a shell and nothing else**. It boots
`shell /dd/startup`, and `startup`'s only job is to `load` the modules you want
resident so that bare-name `F$Fork` lookups resolve regardless of any account's
execution directory. If you would rather not keep a `startup` at all, load what
you need by hand once the REPL is up — the harness does not care.

It does **not** require a disk structured specially for it. If your disk boots
`shell`, the REPL works.

## Running without any disk

Two things need nothing at all:

- `tools/conformance.sh 68k --noshell` — CONF68K runs each test as its own boot
  program. Every module in `test/68k-conformance/CMDS/` is our own 68000
  assembly. This is what CI runs.
- `make warnings` — compiles only.

`make test`, `make hammer`, `make live-verify` and the REPL all need the disk
above.
