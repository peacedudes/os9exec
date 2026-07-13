--- CARD ---
id:        module-sync-bytes
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Every OS-9 module begins with a 2-byte sync code: hexadecimal $4AFC
context:   -
source:    OS-9 Insights §2.1 "How Modules Are Identified"
--- END ---

--- CARD ---
id:        module-header-size
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     The first 48 bytes of a module constitute the module header, containing sync bytes, module metadata (length, name, type, protection, revision), and header parity word
context:   -
source:    OS-9 Insights §2.1
--- END ---

--- CARD ---
id:        module-header-parity
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Module header parity is calculated by XOR'ing the first 23 words (46 bytes) and complementing the result; verification involves XOR'ing all 24 header words, which should yield 0 if valid
context:   -
source:    OS-9 Insights §2.2 "The Header Check"
--- END ---

--- CARD ---
id:        module-crc-check
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     OS-9 performs a 3-byte CRC check on the entire module starting from sync bytes; CRC algorithms detect errors more reliably than parity checking
context:   -
source:    OS-9 Insights §2.3
--- END ---

--- CARD ---
id:        module-not-reverified
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     modules
claim:     Once a module is validated and placed in the module directory, OS-9 does not re-verify its CRC; modules are verified only once during bootstrap (ROM) or load (RAM)
context:   -
source:    OS-9 Insights §2.5 "Circumventing CRC Protection"
--- END ---

--- CARD ---
id:        signal-basics
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     signals-ipc
claim:     Signals are a narrow communication channel between processes; they act like hardware interrupts, can interrupt after any instruction when unmasked, and indicate exceptional conditions or serve as inter-process communication
context:   Signals are asynchronous; unmasked signals can interrupt at any point
source:    OS-9 Insights §8 Introduction
--- END ---

--- CARD ---
id:        signal-mask-nesting
type:      FACT
target:    68k
verify:    from-manual
topic:     signals-ipc
claim:     The signal mask level P$SigLvl in the process descriptor is an unsigned byte; when nonzero, the intercept routine is not called. F$SigMask with d1=1 increments it, d1=-1 decrements it, d1=0 clears to zero
context:   Overflow past 255 or underflow past 0 are silently ignored
source:    OS-9 Insights §8.2 "Masking Signals"
--- END ---

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
id:        signal-intercept-state
type:      FACT
target:    68k
verify:    from-manual
topic:     signals-ipc
claim:     When process enters signal intercept routine, signals are automatically masked with P$SigLvl set to 1. One state is stored in system state for non-recursive cases (fast path); recursive states are stored on user stack
context:   -
source:    OS-9 Insights §8.5 "The Signal-State Stack"
--- END ---

--- CARD ---
id:        signal-state-size
type:      FACT
target:    68k
verify:    from-manual
topic:     signals-ipc
claim:     The process state stored for a signal intercept routine includes all MPU registers (72 bytes total), plus FPU registers if FPU is active (168 bytes combined with CPU state)
context:   -
source:    OS-9 Insights §8.5 footnote
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
id:        signal-send-restrictions
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     signals-ipc
claim:     Most signals can be sent to any process, but the kill signal is restricted to same user/group; super user (group 0) can send kill to any process
context:   -
source:    OS-9 Insights §8 Introduction
--- END ---

--- CARD ---
id:        signal-broadcast
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     signals-ipc
claim:     Signals sent to process 0 broadcast to all processes with the same user/group as sender, excluding the sender itself
context:   -
source:    OS-9 Insights §8.8 "Broadcast"
--- END ---

--- CARD ---
id:        signal-queue-behavior
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     signals-ipc
claim:     OS-9 does not discard signals when a process has a signal pending; it queues signals and delivers them in order sent. Queued signals can be 10x more expensive than unqueued
context:   -
source:    OS-9 Insights §8.9 "Queued Signals"
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

--- CARD ---
id:        event-concept
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     events-ipc
claim:     Events provide a synchronization mechanism between processes; they represent conditions that processes wait for and other processes or the system signal
context:   -
source:    OS-9 Insights §11 "Events"
--- END ---

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
id:        semaphore-readers-writers
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     semaphores-ipc
claim:     Readers/writers semaphore pattern allows multiple readers or single writer; implemented by managing separate reader and writer semaphores with a control semaphore
context:   -
source:    OS-9 Insights §12.3.1 "Readers/Writers"
--- END ---

