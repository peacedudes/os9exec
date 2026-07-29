# kermit end-to-end between two emulator instances (hostterm-pty Task 6)

Acceptance test for `docs/superpowers/plans/2026-07-29-hostterm-pty.md`,
Task 6, run against commit `3e53f26` ("Core: OS9Tn=pty allocates a
terminal and reports its device"), the tip of the six commits listed as
done. Backs no skill claim directly; records a defect and two kermit
usage facts that should be added to `os9-dev` (see "Skill feedback").

## Verdict (original run, commit `3e53f26`)

**FAIL.** The text-file leg (A -> B) is clean and byte-perfect on both
sides. The binary-file leg (B -> A) also delivers byte-perfect data
(confirmed by CRC via `ident` and by a host-level `cmp`/`md5`), but
`kermit`'s own sender-side protocol reports **"Send failed"** every time
the pty-master-holding instance is the *receiver* -- reproduced with
both a 949-byte text file and the 12940-byte binary module, so it is not
a size effect. The data arrives intact; the transfer does not complete
*cleanly*, which is what Task 6 asks to prove. See "Finding: sender-side
false failure" below for the evidence and a root-cause hypothesis in
`hostterm_close()`.

## Update 2026-07-29: PASS after the Task 6a binding-lifetime fix

**Re-run against commit `d98acc0`** ("Fix: a bound /tN outlives the path
that opened it") confirms the hypothesis below was correct and the
defect is fixed. See "Re-run: PASS in both directions" for the full
evidence. **Both directions now complete cleanly, byte-perfect, with no
sender-side "Send failed" and no retry timeouts.** The FAIL verdict
above stands as the historical record of what commit `3e53f26` did; it
is superseded by the PASS below, not edited away.

## Rig

**Driver used: a scripted Python driver, not `tools/os9repl.sh`.**
`tools/os9repl.sh` is one tmux session per call; this task needs two
independent, simultaneously-live emulator processes with the driver
reading and writing both continuously and reacting to output from one to
configure the other (the pty slave name printed by the `OS9T1=pty`
instance has to be read before the second instance can even be
launched). A small Python script using `subprocess.Popen` with `stdin`/
`stdout` pipes (no pty needed for the *driving* side -- confirmed a
plain pipe is sufficient to drive an interactive `os9exec -r shell`
session, matching how the project's own `<<EOF` heredoc invocations
already behave) proved reliable and is what produced every transcript
below. Script: `driver.py` (a small `Inst` class: launch, `send(line)`,
`wait_for(regex, timeout)` against a continuously-read output buffer).
Two `tools/os9repl.sh` sessions were not attempted since the Python
driver worked on the first real try once the `kermit` argument syntax
was worked out (see below) and gave exact byte-level control over
timing that a tmux-snapshot poll loop would have made harder to get
right for a protocol like Kermit's.

Both instances always ran `-r` (no baud pacing, per the plan's own
warning) and `OS9DISK=$PWD/h0`. Scratch host directories were mounted at
`/h5` via `OS9H5=<dir>` (never wrote into the system disk).

**Kermit argument-syntax correction needed before anything worked.** The
plan's own Task 6 example (`kermit s /t1 /dd/echo`, `kermit r /t1`) does
not work as written: `kermit`'s usage line is `s[dx8ifl line] file`, and
the `l` character in that flag bundle is not decorative -- without it,
`kermit` silently ignores the device argument entirely and streams its
protocol bytes to the console/default line instead (confirmed by
capturing raw output: `kermit s /t1 /dd/startup` produced a single
Kermit `S`-packet on the **console**, and no `# /t1 is /dev/...` open
message ever appeared, meaning `/t1` was never opened at all). The
working forms are `kermit sl /t1 file` and `kermit rl /t1` for text, and
`kermit sli /t1 file` / `kermit ril /t1` for binary (see next section for
why `i` is needed and why `8` is actively harmful). This is a plan
inaccuracy, not an emulator defect, and is now recorded in "Skill
feedback" for `os9-dev` since kermit isn't documented there at all.

## Leg 1: text file, A -> B

Instance A: `OS9T1=pty`. Instance B: `OS9T1=<A's reported slave>`.
File: `/dd/startup` (949 bytes, plain ASCII with CR line endings).

Commands, in order:
- A: `kermit sl /t1 /dd/startup`
- B: `chd /h5` then `kermit rl /t1`
- B: `dir /h5`, `dump /h5/startup`
- A: `dump /dd/startup`

Raw captured output (A):

```
$ kermit sl /t1 /dd/startup
# /t1 is /dev/ttys002   (attach with: screen /dev/ttys002)
Kermit: Sending /dd/startup as STARTUP
...

Kermit: done.

$ Read I/O error - Error #000:025 (E_???) <<unknown error code>>

$
```

Raw captured output (B):

```
$ kermit rl /t1
Kermit: Receiving STARTUP as startup
...

Kermit: done.

$ Read I/O error - Error #000:025 (E_???) <<unknown error code>>

$ dir /h5

                           Directory of /h5 04:56:44
startup

$ dump /h5/startup

  Addr     0 1  2 3  4 5  6 7  8 9  A B  C D  E F 0 2 4 6 8 A C E
--------  ---- ---- ---- ---- ---- ---- ---- ---- ----------------
00000000  2a20 4f53 2d39 2053 7973 7465 6d20 5374 * OS-9 System St
00000010  6172 7475 7020 5363 7269 7074 0d2a 2050 artup Script.* P
... (identical body omitted here; full transcript in leg1_full_log.txt
     below, and the byte-for-byte comparison is what matters, next)
```

### The dump comparison (not by eye)

`dump /dd/startup` on A and `dump /h5/startup` on B were captured in
full (60 hex-dump lines each) and diffed as text:

```
$ diff leg1_a_dump.txt leg1_b_dump.txt
1c1
< dump /dd/startup
---
> dump /h5/startup
```

The **only** difference between the two full 60-line hex dumps is the
echoed command line itself (`/dd/startup` vs `/h5/startup`); all 60
lines of hex/ASCII body are identical, character for character.

This was cross-checked independently at the host level, bypassing
`dump`/`ident` entirely, since both `/dd` and `/h5` are host-directory
mounts on this machine:

```
$ cmp h0/startup <B's /h5 scratch dir>/startup
$ echo $?
0
$ md5 h0/startup <B's /h5 scratch dir>/startup
MD5 (h0/startup) = b0c94b9b92c513bd34096d291b5887ca
MD5 (<B's /h5 scratch dir>/startup) = b0c94b9b92c513bd34096d291b5887ca
```

**Leg 1 result: PASS.** Byte-for-byte identical, both `kermit` processes
report `done.` cleanly, no retries, no failure messages other than the
generic post-kermit "Read I/O error" (see "Minor, unrelated artifact"
below -- present here too, and harmless).

## Leg 2: binary module, B -> A

Instance A: `OS9T1=pty` (receiver this time). Instance B:
`OS9T1=<A's reported slave>` (sender this time). File:
`/dd/CMDS/hello`, a 12940-byte 68000 `Prog Mod` -- `ident` on the
original: `Module CRC: $1A4CE4 Good CRC`.

Three attempts were needed to find working `kermit` flags for a binary
file; all three are reported since the first two are themselves
findings.

**Attempt 1 -- `kermit sl /t1 file` / `kermit rl /t1` (same flags as the
working text leg):**

```
$ kermit sl /t1 /dd/CMDS/hello
Kermit: Sending /dd/CMDS/hello as HELLO

Kermit: Send failed.
```

Fails immediately, zero data packets exchanged. `kermit` refuses to even
start a binary-content file without an explicit mode flag.

**Attempt 2 -- `kermit s8il /t1 file` / `kermit r8il /t1` (added `8`
8-bit-quoting and `i` image-mode flags):**

Sender (B):

```
$ kermit s8il /t1 /dd/CMDS/hello
Kermit: Sending /dd/CMDS/hello as HELLO
...............................................................
TTTTTTTTTTT
Kermit: Send failed.
```

Receiver (A) -- claims success, but:

```
$ kermit r8il /t1
# /t1 is /dev/ttys002   (attach with: screen /dev/ttys002)
Kermit: Receiving HELLO as hello
...............................................................
Kermit: done.
```

```
$ ident /h5/hello
ident: module sync wrong in file "/h5/hello" at 0x0
000:205 (E$BMID)   Bad Module ID.
ident: unexpected eof on file "/h5/hello"
000:211 (E$EOF)    The end of file has been reached.
```

The received file was 16113 bytes, not 12940. A host hex dump of it
shows the file contains kermit's own **wire-encoded** bytes, not the
decoded payload: the correct module sync `4AFC` arrives on the wire as
`4A 59 7C` (`59`='Y' is kermit's 8th-bit quote-prefix character, and
`7C`=`FC XOR 80` is exactly the quoted form of the real second sync
byte `FC`), and this pattern repeats throughout -- the receiver is not
un-quoting the 8-bit-quoted stream before writing it to disk. The `8`
flag forces 8-bit quoting; since the hostterm pty channel is already
fully 8-bit-transparent (an already-verified fact this plan is built
on), quoting was never needed and, combined with whatever local/receiver
default is in play, this flag actively corrupts the transfer. This is a
`kermit`-flag usage mistake on the driver's part, not a hostterm defect,
and is recorded in "Skill feedback".

**Attempt 3 -- `kermit sli /t1 file` / `kermit ril /t1` (image mode
only, no forced 8-bit quoting) -- the one that matters:**

```
$ kermit sli /t1 /dd/CMDS/hello
Kermit: Sending /dd/CMDS/hello as HELLO
......................................................
TTTTTTTTTTT
Kermit: Send failed.

$ Read I/O error - Error #000:025 (E_???) <<unknown error code>>

$
```

```
$ kermit ril /t1
# /t1 is /dev/ttys002   (attach with: screen /dev/ttys002)
Kermit: Receiving HELLO as hello
.....................................................

Kermit: done.

$ Read I/O error - Error #000:025 (E_???) <<unknown error code>>

$ dir /h5

                           Directory of /h5 05:06:35
hello

$ ident /h5/hello
Header for:      hello
Module size:     $328C       #12940
Owner:           0.0
Module CRC:      $1A4CE4     Good CRC
Header parity:   $394        Good parity
Edition:         $7          #7
Ty/La At/Rev     $101        $8001
Permission:      $555        -----e-r-e-r-e-r
Exec off:        $92         #146
Data size:       $BEA        #3050
Stack size:      $C00        #3072
Init. data off:  $313A       #12602
Data ref. off:   $3258       #12888
68000 Prog Mod, Object Code, Sharable

$ ident /dd/CMDS/hello
Header for:      hello
Module size:     $328C       #12940
Owner:           0.0
Module CRC:      $1A4CE4     Good CRC
Header parity:   $394        Good parity
Edition:         $7          #7
Ty/La At/Rev     $101        $8001
Permission:      $555        -----e-r-e-r-e-r
Exec off:        $92         #146
Data size:       $BEA        #3050
Stack size:      $C00        #3072
Init. data off:  $313A       #12602
Data ref. off:   $3258       #12888
68000 Prog Mod, Object Code, Sharable

$
```

`ident` on the received copy reports **Good CRC**, and every field
(size, CRC, edition, permission, offsets) matches the original
identically. This was cross-checked at the host level too, bypassing
`ident`:

```
$ cmp h0/CMDS/hello <A's /h5 scratch dir>/hello
$ echo $?
0
$ md5 h0/CMDS/hello <A's /h5 scratch dir>/hello
MD5 (h0/CMDS/hello) = e8638adafdcc9e3ddaf8c6542a04317c
MD5 (<A's /h5 scratch dir>/hello) = e8638adafdcc9e3ddaf8c6542a04317c
```

**The received data is byte-perfect.** But the sender (`B`, the
instance that opened `/t1` by name) reported **"Send failed"** with
eleven retry timeouts (`TTTTTTTTTTT`), even though the receiver (`A`,
the instance that holds the pty master) reported clean success and the
file content is provably correct.

## Finding: sender-side false failure when the pty-master instance is the receiver

This is the reason for the FAIL verdict. It reproduces independent of
file size and content, which rules out anything about the binary
module's bytes specifically:

A follow-up probe sent the **same small text file used in Leg 1**
(`/dd/startup`, 949 bytes, plain ASCII) but with the roles swapped from
Leg 1 -- `B` (named-open side) sending, `A` (pty-master side) receiving,
exactly Leg 2's direction:

```
$ kermit sli /t1 /dd/startup          <- run on B (sender, named-open)
Kermit: Sending /dd/startup as STARTUP
...
TT
                                        <- never returns to prompt within 20s
```

```
$ kermit rli /t1                       <- run on A (receiver, pty master)
# /t1 is /dev/ttys002   (attach with: screen /dev/ttys002)
Kermit: Receiving STARTUP as startup
...

Kermit: done.
```

Same asymmetry with a 3-packet transfer as with the ~55-packet binary
transfer: the receiver (pty-master side) always reports clean success;
the sender (named-open side) always times out waiting for the final
handshake ACK, when and only when the pty-master-holding instance is the
*receiver*. Leg 1 (pty-master instance A sending, named-open instance B
receiving) showed no such issue in either the small-file or larger
runs attempted.

**Hypothesis, not confirmed by code change (per the task's "do not fix,
report" instruction):** `hostterm_close()` in
`Source/OS9exec_core/hostterm.c` (~lines 270-286) closes **both** the
master fd (`h->fd`) and the extra slave-holding reference
(`h->spareFd`, added in Task 5 to work around a different, already-
documented pty quirk) unconditionally and immediately, the moment the
owning instance's `kermit` process calls `I$Close` on `/t1` -- which
happens right after that instance's kermit believes it has sent its
final ACK. If the peer has not yet drained that ACK out of the pty's
kernel buffer at the moment the master closes, macOS's pty semantics may
discard it -- the same *class* of data-loss trap already documented in
the plan for closing the *slave* side ("Closing the last open reference
to a pty slave discards unread output on macOS, deterministically"),
but here on the *master* side, when the master-holding instance
(`OS9T1=pty`) is the one that finishes and exits first. This is a
hypothesis based on the code and the reproduction pattern, not verified
with `dtruss`/`strace` timing evidence, and is left for the controller
to confirm and scope -- per the task instructions, no source change was
made.

## Re-run: PASS in both directions (after Task 6a's fix)

Re-run 2026-07-29 against commit `d98acc0`, using the same rig, driver
shape, files and `kermit` flags as the original run (`kermit sl`/`kermit
rl` for text, `kermit sli`/`kermit ril` for binary -- see above for why).
Driver: same `driver.py` `Inst` class (`subprocess.Popen` with
stdin/stdout pipes), copied into a fresh scratch directory so this run's
artifacts don't mix with the original's. Both instances tracked by PID
and terminated via `proc.terminate()`/`proc.wait()` in a `finally` block;
confirmed no leftover `os9exec` process after each script exit.

**Leg 1 re-run, text, A -> B (A = pty-master, sending)** -- this
direction already passed before the fix; re-run to confirm the fix did
not regress it:

```
===== B kermit receive =====
kermit rl /t1
Kermit: Receiving STARTUP as startup
...

Kermit: done.

$ Read I/O error - Error #000:025 (E_???) <<unknown error code>>

===== A kermit send =====
kermit sl /t1 /dd/startup
# /t1 is /dev/ttys002   (attach with: screen /dev/ttys002)
Kermit: Sending /dd/startup as STARTUP
...

Kermit: done.

$ Read I/O error - Error #000:025 (E_???) <<unknown error code>>
```

`diff` of the two full 60-line hex dumps again shows only the echoed
command line differing; host-level cross-check:

```
$ cmp h0/startup <B's scratch dir>/startup ; echo $?
0
$ md5 h0/startup <B's scratch dir>/startup
MD5 (h0/startup) = b0c94b9b92c513bd34096d291b5887ca
MD5 (<B's scratch dir>/startup) = b0c94b9b92c513bd34096d291b5887ca
```

Still clean. No regression.

**Leg 2 re-run, binary, B -> A (A = pty-master, receiving)** -- this is
the direction that failed originally. Same file, same flags
(`kermit sli`/`kermit ril`):

```
===== B2 kermit send =====
kermit sli /t1 /dd/CMDS/hello
Kermit: Sending /dd/CMDS/hello as HELLO
......................................................

Kermit: done.

$ Read I/O error - Error #000:025 (E_???) <<unknown error code>>

===== A2 kermit receive =====
kermit ril /t1
# /t1 is /dev/ttys002   (attach with: screen /dev/ttys002)
Kermit: Receiving HELLO as hello
.....................................................

Kermit: done.

$ Read I/O error - Error #000:025 (E_???) <<unknown error code>>
```

**The sender (`B2`) now reports `Kermit: done.` with zero retries** --
no `TTTTTTTTTTT`, no "Send failed", where the original run showed eleven
timeout retries and a hard failure on this exact leg. `ident` on the
received copy:

```
$ ident /h5/hello
...
Module CRC:      $1A4CE4     Good CRC
...
$ ident /dd/CMDS/hello
...
Module CRC:      $1A4CE4     Good CRC
...
```

Same CRC, same size, same every field, as before. Host-level
cross-check:

```
$ cmp h0/CMDS/hello <A2's scratch dir>/hello ; echo $?
0
$ md5 h0/CMDS/hello <A2's scratch dir>/hello
MD5 (h0/CMDS/hello) = e8638adafdcc9e3ddaf8c6542a04317c
MD5 (<A2's scratch dir>/hello) = e8638adafdcc9e3ddaf8c6542a04317c
```

**Leg 2b re-run: the small-text asymmetry probe (B -> A, same direction
as Leg 2 but with the 949-byte text file, to confirm the fix is not
file-size- or content-dependent)**:

```
===== B3 kermit send (small text, B->A) =====
kermit sli /t1 /dd/startup
Kermit: Sending /dd/startup as STARTUP
...

Kermit: done.

===== A3 kermit receive (small text, B->A) =====
kermit rli /t1
# /t1 is /dev/ttys002   (attach with: screen /dev/ttys002)
Kermit: Receiving STARTUP as startup
...

Kermit: done.
```

Also clean, both sides, no retries. The asymmetry that originally
reproduced with both a 3-packet text transfer and a ~55-packet binary
transfer is gone in both cases.

**Conclusion.** The hypothesis in "Finding: sender-side false failure"
above was correct: `hostterm_close()` was closing the pty master (and
its spare slave reference) the instant the owning instance's `kermit`
process closed `/t1` -- right after that instance believed it had sent
its final ACK, discarding it before the peer could read it if the peer
had not already drained it. Task 6a's fix (commit `d98acc0`) makes a
bound `/tN` endpoint persist for the emulator's lifetime once
established, closing only the *path*, never the underlying host fd, so
that race no longer exists. **Verdict updated: PASS**, both directions,
both file sizes, byte-perfect data (unchanged from the original run) and
now also a clean protocol-level handshake with no sender-side retries or
failure.

## Minor, unrelated artifact (reported, not chased further)

Every `kermit` invocation that actually opened a device line -- success
or failure, either direction, both legs -- was followed by:

```
$ Read I/O error - Error #000:025 (E_???) <<unknown error code>>
```

on the shell's own **main console** (not `/t1`), right after `kermit`
exited and before the next `$` prompt. Confirmed this is unrelated to
hostterm specifically: a bare `kermit` invocation with **no** device
argument at all (just the usage-line printout) produces no such message.
So it is tied to "kermit opened *some* line and then exited", not to
`/t1`/hostterm in particular, and it never correlated with whether the
transfer itself succeeded (Leg 1's clean success shows it too). Not
investigated further since it does not affect the file content in any
observed case; flagged here in case it is the same root cause as the
sender-side failure above (both are host-fd-close-adjacent) or a
separate, pre-existing `kermit`/console interaction.

## Skill feedback

- **`kermit` is not documented in `os9-dev` at all** -- confirmed with
  `grep -rli kermit` across both `os9-dev` and `os9-systems-dev`: zero
  hits. Given how load-bearing its exact argument syntax turned out to
  be (see below), this is a real gap, not a nice-to-have.
- **New fact for `os9-dev`:** `kermit`'s `s`/`r` mode-letter is followed
  by an *optional* flag-character bundle from `{d,x,8,i,f,l}`, but that
  bundle is not merely decorative -- `l` is what tells `kermit` to
  consume the next word as the line device at all. `kermit s /t1 file`
  (no `l`) silently ignores `/t1` and sends its protocol stream to the
  default line instead, with no error. Working forms:
  `kermit sl <line> file...` / `kermit rl <line>` for text,
  `kermit sli <line> file...` / `kermit ril <line>` for binary.
- **New fact for `os9-dev`:** the `8` (8-bit-quoting) flag actively
  corrupts a transfer over an already-8-bit-clean channel (a hostterm
  pty, per this plan's own verified facts) -- it does not merely add
  needless overhead. The receiver did not undo the quoting, leaving
  literal Kermit-protocol quote-prefix/XOR bytes in the delivered file.
  Use `i` (image/binary mode) alone for a binary file over hostterm;
  do not add `8`.
- **New finding, not previously in either skill:** the sender-false-
  failure defect described above (`hostterm_close()` closing the master
  and spare-slave fd immediately on `I$Close`, plausibly racing the
  peer's read of final bytes). Worth a card once the controller decides
  whether/how to fix it -- not added as a skill claim here since it is
  unconfirmed and the fix (if any) is out of this task's scope.
- The already-known gaps listed in the task prompt (`tsmon`'s GetStt/
  SetStt protocol, `KeyToBuffer`'s `PD_XON`/`PD_XOFF` wedge, pty raw-mode
  write-ordering, `tcgetattr` on a bare `posix_openpt` master, glibc's
  `XOPEN2K` gating) were not re-investigated, per instructions.
- No other gaps or inaccuracies noticed in `os9-systems-dev` for this
  task; it was consulted (hostterm.c's own console-dispatch seams) but
  this task did not touch systems-level territory beyond reading the
  existing hostterm implementation.

## Process hygiene

Both emulator instances were tracked by PID from `subprocess.Popen`
throughout (never `pkill -f os9exec`) and terminated via
`proc.terminate()`/`proc.wait()` in a `finally` block after every run,
confirmed by `ps aux | grep os9exec` showing no leftover process after
each script exit. No writes were made to the system disk (`h0/`) beyond
the pre-existing `/dd/startup` and `/dd/CMDS/hello` reads; all received
files landed in private scratch directories under
`/private/tmp/.../scratchpad/kermit-test/` (outside the repo).
