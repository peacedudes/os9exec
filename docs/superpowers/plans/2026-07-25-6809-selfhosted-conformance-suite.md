# Self-hosted 6809 conformance suite — Phase 1 implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a 360K RBF disk image that any OS-9/6809 system can mount and run in three commands, reporting per test whether the system behaves as Microware's documentation says it does.

**Architecture:** Sources live in `test/6809-conformance/`. A host-side build cross-assembles them with `lwasm`, converts every text file to CR-only, and drives `os9exec` non-interactively with a procedure file to create the image, stamp non-super ownership, and set attributes. ToolShed verifies the result host-side; a live NitrOS-9 run over DriveWire verifies it for real.

**Tech Stack:** `lwasm`/`lwlink` (Homebrew), ToolShed `os9` (`/usr/local/bin/os9`), `os9exec` (this repo), `tools/nitros9repl.sh` + `drivewire-cli`, BASIC09 on the guest for packing.

**Spec:** `docs/superpowers/specs/2026-07-25-6809-selfhosted-conformance-suite-design.md`

## Global Constraints

- **Oracle policy.** Microware documentation is the standard. os9exec and NitrOS-9 are candidates for defect, never authorities. **No test ships without a citation** — document and section — recorded in `DOCS/claims`. A claim that cannot be pinned is dropped, not guessed.
- **No verbatim Microware text on the image.** Cite document and section; state expectations in our own prose.
- **No non-ASCII bytes anywhere on the image.**
- **Nothing outside the suite disk is ever written.** No test touches `/dd`.
- **Nothing runs privileged.** A test needing super reports `SKIP` with the reason.
- **Directory names are capitalized, file names are not** (OS-9 convention).
- **Every text file on the image is CR-only (0x0D).** A LF-terminated procedure file is *one line* to OS-9: the shell echoes the whole file and runs nothing, with no error. This is silent and it will waste a day if it reaches someone else's machine.
- **Attributes and ownership are set on the RBF side**, never host-side. A host directory has nowhere to store an owner ID.
- **Never `pkill -f os9exec`** — other sessions match that pattern. Match a path.
- Verified facts this plan rests on (do not re-derive): `lwasm --format=os9` produces a valid 6809 program module (Good CRC via ToolShed `ident`); `os9exec shell /h1/<proc>` drives a build non-interactively; `login <acct>` inside that procedure stamps subsequent files with that account's ID (`dir -e` shows `1.7` after `login claude`, `0.0` before); ToolShed `copy -o=` can only set the low byte, so it **cannot** produce a non-zero group and is not an ownership route.

---

## File Structure

**Suite sources — the thing that ships (new):**

**`SRC/` contains only tests that ship.** The build stages `SRC/*.a` and `SRC/*.bas` with a wildcard, so anything left there reaches the recipient's machine. Development scaffolding — smoke modules, buffer probes, anything that emits a `RESULT` line without asserting a documented OS-9 behaviour — lives in `test/6809-conformance/dev/` and is never staged. A smoke module that shipped would add a fabricated `PASS` to a report whose only value is truthful accounting, and would consume a test number reserved for a real claim.

- `test/6809-conformance/SRC/report.i` — shared assembly include: the result-line emitter. One responsibility: turn a verdict plus two numbers into a `RESULT` line on stdout.
- `test/6809-conformance/SRC/t01open.a` … `t0Nxxx.a` — one file per assembly test.
- `test/6809-conformance/SRC/t0Nxxx.bas` — one file per BASIC09 test.
- `test/6809-conformance/DOCS/claims.md` — master claims table (ID, citation, expected behaviour). Converted to `DOCS/claims` on the image.
- `test/6809-conformance/DOCS/ourruns.md` — our NitrOS-9 baseline, labelled candidate.
- `test/6809-conformance/text/readme`, `text/runall`, `text/runone`, `text/rebuild` — LF masters of the on-image text files. Converted to CR at build.

**Build and verification tooling (new):**

- `tools/selfhost6809/build-image.sh` — the whole build: assemble, stage, drive os9exec, set attributes. One responsibility: produce `conf6809.dsk`.
- `tools/selfhost6809/mkimage.proc` — generated os9exec procedure file (CR-only). Not committed; regenerated each build.
- `tools/selfhost6809/verify-image.sh` — host-side assertions against a built image using ToolShed. This is the failing test that drives Task 1.
- `tools/selfhost6809/run-on-nitros9.sh` — serve the image over DriveWire, run the suite as a non-super user, retrieve the report.

