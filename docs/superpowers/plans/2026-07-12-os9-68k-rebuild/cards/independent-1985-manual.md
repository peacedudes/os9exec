--- CARD ---
id:        scheduler-age-comparison-algorithm
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     process-management
claim:     The scheduler maintains the active process queue sorted by process age. When a process enters the queue, its age is initialized to its assigned priority. Whenever a new process is placed in the active queue, the ages of all other processes are incremented. The scheduler selects the process with the highest age for execution next; because the queue is kept sorted by age, the oldest process will be at the head.
context:   Ages are never incremented beyond $FFFF (65535). This age-based scheduling ensures all processes get CPU time, even low-priority ones.
source:    the independent 1985-era OS-9/68000 technical manual, §4-6 "Execution Scheduling"
--- END ---

--- CARD ---
id:        d-minpty-minimum-priority-threshold
type:      FACT
target:    68k
verify:    from-manual
topic:     process-management
claim:     D_MinPty is a system global variable that defines a minimum priority below which processes are neither aged nor considered candidates for execution. When D_MinPty is set to some priority level above zero, all processes running below that level are stopped completely while the critical task runs to completion.
context:   D_MinPty is usually set to zero on most systems. If set to a priority level above all running tasks' priorities, the system will be completely shut down and can only be recovered by a reset. It is critical to restore D_MinPty to a normal level after the critical task finishes.
source:    the independent 1985-era OS-9/68000 technical manual, §4-6 "Pre-emptive Task Switching"
--- END ---

--- CARD ---
id:        d-maxage-maximum-age-task-class-division
type:      FACT
target:    68k
verify:    from-manual
topic:     process-management
claim:     D_MaxAge is a system global variable that defines a maximum age above which processes are not allowed to mature. When activated (non-zero), it essentially divides tasks into two classes: low priority (which stop aging at the MaxAge cutoff) and high priority (which receive all available CPU time). Low priority tasks run only when high priority tasks are inactive.
context:   D_MaxAge is usually set to zero. Both D_MinPty and D_MaxAge are accessible by the superuser through the F$SetSys system call.
source:    the independent 1985-era OS-9/68000 technical manual, §4-6 "Pre-emptive Task Switching"
--- END ---

--- CARD ---
id:        process-system-call-not-preempted
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     process-management
claim:     Any process performing a system call will not be pre-empted until the call finishes, even if a higher-priority process becomes active. This exception exists because system-call routines may execute critical sections that affect shared system resources and could block other unrelated processes.
context:   The process may voluntarily give up its timeslice during a system call, but it cannot be forcibly pre-empted.
source:    the independent 1985-era OS-9/68000 technical manual, §4-6 "Exception And Interrupt Processing"
--- END ---

--- CARD ---
id:        process-state-active
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     process-management
claim:     The ACTIVE process state includes all executable processes. These processes are given time slices for execution according to their relative priority with respect to all other active processes. The scheduler allocates CPU time using an age-comparison algorithm that gives all active processes some CPU time, even if they have very low relative priority.
context:   Active processes are maintained in a queue sorted by age.
source:    the independent 1985-era OS-9/68000 technical manual, §4-5 "Process States"
--- END ---

--- CARD ---
id:        process-state-waiting
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     process-management
claim:     The WAITING process state is entered when a process executes an F$WAIT system service request. The process remains inactive until any of its descendant processes terminates or until it receives a signal.
context:   The wait state allows a parent process to wait for child process termination.
source:    the independent 1985-era OS-9/68000 technical manual, §4-5 "Process States"
--- END ---

--- CARD ---
id:        process-state-sleeping
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     process-management
claim:     The SLEEPING process state is entered when a process executes an F$Sleep service request, specifying a time interval for which the process remains inactive. The process resumes when the specified time has elapsed or when it receives a signal. A zero time interval specifies an infinite sleep period.
context:   Processes use sleep to avoid wasting CPU time while waiting for external events such as I/O completion.
source:    the independent 1985-era OS-9/68000 technical manual, §4-5 "Process States"
--- END ---

--- CARD ---
id:        timeslicing-tick-rate
type:      FACT
target:    68k
verify:    from-manual
topic:     process-management
claim:     The system's CPU is interrupted by a real-time clock at a regular rate, typically 100 times per second. Each basic time interval is called a "tick", normally lasting 10 milliseconds. At each tick, OS-9 can suspend one program and begin execution of another.
context:   The tick rate is system-dependent and can be configured, though 100 Hz is typical.
source:    the independent 1985-era OS-9/68000 technical manual, §4-1 "Overview of Multitasking"
--- END ---

