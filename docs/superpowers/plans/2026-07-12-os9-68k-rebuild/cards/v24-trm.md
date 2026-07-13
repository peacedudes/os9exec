# OS-9 v2.4 Technical Reference Manual — Index Cards

**Architecture scope confirmed:** 68000 family (68000, 68008, 68010, 68020, 68030, 68040, 68070, CPU32).

---

--- CARD ---
id:        four-level-modularity
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     system-architecture
claim:     OS-9 organizes into four hierarchical levels: Kernel/Clock/Init (level 1), File Managers (level 2), Device Drivers (level 3), Device Descriptors (level 4). Each level handles increasingly hardware-specific tasks.
context:   Non-executable modules (Init, device descriptors) are referenced directly by the kernel/file managers; executable modules are called.
source:    OS-9 v2.4 Technical Reference Manual, "System Modularity", p. 1-1
--- END ---

--- CARD ---
id:        reentrancy-shared-code
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     process-management
claim:     Re-entrant code does not modify itself, allowing multiple processes to share a single code module simultaneously. Each process has its own independent data area.
context:   68000 instruction set directly supports re-entrant programming via PC-relative addressing and base-register-relative memory access.
source:    OS-9 v2.4 Technical Reference Manual, "Memory Modules", p. 1-7
--- END ---

--- CARD ---
id:        position-independence-dynamic-load
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     modules
claim:     Position-independent modules can be loaded anywhere in available memory; OS-9 determines load address at runtime. Programmers avoid absolute addressing modes.
context:   Compilers (C, BASIC09) generate position-independent code automatically; assembly-language programmers must be explicit.
source:    OS-9 v2.4 Technical Reference Manual, "Memory Modules", p. 1-7
--- END ---

--- CARD ---
id:        module-structure-three-parts
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Each module has three parts: module header (created by linker at link-time), module body (initialization data, program code, constants), and 3-byte CRC value (computed over all bytes except the CRC itself).
context:   The CRC is 24-bit and checks module integrity on ROM search and before execution.
source:    OS-9 v2.4 Technical Reference Manual, "Basic Module Structure", p. 1-8
--- END ---

--- CARD ---
id:        module-crc-24bit
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     OS-9 computes a 24-bit CRC value over the entire module from the first byte of the module header through the byte just before the CRC itself. Invalid CRC prevents loading from disk or ROM.
context:   Use OS-9 Fixmod utility to update CRC on patched/modified modules. F$CRC system call computes CRC for arbitrary data ranges.
source:    OS-9 v2.4 Technical Reference Manual, "The CRC Value", p. 1-8
--- END ---

--- CARD ---
id:        rom-module-detection
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     system-startup
claim:     On system reset, OS-9 kernel searches ROM by looking for module header sync code ($4AFC). When found, kernel verifies header parity, obtains module size from header, and computes CRC. Valid modules are entered into the module directory.
context:   ROMed modules are automatically linked and included in the system module directory. Non-system modules found in ROM are also entered for user-supplied software discovery.
source:    OS-9 v2.4 Technical Reference Manual, "ROMed Memory Modules", p. 1-9
--- END ---

--- CARD ---
id:        module-header-sync-code
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Module header sync code is the constant value $4AFC (two bytes). This byte pattern marks the start of every module in ROM or memory.
context:   The kernel uses this sync code to detect module boundaries during ROM scanning.
source:    OS-9 v2.4 Technical Reference Manual, "Module Header Definitions", p. 1-9
--- END ---

--- CARD ---
id:        module-type-codes
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Module types: 0=wildcard, 1=Prgm, 2=Sbrtn, 3=Multi, 4=Data, 5=CSDData, 6-10=reserved, 11=TrapLib, 12=Systm, 13=Flmgr, 14=Drivr, 15=Devic, 16+=user-definable.
context:   Type field is M$Type at offset $12 in module header.
source:    OS-9 v2.4 Technical Reference Manual, "Module Header Definitions", p. 1-9
--- END ---

--- CARD ---
id:        module-language-codes
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Module language: 0=wildcard, 1=Objct (68000), 2=ICode, 3=PCode, 4=CCode, 5=CblCode, 6=FrtnCode, 7-15=reserved, 16-255=user-definable. Identifies runtime requirement for executable modules.
context:   Language field is M$Lang at offset $13 in module header.
source:    OS-9 v2.4 Technical Reference Manual, "Module Header Definitions", p. 1-10
--- END ---

--- CARD ---
id:        module-attributes-reentrant-sticky
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Module attributes (M$Attr at offset $14): bit 5=system-state, bit 6=sticky (retained when link count=0 until link count=-1 or memory needed), bit 7=re-entrant (sharable).
context:   Attribute byte field is 1 byte; other bits reserved.
source:    OS-9 v2.4 Technical Reference Manual, "Module Header Definitions", p. 1-10
--- END ---

--- CARD ---
id:        module-header-offset-table
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Standard module header offsets: $00=M$ID (sync bytes), $02=M$SysRev, $04=M$Size, $08=M$Owner, $0C=M$Name, $10=M$Accs, $12=M$Type, $13=M$Lang, $14=M$Attr, $15=M$Revs, $16=M$Edit, $18=M$Usage, $1C=M$Symbol, $20=reserved, $2E=M$Parity, $30+=module-specific.
context:   Resolve offsets in assembly via linking with sys.l/usr.l libraries which define symbolic names.
source:    OS-9 v2.4 Technical Reference Manual, Figure 1-4, p. 1-12
--- END ---