--- CARD ---
id:        pipe-unnamed
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     pipes-ipc
claim:     Unnamed pipes connect processes via standard file I/O paths; child process inherits parent process file descriptors during F$Fork, enabling pipe communication
context:   -
source:    OS-9 Insights §17.1 "Unnamed Pipes"
--- END ---

--- CARD ---
id:        pipe-shell-style
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     pipes-ipc
claim:     Shell-style pipes use F$Fork to create child processes with redirected stdin/stdout, then close unused ends to establish unidirectional communication
context:   -
source:    OS-9 Insights §17.2 "Shell-Style Pipes"
--- END ---

--- CARD ---
id:        pipe-named
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     pipes-ipc
claim:     Named pipes are file-based communication channels allowing unrelated processes to connect; created as special files and opened by name
context:   -
source:    OS-9 Insights §17.3 "Named Pipes"
--- END ---

--- CARD ---
id:        process-fork
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     process-mgmt
claim:     F$Fork creates a child process that inherits parent process attributes (priority, memory, I/O paths, user/group); parent and child run independently
context:   -
source:    OS-9 Insights §7.2 "Family Relationships"
--- END ---

--- CARD ---
id:        process-chain
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     process-mgmt
claim:     F$Chain eliminates the calling process and replaces it with requested process, saving memory compared to F$Fork; no parent process remains after chain
context:   F$Chain is faster than F$Fork but only one process exists after call
source:    OS-9 Insights §7.4 "Chaining New Processes"
--- END ---

--- CARD ---
id:        process-priority
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     scheduling
claim:     Process priority determines CPU scheduling frequency; only relative priority values matter, not absolute numbers (priority 50 behaves like 5000 relative to others)
context:   -
source:    OS-9 Insights §7.5 "Tuning Your Operation"
--- END ---

--- CARD ---
id:        memory-allocation-first-fit
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     memory-mgmt
claim:     First-fit memory allocation finds the first free block large enough for a request; simpler but may fragment worse than best-fit
context:   -
source:    OS-9 Insights §4.1.3 "First-Fit Allocation"
--- END ---

--- CARD ---
id:        memory-allocation-best-fit
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     memory-mgmt
claim:     Best-fit memory allocation searches for the smallest free block that satisfies a request; reduces fragmentation compared to first-fit
context:   -
source:    OS-9 Insights §4.1.4 "Best-Fit Allocation"
--- END ---

--- CARD ---
id:        memory-colored
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     memory-mgmt
claim:     Colored memory partitions address space to avoid cache conflicts; useful on systems with hardware limitations
context:   -
source:    OS-9 Insights §5.3 "Colored Memory"
--- END ---

--- CARD ---
id:        buddy-allocator-overview
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     memory-mgmt
claim:     Buddy system allocator pairs free blocks of same size for faster allocation/deallocation; does not maintain fragment lists, improving speed and predictability
context:   Complicates MMU memory protection because D_MinBlk must equal MMU page size
source:    OS-9 Insights §6 "The Buddy System Allocator"
--- END ---

--- CARD ---
id:        scheduler-aging
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     scheduling
claim:     OS-9 scheduler ages process priority over time; processes waiting for I/O have priority increased gradually to prevent starvation
context:   -
source:    OS-9 Insights §10.2 "Aging"
--- END ---

--- CARD ---
id:        scheduler-preemption
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     scheduling
claim:     System-state preemption allows kernel operations to preempt running processes; the kernel can interrupt any user-state process
context:   -
source:    OS-9 Insights §10.5 "Preemption"
--- END ---

--- CARD ---
id:        interrupt-masking
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     interrupts
claim:     OS-9 can mask interrupts to protect critical kernel operations; this is done at the CPU level, not via software flags alone
context:   -
source:    OS-9 Insights §18.6 "Masking Interrupts"
--- END ---

--- CARD ---
id:        interrupt-latency-factors
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     interrupts
claim:     Interrupt latency increases when signals are masked, system calls that mask interrupts are active, or kernel is in critical sections
context:   -
source:    OS-9 Insights §21.4 "Interrupt Latency"
--- END ---

--- CARD ---
id:        io-unified-filesystem
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     io
claim:     OS-9 I/O uses a unified file system where devices and files appear as paths in a single namespace; same I/O operations work on both
context:   -
source:    OS-9 Insights §15.1 "The Unified File System"
--- END ---

--- CARD ---
id:        io-record-locking
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     io
claim:     OS-9 supports record locking for synchronized access to file regions between processes
context:   -
source:    OS-9 Insights §15.10 "Record Locking"
--- END ---

