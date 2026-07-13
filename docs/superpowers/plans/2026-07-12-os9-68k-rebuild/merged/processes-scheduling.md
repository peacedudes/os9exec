# Process management and scheduling

Sifted/merged from `processes-scheduling-RAW.md` (198 cards) plus one stray card
pulled in from `kernel-and-syscalls-RAW.md` (`atomic-kernel-minimal`) — 199 raw
cards considered. Roughly 75 were consolidated into ~31 merged cards (true
duplicates from different source documents describing the same fact), and
about 73 were pruned outright: mis-bucketed BASIC09-syntax cards (~20),
mis-bucketed RBF record-locking cards (~12), mis-bucketed file-manager/device-driver
architecture cards (~16), generic/no-delta restatements (~10), and a handful of
off-topic singles (C-compiler toolchain names, OS-9 history trivia, assembly
calling conventions, terminal-type env var, etc.). Result: ~82 cards below,
grouped by sub-topic. No genuine 6809-vs-68k factual conflict turned up in this
cluster — the surviving raw cards are either architecture-general (`all`) or
68k-specific by nature (register conventions, F$DFork family), not competing
descriptions of the same mechanism on two CPUs.

One genuine cross-source *disagreement* (not a 6809/68k split) was preserved
rather than silently resolved: two manuals disagree on which shell special
character sets process priority at fork time (see `shell-special-characters`
and `shell-priority-modifier-char`).

---

## Process fundamentals

--- CARD ---
id:        process-definition
type:      CONCEPT
target:    all
verify:    from-manual
topic:     process-management
claim:     A process is a forked program that has not exited, together with its data memory and process descriptor (a kernel structure). A single re-entrant program module can have multiple simultaneous incarnations, each a separate process with its own ID and data area.
context:   -
source:    The OS-9 Guru, section 3.5, page 57
--- END ---

--- CARD ---
id:        process-id
type:      FACT
target:    all
verify:    from-manual
topic:     process-management
claim:     Each process is assigned a unique Process ID (PID) by the kernel at creation time (returned by F$Fork/F$DFork). PIDs are always greater than 1 (0 is unused, 1 is the system process); after a process dies its ID may be reused, but no two processes ever share an ID simultaneously.
context:   PID is the handle used by every system call that targets or communicates with a specific process.
source:    The OS-9 Guru §3.5 p.57 + the independent 1985-era OS-9/68000 technical manual §4-2 "Process Creation" + The OS-9 Primer, Ch.7 "Process identification"
--- END ---

--- CARD ---
id:        process-priority
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     scheduling
claim:     Each process has a priority value, assigned at fork time (usually inherited from the parent, stored in d2.w when a child is created) and changeable later via F$SPrior. Priority is the primary input to the scheduler's CPU-time allocation, but only relative values matter — a priority of 50 behaves identically to 5000 if every other process is scaled the same way.
context:   The specific d2.w register convention is 68k-only; the priority concept itself is architecture-general.
source:    The OS-9 Guru §3.5 p.57 + the independent 1985-era OS-9/68000 technical manual §4-1 + OS-9 Insights §7.5 "Tuning Your Operation"
--- END ---

--- CARD ---
id:        process-states-overview
type:      FACT
target:    all
verify:    from-manual
topic:     process-management
claim:     A process can be in one of: Active (requesting processor time), Waiting (for a child process), Sleeping (for a time interval, event, or IPC), Waiting-for-event, Debugged (under debugger control), or Dead (reporting its exit status to a waiting parent). Only Active processes receive processor time.
context:   The Active/Waiting/Sleeping trio are the three scheduler-visible queues; Debugged and Dead are special-case states layered on top (see F$DFork family and F$Wait below).
source:    The OS-9 Guru, section 3.5, page 57
--- END ---

--- CARD ---
id:        process-state-active
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     scheduling
claim:     The ACTIVE state covers all executable processes. They sit in a queue sorted by age and receive time slices according to their relative priority against every other active process; the age-comparison algorithm ensures even very low-priority active processes eventually get CPU time.
context:   -
source:    the independent 1985-era OS-9/68000 technical manual §4-5 "Process States" + OS-9 v2.4 Technical Reference Manual, "Process States", p. 2-25
--- END ---

--- CARD ---
id:        process-state-waiting
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     scheduling
claim:     The WAITING state is entered via the F$Wait system call. The process stays inactive until one of its descendant processes terminates or it receives a signal — this is the mechanism a parent uses to block for a child's exit status.
context:   -
source:    the independent 1985-era OS-9/68000 technical manual §4-5 "Process States" + OS-9 v2.4 Technical Reference Manual, "Process States", p. 2-25
--- END ---

--- CARD ---
id:        process-state-sleeping
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     scheduling
claim:     The SLEEPING state is entered via F$Sleep, specifying a time interval; the process resumes when the interval elapses or it receives a signal. A zero interval means sleep indefinitely (until signaled). This lets a process avoid burning CPU while waiting on an external event.
context:   -
source:    the independent 1985-era OS-9/68000 technical manual §4-5 "Process States" + OS-9 v2.4 Technical Reference Manual, "Process States", p. 2-25
--- END ---

--- CARD ---
id:        process-descriptor
type:      CONCEPT
target:    all
verify:    from-manual
topic:     process-management
claim:     Every process has a kernel-maintained Process Descriptor tracking its state, memory allocation, priority, I/O paths, signals, and other administrative data. It is automatically created and maintained by OS-9; user programs never access it directly and don't need to be aware of its existence.
context:   -
source:    the independent 1985-era OS-9/68000 technical manual §4-2 "Process Creation" + The OS-9 Primer, Ch.7 "Process Descriptors"
--- END ---

