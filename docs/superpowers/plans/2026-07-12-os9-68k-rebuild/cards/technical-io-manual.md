--- CARD ---
id:        unified-io-system-components
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     io-architecture
claim:     OS-9 I/O system consists of four modular components: the kernel, file managers, device drivers, and device descriptors. These can be installed, removed, or customized while the system runs.
context:   Fundamental architectural principle; applies to all OS-9/68k systems
source:    Technical I/O Manual v2.4, "The OS-9 Unified Input/Output System", p. 1-1
--- END ---

--- CARD ---
id:        kernel-io-supervision
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     kernel-io
claim:     The kernel supervises the overall I/O system by maintaining I/O module data structures and ensuring the appropriate file manager and device driver process each request.
context:   Central to understanding how requests flow through the system
source:    Technical I/O Manual v2.4, "The Kernel and I/O", p. 1-4
--- END ---

--- CARD ---
id:        file-manager-class-operations
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     file-managers
claim:     File managers handle logical operations on a class of devices (e.g., RBF manages disk directories, SCF edits terminal data streams). They work at the generic class level, not individual devices.
context:   Explains the abstraction boundary between logical and physical device management
source:    Technical I/O Manual v2.4, "The Kernel and I/O", p. 1-4
--- END ---

--- CARD ---
id:        driver-hardware-translation
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     device-drivers
claim:     Device drivers operate on physical hardware, translating file manager logical requests into specific hardware operations. They isolate file managers from hardware dependencies like control register organization and data transfer modes.
context:   -
source:    Technical I/O Manual v2.4, "The Kernel and I/O", p. 1-5
--- END ---

--- CARD ---
id:        file-manager-data-processing
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     file-managers
claim:     File managers process raw data streams to/from device drivers for similar device classes. They buffer data, allocate buffers dynamically, monitor and transform data (e.g., adding line-feeds after carriage returns), and handle mass storage allocation and directory processing.
context:   File managers are re-entrant; one instance serves an entire class of similar devices
source:    Technical I/O Manual v2.4, "File Managers", p. 1-17
--- END ---

--- CARD ---
id:        file-manager-reentrant
type:      FACT
target:    68k
verify:    from-manual
topic:     file-managers
claim:     File managers are re-entrant. A single file manager instance can be used for an entire class of devices with similar operational characteristics. OS-9 systems can have any number of file manager modules loaded simultaneously.
context:   -
source:    Technical I/O Manual v2.4, "File Managers", p. 1-17
--- END ---

--- CARD ---
id:        io-module-ownership-requirements
type:      FACT
target:    68k
verify:    from-manual
topic:     io-architecture
claim:     I/O system modules (file managers, drivers, descriptors) must be owned by a super-user (0.n) and have the system-state bit set in the module header attribute byte.
context:   System-state bit requirement may be enforced in future OS-9 revisions
source:    Technical I/O Manual v2.4, "File Managers", p. 1-17
--- END ---

--- CARD ---
id:        device-descriptor-information
type:      FACT
target:    68k
verify:    from-manual
topic:     device-descriptors
claim:     Device descriptors contain: device name (logical name), hardware port addresses, associated file manager name, device driver name, and device operating parameters (fixed like interrupt level/port, or variable like terminal editing settings).
context:   Descriptors enable reusing the same driver for multiple ports
source:    Technical I/O Manual v2.4, "The Kernel and I/O", p. 1-5
--- END ---

--- CARD ---
id:        device-descriptor-module-type
type:      FACT
target:    68k
verify:    from-manual
topic:     device-descriptors
claim:     Device descriptor modules are non-executable memory modules with type code DEVIC. The module name serves as the device's logical name (e.g., /d0 corresponds to descriptor d0).
context:   -
source:    Technical I/O Manual v2.4, "Device Descriptor Modules", p. 1-8 to 1-9
--- END ---

--- CARD ---
id:        device-descriptor-multiple-synonyms
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     device-descriptors
claim:     A single I/O device may have multiple device descriptors with different names and initialization parameters. For example, a serial/parallel driver can have one descriptor for terminal mode (/T1) and another for printer mode (/P1) on the same port.
context:   Kernel matching logic on I$Attach distinguishes these as "synonymous devices"
source:    Technical I/O Manual v2.4, "Device Descriptor Modules", p. 1-8
--- END ---

