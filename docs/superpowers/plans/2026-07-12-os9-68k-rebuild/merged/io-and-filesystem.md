# I/O, devices, and the filesystem

Merged from 197 raw bucketed cards (plus one pulled in from misc-unclustered) down to
117 surviving cards. Organized by sub-topic: I/O system architecture; device/path
descriptors and kernel I/O tables; file managers and I$ service requests; directories
and path resolution; shell I/O redirection; RBF disk structure (identification sector,
allocation map, file descriptor/segments, directory format); raw physical I/O; record
locking; file security; C-language I/O bindings; device naming; disk/directory
utilities. True duplicates across source documents were merged with combined citations;
generic filesystem-101 facts, mis-bucketed BASIC09/compiler-toolchain trivia, and
thin/redundant restatements were pruned.

---

## I/O system architecture

--- CARD ---
id:        os9-io-four-level-architecture
type:      CONCEPT
target:    all
verify:    from-manual
topic:     io-architecture
claim:     OS-9's I/O system is organized into four modular levels: the kernel (dispatches I/O requests), file managers (logical operations for a device class, e.g. RBF for disk, SCF for character devices), device drivers (physical hardware I/O), and device descriptors (per-device configuration data). A new physical device only needs a new driver + descriptor if an existing file manager class already fits its behavior.
context:   Non-executable modules (device descriptors) are referenced directly by the kernel/file managers; executable modules (drivers, file managers) are called through jump tables. This four-level layering is a version-independent OS-9 design principle, not a 68k-only detail.
source:    OS-9 v2.4 Technical Reference Manual, "System Modularity", p. 1-1 + Technical I/O Manual v2.4, "The OS-9 Unified Input/Output System", p. 1-1 + The OS-9 Guru, §1.8.4-8.6, pp. 12-13
--- END ---

--- CARD ---
id:        file-manager-vs-driver-separation
type:      CONCEPT
target:    all
verify:    from-manual
topic:     io-architecture
claim:     A file manager performs logical operations for its device class (directory structure, line editing, record semantics), working at the generic class level rather than any individual device. A device driver performs raw physical I/O and has no knowledge of why the I/O is happening or how to interpret the data; it translates the file manager's logical request into the specific hardware operations (register layout, transfer mode) a given controller needs.
context:   This separation is what lets a device driver be written with no understanding of the filing system, and lets one file manager (e.g. RBF) work across many different physical disk controllers.
source:    The OS-9 Guru, §1.8.5, p. 13 + Technical I/O Manual v2.4, "The Kernel and I/O", pp. 1-4 to 1-5
--- END ---

--- CARD ---
id:        dynamic-io-system-loading
type:      FACT
target:    all
verify:    from-manual
topic:     io-architecture
claim:     File managers, device drivers, and device descriptors can all be loaded or unloaded while OS-9 is running -- no reboot is required to add a new I/O interface or repurpose an existing physical device with a different driver.
context:   Simplifies debugging of new I/O components and lets a manufacturer supply a driver that loads on demand when its hardware is present.
source:    The OS-9 Guru, §1.9, p. 15
--- END ---

--- CARD ---
id:        device-independent-io-and-unified-namespace
type:      CONCEPT
target:    all
verify:    from-manual
topic:     io-architecture
claim:     OS-9 exposes one unified namespace in which devices and disk files are both addressed as paths, and the same open/read/write/close operations work on either -- so a program writing to a terminal can have its output redirected to a disk file transparently, with no code change.
context:   -
source:    The OS-9 Guru, §1.7.5, p. 9 + OS-9 Insights, §15.1 "The Unified File System"
--- END ---