--- CARD ---
id:        process-descriptor-table
type:      FACT
target:    all
verify:    from-manual
topic:     process-management
claim:     The process descriptor table is an array of process-descriptor addresses; a process's ID is simply its index into this table (a zero entry means that ID is unused). The table starts small and the kernel doubles its size when it fills up.
context:   -
source:    The OS-9 Guru, section 3.5, page 59
--- END ---

--- CARD ---
id:        process-group-user-id
type:      FACT
target:    68k
verify:    from-manual
topic:     security
claim:     Each process carries a Group ID and User ID inherited from its parent (passed in d1.l when the child is created via F$Fork/F$DFork). These IDs identify ownership of the process and any files it creates, and are the basis for file/system security checks.
context:   -
source:    the independent 1985-era OS-9/68000 technical manual §4-2 "Process Creation"
--- END ---

--- CARD ---
id:        security-user-group
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     security
claim:     Each process's user/group numbers come from the password file at login. The super user (group 0) has unrestricted access to system resources regardless of individual file/process ownership.
context:   -
source:    OS-9 Insights §22 "Security"
--- END ---

---

## Process creation and termination

--- CARD ---
id:        f-fork-basics
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     process-management
claim:     F$Fork creates a child process that inherits the parent's priority, memory model, open I/O paths, and user/group IDs; parent and child then execute independently and concurrently. At the C level this is wrapped as _os_fork().
context:   A new process can only be created by an existing one — there is no bootstrap path around F$Fork/F$DFork for ordinary processes.
source:    OS-9 Insights §7.2 "Family Relationships" + Using Professional OS-9 v2.4, "I/O Redirection Modifiers", p. 5-10 + The OS-9 Primer, Ch.7 "Creating child tasks"
--- END ---

--- CARD ---
id:        f-fork-concurrency-gotcha
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     process-management
claim:     Both parent and child run concurrently right after F$Fork — if the parent immediately calls F$Wait it blocks until the child dies, but a child process descriptor is only reclaimed/returned once the parent actually does that F$Wait. Recursively invoking a program that itself calls F$Fork risks creating a new child on every incarnation until the process table fills.
context:   -
source:    the independent 1985-era OS-9/68000 technical manual, §14-14 "F$Fork Caveats"
--- END ---

--- CARD ---
id:        fork-parameter-passing
type:      FACT
target:    68k
verify:    from-manual
topic:     process-management
claim:     A parent passes a parameter string to its child via F$Fork; the kernel copies it to the top of the child's data space (static storage), combined with the parent's environment variables. The shell appends an end-of-line character to the parameter string it builds, simplifying string-oriented parsing; parameter size is passed in d5.l at creation time.
context:   This is the kernel-level mechanism underneath the C runtime's argc/argv construction (see `c-argv-quoting`).
source:    The OS-9 Guru §3.5 p.58 + the independent 1985-era OS-9/68000 technical manual §14-14 "F$Fork"
--- END ---

--- CARD ---
id:        c-argv-quoting
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     process-management
claim:     The C startup routine converts OS-9's raw parameter string into a conventional null-terminated argv array for main(argc, argv). It also runs together as one argument any text enclosed in matching single or double quotes; if a quote character is needed literally inside a quoted string, use the other quote type as the delimiter.
context:   This quote-joining is a C-runtime convenience layered on top of the kernel's raw parameter string, not something the OS-9 kernel itself does.
source:    OS-9 C Compiler manual, "Access to Command Line Parameters", p. 1-6
--- END ---

--- CARD ---
id:        f-chain
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     process-management
claim:     F$Chain transforms the calling process into a new process executing a different program module: it terminates the caller and reuses its process descriptor and ID for the new program, rather than allocating a fresh ID the way F$Fork does. This saves memory versus fork+exit, but only one process exists after the call (no parent survives it).
context:   -
source:    The OS-9 Guru §3.5 p.59 + OS-9 Insights §7.4 "Chaining New Processes"
--- END ---

--- CARD ---
id:        process-disinheritance
type:      GOTCHA
target:    all
verify:    from-manual
topic:     process-management
claim:     A process inherits its parent at fork time; if the parent dies before the child, the child becomes disinherited (it does not become a grandchild of the original grandparent). There is no system call to disinherit a child programmatically — the workaround is to fork an intermediate process that itself forks the desired child and then immediately dies.
context:   -
source:    The OS-9 Guru, section 3.5.1, page 60
--- END ---

--- CARD ---
id:        fork-size-modifier
type:      GOTCHA
target:    all
verify:    VERIFIED
topic:     process-management
claim:     BASIC09 programs (and any forked module) run with a default memory allocation that can be too small for anything non-trivial. The shell's `#<size>k` execution modifier (e.g. `basic09 #32k`) raises the memory given to the forked process — confirmed live: `basic09 #32k` launches cleanly where a larger BASIC09 program otherwise fails or behaves oddly under the default allocation.
context:   This is a general OS-9 shell mechanism for forking any module with more memory than its module-declared default, not a BASIC09-specific flag — it's just most likely to bite a newcomer running BASIC09 programs past trivial size.
source:    authored
--- END ---

--- CARD ---
id:        sbrk-ibrk-memory-request
type:      FACT
target:    all
verify:    from-manual
topic:     process-management
claim:     sbrk() requests new memory for a running process from *outside* its initial allocation (going back to the OS for more); ibrk() instead requests memory that still fits *inside* the process's already-granted initial allocation. Both return -1 if the requested contiguous amount cannot be granted.
context:   sbrk() failing does not necessarily mean total system memory is exhausted — it can fail if OS-9 declines to grant more for any reason.
source:    OS-9 C Compiler manual, "Sbrk,Ibrk", p. 3-33; "Memory Management", p. 2-5
--- END ---