--- CARD ---
id:        f-dfork-debugger-suspended-process-creation
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     process-management
claim:     F$DFork creates a child process under control of a debugger, similar to F$Fork but with key differences: (1) the created process is not placed in the active queue but left in a "suspended" state; (2) the trace bit of the child's status register is set; (3) execution is controlled through F$DExec and F$DExit system calls; (4) a permanent register buffer in the caller's data area is associated with the child process and contains its initial register image.
context:   F$DFork input registers: d0.w=module type/revision (0=any), d1.l=additional stack space, d2.l=parameter size, d3.w=number of I/O paths, d4.w=module priority, (a0)=module name ptr, (a1)=parameter ptr, (a2)=register buffer. Output: d0.w=child process ID, (a2)=initial register image.
source:    the independent 1985-era OS-9/68000 technical manual, §14-11 "F$DFork"
--- END ---

--- CARD ---
id:        f-dfork-gotcha-never-executes-alone
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     process-management
claim:     A process created by F$DFork will never execute at all unless explicitly told to do so via F$DExec. The trace bit set in the child's status register causes the system trace exception handler to occur once for each user instruction executed, making user programs run very slowly during debugging.
context:   This is a consequence of the suspended state and trace-bit design for debugger control.
source:    the independent 1985-era OS-9/68000 technical manual, §14-11 "F$DFork"
--- END ---

--- CARD ---
id:        f-dexec-debugged-child-execution-control
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     process-management
claim:     F$DExec controls the execution of a suspended child process created by F$DFork. Execution terminates when: (1) the specified number of instructions have been executed, (2) a breakpoint is reached, or (3) an unexpected exception occurs. Any OS-9 system calls made by the suspended program are executed at "full speed" and are counted as one logical instruction.
context:   Input registers: d0.w=child process ID, d1.l=number of instructions to execute (0=continuous), d2.w=number of breakpoints, (a0)=breakpoint list, register buffer contains child register image. Output: d0.l=total instructions executed, d1.l=remaining count, d2.w=exception offset (if any), d3.w=classification word, d4.l=access address, d5.w=instruction register. Carry bit set on error.
source:    the independent 1985-era OS-9/68000 technical manual, §14-8 "F$DExec"
--- END ---

--- CARD ---
id:        f-dexec-register-buffer-synchronization
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     process-management
claim:     The register buffer passed to F$DFork is used by the system to save and restore the child process's registers on each F$DExec call. Changing the contents of the register buffer alters the child process's registers when execution resumes.
context:   The register buffer is permanently associated with the debugged child process and provides the mechanism for the debugger to inspect and modify the child's register state.
source:    the independent 1985-era OS-9/68000 technical manual, §14-8 "F$DExec"
--- END ---

--- CARD ---
id:        f-dexec-trace-through-trap-handlers-allowed
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     process-management
claim:     Tracing via F$DExec is allowed through user trap handlers, intercept routines, and the F$Chain system call. This may seem strange but is by design.
context:   This allows debuggers to step through code that calls trap handlers or chains to other programs.
source:    the independent 1985-era OS-9/68000 technical manual, §14-8 "F$DExec Caveats"
--- END ---

--- CARD ---
id:        f-dexit-terminate-debugged-child
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     process-management
claim:     F$DExit terminates a suspended child process that was created with F$DFork. Normal termination by the child process does not release any of its resources, in order to permit post-mortem examination by the debugger.
context:   Input: d0.w=process ID of child to terminate. Output: none. Carry bit set on error. Possible error: E$IPrcID.
source:    the independent 1985-era OS-9/68000 technical manual, §14-10 "F$DExit"
--- END ---

--- CARD ---
id:        f-fork-process-creation-from-module
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     process-management
claim:     F$Fork creates a new process which becomes a "child" of the caller. The system parses the module name string and searches the system module directory. If the program is already in memory, the module is linked and executed. Otherwise, the name string is used as a pathlist to load the file from mass storage. The primary module's module header determines the process's initial data area size. OS-9 allocates contiguous RAM equal to the required data storage size plus any additional size given in d1, plus the size of any parameter passed.
context:   Input: d0.w=desired module type/revision (0=any), d1.l=additional memory size, d2.l=parameter size, d3.w=number of I/O paths to copy, d4.w=priority, (a0)=module name pointer, (a1)=parameter pointer. Output: d0.w=child process ID, (a0)=updated past module name.
source:    the independent 1985-era OS-9/68000 technical manual, §14-14 "F$Fork"
--- END ---

--- CARD ---
id:        f-fork-child-parent-concurrent-execution
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     process-management
claim:     Both child and parent process execute concurrently after F$Fork. If the parent immediately executes F$Wait, it will wait until the child dies before resuming. Caution should be exercised when recursively calling a program that uses F$Fork since a new child may be created with each incarnation until the process table becomes full.
context:   A child process descriptor is returned only when the parent does an F$Wait call.
source:    the independent 1985-era OS-9/68000 technical manual, §14-14 "F$Fork Caveats"
--- END ---

