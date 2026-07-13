# Mental model and Unix/Linux Rosetta

Sifted/merged from `mental-model-rosetta-RAW.md` (8 cards) plus one card
pulled from `assembly-RAW.md` (`system-call-vs-utility-programs`, a genuine
mental-model point mis-bucketed by keyword match on "processor"). This is
the eager-tier "connective tissue" content — the delta between what a
model already assumes and how OS-9 actually works — not raw extracted
facts, and not subject to the same prune-only-generic-restatements test as
other clusters since every surviving card here already carries real delta.

9 cards in, 6 out. Dropped: `ro` (guru-book.md — a malformed/truncated id
and a genuine duplicate of the two authored fork/shared-memory cards
already present, adding nothing new); `trap-vs-subroutine` (os9-primer.md
— compares OS-9000 to OS-9/68k; OS-9000 is out of scope for this skill);
`fastrak-cross-dev` (os9-primer.md — a historical cross-development-tool
fact with no practical value for an AI agent working against this
project's actual toolchain, which is `os9exec`, not FasTrak).

---

## Process model

--- CARD ---
id:        fork-does-not-share-memory
type:      ROSETTA
target:    all
verify:    authored
topic:     process-mgmt
claim:     Unix `fork()` conceptually clones the parent's address space (historically copy-on-write). OS-9's `F$Fork` does not: the child gets a fully independent data area built fresh from the named module's header, with no shared pages and no copy-on-write mechanism at all. The child inherits open paths, current directories, and environment, but never memory content — any data the child needs must be passed explicitly (via the parameter area) or shared deliberately through a named IPC mechanism (pipe, data module, event, signal).
context:   `F$Chain` is the closer analog to Unix's fork-then-exec pattern fused into one call: it reuses the current process (no new PID) and reconfigures its data area in place, while preserving already-open paths — unlike a real Unix `exec()`, which typically leaves open file descriptors alone but doesn't have the "reconfigure in place, same PID" framing to begin with.
source:    authored
--- END ---

--- CARD ---
id:        system-state-vs-user-state
type:      CONCEPT
target:    all
verify:    from-manual
topic:     kernel-concepts
claim:     OS-9 distinguishes system state (kernel/supervisor mode) from user state (application mode); system calls are the transition point between them.
context:   Roughly analogous to Unix's kernel-mode/user-mode split, but on 68k this maps directly onto the CPU's own supervisor/user bit (confirmed live against a real compiled module's header) rather than being purely an OS-9 software convention.
source:    OS-9 Insights, Chapter 22 "Security"
--- END ---

--- CARD ---
id:        system-call-vs-utility-programs
type:      CONCEPT
target:    all
verify:    from-manual
topic:     programming-interface
claim:     OS-9 capabilities are reachable two ways: through the shell's utility command set (user-facing, interactive) or directly via system calls from within a program written in assembler or a high-level language (the programmatic interface used for process creation, file operations, and everything else).
context:   Useful framing for an AI agent: a task solvable interactively with a shell command and a task requiring a compiled program calling F$/I$ syscalls directly are two different levels of the same capability set, not two unrelated subsystems.
source:    Using Professional OS-9 v2.4, "An Overview of OS-9", p. 1-2
--- END ---

## IPC and shared state

--- CARD ---
id:        shared-memory-is-a-named-data-module
type:      ROSETTA
target:    all
verify:    authored
topic:     ipc
claim:     Unix shared memory (`shmget`/`mmap` with `MAP_SHARED`) has a direct OS-9 analog: a data module, created via `F$DatMod`, is a named, shared, mutable memory region that multiple processes access through ordinary pointer addressing once linked to it.
context:   As with any shared-memory mechanism, there is no implicit atomicity — concurrent access must be synchronized explicitly (typically with an event or signal), the same discipline Unix shared memory requires.
source:    authored
--- END ---

## Shell

--- CARD ---
id:        pipe-operator-is-bang-not-pipe
type:      ROSETTA
target:    all
verify:    authored
topic:     shell
claim:     Unix's `|` pipe operator has no OS-9 equivalent at all — OS-9's shell uses `!` for the same purpose (`cmd1 ! cmd2`, chainable as `a ! b ! c`). Muscle memory or copy-pasted scripts using `|` fail silently on OS-9 rather than producing an error, because `|` simply isn't a shell metacharacter there.
context:   Redirection symbols also differ in scope from Unix (`>`, `>>`, plus OS-9-specific append/create variants) — check the actual shell syntax rather than assuming Unix redirection carries over unchanged.
source:    authored
--- END ---

## File I/O

--- CARD ---
id:        mknod-creates-directory-not-device
type:      ROSETTA
target:    all
verify:    from-manual
topic:     io
claim:     Unlike Unix's mknod() (which creates a general-purpose special/device file), OS-9's C library mknod() call only creates a new directory. Ordinary files are created with creat(), not mknod().
context:   A programmer coming from Unix should not expect mknod() to create device nodes on OS-9 — the name is shared but the semantics are OS-9's own (directory creation).
source:    OS-9 C Compiler manual, "Mknod", p. 3-22 to 3-23
--- END ---