--- CARD ---
id:        device-descriptor-standard-fields
type:      FACT
target:    68k
verify:    from-manual
topic:     device-descriptors
claim:     Standard device descriptor fields at fixed offsets from module base: M$Port ($30) = hardware port address, M$Vector ($34) = interrupt vector number, M$IRQLvl ($35) = interrupt level, M$Prior ($36) = polling priority, M$Mode ($37) = access mode capabilities, M$FMgr ($38) = file manager name offset, M$PDev ($3A) = driver name offset, M$DevCon ($3C) = device config offset, M$Opt ($46) = init table size, M$DTyp ($48) = device type code.
context:   These fields are module-relative offsets; resolved in assembly via sys.l or usr.l
source:    Technical I/O Manual v2.4, "Device Descriptor Modules", p. 1-10
--- END ---

--- CARD ---
id:        interrupt-vector-ranges-68k
type:      FACT
target:    68k
verify:    from-manual
topic:     interrupts
claim:     Interrupt vector numbering: 25–31 for auto-vectored interrupts (levels 1–7), 57–63 for 68070 on-chip auto-vectored interrupts (levels 1–7), 64–255 for vectored interrupts.
context:   These vector numbers are 68k-specific; used in M$Vector field
source:    Technical I/O Manual v2.4, "Device Descriptor Modules", p. 1-10
--- END ---

--- CARD ---
id:        level-7-interrupt-non-maskable
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     interrupts
claim:     Level 7 interrupts are non-maskable and can interrupt the kernel during critical system operations. Level 7 should not be used by OS-9 I/O devices (reserved for hardware unknown to the system, e.g., dynamic RAM refresh).
context:   A device configured at level 7 risks corrupting system state
source:    Technical I/O Manual v2.4, "Device Descriptor Modules", p. 1-11
--- END ---

--- CARD ---
id:        device-descriptor-mode-bits
type:      FACT
target:    68k
verify:    from-manual
topic:     device-descriptors
claim:     M$Mode (device descriptor offset $37) bits: bit 0 = read access, bit 1 = write access, bit 2 = executable access, bit 6 = single-user (non-sharable), bit 7 = directory file access. Kernel validates caller's access mode against this.
context:   Used to enforce access control on device open/create
source:    Technical I/O Manual v2.4, "Device Descriptor Modules", p. 1-11
--- END ---

--- CARD ---
id:        device-descriptor-m-devcon
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     device-descriptors
claim:     M$DevCon (device descriptor offset $3C) points to an optional device configuration table for driver-specific parameters or OEM constants. Unlike the standard initialization table, M$DevCon values are NOT copied into path descriptors; kernel and file manager do not reference them.
context:   Available to driver only during INIT and TERM; other routines must search device table to find descriptor
source:    Technical I/O Manual v2.4, "Device Descriptor Modules", p. 1-12
--- END ---

--- CARD ---
id:        device-type-class-codes
type:      FACT
target:    68k
verify:    from-manual
topic:     device-descriptors
claim:     M$DTyp (first byte of init table) indicates device class: 0=SCF, 1=RBF, 2=PIPEMAN, 3=SBF, 4=NFM, 5=CDFM, 6=UCM, 7=SOCKMAN, 8=PKMAN, 9=IFMAN, 10=NVRAM, 11=GFM.
context:   Kernel uses this to route to the correct file manager
source:    Technical I/O Manual v2.4, "Device Descriptor Modules", p. 1-12 to 1-13
--- END ---

--- CARD ---
id:        device-descriptor-m-opt-max-128
type:      FACT
target:    68k
verify:    from-manual
topic:     device-descriptors
claim:     M$Opt (device descriptor offset $46) contains the size of the device's standard initialization table (M$DTyp through M$DTyp + M$Opt). Theoretical maximum is 128 bytes, but individual file managers may enforce a smaller ceiling.
context:   Values in this table are copied into the path descriptor option area when path opens
source:    Technical I/O Manual v2.4, "Device Descriptor Modules", p. 1-13
--- END ---

