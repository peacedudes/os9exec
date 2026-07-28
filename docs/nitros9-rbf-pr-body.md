# RBF: reload the full lock request on a post-park retry

A record-lock retry after a park re-presented a clobbered byte count, so a
woken waiter proceeded holding no lock. Two processes doing an ordinary
read-modify-write cycle on one record lose updates, silently.

One file, two hunks, +4 bytes.

## The bug

`L0B1D` is the entry to the record-lock claim, and also where a process lands
when it retries after being parked on another process's lock.

On the first pass the caller's byte count is live in `X:D`. On a retry it is
not. `L0B11` arrives from the sleep path with `B` holding `L1053`'s `P$Signal`
read — 0 once an `S$Wake` has been consumed — and `X` holding the
`F$Sleep`/`PE.TmOut` remainder, also 0. Only `A` was reloaded.

So any request under 256 bytes re-presented after a park as `X:D = 0`, and
that is exactly the value `L0B9F` treats as the *dismiss* request. It released
the path's locks, returned carry clear, and the woken waiter carried straight
on into its read-modify-write **holding no lock at all**.

The failure is silent. Nothing errors; the updates simply go missing.

## The fix

Re-present the caller's request from the frame `L0B1B` already pushes:

```
-                    lda       ,s
+                    ldd       ,s                  get byte count LSW back
+                    ldx       2,s                 and its MSW: the entry request
```

The four extra bytes push `L0B11` out of short-branch reach, so one `bne`
becomes `lbne`. That second hunk is a consequence of the first, not a separate
change.

## How it was measured

NitrOS-9 L2 V3.3.0 on an emulated CoCo3 (XRoar), A/B/A against the same disk
image with only the RBF module swapped.

Two concurrent update-mode racers run an unprotected
`SEEK / GET / increment / SEEK / PUT` loop on one record, with **no** `SS_Lock`
anywhere — the automatic record lock is the whole point. Each racer signals its
own completion through a separate file, so a short count can never be confused
with a racer that simply had not finished yet; a run where both racers have not
reported is discarded rather than counted.

| workload | stock | with this patch |
|---|---|---|
| 2 racers x 100 increments | 173, 183, 190, 190, 191, 197 of 200 | 200/200, every run |
| 2 racers x 200 increments | 21, 21, 48, 59, 64, 73, 110 lost of 400 | 400/400, every run |

The amount lost varies from run to run — 3 to 27 of 200, 21 to 110 of 400 —
which is the signature of a real race rather than a fixed miscount. The fix
lost nothing on any run.

The contended record deliberately sits away from end of file, so this exercises
the ordinary record lock rather than the EOF lock.

## Regression checks against the patched build

- A contended record lock still blocks the waiter — 5s, against a 2s
  uncontended control — and the waiter reads the **post-write** value, in the
  correct order (holder acquires, waiter announces and blocks, holder writes and
  releases, waiter returns).
- Crossed two-path holds still return exactly one `E$DeadLk` (#254), with the
  other half completing. Four runs.
- A reader still trails a live producer, gaining each record within a second or
  two of its write — verified for update-mode and write-only producers, and for
  write-only and update-mode *creators*. A negative control on an
  already-closed file reads every record in the same instant, which is what
  "not following" looks like.
- A 23762-byte file copied, and copied again into a newly created
  subdirectory, comes back byte-identical with good module CRCs — roughly 93
  sectors of write-at-end-of-file segment extension.

## Build

Assembles for both targets with no warnings, CRC and header parity valid on
each: 6809 (`H6309=0`) 4846 -> 4850 bytes, 6309 (`H6309=1`) 4751 -> 4755.
Applies cleanly to current `main`. Level 1 RBF has no record locking, so this
is a Level 2 change only.