**Modified:**

- `GNUmakefile` — add the `selfhost-6809` target.

---

### Task 1: Image skeleton with non-super ownership

Produces an empty but correctly-shaped image. Nothing runs yet; this task exists because ownership and attributes are the part most likely to be silently wrong, and everything later sits on top of them.

**Files:**
- Create: `tools/selfhost6809/verify-image.sh`
- Create: `tools/selfhost6809/build-image.sh`
- Modify: `GNUmakefile`

**Interfaces:**
- Consumes: nothing.
- Produces: `build-image.sh` writes `build/selfhost6809/conf6809.dsk`. `verify-image.sh <image>` exits 0 if the image satisfies every structural assertion, non-zero otherwise, printing one line per failed assertion.

- [ ] **Step 1: Write the failing test**

Create `tools/selfhost6809/verify-image.sh`:

```bash
#!/usr/bin/env bash
# verify-image.sh <image> -- host-side structural assertions on a built
# conformance image, using ToolShed as an oracle independent of the
# emulator that built it.
set -uo pipefail
IMG="${1:?usage: verify-image.sh <image>}"
OS9=${OS9:-/usr/local/bin/os9}
fails=0
fail() { printf 'ASSERT-FAIL %s\n' "$*"; fails=$((fails+1)); }

[ -f "$IMG" ] || { printf 'ASSERT-FAIL image %s does not exist\n' "$IMG"; exit 1; }

listing=$("$OS9" dir -e "$IMG," 2>&1)

# Every required directory is present.
for d in CMDS SRC DOCS SCRATCH RESULTS REBUILT; do
    printf '%s\n' "$listing" | grep -aq "[[:space:]]$d\$" || fail "missing directory $d"
done

# Nothing on the image is owned by group 0. Microware defines the super user
# as ANY user in group zero, so a group-0 owner would ship privileged files.
if printf '%s\n' "$listing" | grep -aqE '^[[:space:]]*0\.'; then
    fail "found group-0 (super user) ownership in root listing"
fi

## Assertions added in Task 4, once the files they check exist.
## Until then this script must not assert them: a check for something no task
## has created yet fails for the wrong reason and teaches you to ignore it.

if [ "$fails" -eq 0 ]; then printf 'VERIFY-OK %s\n' "$IMG"; exit 0; fi
printf 'VERIFY-FAILED %d assertion(s)\n' "$fails"; exit 1
```

Make it executable: `chmod +x tools/selfhost6809/verify-image.sh`

- [ ] **Step 2: Run it to verify it fails**

Run: `./tools/selfhost6809/verify-image.sh build/selfhost6809/conf6809.dsk`
Expected: `ASSERT-FAIL image build/selfhost6809/conf6809.dsk does not exist`, exit 1.

- [ ] **Step 3: Write the build script**

Create `tools/selfhost6809/build-image.sh`. The os9exec procedure it generates **must be CR-only** and must `login` before creating anything that ships:

```bash
#!/usr/bin/env bash
# build-image.sh -- build the self-hosted 6809 conformance image.
set -euo pipefail
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="$REPO/build/selfhost6809"
STAGE="$OUT/stage"          # host-native tree os9exec sees as /h1
IMGDIR="$OUT/dev"           # os9exec startPath: the image appears here as h7
ACCT=${CONF_ACCT:-claude}   # non-super account in h0/SYS/password (1.7)

rm -rf "$OUT"; mkdir -p "$STAGE" "$IMGDIR"
ln -s "$REPO/h0" "$IMGDIR/h0"
ln -s "$STAGE"   "$IMGDIR/h1"

# --- the os9exec build procedure, CR-only (LF would make it ONE line) ---
{
  echo "mount -k=360k h7"
  echo "login $ACCT"
  for d in CMDS SRC DOCS SCRATCH RESULTS REBUILT; do echo "makdir /h7/$d"; done
  echo "echo BUILD-STRUCTURE-DONE"
  echo "dir -e /h7"
} | tr '\n' '\r' > "$STAGE/mkimg"

cd "$IMGDIR"
OS9DISK="$IMGDIR/h0" OS9STOP=1 "$REPO/os9exec" shell /h1/mkimg </dev/null 2>&1 \
  | grep -av '^#' | tee "$OUT/build.log"

grep -aq BUILD-STRUCTURE-DONE "$OUT/build.log" \
  || { echo "build: os9exec procedure did not complete" >&2; exit 1; }

mv "$IMGDIR/h7" "$OUT/conf6809.dsk"
printf 'built %s\n' "$OUT/conf6809.dsk"
```