--- CARD ---
id:        path-descriptor-256-byte-layout
type:      FACT
target:    68k
verify:    from-manual
topic:     path-descriptors
claim:     Path descriptors are 256 bytes, divided into three sections: universal header (42 bytes, all file managers), file-manager-specific (86 bytes), and option area (128 bytes for operating parameters and user-modifiable settings).
context:   The option area is copied from the device descriptor when the path opens
source:    Technical I/O Manual v2.4, "Path Descriptors", p. 1-14
--- END ---

--- CARD ---
id:        path-descriptor-universal-fields
type:      FACT
target:    68k
verify:    from-manual
topic:     path-descriptors
claim:     Universal path descriptor fields: PD_PD ($00) = path number, PD_MOD ($02) = access mode, PD_DEV ($04) = device table entry address, PD_CPR ($08) = requester process ID, PD_RGS ($0A) = caller register stack address, PD_BUF ($0E) = data buffer address, PD_USER ($12) = owner group/user ID, PD_COUNT ($1A) = path use count.
context:   Kernel maintains these; offsets are module-relative
source:    Technical I/O Manual v2.4, "Path Descriptors", p. 1-14 to 1-15
--- END ---

--- CARD ---
id:        path-descriptor-extended-fields
type:      FACT
target:    68k
verify:    from-manual
topic:     path-descriptors
claim:     Extended path descriptor fields: PD_LProc ($1C) = last active process ID, PD_ErrNo ($20) = global errno for C file managers, PD_SysGlob ($24) = system global pointer for C file managers, PD_FST ($2A) = file manager working storage (86 bytes reserved), PD_OPT ($80) = option table (128 bytes).
context:   -
source:    Technical I/O Manual v2.4, "Path Descriptors", p. 1-14 to 1-15
--- END ---

--- CARD ---
id:        path-descriptor-option-area-user-modifiable
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     path-descriptors
claim:     Path descriptor option area (PD_OPT, offset $80, 128 bytes) is initialized by copying the device descriptor init table. Users can inspect all values via I$GetStt(SS_Opt) and modify some via I$SetStt; file manager protects certain values from inappropriate changes.
context:   Allows per-path customization (e.g., terminal backspace/delete characters)
source:    Technical I/O Manual v2.4, "Device Descriptor Modules", p. 1-13
--- END ---

--- CARD ---
id:        path-descriptor-user-id-ownership
type:      FACT
target:    68k
verify:    from-manual
topic:     path-descriptors
claim:     PD_USER (path descriptor offset $12) stores the group/user ID of the process that originally created the path. This is used for permission checks on subsequent I/O operations.
context:   -
source:    Technical I/O Manual v2.4, "Path Descriptors", p. 1-15
--- END ---

--- CARD ---
id:        path-descriptor-count-field-usage
type:      FACT
target:    68k
verify:    from-manual
topic:     path-descriptors
claim:     PD_COUNT (path descriptor offset $1A) tracks the number of open paths sharing this descriptor. Set to 1 on first open; incremented by I$Dup. When decremented to 0 via I$Close, the path descriptor is deallocated.
context:   Enables multiple processes to share a single open file context
source:    Technical I/O Manual v2.4, "Path Descriptors", p. 1-14 to 1-15
--- END ---

--- CARD ---
id:        kernel-device-and-path-tables
type:      FACT
target:    68k
verify:    from-manual
topic:     kernel-io
claim:     Kernel maintains two key internal data structures: the device table (list of all attached devices) and the path table (list of all open I/O paths). These tables reflect the device descriptor and path descriptor structures respectively.
context:   -
source:    Technical I/O Manual v2.4, "The Kernel and I/O", p. 1-4
--- END ---

--- CARD ---
id:        i-attach-device-table-matching
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     kernel-io
claim:     I$Attach compares device specifications against existing device table entries: if port, file manager, driver, and descriptor all match, increment use count; if only port/manager/driver match (descriptor differs), create new entry for synonymous device; if no match, allocate driver storage, set V_PORT, and call driver INIT.
context:   This logic enables multiple device names on the same port with different parameters
source:    Technical I/O Manual v2.4, "The Kernel and I/O", p. 1-5 to 1-7
--- END ---

--- CARD ---
id:        i-attach-init-error-handling
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     kernel-io
claim:     If driver INIT returns an error during I$Attach, the kernel calls driver TERM, deallocates any allocated resources, and returns the error to the caller without creating a device table entry.
context:   Ensures failed INIT cleanup is symmetric
source:    Technical I/O Manual v2.4, "The Kernel and I/O", p. 1-6
--- END ---

