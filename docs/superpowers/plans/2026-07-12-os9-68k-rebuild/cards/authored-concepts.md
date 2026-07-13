# Authored CONCEPT/ROSETTA cards

Written directly by Claude (not extracted by a subagent), drawing on this
project's accumulated hands-on OS-9 experience: live syscall/behavior
verification via `os9exec` + the REPL harness, real compiled-program
testing (`sizeof()`, module-header inspection), and cross-referencing
already-confirmed facts in `~/.claude/skills/os9-dev/references/common/
unix-differences.md`. `verify: authored` per the design spec's honesty
ledger — these are synthesis, not manual extraction, even where the
underlying fact has separately been manual-confirmed elsewhere in the pile.

--- CARD ---
id:        two-current-directories-not-one
type:      CONCEPT
target:    all
verify:    authored
topic:     directories
claim:     Unix has one current working directory. OS-9 has two, independently settable: the data directory (`chd`), which governs where relative *data* file paths resolve, and the execution directory (`chx`), which governs where relative *command names* resolve when something is run. They are genuinely separate state, not two names for the same thing.
context:   A program visible via `dir` (relative to the data directory) can still be "not found" when you try to run it by relative name — running is a command lookup, which resolves via the execution directory/PATH, not the data directory.
source:    authored
--- END ---