--- CARD ---
id:        wait-exit-implicit-status
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     process-management
claim:     wait() blocks the caller until a child terminates, then returns the terminated child's ID and places its exit status into the caller-supplied status pointer. That status is either the argument passed to the child's exit()/_exit() call, or the terminating signal number. A C program that falls off the end of main() without calling exit() is treated as an implicit exit(0).
context:   A wait() must be executed per forked child, or the parent never reaps that child's status.
source:    OS-9 C Compiler manual, "Wait", p. 3-44; "Exit,_Exit", p. 3-14
--- END ---

--- CARD ---
id:        abort-core-dump
type:      FACT
target:    all
verify:    from-manual
topic:     process-management
claim:     Calling abort() writes a memory image of the current process to a file named "core" in the current data directory, then exits with status 1 — OS-9's analogue to the Unix core-dump convention.
context:   -
source:    OS-9 C Compiler manual, "Abort", p. 3-2
--- END ---

--- CARD ---
id:        system-call-max-string-length
type:      FACT
target:    all
verify:    from-manual
topic:     process-management
claim:     system() hands its string argument to the OS-9 Shell for execution as a command line, blocking until the shell command completes and returning its exit status. The maximum command-string length system() accepts is 80 characters; longer command lines require os9fork() instead.
context:   -
source:    OS-9 C Compiler manual, "System", p. 4-31
--- END ---

---

## Debugger-controlled processes (F$DFork family)

--- CARD ---
id:        f-dfork-creation
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     debugging
claim:     F$DFork creates a child process under debugger control, similar to F$Fork but: (1) the child is left "suspended" rather than placed in the active queue; (2) its status register trace bit is set; (3) execution is driven through F$DExec/F$DExit; (4) a permanent register buffer in the caller's data area is bound to the child and holds its initial register image.
context:   Input registers: d0.w=module type/revision (0=any), d1.l=extra stack, d2.l=parameter size, d3.w=# I/O paths, d4.w=module priority, (a0)=module name, (a1)=parameter ptr, (a2)=register buffer. Output: d0.w=child PID, (a2)=initial register image.
source:    the independent 1985-era OS-9/68000 technical manual, §14-11 "F$DFork"
--- END ---

--- CARD ---
id:        f-dfork-never-executes-alone
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     debugging
claim:     A process created by F$DFork never executes at all unless explicitly told to via F$DExec. The trace bit set on it fires the system trace exception once per user instruction, making the debugged program run very slowly.
context:   Direct consequence of the suspended-state/trace-bit design for debugger control.
source:    the independent 1985-era OS-9/68000 technical manual, §14-11 "F$DFork"
--- END ---

--- CARD ---
id:        f-dexec-control
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     debugging
claim:     F$DExec drives execution of a suspended F$DFork child. Execution stops when: (1) the requested instruction count is reached, (2) a breakpoint hits, or (3) an unexpected exception occurs. System calls the suspended program makes run at full speed and count as a single logical instruction.
context:   Input: d0.w=child PID, d1.l=# instructions (0=continuous), d2.w=# breakpoints, (a0)=breakpoint list. Output: d0.l=instructions executed, d1.l=remaining count, d2.w=exception offset, d3.w=classification word, d4.l=access address, d5.w=instruction register; carry set on error.
source:    the independent 1985-era OS-9/68000 technical manual, §14-8 "F$DExec"
--- END ---

--- CARD ---
id:        f-dexec-register-buffer
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     debugging
claim:     The register buffer bound at F$DFork time is used to save/restore the child's registers on every F$DExec call; a debugger editing that buffer directly changes the child's registers the next time it resumes.
context:   -
source:    the independent 1985-era OS-9/68000 technical manual, §14-8 "F$DExec"
--- END ---

--- CARD ---
id:        f-dexec-trace-through-traps
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     debugging
claim:     F$DExec tracing is allowed through user trap handlers, intercept routines, and F$Chain — deliberately, so a debugger can single-step through code that calls trap handlers or chains to another program, even though it may seem surprising.
context:   -
source:    the independent 1985-era OS-9/68000 technical manual, §14-8 "F$DExec Caveats"
--- END ---

--- CARD ---
id:        f-dexit
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     debugging
claim:     F$DExit terminates a suspended F$DFork child. Unlike normal termination, it does not release the child's resources, so a debugger can perform post-mortem examination afterward.
context:   Input: d0.w=child PID to terminate. Possible error: E$IPrcID.
source:    the independent 1985-era OS-9/68000 technical manual, §14-10 "F$DExit"
--- END ---

---

## Scheduler and timeslicing

--- CARD ---
id:        priority-aging-scheduler
type:      CONCEPT
target:    all
verify:    authored
topic:     scheduling
claim:     OS-9's scheduler is preemptive and priority-driven with a default short timeslice (2 ticks, ~20ms), but avoids simple priority starvation via aging: a process's effective scheduling weight is its priority plus how many context switches it has waited through, so the process with the highest current age runs next. This means a low-priority process does eventually run instead of starving indefinitely under constant high-priority load.
context:   A cap (`D_MaxAge`) limits how much aging can boost the lowest-priority class specifically, so a genuinely active high-priority process still gets nearly all of the CPU rather than being displaced purely by an aging low-priority process. Set process priority with the `setpr` command or the `F$SPrior` syscall.
source:    authored
--- END ---