Make it executable: `chmod +x tools/selfhost6809/build-image.sh`

- [ ] **Step 4: Add the make target**

In `GNUmakefile`, add:

```make
selfhost-6809:
	./tools/selfhost6809/build-image.sh
	./tools/selfhost6809/verify-image.sh build/selfhost6809/conf6809.dsk
```

- [ ] **Step 5: Run the build and the assertions**

Run: `make selfhost-6809`
Expected: the build log shows `BUILD-STRUCTURE-DONE`, then `VERIFY-OK`.

If ownership assertions fail, check the `dir -e` output in `build.log`: files created **before** `login` are `0.0`. The `login` line must precede every `makdir`.

- [ ] **Step 6: Prove the assertions can fail**

Build a scratch image with the `login` line omitted from the generated procedure. Everything is then created as `0.0`, and `verify-image.sh` must report `found group-0 (super user) ownership`. Rebuild normally afterwards and confirm the good `1.7` image is restored.

Do **not** try to do this by pointing `ACCT` at the `su` account: its password field in `h0/SYS/password` is not empty, so `login su` inside a procedure file stops for a password prompt and consumes the following line as the answer. Omitting the login is the equivalent falsification and it works non-interactively.

A check that has never failed is not evidence — three checks in this codebase turned out to be incapable of failing.

- [ ] **Step 7: Commit**

```bash
git add tools/selfhost6809/ GNUmakefile
git commit -m "Core: build a 6809 conformance image skeleton with non-super ownership"
```

---

### Task 2: The result-line emitter

Every test's output goes through one routine, so the report format is defined in exactly one place.

**Files:**
- Create: `test/6809-conformance/SRC/report.i`

**Interfaces:**
- Produces: `emit` — call with `X` pointing at the assembled line in `OUTBUF`, having been advanced past the last character. Writes the line plus a CR to path 1 and returns. Also `copys` (copy `B` bytes from `Y` to `X`) and `prdec` (append `VALUE,u` as decimal at `X`).
- Data-area offsets `OUTBUF=0`, `VALUE=128`, `DIGIT=130`, `ERRB=131`, `PATHNUM=132` are fixed here and used unchanged by every test. All fit the module's 256-byte data area (`$0100`).

**The buffer is 128 bytes and that number is load-bearing.** A result line is written into `OUTBUF` at offset 0; the first scalar sits immediately after it. With five-digit numeric fields, `RESULT tNN VERDICT  obs=00000 exp=00000  ` costs 39 bytes before the description begins, and the spec's own example lines run to 73 characters. If the buffer were 60 bytes — the value the corpus's short-line helpers used — a normal-length description would silently overwrite `VALUE`, then `DIGIT`, `ERRB` and `PATHNUM`. A corrupted `PATHNUM` means a botched `I$Close` after the line is emitted, on the recipient's machine, long after anyone is watching. Any later change to these offsets must keep `OUTBUF` at least as long as the longest line the format can produce.

- [ ] **Step 1: Write the include**

Lift `copys`, `crwrite`, `prdec` and `dectbl` from `test/6809-live-verification/syscall-iseek.a:126-160` — our own code, already proven on the guest. Two deliberate changes from the original:

1. `crwrite` hardcodes `ldy #60`, writing the full buffer including trailing junk. Replace with a computed length so the report is cleanly parseable:

```
emit    lda #$0D
        sta ,x+              terminate the line with CR
        tfr x,d              D = end address
        leax OUTBUF,u        X = start of buffer (I$WritLn wants it here)
        pshs x               push start
        subd ,s++            D = end - start = byte count
        tfr d,y              Y = byte count
        lda #1               path 1 = standard output
        swi2
        fcb $8C              I$WritLn
        rts
```

Verify the length on the guest in Step 3 rather than trusting it — a wrong count silently truncates or over-runs every report line in the suite.

2. `prdec` emits five digits with leading zeros (`00216`). Keep it — fixed-width fields make the report easier to diff. It is recorded in the spec's test contract; the `DOCS/claims` file that also states it is created in Task 3, so there is nothing to write here.

- [ ] **Step 2: Assemble a smoke module that uses it**

Create `dev/smoke.a` — in `dev/`, **not** `SRC/`, because it asserts nothing about OS-9 and must never ship. It emits one known line:

```
        nam DevSmoke
        mod eom,nm,$11,$81,start,$0100
nm      fcs /DevSmoke/
        use report.i
start   equ *
        leax OUTBUF,u
        leay MSG,pcr
        ldb #MSGL
        lbsr copys
        lbsr emit
        clrb
        swi2
        fcb $06
MSG     fcc /SMOKE emitter ok obs=00001 exp=00001/
MSGL    equ *-MSG
        emod
eom     equ *
```

- [ ] **Step 3: Run it and verify the output is exact**

Run: `lwasm --format=os9 -I test/6809-conformance/SRC --output=/tmp/smoke test/6809-conformance/dev/smoke.a`
Then `/usr/local/bin/os9 ident /tmp/smoke` — expected: `Good` CRC.

Copy it onto a scratch image, serve it to NitrOS-9 via `tools/nitros9repl.sh`, and run it.
Expected, byte for byte, with no trailing padding:
`SMOKE emitter ok obs=00001 exp=00001`

If the line is padded or truncated, the length arithmetic in `emit` is wrong. Fix it here — every later task depends on it.

- [ ] **Step 4: Commit**

```bash
git add test/6809-conformance/SRC/report.i test/6809-conformance/dev/
git commit -m "Tests: shared result-line emitter for the 6809 conformance suite"
```

---

### Task 3: First conformance test — `I$Open` of an absent pathlist

**Files:**
- Create: `test/6809-conformance/SRC/t01open.a`
- Create: `test/6809-conformance/DOCS/claims.md`
- Modify: `tools/selfhost6809/build-image.sh`

**Interfaces:**
- Consumes: `report.i` (`copys`, `prdec`, `emit`) and its data-area offsets from Task 2.
- Produces: a module `t01open` in `CMDS/` emitting exactly one `RESULT t01` line.

**Claim:** opening a pathlist that does not exist reports `E$PNNF` (216). Before writing the test, find and record the citation — `references/common/error-codes.md` gives the code, but `DOCS/claims` must name the Microware document and section that specifies the behaviour. **If no citation can be pinned, drop this test and pick another.**

- [ ] **Step 1: Write the test**

```
        nam ConfT01
        mod eom,nm,$11,$81,start,$0100
nm      fcs /ConfT01/
        use report.i
EXPECT  equ 216
start   equ *
        leax fname,pcr
        lda #1               read access
        swi2
        fcb $84              I$Open
        bcc opened           no error at all -- that is a FAIL
        stb ERRB,u
        cmpb #EXPECT
        beq pass
        bra fail
opened  equ *
        clr ERRB,u           open succeeded: observed "error" is 0
fail    equ *
        leax OUTBUF,u
        leay FAILM,pcr
        ldb #FAILML
        lbsr copys
        bra tail
pass    equ *
        leax OUTBUF,u
        leay PASSM,pcr
        ldb #PASSML
        lbsr copys
tail    equ *
        clra
        ldb ERRB,u
        std VALUE,u
        lbsr prdec
        leay EXPM,pcr
        ldb #EXPML
        lbsr copys
        ldd #EXPECT
        std VALUE,u
        lbsr prdec
        leay DESC,pcr
        ldb #DESCL
        lbsr copys
        lbsr emit
        clrb
        swi2
        fcb $06
fname   fcs "SCRATCH/nosuchfile"
PASSM   fcc /RESULT t01 PASS  obs=/
PASSML  equ *-PASSM
FAILM   fcc /RESULT t01 FAIL  obs=/
FAILML  equ *-FAILM
EXPM    fcc / exp=/
EXPML   equ *-EXPM
DESC    fcc /  I$Open of an absent pathlist reports E$PNNF/
DESCL   equ *-DESC
        emod
eom     equ *
```

Two assembler details, both verified — do not "fix" them into the more familiar form:

- **`use` is lwasm's include directive**, and `-I test/6809-conformance/SRC` puts `report.i` on its search path.
- **`fcs "…"` with double quotes, not `fcs /…/`.** The conventional `/` delimiter terminates at the first `/` *inside* the string, so any pathlist silently truncates to its first component. Verified: the quoted form emits `SCRATCH/nosuchfil` + `e5`, the correct high-bit-terminated string.

- [ ] **Step 2: Assemble and verify the module**

Run: `lwasm --format=os9 -I test/6809-conformance/SRC --output=/tmp/t01open test/6809-conformance/SRC/t01open.a`
Then: `/usr/local/bin/os9 ident /tmp/t01open`
Expected: `Good` CRC, `Prog mod, 6809 Obj`.