--- CARD ---
id:        program-module-header-fields
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Program module headers add: M$Exec (offset to entry), M$Excpt (default trap handler), M$Mem (data area size), M$Stack (min stack size), M$IData (initialized data offset), M$IRefs (initialized references offset).
context:   These fields follow universal header at offsets $30-$44 for program modules.
source:    OS-9 v2.4 Technical Reference Manual, "Additional Header Fields For Individual Modules", p. 1-13
--- END ---

--- CARD ---
id:        module-a6-register-bias
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     process-management
claim:     Register a6 (static storage/data area base pointer) is biased by $8000 to allow object programs to access 64K of data using indexed addressing from a single base register.
context:   OS-9 linker automatically adjusts for this bias in generated code; programmers usually need not account for it manually.
source:    OS-9 v2.4 Technical Reference Manual, Figure 2-3, p. 1-24
--- END ---

--- CARD ---
id:        kernel-responsibility-three-areas
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     kernel
claim:     Kernel manages three areas: resource coordination (system calls, I/O routing), process control (creation, scheduling, termination), and memory management (allocation, module directory, memory protection).
context:   Kernel is ROMable and compact, residing at level 1 of four-level modularity.
source:    OS-9 v2.4 Technical Reference Manual, "Responsibilities of the Kernel", p. 2-1
--- END ---

--- CARD ---
id:        system-call-two-types
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     system-calls
claim:     Two categories of system calls: I/O calls (reads, writes, directed to file managers/drivers) and system function calls (memory management, multitasking, executed directly by kernel). System function calls are divided into user-state and system-state.
context:   User-state calls use F$ prefix; I/O calls use I$ prefix. System-state calls are restricted to privileged code.
source:    OS-9 v2.4 Technical Reference Manual, "System Call Overview", p. 2-2
--- END ---

--- CARD ---
id:        user-state-vs-system-state
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     privilege
claim:     User-state: normal program environment, no direct hardware access, time-sliced, memory protected, can be aborted. System-state: full processor access, no time-slicing after entry, direct hardware control, non-maskable interrupts possible, dangerous (system crash risk).
context:   On 68000 family, system-state is synonymous with supervisor mode.
source:    OS-9 v2.4 Technical Reference Manual, "User-state and System-state", p. 2-2
--- END ---

--- CARD ---
id:        system-call-processing-trap0
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     system-calls
claim:     All OS-9 system calls are processed via TRAP #0 instruction. Assembler macro OS9 generates TRAP #0; dc.w <function>. C library functions wrap system calls transparently.
context:   System calls use register-based parameter passing; all registers are saved/restored by kernel.
source:    OS-9 v2.4 Technical Reference Manual, "Kernel System Call Processing", p. 2-4
--- END ---

--- CARD ---
id:        memory-module-link-count
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     memory-management
claim:     Kernel maintains link count for each module in directory. When process links to module, count increments. When process unlinks, count decrements. When count reaches zero, module de-allocated and removed from directory, unless sticky.
context:   Sticky modules remain in memory until link count becomes -1 or memory is needed elsewhere. Sticky bit is M$Attr bit 6.
source:    OS-9 v2.4 Technical Reference Manual, "Memory Management", p. 2-6
--- END ---

--- CARD ---
id:        os9-memory-map-single-space
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     memory-management
claim:     OS-9 uses a software memory management system with a single memory map. All user tasks share a common address space (not isolated).
context:   Recommendation: keep memory sections in contiguous reserved blocks, arranged for future expansion. Physically contiguous RAM preferred.
source:    OS-9 v2.4 Technical Reference Manual, "OS-9 Memory Map", p. 2-7
--- END ---

--- CARD ---
id:        process-creation-steps
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     process-management
claim:     Process creation via F$Fork: (1) locate/load module, (2) allocate/initialize process descriptor, (3) allocate stack/data areas from free memory, (4) initialize process registers to proper addresses. If any step fails, fork aborts; success adds process to active queue.
context:   Process descriptor contains state, memory allocation, priority, I/O paths. Automatically initialized and maintained by kernel.
source:    OS-9 v2.4 Technical Reference Manual, "Process Creation", p. 2-23
--- END ---

--- CARD ---
id:        new-process-registers-initial
type:      FACT
target:    68k
verify:    from-manual
topic:     process-management
claim:     Registers passed to new process: sr=0000, pc=module entry, d0.w=process ID, d1.l=group/user ID, d2.w=priority, d3.w=#paths inherited, d4.l=undefined, d5.l=parameter size, d6.l=total memory, d7.l=undefined, a0=undefined, a1=top-of-memory, a2=undefined, a3=module ptr, a4=undefined, a5=parameter ptr, a6=data area base ($8000-biased), a7=stack ptr.
context:   a6 biased by $8000. Process may inherit I/O paths from parent via d3.w.
source:    OS-9 v2.4 Technical Reference Manual, Figure 2-3, p. 1-24
--- END ---