--- CARD ---
id:        chx-chd-resolution-is-open-mode-not-command
type:      CONCEPT
target:    all
verify:    authored
topic:     directories
claim:     The underlying rule unifying `chx` vs `chd` resolution is not "which command did you type" but "what mode was this specific file open performed in": a relative pathname resolves against the execution directory if the open is in execute mode, and against the data directory otherwise. A PATH lookup for a command to run is an execute-mode open (resolves via chx); typing a relative argument to the `chx` or `chd` command itself is a non-execute open (resolves via chd).
context:   This one rule explains two facts that look unrelated at first (PATH lookup uses chx; `chx somepath` itself resolves relative to chd) — it is a single, coherent, documented OS-9 principle (confirmed against the OS-9 v2.4 Technical Reference Manual's "Current Directories" material), not two separate quirks or an os9exec-specific invention. When behavior is confirmed only by live testing, it's still worth checking whether a manual states the same behavior as one clean rule — that's a stronger, more teachable answer than an empirically-derived rule of thumb.
source:    authored
--- END ---

--- CARD ---
id:        module-is-the-fundamental-unit
type:      CONCEPT
target:    all
verify:    authored
topic:     modules
claim:     In OS-9, the unit of loadable code and data is the memory module, not the file. Programs, device drivers, file managers, and libraries are all modules: a standard header (type, language, CRC, attributes) plus a body, tracked by name in an in-memory module directory. "Loading" means making a module known to the directory; "running" means forking a process from an already-known module.
context:   This is the single biggest mental shift from a Unix/ELF mental model: there is no fixed load address, no per-process private code copy (one resident module is shared reentrantly across every process running it, each with its own private data area), and a module can outlive the process that loaded it if flagged "sticky." `F$Link` finds an already-resident module and increments its link count; `F$Load` only reads from disk if the module isn't already resident.
source:    authored
--- END ---

--- CARD ---
id:        os9-line-ending-is-cr-not-lf
type:      GOTCHA
target:    all
verify:    authored
topic:     io
claim:     OS-9's line terminator is CR (0x0D), not LF (0x0A) — including in C source: the `\n` escape produces CR on this platform's C compiler, not linefeed. A separate escape is defined specifically to produce a true LF when one is genuinely needed.
context:   This is the most common host-boundary bug when moving files between a Unix host and OS-9: a Unix-authored (LF) source file can make the OS-9 C compiler read the entire file as one line, producing cascading syntax errors at spots that look nothing like the real problem. RBF stores bytes exactly with no transparent translation; conversion tools like `flip -m` only apply to host-side files, not to files already inside an RBF disk image — those must be edited natively (`vi`/`ed`), which already produce correct native line endings.
source:    authored
--- END ---

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

--- CARD ---
id:        control-keys-inverted-from-unix
type:      GOTCHA
target:    all
verify:    authored
topic:     shell
claim:     Several interactive control keys are inverted or unfamiliar relative to Unix habit: Ctrl-C backgrounds the foreground process (like Unix `&`) rather than killing it, and only works before the process has performed terminal I/O; Ctrl-E is the actual kill key; Ctrl-A redisplays the line rather than moving the cursor to line-start; flow control is Ctrl-W to pause / any key to resume, not Ctrl-S/Ctrl-Q; and shell EOF/exit is ESC on a blank line, not Ctrl-D.
context:   A related hazard: an arrow key sends an ANSI escape sequence that begins with a raw ESC byte, and the line editor treats a bare leading ESC as "exit" — this can make arrow-key input look like it randomly exits a shell or editor session. `tmode`/`xmode` let you remap these per-device; remapping EOF to Ctrl-D is one confirmed fix for the arrow-key hazard specifically.
source:    authored
--- END ---

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
id:        signals-are-numbers-carrying-data
type:      CONCEPT
target:    all
verify:    authored
topic:     ipc
claim:     OS-9 signals are plain numeric codes rather than a fixed named set: 0 is Kill and 1 is Wakeup (both uninterceptable), 2 is Abort (Ctrl-E) and 3 is Interrupt (Ctrl-C), and the wide range above that is user-definable — the numeric code itself can carry a small payload of meaning, not just identify which signal fired.
context:   A signal handler runs immediately and cannot itself be interrupted by another signal (the kernel masks signals for the duration) — the practical implication is that a handler must stay tiny and avoid blocking calls or sleeping; real work happens in the mainline under an explicit mask, woken by the signal, not inside the handler itself. Masking nests by increment/decrement rather than being a simple on/off flag, and clearing the mask to zero wipes all nesting at once, not just the caller's own increment.
source:    authored
--- END ---

--- CARD ---
id:        events-are-one-primitive-not-three
type:      CONCEPT
target:    all
verify:    authored
topic:     ipc
claim:     Where Unix/POSIX splits synchronization into separate mutex, condition-variable, and counting-semaphore primitives, OS-9 covers all three with a single mechanism: the event, which is a counter with processes able to block waiting for it to reach a target value. A binary mutex is just an event constrained to the range (1,1); a condition variable is a wait/signal pair on an ordinary event.
context:   Recognizing this collapses three separate Unix concepts into one OS-9 API (`F$Event`) to learn, rather than three.
source:    authored
--- END ---

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

--- CARD ---
id:        priority-aging-scheduler
type:      CONCEPT
target:    all
verify:    authored
topic:     scheduling
claim:     OS-9's scheduler is preemptive and priority-driven with a default short timeslice (2 ticks, ~20ms), but avoids simple priority starvation via aging: a process's effective scheduling weight is its priority plus how many context switches it has waited through, so the highest current age runs next. This means a low-priority process does eventually run instead of starving indefinitely under constant high-priority load.
context:   A cap (`D_MaxAge`) limits how much aging can boost the lowest-priority class specifically, so that a genuinely active high-priority process still gets nearly all of the CPU rather than being displaced purely by an aging low-priority process. Set process priority with the `setpr` command or the `F$SPrior` syscall.
source:    authored
--- END ---

--- CARD ---
id:        c-compiler-is-kr-not-ansi
type:      GOTCHA
target:    68k
verify:    authored
topic:     c-language
claim:     The OS-9 C compiler is a K&R-era compiler, not ANSI C: no `const`/`volatile` qualifiers, old-style (non-prototyped) function definitions only, no call-site argument type-checking, no `//` comments, no bit-fields, and `<strings.h>` instead of the ANSI `<string.h>` header name.
context:   `int`, `long`, and all pointer types are 32-bit on the 68k compiler — this was independently confirmed live via `sizeof()` on a real compiled program on this project's own 68k toolchain, correcting an earlier written claim (inherited by mistake from 6809-specific material) that `int` was 16-bit here.
source:    authored
--- END ---

--- CARD ---
id:        more-dots-not-more-dotdotslash
type:      GOTCHA
target:    all
verify:    authored
topic:     directories
claim:     Where Unix stacks `../../..` for each additional directory level up, OS-9 uses one additional dot per level instead: `...` means `../..` (two levels up) and `....` means `../../..` (three levels up). A single `..` (one level up) is unchanged from Unix.
context:   This has been verified for both the `dir` command and `chd`. On an RBF disk image, going "up" past the device root does not escape the filesystem — the root's parent is itself, a clamping behavior confirmed directly against the OS-9 v2.4 Technical Reference Manual's own directory-resolution text, not just inferred from testing.
source:    authored
--- END ---

--- CARD ---
id:        f-link-vs-f-load-module-directory-lifecycle
type:      CONCEPT
target:    all
verify:    authored
topic:     modules
claim:     `F$Link` searches the in-memory module directory for a module matching a given name and, if found, increments its link count rather than reloading anything from disk. `F$Load` instead reads a file from disk, adds whatever modules it contains to the module directory, and only then behaves like a link. `F$UnLink` decrements a module's link count and, once that count reaches zero, frees the module's memory and removes it from the directory (unless it's flagged "sticky," which keeps it resident regardless).
context:   This link-count model is how OS-9 shares one in-memory copy of a reentrant module (a program run by multiple processes at once, or a shared library) without duplicating it per process — the module directory is closer to a Unix dynamic linker's loaded-shared-object table than to anything in the ordinary process/file model.
source:    authored
--- END ---

--- CARD ---
id:        errno-not-cleared-on-success
type:      GOTCHA
target:    all
verify:    authored
topic:     error-handling
claim:     OS-9 C system calls signal failure by returning -1 and leaving the specific error code in `errno`. Critically, `errno` is not reset on a successful call — it retains whatever value was left by the most recent *failed* call.
context:   Correct usage always checks the call's own return value first and only consults `errno` when failure was actually indicated by that return value — checking `errno` unconditionally after any call, success or not, can report a stale error from an earlier, unrelated failure.
source:    authored
--- END ---

--- CARD ---
id:        path-number-vs-file-pointer-distinct-types
type:      GOTCHA
target:    all
verify:    authored
topic:     io
claim:     OS-9 C exposes two distinct handle types for an open file: the low-level "path number" returned by system calls like `open()`/`creat()`, and the high-level "file pointer" (`FILE *`) returned by `fopen()` and used by the standard buffered I/O functions. They are not interchangeable, even though a `FILE *` internally wraps a path number plus a buffer and status structure.
context:   Passing a file pointer where a system call expects a path number, or the reverse, is a well-known beginner mistake that reliably crashes the program rather than failing gracefully — worth calling out explicitly since Unix's analogous `int fd` vs `FILE *` distinction is more forgiving in casual misuse (though still technically wrong there too).
source:    authored
--- END ---

--- CARD ---
id:        basic09-memory-ceiling-execution-modifier
type:      GOTCHA
target:    all
verify:    VERIFIED
topic:     basic09-syntax
claim:     BASIC09 programs run with a default memory allocation that can be too small for anything non-trivial. The shell's `#<size>k` execution modifier (e.g. `basic09 #32k`) raises the memory given to the forked process — confirmed live: `basic09 #32k` launches cleanly where a larger BASIC09 program otherwise fails or behaves oddly under the default allocation.
context:   This is a general OS-9 shell mechanism (applies to forking any module with more memory than its default), not a BASIC09-specific flag — but it's most likely to bite a newcomer specifically when BASIC09 programs grow past trivial size.
source:    authored
--- END ---

--- CARD ---
id:        named-pipe-as-scratch-storage
type:      CONCEPT
target:    all
verify:    authored
topic:     ipc
claim:     A named pipe (`/pipe/<name>`) is a convenient scratch/temporary-data mechanism distinct from a plain file: data written to it persists until read once, at which point it disappears automatically — no explicit cleanup or deletion step is needed.
context:   This makes it a good lightweight choice for transient inter-process handoff during testing or one-shot data transfer, where creating and then remembering to delete a temporary file would be extra ceremony.
source:    authored
--- END ---