--- CARD ---
id:        scheduler-age-algorithm
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     scheduling
claim:     The active queue is kept sorted by age. When a process enters the queue its age is initialized to its assigned priority; every time a new process enters the active queue, every other active process's age is incremented. The scheduler always runs the process with the highest age next (the oldest process sits at the queue head), so every active process eventually gets CPU time. Ages are capped at $FFFF (65535) and never incremented past it.
context:   -
source:    the independent 1985-era OS-9/68000 technical manual §4-6 "Execution Scheduling" + OS-9 v2.4 Technical Reference Manual, "Process Scheduling", p. 2-27
--- END ---

--- CARD ---
id:        d-minpty-priority-threshold
type:      FACT
target:    68k
verify:    from-manual
topic:     scheduling
claim:     D_MinPty is a system global that sets a minimum priority below which processes are neither aged nor considered for execution. Setting it above zero freezes every process running below that level while the requesting (presumably critical) task runs to completion — combined with signal masking, this gives deterministic real-time behavior for that task.
context:   D_MinPty is usually zero. If set above all running tasks' priorities the system halts completely and needs a reset to recover — restore it to normal promptly after the critical task finishes. Accessible via F$SetSys (superuser only).
source:    the independent 1985-era OS-9/68000 technical manual §4-6 "Pre-emptive Task Switching" + OS-9 Insights §8.2
--- END ---

--- CARD ---
id:        d-maxage-task-class-division
type:      FACT
target:    68k
verify:    from-manual
topic:     scheduling
claim:     D_MaxAge is a system global that caps how high a process's age can climb. When non-zero it effectively splits processes into two classes: low-priority tasks stop aging at the MaxAge cutoff, while high-priority tasks receive essentially all available CPU time; low-priority tasks only run when no high-priority task is active.
context:   Usually zero. Both D_MinPty and D_MaxAge are superuser-accessible via F$SetSys.
source:    the independent 1985-era OS-9/68000 technical manual, §4-6 "Pre-emptive Task Switching"
--- END ---

--- CARD ---
id:        timeslice-tick-duration
type:      FACT
target:    68k
verify:    from-manual
topic:     scheduling
claim:     A hardware real-time clock interrupts the CPU at a regular rate, typically 100 times/second; each such basic interval is a "tick" (normally 10ms). A time slice is normally two ticks (~20ms) rather than one, because OS-9 cannot resolve sub-tick time and a single-tick slice would be rounded to nearly nothing.
context:   The tick rate is system-configurable; 100Hz/10ms is the typical default.
source:    The OS-9 Guru §3.5 p.58 + the independent 1985-era OS-9/68000 technical manual §4-1 "Overview of Multitasking"
--- END ---

--- CARD ---
id:        preemptive-multitasking-concept
type:      CONCEPT
target:    all
verify:    from-manual
topic:     scheduling
claim:     OS-9 is a preemptive, time-sliced multitasking system: it runs one program for a short duration, saves its state, and switches to the next, repeating rapidly enough (many times per second) that concurrent execution appears simultaneous to the user. A higher-priority process becoming ready preempts whatever lower-priority process is currently running.
context:   The slice length is a tradeoff: short enough for acceptable concurrency, long enough that scheduler overhead doesn't dominate.
source:    The OS-9 Guru §1.3 p.3 and §3.5 pp.58-59 + The OS-9 Primer, Ch.11 "Scheduling and preemption" + Using Professional OS-9 v2.4, "Multi-tasking", pp. 1-4 to 1-5
--- END ---

--- CARD ---
id:        round-robin-same-priority
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     scheduling
claim:     Processes that share the same priority divide CPU time round-robin: each runs for a time slice, then yields to the next ready process at that same priority level.
context:   -
source:    The OS-9 Primer, Chapter 11: "Scheduling algorithm"
--- END ---

--- CARD ---
id:        system-state-not-preempted
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     scheduling
claim:     A process executing in system state (e.g., inside a system call) is not pre-empted until it finishes or voluntarily yields its time slice — even if a higher-priority process becomes active in the meantime. This exists because system-state code may be mid-way through a critical section touching shared kernel resources, and interrupting it there could corrupt state or block unrelated processes.
context:   Time-slicing applies normally to user-state processes and to system-state code that explicitly sleeps or yields.
source:    the independent 1985-era OS-9/68000 technical manual §4-6 "Exception And Interrupt Processing" + The OS-9 Guru §3.5.2 p.59 + OS-9 v2.4 Technical Reference Manual, "Pre-emptive Task-switching", p. 2-28
--- END ---

---

## Privilege states (user vs. system state)

--- CARD ---
id:        user-vs-system-state
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     privilege
claim:     The 68000 has two operating states, and OS-9 maps its own process model onto them: user state (application programs — no direct hardware access, time-sliced, memory-protected, can be aborted, certain instructions forbidden) and system state/supervisor state (OS components — full processor access, no time-slicing once entered, direct hardware control, non-maskable interrupts possible, and correspondingly dangerous: a bug here can crash the whole system). System calls and interrupts automatically enter supervisor state.
context:   On the 68000 family, "system state" and "supervisor mode" are synonymous. Memory-protection hardware, where present, can further restrict what a user-state program's memory accesses are allowed to touch.
source:    The OS-9 Guru §3.2.8 p.41 and §3.5.2 p.59 + OS-9 v2.4 Technical Reference Manual, "User-state and System-state", p. 2-2
--- END ---

---

## Exceptions and interrupts