--- CARD ---
id:        device-descriptor-role
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     drivers-fm
claim:     Device descriptors define device parameters and entry points; tell OS-9 how to interact with hardware and drivers
context:   -
source:    OS-9 Insights §25 "Building a Device Descriptor"
--- END ---

--- CARD ---
id:        realtime-min-priority
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     scheduling
claim:     Setting D_MinPty prevents other processes from running; combines with signal masking for deterministic real-time performance
context:   -
source:    OS-9 Insights §8.2
--- END ---

--- CARD ---
id:        atomic-kernel-interrupt-rate
type:      FACT
target:    68k
verify:    from-manual
topic:     performance
claim:     Atomic kernel can handle theoretical maximum interrupt rates in the thousands per second; actual practical limits depend on system load
context:   -
source:    OS-9 Insights §21.3 "Interrupts"
--- END ---

--- CARD ---
id:        security-user-group
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     security
claim:     Each process carries user and group numbers from password file; super user (group 0) has unrestricted access to system resources
context:   -
source:    OS-9 Insights §22 "Security"
--- END ---

--- CARD ---
id:        system-state-vs-user-state
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     kernel-concepts
claim:     OS-9 distinguishes system state (kernel mode) from user state (application mode); system calls transition between them
context:   -
source:    OS-9 Insights Ch. 22 "Security"
--- END ---

--- CARD ---
id:        trap-handler-linking
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     kernel-concepts
claim:     Programs can link custom trap handlers via OS-9 API; trap handler performance is measurably faster than function call in some cases
context:   -
source:    OS-9 Insights §14 "Traps"
--- END ---

--- CARD ---
id:        module-reentrant
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     modules
claim:     Reentrant modules can be shared across multiple processes without per-process data duplication; require careful design to maintain state isolation
context:   -
source:    OS-9 Insights §2.8.1 "Reentrant Modules"
--- END ---

--- CARD ---
id:        module-attribute-byte
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Module attribute byte encodes module properties including reentrant flag and other control bits
context:   -
source:    OS-9 Insights §2.8 "The Module Attribute Byte"
--- END ---

--- CARD ---
id:        memory-fragmentation-external
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     memory-mgmt
claim:     External memory fragmentation occurs when free memory is scattered across many small blocks; makes large allocations impossible despite total free space
context:   -
source:    OS-9 Insights §4.3 "Memory Fragmentation"
--- END ---

--- CARD ---
id:        memory-ghost-fragmentation
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     memory-mgmt
claim:     Ghost fragmentation occurs when processes hold memory that appears allocated but is not actively used; waste occurs without visible fragmentation
context:   -
source:    OS-9 Insights §3.2 "Memory Fragmentation"
--- END ---

--- CARD ---
id:        deadlock-three-way
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     synchronization
claim:     Three-way deadlock can occur in I/O scenarios where device, driver, and process wait on each other; requires careful protocol design to avoid
context:   -
source:    OS-9 Insights §15.10, Figure 15.2 "Example of Three-Way Deadlock"
--- END ---

--- CARD ---
id:        alarm-guard-pattern
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     alarms
claim:     Alarms can act as guards to interrupt long-running operations; typically trigger after a timeout and send a signal to the process
context:   -
source:    OS-9 Insights §9.1.1 "Alarms as Guards"
--- END ---

--- CARD ---
id:        alarm-ticker-pattern
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     alarms
claim:     Alarms can serve as tickers for thread-like concurrency within a single process; multiple alarms fire at different intervals to drive different tasks
context:   -
source:    OS-9 Insights §9.1.2 "Alarms as Tickers for Threads"
--- END ---

--- CARD ---
id:        path-options-concept
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     io
claim:     Path options control I/O behavior per path (buffering, echo, parity, flow control); can be read/modified via I$GetStt and I$SetStt
context:   -
source:    OS-9 Insights §15.3 "Path Options"
--- END ---

--- CARD ---
id:        rbf-sector-allocation
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     rbf-disk
claim:     RBF disk format includes identification sector, allocation map, root directory, and file descriptors; allocation is tracked via bitmap
context:   -
source:    OS-9 Insights §19 "The RBF Disk Format"
--- END ---

--- CARD ---
id:        svc-classification
type:      FACT
target:    68k
verify:    from-manual
topic:     performance
claim:     System calls (SVCs) are classified by performance impact: fast user-state operations, I/O operations, and slow memory-management operations
context:   -
source:    OS-9 Insights §21.2 "Classes of SVC"
--- END ---