- [ ] **Step 3: Record the claim**

Create `test/6809-conformance/DOCS/claims.md` with the table header and this row: test ID, the Microware document and section, and the expected behaviour in our own prose. No verbatim manual text.

- [ ] **Step 4: Teach the build to assemble and install tests**

In `build-image.sh`, before the os9exec step, assemble every `SRC/*.a` into `$STAGE/CMDS/`, and copy sources into `$STAGE/SRC/` with CR conversion. Then extend the generated procedure to copy them onto the image and set attributes — under the non-super account, after `login`:

```bash
  for m in "$STAGE"/CMDS/*; do
    b=$(basename "$m")
    echo "copy /h1/CMDS/$b /h7/CMDS/$b"
    echo "attr /h7/CMDS/$b -e -pe -pr"    # 68k spelling: -e SETS, -ne clears
  done
```

Note the inversion: on 68k `-e` sets and `-ne` clears; on 6809 it is the other way round. The build runs under os9exec, so 68k spelling applies.

- [ ] **Step 4b: Make a failed install detectable**

**The OS-9 shell does not abort a procedure when a command fails.** A `copy` whose source is missing prints `Error #000:216` and the procedure runs on to completion, leaving the destination empty — verified directly. So the build's `BUILD-STRUCTURE-DONE` marker proves only that the procedure *ran*, never that it *worked*, and a test that silently failed to install is invisible: its line is simply absent from the report, which reads as a shorter suite rather than as a fault.

Close it in two places:

1. `build-image.sh` writes a manifest as it stages — one line per module, `<name> <bytes>` — to `build/selfhost6809/manifest.txt`.
2. `verify-image.sh` reads the manifest and asserts, for every entry, that the module exists in `CMDS/` on the image with a matching byte count, and that `build.log` contains no line matching `Error #`.

Prove both can fail before believing them: delete one staged module after the copy loop and confirm the manifest check reports it; inject a line containing `Error #` into a copy of the log and confirm that check reports it too.

- [ ] **Step 5: Build and verify**

Run: `make selfhost-6809`
Expected: `VERIFY-OK`. Then confirm with ToolShed that the module carries public read and execute:
`/usr/local/bin/os9 dir -e build/selfhost6809/conf6809.dsk,/CMDS`

- [ ] **Step 6: Run it on NitrOS-9 and confirm the verdict**

Serve the image and run `t01open` as a non-super user.
Expected: `RESULT t01 PASS  obs=00216 exp=00216  I$Open of an absent pathlist reports E$PNNF`

- [ ] **Step 7: Prove it can report FAIL**

Change `EXPECT` to `215`, rebuild, rerun. Expected: `RESULT t01 FAIL  obs=00216 exp=00215`. Revert. **Do not skip this** — a suite that cannot go red is not evidence.

- [ ] **Step 8: Commit**

```bash
git add test/6809-conformance/ tools/selfhost6809/build-image.sh
git commit -m "Tests: first 6809 conformance test -- I\$Open of an absent pathlist"
```

---

### Task 4: The runner and the report

**Files:**
- Create: `test/6809-conformance/text/runall`, `text/runone`, `text/readme`
- Create: `test/6809-conformance/SRC/tally.bas`
- Modify: `tools/selfhost6809/build-image.sh`

**Interfaces:**
- Consumes: modules in `CMDS/` emitting `RESULT` lines.
- Produces: `RESULTS/report` on the image; `tally` prints counts per verdict.

- [ ] **Step 1: Write the runner procedure**

`text/runall` (LF master; converted to CR at build). It must record the running user so results are attributable, and must write only inside the suite disk:

```
echo CONF6809 run starting
echo RUN prebuilt >+/x0/RESULTS/report
procs >+/x0/RESULTS/report
t01open >+/x0/RESULTS/report
tally
```

**Every line that belongs in the report must be redirected into it.** `procs` records who ran the suite; unredirected, it prints to the terminal and the report carries no attribution at all — while a readme claiming otherwise makes the file look attributable when it is not.

**The append form is `>+`, and this is documented, not a thing to discover.** On OS-9 `>` redirects stdout but *fails if the file already exists*; `>>` redirects **stderr**, not append; `>+` appends to an existing file or creates it; `>-` truncates or creates. The Unix reflex that `>>` appends is the trap — using it here would send each test's stderr to the report while the `RESULT` line went to the terminal, producing an empty report that looks like a suite that ran. Source: the skill's `common/os9-tools-and-shell.md` redirection table and `common/unix-differences.md`, both `Live`-tagged on 68k and NitrOS-9.