--- CARD ---
id:        irq-vectored-interrupts
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions
claim:     Exception vectors 64-255 provide 192 vectored interrupts; multiple devices may share a vector. F$IRQ installs a handler into the system's interrupt tables. Interrupt service routines run in system state with no associated current process, and a device driver must supply an error entry point for the system to call if an error exception occurs during interrupt processing.
context:   -
source:    the independent 1985-era OS-9/68000 technical manual §4-10 "Vectored Interrupts"
--- END ---

--- CARD ---
id:        error-exceptions-strap
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     exceptions
claim:     Error exceptions (vectors 2-8, 10-24, 48-63) are normally fatal and terminate the process unconditionally. F$STrap (wrapped at the C level as _os_strap()) installs a user subroutine to catch exceptions in this group as non-fatal instead: when one occurs in user state, the handler runs with a pointer to the process's normal data space and all user registers stacked, and it decides whether/where to resume. A process without such a handler is simply terminated (except that a process created via F$DFork keeps its resources intact afterward, for post-mortem debugger examination).
context:   If an error exception occurs in system state, it usually means a system call was passed bad data; an error is returned rather than crashing outright, though nonsense parameters can still damage system data structures.
source:    the independent 1985-era OS-9/68000 technical manual §4-7/§4-8 "Error Exceptions" + OS-9 v2.4 Technical Reference Manual, "Error Exceptions", p. 2-31 + The OS-9 Primer, Ch.14 "Catching hardware exceptions"
--- END ---

---

## Signals

--- CARD ---
id:        signal-concept
type:      CONCEPT
target:    all
verify:    from-manual
topic:     signals
claim:     Signals are a narrow, asynchronous communication channel between processes: software interrupts one process sends another, which can fire after any instruction when unmasked and either indicate an exceptional condition or serve as general-purpose inter-process notification/control.
context:   -
source:    OS-9 Insights §8 Introduction + The OS-9 Primer, Ch.9 "Signals and alarms"
--- END ---

--- CARD ---
id:        signal-intercept-dispatch
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     signals
claim:     If the target process is active when a signal arrives, its intercept routine runs immediately; if it's sleeping/waiting, the process moves to the active queue, the intercept routine runs, and the original wait/sleep is resumed afterward via a queued call. A process with no intercept handler for a given signal (other than signals 0 and 1) is simply killed.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, "Signals", p. 4-2
--- END ---

--- CARD ---
id:        signal-mask-nesting
type:      FACT
target:    68k
verify:    from-manual
topic:     signals
claim:     The signal mask level P$SigLvl in the process descriptor is an unsigned byte; while nonzero, the intercept routine is not called. F$SigMask with d1=1 increments it, d1=-1 decrements it, d1=0 clears it to zero. Overflow past 255 or underflow past 0 is silently ignored.
context:   -
source:    OS-9 Insights §8.2 "Masking Signals"
--- END ---

--- CARD ---
id:        signal-intercept-state-storage
type:      FACT
target:    68k
verify:    from-manual
topic:     signals
claim:     Entering a signal intercept routine automatically masks signals (P$SigLvl set to 1). The non-recursive (fast-path) case stores one saved state in system state; recursive intercept calls instead store their state on the user stack.
context:   -
source:    OS-9 Insights §8.5 "The Signal-State Stack"
--- END ---

--- CARD ---
id:        signal-state-size
type:      FACT
target:    68k
verify:    from-manual
topic:     signals
claim:     The process state saved for a signal intercept routine covers all MPU registers (72 bytes), plus FPU registers if the FPU is active (168 bytes total combined with the CPU state).
context:   -
source:    OS-9 Insights §8.5 footnote
--- END ---

--- CARD ---
id:        signal-send-restrictions
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     signals
claim:     Most signals can be sent to any process, but the kill signal is restricted to same user/group only; the super user (group 0) can send kill to any process.
context:   -
source:    OS-9 Insights §8 Introduction
--- END ---

--- CARD ---
id:        signal-broadcast
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     signals
claim:     Sending a signal to process 0 broadcasts it to every process sharing the sender's user/group, excluding the sender itself.
context:   -
source:    OS-9 Insights §8.8 "Broadcast"
--- END ---

--- CARD ---
id:        signal-queue-behavior
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     signals
claim:     OS-9 does not discard a signal sent to a process that already has one pending — it queues signals and delivers them in the order sent. Queued signal delivery can be roughly 10x more expensive than unqueued.
context:   -
source:    OS-9 Insights §8.9 "Queued Signals"
--- END ---

--- CARD ---
id:        control-keys-inverted-from-unix
type:      GOTCHA
target:    all
verify:    authored
topic:     signals
claim:     Several interactive control keys are inverted or unfamiliar relative to Unix habit: Ctrl-C backgrounds the foreground process (like Unix `&`) rather than killing it, and only works before the process has performed terminal I/O; Ctrl-E is the actual kill/abort key; Ctrl-A redisplays the line rather than moving the cursor to line-start; flow control is Ctrl-W to pause / any key to resume, not Ctrl-S/Ctrl-Q; and shell EOF/exit is ESC on a blank line, not Ctrl-D.
context:   A related hazard: an arrow key sends an ANSI escape sequence that begins with a raw ESC byte, and the line editor treats a bare leading ESC as "exit" — this can make arrow-key input look like it randomly exits a shell or editor session. `tmode`/`xmode` let you remap these per-device; remapping EOF to Ctrl-D is one confirmed fix for the arrow-key hazard specifically. (Ctrl-C = interrupt, Ctrl-E = abort are corroborated as device-driver signal keys in The OS-9 Guru §2.8 p.29 and Using Professional OS-9 v2.4 p.3-6, both customizable.)
source:    authored + The OS-9 Guru §2.8 + Using Professional OS-9 v2.4, "Using the Keyboard", p. 3-6
--- END ---

