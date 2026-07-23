# NitrOS-9 RBF: two real bugs, both fixed (not yet upstreamed)

## Bug 1 — write-only opens took locks they shouldn't

**Broken:** RBF locked records/EOF for *any* open mode, so a plain
write-only writer (`>`) blocked read-only followers for no reason — the
design only intends locking for paths open for update (read+write).

**Fixed:** gate all three lock-acquire sites on open mode; write-only
paths now take no lock at all (+24 bytes). `docs/nitros9-rbf-lockmode.patch`.

## Bug 2 — a lock retry could silently grant zero lock

**Broken:** after a process parked waiting for a contended record lock,
its retry reloaded the wrong registers — a stale leftover value instead
of the original request. For small requests this decoded as "claim
nothing," so the woken process ran its read-modify-write **completely
unlocked**, silently corrupting concurrent updates.

**Fixed:** reload the full original request on retry instead of the
stale value (+4 bytes). `docs/nitros9-rbf-lostupdate-regfix.patch`.
Measured: stock loses 21–110 updates per 400 concurrent writes; fixed
loses 0, across 5 runs.

## Status

Both fixes proven independently (live A/B/A). Combined into one module,
boot-verified live. **Not pushed upstream — owner wants the combined
build put through more testing first.** Full detail, checksums, and
build provenance: `docs/nitros9-rbf-reference/README.md`.
