# RBF record and end-of-file locking: what was broken, and what it does now

Two mechanisms, one design. A read on a path open for update locks the
record it read, and the following write lets it go — so a read-modify-write
cycle is safe against another process doing the same thing, without the
application making any locking calls. The end-of-file case is the same
lock placed where there is no data yet: a writer holds the position past
the last byte, so a reader that catches up waits there instead of being
told the file is finished.

Neither worked. The record half is below under "Record locking"; the
end-of-file half follows immediately.

# End-of-file lock

Two programs, one file. A writer appends a timestamped record once a
second and closes. A reader opened while the writer is still running
follows it. This is the case the EOF lock exists for: a reader that has
caught up to a file someone is still writing has not reached the end of
it, it is merely early.

Both transcripts below are the same two programs, on the same disk image,
against binaries that differ only in `Source/OS9exec_core/file_rbf.c`.

`W<n>` is the writer; `R<n>` is the reader, showing the timestamp inside
the record next to the time it actually came back from `READ`.

## Before

```
W1 wrote at 07:45:25
W2 wrote at 07:45:26
W4 wrote at 07:45:28
R end n=0 err=211
W5 wrote at 07:45:29
W6 wrote at 07:45:30
```

The reader read **nothing at all** — `n=0` — and got `E$EOF` (211)
immediately, while the writer was demonstrably still writing around it.
It did not block, did not wait, and did not come back. A reader that
opens while a file is being written is told the file is finished.

That was not a race or a timing artifact: it happened on every run, and
a path that opened first stayed blind to that file for the rest of its
life, even after the writer closed. Reopening the file afterwards read
everything correctly, so the data was always there and always intact.

## After

```
W1 wrote at 07:45:58
W2 wrote at 07:45:59
W4 wrote at 07:46:01
W5 wrote at 07:46:02
R1 line=07:45:58 seen at 07:46:02     <- reading while the writer runs
W6 wrote at 07:46:03
R2 line=07:45:59 seen at 07:46:03
W done
R3 line=07:46:00 seen at 07:46:03
R4 line=07:46:01 seen at 07:46:03
R5 line=07:46:02 seen at 07:46:03
R6 line=07:46:03 seen at 07:46:03     <- caught up, zero lag
R end n=6 err=211                     <- real end of file, after close
```

The reader now reads records as they appear, interleaved with the
writer's own output rather than arriving in one burst afterwards. It
catches up to zero lag by the last record, and only sees `E$EOF` once the
writer has closed and there genuinely is nothing more coming.

## What the fix is

Nothing is locked. The wait sits past the last byte, where there is no
data to lock — which is why a second writer appending is unaffected, and
why two programs logging to the same file do not shut each other out.

1. Paths open on the same file are linked in a ring. Every path keeps its
   own buffers, but can now reach the others'. Previously two paths on
   one file were two independent views of it: each held the file
   descriptor it copied at open, so a reader never saw the size or the
   segment list change underneath it, and could neither tell the file had
   grown nor find the sectors it had grown into.
2. A write publishes the new size and segment list to the other paths,
   and a sector still dirty in one path's buffer is read from there
   rather than from the device.
3. A read that finds nothing checks whether **another process** still has
   the file open for writing. If so it sleeps instead of reporting the
   end of the file. The writer wakes every sleeper on each write, and
   again on close.
4. A conflict with one's own process is refused with `E$DEADLK` rather
   than slept on — waiting for yourself is what makes the wait never end.

## Record locking

`dogfood-recordlock.bas`. Two paths on one file, both open for update, in
one process. Path A reads the record — which should lock it. Path B then
reads the same bytes.

```
before:   reclock: A read and locked RECORD01
          reclock: FAIL B read it anyway =RECORD01

after:    reclock: A read and locked RECORD01
          reclock: OK conflicting read refused err=254
```

Before, the second read handed over a record the first path was in the
middle of updating — the lost update the mechanism exists to prevent.
After, it is refused with `E$DEADLK`, because the only process that could
release that lock is the one asking for it.

**Why this test and not a counter race.** The obvious test — two processes
racing to increment a shared counter — cannot answer the question. It only
shows whether an update was *lost*, and on a cooperatively scheduled
emulator a read-modify-write cycle usually completes without a task switch
landing in the middle. It therefore passes whether or not any lock exists,
and did pass, 600/600, against code that had no locking whatsoever. To
tell a working lock from a scheduler that never interleaves, you have to
make a conflict happen and watch it be refused.

### Across two processes

The case above is one process, which keeps it deterministic. The one that
matters in practice is two:

```
before:   hold: locked at 11:27:54
          wait: asking at 11:27:54
          wait: got it at 11:27:54  data=RECORD01   <- the pre-update value

after:    hold: locked at   11:27:55
          wait: asking at   11:27:55
          hold: released at 11:27:55
          wait: got it at   11:27:55  data=RECORD02 <- waited, got the update
```

Before, the waiter was handed a record the holder was partway through
updating, and read the stale value. After, it waits and reads what the
holder actually wrote. That is the lost update itself, not a proxy for it.

### `SS_Lock` and `SS_Ticks`

`SS_Lock` previously returned success while doing nothing, so a program
that locked defensively was told it had succeeded and got no protection —
worse than an honest error. It now takes or releases a record directly,
reports `E$LOCK` when another process holds the bytes, and refuses a
self-conflict with `E$DEADLK`. A zero-byte read or write drops everything
the path holds, as documented. `dogfood-sslock.a` walks the four steps;
before the fix, step 2 let a second path lock bytes the first was holding.

`SS_Ticks` bounds the wait, for a program that must not hang behind a peer
that has stopped responding. It has a dependency worth knowing about: a
timeout can only fire if the waiting process is re-run while it waits, and
without a system tick almost nothing re-runs it. Measured against a
two-second hold, a blocked reader got two chances to look and then none
until the holder released — so the limit was never noticed:

```
without -q:   ssticks: FAIL blocked until the holder finished
with    -q:   ssticks: OK   gave up as asked (E_LOCK)
```

A timeout is only as good as the scheduling underneath it, which is why
the optional system tick and the lock work are one story rather than two.

## Reproducing

`dogfood-eoflock-writer.bas` / `-reader-readonly.bas` are the original
pair that first demonstrated the bug. The compact pair used above writes
six records instead of forty so a run takes about eight seconds.

Four further tests are single-process, deterministic, and need no timing
at all — each isolates one thing the ring has to get right:

| Test | What fails without it |
|---|---|
| `dogfood-eoflock-visibility.bas` | a reader cannot see a record written after it opened |
| `dogfood-eoflock-invalidate.bas` | a reader that cached a sector keeps serving itself the stale copy |
| `dogfood-eoflock-writerclose.bas` | a reader loses the file's contents when the writer closes |
| `dogfood-eoflock-deadlock.bas` | a process waits for itself and hangs, instead of being refused |
| `dogfood-recordlock.bas` | a read hands over a record another path is updating |
| `dogfood-sslock.a` | `SS_Lock` says it locked and did not |

Two need a second process, so they are a pair run as `holder & waiter`:

| Test | What fails without it |
|---|---|
| `dogfood-recordlock-holder/-waiter.bas` | a reader gets a record mid-update |
| `dogfood-ssticks.a` (+ holder) | a bounded wait waits forever anyway |

The emulator's own regression suite is green (129/129) with these
changes, unchanged from before them.
