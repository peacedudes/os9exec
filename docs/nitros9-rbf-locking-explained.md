# RBF record locking: how it works, and two bugs in it

**How it works.** RBF's locking is one simple rule, not a feature you call: a
`READ` on a path open for update locks the bytes it just returned, and the
next `WRITE` on that same path releases them — anyone else touching those
bytes just sleeps until then, woken automatically. End-of-file uses the
exact same rule, applied past the last byte — a "ghost" lock over data that
doesn't exist yet. A writer appending holds it; a reader that catches up
doesn't conclude "done," it hits the ghost and waits, because more may still
come. Every `WRITE` releases and re-takes the ghost at the new end, waking
the reader the instant something lands; only `CLOSE` lets it go for good.
One lock, one wait/wake rule, and it buys two things for free: a
read-modify-write cycle can never lose an update, and a plain growing file
can coordinate a slow writer and reader exactly like a pipe — no pipe
required.

**What broke, and the fix.** Both `os9exec` and NitrOS-9 — independent
implementations, decades apart — locked regardless of open mode. But only
an update-mode path should ever claim or wait on anything: a write-only
writer taking a lock blocks innocent readers for no reason, and worse,
blocks *other* write-only writers from freely interleaving into the same
file — defeating the exact shared-log use the design exists for. NitrOS-9
had a second, sharper bug: a process woken from a wait, on retrying its
claim, reloaded stale leftover register values instead of its real
request — which for small requests silently decoded as "claim nothing," so
the process ran its read-modify-write completely unlocked, losing updates
under real concurrent load. Both fixes are the same idea restated: gate
every lock on update-mode only, and make the retry reload the real request.

## Patches

- `nitros9-rbf-lockmode.patch` — gates all three lock-acquire sites
  (record lock, EOF lock, and `Creat`'s EOF lock at file-creation time) on
  open mode. +24 bytes.
- `nitros9-rbf-lostupdate-regfix.patch` — reloads the full original request
  on retry instead of stale leftover registers. +4 bytes.

Measured live, A/B/A against stock: the mode-gate stops a write-only
producer/creator from blocking read-only followers (previously followed at
~1s/record, now stops immediately); the retry fix drops concurrent-update
losses from 21–110 per 400 to 0, across repeated runs, with wait-chain
blocking and deadlock detection (`E$DeadLk`) unregressed.