--- CARD ---
id:        i-detach-use-count-cleanup
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     kernel-io
claim:     I$Detach decrements the device use count. When it reaches zero, the kernel checks if another device shares the same static storage; if not, calls driver TERM, deallocates storage, and removes the device table entry.
context:   Ensures drivers clean up resources only when the last reference closes
source:    Technical I/O Manual v2.4, "The Kernel and I/O", p. 1-6
--- END ---

--- CARD ---
id:        i-dup-path-count
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     kernel-io
claim:     I$Dup increments the use count (PD_COUNT) of an existing path descriptor instead of creating a new one. Does not call file manager or driver.
context:   -
source:    Technical I/O Manual v2.4, "Kernel I/O Service Requests", p. 1-7
--- END ---

--- CARD ---
id:        path-creation-lifecycle
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     path-descriptors
claim:     Each I$Open or I$Create creates a new path descriptor and adds an entry to the path table. I$Dup increments the use count of an existing descriptor. When use count reaches 0 after I$Close, the descriptor is deallocated and removed from the path table.
context:   -
source:    Technical I/O Manual v2.4, "The Kernel and I/O", p. 1-6
--- END ---

--- CARD ---
id:        file-manager-request-routing
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     file-managers
claim:     Kernel handles I$Attach, I$Detach, I$Dup directly without calling file managers. File managers process all other requests (I$Open, I$Create, I$Read, I$Write, I$Close, I$GetStt, I$SetStt, etc.).
context:   I$Attach/Detach/Dup manage device attachment; file managers manage file/path state
source:    Technical I/O Manual v2.4, "Kernel I/O Service Requests", p. 1-7
--- END ---

--- CARD ---
id:        file-manager-offset-table-entry-points
type:      FACT
target:    68k
verify:    from-manual
topic:     file-managers
claim:     File manager modules are a collection of subroutines accessed through an offset table. The table contains the starting address of each subroutine relative to the table base. The M$Exec module header field points to this offset table.
context:   Standard entry points include Create, Open, MakDir, ChgDir, Delete, Seek, Read, Write, ReadLn, WriteLn, GetStat, SetStat, Close
source:    Technical I/O Manual v2.4, "File Manager Organization", p. 1-17
--- END ---

--- CARD ---
id:        file-manager-register-conventions
type:      FACT
target:    68k
verify:    from-manual
topic:     file-managers
claim:     When kernel calls file manager routines, standard parameters are passed: (a1) = path descriptor address, (a4) = process descriptor address, (a5) = caller register stack address, (a6) = system global data area address. Routines are called in system state.
context:   These conventions apply to RBF and SCF; SBF has variations (see CAVEAT)
source:    Technical I/O Manual v2.4, "File Manager Organization", p. 1-18
--- END ---

--- CARD ---
id:        i-chgdir-directory-search
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$ChgDir searches for a directory file on multi-file devices. If found, saves directory address in caller's process descriptor (P$DIO). Subsequent I$Open/I$Create search this directory when pathlist does not start with /. File managers without directory support return carry set and E_UNSUPPORTED.
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-19
--- END ---

--- CARD ---
id:        i-close-flush-buffers
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$Close ensures all output to a device is completed (flushing last buffer if needed) and releases allocated buffer space. May perform end-of-file processing (e.g., writing EOF records on tapes).
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-19
--- END ---

--- CARD ---
id:        i-create-synonymous-with-open
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$Create performs the same function as I$Open. On multi-file devices, it creates a new file. File managers without multi-file support treat I$Create as synonymous with I$Open.
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
context:   Only for multi-file device managers
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-19
--- END ---

--- CARD ---
id:        i-getstat-device-status-wildcard
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$GetStt is a wildcard call to determine status of various device/file manager features. File manager handles known status codes; unknown codes are passed to driver for device-specific status reporting.
context:   Example: get file size, device capacity, device-specific configuration
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-19
--- END ---

--- CARD ---
id:        i-makdir-directory-creation
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$MakDir creates a directory file on multi-file devices. File managers without directory support return carry set and an unknown service error.
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-19
--- END ---