--- CARD ---
id:        kill-command
type:      FACT
target:    all
verify:    from-manual
topic:     signals
claim:     The `kill` shell command sends the S_KILL signal (signal 0) to a process by PID, aborting it; `kill 0` sends it to every process owned by the current user (the broadcast convention).
context:   -
source:    The OS-9 Primer, Ch.4 "Built-in Commands" + Using Professional OS-9 v2.4, "Built-in Shell Commands", p. 5-6
--- END ---

---

## Events and semaphores

--- CARD ---
id:        events-one-primitive
type:      CONCEPT
target:    all
verify:    authored
topic:     events
claim:     Where Unix/POSIX splits synchronization into separate mutex, condition-variable, and counting-semaphore primitives, OS-9 covers all three with a single mechanism: the event, a counter that processes can block on until it reaches a target value. A binary mutex is just an event constrained to the range (1,1); a condition variable is a wait/signal pair on an ordinary event.
context:   Recognizing this collapses three separate Unix concepts into one OS-9 API (`F$Event`) to learn, rather than three.
source:    authored
--- END ---

--- CARD ---
id:        event-wait-signal-atomicity
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     events
claim:     Wait suspends the calling process until the event value is within a target range, adds a wait increment to the event's value, then returns control; Signal adds a signal increment and checks for processes to wake. Both operations are atomic with respect to time-slicing, which is what prevents race conditions on the resource being guarded (e.g. modeling exclusive printer access with event value=1, wait increment=-1, signal increment=+1). F$Event provides all of this in one syscall to minimize the number of calls needed.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, "The Wait and Signal Operations", p. 4-9
--- END ---

--- CARD ---
id:        semaphore-readers-writers
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     events
claim:     The readers/writers semaphore pattern (allowing multiple concurrent readers or a single exclusive writer) is implemented on OS-9 by managing separate reader and writer semaphores plus a control semaphore.
context:   -
source:    OS-9 Insights §12.3.1 "Readers/Writers"
--- END ---

---

## Pipes

--- CARD ---
id:        pipe-concept-fifo
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     pipes
claim:     A pipe is a FIFO memory buffer letting concurrent processes exchange data — a writer's output becomes a reader's input. The Pipe File Manager (PIPEMAN) coordinates all multi-process access to it; typically one writer and one reader use a given pipe, though any number of processes may access it simultaneously. No physical device is needed — the pipe's path descriptor uses a null driver, and the default buffer is 90 bytes (configurable).
context:   -
source:    OS-9 v2.4 Technical Reference Manual, "Pipes", p. 4-11 + the independent 1985-era OS-9/68000 technical manual, Introduction "Level 2 - File Managers"
--- END ---

--- CARD ---
id:        pipe-unnamed
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     pipes
claim:     Unnamed pipes connect processes purely through inherited file I/O paths: I$Open creates a brand-new anonymous pipe, and it is only shareable across processes if they're related via F$Fork inheritance (a child inherits the parent's open paths). This is exactly how the shell builds command pipelines.
context:   Two independent (unrelated) processes each opening "/pipe" get two separate, unconnected pipes — sharing requires common ancestry via fork.
source:    OS-9 Insights §17.1 "Unnamed Pipes" + OS-9 v2.4 Technical Reference Manual, "Named and Unnamed Pipes" and "Opening Pipes", pp. 4-11 to 4-12
--- END ---

--- CARD ---
id:        pipe-named
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     pipes
claim:     Named pipes are file-based communication channels that let unrelated processes connect: they're created as special files and opened by name. I$Open on a named pipe searches a linked list for that name; if found, it returns the existing path (much like I$Dup) rather than creating a new one, and the pipe persists even while empty as long as it's still open somewhere.
context:   -
source:    OS-9 Insights §17.3 "Named Pipes" + OS-9 v2.4 Technical Reference Manual, "Named and Unnamed Pipes" and "Opening Pipes", pp. 4-11 to 4-12
--- END ---

--- CARD ---
id:        pipe-shell-style
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     pipes
claim:     Shell-style pipes are built with F$Fork: the shell creates child processes with redirected stdin/stdout, then closes the unused ends to establish one-directional communication between them.
context:   -
source:    OS-9 Insights §17.2 "Shell-Style Pipes"
--- END ---

--- CARD ---
id:        pipe-full-blocking-deadlock
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     pipes
claim:     Writes to a full named pipe block until space frees up (unless the writer is interrupted by a signal). For an unnamed pipe, if every process with access to it is simultaneously blocked trying to write (pipe full, nobody reading), OS-9 detects the resulting deadlock and the first waiting writer gets an error rather than hanging forever.
context:   -
source:    The OS-9 Primer, Ch.8 "Writing to a full pipe" and "Potential Problems with Pipes"
--- END ---

--- CARD ---
id:        pipe-efficiency-advantages
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     pipes
claim:     Pipes are an efficient, flexible IPC mechanism, with advantages over signals: messages can be longer than 16 bits, messages queue naturally, a process can check for pending data before reading, and named pipes give easy process-independent coordination without a fork relationship. Reader and writer can also use different transfer sizes per read/write call.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, "Pipes", p. 4-11
--- END ---

--- CARD ---
id:        pipe-read-eof-behavior
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     pipes
claim:     I$Read/I$ReadLn return the next available bytes from a pipe; if not enough data is ready, the reading process sleeps until more arrives. EOF is recognized only when the pipe is empty AND the number of readers equals the number of users; if data was read before EOF was reached, no error is returned — just a byte count smaller than requested. Read/Write are faster than ReadLn/WritLn since the latter must scan for carriage returns.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, "Read/ReadLn", p. 4-13
--- END ---

