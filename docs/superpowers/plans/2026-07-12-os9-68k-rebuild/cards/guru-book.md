# Extracted Facts from The OS-9 Guru

--- CARD ---
id:        os9-name-origin
type:      FACT
target:    all
verify:    from-manual
topic:     history
claim:     OS-9 abbreviation stands for "Operating System for the 6809 microprocessor"; originally written by Microware under contract to Motorola.
context:   OS-9/6809 is jointly owned by Microware and Motorola; the 68000 version is the modern canonical form.
source:    The OS-9 Guru, section 1.1, page 1
--- END ---

--- CARD ---
id:        os9-architecture-scope
type:      FACT
target:    68k
verify:    from-manual
topic:     history
claim:     OS-9 was rewritten for the 68000 family of microprocessors and reached wide popularity on industrial markets in this form.
context:   OS-9/6809 was popular but the 68000 version is the focus of this book (v2.4, equivalent to CD-RTOS).
source:    The OS-9 Guru, section 1.1, page 1
--- END ---

--- CARD ---
id:        multitasking-mechanism
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     scheduler
claim:     OS-9 performs automatic time-slicing: runs one program for a short duration (typically 20ms), then saves its state and executes the next program, giving the illusion of concurrent execution.
context:   The time-slice length is chosen short enough for acceptable concurrency without excessive scheduler overhead.
source:    The OS-9 Guru, section 1.3, page 3
--- END ---

--- CARD ---
id:        real-time-definition
type:      CONCEPT
target:    all
verify:    from-manual
topic:     real-time
claim:     Real-time means real-world events are processed as they happen; a real-time system must respond to external events within a specified time limit for each event type.
context:   Response time varies by application and event; can be microseconds or minutes. Non-real-time systems also respond within time but it's not critical.
source:    The OS-9 Guru, section 1.7.2, page 6
--- END ---

--- CARD ---
id:        interrupt-vs-polling
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     interrupts
claim:     Interrupt-driven I/O is far more efficient and reliable than polling; requires the OS to support adding new interrupt handlers and enable communication between handlers and waiting programs.
context:   Polling delays response by requiring a program to test a condition repeatedly; interrupts directly signal the processor when an event occurs.
source:    The OS-9 Guru, section 1.7.2, page 7
--- END ---

--- CARD ---
id:        68000-sync-word-magic-number
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Module headers begin with $4AFC, an illegal instruction in the 68000 family, used as a magic number for rapid module detection in ROM during coldstart.
context:   The kernel scans ROM word-by-word and only attempts CRC/parity checks when this sync word is found, saving startup time.
source:    The OS-9 Guru, section 3.2.1, page 35
--- END ---

--- CARD ---
id:        module-concept
type:      CONCEPT
target:    all
verify:    from-manual
topic:     modules
claim:     An OS-9 module is a program, data structure, OS component, or byte string with an identifying header and CRC; modules are the fundamental unit of OS-9's dynamic configurability.
context:   Programs locate modules by name via the module directory, similar to file access; allows ROM-ability, re-entrancy, and dynamic loading.
source:    The OS-9 Guru, section 3.1, page 33
--- END ---

--- CARD ---
id:        module-directory
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     The module directory is a kernel-maintained table with one entry per loaded module; each entry holds module address, link count, group identifier, and parity check value.
context:   Module lookup compares requested name against module header; link count tracks active uses; group ID ties modules loaded from same file.
source:    The OS-9 Guru, section 3.1, page 33
--- END ---

--- CARD ---
id:        position-independence-requirement
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     All OS-9 programs must be position-independent (no absolute program addresses); Motorola 68000 processors are designed to support this via relative addressing.
context:   Enables modules to load anywhere in free memory without modification; prerequisite for ROMmability and re-entrancy.
source:    The OS-9 Guru, section 1.9, page 14
--- END ---

--- CARD ---
id:        register-indirect-addressing
type:      FACT
target:    68k
verify:    from-manual
topic:     memory
claim:     OS-9 programs access data memory "register indirect": OS-9 loads a 68000 processor register with the base address of program's allocated data area, program accesses variables relative to this register.
context:   Enables ROM-resident program code with RAM variables; supports multiple concurrent process incarnations using same program module but separate data spaces.
source:    The OS-9 Guru, section 1.9, page 14
--- END ---

--- CARD ---
id:        reentrant-program-definition
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     modules
claim:     A re-entrant program makes all data accesses relative to a processor register set by OS-9; this allows multiple simultaneous incarnations of the program to run independently.
source:    The OS-9 Guru, section 1.9, page 15
--- END ---

--- CARD ---
id:        module-type-codes
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     Module type field (high byte of type/language word) reserved codes: 1=program, 2=subroutine, 4=data module, 11=trap handler, 12=OS component, 13=file manager, 14=device driver, 15=device descriptor.
context:   Kernel validates type code against usage; attempting fork of non-program-type module returns E_NEMOD.
source:    The OS-9 Guru, section 3.2.7, page 39
--- END ---