`procs` prints the `Grp.Usr` of the running process — that is how the report records who ran it.

- [ ] **Step 2: Confirm the documented append behaviour on the guest**

Run a two-line procedure on NitrOS-9 that writes twice to the same path with `>+`, then `list` it. Expected: both lines present, in order. This confirms the documented behaviour on this guest; it is not an investigation, and if it disagrees with the documentation that disagreement is itself the finding — report it rather than quietly switching forms.

Also confirm the negative, so nobody re-derives it later: the same procedure using `>>` puts nothing on stdout's path, because `>>` is stderr.

- [ ] **Step 3: Write the tally program — and establish the BASIC09 pack pipeline here**

`SRC/tally.bas` opens `/x0/RESULTS/report`, counts lines containing each verdict, and prints:

```
CONF6809 totals: PASS=n FAIL=n SKIP=n ERROR=n
```

This is the suite's **first BASIC09 artifact**, so the pack pipeline is built here rather than in Task 5. There is no host-side BASIC09: packing requires a booted guest, via `tools/nitros9repl.sh` and `tools/b09run.sh`. Two known traps, both of which have cost time before:

- **`PACK` writes to the execution directory (CHX), not the data directory.** If the output "didn't appear", look in CHX.
- **A second `PACK` of the same procedure in one session fails with error 51.** Packing converts the in-workspace copy too, so there is no source structure left for the extra compiler pass. Pack once per fresh session.

Commit the packed module alongside its source: the step needs a booted guest, so a build must not depend on XRoar being available whenever a module has not changed.

Extend `build-image.sh` to stage `SRC/*.bas` sources and their packed modules the same way it stages assembly tests, including the manifest entry and byte-count check.

- [ ] **Step 4: Run the whole thing end to end**

Expected: `RESULTS/report` contains the `t01` line and the totals line reports `PASS=1`.

- [ ] **Step 5: Write the `rebuild` procedure**

`text/rebuild` reassembles `SRC/*.a` with the host system's own `asm` and repacks the BASIC09 sources with its `basic09`, **writing the results into `REBUILT/`, never into `CMDS/`**, then runs the suite from `REBUILT/`.

Three requirements, each with a failure it prevents:

- **`CMDS/` is never modified.** Overwriting it destroys the prebuilt-versus-rebuilt comparison the first time anyone uses the feature, and a recipient whose `asm` or `basic09` is present but incompatible — the case most worth hearing about — would be left with neither a working prebuilt module nor a working rebuilt one, from one irreversible command. Note that a tool-presence guard does **not** catch this: the tools exist, they simply produce something different.
- **Guard on tool presence anyway**: if `asm` or `basic09` is absent, print one line saying so and exit without creating anything.
- **Write a boundary line into the report** before the run, naming which artefacts produced what follows — `RUN prebuilt` or `RUN rebuilt`. The report is append-only, so without a marker two runs merge into one indistinguishable list.

The readme must tell the recipient to send the whole report, both runs included.

- [ ] **Step 6: Prove SKIP works**

Add a temporary test whose prerequisite is deliberately absent; confirm it emits `SKIP` with a stated reason and that the tally counts it as `SKIP`, **not** `FAIL`. Remove it afterwards.

- [ ] **Step 7: Extend `verify-image.sh` now that the files exist**

Task 1 deliberately left these out. Add to `tools/selfhost6809/verify-image.sh`:

```bash
# Every required top-level file is present, lowercase per OS-9 convention.
for f in readme runall runone rebuild; do
    printf '%s\n' "$listing" | grep -aq "[[:space:]]$f\$" || fail "missing file $f"
done

# runone and runall must invoke the same set of tests. The shell has no
# positional-parameter substitution, so runone is hand-maintained in lockstep
# with runall; without this check a test added to one and forgotten in the
# other drifts silently forever, with no failure signal anywhere.
ra=$(grep -aoE '^t[0-9]+[a-z]*' "$SRCDIR/text/runall" | sort)
ro=$(grep -aoE '^t[0-9]+[a-z]*' "$SRCDIR/text/runone" | sort)
[ "$ra" = "$ro" ] || fail "runall and runone invoke different tests"

# Public read, or the disk is unreadable to anyone but our build account.
for f in readme runall runone rebuild; do
    printf '%s\n' "$listing" | grep -a "[[:space:]]$f\$" | grep -aq '\-\-\-\-r' \
        || fail "$f lacks public read"
done

# Text files carry no non-ASCII bytes and no LF. Extract and check the real
# bytes: assembled modules legitimately contain high bytes, so this is
# per-file, never a scan of the whole image.
tmp=$(mktemp -d)
for f in readme runall runone rebuild; do
    if ! "$OS9" copy "$IMG,/$f" "$tmp/$f" >/dev/null 2>&1; then
        fail "cannot extract $f"; continue
    fi
    # grep exits 0 = found, 1 = not found, >=2 = error. Treat an error as a
    # failed assertion: `grep ... && fail` alone silently passes on exit 2,
    # which is a check that cannot fail.
    LC_ALL=C grep -q $'[\x80-\xff]' "$tmp/$f"
    case $? in 0) fail "$f contains non-ASCII bytes";; 1) ;; *) fail "cannot scan $f";; esac
    LC_ALL=C grep -q $'\n' "$tmp/$f"
    case $? in 0) fail "$f contains LF -- must be CR-only";; 1) ;; *) fail "cannot scan $f";; esac
done
rm -rf "$tmp"
```

Prove each new assertion can fail before believing it: stage a copy of the image with an LF-terminated `readme` and confirm the CR check fires.

- [ ] **Step 8: Commit**

```bash
git add test/6809-conformance/ tools/selfhost6809/verify-image.sh
git commit -m "Tests: guest-side runner, report and tally for the 6809 suite"
```

---

### Task 5: BASIC09 test and the pack pipeline

**Files:**
- Create: `test/6809-conformance/SRC/t02int.bas`
- Modify: `tools/selfhost6809/build-image.sh`

**Interfaces:**
- Consumes: nothing from earlier tasks (BASIC09 tests emit their own `RESULT` line with `PRINT`).
- Produces: a packed module `t02int` in `CMDS/`, plus its source in `SRC/`.

**Claim:** BASIC09 `INTEGER` on the 6809 is 16-bit, and its documented overflow behaviour. Citation required in `DOCS/claims` before the test ships.

- [ ] **Step 1: Write the procedure**

It must print exactly one line in the standard format, e.g.
`PRINT "RESULT t02 PASS  obs="; obs; " exp="; exp; "  INTEGER is 16-bit on 6809"`
Match the field layout of the assembly tests so one parser handles both.

- [ ] **Step 2: Pack it using the pipeline established in Task 4**

Task 4 built the BASIC09 pack path for `tally.bas`; follow it. The same two traps apply: `PACK` writes to the **execution** directory, not the data directory, and a second `PACK` of the same procedure in one session fails with error 51 because packing converts the in-workspace copy too. Pack once per fresh session.

- [ ] **Step 3: Commit the packed module alongside the source**

The pack step needs a booted guest, so the built module is committed — otherwise the build is unreproducible whenever XRoar is unavailable.

- [ ] **Step 4: Run and verify**

Expected: `RESULT t02 …` appears in the report and the tally increments.

- [ ] **Step 5: Commit**

```bash
git add test/6809-conformance/
git commit -m "Tests: first BASIC09 conformance test and its pack pipeline"
```

---

### Task 6: Remaining slice tests

Each test starts as a copy of `SRC/t01open.a` (assembly) or `SRC/t02int.bas` (BASIC09). The structure — `EXPECT` constant, pass/fail branch, `copys`/`prdec`/`emit` tail, message table — is unchanged. What differs per test is listed exactly below.

**Files:** one new `SRC/` file per test, plus a row in `DOCS/claims.md`.

For every test, in this order: pin the citation (drop the test if it cannot be pinned), write it, assemble and check the CRC with `os9 ident`, run it on the guest, **perturb `EXPECT` and confirm the report says `FAIL`**, revert, rerun the whole suite, confirm the tally count grew by one, commit.

- [ ] **Step 1: `t03eof.a`** — `I$Read` at end of file reports `E$EOF` (211).
  Changes from `t01open.a`: `EXPECT equ 211`; the body creates a fixture in `SCRATCH/` with `I$Create` ($83), writes a known short record with `I$Write` ($8A), closes ($8F), reopens for read, reads the record successfully, then reads again — the second read is the one under test. Delete the fixture before exiting so reruns are clean.

- [ ] **Step 2: `t04bmode.a`** — writing to a path opened read-only reports `E$BMode` (203).
  Changes: `EXPECT equ 203`; open an existing `SCRATCH/` file with `A=1` (read), then attempt `I$Write`. Note the console exemption recorded in `references/common/error-codes.md` — this test must use a **disk file**, never a console path, or it will pass vacuously.