---

## Alarms

--- CARD ---
id:        alarm-guard-pattern
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     alarms
claim:     Alarms can act as guards to interrupt a long-running operation: they trigger after a timeout and send a signal to the process, letting it break out of an otherwise-unbounded wait.
context:   -
source:    OS-9 Insights §9.1.1 "Alarms as Guards"
--- END ---

--- CARD ---
id:        alarm-ticker-pattern
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     alarms
claim:     Alarms can serve as tickers to simulate thread-like concurrency inside a single process: multiple alarms firing at different intervals drive different pieces of work within that one process.
context:   -
source:    OS-9 Insights §9.1.2 "Alarms as Tickers for Threads"
--- END ---

--- CARD ---
id:        alarm-time-of-day-sensitivity
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     alarms
claim:     A time-of-day alarm fires a signal at a specific datetime and is sensitive to system clock changes: if the clock is corrected, the alarm fires at the corrected time, not at the originally-scheduled interval. This enables cron-like utilities and alarm-clock behavior, but means "alarm at 5:00" and "alarm in 1 hour" can diverge if the clock is adjusted in between.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, "Time of Day Alarms", p. 4-5
--- END ---

--- CARD ---
id:        alarm-system-state
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     alarms
claim:     A system-state alarm has the kernel run a specified system-state subroutine at very high priority when it expires, instead of sending a signal to a requesting process. That subroutine runs in the system process, with its stack pointer biased into the system process descriptor (~1K of free space there). The kernel auto-deletes a process's pending alarms when it exits, which is undesirable for a persistent alarm (e.g. a disk-motor shutdown timer) — the workaround is to issue the F$Alarm request on behalf of the system process instead of the requesting one.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, "System-state Alarms", p. 4-6
--- END ---

---

## Kernel variants and system globals

--- CARD ---
id:        kernel-variants
type:      FACT
target:    68k
verify:    from-manual
topic:     kernel
claim:     OS-9 ships two kernel variants per processor: a standard kernel with full development facilities, and a smaller, faster atomic kernel aimed at embedded systems where execution time matters more than development conveniences.
context:   -
source:    The OS-9 Primer, Ch.2 "Which kernel to use" + The OS-9 Primer, Ch.2 "Atomic Kernel"
--- END ---

--- CARD ---
id:        system-globals
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     kernel
claim:     OS-9 maintains system global variables: some are read-only information about system state, others are modifiable and let a privileged process control system behavior (e.g. D_MinPty, D_MaxAge, set via F$SetSys).
context:   -
source:    The OS-9 Primer, Chapter 16: "System global variables"
--- END ---

---

## Shell process control

--- CARD ---
id:        shell-builtin-commands
type:      FACT
target:    all
verify:    from-manual
topic:     shell
claim:     The shell has built-in commands that execute without loading a program or creating a new process, and regardless of the current execution directory: chd/chx (change data/execution directory), kill/w/wait (process control), setenv/unsetenv (environment), setpr (priority), logout, profile, ex (chain), set. These built-ins do not appear in the CMDS directory and change only the shell's own private state, which is not inherited by forked children.
context:   -
source:    The OS-9 Guru §2.3 p.24 + Using Professional OS-9 v2.4, "Built-in Shell Commands", p. 5-6
--- END ---

--- CARD ---
id:        shell-env-vars-per-process
type:      CONCEPT
target:    all
verify:    from-manual
topic:     shell
claim:     Environment variables live per-process in static storage, not globally: each forked process gets its own copy from its parent, and modifications are private unless explicitly passed on to children via os9exec(). setenv declares/sets a variable in the shell's own environment storage; if a child shell redefines a variable, that redefinition is visible only to that child shell and its own descendants, never propagating back up to the parent.
context:   Different processes can hold the same variable name with different values — this is what makes them useful as per-process configuration.
source:    The OS-9 Guru §2.4 p.25 + Using Professional OS-9 v2.4, "The Shell Environment", pp. 5-3 to 5-4
--- END ---

--- CARD ---
id:        shell-env-vars-auto-four
type:      FACT
target:    68k
verify:    from-manual
topic:     shell
claim:     On timesharing login, four environment variables are set automatically: PORT (terminal name, set by tsmon), HOME (home directory from the password file), SHELL (first process run at login), USER (the name typed at login). On single-user systems these can instead be set manually with setenv or in a procedure file.
context:   -
source:    Using Professional OS-9 v2.4, "The Shell Environment", p. 5-3
--- END ---

--- CARD ---
id:        shell-current-dirs
type:      FACT
target:    all
verify:    from-manual
topic:     shell
claim:     Each process has its own current data directory and current execution directory; a child inherits both from its parent at fork time, but subsequent changes in either direction don't propagate (child changes don't affect the parent, and vice versa). The shell's chx/chd built-ins change the shell's own directories; the execution directory is consulted when opening a path in execute mode, the data directory otherwise.
context:   -
source:    The OS-9 Guru, section 2.6, page 27
--- END ---

--- CARD ---
id:        shell-special-characters
type:      FACT
target:    all
verify:    from-manual
topic:     shell
claim:     Shell special characters: ';' sequential execution, '&' concurrent (background) execution, '!' pipe output into input, '*' set priority, '#' set data space, '()' subshell, '<' redirect stdin, '>' redirect stdout, '>>' redirect stderr. Parameters containing spaces or special characters must be quoted.
context:   This source's claim that '*' sets priority conflicts with `shell-priority-modifier-char` below (a different manual attributes that role to '^'/caret) — a genuine cross-source disagreement, not resolved here; verify against a primary TRM before relying on either.
source:    The OS-9 Guru, section 2.2, figure 3, page 23
--- END ---