--- CARD ---
id:        module-language-codes
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     Module language field (low byte of type/language word) reserved codes: 1=object/machine code executable, 2=compiled BASIC, 3=Pascal, 4=C, 5=Cobol, 6=Fortran (codes 3-6 currently undefined for execution).
context:   Shell uses language code to fork appropriate interpreter for intermediate code programs.
source:    The OS-9 Guru, section 3.2.7, pages 39-40
--- END ---

--- CARD ---
id:        module-sharable-attribute
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     Module attribute bit 7 (sharable flag): when set, kernel permits link count greater than one; when clear, only one simultaneous link allowed.
context:   Typically set for all modules; if clear on device descriptor, only one path can open device simultaneously; if clear on program module, program is not re-entrant and cannot self-modify.
source:    The OS-9 Guru, section 3.2.8, page 41
--- END ---

--- CARD ---
id:        module-sticky-attribute
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     Module attribute bit 6 (sticky flag): when set, module remains in memory even when link count reaches zero; further reduction removes from directory; memory pressure may evict sticky modules.
context:   Used for commonly used utilities to remain in memory between uses, reducing disk access.
source:    The OS-9 Guru, section 3.2.8, pages 41-42
--- END ---

--- CARD ---
id:        module-supervisor-state-attribute
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Module attribute bit 5 (supervisor state flag): when set, module runs in 68000 supervisor state; clear means user state; set for all OS components.
context:   Supervisor state accesses all memory and can execute privileged instructions; user state is restricted for protection in multi-user systems.
source:    The OS-9 Guru, section 3.2.8, page 41
--- END ---

--- CARD ---
id:        68000-supervisor-vs-user-state
type:      FACT
target:    68k
verify:    from-manual
topic:     processor
claim:     68000 family has two operating states: supervisor (for system code) and user (for application programs); status register bit indicates which state; certain instructions forbidden in user state.
context:   System calls and interrupts automatically enter supervisor state; memory protection hardware can restrict user-state program memory access.
source:    The OS-9 Guru, section 3.2.8, page 41
--- END ---

--- CARD ---
id:        module-revision-override-mechanism
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     modules
claim:     When a module with same name, type, and language loads, kernel compares revision numbers; higher revision overwrites module directory entry; higher revision does NOT wait for old module's link count to drop to zero.
context:   Allows updating modules in ROM from boot file, but requires careful usage to avoid using old module after replacement.
source:    The OS-9 Guru, section 3.2.8, pages 42-43
--- END ---

--- CARD ---
id:        module-access-permissions-field
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     Module access permissions use low 12 bits of word: bits 0-2 (private), 4-6 (group), 8-10 (public), each with read(0)/write(1)/execute(2) flags; bit 3,7,11 reserved.
context:   read/execute allow load/link/unlink/fork; write affects write-to-module capability on systems with memory protection.
source:    The OS-9 Guru, section 3.2.6, page 37
--- END ---

--- CARD ---
id:        module-crc-validation
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     Modules end with 24-bit CRC; linker adds zero padding byte to ensure even length (68000 requirement); kernel checks CRC during module install via F$CRC system call.
context:   Linker generates CRC over entire module header+body+padding; accumulator initialized to $FFFFFF, final value one's-complemented; valid result is $800FE3 when CRC bytes included in calculation.
source:    The OS-9 Guru, section 3.2.18, pages 47-48
--- END ---

--- CARD ---
id:        module-header-parity
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Module header parity word is one's complement of word-by-word XOR of all preceding header words; valid header when XOR of all header words including parity yields $FFFF.
context:   Protects against header corruption; kernel recalculates header checksum on module link and returns E_BMHP if mismatch detected.
source:    The OS-9 Guru, section 3.2.11, page 43
--- END ---

--- CARD ---
id:        module-header-checksum-algorithm
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     In OS-9 v2.2, module header checksum is low 16 bits of sum of all words including parity; in later versions, after each word addition the result is rotated right by (word value mod 16) bits.
context:   More complex rotation algorithm in v2.4+ provides better corruption detection.
source:    The OS-9 Guru, section 3.2.11, page 44
--- END ---

--- CARD ---
id:        data-initialization-table-format
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     Data initialization table (built by linker): each entry has start offset in static storage (long), size in bytes (long), followed by initialization bytes; kernel copies this data to static storage on fork.
context:   Implements C static variable initialization at program startup.
source:    The OS-9 Guru, section 3.2.16, page 45
--- END ---

--- CARD ---
id:        data-pointers-initialization-table-format
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     Data pointers initialization table (two sub-tables): first table initializes pointers to program functions (add module base address); second table initializes pointers to static storage (add static storage base address).
context:   Each sub-table: high word offset, word count, then list of low word offsets; terminated by zero count.
source:    The OS-9 Guru, section 3.2.17, page 46
--- END ---

