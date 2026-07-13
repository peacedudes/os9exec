# Inter-process communication

19 raw cards in -> 13 kept, 0 merged (no true duplicates found), 6 pruned as mis-bucketed/generic (coloured-memory-allocation-calls [memory mgmt, not IPC], device-type-class-codes [device-manager architecture], esc-ctrl-eof-file-termination [terminal I/O, not IPC], io-devices-uniform-file-access [generic I/O-model restatement, duplicate of the EOF card's point], file-managers-standard [general file-manager roster, not an IPC mechanism itself], rbf-record-locking [RBF file-system locking, not process-to-process IPC]). Organized below by sub-topic: pipes, signals, events/alarms/semaphores. No shared-memory (data module) cards were present in the raw bucket.

## Pipes

--- CARD ---
id:        named-pipe-as-scratch-storage
type:      CONCEPT
target:    all
verify:    authored
topic:     pipes-ipc
claim:     A named pipe (`/pipe/<name>`) is a convenient scratch/temporary-data mechanism distinct from a plain file: data written to it persists until read once, at which point it disappears automatically — no explicit cleanup or deletion step is needed.
context:   This makes it a good lightweight choice for transient inter-process handoff during testing or one-shot data transfer, where creating and then remembering to delete a temporary file would be extra ceremony.
source:    authored
--- END ---

--- CARD ---
id:        pipe-unnamed-90-bytes
type:      FACT
target:    68k
verify:    from-manual
topic:     pipes-ipc
claim:     An unnamed pipe is 90 bytes by default when created without size specification; the size can be overridden via S_ISIZE option to _os_create().
context:   -
source:    The OS-9 Primer, Chapter 8: "Pipe sizes"
--- END ---

--- CARD ---
id:        pipe-operator-filter-chain
type:      FACT
target:    68k
verify:    from-manual
topic:     pipes-ipc
claim:     ! (exclamation mark) operator constructs pipes; allows output of one program to become input of next program in a chain (filter construction).
context:   Pipes enable complex data processing workflows.
source:    Using Professional OS-9 v2.4, "The Shell", Shell Command Line Processing section, p. 5-8
--- END ---

--- CARD ---
id:        pipe-close-memory-unnamed-named
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     pipes-ipc
claim:     Unnamed pipe close: path count decrements; no paths remaining → memory returned to system. Named pipe close: path count decrements; non-empty named pipe kept open (waiting for reader), memory returned only when empty.
context:   Named pipe persistence allows deferred reader access.
source:    OS-9 v2.4 Technical Reference Manual, "Close", p. 4-14
--- END ---

## Signals

--- CARD ---
id:        signal-mask-discipline
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     signals-ipc
claim:     Masking signals works even when sleeping (F$Sleep unmasks signals internally). A sequence of _os_sigmask(1); sleep(0) is a valid pattern for signal-driven programs
context:   -
source:    OS-9 Insights §8.2 "Masking Signals"
--- END ---

--- CARD ---
id:        signal-nesting-counter-overflow
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     signals-ipc
claim:     A program mixing longjmp() and F$RTE exits without calling F$SigReset can theoretically overflow the signal intercept nesting counter; this requires ~4 billion longjmp() calls without corresponding F$SigReset calls
context:   Remote but possible edge case
source:    OS-9 Insights §8.6 "An Obscure Problem"
--- END ---

--- CARD ---
id:        signal-async-safety-strategies
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     signals-ipc
claim:     Three async-safe strategies exist: use async-safe algorithms, mask signals during mainline data structure updates, or use single-instruction atomic operations (move, addq, subq, bset, bclr, tas)
context:   68k adds cas (compare-and-swap) for singly-linked updates and cas2 for doubly-linked; these do the final part atomically
source:    OS-9 Insights §8.7 "Async Safety"
--- END ---

--- CARD ---
id:        signal-intercept-d0
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     signals-ipc
claim:     Kernel sets d0 to the number of queued signals when calling intercept routine (including the one being delivered); value of 1 means no waiting signals
context:   -
source:    OS-9 Insights §8.1 "An Undocumented Feature"
--- END ---

## Events, alarms, semaphores

--- CARD ---
id:        semaphore-types
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     semaphores-ipc
claim:     Binary semaphores come in three varieties: busy-waiting (active loop), sleeping (F$Sleep), and event-based; OS-9 v3.0 adds F$Sema system call for native support
context:   -
source:    OS-9 Insights §12 "Binary Semaphores"
--- END ---

--- CARD ---
id:        event-functions-via-f-event
type:      FACT
target:    68k
verify:    from-manual
topic:     events-ipc
claim:     F$Event system call functions (resolve via funcs.a/sys.l/usr.l): Ev$Link, Ev$UnLnk, Ev$Creat, Ev$Delet, Ev$Wait, Ev$WaitR, Ev$Read, Ev$Info, Ev$Pulse, Ev$Signl, Ev$Set, Ev$SetR.
context:   All event operations accessible through single F$Event syscall to minimize kernel interface.
source:    OS-9 v2.4 Technical Reference Manual, Figure 4-10, p. 4-10
--- END ---

--- CARD ---
id:        alarm-user-state-types
type:      FACT
target:    68k
verify:    from-manual
topic:     alarms-ipc
claim:     User-state alarm functions via F$Alarm: A$Delete (remove pending), A$Set (signal after interval), A$Cycle (signal periodically), A$AtDate (signal at Gregorian datetime), A$AtJul (signal at Julian datetime).
context:   Signals sent to requesting process; kernel handles timing.
source:    OS-9 v2.4 Technical Reference Manual, "User-state Alarms", p. 4-4
--- END ---

--- CARD ---
id:        cyclic-alarm-time-base
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     alarms-ipc
claim:     Cyclic alarm provides time base within program, synchronizing time-dependent tasks. Signal identifier specifies which display/function to update. System handles all timing; handler reads sensor/updates display.
context:   Useful for real-time applications (games, simulations, dashboards) needing multiple time bases; more efficient than polling.
source:    OS-9 v2.4 Technical Reference Manual, "Cyclic Alarms", p. 4-4
--- END ---

--- CARD ---
id:        relative-time-alarm-timeout
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     alarms-ipc
claim:     Relative time alarm sets time limit for specific action. Common use: send keyboard abort signal at max time, issue I$Read; if alarm arrives before input, Read returns error; otherwise cancel alarm.
context:   Useful for implementing read timeouts without complex state machines.
source:    OS-9 v2.4 Technical Reference Manual, "Relative Time Alarms", p. 4-5
--- END ---