--- CARD ---
id:        i-open-allocate-initialize
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$Open allocates required buffers, initializes path descriptor variables, and parses pathname. On multi-file devices, performs directory searching to locate the specified file.
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-19
--- END ---

--- CARD ---
id:        i-read-returns-requested-bytes
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$Read returns the number of bytes requested to the user's data buffer. Returns EOF error if no further data available. Generally performs no editing on data. File manager may use internal buffer or direct buffer, copying data to user area if needed.
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-19 to 1-20
--- END ---

--- CARD ---
id:        i-readln-terminates-on-carriage-return
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$ReadLn differs from I$Read: (1) terminates on first carriage return (end-of-record), (2) performs input editing appropriate for the device. Typically uses internal buffer and copies to user area.
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-20
--- END ---

--- CARD ---
id:        i-seek-logical-positioning
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$Seek (random-access devices only) repositions file pointer to specified byte. Logical movement; does not affect physical device. No error if position beyond EOF. Devices without random access support do nothing and return no error.
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-20
--- END ---

--- CARD ---
id:        i-setstat-set-device-status
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$SetStt sets status of various device/file manager features. File manager handles known codes; unknown codes passed to driver. Example: format disk track (behavior differs by controller type).
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-20
--- END ---

--- CARD ---
id:        i-write-expand-file
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$Write writes data (generally unedited) using driver output routine. Writing past EOF expands file with new data. On fixed-length-record devices (e.g., RBF), often must pre-read sector before writing unless entire sector is written.
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-20
--- END ---

--- CARD ---
id:        i-writeln-output-editing
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     file-managers
claim:     I$WriteLn transfers data up to and including first carriage return (end-of-record), performing output editing (e.g., SCF appends line-feed after CR and nulls if appropriate).
context:   -
source:    Technical I/O Manual v2.4, "File Manager I/O Service Requests", p. 1-20
--- END ---

--- CARD ---
id:        driver-module-format-standard
type:      FACT
target:    68k
verify:    from-manual
topic:     device-drivers
claim:     Device driver modules conform to standard OS-9 memory module format with type code Drivr. Should have system-state bit set in module header attribute byte. Must be owned by super-user (0.n).
context:   -
source:    Technical I/O Manual v2.4, "Device Driver Modules", p. 1-21
--- END ---

--- CARD ---
id:        driver-entry-point-offset-table
type:      FACT
target:    68k
verify:    from-manual
topic:     device-drivers
claim:     Driver M$Exec field points to offset table with seven entry point offsets: Init, Read, Write, GetStat, SetStat, Term, Trap. The M$Mem field specifies local static storage size required (global I/O storage + file manager storage + driver variables).
context:   Trap is currently unused (offset should be 0)
source:    Technical I/O Manual v2.4, "Driver Module Format", p. 1-21
--- END ---

--- CARD ---
id:        driver-init-term-register-conventions
type:      FACT
target:    68k
verify:    from-manual
topic:     device-drivers
claim:     INIT and TERM routines (called by kernel) receive: (a1) = device descriptor address, (a2) = driver static storage address, (a4) = process descriptor address, (a6) = system global storage address.
context:   INIT initializes hardware and enables interrupts; TERM de-initializes and disables interrupts
source:    Technical I/O Manual v2.4, "Driver Module Format", p. 1-22
--- END ---

--- CARD ---
id:        driver-read-write-getstat-register-conventions
type:      FACT
target:    68k
verify:    from-manual
topic:     device-drivers
claim:     READ, WRITE, GETSTAT, SETSTAT routines (called by file manager) receive: (a1) = path descriptor, (a2) = driver static storage, (a4) = process descriptor, (a5) = caller register stack, (a6) = system global storage. CAVEAT: SBF READ/WRITE have undefined a1/a5; SBF GETSTAT/SETSTAT have undefined a5.
context:   All routines execute in system state
source:    Technical I/O Manual v2.4, "Driver Module Format", p. 1-22
--- END ---

--- CARD ---
id:        driver-irq-register-conventions
type:      FACT
target:    68k
verify:    from-manual
topic:     device-drivers
claim:     IRQ routine (called by kernel interrupt polling) receives: (a2) = driver static storage, (a3) = device port address, (a6) = system global storage. May only destroy d0, d1, a0, a2, a3, a6. Returns carry clear if serviced, carry set if not serviced (continue polling).
context:   a2 and a3 values are those from F$IRQ installation
source:    Technical I/O Manual v2.4, "Driver Module Format", p. 1-23
--- END ---