--- CARD ---
id:        program-module-entry-point-offset
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     Program entry point offset (execution offset in module header) is offset from module header start to first instruction; kernel calculates absolute address by adding this to module's memory address.
context:   Entry point need not be at module start; linker reads entry point from "root" psect in linked files.
source:    The OS-9 Guru, section 3.2.12, page 44
--- END ---

--- CARD ---
id:        trap-entry-point-offset
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Default trap entry point offset in module header: kernel calls this routine if program uses 68000 TRAP instruction without installing handler module for that trap number.
source:    The OS-9 Guru, section 3.2.13, page 44
--- END ---

--- CARD ---
id:        module-minimum-stack-size
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     Linker sets minimum stack size field to assumed maximum stack depth; cannot calculate actual maximum (recursion is unbounded); default is 3k bytes, overridable via linker option.
context:   Kernel adds stack size + data size + parameter string size + additional stack parameter to calculate total memory needed for fork.
source:    The OS-9 Guru, section 3.2.15, page 45
--- END ---

--- CARD ---
id:        c-compiler-startup-file
type:      FACT
target:    68k
verify:    from-manual
topic:     compilation
claim:     C compiler adds 'LIB/cstart.r' at front of link file list; cstart.r is root psect containing initialization code that calls main() function.
context:   cstart.r created from 'C/SOURCE/cstart.a'; provides C program entry point and initialization.
source:    The OS-9 Guru, section 3.2.12, page 44
--- END ---

--- CARD ---
id:        module-groups-memory-allocation
type:      CONCEPT
target:    all
verify:    from-manual
topic:     modules
claim:     Multiple modules from same file form a "module group"; group identifier is address of first module; all modules in group remain in directory until entire group's link count reaches zero.
context:   Solves problem of non-aligned module sizes preventing individual freeing; ensures contiguous memory blocks stay valid for memory management.
source:    The OS-9 Guru, section 3.3.1, page 49
--- END ---

--- CARD ---
id:        memory-minimum-allocation-unit
type:      FACT
target:    all
verify:    from-manual
topic:     memory
claim:     OS-9 allocates memory in multiples of 16-byte minimum allocation unit; areas of free memory linked together with controlling structure at start of area.
context:   16-byte structure minimum needed for free-list management; smaller areas cannot be freed.
source:    The OS-9 Guru, section 3.3.1, page 49
--- END ---

--- CARD ---
id:        process-memory-allocation-limit
type:      FACT
target:    all
verify:    from-manual
topic:     memory
claim:     Program can dynamically allocate up to 32 separate memory areas (including initial static storage and stack); kernel tracks these for automatic cleanup on exit.
context:   Prevents runaway allocation; contiguous areas are merged to maximize 32-entry usage.
source:    The OS-9 Guru, section 3.4, page 50
--- END ---

--- CARD ---
id:        coloured-memory-concept
type:      CONCEPT
target:    all
verify:    from-manual
topic:     memory
claim:     Coloured memory (OS-9 v2.3+) allows differentiating memory areas by type (e.g., display RAM, battery-backed RAM) and priority; programs request specific colors; multiple programs can allocate separate areas of same color.
context:   Solves problems of mixed memory speeds and special-property memory areas (graphics, inter-processor comms); colors have associated priorities; high-priority memory allocated first.
source:    The OS-9 Guru, section 3.4.1, page 51
--- END ---

--- CARD ---
id:        coloured-memory-allocation-calls
type:      FACT
target:    all
verify:    from-manual
topic:     memory
claim:     F$SRqMem (C: _srqmem()) allocates uncoloured memory; F$SRqCMem (C: _srqcmem()) allocates coloured memory; F$Load/F$DatMod have extended formats for color specification.
context:   Uncoloured allocation uses priority order across all colors; zero-priority memory can only be allocated via coloured call.
source:    The OS-9 Guru, section 3.4.1, page 52
--- END ---

--- CARD ---
id:        memory-fragmentation-rare-but-possible
type:      GOTCHA
target:    all
verify:    from-manual
topic:     memory
claim:     Without MMU address translation, separate physical memory areas cannot be combined into single logical area; fragmentation is rare in practice unless application is very tight on memory or allocates very large blocks.
context:   OS-9 design avoids most fragmentation problems through careful allocation strategies.
source:    The OS-9 Guru, section 3.4, page 50
--- END ---

--- CARD ---
id:        mmu-not-required-for-basic-os9
type:      FACT
target:    all
verify:    from-manual
topic:     memory
claim:     OS-9 does not require a memory management unit (MMU); programs see actual physical memory addresses; register-indirect addressing provides necessary isolation.
context:   MMU optional for memory protection; can be added via System Security Module (ssm).
source:    The OS-9 Guru, section 1.9, page 14
--- END ---