--- CARD ---
id:        shell-priority-commands
type:      FACT
target:    all
verify:    from-manual
topic:     shell
claim:     `setpr <proc ID> <priority>` is a shell built-in that changes a running process's priority after the fact; requires owning the process or being superuser.
context:   -
source:    The OS-9 Primer, Ch.4 "Built-in Commands" + Using Professional OS-9 v2.4, "Built-in Shell Commands", p. 5-6
--- END ---

--- CARD ---
id:        shell-priority-modifier-char
type:      FACT
target:    68k
verify:    from-manual
topic:     shell
claim:     A caret (^) modifier on a shell command line sets that command's process priority at launch time, without a permanent setpr change afterward.
context:   This disagrees with `shell-special-characters` above, which attributes the "set priority" role to '*' instead of '^' — a genuine cross-source conflict, kept unresolved rather than silently picking one; verify against a primary TRM before relying on either.
source:    Using Professional OS-9 v2.4, "Shell Command Line Processing", p. 5-8
--- END ---

--- CARD ---
id:        shell-wait-commands
type:      FACT
target:    all
verify:    from-manual
topic:     shell
claim:     `w` waits for a single (specific) child process to terminate; plain `wait` blocks the shell until all of its background child processes have terminated before returning the prompt.
context:   -
source:    Using Professional OS-9 v2.4, "Built-in Shell Commands", p. 5-6 + The OS-9 Primer, Ch.4 "Built-in Commands"
--- END ---

--- CARD ---
id:        shell-background-execution
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     shell
claim:     The shell runs a command as a background process via the `&` operator; the shell continues waiting for new commands while that process runs.
context:   -
source:    The OS-9 Primer, Chapter 4: "Background tasks"
--- END ---

--- CARD ---
id:        shell-foreground-background-process
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     shell
claim:     A foreground process requires user interaction (editing a file, a program prompting for input); a background process needs no user attention (printing, sorting datafiles). Exactly one foreground process can run at a time, alongside any number of background processes — e.g. printing in the background while editing a file in the foreground.
context:   -
source:    Using Professional OS-9 v2.4, "Multi-tasking", p. 1-4
--- END ---

--- CARD ---
id:        shell-procedure-file-scoping
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     shell
claim:     A procedure file (a file of shell command lines) is executed by a newly created child shell, as if the commands had been typed manually; control returns to the parent shell once the file finishes. Built-ins like chd/chx inside that procedure file only affect the child shell, never the parent — so a procedure file can't accidentally change the parent's working directories.
context:   -
source:    Using Professional OS-9 v2.4, "Shell Command Line Processing", p. 5-7
--- END ---

--- CARD ---
id:        procs-utility
type:      FACT
target:    68k
verify:    from-manual
topic:     shell
claim:     The `procs` utility lists currently running processes — used for monitoring system activity and process status.
context:   -
source:    Using Professional OS-9 v2.4, "Basic Utilities", p. 3-7
--- END ---

--- CARD ---
id:        break-utility
type:      FACT
target:    68k
verify:    from-manual
topic:     debugging
claim:     The `break` utility issues F$SysDbg, halting OS-9 and every user process and handing control to the ROM debugger; only superusers can run it, and it must be invoked from the system console. If there's no ROM debugger, or it's disabled, `break` resets the system instead.
context:   For system-level debugging only — never on a production system.
source:    Using Professional OS-9 v2.4, "The OS-9 Utilities", break section, p. 10
--- END ---

--- CARD ---
id:        sysgo
type:      CONCEPT
target:    all
verify:    from-manual
topic:     boot
claim:     Sysgo (SYSGO) is the first user process the kernel forks after coldstart; its standard I/O is the system console. It changes to the CMDS directory, forks a shell to run the system startup file, then loops forking/waiting for the shell so it can restart it if the user's shell ever terminates — acting as insurance against every process dying out from under the system.
context:   On non-disk systems, a custom SYSGO module can replace the standard disk-based one.
source:    The OS-9 Guru §2.1 p.19 + the independent 1985-era OS-9/68000 technical manual §2-5 "SYSGO"
--- END ---

---

## Process memory model

--- CARD ---
id:        process-code-data-separation
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     process-management
claim:     OS-9 splits every process into two logically separate areas: code (read-only, position-independent, ROMable, and shared across every process running that module) and data (unique per process, writable). A process accesses its variables "register indirect": a 68000 register (by convention a6) is loaded with the base address of the process's data area, and all variable access goes through it. This lets one ROM-resident program module support multiple concurrent incarnations, each with its own private data space. If the program has initialized variables, the OS-9 linker builds an initialization table that the kernel copies into the fresh data area at fork time.
context:   -
source:    The OS-9 Guru §1.9 p.14 + the independent 1985-era OS-9/68000 technical manual §4-1 "Process Memory Areas" + OS-9 v2.4 Technical Reference Manual, "Process Memory Areas", p. 2-25
--- END ---

--- CARD ---
id:        process-memory-area-limit
type:      FACT
target:    all
verify:    from-manual
topic:     process-management
claim:     A process can dynamically allocate up to 32 separate memory areas total (counting its initial static storage and stack); the kernel tracks all of them for automatic cleanup on exit. Contiguous areas are merged where possible to make the most of that 32-entry ceiling.
context:   Prevents runaway allocation from a single process.
source:    The OS-9 Guru, section 3.4, page 50
--- END ---