--- CARD ---
id:        f-fork-child-initial-registers-memory-layout
type:      FACT
target:    68k
verify:    from-manual
topic:     process-management
claim:     When F$Fork creates a child process, the following registers are set: sr=0000, pc=module entry point, d0.w=process ID, d1.l=group/user ID, d2.w=priority, d3.w=number of I/O paths inherited, d4.l=undefined, d5.l=parameter size, d6.l=total initial memory allocation, d7.l=undefined, (a0)=undefined, (a1)=top of memory pointer, (a2)=undefined, (a3)=primary module pointer, (a4)=undefined, (a5)=parameter pointer, (a6)=static storage (data) base pointer biased by $8000, (a7)=stack pointer (same as a5).
context:   The (a6) register bias by $8000 allows object programs to access 64K of data using indexed addressing. The linker automatically adjusts for this bias.
source:    the independent 1985-era OS-9/68000 technical manual, §4-4 "Figure 7: New Process Initial Memory Map"
--- END ---

--- CARD ---
id:        module-reentrant-shared-execution
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     modules
claim:     Re-entrant modules are non-self-modifying programs whose code can be shared by multiple processes simultaneously. Each process using a re-entrant module must have its own independent memory area for variables. Almost all OS-9 family software is re-entrant, allowing the same "copy" of a module to be shared by different processes, significantly saving memory.
context:   Re-entrance is indicated by bit 7 of the module's M$Attr field. Example: if Scred requires 26K to load and two processes need it, both can share one copy instead of requiring 52K.
source:    the independent 1985-era OS-9/68000 technical manual, §1-3 "Module Requirements"
--- END ---

--- CARD ---
id:        module-link-count-lifecycle
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     modules
claim:     The operating system maintains a module directory that tracks all modules in memory. Each directory entry contains the module's address and a link count—the number of processes using the module. When a process links to a module, the link count increments. When a process unlinks, the link count decrements. When a module's link count reaches zero, its memory is deallocated and it is removed from the module directory.
context:   This automatic link-count management allows efficient module lifecycle handling without explicit deallocation by individual processes.
source:    the independent 1985-era OS-9/68000 technical manual, §1-1 "Kernel Memory Management Functions"
--- END ---

--- CARD ---
id:        module-header-m-id-sync-bytes
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     The module header begins with the M$ID field at offset $00, containing sync bytes with the constant value $4AFC. These constant bytes are used to locate modules during the startup memory search.
context:   Every valid OS-9 module must begin with these sync bytes.
source:    the independent 1985-era OS-9/68000 technical manual, §1-4 "Module Header Definitions"
--- END ---

--- CARD ---
id:        module-header-m-size-total-size
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     The M$Size field at offset $04 contains the overall size of the module in bytes, including the module header and CRC check value.
context:   This total size field is essential for the kernel to know how much memory a module occupies.
source:    the independent 1985-era OS-9/68000 technical manual, §1-4 "Module Header Definitions"
--- END ---

--- CARD ---
id:        module-header-m-name-offset-to-string
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     The M$Name field at offset $0A contains the offset to the module name string relative to the start (first sync byte) of the module. The name string can be located anywhere in the module and consists of ASCII characters terminated by a null (zero) byte.
context:   This allows flexible placement of the name string within the module.
source:    the independent 1985-era OS-9/68000 technical manual, §1-4 "Module Header Definitions"
--- END ---

--- CARD ---
id:        module-header-m-type-code-values
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     The M$Type field contains the module type code. Standard types are: 0=Not Used (wildcard in system calls), 1=Program Module (Prgm), 2=Subroutine Module (Sbrtn), 3=Multi-Module (Multi), 4=Data Module (Data), 11=User Trap Library (TrapLib), 12=System Module (Systm), 13=File Manager Module (Flmgr), 14=Physical Device Driver (Drier), 15=Device Descriptor Module (Devic). Types 16-255 are user-definable.
context:   Types 5-10 are reserved. Types 16-10 are user-definable.
source:    the independent 1985-era OS-9/68000 technical manual, §1-5 "Module Header Definitions"
--- END ---

--- CARD ---
id:        module-header-m-lang-code-values
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     The M$Lang field contains the module language code. Standard values are: 0=Not Used (wildcard), 1=Object (68000 machine language - Objet), 2=Basic I-code (ICode), 3=Pascal P-code (PCode), 4=C I-code (CCode), 5=Cobol I-code (CblCode), 6=Fortran I-code (FrtaCode). Types 7-15 are reserved, and types 16-255 are user-definable.
context:   The language code identifies which runtime system, if any, is required for module execution.
source:    the independent 1985-era OS-9/68000 technical manual, §1-5 "Module Header Definitions"
--- END ---