--- CARD ---
id:        inter-task-memory-protection-via-mmu
type:      FACT
target:    68k
verify:    from-manual
topic:     memory
claim:     OS-9 v2.2+ can use MMU for inter-task memory protection via System Security Module (ssm); prevents programs accessing unallocated memory; OS does not use MMU for address translation.
context:   SSM chips handle MMU functions; kernel suspends protections for supervisor-state accesses.
source:    The OS-9 Guru, section 3.4.3, page 55
--- END ---

--- CARD ---
id:        system-minimum-allocatable-block-size
type:      FACT
target:    68k
verify:    from-manual
topic:     memory
claim:     System minimum allocatable block size for MMU (typical 4k bytes) differs from OS-9 logical block size (16 bytes); kernel allocates physical blocks and manages free fragments.
context:   Prevents waste when program makes small allocations; kernel tracks free fragments within larger allocated blocks.
source:    The OS-9 Guru, section 3.4.3, page 56
--- END ---

--- CARD ---
id:        system-minimum-block-without-mmu
type:      FACT
target:    all
verify:    from-manual
topic:     memory
claim:     Without inter-task memory protection, system minimum allocatable block size defaults to 256 bytes.
context:   Larger than 16-byte logical unit but still reasonable for non-protected systems.
source:    The OS-9 Guru, section 3.4.3, page 56
--- END ---

--- CARD ---
id:        process-definition
type:      CONCEPT
target:    all
verify:    from-manual
topic:     processes
claim:     A process is a forked program that has not exited, together with its data memory and process descriptor (OS kernel structure); single re-entrant program can have multiple simultaneous incarnations (processes).
context:   Each process has unique ID; kernel maintains separate process descriptor for each.
source:    The OS-9 Guru, section 3.5, page 57
--- END ---

--- CARD ---
id:        process-id-assignment
type:      FACT
target:    all
verify:    from-manual
topic:     processes
claim:     Process ID returned by F$Fork uniquely identifies process; always greater than 1 (zero unused, system process uses 1); after process dies, ID may be reused; no two processes share ID simultaneously.
context:   Process ID used for all system calls that communicate with or modify specific process.
source:    The OS-9 Guru, section 3.5, page 57
--- END ---

--- CARD ---
id:        process-priority-value
type:      FACT
target:    all
verify:    from-manual
topic:     processes
claim:     Each process has priority value assigned at fork time, usually inherited from parent; can be changed via F$SPrior system call; primary mechanism for determining processor time share.
context:   Used by scheduler to determine which active processes run and their scheduling behavior.
source:    The OS-9 Guru, section 3.5, page 57
--- END ---

--- CARD ---
id:        process-states
type:      FACT
target:    all
verify:    from-manual
topic:     processes
claim:     Process states: Active (requesting processor time), Waiting (for child process), Sleeping (for time/event/IPC), Waiting for event (IPC event), Debugged (debugger permission), Dead (reporting exit status).
context:   Only active processes receive processor time; scheduler manages state transitions.
source:    The OS-9 Guru, section 3.5, page 57
--- END ---

--- CARD ---
id:        time-slice-definition
type:      FACT
target:    all
verify:    from-manual
topic:     scheduler
claim:     Time slice is processor execution unit for one process; typically multiple ticks (default 2 ticks at 10ms each = 20ms); generated by hardware timer interrupt.
context:   Two ticks per slice rather than one because OS cannot resolve sub-tick time; single-tick slice effectively vanishingly small due to rounding.
source:    The OS-9 Guru, section 3.5, page 58
--- END ---

--- CARD ---
id:        time-slice-purpose
type:      CONCEPT
target:    all
verify:    from-manual
topic:     scheduler
claim:     Time slicing creates appearance of concurrent execution; length chosen short enough for acceptable concurrency but not so short that scheduler overhead dominates.
context:   Only active processes receive time; scheduler ensures equal/even division of time unless user/programmer requests otherwise.
source:    The OS-9 Guru, section 3.5, pages 58-59
--- END ---