- [ ] **Step 3: `t05fna.a`** — opening without adequate permission reports `E$FNA` (214).
  Changes: `EXPECT equ 214`; the target is a file placed on the image at build time with public read cleared (`attr /h7/SCRATCH/denied -npr` under os9exec's 68k spelling). Requires a build-script addition to create and restrict that file.
  **Guard first:** read the running process's group with `F$ID` ($0C). If the group is zero, the operator is a super user, permission checks are bypassed by design, and the test must emit `SKIP` with that reason — not `FAIL`. This is the only test in the slice whose correctness depends on who is running it.

- [ ] **Step 4: `t06seek.a`** — the `I$Seek` position-register convention.
  Adapt `test/6809-live-verification/syscall-iseek.a`, which already establishes the technique. Carry over its hard-won detail: **`U` is this program's own data-area base pointer and I$Seek overwrites it with the position's low word**, so it must be pushed before the call and pulled immediately after, before any `,u`-relative reference. `PULS` does not affect CC, so the carry and `B` from the call survive the restore. The test creates its own fixture rather than depending on one from another test.

- [ ] **Step 5: `t07div0.bas`** — the documented error code for integer division by zero.
  Changes from `t02int.bas`: an `ON ERROR GOTO` handler captures the code the program actually receives, which is what makes the observed value trustworthy rather than a report of how the runtime died. Project memory records that code 105 here is correct and not a defect — so this test is expected green, and its value is confirming that on hardware.

- [ ] **Step 6: `t08extend.bas`** — seek past end of file, then write; the documented extension behaviour.
  Changes: creates its own file in `SCRATCH/`, seeks beyond EOF, writes, then reopens and checks the resulting size. Report the observed size as `obs=`.

- [ ] **Step 7:** Confirm the tally equals the number of tests after every addition. A test that fails to run at all leaves the tally short, and that is the **only** signal it gives — nothing prints when a module silently does not load.

- [ ] **Step 8: Commit** after each test individually, never as a batch.

---

### Task 7: Distribution readiness

**Files:**
- Create: `test/6809-conformance/DOCS/ourruns.md`
- Modify: `test/6809-conformance/text/readme`
- Create: `tools/selfhost6809/run-on-nitros9.sh`

- [ ] **Step 1: Write `run-on-nitros9.sh`**

Serves the image via `NITROS9REPL_EXTRA_DWCLI="--disk0 <image>"`, logs in as a non-super user, runs `runall`, and retrieves `RESULTS/report`.

- [ ] **Step 2: Record our baseline**

Run it, and record the output in `DOCS/ourruns.md`, labelled explicitly as a candidate observation from a reimplementation — not a standard.

- [ ] **Step 3: Write the readme**

Three commands, what the disk writes (`SCRATCH/` and `RESULTS/` on this disk only, nothing else, ever), the owner ID the files carry and why, what a `SKIP` means, and how to send results back.

- [ ] **Step 4: Run the full verification protocol from the spec**

All six items. Every one must be **observed**, not assumed:
1. Full run completes and writes `RESULTS/report`.
2. A deliberately broken expectation produces `FAIL`.
3. A missing prerequisite produces `SKIP`, not `FAIL`.
4. The run is performed as a non-super user, and the report records which.
5. `rebuild` is exercised at least once.
6. The image is confirmed to have written nothing outside itself.

For item 6: snapshot the guest's `/dd` directory listings before and after a run and diff them.

- [ ] **Step 5: Commit**

```bash
git add test/6809-conformance/ tools/selfhost6809/
git commit -m "Docs: distribution readme and recorded baseline for the 6809 suite"
```

---

## Phase 2 — record locking

A separate plan, written once Phase 1 has produced a report from a real run. Its three tests and the two constraints they must respect (racers self-report completion; every run proves its own preconditions) are specified in the spec under "Phase 2 — record locking". A timing-sensitive test that cannot establish its preconditions reports `ERROR`, never `FAIL`.

## Open item found during design, not part of this plan

`is_super()` (`Source/OS9exec_core/procstuff.c:224`) requires `group==0 && user==0`. Microware training manuals define the super user as any user in **group zero** (`Microware_Training_OS-9_Starter.txt:1349`, `Microware_Training_OS-9_Advanced.txt:208`). It is the only privilege test in the codebase and it gates RBF permission enforcement. Awaiting a decision on whether to fix now or record as a divergence.
