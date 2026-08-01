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
  startup          <- shipped in docs/h0-template/, ours, edit freely
  CMDS/            <- shipped empty; this is the copy step below
  SYS/password     <- shipped, ours: the accounts the login tests use
  SYS/errmsg       <- from your OS-9 disk (Microware's; boot warns without it)
```

Copy `docs/h0-template/` to wherever you keep it, then fill `CMDS/`.

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

Three of those are trap handlers rather than utilities and are the ones most
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