--- CARD ---
id:        fork-system-call
type:      FACT
target:    all
verify:    from-manual
topic:     processes
claim:     F$Fork creates new process; parent can pass parameter string to child (copied to child's static storage), specify child priority, request additional static storage beyond program module minimum.
context:   Process begins execution at program module entry point; parent continues after fork.
source:    The OS-9 Guru, section 3.5, page 58
--- END ---

--- CARD ---
id:        chain-system-call
type:      FACT
target:    all
verify:    from-manual
topic:     processes
claim:     F$Chain transforms calling process into new process executing different program; terminates calling process and reuses its process descriptor/ID for new process.
context:   Differs from fork: child reuses parent's ID rather than getting new one.
source:    The OS-9 Guru, section 3.5, page 59
--- END ---

--- CARD ---
id:        process-descriptor-table
type:      FACT
target:    all
verify:    from-manual
topic:     processes
claim:     Process descriptor table is array of process descriptor addresses; process ID is index into table; zero entry means ID unused; table starts small and kernel doubles size if full.
context:   Enables efficient process lookup and management.
source:    The OS-9 Guru, section 3.5, page 59
--- END ---

--- CARD ---
id:        dead-process-behavior
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     processes
claim:     Dead process (exited but not waited for) retains process descriptor indefinitely until parent executes F$Wait (wait for child to die) or parent dies; kernel deallocates other resources (memory, paths, module).
context:   Ensures parent can retrieve child exit status; can confuse users who expect immediate cleanup.
source:    The OS-9 Guru, section 3.5.1, page 60
--- END ---

--- CARD ---
id:        process-disinheritance
type:      FACT
target:    all
verify:    from-manual
topic:     processes
claim:     Process inherits parent by fork; parent dies before child → child disinherited; no system call for programmatic disinheritance; workaround: fork intermediate process that forks desired child then dies.
context:   Disinherited child does not become grandchild of original grandparent.
source:    The OS-9 Guru, section 3.5.1, page 60
--- END ---

--- CARD ---
id:        system-state-vs-user-state-processes
type:      FACT
target:    68k
verify:    from-manual
topic:     processes
claim:     OS components execute in 68000 supervisor state (system state); application programs execute in user state; system calls and interrupts automatically enter supervisor state.
context:   User state restricts illegal operations like interrupt masking; allows scheduling to preempt at any point.
source:    The OS-9 Guru, section 3.5.2, page 59
--- END ---

--- CARD ---
id:        scheduler-deferment-in-supervisor-state
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     scheduler
claim:     When processor in supervisor state, scheduling is deferred (process cannot be preempted); makes system calls indivisible and atomic from scheduler perspective.
context:   Prevents race conditions in OS operations.
source:    The OS-9 Guru, section 3.5.2, page 59
--- END ---

--- CARD ---
id:        tree-structured-io-system
type:      CONCEPT
target:    all
verify:    from-manual
topic:     io
claim:     OS-9 I/O system separated into file managers (logical operations), device drivers (physical I/O), and device descriptors (metadata); each file manager can work through multiple device drivers.
context:   Modular separation simplifies customization; new I/O interface only needs new device driver if fitting existing file manager class.
source:    The OS-9 Guru, section 1.8.4-8.6, pages 12-13
--- END ---

--- CARD ---
id:        device-manager-separation-of-concerns
type:      GOTCHA
target:    all
verify:    from-manual
topic:     io
claim:     File manager performs logical data manipulation (file structure, line editing); device driver performs physical I/O and knows nothing about why I/O is needed or data interpretation.
context:   This separation is key to OS-9's customizability; device driver writer needs no understanding of filing system.
source:    The OS-9 Guru, section 1.8.5, page 13
--- END ---

--- CARD ---
id:        dynamic-io-system-loading
type:      FACT
target:    all
verify:    from-manual
topic:     io
claim:     OS-9 I/O system dynamically modifiable at runtime: new file managers, device drivers, descriptors can load anytime; existing I/O interface can be repurposed with new drivers without reboot.
context:   Simplifies debugging new I/O components; manufacturers can supply device drivers with hardware loaded on demand.
source:    The OS-9 Guru, section 1.9, page 15
--- END ---

--- CARD ---
id:        device-descriptor-module-structure
type:      FACT
target:    all
verify:    from-manual
topic:     io
claim:     Device descriptor is data-only module specifying: device name (same as module name), file manager module name, device driver module name, hardware configuration (memory addresses, etc.).
context:   Enables OS to select appropriate file manager and device driver for I/O request.
source:    The OS-9 Guru, section 1.8.6, page 13
--- END ---

--- CARD ---
id:        device-independent-io-functions
type:      CONCEPT
target:    all
verify:    from-manual
topic:     io
claim:     OS-9 provides unified device-independent I/O: same functions (open, read, write) access all device types; enables I/O redirection without program modification.
context:   Program writing to terminal can have output redirected to disk file transparently.
source:    The OS-9 Guru, section 1.7.5, page 9
--- END ---

--- CARD ---
id:        trap-handler-module
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     Trap handler is module type (code 11) containing subroutines called via 68000 TRAP instruction; program specifies trap handler module names to use, then calls functions by trap number.
context:   Allows programs to call subroutines in separate module without determining its address; alternative to explicit module linking.
source:    The OS-9 Guru, section 1.8.7, page 13
--- END ---

--- CARD ---
id:        kernel-customization-modules
type:      FACT
target:    all
verify:    from-manual
topic:     kernel
claim:     Kernel customization modules listed in init configuration module; kernel calls their initialization functions on startup; can add new system calls or replace existing ones.
context:   Allows extending/modifying kernel functionality without rebuilding kernel itself.
source:    The OS-9 Guru, section 1.8.8, page 14
--- END ---

--- CARD ---
id:        rbf-disk-caching
type:      FACT
target:    all
verify:    from-manual
topic:     io
claim:     RBF (disk file manager) supports disk caching to improve performance.
source:    The OS-9 Guru, section 7.9.1, page 134
--- END ---

--- CARD ---
id:        shell-not-builtin
type:      CONCEPT
target:    all
verify:    from-manual
topic:     shell
claim:     Shell is separate program (not built into OS); reads command lines and executes programs or built-in commands; multiple shells can run concurrently for same/different users.
context:   Users can run alternate command line interpreters (e.g., mshell) instead of shell.
source:    The OS-9 Guru, section 2.2, page 21
--- END ---

--- CARD ---
id:        shell-wild-card-expansion
type:      FACT
target:    all
verify:    from-manual
topic:     shell
claim:     Shell implements wildcard file name matching: '*' matches any number of characters (or none), '?' matches single character; shell (not forked program) reads directory and expands names.
context:   Program receives expanded names as parameters; shell uses F$CmpNam system call for comparison.
source:    The OS-9 Guru, section 2.2, page 22
--- END ---

--- CARD ---
id:        shell-special-characters
type:      FACT
target:    all
verify:    from-manual
topic:     shell
claim:     Shell special characters: ';' sequential execution, '&' concurrent execution, '!' pipe output→input, '*' set priority, '#' set data space, '()' subshell, '<' redirect stdin, '>' redirect stdout, '>>' redirect stderr.
context:   Characters modify shell behavior; parameters containing spaces/special chars must be quoted.
source:    The OS-9 Guru, section 2.2, figure 3, page 23
--- END ---

--- CARD ---
id:        shell-builtin-commands
type:      FACT
target:    all
verify:    from-manual
topic:     shell
claim:     Shell built-in commands: chd/chx (change data/execution directory), kill/w/wait (process control), setenv/unsetenv (environment), setpr (priority), logout, profile, ex (chain).
context:   These change private shell state (not inherited by forked children); cannot be separate programs.
source:    The OS-9 Guru, section 2.3, page 24
--- END ---

--- CARD ---
id:        environment-variables-per-process
type:      CONCEPT
target:    all
verify:    from-manual
topic:     shell
claim:     Environment variables exist per-process in static storage, not globally; each forked process gets copy from parent; modifications are private unless passed on to children via os9exec().
context:   Different processes can have same env var with different values; enables per-process configuration.
source:    The OS-9 Guru, section 2.4, page 25
--- END ---

--- CARD ---
id:        pathlist-concept
type:      FACT
target:    all
verify:    from-manual
topic:     files
claim:     Pathlist is text string identifying device or file; if starts with '/' first element is device name (e.g. /dd); otherwise relative to current data or execution directory.
context:   Load utility uses execute directory; list utility uses data directory.
source:    The OS-9 Guru, section 2.5, page 26
--- END ---

--- CARD ---
id:        hierarchical-directory-structure
type:      FACT
target:    all
verify:    from-manual
topic:     files
claim:     RBF disk manager supports hierarchical directories; pathlist elements separated by '/'; each directory contains '.' (self) and '..' (parent); multiple '.' allowed for multi-level ascent.
context:   Root directory '..' refers to itself; extra '.' beyond hierarchy depth discarded.
source:    The OS-9 Guru, section 2.5, page 26
--- END ---

--- CARD ---
id:        current-data-and-execution-directories
type:      FACT
target:    all
verify:    from-manual
topic:     files
claim:     Each process has separate current data directory and current execution directory; child inherits from parent but changes don't affect parent; chx/chd built-in commands change shell's directories.
context:   Execute directory used when opening with execute mode; data directory otherwise.
source:    The OS-9 Guru, section 2.6, page 27
--- END ---

--- CARD ---
id:        scf-line-editing-buffer-size
type:      FACT
target:    all
verify:    from-manual
topic:     io
claim:     SCF (Sequential Character File manager) line buffer is 512 bytes maximum line length including carriage return; separate buffer allocated per open path.
context:   Line editing is SCF function, not shell function (though mshell does its own).
source:    The OS-9 Guru, section 2.7, page 28
--- END ---

--- CARD ---
id:        scf-editing-keys
type:      FACT
target:    all
verify:    from-manual
topic:     io
claim:     SCF line editing keys (customizable via tmode/xmode): [BS]/[BkSp]=delete left, [^X]=delete line, [^D]=reprint line, [^A]=redisplay from cursor, [ESC]=EOF if first char.
context:   Setting key code to zero disables feature; default assignments customizable.
source:    The OS-9 Guru, section 2.7, page 28
--- END ---

--- CARD ---
id:        signal-keys-device-driver
type:      FACT
target:    all
verify:    from-manual
topic:     io
claim:     Device driver signal keys (customizable): [^E]=abort (sends quit signal to last process using terminal), [^C]=interrupt (normally kills last process, but shell handlers it to background process).
context:   Signal delivery mechanism for I/O devices; different from system signals in IPC.
source:    The OS-9 Guru, section 2.8, page 29
--- END ---

--- CARD ---
id:        scf-terminal-queueing
type:      FACT
target:    all
verify:    from-manual
topic:     io
claim:     SCF queues concurrent terminal accesses (input/output); background process writing to terminal is put to sleep until active input request completes, then awakened.
context:   Prevents output garbling from concurrent I/O.
source:    The OS-9 Guru, section 2.9, page 30
--- END ---

--- CARD ---
id:        bootup-sequence
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     boot
claim:     Boot sequence: ROM bootstrap loads OS-9 kernel to RAM, kernel coldstart scans ROM/boot file for modules and enters in directory, links to init config module, initializes tables/paths/directories, forks initial program (sysgo), forks system process, coldstart completes and yields to programs/interrupts.
context:   System process manages wakeup of sleeping/alarmed programs; not visible to user.
source:    The OS-9 Guru, section 2.1, page 18
--- END ---

--- CARD ---
id:        bootstrap-program-role
type:      CONCEPT
target:    all
verify:    from-manual
topic:     boot
claim:     Bootstrap is self-contained ROM program that loads OS-9 kernel (from ROM or disk boot file) into RAM and jumps to kernel coldstart; is not part of OS-9, cannot use OS facilities.
context:   Must be adapted per hardware; provided as example source code by Microware for implementor customization.
source:    The OS-9 Guru, section 2.1, page 17
--- END ---

--- CARD ---
id:        sysgo-initial-program
type:      FACT
target:    all
verify:    from-manual
topic:     boot
claim:     Sysgo is Microware-provided initial program forked by kernel coldstart; changes to CMDS directory, forks shell to process startup file, then forks/waits for shell in loop for command prompt.
context:   Startup file contains system initialization commands; if user terminates shell, sysgo restarts it.
source:    The OS-9 Guru, section 2.1, page 19
--- END ---

--- CARD ---
id:        startup-file-private-changes
type:      GOTCHA
target:    all
verify:    from-manual
topic:     boot
claim:     Startup file shell incarnation is separate from command-prompt shell incarnation; private changes in startup (current directories, prompt string) don't carry to command shell.
context:   Current directories (chx/chd) and prompt changes (via -p=...) are process-private.
source:    The OS-9 Guru, section 2.1, page 19
--- END ---

--- CARD ---
id:        default-device-concept
type:      FACT
target:    all
verify:    from-manual
topic:     io
claim:     '/dd' (default device) is alias for primary storage device (hard disk or RAM disk); only one dd descriptor loaded at a time; programs use '/dd' for config/data files; can be changed by loading device descriptor.
context:   Usually hard disk; some configs use RAM disk; loading dd changes require new boot file/ROM or higher revision descriptor.
source:    The OS-9 Guru, section 2.10, page 30
--- END ---

--- CARD ---
id:        typical-device-names
type:      FACT
target:    all
verify:    from-manual
topic:     io
claim:     Typical OS-9 device names: /d0 /d1 (floppies), /h0 /hOfmt (hard disk, with/without format protect), /mt0 (tape), /nil (null device), /p /p1 /p2 (parallel/serial printers), /r0 (RAM disk), /term /t1 /t2 (terminals).
context:   All device names begin with '/'; device name = device descriptor module name.
source:    The OS-9 Guru, section 2.1, figure 2, page 21
--- END ---

--- CARD ---
id:        directory-structure-convention
type:      FACT
target:    all
verify:    from-manual
topic:     files
claim:     Typical OS-9 root directories: CMDS (utilities), DEFS (headers), LIB (libraries), SYS (management files), SYSMODS (system sources), IO (device descriptors/drivers); CMDS contains BOOTOBJS (customized boot modules).
context:   Not absolute requirement but Microware example followed by most implementors.
source:    The OS-9 Guru, section 2.10, page 30
--- END ---

--- CARD ---
id:        rbf-long-filenames
type:      FACT
target:    all
verify:    from-manual
topic:     files
claim:     RBF disk file manager supports hierarchical directories, long file names (up to 28 characters), almost unlimited file size, and true record locking.
context:   More sophisticated than contemporary operating systems.
source:    The OS-9 Guru, section 1.7.9, page 10
--- END ---

--- CARD ---
id:        module-link-count-semantics
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     Module link count tracks number of active users; F$Link increments count, F$UnLink decrements; when link count reaches zero, module removed from directory (unless sticky); when count goes to -1 for sticky module, module is finally removed.
context:   Implicitly loaded programs (via fork) have link count incremented; explicit load also increments.
source:    The OS-9 Guru, section 3.3, page 48
--- END ---

--- CARD ---
id:        module-file-format
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     File containing module can hold one or more modules merged sequentially; linker generates module header and CRC; files can be disk files or loaded to memory by load utility.
context:   To disk file manager, module file indistinguishable from other files.
source:    The OS-9 Guru, section 3.3, page 48
--- END ---

--- CARD ---
id:        fork-module-loading-behavior
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     processes
claim:     F$Fork attempts to find program module in directory; if not found, loads file of same name from execution directory; if wrong type/language in directory, returns E_NEMOD rather than attempting load.
context:   Prevents accidentally loading executable version when non-object module exists.
source:    The OS-9 Guru, section 3.3, page 48
--- END ---

--- CARD ---
id:        68000-instruction-word-alignment
type:      FACT
target:    68k
verify:    from-manual
topic:     processor
claim:     68000 family requires all instruction words on even byte addresses; linker adds zero padding byte at end of module body before CRC to ensure even length.
context:   Hardware constraint requiring runtime enforcement.
source:    The OS-9 Guru, section 3.2.18, page 47
--- END ---

--- CARD ---
id:        os9-written-in-68000-assembly
type:      FACT
target:    68k
verify:    from-manual
topic:     implementation
claim:     OS-9 written in 68000 family assembly language for speed/size optimization; this prevents porting to other processor families; Microware wrote OS-9000 in C for portability to 80386/80486.
context:   Design choice reflects era when processor speed and memory cost were critical factors.
source:    The OS-9 Guru, section 1.7.7, page 10
--- END ---

--- CARD ---
id:        crc-calculation-processor-cost
type:      GOTCHA
target:    all
verify:    from-manual
topic:     modules
claim:     CRC calculation requires significant processor effort; time cost is imperceptible during module load but high-performance applications should pre-load needed modules before application start.
context:   Better practice anyway to reduce disk usage in time-critical apps.
source:    The OS-9 Guru, section 3.2.18, page 48
--- END ---

--- CARD ---
id:        module-corruption-header-protection
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     Kernel rechecks module header checksum on link (not CRC of whole module) and returns E_BMHP if header corrupted; module body can be altered (for debugger breakpoints) but not header.
context:   Protects against disastrous system errors from corrupted module headers; allows data modules and debugging.
source:    The OS-9 Guru, section 3.2.11, page 44
--- END ---

--- CARD ---
id:        user-group-privilege-mechanism
type:      FACT
target:    all
verify:    from-manual
topic:     security
claim:     Only group-zero (super-user) modules can make certain system calls; programmers can compile privileged programs giving controlled resource access to non-privileged users.
context:   Enables secure delegation of system capabilities.
source:    The OS-9 Guru, section 3.2.4, page 37
--- END ---

--- CARD ---
id:        super-user-group-zero
type:      FACT
target:    all
verify:    from-manual
topic:     security
claim:     Super-user group is group zero; any user in group zero can link modules regardless of group permission bits; highest privilege level in OS-9.
context:   Separate from individual user IDs within group.
source:    The OS-9 Guru, section 3.2.4, page 37
--- END ---

--- CARD ---
id:        module-access-permission-levels
type:      FACT
target:    all
verify:    from-manual
topic:     security
claim:     Module access checks three levels: (1) same user+group→private perms, (2) same group→group perms, (3) different group→public perms; group-zero always gets highest privilege check.
context:   User ID compared before group ID; public check is fallback.
source:    The OS-9 Guru, section 3.2.4, page 37
--- END ---

--- CARD ---
id:        68000-trap-instruction
type:      FACT
target:    68k
verify:    from-manual
topic:     processor
claim:     68000 TRAP instruction used for software exceptions; OS-9 allows programs to install trap handlers (modules) for TRAP #1-#15; if no handler installed, default trap entry point in program module called.
context:   TRAP #0 reserved for OS-9 system calls.
source:    The OS-9 Guru, section 10.4, page 202
--- END ---

--- CARD ---
id:        system-calls-trap-zero
type:      FACT
target:    68k
verify:    from-manual
topic:     system-calls
claim:     OS-9 system calls implemented via 68000 TRAP #0 instruction; system call number passed in register; kernel routes to appropriate function.
context:   All kernel services accessed through TRAP #0 mechanism.
source:    The OS-9 Guru, section 10.3, page 200
--- END ---

--- CARD ---
id:        process-parameter-passing
type:      FACT
target:    all
verify:    from-manual
topic:     processes
claim:     Parent process can pass parameter string to child via F$Fork; string copied to top of child's data space (static storage); combined with environment variables from parent.
context:   Parameter string includes command line args and environment variables.
source:    The OS-9 Guru, section 3.5, page 58
--- END ---

--- CARD ---
id:        ro
type:      ROSETTA
target:    68k
verify:    from-manual
topic:     terminology
claim:     UNIX "fork" and "exec" operations combined into single OS-9 F$Fork; UNIX shared text/separate data accomplished by OS-9 re-entrant modules with register-indirect data access.
context:   OS-9 model simpler for implementor; achieved similar goals with different mechanism.
source:    The OS-9 Guru, section 1.9, page 15
--- END ---