--- CARD ---
id:        kernel-device-and-path-tables
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     io-architecture
claim:     The kernel maintains two internal I/O tables: the device table (one entry per attached device, created on first I$Attach, holding file manager/driver names, the driver's static storage pointer, and a use count) and the path table (one entry per open I/O path, created on I$Open and destroyed on I$Close). When a path is opened, the kernel links to the device descriptor, records the device in the device table, and creates the path descriptor entry in the path table.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, "The Kernel and I/O", p. 3-3 + Technical I/O Manual v2.4, "The Kernel and I/O", p. 1-4
--- END ---

--- CARD ---
id:        system-call-two-types
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     io-architecture
claim:     OS-9 system calls fall into two categories: I/O calls (I$ prefix, directed to file managers/drivers -- opens, reads, writes) and system function calls (F$ prefix, executed directly by the kernel -- memory management, multitasking). System function calls further split into user-state and system-state, with system-state calls restricted to privileged code.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, "System Call Overview", p. 2-2
--- END ---

---

## Device descriptors, path descriptors, and the kernel I/O tables

--- CARD ---
id:        device-descriptor-mode-bits
type:      FACT
target:    68k
verify:    from-manual
topic:     device-descriptors
claim:     M$Mode (device descriptor offset $37) bits: bit 0 = read access, bit 1 = write access, bit 2 = executable access, bit 6 = single-user (non-sharable), bit 7 = directory file access. The kernel validates a caller's requested access mode against this field.
context:   Used to enforce access control on device open/create.
source:    Technical I/O Manual v2.4, "Device Descriptor Modules", p. 1-11
--- END ---

--- CARD ---
id:        device-descriptor-m-devcon
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     device-descriptors
claim:     M$DevCon (device descriptor offset $3C) points to an optional device configuration table for driver-specific parameters or OEM constants. Unlike the standard initialization table, M$DevCon values are NOT copied into path descriptors -- the kernel and file manager never reference them.
context:   Available to the driver only during INIT and TERM; other driver routines must search the device table to find the descriptor.
source:    Technical I/O Manual v2.4, "Device Descriptor Modules", p. 1-12
--- END ---

--- CARD ---
id:        device-descriptor-m-opt-and-path-option-area
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     device-descriptors
claim:     A device descriptor's M$Opt field (offset $46) gives the size of its standard initialization table (M$DTyp through M$DTyp+M$Opt, theoretical max 128 bytes though individual file managers may enforce a smaller ceiling). That table is copied into the path descriptor's 128-byte option area (PD_OPT, offset $80) every time a path is opened. Path options control per-path I/O behavior (buffering, echo, parity, flow control, backspace/delete characters, etc.); a user can inspect the whole area via I$GetStt(SS_Opt) and modify some values via I$SetStt -- the file manager protects certain values from inappropriate changes.
context:   Contrast with M$DevCon (see [[device-descriptor-m-devcon]]), whose contents are never copied into the path descriptor.
source:    Technical I/O Manual v2.4, "Device Descriptor Modules", pp. 1-12 to 1-13 + OS-9 Insights, §15.3 "Path Options"
--- END ---

--- CARD ---
id:        device-descriptor-multiple-synonyms
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     device-descriptors
claim:     A single I/O device may have multiple device descriptors with different names and initialization parameters. For example, a serial/parallel driver can have one descriptor for terminal mode (/T1) and another for printer mode (/P1) on the same physical port.
context:   The kernel's I$Attach matching logic distinguishes these as "synonymous devices" -- see [[i-attach-device-table-matching]].
source:    Technical I/O Manual v2.4, "Device Descriptor Modules", p. 1-8
--- END ---

--- CARD ---
id:        i-attach-device-table-matching
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     kernel-io
claim:     I$Attach compares a device specification against existing device table entries: if port, file manager, driver, and descriptor all match, it increments the use count; if only port/manager/driver match (descriptor differs), it creates a new entry for a synonymous device; if nothing matches, it allocates driver storage, sets V_PORT, and calls the driver's INIT routine.
context:   This logic is what enables multiple device names on the same port with different parameters (see [[device-descriptor-multiple-synonyms]]).
source:    Technical I/O Manual v2.4, "The Kernel and I/O", pp. 1-5 to 1-7
--- END ---

--- CARD ---
id:        i-attach-init-error-handling
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     kernel-io
claim:     If a driver's INIT routine returns an error during I$Attach, the kernel calls the driver's TERM routine, deallocates any allocated resources, and returns the error to the caller without creating a device table entry.
context:   Ensures failed-INIT cleanup is symmetric.
source:    Technical I/O Manual v2.4, "The Kernel and I/O", p. 1-6
--- END ---

--- CARD ---
id:        i-detach-use-count-cleanup
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     kernel-io
claim:     I$Detach decrements a device's use count. When it reaches zero, the kernel checks whether another device shares the same static storage; if not, it calls the driver's TERM routine, deallocates storage, and removes the device table entry.
context:   Ensures drivers clean up resources only when the last reference closes.
source:    Technical I/O Manual v2.4, "The Kernel and I/O", p. 1-6
--- END ---

--- CARD ---
id:        path-descriptor-three-sections
type:      FACT
target:    68k
verify:    from-manual
topic:     path-descriptors
claim:     A path descriptor has three sections: (1) a universal 30-byte section (PD_PD to PD_LProc) common to all path descriptors, (2) a file-manager-specific section (PD_FST) for file pointers/state, and (3) a 128-byte option area (PD_OPT), initialized from the device descriptor and alterable via GetStat/SetStat.
context:   Path descriptors are dynamically allocated/deallocated as paths open and close; their layouts are linked via sys.l/usr.l.
source:    OS-9 v2.4 Technical Reference Manual, "Path Descriptors", p. 3-7
--- END ---

--- CARD ---
id:        path-descriptor-count-and-lifecycle
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     path-descriptors
claim:     Each I$Open or I$Create allocates a new path descriptor and a path-table entry, setting its share counter PD_COUNT (offset $1A) to 1. I$Dup does not call the file manager or driver at all -- it just increments PD_COUNT on the existing path descriptor, letting multiple processes share one open-file context. I$Close decrements PD_COUNT; only when it reaches 0 is the path descriptor actually deallocated and removed from the path table.
context:   -
source:    Technical I/O Manual v2.4, "Path Descriptors", pp. 1-14 to 1-15 + "Kernel I/O Service Requests", p. 1-7 + "The Kernel and I/O", p. 1-6
--- END ---

--- CARD ---
id:        driver-multi-port-static-storage
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     device-drivers
claim:     Multi-port drivers share a single static storage area across all ports on the same device, distinguishing ports via the device table entry or the V_PORT field. A single driver instance manages multiple independent device paths.
context:   Reduces memory overhead; driver code is shared, with per-port data kept in a static extension.
source:    Technical I/O Manual v2.4, "Device Drivers that Control Multiple Devices", pp. 1-27 to 1-28
--- END ---

--- CARD ---
id:        driver-multi-class-different-managers
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     device-drivers
claim:     Multi-class drivers handle different device classes (e.g., block and character modes) by supporting different file managers: a single driver associates with multiple device descriptors, each pointing to a different file manager class.
context:   Allows hardware to be used flexibly; common with intelligent controllers.
source:    Technical I/O Manual v2.4, "Device Drivers that Control Multiple Devices", p. 1-31
--- END ---

---

## File managers (RBF, SCF) and I$ service requests

--- CARD ---
id:        rbf-random-block-file-manager
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     file-managers
claim:     The Random Block File Manager (RBF) is the file manager for random-access, block-structured devices (floppy/hard disk): it maintains directory structures, handles block-oriented I/O, and manages sector/segment allocation. It is one of OS-9's standard file managers alongside SCF (character devices) and SBF (sequential block storage).
context:   -
source:    OS-9 v2.4 Technical Reference Manual, "I/O Overview", p. 1-4 + Technical I/O Manual v2.4, "File Managers", p. 1-17
--- END ---

--- CARD ---
id:        scf-sequential-character-file-manager
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     file-managers
claim:     The Sequential Character File Manager (SCF) handles all non-mass-storage devices that transfer a character at a time (terminals, character printers), providing a standardized interface. It performs line editing, handles special characters/NULLs, strips parity, and implements data flow control.
context:   SCF is one of OS-9's standard file managers.
source:    the independent 1985-era OS-9/68000 technical manual, Introduction "Level 2 - File Managers" + Technical I/O Manual v2.4, "File Managers", p. 1-17
--- END ---

--- CARD ---
id:        rbf-disk-caching
type:      FACT
target:    all
verify:    from-manual
topic:     file-managers
claim:     RBF (the disk file manager) supports disk caching to improve performance.
context:   -
source:    The OS-9 Guru, §7.9.1, p. 134
--- END ---

--- CARD ---
id:        scf-line-editing-buffer-size
type:      FACT
target:    all
verify:    from-manual
topic:     file-managers
claim:     SCF's line buffer is 512 bytes maximum line length including the carriage return; a separate buffer is allocated per open path.
context:   Line editing is an SCF function, not a shell function (though mshell does its own).
source:    The OS-9 Guru, §2.7, p. 28
--- END ---

--- CARD ---
id:        scf-editing-keys
type:      FACT
target:    all
verify:    from-manual
topic:     file-managers
claim:     SCF line-editing keys, customizable via tmode/xmode: [BS]/[BkSp] deletes left, [^X] deletes the line, [^D] reprints the line, [^A] redisplays from cursor, [ESC] as the first character signals EOF.
context:   Setting a key code to zero disables that feature; default assignments are customizable.
source:    The OS-9 Guru, §2.7, p. 28
--- END ---

--- CARD ---
id:        i-open-allocate-initialize
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$Open allocates required buffers, initializes path descriptor variables, and parses the pathname. On multi-file devices, it performs directory searching to locate the specified file.
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-19
--- END ---

--- CARD ---
id:        i-create-synonymous-with-open
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$Create performs the same function as I$Open. On multi-file devices it creates a new file; file managers without multi-file support treat I$Create as synonymous with I$Open.
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-19
--- END ---

--- CARD ---
id:        i-delete-allocation-cleanup
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$Delete searches for a file (similar to I$Open), removes the filename from the directory, and returns the media space previously in use to the free pool.
context:   Only for multi-file device managers.
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-19
--- END ---

--- CARD ---
id:        i-getstat-device-status-wildcard
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$GetStt is a wildcard call to determine the status of various device/file-manager features. The file manager handles known status codes; unknown codes are passed to the driver for device-specific status reporting.
context:   Example: file size, device capacity, device-specific configuration.
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-19
--- END ---

--- CARD ---
id:        i-makdir-directory-creation
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$MakDir creates a directory file on multi-file devices. File managers without directory support return carry set and an unknown-service error.
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-19
--- END ---

--- CARD ---
id:        i-read-returns-requested-bytes
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$Read returns the number of bytes requested into the user's data buffer, returning an EOF error if no further data is available. It generally performs no editing on the data; the file manager may use an internal or direct buffer, copying to the user area if needed.
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", pp. 1-19 to 1-20
--- END ---

--- CARD ---
id:        i-readln-terminates-on-carriage-return
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$ReadLn differs from I$Read in two ways: it terminates on the first carriage return (end-of-record), and it performs input editing appropriate for the device. It typically uses an internal buffer and copies to the user area.
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-20
--- END ---

--- CARD ---
id:        i-seek-logical-positioning
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$Seek (random-access devices only) repositions the file pointer to a specified byte. It's a logical movement that does not affect the physical device, and there is no error if the position is beyond EOF. Devices without random-access support do nothing and return no error.
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-20
--- END ---

--- CARD ---
id:        i-setstat-set-device-status
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$SetStt sets the status of various device/file-manager features. The file manager handles known codes; unknown codes are passed to the driver. Example: formatting a disk track, whose behavior differs by controller type.
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-20
--- END ---

--- CARD ---
id:        i-write-expand-file
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$Write writes data (generally unedited) via the driver's output routine. Writing past EOF expands the file with new data. On fixed-length-record devices (e.g. RBF), the file manager often must pre-read a sector before writing to it unless the entire sector is being written.
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-20
--- END ---

--- CARD ---
id:        i-writeln-output-editing
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$WriteLn transfers data up to and including the first carriage return (end-of-record), performing output editing (e.g. SCF appends a line-feed after the CR, and nulls, if appropriate).
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-20
--- END ---

---

## Directories and path resolution

--- CARD ---
id:        two-current-directories-not-one
type:      CONCEPT
target:    all
verify:    authored
topic:     directories
claim:     Unix has one current working directory. OS-9 has two, independently settable: the data directory (`chd`), which governs where relative *data* file paths resolve, and the execution directory (`chx`), which governs where relative *command names* resolve when something is run. They are genuinely separate state, not two names for the same thing.
context:   A program visible via `dir` (relative to the data directory) can still be "not found" when you try to run it by relative name -- running is a command lookup, which resolves via the execution directory/PATH, not the data directory.
source:    authored + The OS-9 Primer, Chapter 3, "Data directories versus execution directories"
--- END ---

--- CARD ---
id:        chx-chd-resolution-is-open-mode-not-command
type:      CONCEPT
target:    all
verify:    authored
topic:     directories
claim:     The underlying rule unifying `chx` vs `chd` resolution is not "which command did you type" but "what mode was this specific file open performed in": a relative pathname resolves against the execution directory if the open is in execute mode, and against the data directory otherwise. A PATH lookup for a command to run is an execute-mode open (resolves via chx); typing a relative argument to the `chx` or `chd` command itself is a non-execute open (resolves via chd).
context:   This one rule explains two facts that look unrelated at first (PATH lookup uses chx; `chx somepath` itself resolves relative to chd) -- it is a single, coherent, documented OS-9 principle (confirmed against the OS-9 v2.4 Technical Reference Manual's "Current Directories" material), not two separate quirks or an os9exec-specific invention.
source:    authored
--- END ---

--- CARD ---
id:        chd-chx-shell-commands
type:      FACT
target:    68k
verify:    from-manual
topic:     directories
claim:     `chd <path>` and `chx <path>` are built-in shell commands that change the current data directory and current execution directory respectively, each accepting a full or relative path (relative paths resolve against that same directory kind). With no argument, `chd` changes to the directory named by the HOME environment variable, if set.
context:   -
source:    Using Professional OS-9 v2.4, "The Shell", Built-in Shell Commands section, p. 5-6
--- END ---

--- CARD ---
id:        home-directory-user-base
type:      FACT
target:    68k
verify:    from-manual
topic:     directories
claim:     The home directory specified in a user's password-file entry is that user's initial current data directory at login, and is where a no-argument `chd` changes to if the HOME environment variable is set.
context:   Personal workspace for each user on a timesharing system.
source:    Using Professional OS-9 v2.4, "The Shell", The Shell Environment section, p. 5-3
--- END ---

--- CARD ---
id:        command-search-path
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     directories
claim:     The OS-9 shell searches for executable commands in the execution directory or in a configurable search path; external commands are not built into the shell.
context:   -
source:    The OS-9 Primer, Chapter 4, "Command execution"
--- END ---

--- CARD ---
id:        more-dots-not-more-dotdotslash
type:      GOTCHA
target:    all
verify:    authored
topic:     directories
claim:     Where Unix stacks `../../..` for each additional directory level up, OS-9 uses one additional dot per level instead: `...` means `../..` (two levels up) and `....` means `../../..` (three levels up). A single `..` (one level up) is unchanged from Unix. Specifying more dot-levels than actually exist is not an error -- it simply clamps at the root directory (e.g. `dir .........` safely shows the root when the caller is unsure of nesting depth).
context:   This has been verified for both the `dir` command and `chd`. On an RBF disk image, going "up" past the device root does not escape the filesystem -- the root's parent is itself, a clamping behavior confirmed directly against the OS-9 v2.4 Technical Reference Manual's own directory-resolution text, not just inferred from testing.
source:    authored + Using Professional OS-9 v2.4, "Accessing Files" p. 4-9 and "Climbing Directory Trees" p. 4-14 + The OS-9 Guru, §2.5, p. 26
--- END ---

--- CARD ---
id:        pathlist-syntax-absolute-vs-relative
type:      CONCEPT
target:    all
verify:    from-manual
topic:     directories
claim:     A pathlist is the text string identifying a device or file. An absolute pathlist begins with `/`; its first element is a device name (e.g. `/h0`), and it does not depend on the current directory. A relative pathlist does not begin with `/` and resolves starting from the current directory (data or execution directory -- see [[chx-chd-resolution-is-open-mode-not-command]]).
context:   Example: `/h0/usr/nicholle` is absolute; `doc/letters/jim` is relative.
source:    The OS-9 Guru, §2.5, p. 26 + Using Professional OS-9 v2.4, "The OS-9 File System", Accessing Files section, p. 4-9
--- END ---

--- CARD ---
id:        shell-wild-card-expansion
type:      FACT
target:    all
verify:    from-manual
topic:     directories
claim:     The OS-9 shell itself expands wildcard filenames before running a program -- not the forked program. `*` matches any number of characters (or none); `?` matches a single character. The shell reads the directory and expands names using the F$CmpNam system call, so the program only ever receives already-expanded names as parameters.
context:   -
source:    The OS-9 Guru, §2.2, p. 22 + Using Professional OS-9 v2.4, "Dir: Displaying Contents" section, p. 4-11
--- END ---

---

## Shell I/O redirection

--- CARD ---
id:        shell-stdio-redirection-basics
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     io-redirection
claim:     The OS-9 shell redirects standard I/O with three modifiers, each immediately followed by a path: `<` (redirect stdin), `>` (redirect stdout), `>>` (redirect stderr). By default a child process inherits its parent's standard I/O paths.
context:   Redirection modifiers are stripped from the command line and are not passed to the program as parameters unless quoted.
source:    The OS-9 Primer, Chapter 4 + Using Professional OS-9 v2.4, "The Shell", pp. 5-8 to 5-10
--- END ---

--- CARD ---
id:        io-append-overwrite
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     io-redirection
claim:     Shell output redirection with `>-` overwrites files, `>+` appends to files; this prevents accidental data loss by refusing to overwrite an existing file without explicit intent.
context:   -
source:    The OS-9 Primer, Chapter 4, "Redirection Appending and Overwriting"
--- END ---

--- CARD ---
id:        combined-io-redirection
type:      GOTCHA
target:    all
verify:    from-manual
topic:     io-redirection
claim:     To redirect both stdout and stderr to the same file, the syntax must be `$:command >>file`; the naively simpler `command >file >>file` fails because it attempts to create the destination file twice.
context:   -
source:    The OS-9 Primer, Chapter 4, "Modified Standard Output and Error Output"
--- END ---

--- CARD ---
id:        command-line-modifier-characters-stripped
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     io-redirection
claim:     Characters comprising execution modifiers (`#`, `^`, `>`, `<`, `>>`), separators (`;`, `&`, `!`), and wildcards (`*`, `?`) are stripped from command-line parts before being passed to the program as parameters -- they cannot be delivered to a program literally unless quoted.
context:   Quoting allows literal passing of these special characters.
source:    Using Professional OS-9 v2.4, "The Shell", Shell Command Line Processing section, p. 5-8
--- END ---

---

## RBF disk structure: identification sector (LSN 0)

--- CARD ---
id:        rbf-tree-structured-filesystem
type:      CONCEPT
target:    all
verify:    from-manual
topic:     rbf-storage
claim:     RBF implements a tree-structured file system designed for efficient disk space usage, resistance to accidental damage, and fast file access with relative simplicity.
context:   -
source:    Disk File Organization manual, "Basic Disk Organization", p. 7-1 + The OS-9 Guru, §1.7.9, p. 10 + Using Professional OS-9 v2.4, "An Overview of OS-9", Storing Information section, p. 1-3
--- END ---

--- CARD ---
id:        lsn-logical-sector-numbering
type:      CONCEPT
target:    all
verify:    from-manual
topic:     rbf-storage
claim:     OS-9 abstracts disk addresses as Logical Sector Numbers (LSNs) numbered 0 to (n-1), eliminating hardware dependencies on track, surface, and sector numbering. The disk driver or controller is responsible for mapping LSNs to physical addresses.
context:   -
source:    Disk File Organization manual, "Basic Disk Organization", p. 7-1
--- END ---

--- CARD ---
id:        identification-sector-lsn-zero
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-storage
claim:     LSN 0 always contains the identification sector, which describes the disk's physical and logical format, allocation map size, root directory location, volume name, creation date/time, and (for bootable disks) the starting LSN and size of the OS9Boot file.
context:   -
source:    Disk File Organization manual, "Identification Sector", p. 7-3
--- END ---

--- CARD ---
id:        standard-disk-structure
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-storage
claim:     Every OS-9 disk has the same basic structure: identification sector at LSN 0, disk allocation map usually at LSN 1, and root directory immediately following the allocation map, with file data organized into segments and allocation tracked via the bitmap.
context:   -
source:    Disk File Organization manual, "Basic Disk Organization", p. 7-1 + OS-9 v2.4 Technical Reference Manual, "Disk File Organization", p. 7-1 + OS-9 Insights, §19 "The RBF Disk Format"
--- END ---

--- CARD ---
id:        dd-tot-total-sectors
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_TOT field (offset $00, 3 bytes) in the identification sector contains the total number of sectors on the media.
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-tks-track-size
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_TKS field (offset $03, 1 byte) in the identification sector specifies track size in sectors.
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-map-allocation-map-bytes
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_MAP field (offset $04, 2 bytes) in the identification sector specifies the actual number of bytes used in the allocation map.
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-bit-cluster-size
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_BIT field (offset $06, 2 bytes) in the identification sector specifies the number of sectors per cluster. Cluster size (sectors per cluster) is always an integral power of two.
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-dir-root-directory-lsn
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_DIR field (offset $08, 3 bytes) in the identification sector contains the LSN of the root directory's file descriptor.
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-own-owner-id
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_OWN field (offset $0B, 2 bytes) in the identification sector contains the owner ID of the disk itself (distinct from a file's own FD_OWN owner field).
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-att-attributes
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_ATT field (offset $0D, 1 byte) in the identification sector contains disk-level file attributes.
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-dsk-disk-id
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_DSK field (offset $0E, 2 bytes) in the identification sector contains the disk ID.
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-fmt-disk-format
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_FMT field (offset $10, 1 byte) describes disk format with bits indicating: bit 0 = single/double side, bit 1 = FM/MFM density, bit 2 = double track (96/135 TPI), bit 3 = quad track density (192 TPI), bit 4 = octal track density (384 TPI).
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-spt-sectors-per-track
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_SPT field (offset $11, 2 bytes) in the identification sector contains sectors per track as a two-byte value.
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-bt-bootstrap-lsn
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_BT field (offset $15, 3 bytes) in the identification sector contains the system bootstrap's starting LSN.
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-bsz-bootstrap-size
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_BSZ field (offset $18, 2 bytes) in the identification sector specifies the size of the system bootstrap.
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-dat-creation-date
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_DAT field (offset $1A, 5 bytes) in the identification sector contains the disk's creation date in Year/Month/Day/Hour/Minute format.
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-nam-volume-name
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_NAM field (offset $1F, 32 bytes) in the identification sector contains the volume name.
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-opt-path-descriptor-options
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_OPT field (offset $3F, 32 bytes) in the identification sector contains default path descriptor options for the device.
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-sync-media-integrity-code
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_SYNC field (offset $60, 4 bytes) in the identification sector contains a media integrity code.
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-maplsn-bitmap-start
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_MapLSN field (offset $64, 4 bytes) specifies the allocation map's starting sector number; a value of 0 defaults to LSN 1.
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-lsnsize-logical-sector-size
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_LSNSize field (offset $68, 2 bytes) in the identification sector specifies the media's logical sector size; a value of 0 defaults to 256 bytes.
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-versid-sector-version
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_VersID field (offset $6A, 2 bytes) in the identification sector contains the Sector 0 Version ID.
context:   -
source:    Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

---

## RBF disk structure: allocation map and clusters

--- CARD ---
id:        disk-sector-cluster-allocation
type:      FACT
target:    68k
verify:    from-manual
topic:     rbf-allocation-map
claim:     Data is organized into sectors (pre-defined fixed-size byte groups, e.g. 256 bytes, fixed per device and set during formatting) and allocated in clusters (groupings of sectors whose size depends on device capacity and physical characteristics).
context:   Cluster size is device-dependent and shown by the `free` utility.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Free and Mfree section, p. 3-9 + "The OS-9 File System", OS-9 File Storage section, p. 4-1
--- END ---

--- CARD ---
id:        allocation-map-cluster-tracking
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-allocation-map
claim:     Each bit in the allocation map represents one cluster of the disk (the true allocation unit, which may span more than one sector -- see [[disk-sector-cluster-allocation]]). A set bit indicates the cluster is in use, defective, or non-existent; a cleared bit indicates it is free.
context:   -
source:    Disk File Organization manual, "Allocation Map" + Using Professional OS-9 v2.4, "The OS-9 File System", OS-9 File Storage section, p. 4-1
--- END ---

---

## RBF disk structure: file descriptor, segments, and file pointers

--- CARD ---
id:        os9-attribute-byte-bit-layout
type:      FACT
target:    all
verify:    VERIFIED
topic:     disk-io
claim:     The OS-9 file attribute byte (`fd_att`, the first byte of the file descriptor sector) breaks down bit-for-bit as: 0x01 owner read, 0x02 owner write, 0x04 owner execute, 0x08 public read, 0x10 public write, 0x20 public execute, 0x80 directory, and one more bit at 0x40 controlling shareability. It maps onto os9lib `<stat.h>`'s `S_IREAD`/`S_IWRITE`/`S_IEXEC`/`S_IOREAD`/`S_IOWRITE`/`S_IOEXEC`/`S_ISHARE`/`S_IFDIR`.
context:   UNRESOLVED CONFLICT flagged for a human before this ships: this card's live-tested os9lib mapping labels bit 0x40 "sharable" (set = sharable), while the Disk File Organization manual's own file-descriptor prose labels the same bit "non-sharable" (set = single-user, i.e. opposite polarity). The two empirical test bytes on record (`att=0x2f` for owner-rwx+public-rx, `att=0xbf` for a fully-permissioned directory) do not actually disambiguate polarity, since neither example isolates that bit meaningfully. The two access classes are *owner* and *public* -- there is no group class. Verified live against os9exec + os9lib headers for the bits that are unambiguous.
source:    authored + Disk File Organization manual, "File Descriptor Content Description", Figure 7-2
--- END ---

--- CARD ---
id:        fd-own-owner-id
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-file-descriptor
claim:     FD_OWN field (offset $01, 2 bytes) in the file descriptor contains the owner's User ID.
context:   -
source:    Disk File Organization manual, "File Descriptor Content Description", Figure 7-2
--- END ---

--- CARD ---
id:        fd-dat-modification-date
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-file-descriptor
claim:     FD_DAT field (offset $03, 5 bytes) in the file descriptor contains the date last modified, in Year/Month/Day/Hour/Minute format. This field updates when a file is opened in write or update mode, useful for date-dependent backups.
context:   -
source:    Disk File Organization manual, "File Descriptor Content Description", Figure 7-2
--- END ---

--- CARD ---
id:        fd-siz-file-size
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-file-descriptor
claim:     FD_SIZ field (offset $09, 4 bytes) in the file descriptor contains the file size in bytes.
context:   -
source:    Disk File Organization manual, "File Descriptor Content Description", Figure 7-2
--- END ---

--- CARD ---
id:        fd-creat-creation-date
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-file-descriptor
claim:     FD_CREAT field (offset $0D, 3 bytes) in the file descriptor contains the creation date in Year/Month/Day format.
context:   -
source:    Disk File Organization manual, "File Descriptor Content Description", Figure 7-2
--- END ---

--- CARD ---
id:        fd-seg-segment-list
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-file-descriptor
claim:     FD_SEG field (offset $10, 240 bytes) in the file descriptor contains the segment list: five-byte entries continuing until the end of the sector. For 256-byte sectors this yields 48 entries; each entry has a 3-byte LSN and a 2-byte sector count. Unused segment entries must be zero.
context:   -
source:    Disk File Organization manual, "File Descriptor Content Description", Figure 7-2
--- END ---

--- CARD ---
id:        multiple-contiguous-segment-structure
type:      CONCEPT
target:    all
verify:    from-manual
topic:     rbf-file-structure
claim:     OS-9 uses a multiple-contiguous-segment file structure: segments are physically contiguous sectors, and if a file cannot fit in one segment, additional segments are allocated (e.g. when expanded after creation, or when sufficient contiguous free space is unavailable).
context:   -
source:    Disk File Organization manual, "Basic File Structure", p. 7-4
--- END ---

--- CARD ---
id:        segment-minimization-strategy
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-file-structure
claim:     File segments are kept in close physical proximity to minimize disk head movement. Small files typically have only one segment, giving the fastest access time. It is good practice to initialize file size to the maximum expected size during creation to optimize storage allocation.
context:   -
source:    Disk File Organization manual, "Basic File Structure", p. 7-4
--- END ---

--- CARD ---
id:        segment-list-mapping
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-file-structure
claim:     The RBF file manager uses the file descriptor's segment list to maintain the file pointer, the logical end-of-file, and the conversion of logical file pointers to physical disk sector numbers, enabling efficient random access and record locking.
context:   -
source:    Disk File Organization manual, "Basic File Structure"
--- END ---

--- CARD ---
id:        initial-file-allocation
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-file-structure
claim:     When a file is created it has no data segments allocated. Write operations past the end-of-file trigger allocation of additional sectors in minimum-allocation-size increments. An attempt is made to expand the last segment before adding a new segment.
context:   -
source:    Disk File Organization manual, "Segment Allocation", p. 7-5
--- END ---

--- CARD ---
id:        segment-truncation-strategy
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-file-structure
claim:     When a file is closed, if not all allocated sectors are used, the last segment is truncated and the unused sectors are deallocated in the bitmap. However, if a file is closed while not at end-of-file (in write or update mode), the last segment is NOT truncated, preserving excess space for random-access files. A `seek(0)` before close prevents loss of that reserved space.
context:   This behavior differs from typical filesystems and is necessary to avoid rapid segment fragmentation in random-access databases.
source:    Disk File Organization manual, "Segment Allocation", p. 7-5
--- END ---

--- CARD ---
id:        file-pointer-movement-automatic
type:      FACT
target:    68k
verify:    from-manual
topic:     rbf-file-structure
claim:     When a file is opened or created, OS-9 maintains a file pointer tracking the next byte to read or write. Sequential reads/writes automatically advance the pointer (successive operations transfer data sequentially); random access uses the I$Seek system call.
context:   -
source:    Using Professional OS-9 v2.4, "OS-9 File Storage", pp. 4-1 to 4-2
--- END ---

---

## RBF disk structure: directory file format

--- CARD ---
id:        root-directory-structure
type:      CONCEPT
target:    all
verify:    from-manual
topic:     rbf-directory
claim:     The root directory is the parent directory of all other files and directories on the disk, accessed via the physical device name (e.g. /d1). It is located at the LSN specified in DD_DIR of the identification sector.
context:   -
source:    Disk File Organization manual, "Root Directory", p. 7-4
--- END ---

--- CARD ---
id:        root-directory-device
type:      FACT
target:    68k
verify:    from-manual
topic:     rbf-directory
claim:     A system's root directory (typically /h0) conventionally holds the standard system directories (CMDS, DEFS, SYSMODS, etc.) plus user directories, all organized hierarchically beneath it.
context:   -
source:    Using Professional OS-9 v2.4, "The OS-9 File System", Basic File System Oriented Utilities section, p. 4-11
--- END ---

--- CARD ---
id:        directory-32-byte-entries
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-directory
claim:     Directory files consist of integral 32-byte entries. Each entry has a 28-byte file name field (DIR_NM, bytes 0-27) with the sign bit of the last character set. The first byte is zero for deleted/unused entries. Byte 28 is unused (must be zero). Bytes 29-31 contain the 3-byte LSN of the file's FD sector (DIR_FD).
context:   -
source:    Disk File Organization manual, "Directory File Format", p. 7-6
--- END ---

--- CARD ---
id:        dot-and-double-dot-entries
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-directory
claim:     When a directory file is created, two entries are automatically created: dot (.) referring to the directory itself, and double-dot (..) referring to its parent directory.
context:   -
source:    Disk File Organization manual, "Directory File Format", p. 7-6
--- END ---

---

## RBF raw physical I/O

--- CARD ---
id:        raw-physical-io-via-at-suffix
type:      CONCEPT
target:    all
verify:    from-manual
topic:     rbf-io
claim:     A device can be opened for raw physical I/O by appending the `@` character to the device name (e.g. /d2@). Standard open/close/read/write/seek system calls apply. This allows byte/sector access by physical address, bypassing the normal file system.
context:   Used for diagnostic and utility programs; bypasses file security.
source:    Disk File Organization manual, "Raw Physical I/O on RBF Devices", p. 7-7
--- END ---

--- CARD ---
id:        raw-io-seek-calculation
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-io
claim:     For raw physical I/O, seek address = LSN x logical sector size. To find logical sector size, read PD_SctSiz from the path descriptor; if 0, assume 256 bytes. Example: for 1024-byte sectors, seek to address 3072 to read sector 3.
context:   -
source:    Disk File Organization manual, "Raw Physical I/O on RBF Devices", p. 7-7
--- END ---

--- CARD ---
id:        raw-io-security-restrictions
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-io
claim:     Only super-users can open the raw device for write. Non-super-users can only read the identification sector (LSN 0) and the allocation bitmap; attempts to read past this return an end-of-file error. Raw physical I/O bypasses file security entirely.
context:   -
source:    Disk File Organization manual, "Raw Physical I/O on RBF Devices", p. 7-7
--- END ---

---

## RBF record locking

--- CARD ---
id:        byte-level-record-locking
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     RBF provides true record locking on a byte basis. When a file is opened in update mode, any read operation locks out records starting at the current file pointer for the number of bytes requested. The locked region remains locked until the next read, write, or close.
context:   -
source:    Disk File Organization manual, "Record Locking and Unlocking", p. 7-8
--- END ---

--- CARD ---
id:        read-mode-no-locking
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     Reading files opened in read or execute mode does not cause record locking, since records cannot be updated in these modes.
context:   -
source:    Disk File Organization manual, "Record Locking and Unlocking", p. 7-8
--- END ---

--- CARD ---
id:        open-create-update-mode-guideline
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     Do not open a file for update if only reading is intended. Files opened read-only do not cause record locking and generally run faster. On multi-user systems, files routinely opened for update on shared files can cause extended record-lock contention.
context:   -
source:    Disk File Organization manual, "Record Locking Details for I/O Functions", p. 7-10
--- END ---

--- CARD ---
id:        readline-lock-behavior
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     Read and ReadLine operations cause record lock-out only if the file is opened in update mode. The locked region includes all bytes from the current file pointer for the requested byte count. A ReadLine requesting 256 bytes locks exactly 256 bytes regardless of the actual bytes read before the carriage return.
context:   -
source:    Disk File Organization manual, "Record Locking Details for I/O Functions", p. 7-10
--- END ---

--- CARD ---
id:        record-lock-release-conditions
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     A locked record is released by: another read operation, a write operation, file close, or a record-lock SetStat call. Any read or write of zero bytes releases any record lock, EOF lock, or file lock.
context:   -
source:    Disk File Organization manual, "Record Locking Details for I/O Functions", p. 7-10
--- END ---

--- CARD ---
id:        write-release-behavior
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     Write and WriteLine calls always release any currently locked record. A write of zero bytes releases the EOF lock and file lock. Writing usually does not lock any portion of the file unless it occurs at end-of-file, which gains an EOF lock.
context:   -
source:    Disk File Organization manual, "Record Locking Details for I/O Functions", p. 7-10
--- END ---

--- CARD ---
id:        seek-no-effect-locking
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     Seek operations do not affect record locking.
context:   -
source:    Disk File Organization manual, "Record Locking Details for I/O Functions", p. 7-10
--- END ---

--- CARD ---
id:        setstatus-lock-operations
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     Two SetStat codes deal with record locking: SS_Lock locks or releases part of a file; SS_Ticks sets the length of time a program will wait for a locked record.
context:   -
source:    Disk File Organization manual, "Record Locking Details for I/O Functions", p. 7-10
--- END ---

---

## File security and permissions

--- CARD ---
id:        access-control-per-path
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-security
claim:     Whenever a file is opened, access permissions are checked on all directories in the pathlist and on the file itself. Without read permission on a directory, no files in that directory are accessible.
context:   -
source:    Disk File Organization manual, "File Security", p. 7-11
--- END ---

--- CARD ---
id:        fd-own-byte-limit-caveat
type:      GOTCHA
target:    all
verify:    from-manual
topic:     rbf-security
claim:     FD_OWN is a 2-byte field, but RBF only reads the low-order byte of both group ID and user ID from the password file. A user with ID 256.512 is mistaken for the super-user by RBF.
context:   The password file permits 2 bytes for group ID and 2 bytes for user ID, but RBF truncates to 1 byte per field.
source:    Disk File Organization manual, "File Security", p. 7-11
--- END ---

--- CARD ---
id:        os9lib-s-ifmt-is-too-narrow-for-s-isreg
type:      GOTCHA
target:    68k
verify:    VERIFIED
topic:     c-library
claim:     os9lib's `<stat.h>` defines `S_IFMT` as `0x0080` -- a *single bit* -- while `S_IFREG` is `0x0100`. The stock macro `S_ISREG(m)` is therefore `((m & 0x0080) == 0x0100)`, which can never be true for any value of m. Regular files are silently misclassified, typically falling through to `S_ISCHR(m)` (`(m & 0x0080) == 0x0000`, true whenever the directory bit is clear) and being reported as character devices.
context:   This is the single nastiest header trap found in the whole port. Unix code that decodes `st_mode` with the standard macros compiles clean, links clean, runs, and quietly prints `c` for every regular file -- GNU `ls -l` did exactly this. The `S_IFMT` a porter actually needs is `0x0380` (wide enough to span S_IFDIR 0x0080, S_IFREG 0x0100 and S_IFIFO 0x0200). Beware that a project may already carry a private fix: GNU fileutils' `system.h` redefines `S_IFMT` to `0x0380` correctly, but `filemode.c` includes `<sys/stat.h>` rather than `system.h` and so never sees it -- so the same program can hold both the correct and the broken definition, in different translation units.
source:    authored
--- END ---

---

## C language I/O bindings

--- CARD ---
id:        opening-a-directory-needs-the-directory-bit
type:      GOTCHA
target:    all
verify:    VERIFIED
topic:     io
claim:     A directory cannot be opened as an ordinary file. `open(path, S_IREAD)` on a directory fails; the directory bit must be set in the access mode -- `open(path, S_IFDIR | S_IREAD)` (0x80 in the mode byte). The same applies to `fopen()`, which offers no way to set that bit and therefore cannot open a directory at all.
context:   Two consequences worth internalising. First, this is why straightforward Unix code that does `fopen(dir, "r")` to list a directory simply does not work on OS-9. Second, the failure is *useful*: since a plain `open()` fails on a directory and succeeds on a file, trying the plain open first and falling back to the dir-bit open is a reliable way to discover which one you have, without needing a separate stat.
source:    authored
--- END ---

--- CARD ---
id:        os9-line-ending-is-cr-not-lf
type:      GOTCHA
target:    all
verify:    authored
topic:     io
claim:     OS-9's line terminator is CR (0x0D), not LF (0x0A) -- including in C source: the `\n` escape produces CR on this platform's C compiler, not linefeed. A separate escape (`\e`) is defined specifically to produce a true LF when one is genuinely needed. Programs written the K&R way (using `\n` for newline) still work correctly since OS-9's own convention is CR.
context:   This is the most common host-boundary bug when moving files between a Unix host and OS-9: a Unix-authored (LF) source file can make the OS-9 C compiler read the entire file as one line, producing cascading syntax errors at spots that look nothing like the real problem. RBF stores bytes exactly with no transparent translation; conversion tools like `flip -m` only apply to host-side files, not to files already inside an RBF disk image -- those must be edited natively (`vi`/`ed`), which already produce correct native line endings. This CR-vs-LF convention is OS-9-wide, not tied to CPU architecture.
source:    authored + OS-9 C Compiler manual, "Differences From The K & R Specification" / "Control Character Escape Sequences", pp. 1-2, 1-4
--- END ---

--- CARD ---
id:        path-number-vs-file-pointer-distinct-types
type:      GOTCHA
target:    all
verify:    authored
topic:     io
claim:     OS-9 C exposes two distinct handle types for an open file: the low-level "path number" returned by system calls like `open()`/`creat()`, and the high-level "file pointer" (`FILE *`) returned by `fopen()` and used by the standard buffered I/O functions. They are not interchangeable, even though a `FILE *` internally wraps a path number plus a buffer and status structure. Passing a file pointer where a system call expects a path number, or the reverse, is a well-known beginner mistake that reliably crashes the program rather than failing gracefully.
context:   Worth calling out explicitly since Unix's analogous `int fd` vs `FILE *` distinction is more forgiving in casual misuse (though still technically wrong there too).
source:    authored + OS-9 C Compiler manual, "Introduction to the C Standard Library", p. 4-1
--- END ---

--- CARD ---
id:        three-standard-file-descriptors
type:      FACT
target:    all
verify:    from-manual
topic:     io
claim:     Three file pointers are open automatically the moment a C program starts running: stdin (path number 0), stdout (path number 1), and stderr (path number 2). All are buffered by default except stderr, which is always unbuffered.
context:   Mirrors the Unix stdio convention, but the underlying path numbers 0/1/2 are OS-9's own standard path numbering, established by the parent process (typically the Shell) before the program starts.
source:    OS-9 C Compiler manual, "Fopen", pp. 4-7 to 4-8
--- END ---

--- CARD ---
id:        l3tol-ltoi3s-3byte-lsn-conversion
type:      FACT
target:    all
verify:    from-manual
topic:     io
claim:     OS-9 stores certain system values, including disk addresses, in 3-byte form rather than 4-byte form. The standard library provides l3tol() (3-byte to long conversion) and ltoi3s() (long to 3-byte conversion) to enable C arithmetic on these values.
context:   Documents that OS-9 RBF disk block addresses (LSNs) are natively 3 bytes wide (matching the 3-byte LSN fields seen throughout the identification sector and file descriptor -- see [[dd-dir-root-directory-lsn]], [[fd-seg-segment-list]]), a fact independent of CPU word size, requiring explicit conversion when treated as C longs.
source:    OS-9 C Compiler manual, "L3tol,Ltoi3s", p. 4-15
--- END ---

--- CARD ---
id:        read-readln-raw-vs-line-edited
type:      CONCEPT
target:    all
verify:    from-manual
topic:     io
claim:     OS-9 offers two read strategies for character-oriented files/devices: read() transfers up to a requested byte count in raw ("raw mode") form with no editing of the input stream, while readln() honors device character mappings (line editing/echoing) and returns as soon as a carriage return is seen on the input. readln() is preferred for interactive terminal input; read() for other file input.
context:   The stdio getc() function auto-selects between these two underlying strategies based on a flag bit (_SCF or _RBF) set in the file structure the first time getc() is called on that stream -- reflecting whether the file's device is a Sequential Character File device or a Random Block File device. The programmer can force the choice by setting the flag bits directly before first use.
source:    OS-9 C Compiler manual, "Getc,Getchar", pp. 4-11 to 4-12
--- END ---

--- CARD ---
id:        write-writeln-distinction
type:      CONCEPT
target:    all
verify:    from-manual
topic:     io
claim:     Mirroring read()/readln(), OS-9 provides both write() (a raw byte transfer with no line-editing) and writeln() (which applies line-editing and stops writing after the first embedded `\n` in the buffer, even if the requested count hasn't been exhausted).
context:   Same SCF-vs-RBF device-class distinction as read()/readln() applies on the output side.
source:    OS-9 C Compiler manual, "Write,Writeln", p. 3-45
--- END ---

--- CARD ---
id:        getstat-setstat-status-model
type:      CONCEPT
target:    all
verify:    from-manual
topic:     io
claim:     getstat()/setstat() take a numeric "code" plus the path number of an already-open file and operate on a path descriptor's status/option area. Code 0 reads/writes a 32-byte status-packet buffer (device- and file-manager-specific structures); code 2 gets/sets the current file size as a long integer; other codes are defined per device/file-manager class.
context:   Full semantics for each code are deferred to the OS-9 System Programmer's Manual -- this compiler's manual only summarizes enough to call the C wrappers correctly.
source:    OS-9 C Compiler manual, "Getstat", pp. 3-16 to 3-17; "Setstat", p. 3-37
--- END ---

---

## Device naming and console

--- CARD ---
id:        typical-device-names
type:      FACT
target:    all
verify:    from-manual
topic:     io
claim:     Typical OS-9 device names: /d0 /d1 (floppies), /h0 /hOfmt (hard disk, with/without format protect), /mt0 (tape), /nil (null device), /p /p1 /p2 (parallel/serial printers), /r0 (RAM disk), /term /t1 /t2 (terminals). All device names begin with `/`; a device name is also its device descriptor module name.
context:   -
source:    The OS-9 Guru, §2.1, figure 2, p. 21 + Using Professional OS-9 v2.4, "Backing Up System Disk" section, p. 2-3
--- END ---

--- CARD ---
id:        default-device-concept
type:      FACT
target:    all
verify:    from-manual
topic:     io
claim:     `/dd` (default device) is an alias for the primary storage device (hard disk or RAM disk); only one dd descriptor is loaded at a time. Programs use `/dd` for config/data files; it can be changed by loading a different device descriptor.
context:   Usually the hard disk; some configurations use a RAM disk. Changing dd requires a new boot file/ROM or loading a higher-revision descriptor.
source:    The OS-9 Guru, §2.10, p. 30
--- END ---

--- CARD ---
id:        console-system-device-debugger
type:      FACT
target:    68k
verify:    from-manual
topic:     io
claim:     The ROM debugger only communicates with the system's console device; the `break` utility should only be called from the console. If invoked from another terminal, the console must be used to communicate with the debugger.
context:   -
source:    Using Professional OS-9 v2.4, "The OS-9 Utilities", break section, p. 10
--- END ---

---

## Disk and directory utilities

--- CARD ---
id:        disk-format-parameters-density-sides
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-operations
claim:     The `format` utility accepts command-line parameters: -sd (single density), -dd (double density), -ss (single sided), -ds (double sided); the correct format varies by disk drive type and manufacturer and is usually set to maximum capacity.
context:   Refer to hardware documentation and the system disk label for correct format parameters.
source:    Using Professional OS-9 v2.4, "Starting OS-9", Formatting a Disk section, p. 2-3
--- END ---

--- CARD ---
id:        backup-utility-two-pass-verification
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-operations
claim:     The `backup` utility makes exact disk copies in two passes: the first pass reads the source disk into a memory buffer and writes it to the destination; the second pass verifies all data copied correctly. Errors on the first pass indicate a source disk problem; errors on the second pass indicate a destination disk problem. Never back up a system disk to a destination disk that `format` reported any bad sectors on -- backup will likely fail; if backup repeatedly fails on the second pass, reformat the destination to rule out bad sectors.
context:   -
source:    Using Professional OS-9 v2.4, "Starting OS-9", The Backup Procedure section, p. 2-5 + Formatting a Disk section, pp. 2-4 to 2-5
--- END ---

--- CARD ---
id:        format-track-verification-bad-sectors
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-operations
claim:     During format's verification phase, the hexadecimal number of each track is displayed as it is verified; if bad sectors are found, an error message shows the sector number, and the output includes good-sector count, unusable-sector count, and total verified sectors.
context:   -
source:    Using Professional OS-9 v2.4, "Starting OS-9", Formatting a Disk section, pp. 2-4 to 2-5
--- END ---

--- CARD ---
id:        free-disk-space-sectors-bytes
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-management
claim:     The `free` utility displays unused disk space in sectors and bytes, plus disk name, creation date, cluster size, and largest free block size. It uses a 4K buffer by default; this can be increased with the -b option (e.g. -b=10 for 10K).
context:   -
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Free and Mfree section, p. 3-9
--- END ---

--- CARD ---
id:        dir-command-options
type:      FACT
target:    68k
verify:    from-manual
topic:     file-commands
claim:     The `dir` utility lists directory contents; typing `dir` alone lists the current data directory, or add a pathlist to view another directory without changing the current one. The -e option gives an extended listing (owner, last-modified date, attributes, storage sector, byte count); the -r option recurses into subdirectories; -e and -r combine as -er.
context:   Wildcards are also accepted in the pathlist argument.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", Dir section and Dir Options section, pp. 4-11 to 4-12
--- END ---

--- CARD ---
id:        pd-display-current-path
type:      FACT
target:    68k
verify:    from-manual
topic:     file-commands
claim:     The `pd` utility displays the complete pathlist from the root directory to the current data directory; `pd -x` displays the pathlist to the current execution directory.
context:   Helps when confusion arises about location in the file system.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", Using the Pd Utility section, p. 4-16
--- END ---
