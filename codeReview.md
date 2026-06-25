# Code Review — arm64-uae-integration branch

Scope: the files most heavily touched by the arm64 port plus the core
emulation engine.  Not a fine-toothed lint of every line — focus is on
correctness issues, LP64 hazards, and things that could silently corrupt data.

---

## 1. CRITICAL — `fileaccess.c:1657` — 8-byte write overflows 16-byte buffer

```c
#define   FDS 16
byte      fdbeg[FDS];                  /* buffer for preparing FD */
ulong*    sizeP   = (ulong*)&fdbeg[9]; /* the position of the size field */
```

On arm64 `ulong` is 8 bytes.  Writing `*sizeP` at offset 9 of a 16-byte
buffer writes bytes 9–16 (inclusive), i.e. one byte past the end of `fdbeg`.
Worse: the OS-9 FD creation-date bytes at `fdbeg[13–15]` are set earlier in
the function and then silently zeroed by the 8-byte store (bytes 13–16 of
a little-endian zero-padded 64-bit value).  Every call to `getFD` that
handles a regular file corrupts the creation date in the FD reply.

**Fix:** `uint32_t* sizeP = (uint32_t*)&fdbeg[9];`  
The OS-9 FD size field is exactly 4 bytes; `uint32_t*` writes the right width.
The `os9_long()` byte-swap result already fits in 32 bits so no other changes
are needed — the compiler truncates the 32-bit swap result correctly into
the `uint32_t` store.

Affects: all `*sizeP = ...` assignments on lines 1904, 1907, 1912, 1920,
1923, and 1929.

---

## 2. Significant — `fcalls.c:937,954` — 8-byte NULL write into 4-byte arena field

```c
ulong **b, memsz;
...
b= (ulong**)FROM68K(rp->a[0]); *b= NULL; /* no segments available */
```

`FROM68K(rp->a[0])` is a host pointer into the 68k arena.  Casting to
`ulong**` and dereferencing writes 8 bytes (on arm64) where OS-9 expects a
4-byte pointer.  For the NULL case the upper 4 bytes happen to be zero, so
the adjacent arena bytes are clobbered with `0x00000000` — which may or may
not matter depending on what lives next in the arena.

**Fix:**
```c
/* declare b separately so memsz keeps type ulong */
uint32_t *b;
ulong memsz;
...
b= (uint32_t*)FROM68K(rp->a[0]); *b= 0; /* no segments available */
```

---

## 3. Latent — `procstuff.c:377` — 64-bit host pointer truncated into 32-bit ISP

```c
#ifdef USE_UAEMU
cp->os9regs.isp= (ulong) &trapframebuf[TRAPFRAMEBUFLEN];
#endif
```

`cp->os9regs.isp` is `uaecptr` (`uint32_t`) in UAE's `regstruct`.  The
right-hand side is a full 64-bit host address, truncated on assignment.
If UAE ever pushes a 68k exception frame onto ISP the engine will write
to a wrong/unmapped location.

In practice UAE doesn't use ISP for normal OS-9 system calls (traps are
intercepted before the exception frame is pushed), so this doesn't manifest
in current tests.  However it will crash if any 68k code triggers a real CPU
exception (e.g. a bus error inside emulated code that UAE's exception logic
tries to handle).

**No simple fix without architectural thought:** `trapframebuf` is in host
memory, but ISP must be a 68k virtual address (arena offset).  Options:
allocate a small scratch block inside the arena and use `TO68K()` of its end,
or accept the restriction that UAE hardware exceptions are not survivable.

---

## 4. Style/clarity — `fileaccess.c:1400, 1432` — redundant `ulong` cast before `uint32_t*` write

```c
os9err pFpos( _pid_, syspath_typ* spP, uint32_t *posP )
    ...
    *posP= (ulong) ftell( spP->stream );  // line 1400
```

The cast to `ulong` (64-bit) before assigning to `*posP` (`uint32_t`)
widens then immediately truncates, obscuring the intent.  The correct idiom
is a direct `uint32_t` cast:

```c
*posP= (uint32_t) ftell( spP->stream );
```

Same pattern on line 1432 in `pFsize`.

---

## 5. Style/clarity — `fcalls.c:839` — host pointer byte-swapped into 68k table

```c
*ptr= os9_long((ulong) &procs[k]); /* not the right ptr, but ... */
```

Already acknowledged in-comment.  Casting a 64-bit host pointer to `ulong`,
then byte-swapping the low 32 bits and storing in a `uint32_t` OS-9 field —
the comment is honest that this is wrong.  Documents correctly; no action
needed beyond leaving the note.

---

## 6. Minor — `filestuff.c:1549–1625` — `vsprintf` with fixed 500-byte buffer

Six output helpers (`usrpath_printf`, `upo_printf`, `upho_printf`,
`upe_printf`, `uphe_printf`, `main_printf`) all use:

```c
char buffer[MAXPRINTFLEN]; /* 500 bytes */
vsprintf(buffer, format, vp);
```

`vsprintf` is unbounded.  In practice the emulated OS-9 programs never
produce lines near 500 bytes, but any caller that passes a format with large
data (e.g. a very long filename) can overflow the stack.

**Low risk in practice.** A defensive fix is `vsnprintf(buffer, MAXPRINTFLEN,
format, vp)` across all six sites.

---

## 7. Minor — `trapframebuf` has no explicit definition

`os9_ll.h` declares:
```c
extern ulong trapframebuf[TRAPFRAMEBUFLEN];
```
No `.c` file defines it.  The binary gets the symbol via C's tentative-definition
/ common-storage coalescing.  This is a non-standard but widely supported C
extension; it works on macOS clang.  Adding an explicit definition in one
`.c` file (e.g. `os9exec_nt.c`) would be cleaner and avoid relying on the
extension.

---

## 8. Minor — `fcalls.c:826, 844` — pointer arithmetic via `long` cast

```c
lim= (uint32_t *)( rp->d[1] + (long)ptr );  // line 826
rp->d[1]= (long)ptr - (long)FROM68K(rp->a[0]); // line 844
```

`ptr` is a host pointer; casting to `long` and adding/subtracting another
host pointer works because the arena is small (< 2 GB) and pointer differences
fit in 32 bits.  Using `ptrdiff_t` or `intptr_t` would be more idiomatic, but
the current code is correct for any plausible arena size.

---

## Summary

| # | Severity | File | Issue |
|---|----------|------|-------|
| 1 | **Critical** | `fileaccess.c:1657` | `ulong*` into 16-byte buffer — overflow + date corruption |
| 2 | **Significant** | `fcalls.c:937,954` | 8-byte NULL write into 4-byte arena field |
| 3 | Latent | `procstuff.c:377` | ISP host pointer truncated; UAE exceptions unsafe |
| 4 | Style | `fileaccess.c:1400,1432` | Misleading `ulong` widening before `uint32_t*` write |
| 5 | Note | `fcalls.c:839` | Acknowledged hack; host ptr as OS-9 field |
| 6 | Minor | `filestuff.c` | `vsprintf` with 500-byte buffer; no bound |
| 7 | Minor | `os9_ll.h` | `trapframebuf` — implicit tentative definition |
| 8 | Minor | `fcalls.c:826,844` | Pointer arithmetic via `long`; safe for small arena |

Issues 1 and 2 are the only ones that demonstrably corrupt data on arm64.
Issue 3 is latent and would need architectural thought to fix correctly.
Issues 4–8 are polish.
