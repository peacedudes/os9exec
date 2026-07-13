--- CARD ---
id:        module-fundamental-unit
type:      CONCEPT
target:    all
topic:     modules
claim:     The module is the fundamental unit of OS-9: all executable code, device drivers, file managers, and system components are modules stored in memory.
context:   Unlike file systems, modules are already in memory and activated by name, not pathname.
source:    The OS-9 Primer, Chapter 5: "OS-9 Memory Modules"
--- END ---
--- CARD ---
id:        module-three-sections
type:      FACT
target:    68k
topic:     modules
claim:     Every OS-9 module consists of three sections: header (at least 48 bytes), body (type-specific content), and CRC (cyclic redundancy checksum).
context:   -
source:    The OS-9 Primer, Chapter 5: "Module Format"
--- END ---
--- CARD ---
id:        module-header-minimum
type:      FACT
target:    68k
topic:     modules
claim:     The first 48 bytes of a module header follow the modhcom structure defined in module.h and are standardized across all OS-9 module types.
context:   Module headers may be larger than 48 bytes depending on type.
source:    The OS-9 Primer, Chapter 5: "Module Header"
--- END ---
--- CARD ---
id:        module-name-arbitrary-size
type:      FACT
target:    68k
topic:     modules
claim:     A module name is variable length and located within the module body; the header contains only an offset pointing to where the name begins.
context:   -
source:    The OS-9 Primer, Chapter 5: "Name"
--- END ---
--- CARD ---
id:        module-link-count
type:      FACT
target:    68k
topic:     modules
claim:     Each module maintains a link count indicating how many processes or devices are currently using it; modules are removed from memory when their link count reaches zero.
context:   Link count is not guaranteed to be accurate due to commands like link/unlink that can artificially affect it.
source:    The OS-9 Primer, Chapter 5: "Link Count"
--- END ---
--- CARD ---
id:        module-edition
type:      FACT
target:    68k
topic:     modules
claim:     A module edition is a 16-bit unsigned value assigned at module creation; it aids users in identifying module versions and is checked when loading module updates from disk.
context:   -
source:    The OS-9 Primer, Chapter 5: "Edition"
--- END ---
--- CARD ---
id:        module-reentrant
type:      FACT
target:    68k
topic:     modules
claim:     A reentrant module is one where code does not change during execution and does not contain changeable variables embedded in the code section; reentrant modules can be safely used by multiple processes simultaneously.
context:   Microware compilers typically generate reentrant code; assembly code should be reentrant but that responsibility falls on the programmer.
source:    The OS-9 Primer, Chapter 5: "Attribute and Revisions"
--- END ---
--- CARD ---
id:        module-sticky
type:      FACT
target:    68k
topic:     modules
claim:     A sticky (or "ghost") module remains in memory even after its link count reaches zero; sticky modules are only removed when the system needs the memory or explicitly unlinked to -1.
context:   Frequently used utilities are marked sticky to avoid repeated load overhead.
source:    The OS-9 Primer, Chapter 5: "Attribute and Revisions"
--- END ---
--- CARD ---
id:        module-system-state
type:      FACT
target:    68k
topic:     modules
claim:     A module attribute specifies whether it runs in system state (supervisory mode on 68k CPU) or user state; system state is reserved for device drivers and OS extensions, while most utilities are user state.
context:   -
source:    The OS-9 Primer, Chapter 5: "Attribute and Revisions"
--- END ---
--- CARD ---
id:        module-revision-update-logic
type:      BEHAVIOR
target:    68k
topic:     modules
claim:     When loading a module from disk with the load command, if a module with the same name already exists in memory, the revision is compared; if the disk version has higher revision, it replaces the in-memory version.
context:   Existing processes continue using the old module; only new requests get the new version. Existing programs are not affected by the replacement.
source:    The OS-9 Primer, Chapter 5: "Attribute and Revisions"
--- END ---
--- CARD ---
id:        module-type-language-field
type:      FACT
target:    68k
topic:     modules
claim:     A module type/language field is a 16-bit value combining the module type (8 upper bits) and language (8 lower bits); common types are Program, Data, File Manager, Device Driver, Device Descriptor, Trap Handler, Subroutine Module, and System Module.
context:   -
source:    The OS-9 Primer, Chapter 5: "Type"
--- END ---
--- CARD ---
id:        module-permissions-unix-like
type:      FACT
target:    68k
topic:     modules
claim:     Module permissions are represented as three octal digits: owner rwx, group rwx, world rwx (similar to Unix file permissions); individual read/write/execute bits can be set per group.
context:   -
source:    The OS-9 Primer, Chapter 5: "Perm"
--- END ---
--- CARD ---
id:        module-crc-integrity
type:      BEHAVIOR
target:    68k
topic:     modules
claim:     Before a module can be entered into the module directory, it must pass a CRC check: the computed CRC must match the CRC stored in the module; corrupted modules are refused.
context:   -
source:    The OS-9 Primer, Chapter 5: "Module CRC"
--- END ---
--- CARD ---
id:        mdir-command
type:      FACT
target:    68k
topic:     modules
claim:     The mdir command displays all modules currently in memory; mdir -e shows extended information including address, size, owner, permissions, type, revisions, edition, link count, and module name.
context:   -
source:    The OS-9 Primer, Chapter 5: "Module Header"
--- END ---
--- CARD ---
id:        module-address-position-independent
type:      FACT
target:    68k
topic:     modules
claim:     Module address is assigned when loaded and may differ across successive loads; OS-9 modules are typically built position-independent so this is not a problem.
context:   -
source:    The OS-9 Primer, Chapter 5: "Address"
--- END ---
--- CARD ---
id:        process-creation-fork
type:      BEHAVIOR
target:    all
topic:     process-mgmt
claim:     Child processes (tasks) are created via _os_fork() or implicitly when a program is executed; each child has its own memory space and can run concurrently.
context:   Process creation is the primary OS-9 way to achieve concurrent execution.
source:    The OS-9 Primer, Chapter 7: "Creating child tasks"
--- END ---
--- CARD ---
id:        process-descriptor-kernel
type:      CONCEPT
target:    all
topic:     process-mgmt
claim:     Each process has a Process Descriptor maintained by the kernel that tracks its state, memory, I/O paths, priority, signals, and other runtime information.
context:   -
source:    The OS-9 Primer, Chapter 7: "Process Descriptors"
--- END ---
--- CARD ---
id:        process-id-unique
type:      FACT
target:    all
topic:     process-mgmt
claim:     Each process is assigned a unique process ID (PID) by the kernel at creation time; PIDs can be reused after process termination.
context:   -
source:    The OS-9 Primer, Chapter 7: "Process identification"
--- END ---
--- CARD ---
id:        io-path-abstraction
type:      CONCEPT
target:    all
topic:     io
claim:     An I/O path is an OS-9 abstraction representing an open connection to a device or file; processes use numeric path descriptors (analogous to Unix file descriptors) to reference paths.
context:   -
source:    The OS-9 Primer, Chapter 7: "I/O paths"
--- END ---
--- CARD ---
id:        path-descriptor-convention
type:      FACT
target:    all
topic:     io
claim:     By convention, path descriptor 0 is stdin, 1 is stdout, 2 is stderr; higher-numbered paths are process-local and allocated dynamically.
context:   -
source:    The OS-9 Primer, Chapter 4: "I/O redirection"
--- END ---
--- CARD ---
id:        io-redirection-shell
type:      BEHAVIOR
target:    all
topic:     shell
claim:     The OS-9 shell supports I/O redirection: > redirects stdout, » redirects stderr, < redirects stdin; redirection modifiers send output to files or devices.
context:   -
source:    The OS-9 Primer, Chapter 4: "Modified Standard Output"
--- END ---
--- CARD ---
id:        io-append-overwrite
type:      BEHAVIOR
target:    all
topic:     shell
claim:     Shell output redirection with >- overwrites files, >+ appends to files; this prevents accidental data loss by refusing to overwrite existing files without explicit intent.
context:   -
source:    The OS-9 Primer, Chapter 4: "Redirection Appending and Overwriting"
--- END ---
--- CARD ---
id:        pipe-unnamed-90-bytes
type:      FACT
target:    68k
topic:     io
claim:     An unnamed pipe is 90 bytes by default when created without size specification; the size can be overridden via S_ISIZE option to _os_create().
context:   -
source:    The OS-9 Primer, Chapter 8: "Pipe sizes"
--- END ---
--- CARD ---
id:        pipe-deadlock-detection
type:      BEHAVIOR
target:    68k
topic:     io
claim:     If all processes with access to an unnamed pipe are blocked writing (pipe is full), OS-9 detects this potential deadlock and the first waiting process receives an error.
context:   -
source:    The OS-9 Primer, Chapter 8: "Writing to a full pipe"
--- END ---
--- CARD ---
id:        pipe-named-behavior
type:      BEHAVIOR
target:    68k
topic:     io
claim:     Writes to a full named pipe block until space is available unless a signal is received; if multiple processes are all blocked writing to a full unnamed pipe, the first gets an error.
context:   -
source:    The OS-9 Primer, Chapter 8: "Potential Problems with Pipes"
--- END ---
--- CARD ---
id:        data-module-shared-memory
type:      CONCEPT
target:    68k
topic:     io
claim:     A data module is the preferred OS-9 method for inter-process shared memory: processes reference shared data by module name, not hardcoded address.
context:   -
source:    The OS-9 Primer, Chapter 8: "Data Modules"
--- END ---
--- CARD ---
id:        data-module-creation
type:      BEHAVIOR
target:    68k
topic:     io
claim:     Data modules are created via _os_mkmodule() or _os_datmod(); the first process to create a module specifies its size, and subsequent processes link to it to access the shared memory.
context:   -
source:    The OS-9 Primer, Chapter 8: "Creating a Data Module"
--- END ---
--- CARD ---
id:        device-file-manager
type:      CONCEPT
target:    all
topic:     io
claim:     OS-9 devices are managed by device driver modules and file manager modules; a single device can have multiple open paths active simultaneously.
context:   -
source:    The OS-9 Primer, Chapter 6: "Device I/O subsystem"
--- END ---
--- CARD ---
id:        device-descriptor
type:      FACT
target:    68k
topic:     io
claim:     A device descriptor is a module that describes how a physical or logical device should be accessed; it contains device characteristics, options, and references to the device driver.
context:   -
source:    The OS-9 Primer, Chapter 6: "Device descriptors"
--- END ---
--- CARD ---
id:        signal-async-notification
type:      CONCEPT
target:    all
topic:     ipc
claim:     Signals in OS-9 are software-controlled interrupts that a process can send to another process; they are a primary mechanism for asynchronous notification and process control.
context:   -
source:    The OS-9 Primer, Chapter 9: "Signals and alarms"
--- END ---
--- CARD ---
id:        semaphore-resource-mgmt
type:      CONCEPT
target:    68k
topic:     ipc
claim:     Semaphores and events in OS-9 are used for resource management and process coordination; they prevent race conditions when multiple processes access shared resources.
context:   -
source:    The OS-9 Primer, Chapter 10: "Semaphores and events"
--- END ---
--- CARD ---
id:        preemptive-multitasking
type:      BEHAVIOR
target:    all
topic:     scheduling
claim:     OS-9 is a preemptive multitasking system: when a higher-priority process becomes ready to run, it preempts (interrupts) the currently running lower-priority process.
context:   -
source:    The OS-9 Primer, Chapter 11: "Scheduling and preemption"
--- END ---
--- CARD ---
id:        priority-aging
type:      BEHAVIOR
target:    all
topic:     scheduling
claim:     OS-9 process priority can age over time: a process waiting in the ready queue may have its priority automatically increased if it does not run for an extended period.
context:   This prevents starvation of lower-priority processes.
source:    The OS-9 Primer, Chapter 11: "Priority aging"
--- END ---
--- CARD ---
id:        round-robin-same-priority
type:      BEHAVIOR
target:    all
topic:     scheduling
claim:     Processes with the same priority share CPU time using round-robin scheduling: each process runs for a time slice, then yields to the next same-priority ready process.
context:   -
source:    The OS-9 Primer, Chapter 11: "Scheduling algorithm"
--- END ---
--- CARD ---
id:        d0-d1-return-value
type:      FACT
target:    68k
topic:     c-interface
claim:     In 68k OS-9 C code, register D0 holds the return value of a function; for 64-bit return values, D0 holds the low 32 bits and D1 holds the high 32 bits.
context:   -
source:    The OS-9 Primer, Chapter 12: "Register conventions"
--- END ---
--- CARD ---
id:        d1-first-parameter
type:      FACT
target:    68k
topic:     c-interface
claim:     Register D1 is used to pass the first parameter to an assembly routine when called from C.
context:   -
source:    The OS-9 Primer, Chapter 12: "Register conventions"
--- END ---
--- CARD ---
id:        a5-frame-pointer
type:      FACT
target:    68k
topic:     c-interface
claim:     Register A5 is the frame pointer in 68k OS-9 C code: local variables and function parameters are addressed relative to A5 set up by the link instruction.
context:   -
source:    The OS-9 Primer, Chapter 12: "Frame pointer"
--- END ---
--- CARD ---
id:        a6-global-base
type:      FACT
target:    68k
topic:     c-interface
claim:     Register A6 is the global variable pointer in 68k OS-9: it is initialized to point to the base of global variables plus 0x8000, allowing direct access to 64k of globals using A6-based addressing.
context:   The 0x8000 offset allows use of 16-bit signed offsets to reach globals in the range 0x8000..0xffff bytes from A6.
source:    The OS-9 Primer, Chapter 12: "Global variable pointer"
--- END ---
--- CARD ---
id:        link-unlk-frame-setup
type:      BEHAVIOR
target:    68k
topic:     c-interface
claim:     In 68k OS-9 assembly, link (link A5, #offset) sets up A5 as frame pointer and allocates local stack space; unlk (unlk A5) restores the prior frame pointer and deallocates locals before return.
context:   -
source:    The OS-9 Primer, Chapter 12: "Frame setup"
--- END ---
--- CARD ---
id:        asm-in-c-caveat
type:      GOTCHA
target:    68k
topic:     c-interface
claim:     The _asm() function in Ultra C compiler embeds assembly inside C code, but the compiler does not optimize assembly; embedded assembly may be inadvertently damaged by compiler optimization of surrounding C code.
context:   Do not embed assembly directly inside C function bodies; use separate assembly files or linking instead. (Starting with Ultra C 1.2, macro-form assembly inside functions is supported.)
source:    The OS-9 Primer, Chapter 12: "Placing assembly directly within a C code module"
--- END ---
--- CARD ---
id:        trap-instruction-f-sleep
type:      FACT
target:    68k
topic:     kernel
claim:     OS-9 uses trap #0 for kernel service calls; F$Sleep and other kernel functions are invoked via trap #0 (the instruction is recognized as a macro by the assembler that generates the trap).
context:   -
source:    The OS-9 Primer, Chapter 12: "Global variable pointer"
--- END ---
--- CARD ---
id:        subroutine-module-no-globals
type:      GOTCHA
target:    68k
topic:     shared-libs
claim:     Subroutine modules in OS-9 cannot access static global variables by name; all globals must be passed as parameters or obtained via pointers from the calling program.
context:   This is because subroutine modules are not linked at compile time and have no static storage area.
source:    The OS-9 Primer, Chapter 13: "Subroutine modules"
--- END ---
--- CARD ---
id:        subroutine-index-table
type:      FACT
target:    68k
topic:     shared-libs
claim:     A subroutine module exposes functions via an index table: an array of integers holding offsets from the module header to each callable routine.
context:   -
source:    The OS-9 Primer, Chapter 13: "Index table"
--- END ---
--- CARD ---
id:        trap-module-static-storage
type:      FACT
target:    68k
topic:     shared-libs
claim:     A trap module is given its own static storage area by the system (kept separate for each use); unlike subroutine modules, trap modules may have private static data.
context:   -
source:    The OS-9 Primer, Chapter 13: "Trap Modules"
--- END ---
--- CARD ---
id:        trap-module-limit
type:      FACT
target:    68k
topic:     shared-libs
claim:     A process may link to any number of subroutine modules (limited only by system memory) but may link to a maximum of 15 trap modules at a time.
context:   -
source:    The OS-9 Primer, Chapter 13: "Trap Modules"
--- END ---
--- CARD ---
id:        trap-module-entry-points
type:      FACT
target:    68k
topic:     shared-libs
claim:     Trap modules have exactly three entry points: TrapInit (called on tlink), TrapEnt (called on tcall), and TrapTerm (called on tunlink); only TrapEnt is directly called by user code.
context:   -
source:    The OS-9 Primer, Chapter 13: "Trap Modules"
--- END ---
--- CARD ---
id:        trap-execution-state
type:      FACT
target:    68k
topic:     shared-libs
claim:     Trap module code executes in the CPU state (user vs system) specified by the module attributes, regardless of the calling process's state.
context:   Unlike subroutine code which runs in the caller's context.
source:    The OS-9 Primer, Chapter 13: "Trap Modules"
--- END ---
--- CARD ---
id:        trap-vs-subroutine
type:      ROSETTA
target:    68k
topic:     shared-libs
claim:     OS-9000 subroutine modules are different from OS-9/68k: OS-9000 allows subroutine modules to have their own static storage areas (like trap modules in 68k).
context:   -
source:    The OS-9 Primer, Chapter 13: "Subroutine modules"
--- END ---
--- CARD ---
id:        exception-handler-install
type:      BEHAVIOR
target:    68k
topic:     exceptions
claim:     Exception handlers for hardware exceptions (bus error, divide by zero, etc.) can be installed per-process via _os_strap(); installing a handler allows a process to continue after an exception rather than terminating.
context:   -
source:    The OS-9 Primer, Chapter 14: "Catching hardware exceptions"
--- END ---
--- CARD ---
id:        bus-error
type:      FACT
target:    68k
topic:     exceptions
claim:     A bus error (T_BUSERR) is a hardware exception that occurs when attempting to access non-existent or privileged memory.
context:   -
source:    The OS-9 Primer, Chapter 14: "Hardware Exception Types"
--- END ---
--- CARD ---
id:        address-error
type:      FACT
target:    68k
topic:     exceptions
claim:     An address error (T_ADDERR) is a hardware exception that occurs when attempting to access memory on an improper boundary (e.g., accessing a 16-bit value at an odd address on 68k).
context:   -
source:    The OS-9 Primer, Chapter 14: "Hardware Exception Types"
--- END ---
--- CARD ---
id:        illegal-instruction
type:      FACT
target:    68k
topic:     exceptions
claim:     An illegal instruction exception (T_ILLINS) occurs when the CPU attempts to execute an invalid machine code instruction.
context:   -
source:    The OS-9 Primer, Chapter 14: "Hardware Exception Types"
--- END ---
--- CARD ---
id:        division-by-zero
type:      FACT
target:    68k
topic:     exceptions
claim:     A division by zero exception (T_ZERDIV) is raised when a div or divs instruction attempts to divide by zero.
context:   -
source:    The OS-9 Primer, Chapter 14: "Hardware Exception Types"
--- END ---
--- CARD ---
id:        privilege-violation
type:      FACT
target:    68k
topic:     exceptions
claim:     A privilege violation exception (T_PRIV) occurs when a privileged instruction is executed in user state.
context:   -
source:    The OS-9 Primer, Chapter 14: "Hardware Exception Types"
--- END ---
--- CARD ---
id:        isr-register-convention
type:      GOTCHA
target:    68k
topic:     interrupts
claim:     Register conventions between interrupt service routines (ISRs) and normal C code differ; special care is needed when an ISR calls C routines or vice versa.
context:   -
source:    The OS-9 Primer, Chapter 15: "Register conventions between ISRs and C code"
--- END ---
--- CARD ---
id:        system-globals
type:      CONCEPT
target:    68k
topic:     system
claim:     OS-9 maintains system global variables that provide read-only information about system state and modifiable globals for privileged processes to control system behavior.
context:   -
source:    The OS-9 Primer, Chapter 16: "System global variables"
--- END ---
--- CARD ---
id:        alloc-strategies
type:      FACT
target:    68k
topic:     memory
claim:     OS-9 offers multiple memory allocation schemes: first-fit (allocates from first available block) and buddy allocation (splits/merges power-of-2 blocks).
context:   -
source:    The OS-9 Primer, Chapter 2: "Memory model"
--- END ---
--- CARD ---
id:        kernel-variants
type:      FACT
target:    68k
topic:     kernel
claim:     OS-9 provides two kernel variants for each processor: a standard kernel (with full development facilities) and an atomic kernel (smaller, faster, for embedded systems).
context:   -
source:    The OS-9 Primer, Chapter 2: "Which kernel to use"
--- END ---
--- CARD ---
id:        atomic-kernel-minimal
type:      FACT
target:    68k
topic:     kernel
claim:     The atomic kernel is smaller and faster than the standard kernel but lacks some development facilities; it is designed for embedded systems with emphasis on execution time.
context:   -
source:    The OS-9 Primer, Chapter 2: "Atomic Kernel"
--- END ---
--- CARD ---
id:        colored-memory
type:      CONCEPT
target:    68k
topic:     memory
claim:     Some OS-9 systems support colored memory: distinct memory regions used for different purposes such as non-volatile RAM, fast cache memory, or DMA-accessible regions.
context:   -
source:    The OS-9 Primer, Chapter 8: "Colored memory in data modules"
--- END ---
--- CARD ---
id:        fastrak-cross-dev
type:      FACT
target:    68k
topic:     dev-tools
claim:     FasTrak is Microware's integrated cross-development environment; it runs on UNIX X-Windows or Windows and communicates with OS-9 targets via TCP/IP (Ethernet or SLIP).
context:   -
source:    The OS-9 Primer, Chapter 2: "FasTrak"
--- END ---
--- CARD ---
id:        resident-development
type:      CONCEPT
target:    all
topic:     dev-tools
claim:     Resident development is developing code directly on the OS-9 system itself; it has the advantage of needing no additional hardware but loses the editing tools available on host systems.
context:   -
source:    The OS-9 Primer, Chapter 2: "Development Environments"
--- END ---
--- CARD ---
id:        pcbridge-cross-compiler
type:      FACT
target:    68k
topic:     dev-tools
claim:     PCBridge is Microware's cross-development tool for DOS/Windows PCs; it includes a cross-compiler, libraries, and source-level debugger, communicating with OS-9 targets via serial interface.
context:   -
source:    The OS-9 Primer, Chapter 2: "Personal Computer Hosted Development"
--- END ---
--- CARD ---
id:        shell-background-execution
type:      BEHAVIOR
target:    all
topic:     shell
claim:     The OS-9 shell allows background process execution via the & operator; a background process runs while the shell waits for new commands.
context:   -
source:    The OS-9 Primer, Chapter 4: "Background tasks"
--- END ---
--- CARD ---
id:        procs-command
type:      FACT
target:    all
topic:     shell
claim:     The procs command displays all processes owned by the current user, showing process ID, parent ID, group, priority, memory size, signals, state, CPU time, age, and module name.
context:   -
source:    The OS-9 Primer, Chapter 4: "The procs command"
--- END ---
--- CARD ---
id:        error-redirection
type:      BEHAVIOR
target:    all
topic:     shell
claim:     The OS-9 shell allows error output redirection with » operator to redirect stderr independent of stdout; combining both uses »> syntax.
context:   -
source:    The OS-9 Primer, Chapter 4: "Modified Error Output"
--- END ---
--- CARD ---
id:        combined-io-redirection
type:      BEHAVIOR
target:    all
topic:     shell
claim:     To redirect both stdout and stderr to the same file, use $:command »>file syntax; the simpler command >file »file syntax fails because it attempts to create the file twice.
context:   -
source:    The OS-9 Primer, Chapter 4: "Modified Standard Output and Error Output"
--- END ---
--- CARD ---
id:        stdin-stdin-redirection
type:      BEHAVIOR
target:    all
topic:     shell
claim:     The shell supports stdin redirection with < operator; redirecting stdin from a file is useful for repeatedly testing programs with fixed test input.
context:   -
source:    The OS-9 Primer, Chapter 4: "Modified Standard Input"
--- END ---
--- CARD ---
id:        kill-signal-command
type:      FACT
target:    all
topic:     shell
claim:     The kill command sends the S_KILL signal (signal 0) to a process by PID; kill 0 sends the signal to all processes owned by the current user.
context:   -
source:    The OS-9 Primer, Chapter 4: "Built-in Commands"
--- END ---
--- CARD ---
id:        setpr-priority-command
type:      FACT
target:    all
topic:     shell
claim:     The setpr command changes the priority of a running process; requires owning the process or being superuser.
context:   -
source:    The OS-9 Primer, Chapter 4: "Built-in Commands"
--- END ---
--- CARD ---
id:        profile-command
type:      FACT
target:    all
topic:     shell
claim:     The profile command reads a file of shell commands and executes them sequentially; environment changes made within the profile file persist in the shell.
context:   -
source:    The OS-9 Primer, Chapter 4: "Built-in Commands"
--- END ---
--- CARD ---
id:        wait-command
type:      FACT
target:    all
topic:     shell
claim:     The wait command forces the shell to block until all background processes terminate before returning the prompt.
context:   -
source:    The OS-9 Primer, Chapter 4: "Built-in Commands"
--- END ---
--- CARD ---
id:        ex-command
type:      FACT
target:    all
topic:     shell
claim:     The ex command terminates the current shell and starts a replacement command (e.g., ex MShell replaces shell with MShell).
context:   -
source:    The OS-9 Primer, Chapter 4: "Built-in Commands"
--- END ---
--- CARD ---
id:        logout-command
type:      FACT
target:    all
topic:     shell
claim:     The logout command terminates the shell and ends the user session.
context:   -
source:    The OS-9 Primer, Chapter 4: "Built-in Commands"
--- END ---
--- CARD ---
id:        directory-vs-execution
type:      CONCEPT
target:    all
topic:     filesystem
claim:     OS-9 distinguishes between a data directory (where relative pathnames are resolved) and an execution directory (where shell commands are searched); they can be different.
context:   -
source:    The OS-9 Primer, Chapter 3: "Data directories versus execution directories"
--- END ---
--- CARD ---
id:        command-search-path
type:      BEHAVIOR
target:    all
topic:     shell
claim:     The OS-9 shell searches for executable commands in the execution directory or in a configurable search path; external commands are not built into the shell.
context:   -
source:    The OS-9 Primer, Chapter 4: "Command execution"
--- END ---
--- CARD ---
id:        environment-variables
type:      CONCEPT
target:    all
topic:     shell
claim:     OS-9 supports environment variables: text strings stored in the shell that can be referenced by programs and are inherited by child processes.
context:   -
source:    The OS-9 Primer, Chapter 3: "Environment variables"
--- END ---
--- CARD ---
id:        setenv-unsetenv
type:      FACT
target:    all
topic:     shell
claim:     Environment variables are set via setenv and unset via unsetenv; variable values with spaces must be quoted.
context:   -
source:    The OS-9 Primer, Chapter 3: "Environment variables"
--- END ---
--- CARD ---
id:        login-process
type:      BEHAVIOR
target:    all
topic:     system
claim:     OS-9 login authenticates users and sets up the initial shell; upon login, a shell is created with standard I/O connected to the user's terminal.
context:   -
source:    The OS-9 Primer, Chapter 3: "The login process"
--- END ---