--- CARD ---
id:        module-header-m-attr-reentrant-bit
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     The M$Attr field contains attributes about the module. Bit 7 indicates that the module is re-entrant (sharable by multiple tasks).
context:   Re-entrant modules can be safely shared among concurrent processes.
source:    the independent 1985-era OS-9/68000 technical manual, §1-6 "Module Header Definitions"
--- END ---

--- CARD ---
id:        module-header-m-revision-level-highest-retained
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     modules
claim:     The M$Reve field contains the revision level of the module. If two modules with the same name and type are found during the memory search or loaded into memory, only the module with the highest revision level is kept. This allows easy substitution of modules for updates or corrections, especially for ROMed modules.
context:   The revision level provides a simple mechanism for module version control.
source:    the independent 1985-era OS-9/68000 technical manual, §1-6 "Module Header Definitions"
--- END ---

--- CARD ---
id:        module-header-m-parity-check-one-complement-xor
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     The M$Parity field contains the one's complement of the exclusive-OR (XOR) of all previous header words. This value is used by OS-9 for a quick integrity check of the module.
context:   This parity check allows detecting bit errors in the module header.
source:    the independent 1985-era OS-9/68000 technical manual, §1-6 "Module Header Definitions"
--- END ---

--- CARD ---
id:        module-header-m-exec-execution-entry-point
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     The M$Exec field (additional header at offset $30 for program/file-manager/system modules) contains the offset to the program's entry point, relative to the starting address of the module. This is used to set the PC to the module's entry point when a process is forked.
context:   The M$Exec offset is set by the linker during module creation.
source:    the independent 1985-era OS-9/68000 technical manual, §1-7 "Additional Header Fields For Individual Modules"
--- END ---

--- CARD ---
id:        module-header-m-mem-data-area-size
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     The M$Mem field (additional header for program/trap-handler/device-driver modules) contains the required size of the program's data area (storage for program variables).
context:   This field determines how much memory is allocated for the process's data area when the module is forked.
source:    the independent 1985-era OS-9/68000 technical manual, §1-8 "Additional Header Fields For Individual Modules"
--- END ---

--- CARD ---
id:        module-header-m-stack-minimum-stack-size
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     The M$Staok field (additional header for program/trap-handler modules) contains the minimum required size of the program's stack area.
context:   OS-9 ensures that the allocated stack meets or exceeds this minimum size.
source:    the independent 1985-era OS-9/68000 technical manual, §1-8 "Additional Header Fields For Individual Modules"
--- END ---

--- CARD ---
id:        module-header-m-idata-initialized-data-offset
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     The M$IData field (additional header for program/trap-handler modules) contains the offset to the initialization data area's starting address. This area contains values to be copied to the program's data area. All constant values declared in "vsects" are placed here by the linker. The first 4-byte value is the offset from the beginning of the data area to which the initialized data is copied; the next 4-byte value is the number of initialized data bytes to follow.
context:   This mechanism allows programs to have pre-initialized variables.
source:    the independent 1985-era OS-9/68000 technical manual, §1-8 "Additional Header Fields For Individual Modules"
--- END ---

--- CARD ---
id:        module-header-m-irela-initialized-references-offset
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     The M$IRela field (additional header for program/trap-handler modules) contains the offset to a table of values that locate pointers in the data area. Initialized variables in the program's data area may contain values that are pointers to absolute addresses. Code pointers must be adjusted by adding the absolute starting address of the object code area; data pointers must be adjusted by adding the absolute starting address of the data area. The F$Fork system call uses these tables for automatic pointer relocation at execution time. Each table entry's first word is the most significant (MS) word of the offset to the pointer; the second word is a count of the number of least significant (LS) word offsets to be adjusted.
context:   This allows position-independent code with relocated pointers.
source:    the independent 1985-era OS-9/68000 technical manual, §1-8 "Additional Header Fields For Individual Modules"
--- END ---

--- CARD ---
id:        module-position-independence-requirement
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     modules
claim:     Position-independent code means a program does not know where it will be loaded in memory. OS-9 determines the appropriate load address only when the program is run. OS-9 compilers and interpreters generate position-independent code automatically. In assembly language programming, the programmer must ensure position-independence by avoiding addressing modes that refer to absolute addresses.
context:   This requirement enables OS-9's flexible memory management and allows the same module to be loaded at different addresses in different processes.
source:    the independent 1985-era OS-9/68000 technical manual, §1-3 "Module Requirements"
--- END ---

--- CARD ---
id:        module-self-modifying-prohibited
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     modules
claim:     Re-entrant modules must be non-self-modifying. If one user changes a memory location in a shared module, that data change will affect all users of the program. For this reason, re-entrant modules cannot modify themselves.
context:   Data modules are an exception to this restriction, but careful coordination is required for multiple processes to update a shared data module simultaneously.
source:    the independent 1985-era OS-9/68000 technical manual, §1-3 "Module Requirements"
--- END ---