--- CARD ---
id:        driver-subroutine-error-return
type:      FACT
target:    68k
verify:    from-manual
topic:     device-drivers
claim:     All driver subroutines end with RTS. Error status returned via CCR carry bit; error code in (d1.w). For IRQ routine, only CCR carry is meaningful.
context:   -
source:    Technical I/O Manual v2.4, "Driver Module Format", p. 1-23
--- END ---

--- CARD ---
id:        driver-multi-port-static-storage
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     device-drivers
claim:     Multi-port drivers share a single static storage area across all ports on the same device. The driver distinguishes ports using device table entry or V_PORT field. A single driver instance manages multiple independent device paths.
context:   Reduces memory overhead; driver code is shared; per-port data stored in static extension
source:    Technical I/O Manual v2.4, "Device Drivers that Control Multiple Devices", p. 1-27 to 1-28
--- END ---

--- CARD ---
id:        driver-multi-class-different-managers
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     device-drivers
claim:     Multi-class drivers handle different device classes (e.g., block and character modes) by supporting different file managers. A single driver associates with multiple device descriptors, each pointing to a different file manager class.
context:   Allows hardware to be used flexibly; common with intelligent controllers
source:    Technical I/O Manual v2.4, "Device Drivers that Control Multiple Devices", p. 1-31
--- END ---

--- CARD ---
id:        interrupt-driven-io-irq-routine
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     interrupts
claim:     Interrupt-driven I/O uses driver IRQ routines installed on kernel's IRQ poll table. When hardware interrupts, kernel checks poll table and calls matching IRQ routines. IRQ routines service device and resume waiting processes.
context:   Alternative to polling; improves responsiveness and CPU efficiency
source:    Technical I/O Manual v2.4, "Interrupt Driven I/O", p. 1-37
--- END ---

--- CARD ---
id:        dma-syscache-module-coordination
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     dma-caching
claim:     DMA devices must coordinate with Syscache module to avoid stale data problems. When DMA transfers bypass CPU cache, Syscache ensures coherency by invalidating/flushing affected cache lines.
context:   Critical for systems with instruction/data caches; missing coordination causes silent data corruption
source:    Technical I/O Manual v2.4, "DMA I/O and System Caches", p. 1-39
--- END ---

--- CARD ---
id:        dma-address-translation-mmu
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     dma-caching
claim:     On systems with an MMU, DMA transfers from user buffers require address translation (DMA hardware may not respect virtual-to-physical mappings). OS-9 provides address translation services for DMA.
context:   MMU-based systems must map user virtual addresses to physical addresses for DMA
source:    Technical I/O Manual v2.4, "Address Translation and DMA Transfers", p. 1-42
--- END ---

--- CARD ---
id:        rbf-random-block-file-manager
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     rbf
claim:     Random Block File Manager (RBF) manages random-access disk devices. It maintains directory structures, handles block-oriented I/O, and manages file allocation on block devices.
context:   RBF is one of three main file managers (others: SCF for terminals, SBF for sequential block storage)
source:    Technical I/O Manual v2.4, "File Managers", p. 1-17
--- END ---

--- CARD ---
id:        scf-sequential-character-file-manager
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     scf
claim:     Sequential Character File Manager (SCF) manages character-oriented devices like terminals. It performs line editing, handles special characters/NULLs, manages parity stripping, and implements data flow control.
context:   -
source:    Technical I/O Manual v2.4, "File Managers", p. 1-17
--- END ---

--- CARD ---
id:        sbf-sequential-block-file-manager
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     sbf
claim:     Sequential Block File Manager (SBF) manages sequential block-oriented mass storage devices like tape drives. It handles buffered/unbuffered I/O, tape positioning, end-of-tape processing, and tape streaming.
context:   -
source:    Technical I/O Manual v2.4, "File Managers", p. 1-17
--- END ---

--- CARD ---
id:        pipeman-pipe-file-manager
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     file-managers
claim:     Pipe File Manager (PIPEMAN) supports interprocess communication through memory buffers called pipes.
context:   -
source:    Technical I/O Manual v2.4, "File Managers", p. 1-17
--- END ---
