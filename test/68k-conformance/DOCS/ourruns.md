# What CONF68K does on the systems we can reach

This records our own runs. It is the thing a run on real OS-9/68k hardware
gets compared against — and it is **not** a standard. Every verdict here is
what one emulator did; where it disagrees with `claims.md`, the manual is
right and the system has a defect.

`DOCS/expected` holds the same RESULT lines in machine-readable form, which
is what `tools/conformance.sh` checks a run against.

## os9exec, 2026-07-30

Built from branch `arm64-uae-integration`, macOS arm64 host, 68020/68881 CPU
configuration, system tick on (the default).

Run two ways, because the difference matters:

| Device under test | Result |
|---|---|
| host-native directory (`OS9H8=` a host folder) | 12 PASS, 0 FAIL, 0 SKIP, 0 ERROR |
| real RBF image (`mount -k=500k`, built inside the guest) | 12 PASS, 0 FAIL, 0 SKIP, 0 ERROR |

```
RESULT t01 PASS  obs=000216 exp=000216  I$Open of an absent pathlist reports E$PNNF
RESULT t02 PASS  obs=000211 exp=000211  I$Read of an exhausted file reports E$EOF
RESULT t03 PASS  obs=000203 exp=000203  I$Write on a read-mode path reports E$BMode
RESULT t04 PASS  obs=000000 exp=000000  I$Open with no mode bits set succeeds
RESULT t05 PASS  obs=000203 exp=000203  I$Read on a no-access-mode path reports E$BMode
RESULT t06 PASS  obs=000004 exp=000004  I$Read of more bytes than remain returns a short count
RESULT t07 PASS  obs=070005 exp=070005  write after a seek past EOF extends the file to seek+count
RESULT t08 PASS  obs=000211 exp=000211  a read at a position past EOF reports E$EOF
RESULT t09 PASS  obs=000201 exp=000201  I$Read on a closed path number reports E$BPNum
RESULT t10 PASS  obs=000218 exp=000218  I$Create of an existing name reports E$CEF
RESULT t11 PASS  obs=000065 exp=000065  I$Seek to zero rewinds the path
RESULT t12 PASS  obs=000300 exp=000300  SS_Size reports the number of bytes written
```

## t10 found a real defect on its first run

Worth recording in full, because it is the case the suite was built for and
it turned up immediately.

On its first run t10 reported:

```
RESULT t10 FAIL  obs=000000 exp=000218  I$Create of an existing name reports E$CEF
```

Creating a file whose name already existed returned no error at all. Run
against a real RBF image the same module reported `obs=000218` and passed.
So the defect was in os9exec's **host-native** path only — `pFopen` in
`fileaccess.c` deliberately reopened an existing file on create, with a
comment asserting that I$Create is "open-or-create" and that an existing
file is not an error.

The manual says otherwise: I$Create's own FUNCTION text states that an error
occurs if the pathlist specifies a name that already exists, and os9exec's
own RBF path already returned E$CEF. The host-native path was diverging from
both the specification and the emulator's own other half.

Visible consequence beyond this test: the shell's `>` redirect is documented
as create-only and fails when its target exists. On a host-native device it
was silently overwriting instead.

Fixed the same session; `make test` stayed at 183 passed / 0 failed, and
`make warnings` reported 0/0 on all four toolchains (host cc, linux gcc in
docker, mingw x86_64, mingw i686). Both device types now report 12 PASS.

**The lesson for anyone reading a run of this suite:** a host-native
directory is a shim with no real RBF underneath. A file-system claim can
pass there and fail on a real image, or the reverse. Run both when it
matters — `tools/conformance.sh 68k` and `tools/conformance.sh 68k --rbf`.

## What has NOT been checked

- **No run on real OS-9/68k hardware, or on any OS-9/68k other than
  os9exec.** That is the entire point of shipping the suite, and it is
  exactly the gap it exists to close. Everything above is one
  reimplementation agreeing with the manual, which is weaker evidence than
  it looks.
- **The rebuild path has not been exercised end-to-end by an independent
  assembler.** The modules in `CMDS/` were built by the `r68`/`l68` on our
  own system disk, and that is the only assembler they have met.
- **The two-process tests are only exercised on an RBF image.** t19–t27 need
  real record locking underneath, so they SKIP on a host-native directory —
  which is what CI and `--noshell` run. `tools/conformance.sh 68k --rbf` is
  the only thing that exercises `fork.i` at all, and it is a local gate.

## 2026-07-31 — t19 to t27, the record-locking tests

Nine two-process tests were added, and they found five defects on their first
run against an RBF image. Four are recorded in `DOCS/known-divergences` (t19,
t23, t25, t27); the fifth was found on the way and has no test, because it
kills the caller rather than reporting anything: `I$SetStt` with `SS_Ticks` on
a non-RBF path jumps through an uninitialised dispatch slot and bus-errors.
That one shaped the suite — `canlock` probes with `SS_Lock(0)` precisely to
avoid it.

Current results:

    host-native   18 PASS,  9 SKIP
    RBF image     23 PASS,  4 FAIL   (all four KNOWN)
    --noshell     18 PASS,  9 SKIP   (same as host-native)

The most useful single thing in that table is the PAIR t19/t21. Both make the
same demand of the same lock and differ only in whether the second process
reads or writes. t21 passes, reporting E$Lock exactly as documented; t19
reports 902 — its contender never came back at all. That is not a vague
"locking is broken": it says readers are resumed and writers are not, which is
a one-line answer to where to look. Neither test could have said it alone.

The suite's own numbers 901 and 902 appear in this run's obs= column. They are
not OS-9 error codes: 901 means the access succeeded only after the holder
released, and 902 means it never returned and was killed. See the readme.

**These are still os9exec results.** A FAIL here is os9exec disagreeing with
the manual, and nothing in this file is evidence about real OS-9/68k.