--- CARD ---
id:        process-descriptor-function
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     process-management
claim:     A process descriptor is a data structure that is assigned to each process when created. It contains information about the process including its state, memory allocation, priority, I/O paths, and other administrative data. The process descriptor is automatically initialized and maintained by OS-9. The process does not need to be concerned about the descriptor's existence or contents.
context:   The process descriptor is a kernel-managed structure; user programs should never access it directly.
source:    the independent 1985-era OS-9/68000 technical manual, §4-2 "Process Creation"
--- END ---

--- CARD ---
id:        process-memory-code-data-separation
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     process-management
claim:     All processes divide their memory into two logically separate areas: one for code and one for data. The code area is read-only and contains executable memory modules (the primary module and any linked modules). The data area is separate memory space where all program variables are kept, and the top part is used for the program's stack. This separation enables OS-9's modular software capabilities and allows code sharing among processes.
context:   The data area memory addresses are not known at program-write time; a base address is kept in a register (usually a6 by convention) to access the data area.
source:    the independent 1985-era OS-9/68000 technical manual, §4-1 "Process Memory Areas"
--- END ---

--- CARD ---
id:        process-id-unique-per-process
type:      FACT
target:    68k
verify:    from-manual
topic:     process-management
claim:     Each process is assigned a unique number called a Process ID (PID). The Process ID is assigned by OS-9 and used as its identifier. Other processes can communicate with it by referring to its ID in various system calls.
context:   The Process ID is stored in d0.w when a child process is created via F$Fork or F$DFork.
source:    the independent 1985-era OS-9/68000 technical manual, §4-2 "Process Creation"
--- END ---

--- CARD ---
id:        process-group-id-user-id-inheritance
type:      FACT
target:    68k
verify:    from-manual
topic:     process-management
claim:     Each process has an associated Group ID and User ID, which are inherited from the parent process. These IDs are used to identify all processes and files belonging to a particular user and group of users. The Group ID and User ID are stored in d1.l when a child process is created.
context:   These IDs are used for system and file security.
source:    the independent 1985-era OS-9/68000 technical manual, §4-2 "Process Creation"
--- END ---

--- CARD ---
id:        process-priority-relative-scheduling
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     process-management
claim:     A process's priority value determines how often it receives CPU time relative to other active processes. The scheduler allocates timeslices based on a process's priority. Higher priority processes receive more CPU time. Priority is stored in d2.w when a child process is created and can be retrieved via F$ID.
context:   Process priority is a word value (d2.w in various contexts).
source:    the independent 1985-era OS-9/68000 technical manual, §4-1 "Overview of Multitasking"
--- END ---

--- CARD ---
id:        exception-vector-2-bus-error
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions
claim:     Exception vector 2 is assigned to bus error exceptions. This is handled via F$STrap. These exceptions are usually considered fatal program errors and cause a user program to be unconditionally terminated.
context:   If F$DFork created the process, the process resources will remain intact and control will return to the parent debugger for post-mortem examination.
source:    the independent 1985-era OS-9/68000 technical manual, §4-7 "Error Exceptions"
--- END ---

--- CARD ---
id:        exception-vector-3-address-error
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions
claim:     Exception vector 3 is assigned to address error exceptions. This is handled via F$STrap. These exceptions are usually considered fatal program errors and cause a user program to be unconditionally terminated.
context:   If F$DFork created the process, the process resources will remain intact and control will return to the parent debugger for post-mortem examination.
source:    the independent 1985-era OS-9/68000 technical manual, §4-7 "Error Exceptions"
--- END ---

--- CARD ---
id:        exception-vector-4-illegal-instruction
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions
claim:     Exception vector 4 is assigned to illegal instruction exceptions. This is handled via F$STrap. These exceptions are usually considered fatal program errors and cause a user program to be unconditionally terminated.
context:   If F$DFork created the process, the process resources will remain intact and control will return to the parent debugger for post-mortem examination.
source:    the independent 1985-era OS-9/68000 technical manual, §4-7 "Error Exceptions"
--- END ---

--- CARD ---
id:        exception-vector-5-zero-divide
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions
claim:     Exception vector 5 is assigned to zero divide exceptions. This is handled via F$STrap. These exceptions are usually considered fatal program errors and cause a user program to be unconditionally terminated.
context:   If F$DFork created the process, the process resources will remain intact and control will return to the parent debugger for post-mortem examination.
source:    the independent 1985-era OS-9/68000 technical manual, §4-7 "Error Exceptions"
--- END ---