--- CARD ---
id:        process-division-code-data
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     process-management
claim:     OS-9 divides each process into two logically separate areas: code (read-only, position-independent, ROMable, shared across processes) and data (unique per process, writable, read/written via base register).
context:   Each data area has a base address in a register (usually a6). If program uses initialized variables, OS-9 linker builds initialization tables; kernel copies initial values at fork time.
source:    OS-9 v2.4 Technical Reference Manual, "Process Memory Areas", p. 2-25
--- END ---

--- CARD ---
id:        process-states-three
type:      FACT
target:    68k
verify:    from-manual
topic:     process-management
claim:     Process states: (1) ACTIVE—ready for execution, scheduled by priority; (2) WAITING—inactive until child terminates or signal received (F$Wait system call); (3) SLEEPING—inactive for specified time interval or until signal (F$Sleep, zero ticks=infinite, includes events).
context:   Each state has separate process descriptor queue. State changes via moving descriptor between queues.
source:    OS-9 v2.4 Technical Reference Manual, "Process States", p. 2-25
--- END ---

--- CARD ---
id:        process-scheduling-tick-slice
type:      FACT
target:    68k
verify:    from-manual
topic:     process-scheduling
claim:     Default tick (system clock interrupt) is 0.01 seconds (10 milliseconds); hardware-dependent and reconfigurable. Default time-slice is 2 ticks. Slice is longest time process controls CPU before kernel re-evaluates active queue. Adjust via D_TSlice global or Init module.
context:   Tick length requires rewriting clock driver and re-initializing hardware to change.
source:    OS-9 v2.4 Technical Reference Manual, "Process Scheduling", p. 2-27
--- END ---

--- CARD ---
id:        process-aging-priority-queue
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     process-scheduling
claim:     Active queue organized by process age (task-switch count since entry plus initial priority). Oldest process at queue head. When new process enters active queue, its age set to assigned priority; other processes' ages increment. Ages capped at $FFFF. After time-slice, kernel executes highest-age process.
context:   This algorithm ensures all active processes receive CPU time, even low-priority ones.
source:    OS-9 v2.4 Technical Reference Manual, "Process Scheduling", p. 2-27
--- END ---

--- CARD ---
id:        preemption-min-priority-max-age
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     process-scheduling
claim:     D_MinPty (minimum priority) prevents processes below threshold from execution/aging. D_MaxAge (maximum age) divides tasks into high-priority (receive all CPU, do not age) and low-priority (do not age past D_MaxAge, run only when high-priority idle). Both initialized in Init module; super-users adjust via F$SetSys.
context:   WARNING: if D_MinPty set above all running task priorities, system shuts down; must reset to recover.
source:    OS-9 v2.4 Technical Reference Manual, "Pre-emptive Task-switching", p. 2-27
--- END ---

--- CARD ---
id:        preemption-system-state-exemption
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     process-scheduling
claim:     System-state processes are not pre-empted until they finish, unless they voluntarily yield time-slice. This exception exists because system-state may execute critical code affecting shared resources that could block unrelated processes.
context:   Time-slicing applies to user-state processes and system-state processes that explicitly sleep/yield.
source:    OS-9 v2.4 Technical Reference Manual, "Pre-emptive Task-switching", p. 2-28
--- END ---