--- CARD ---
id:        exception-vector-9-trace-dfork
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions
claim:     Exception vector 9 is assigned to the trace exception, which occurs when the status register trace bit is set. This allows the MPU to single-step instructions. OS-9 provides F$DFork, F$DExec, and F$DExit system calls to control program tracing via this exception.
context:   The trace exception is used by debuggers to implement single-stepping.
source:    the independent 1985-era OS-9/68000 technical manual, §4-8 "The Trace Exception"
--- END ---

--- CARD ---
id:        exception-vector-32-os9-standard-call
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions
claim:     Exception vector 32 (user trap 0) is reserved by OS-9 for standard OS-9 system service requests. User programs call OS-9 services via the TRAP #0 instruction, which vectors to this exception handler.
context:   This is the primary mechanism for user programs to invoke OS-9 system calls.
source:    the independent 1985-era OS-9/68000 technical manual, §4-9 "User Traps"
--- END ---

--- CARD ---
id:        exception-vector-33-47-user-traps-library-linking
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions
claim:     Exception vectors 33-47 (user traps 1-15) provide 15 additional user trap vectors. These are used to link to common library routines at execution time. Library routines are similar to program object code modules and are allocated their own static storage when installed via the F$TLink service request. When a user trap is called, its execution entry point is executed. Trap handlers also have initialization and termination entry points, which are executed when linked and at process termination.
context:   This mechanism allows dynamic linking to library functions.
source:    the independent 1985-era OS-9/68000 technical manual, §4-9 "User Traps"
--- END ---

--- CARD ---
id:        exception-vector-25-31-autovectored-interrupts
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions
claim:     Exception vectors 25-31 (autovectors) provide interrupt polling for I/O devices that do not generate vectored interrupts. These are handled via F$IRQ and are internally handled exactly like vectored interrupts. Level 7 interrupts (vector 31) should not normally be used because they are non-maskable and can interrupt the system at dangerous times.
context:   Level 7 interrupts may be used for "software refresh" of dynamic RAMs or similar functions, but the IRQ service routine may not use any OS-9 system calls or system data structures.
source:    the independent 1985-era OS-9/68000 technical manual, §4-9 "AutoVectored Interrupts"
--- END ---

--- CARD ---
id:        exception-vector-64-255-vectored-interrupts
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions
claim:     Exception vectors 64-255 provide 192 vectored interrupts. Multiple devices may use the same vector if necessary. Interrupt service routines are executed in system state without any associated current process. The device driver must provide an error entry point for the system to execute if any error exceptions occur during interrupt processing. The F$IRQ system call is used to install a handler in the system's interrupt tables.
context:   Vectored interrupts provide efficient device interrupt handling with minimal system overhead.
source:    the independent 1985-era OS-9/68000 technical manual, §4-10 "Vectored Interrupts"
--- END ---

--- CARD ---
id:        os9-modularity-four-levels
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     architecture
claim:     OS-9 has four levels of modularity. Level 1: the kernel, clock module, and INIT module (basic system services). Level 2: file managers (RBF for disk, SCF for character devices, PIPEMAN for interprocess communication). Level 3: device drivers for specific I/O controllers. Level 4: device descriptor modules that associate I/O ports with logical names, drivers, and file managers.
context:   This hierarchical modularity allows flexible system configuration and customization.
source:    the independent 1985-era OS-9/68000 technical manual, Introduction "Four Levels of Modularity"
--- END ---

--- CARD ---
id:        rbf-random-block-file-manager
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     io-system
claim:     The Random Block File Manager (RBF) processes all disk-type device functions. It provides a standardized interface for reading and writing data to block-oriented devices like floppy and hard disks.
context:   RBF is one of three standard file managers in OS-9.
source:    the independent 1985-era OS-9/68000 technical manual, Introduction "Level 2 - File Managers"
--- END ---

--- CARD ---
id:        scf-sequential-character-file-manager
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     io-system
claim:     The Sequential Character File Manager (SCF) handles all non-mass storage devices that operate a character at a time, such as terminals and character printers. It provides a standardized interface for character-based I/O devices.
context:   SCF is one of three standard file managers in OS-9.
source:    the independent 1985-era OS-9/68000 technical manual, Introduction "Level 2 - File Managers"
--- END ---

--- CARD ---
id:        pipeman-interprocess-communication-file-manager
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     io-system
claim:     PIPEMAN (the Pipe File Manager) handles interprocess communication using memory buffers for data transfer between processes. It provides named and unnamed pipe facilities.
context:   PIPEMAN is one of three standard file managers in OS-9.
source:    the independent 1985-era OS-9/68000 technical manual, Introduction "Level 2 - File Managers"
--- END ---

--- CARD ---
id:        os9-device-descriptor-modules
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     io-system
claim:     Device Descriptor modules are small tables that associate specific I/O ports with their logical name, device driver, and file manager. These modules also contain the physical address of the port and initialization data. By using device descriptors, only one copy of each driver is required for each specific type of I/O device regardless of how many devices the system uses.
context:   This design allows efficient resource sharing and easy system configuration.
source:    the independent 1985-era OS-9/68000 technical manual, Introduction "Level 4 - Device Descriptors"
--- END ---

--- CARD ---
id:        kernel-basic-functions
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     architecture
claim:     The kernel's main functions are: (1) service request (system call) processing, (2) memory management, (3) system initialization after reset, (4) MPU management (multiprogramming), (5) input/output management, (6) exception and interrupt processing. The kernel is a relatively compact module written in 68000 assembly language, is position-independent, and is directly ROMable.
context:   The kernel serves as the system administrator, supervisor, and resource manager.
source:    the independent 1985-era OS-9/68000 technical manual, §1-1 "Basic Functions of the Kernel"
--- END ---

--- CARD ---
id:        system-call-categories-three
type:      FACT
target:    68k
verify:    from-manual
topic:     architecture
claim:     OS-9 system calls are divided into three categories: (1) User Mode Function Requests (symbolic names beginning with "F$") — memory management, multiprogramming, etc., mainly processed by the kernel; (2) I/O Requests (symbolic names beginning with "I$") — I/O functions processed in file managers and device drivers; (3) System Mode Function Requests (symbolic names beginning with "U$") — special calls for system software in supervisor state, operating on internal OS-9 data structures, not accessible to user programs.
context:   Parameters for system calls are usually passed and returned in registers.
source:    the independent 1985-era OS-9/68000 technical manual, §3-1 "Kernel System Call Processing"
--- END ---

--- CARD ---
id:        sysgo-first-user-process
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     system-initialization
claim:     SYSGO is the first user process started after the system startup sequence. Its standard I/O is on the system console device. SYSGO typically performs additional high-level system initialization, starts the console shell or other program, and remains in a wait state during system operations as insurance against all processes terminating. If the console terminal process terminates, SYSGO can restart it.
context:   On non-disk systems, a custom SYSGO module can be created to replace the standard disk-based version.
source:    the independent 1985-era OS-9/68000 technical manual, §2-5 "SYSGO"
--- END ---

--- CARD ---
id:        init-module-configuration
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     system-initialization
claim:     The INIT module is an initialization table used by the kernel during system startup. It specifies initial table sizes, initial system device names, and other startup configuration parameters.
context:   INIT is part of Level 1 modularity (kernel, clock module, and INIT) and enables system customization at startup time.
source:    the independent 1985-era OS-9/68000 technical manual, Introduction "Level 1 - The Kernel..."
--- END ---

--- CARD ---
id:        clock-module-real-time-handler
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     system-initialization
claim:     The Clock Module is a software handler for the specific real-time-clock hardware. It manages the system's hardware clock and provides timing services to the operating system.
context:   The clock module is system-dependent and must be configured for each specific hardware platform.
source:    the independent 1985-era OS-9/68000 technical manual, Introduction "Level 1 - The Kernel..."
--- END ---

--- CARD ---
id:        reset-initial-ssp-vector-0
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions
claim:     Exception vector 0 contains the reset initial SSP (supervisor stack pointer) value. This vector provides the address loaded into the system's stack pointer at startup. There must be at least 4K of RAM below and at least 4K of RAM above this address for system global storage. Each time any exception occurs, OS-9 uses this vector to find the base address of system global data.
context:   User programs should not use or alter this vector.
source:    the independent 1985-era OS-9/68000 technical manual, §4-8 "Reset Vectors"
--- END ---

--- CARD ---
id:        reset-initial-pc-vector-1
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions
claim:     Exception vector 1 contains the reset initial PC (program counter). This is the coldstart entry point to OS-9. Its only use after startup is to reset after a catastrophic failure.
context:   User programs should not use or alter this vector.
source:    the independent 1985-era OS-9/68000 technical manual, §4-8 "Reset Vectors"
--- END ---

--- CARD ---
id:        error-exception-handling-non-fatal-vs-fatal
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     exceptions
claim:     Error exceptions (vectors 2-8, 10-24, 48-63) are usually considered fatal program errors and cause unconditional process termination. However, the F$STrap system call may be used to install a user subroutine to catch errors in this group that are considered non-fatal. When an error exception occurs in user state, the routine is executed with a pointer to the normal data space and all user registers stacked, allowing the exception handler to decide whether and where to continue execution.
context:   If an error exception occurs in system state, it usually means a system call has been passed bad data, and an error is returned. System data structures can be damaged by passing nonsense parameters to system calls.
source:    the independent 1985-era OS-9/68000 technical manual, §4-7 "Error Exceptions"
--- END ---