--- CARD ---
id:        exception-vector-trap0-os9
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions
claim:     Vector 32 (User TRAP #0): reserved for OS-9 system calls via F$OS9 entry point. Vectors 33-47 (User TRAP #1-15): user trap handler installation via F$TLink. Vectors 25-31: Level 1-7 autovectors; vectors 57-63 (68070 only): on-chip autovectors.
context:   See architecture-specific vector tables (68000/008/010/070 vs 68020/030/040) for full exception layout.
source:    OS-9 v2.4 Technical Reference Manual, Figure 2-4 & 2-5, p. 2-29 to 2-31
--- END ---

--- CARD ---
id:        exception-error-fatal-dexec
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     exceptions
claim:     Vectors 2-8, 10-24, 48-63 (error exceptions) are fatal and terminate process unconditionally. If F$DFork created process, resources remain intact for postmortem debugger. Catch via F$STrap for non-fatal handlers; runs in user state with all registers stacked.
context:   If error exception occurs in system state, usually means bad data was passed; error returned instead of crash.
source:    OS-9 v2.4 Technical Reference Manual, "Error Exceptions", p. 2-31
--- END ---

--- CARD ---
id:        trace-exception-vector9
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     exceptions
claim:     Vector 9 (trace exception) triggered when status register trace bit set. Allows single-stepping. F$DFork, F$DExec, F$DExit system calls control program tracing.
context:   Trace exception used by OS-9 debugger for interactive debugging.
source:    OS-9 v2.4 Technical Reference Manual, "The Trace Exception", p. 2-32
--- END ---

--- CARD ---
id:        level7-interrupt-nonmaskable-warning
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     interrupts
claim:     Level 7 interrupts are non-maskable and can interrupt system at dangerous times. WARNING: normally should not use Level 7. If used for dynamic RAM refresh or similar, ensure IRQ service routine does NOT use OS-9 system calls or system data structures.
context:   Level 1-6 interrupts are maskable and safe for general use.
source:    OS-9 v2.4 Technical Reference Manual, "AutoVectored Interrupts", p. 2-32
--- END ---

--- CARD ---
id:        io-system-modularity
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     io
claim:     I/O system is modular with four components: (1) kernel (manages I/O modules, ensures appropriate file manager/driver processes each request), (2) file managers (class-level: RBF=random-block, SCF=sequential-character, SBF=sequential-block, PIPEMAN), (3) device drivers (hardware-specific), (4) device descriptors (non-executable tables).
context:   File managers are device-independent; drivers are hardware-specific. Descriptors are easily swappable at runtime.
source:    OS-9 v2.4 Technical Reference Manual, "The OS-9 Unified Input/Output System", p. 3-1
--- END ---

--- CARD ---
id:        file-managers-standard
type:      FACT
target:    68k
verify:    from-manual
topic:     io
claim:     Standard Microware file managers: RBF (random-access disk), SCF (terminals/printers/modems), SBF (tape systems), PIPEMAN (memory pipes). Additional managers: PCF, NFM, ENPMAN, SOCKMAN, IFMAN, PKMAN, GFM, UCM, CDFM, NRF.
context:   RBF, SCF, SBF, PIPEMAN are in standard distribution. Others sold separately or as part of specialized packages (OS-9/ISP, OS-9/ESP, RAVE, CD-RTOS).
source:    OS-9 v2.4 Technical Reference Manual, "I/O Overview", p. 1-4
--- END ---

--- CARD ---
id:        io-request-processing-flow
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     io
claim:     I/O request flow: (1) user process makes request, (2) kernel identifies/validates request, finds file manager/driver/resources, passes to file manager, (3) file manager validates, performs device-independent processing, calls driver for hardware interaction, (4) driver does device-specific work, returns data/status to file manager, (5) file manager returns to kernel, (6) kernel returns to user.
context:   File manager may request kernel for dynamic memory allocation during processing. Kernel supervises entire flow.
source:    OS-9 v2.4 Technical Reference Manual, Figure 1-2, p. 1-6
--- END ---

--- CARD ---
id:        kernel-io-two-data-structures
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     io
claim:     Kernel maintains two internal I/O data structures: (1) device table (reflects device descriptors, created on first I$Attach, contains file manager/driver names and driver static storage pointer, maintains use count), (2) path table (reflects path descriptors, created on I$Open, destroyed on I$Close, represents status of I/O operations).
context:   When path opened, kernel links to device descriptor, saves device info in device table, creates path descriptor in path table.
source:    OS-9 v2.4 Technical Reference Manual, "The Kernel and I/O", p. 3-3
--- END ---

--- CARD ---
id:        device-descriptor-module-nonexec
type:      FACT
target:    68k
verify:    from-manual
topic:     io
claim:     Device descriptor modules are non-executable (type=DEVIC, 15) containing: device name (module name), controller address (M$Port), interrupt vector, IRQ level, polling priority, mode capabilities, file manager name, driver name, initialization table. Can be in ROM, bootfile, or loaded at runtime.
context:   One descriptor per I/O device minimum; may have multiple descriptors per device with different init parameters. For example, one serial driver might have /T1 (terminal) and /P1 (printer) descriptors.
source:    OS-9 v2.4 Technical Reference Manual, "Device Descriptor Modules", p. 3-4
--- END ---

--- CARD ---
id:        device-descriptor-header-offsets
type:      FACT
target:    68k
verify:    from-manual
topic:     io
claim:     Device descriptor header offsets: $30=M$Port (address), $34=M$Vector (vector#), $35=M$IRQLvl (level), $36=M$Prior (polling priority), $37=M$Mode (capabilities), $38=M$FMgr (file mgr name offset), $3A=M$PDev (driver name offset), $3C=M$DevCon (config offset), $3E=reserved, $46=M$Opt (init table size), $48=M$DTyp (device type, first field of init table).
context:   Device type M$DTyp first byte indicates device class (RBF, SCF, etc.). Initialization table copied to path descriptor options on open.
source:    OS-9 v2.4 Technical Reference Manual, Figure 3-1, p. 3-5
--- END ---

--- CARD ---
id:        path-descriptor-three-sections
type:      FACT
target:    68k
verify:    from-manual
topic:     io
claim:     Path descriptor has three sections: (1) universal 30 bytes (PD_PD to PD_LProc), (2) PD_FST file-manager-specific for file pointers/state, (3) PD_OPT 128-byte option area (initialized from device descriptor, alterable via GetStat/SetStat).
context:   Path descriptors dynamically allocated/de-allocated as paths opened/closed. Definitions linked via sys.l/usr.l.
source:    OS-9 v2.4 Technical Reference Manual, "Path Descriptors", p. 3-7
--- END ---

--- CARD ---
id:        path-descriptor-universal-offsets
type:      FACT
target:    68k
verify:    from-manual
topic:     io
claim:     Universal path descriptor offsets: $00=PD_PD (path#), $02=PD_MOD (access mode), $03=PD_CNT (count, obsolete), $04=PD_DEV (device table addr), $08=PD_CPR (requester PID), $0A=PD_RGS (caller reg stack), $0E=PD_BUF (data buffer addr), $12=PD_USER (owner group/user ID), $16=PD_PATHS (open paths list), $1A=PD_COUNT (path count), $1C=PD_LProc (last active PID), $20=PD_ErrNo (errno), $24=PD_SysGlob (sys global ptr), $2A=PD_FST (file mgr storage), $80=PD_OPT (option table).
context:   Maintained by kernel/file manager/driver as appropriate.
source:    OS-9 v2.4 Technical Reference Manual, Figure 3-2, p. 3-8
--- END ---

--- CARD ---
id:        file-manager-role-class-level
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     io
claim:     File managers process raw I/O data streams for a class of similar devices (logical operations). They make device drivers conform to OS-9 I/O standard by removing device-specific details, making driver uniform across device types.
context:   File managers execute in system state. Deal with device-independent operations like buffering, parity, record structure.
source:    OS-9 v2.4 Technical Reference Manual, "File Managers", p. 3-9
--- END ---

--- CARD ---
id:        rbf-random-block-file-manager
type:      FACT
target:    68k
verify:    from-manual
topic:     io
claim:     RBF (Random Block File Manager) handles I/O for random-access, block-structured devices (floppy/hard disk). Maintains directory structures, sector allocation.
context:   One of four standard file managers. Provides file system semantics for disk devices.
source:    OS-9 v2.4 Technical Reference Manual, "I/O Overview", p. 1-4
--- END ---

--- CARD ---
id:        signal-software-interrupt
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     ipc
claim:     Signals are software interrupts enabling one process to send a numbered interrupt to another. If active process receives signal, intercept routine executes immediately; if sleeping/waiting, process moves to active queue, signal routine executes, process resumes after queued call.
context:   Process without intercept handler for signal (except signal 0, 1) is killed. Signals enable immediate inter-process notifications.
source:    OS-9 v2.4 Technical Reference Manual, "Signals", p. 4-2
--- END ---

--- CARD ---
id:        signal-codes-user-state
type:      FACT
target:    68k
verify:    from-manual
topic:     ipc
claim:     User-state signal codes: 0=unconditional abort (super-user to any, else own group/user only), 1=wake-up (sleeping/waiting awakened, not intercepted), 2=keyboard abort (Ctrl-E), 3=keyboard interrupt (Ctrl-C), 4=hang-up (modem loss), 5-31=reserved (deadly to I/O), 32-255=reserved, 256-65535=user-defined.
context:   Can design signal routine to interpret code word as data to indicate execution stages.
source:    OS-9 v2.4 Technical Reference Manual, "Signals", p. 4-2 to 4-3
--- END ---

--- CARD ---
id:        alarm-user-state-types
type:      FACT
target:    68k
verify:    from-manual
topic:     ipc
claim:     User-state alarm functions via F$Alarm: A$Delete (remove pending), A$Set (signal after interval), A$Cycle (signal periodically), A$AtDate (signal at Gregorian datetime), A$AtJul (signal at Julian datetime).
context:   Signals sent to requesting process; kernel handles timing.
source:    OS-9 v2.4 Technical Reference Manual, "User-state Alarms", p. 4-4
--- END ---

--- CARD ---
id:        cyclic-alarm-time-base
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     ipc
claim:     Cyclic alarm provides time base within program, synchronizing time-dependent tasks. Signal identifier specifies which display/function to update. System handles all timing; handler reads sensor/updates display.
context:   Useful for real-time applications (games, simulations, dashboards) needing multiple time bases; more efficient than polling.
source:    OS-9 v2.4 Technical Reference Manual, "Cyclic Alarms", p. 4-4
--- END ---

--- CARD ---
id:        time-of-day-alarm-sensitivity
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     ipc
claim:     Time-of-day alarm signals process at specific datetime; sensitive to system time changes. If clock corrected, alarm fires at corrected time, not adjusted time interval. Enables cron-like utilities and alarm-clock functions.
context:   Example: alarm at 5:00 vs alarm in 1 hour differ when system clock adjusted.
source:    OS-9 v2.4 Technical Reference Manual, "Time of Day Alarms", p. 4-5
--- END ---

--- CARD ---
id:        relative-time-alarm-timeout
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     ipc
claim:     Relative time alarm sets time limit for specific action. Common use: send keyboard abort signal at max time, issue I$Read; if alarm arrives before input, Read returns error; otherwise cancel alarm.
context:   Useful for implementing read timeouts without complex state machines.
source:    OS-9 v2.4 Technical Reference Manual, "Relative Time Alarms", p. 4-5
--- END ---

--- CARD ---
id:        system-state-alarm-high-priority
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     ipc
claim:     System-state alarm counterpart: kernel executes specified system-state subroutine at very high priority when alarm expires (vs user-state sending signal to requesting process). Subroutine runs in system process, stack pointer biased into system process descriptor with ~1K free space.
context:   Kernel auto-deletes process's pending alarms on exit (undesirable for persistent alarms like disk motor shutdown). Workaround: execute F$Alarm on behalf of system process instead.
source:    OS-9 v2.4 Technical Reference Manual, "System-state Alarms", p. 4-6
--- END ---

--- CARD ---
id:        system-state-alarm-constraints
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     ipc
claim:     Alarm subroutine must NOT perform any function resulting in sleeping/queuing: no F$Sleep, F$Wait, F$Load, F$Event (wait), F$IOQU, F$Fork (if might require load). Other functions permitted. Module containing alarm must remain in memory until alarm expires.
context:   Alarms useful for polling (mouse, light pen) but should be conservative: large cycle to minimize CPU waste.
source:    OS-9 v2.4 Technical Reference Manual, "System-state Alarms", p. 4-6
--- END ---

--- CARD ---
id:        event-multiple-value-semaphore
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     ipc
claim:     OS-9 event is a multiple-value semaphore synchronizing concurrent processes accessing shared resources. Event is 32-byte system global with: ID, name (≤11 chars), value (32-bit, range 2B), wait increment, signal increment, link count, queue pointers.
context:   Does not transmit information but can be used to signal/synchronize; processes can read event value.
source:    OS-9 v2.4 Technical Reference Manual, "Events", p. 4-8
--- END ---

--- CARD ---
id:        event-wait-signal-atomicity
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     ipc
claim:     Wait operation suspends process until event within range, adds wait increment to event value, returns control. Signal operation adds signal increment, checks for processes to awaken, returns. Both atomic with respect to time-slicing; prevents race conditions on resource allocation (e.g., printer access with event value=1, wait increment=-1, signal increment=+1).
context:   F$Event system call provides all event operations to minimize required syscalls.
source:    OS-9 v2.4 Technical Reference Manual, "The Wait and Signal Operations", p. 4-9
--- END ---

--- CARD ---
id:        event-functions-via-f-event
type:      FACT
target:    68k
verify:    from-manual
topic:     ipc
claim:     F$Event system call functions (resolve via funcs.a/sys.l/usr.l): Ev$Link, Ev$UnLnk, Ev$Creat, Ev$Delet, Ev$Wait, Ev$WaitR, Ev$Read, Ev$Info, Ev$Pulse, Ev$Signl, Ev$Set, Ev$SetR.
context:   All event operations accessible through single F$Event syscall to minimize kernel interface.
source:    OS-9 v2.4 Technical Reference Manual, Figure 4-10, p. 4-10
--- END ---

--- CARD ---
id:        pipe-fifo-interprocess-data
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     ipc
claim:     OS-9 pipe is FIFO buffer enabling concurrent processes to communicate data; writer output becomes reader input. Pipeman file manager coordinates multi-process access. One-way path typical (one writer, one reader) but any number may access simultaneously.
context:   Default 90-byte buffer (configurable). No physical device needed; descriptor uses null driver.
source:    OS-9 v2.4 Technical Reference Manual, "Pipes", p. 4-11
--- END ---

--- CARD ---
id:        pipe-efficiency-advantages
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     ipc
claim:     Pipes are extremely efficient/flexible for interprocess communication. Advantages over signals: longer messages (>16 bits), queued messages, determination of pending data, easy process-independent coordination (named pipes). Data transfer size independent (read/write different sizes).
context:   Faster than signals; can implement producer-consumer patterns elegantly.
source:    OS-9 v2.4 Technical Reference Manual, "Pipes", p. 4-11
--- END ---

--- CARD ---
id:        named-unnamed-pipes
type:      FACT
target:    68k
verify:    from-manual
topic:     ipc
claim:     Named pipes: shareable by independent processes via name, multiple opens, persistent if empty. Unnamed pipes: opened once, only shareable if parent-child via inherited paths (used by shell for pipelines).
context:   Shell uses unnamed pipes for command pipelines; user programs may use both types.
source:    OS-9 v2.4 Technical Reference Manual, "Named and Unnamed Pipes", p. 4-11
--- END ---

--- CARD ---
id:        pipe-creation-i-create
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     ipc
claim:     I$Create with PIPEMAN creates named/unnamed pipes. Named: I$Create("/pipe/<name>") with error E$CEF if exists. Unnamed: I$Create("/pipe"). Access permissions handled like RBF. Default buffer from descriptor; override with mode size bit + d2 register; else 90-byte default in path descriptor.
context:   Path automatically set to update mode regardless of creation mode.
source:    OS-9 v2.4 Technical Reference Manual, "Creating Pipes", p. 4-12
--- END ---

--- CARD ---
id:        pipe-open-unnamed-vs-named
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     ipc
claim:     Unnamed pipes: I$Open creates new anonymous pipe. Named pipes: I$Open searches linked list for name; if found, returns existing path (like I$Dup). New /pipe path for second process is independent; share only via parent fork inheritance.
context:   Multiple processes sharing same unnamed pipe requires F$Fork path inheritance.
source:    OS-9 v2.4 Technical Reference Manual, "Opening Pipes", p. 4-12
--- END ---

--- CARD ---
id:        pipe-read-wait-behavior
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     ipc
claim:     I$Read/I$ReadLn return next bytes from pipe; if insufficient data ready, reading process sleeps until more available. EOF recognized when pipe empty AND # readers equals # users; if data read before EOF, no error returned, just byte count < requested.
context:   Read/Write faster than ReadLn/WritLn (no carriage return checking, tighter loops).
source:    OS-9 v2.4 Technical Reference Manual, "Read/ReadLn", p. 4-13
--- END ---

--- CARD ---
id:        pipe-write-error-unnamed-named
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     ipc
claim:     Unnamed pipes: I$Write returns E$Write error when all full-pipe writers attempt write (pipe remains full); each writer gets error. Named pipes: no E$Write error; writer sleeps until reader opens/drains pipe.
context:   Difference allows named pipes to work as self-destructing temporary RAM disk files (non-empty, no open paths).
source:    OS-9 v2.4 Technical Reference Manual, "Write/WritLn", p. 4-13
--- END ---

--- CARD ---
id:        pipe-close-memory-unnamed-named
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     ipc
claim:     Unnamed pipe close: path count decrements; no paths remaining → memory returned to system. Named pipe close: path count decrements; non-empty named pipe kept open (waiting for reader), memory returned only when empty.
context:   Named pipe persistence allows deferred reader access.
source:    OS-9 v2.4 Technical Reference Manual, "Close", p. 4-14
--- END ---

--- CARD ---
id:        data-module-nonreentrant-shared
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     ipc
claim:     Data modules enable multiple processes to share data area and transfer data. Can be non-re-entrant (modify itself), unlike program modules. No synchronization by OS-9; requires explicit events/signals for multi-process updates.
context:   Data area contents/organization not restricted by OS-9; determined by user code.
source:    OS-9 v2.4 Technical Reference Manual, "Data Modules", p. 4-16
--- END ---

--- CARD ---
id:        data-module-creation-f-datmod
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     ipc
claim:     F$DatMod system call creates data module with specified attributes, size, name. Data area cleared automatically. Module created with valid CRC, entered into system directory.
context:   Microware C provides _mkdata_module(), modlink(), modload(), munlink(), munload() C calls for data module operations.
source:    OS-9 v2.4 Technical Reference Manual, "Creating Data Modules", p. 4-17
--- END ---

--- CARD ---
id:        data-module-link-count-sticky
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     ipc
claim:     Data module link count tracks process linkage. Module removed when link count reaches zero, unless sticky (set sticky bit at creation). Sticky module remains until link count becomes -1 or memory needed.
context:   Similar to program module link count behavior.
source:    OS-9 v2.4 Technical Reference Manual, "The Link Count", p. 4-17
--- END ---

--- CARD ---
id:        data-module-save-crc-fixmod
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     ipc
claim:     Data module saved to disk has bad CRC if modified since creation; cannot re-load. Use F$SetCRC system call or _setcrc() C call before writing, or use fixmod utility after. Can examine format/contents with dump utility.
context:   This is a modification-tracking issue unique to data modules.
source:    OS-9 v2.4 Technical Reference Manual, "Saving to Disk", p. 4-17
--- END ---

--- CARD ---
id:        user-trap-sixteen-vectors
type:      FACT
target:    68k
verify:    from-manual
topic:     trap-handlers
claim:     68000 family has 16 software trap exception vectors. Vector 0 reserved for OS-9 system calls. Vectors 1-15 available for user-defined trap handlers. Trap 13 (CIO) and trap 15 (math) are Microware-standard.
context:   Trap handlers reduce program size by dynamic linking; execute slightly slower than static linking.
source:    OS-9 v2.4 Technical Reference Manual, "Trap Handlers", p. 5-1
--- END ---

--- CARD ---
id:        trap-handler-three-entry-points
type:      FACT
target:    68k
verify:    from-manual
topic:     trap-handlers
claim:     Trap handler modules (type TrapLib, language Objct) have three entry points: trap execution (called via tcall), trap initialization (called on F$TLink), trap termination (reserved, not currently implemented).
context:   Initialization entry useful for opening files, linking to modules, startup activities.
source:    OS-9 v2.4 Technical Reference Manual, "Trap Handlers", p. 5-1 to 5-2
--- END ---

--- CARD ---
id:        trap-handler-installation-tlink
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     trap-handlers
claim:     F$TLink system call installs trap handler: kernel links to trap module, allocates/initializes static storage, executes initialization routine (called once per handler per program). Program specifies trap number (1-15); invokes via trap instruction + function code.
context:   Program may install handler at startup or defer until first use via exception handler.
source:    OS-9 v2.4 Technical Reference Manual, "Installing and Executing Trap Handlers", p. 5-3
--- END ---

--- CARD ---
id:        trap-assembly-mnemonics-os9-tcall
type:      FACT
target:    68k
verify:    from-manual
topic:     trap-handlers
claim:     OS-9 relocatable assembler mnemonics: OS9 <func> generates TRAP #0; dc.w <func>. tcall <trapnum>,<func> generates TRAP <trapnum>; dc.w <func>. Both are macros making trap calls apparent.
context:   Example: OS9 F$TLink ≡ TRAP #0 / dc.w F$TLink. tcall T$Math,T$DMul ≡ TRAP #5 / dc.w T$DMul.
source:    OS-9 v2.4 Technical Reference Manual, "OS9 and tcall: Equivalent Assembly Language Syntax", p. 5-3
--- END ---

--- CARD ---
id:        trap-late-binding-exception
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     trap-handlers
claim:     Trap handler can be installed at first use (lazy binding). If tcall made before F$TLink, check module's M$Excpt offset. If zero, abort. Otherwise, jump to exception routine, install handler, re-execute tcall. Enables optional trap handlers.
context:   Useful for trap handlers conditionally needed; Example Two demonstrates this pattern.
source:    OS-9 v2.4 Technical Reference Manual, "OS9 and tcall: Equivalent Assembly Language Syntax", p. 5-3
--- END ---

--- CARD ---
id:        math-module-trap15
type:      FACT
target:    68k
verify:    from-manual
topic:     math
claim:     Math module (trap 15) provides: basic floating-point (single/double), extended integer, type conversions, transcendental functions. Available as Math.l (software) or Math881.l (68881/882 FPCP). Can embed in program or install as user trap routine.
context:   32-bit float internally converted to 64-bit double for computation; no speed advantage over double. No de-normalized numbers or negative zero support.
source:    OS-9 v2.4 Technical Reference Manual, "Standard Function Library Module", p. 6-1
--- END ---

--- CARD ---
id:        math-module-libraries
type:      FACT
target:    68k
verify:    from-manual
topic:     math
claim:     Math module files: Math.l (no coprocessor), Math881.l (68881/882 FPCP). Module names: "Math" trap 15 in both cases. Can pre-load via Load command for quick access; not recommended in OS9Boot.
context:   OS-9 compilers automatically link/call math routines; assembly programmers call via F$TLink + trap.
source:    OS-9 v2.4 Technical Reference Manual, "Calling Standard Function Module Routines", p. 6-3
--- END ---

--- CARD ---
id:        math-integer-operations
type:      FACT
target:    68k
verify:    from-manual
topic:     math
claim:     Math integer operations: T$LMul, T$UMul (multiply signed/unsigned), T$LDiv, T$LMod (divide/modulo signed), T$UDiv, T$UMod (divide/modulo unsigned).
context:   32-bit signed (long) and unsigned integers supported.
source:    OS-9 v2.4 Technical Reference Manual, "Integer Operations", p. 6-5
--- END ---

--- CARD ---
id:        math-float-double-operations
type:      FACT
target:    68k
verify:    from-manual
topic:     math
claim:     Single-precision float: T$FAdd, T$FInc, T$FSub, T$FDec, T$FMul, T$FDiv, T$FCmp, T$FNeg. Double-precision: T$DAdd, T$DInc, T$DSub, T$DDec, T$DMul, T$DDiv, T$DCmp, T$DNeg. Operations use IEEE-based floating point; 32-bit float converted to 64-bit internally.
context:   Many functions set MPU status bits (N, Z, V, C) for conditional branches after call.
source:    OS-9 v2.4 Technical Reference Manual, "Single/Double Precision Floating Point Operations", p. 6-5
--- END ---

--- CARD ---
id:        math-conversion-operations
type:      FACT
target:    68k
verify:    from-manual
topic:     math
claim:     Math conversions: ASCII→numeric (T$AtoN, T$AtoL, T$AtoU, T$AtoF, T$AtoD), numeric→ASCII (T$LtoA, T$UtoA, T$FtoA, T$DtoA), numeric→numeric (T$LtoF, T$LtoD, T$UtoF, T$UtoD, T$FtoL, T$DtoL, T$FtoU, T$DtoU, T$FtoD, T$DtoF, T$FTrn, T$DTrn, T$FInt, T$DInt, T$DNrm).
context:   Error convention: C bit set, d1 contains error code.
source:    OS-9 v2.4 Technical Reference Manual, "Numeric Conversions", p. 6-5 to 6-6
--- END ---

--- CARD ---
id:        math-transcendental-functions
type:      FACT
target:    68k
verify:    from-manual
topic:     math
claim:     Transcendental functions: T$Sin, T$Cos, T$Tan, T$Asn, T$Acs, T$Atn, T$Log, T$Log10, T$Sqrt, T$Exp, T$Power. Caller controls precision via passed epsilon (1E-001 to 1E-014 hex representations defined in manual).
context:   Using precision > 14 digits risks infinite loop.
source:    OS-9 v2.4 Technical Reference Manual, "Calling Standard Function Module Routines", p. 6-5 to 6-6
--- END ---

--- CARD ---
id:        rbf-disk-file-organization
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     file-system
claim:     RBF disk file organization: identification sector (boot, disk parameters), allocation map (block usage bitmap), root directory, files in segments. Random-access block-structured storage enabling efficient disk utilization.
context:   File system architecture covered in Chapter 7 of manual.
source:    OS-9 v2.4 Technical Reference Manual, "Disk File Organization", p. 7-1
--- END ---

--- CARD ---
id:        rbf-record-locking
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-system
claim:     Record locking synchronizes concurrent file access. Lock/unlock via I/O functions. Non-sharable files prevent simultaneous opens. End-of-file locks prevent simultaneous append. Deadlock detection prevents infinite wait.
context:   Used for concurrent database access, multi-process file updates.
source:    OS-9 v2.4 Technical Reference Manual, "Record Locking", p. 7-7
--- END ---

--- CARD ---
id:        rbf-file-security
type:      FACT
target:    68k
verify:    from-manual
topic:     file-system
claim:     File security via access permissions (read, write, execute) per owner/group/public. Enforced at file manager level.
context:   Standard Unix-like permission model.
source:    OS-9 v2.4 Technical Reference Manual, "File Security", p. 7-11
--- END ---