--- CARD ---
id:        f-strap-error-exception-handler-installation
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     exceptions
claim:     The F$STrap system call is used to install a user subroutine to catch error exceptions in the groups that are considered non-fatal (vectors 2-8, 10-24, 48-63). When an error exception occurs in user state, the installed handler routine is executed with a pointer to the normal data space used by the process and all user registers stacked. The exception handler must decide whether and where to continue execution.
context:   This allows user programs to handle certain exceptions that would otherwise terminate the process.
source:    the independent 1985-era OS-9/68000 technical manual, §4-8 "Error Exceptions"
--- END ---

--- CARD ---
id:        f-irq-interrupt-handler-installation
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     exceptions
claim:     The F$IRQ system call is used to install an interrupt handler in the system's interrupt tables. Multiple devices may be used on the same interrupt vector if necessary. Interrupt service routines are executed in system state without any associated current process. The device driver must provide an error entry point for the system to execute if any error exceptions occur during interrupt processing.
context:   This enables flexible interrupt handler management and supports multiple devices on the same interrupt level.
source:    the independent 1985-era OS-9/68000 technical manual, §4-10 "Vectored Interrupts"
--- END ---

--- CARD ---
id:        position-independent-code-requirement-assembly
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     modules
claim:     In assembly language programming, the programmer must ensure position-independence by avoiding addressing modes that refer to absolute addresses. OS-9 compilers and interpreters generate position-independent code automatically, but hand-written assembly language requires manual discipline.
context:   Alternatives to absolute addressing are described in the "OS9/68000 MACRO ASSEMBLER USER'S MANUAL".
source:    the independent 1985-era OS-9/68000 technical manual, §1-3 "Module Requirements"
--- END ---

--- CARD ---
id:        timeslicing-cpu-sharing-mechanism
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     process-management
claim:     Timeslicing is the mechanism by which OS-9 allows processes to share CPU time with all other active processes. It is implemented by using both hardware and software functions. The system's CPU is interrupted by a real-time clock at regular intervals (ticks), allowing OS-9 to suspend one program and begin execution of another without affecting the program's execution state. Each second of CPU time is sliced up to be shared among several processes. To a human observer, all processes appear to execute continuously unless the computer becomes overloaded.
context:   Timeslicing happens so rapidly (typically 100 times per second) that concurrent execution appears simultaneous to users.
source:    the independent 1985-era OS-9/68000 technical manual, §4-1 "Overview of Multitasking"
--- END ---

--- CARD ---
id:        f-exit-process-termination
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     process-management
claim:     F$Exit is the means by which a process can terminate itself. When called, it closes all open paths, deallocates the process's data memory area, and unlinks its primary module and user trap handlers. If the parent is waiting via F$Wait, the parent is moved to the active queue and informed of the child's death. The status code passed to F$Exit is returned to the parent. Processes should only return an OS-9 error code or zero if no error occurred.
context:   The parent must perform an F$Wait before the process descriptor is returned to the system. If the parent is dead, the process descriptor is freed immediately.
context:   Input: d1.w = status code to be returned to parent. Output: process is terminated.
source:    the independent 1985-era OS-9/68000 technical manual, §14-12 "F$Exit"
--- END ---

--- CARD ---
id:        f-exit-only-unlinks-primary-module
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     process-management
claim:     F$Exit only unlinks the primary module and user trap handlers. Any other modules that are loaded or linked by the process should be explicitly unlinked before calling F$Exit, or they will not be deallocated when the process terminates.
context:   This is important for proper resource cleanup.
source:    the independent 1985-era OS-9/68000 technical manual, §14-12 "F$Exit Caveats"
--- END ---

--- CARD ---
id:        user-memory-area-variable-storage
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     memory-management
claim:     Each process's user memory area (data area) is a separate memory space where all program variables are kept. The top part of this area is used for the program's stack. The actual memory addresses assigned to the data area are not known at program-write time; a base address is kept in a register (usually a6 by convention) to access the data area.
context:   The data area is read/write accessible, unlike the read-only program code area.
source:    the independent 1985-era OS-9/68000 technical manual, §4-1 "Process Memory Areas"
--- END ---

--- CARD ---
id:        program-parameter-passing-mechanism
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     process-management
claim:     When the shell processes a command line, it passes a string in the parameter area which is a copy of the parameter part (if any) of the command line. The shell inserts an end-of-line character at the end of the parameter string to simplify string-oriented processing. The parameter size is passed in d5.l when the child process is created.
context:   This standard parameter passing mechanism allows programs to receive command-line arguments from the shell.
source:    the independent 1985-era OS-9/68000 technical manual, §14-14 "F$Fork"
--- END ---
