# Chapter 11.2 System Mode Service Requests — Privileged System Calls

## F$All64 — Allocate 64-byte Memory Block

--- CARD ---
id:        f-all64-hex-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-all64
claim:     F$All64 machine code is 0x30 (in 103F 30 instruction format)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.1, line 5238
--- END ---

--- CARD ---
id:        f-all64-input-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-all64
claim:     F$All64 on entry takes X register containing base address of page table (or zero if not yet allocated)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.1, line 5242-5243
--- END ---

--- CARD ---
id:        f-all64-output-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-all64
claim:     F$All64 on success returns A=block number, X=base address of page table, Y=address of allocated block
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.1, lines 5245-5247
--- END ---

--- CARD ---
id:        f-all64-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-all64
claim:     F$All64 dynamically allocates 64-byte blocks of memory by splitting 256-byte pages into four 64-byte sections; first section serves as a page table storing the MSB of all pages; passing X=0 allocates a new base page and first block; new pages obtained via automatic F$SRqMem calls
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.1, lines 5254-5257
--- END ---

--- CARD ---
id:        f-all64-error
type:      FACT
target:    6809
verify:    from-manual
topic:     f-all64
claim:     F$All64 error on LI is E$PthFul (page table full)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.1, line 5252
--- END ---

--- CARD ---
id:        f-all64-usage-li-vs-lii
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-all64
claim:     LI uses F$All64 for both path and process descriptors; LII uses it only for path descriptors because LII process descriptors are 512 bytes
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.1, lines 5289-5290
--- END ---

--- CARD ---
id:        f-all64-block-self-marking
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-all64
claim:     First byte of each 64-byte block contains its block number; callers must not alter this byte
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.1, line 5259
--- END ---

--- CARD ---
id:        f-all64-privileged
type:      FACT
target:    6809
verify:    from-manual
topic:     f-all64
claim:     F$All64 is a privileged system mode service request
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.1, line 5297
--- END ---

## F$AProc — Insert Process in Active Process Queue

--- CARD ---
id:        f-aproc-hex-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-aproc
claim:     F$AProc machine code is 0x2C (in 103F 2C instruction format)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.2, line 5303
--- END ---

--- CARD ---
id:        f-aproc-input-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-aproc
claim:     F$AProc on entry takes X register containing address of process descriptor
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.2, line 5307
--- END ---

--- CARD ---
id:        f-aproc-output-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-aproc
claim:     F$AProc produces no return value on success (None)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.2, line 5309
--- END ---

--- CARD ---
id:        f-aproc-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-aproc
claim:     F$AProc inserts a process into the active process queue for scheduling; ages all existing queued processes and sets the new process's age to its priority value; insertion is by relative age
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.2, lines 5314-5318
--- END ---

--- CARD ---
id:        f-aproc-privileged
type:      FACT
target:    6809
verify:    from-manual
topic:     f-aproc
claim:     F$AProc is a privileged system mode service request
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.2, line 5325
--- END ---

## F$Find64 — Find 64-byte Memory Block

--- CARD ---
id:        f-find64-hex-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-find64
claim:     F$Find64 machine code is 0x2F (in 103F 2F instruction format)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.3, line 5330
--- END ---

--- CARD ---
id:        f-find64-input-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-find64
claim:     F$Find64 on entry takes A=block number and X=address of base page
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.3, lines 5334-5335
--- END ---

--- CARD ---
id:        f-find64-output-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-find64
claim:     F$Find64 on success returns Y=address of the 64-byte block
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.3, line 5337
--- END ---

--- CARD ---
id:        f-find64-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-find64
claim:     F$Find64 returns the address of a 64-byte memory block given its block number; used to locate process or path descriptors by number
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.3, lines 5348-5350
--- END ---

--- CARD ---
id:        f-find64-block-numbering
type:      FACT
target:    6809
verify:    from-manual
topic:     f-find64
claim:     Block numbers range from 1 to N (not zero-indexed)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.3, line 5352
--- END ---

--- CARD ---
id:        f-find64-error-condition
type:      FACT
target:    6809
verify:    from-manual
topic:     f-find64
claim:     F$Find64 error output sets C bit but does not list specific error code; manual states condition indicates block not allocated or not in use
context:   OCR unclear: line 5346 spans page boundary and message is not in standard error-code format
source:    OS-9 System Programmer's Manual, Chapter 11.2.3, lines 5339-5346
--- END ---

--- CARD ---
id:        f-find64-usage-li-vs-lii
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-find64
claim:     LI uses F$Find64 for both process and path descriptors; LII uses it only for path descriptors because LII process descriptors are 512 bytes (see F$GProcP)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.3, lines 5354-5355
--- END ---

--- CARD ---
id:        f-find64-privileged
type:      FACT
target:    6809
verify:    from-manual
topic:     f-find64
claim:     F$Find64 is a privileged system mode service request
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.3, line 5362
--- END ---

## F$IODel — Delete I/O Device from System

--- CARD ---
id:        f-iodel-hex-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-iodel
claim:     F$IODel machine code is 0x33 (in 103F 33 instruction format)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.4, line 5367
--- END ---

--- CARD ---
id:        f-iodel-input-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-iodel
claim:     F$IODel on entry takes X register containing address of an I/O module
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.4, line 5371
--- END ---

--- CARD ---
id:        f-iodel-output-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-iodel
claim:     F$IODel produces no return value on success (None)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.4, line 5373
--- END ---

--- CARD ---
id:        f-iodel-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-iodel
claim:     F$IODel searches the device table for the I/O module address; if found and use count is zero, deletes the module; if use count is nonzero, returns error
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.4, lines 5380-5382
--- END ---

--- CARD ---
id:        f-iodel-error
type:      FACT
target:    6809
verify:    from-manual
topic:     f-iodel
claim:     F$IODel error is E$ModBsy (module busy — use count not zero)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.4, line 5378
--- END ---

--- CARD ---
id:        f-iodel-li-termination
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-iodel
claim:     On LI, F$IODel performs the device termination routine when a device is unlinked for the final time (no other processes using it); on LII, this has been moved to the DETACH system call
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.4, lines 5385-5387
--- END ---

--- CARD ---
id:        f-iodel-lii-behavior
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-iodel
claim:     On LII, F$IODel returns information to the UNLINK system call after determining if a device is busy or not
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.4, lines 5389-5390
--- END ---

--- CARD ---
id:        f-iodel-primary-user
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-iodel
claim:     F$IODel is primarily used by IOMAN and may have limited or no use for other applications
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.4, lines 5392-5393
--- END ---

--- CARD ---
id:        f-iodel-privileged
type:      FACT
target:    6809
verify:    from-manual
topic:     f-iodel
claim:     F$IODel is a privileged system mode service request
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.4, line 5402
--- END ---

## F$IOQu — Enter I/O Queue

--- CARD ---
id:        f-ioqu-hex-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ioqu
claim:     F$IOQu machine code is 0x2B (in 103F 2B instruction format)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.5, line 5413
--- END ---

--- CARD ---
id:        f-ioqu-input-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ioqu
claim:     F$IOQu on entry takes A register containing process number
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.5, line 5417
--- END ---

--- CARD ---
id:        f-ioqu-output-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ioqu
claim:     F$IOQu produces no return value on success (None)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.5, line 5419
--- END ---

--- CARD ---
id:        f-ioqu-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ioqu
claim:     F$IOQu links the calling process into the I/O queue of a specified process and performs an untimed sleep; it is assumed associated routines will later send a wakeup signal
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.5, lines 5424-5426
--- END ---

--- CARD ---
id:        f-ioqu-primary-user
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-ioqu
claim:     F$IOQu is used primarily and extensively by IOMAN and file managers
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.5, line 5426
--- END ---

--- CARD ---
id:        f-ioqu-privileged
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ioqu
claim:     F$IOQu is a privileged system mode service request
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.5, line 5435
--- END ---

## F$IRQ — Add or Remove Device from IRQ Table

--- CARD ---
id:        f-irq-hex-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-irq
claim:     F$IRQ machine code is 0x2A (in 103F 2A instruction format)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.6, line 5440
--- END ---

--- CARD ---
id:        f-irq-input-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-irq
claim:     F$IRQ on entry takes D=address of device status register, X=zero to remove or address of control packet, Y=device IRQ service routine address, U=address of service routine's static storage area
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.6, lines 5444-5453
--- END ---

--- CARD ---
id:        f-irq-output-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-irq
claim:     F$IRQ produces no return value on success (None)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.6, line 5455
--- END ---

--- CARD ---
id:        f-irq-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-irq
claim:     F$IRQ adds a device to or removes a device from the IRQ polling table; to remove pass X=0 and U=address of service routine's static storage
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.6, lines 5463-5464
--- END ---

--- CARD ---
id:        f-irq-error
type:      FACT
target:    6809
verify:    from-manual
topic:     f-irq
claim:     F$IRQ error is E$Poll; can be caused by full polling table or bad (zero) mask byte
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.6, lines 5460-5461
--- END ---

--- CARD ---
id:        f-irq-packet-format-flip-byte
type:      FACT
target:    6809
verify:    from-manual
topic:     f-irq
claim:     F$IRQ control packet at [X] contains flip byte selecting whether bits in device status register are active when set (high) or cleared (low); set bit identifies active-low bit(s)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.6, lines 5449-5476
--- END ---

--- CARD ---
id:        f-irq-packet-format-mask-byte
type:      FACT
target:    6809
verify:    from-manual
topic:     f-irq
claim:     F$IRQ control packet at [X+1] contains mask byte selecting one or more bits within the device status register that serve as interrupt request flag(s); set bit identifies active bit(s)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.6, lines 5477-5479
--- END ---

--- CARD ---
id:        f-irq-packet-format-priority
type:      FACT
target:    6809
verify:    from-manual
topic:     f-irq
claim:     F$IRQ control packet at [X+2] contains device priority number ranging from 0 (lowest) to 255 (highest)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.6, lines 5481-5483
--- END ---

--- CARD ---
id:        f-irq-primary-user
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-irq
claim:     F$IRQ is primarily used by device driver routines
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.6, line 5465
--- END ---

--- CARD ---
id:        f-irq-privileged
type:      FACT
target:    6809
verify:    from-manual
topic:     f-irq
claim:     F$IRQ is a privileged system mode service request
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.6, line 5488
--- END ---

## F$NProc — Start Next Process

--- CARD ---
id:        f-nproc-hex-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-nproc
claim:     F$NProc machine code is 0x2D (in 103F 2D instruction format)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.7, line 5493
--- END ---

--- CARD ---
id:        f-nproc-input-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-nproc
claim:     F$NProc takes no input parameters
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.7, line 5497
--- END ---

--- CARD ---
id:        f-nproc-output-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-nproc
claim:     F$NProc does not return control to the caller; execution transfers to the next process
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.7, line 5499
--- END ---

--- CARD ---
id:        f-nproc-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-nproc
claim:     F$NProc removes the next process from the Active Process Queue and initiates its execution; if queue is empty, waits for an interrupt before rechecking the queue
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.7, lines 5501-5503
--- END ---

--- CARD ---
id:        f-nproc-caveat-queue-membership
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-nproc
claim:     The process calling F$NProc must already be in one of the three process queues; if not, it becomes unknown to the system even though its descriptor remains (will be shown by Procs command but is invisible to scheduler)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.7, lines 5512-5514
--- END ---

--- CARD ---
id:        f-nproc-privileged
type:      FACT
target:    6809
verify:    from-manual
topic:     f-nproc
claim:     F$NProc is a privileged system mode service request
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.7, line 5517
--- END ---

## F$Ret64 — Deallocate 64-byte Memory Block

--- CARD ---
id:        f-ret64-hex-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ret64
claim:     F$Ret64 machine code is 0x31 (in 103F 31 instruction format)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.8, line 5522
--- END ---

--- CARD ---
id:        f-ret64-input-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ret64
claim:     F$Ret64 on entry takes A=block number and X=address of the base page
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.8, lines 5526-5527
--- END ---

--- CARD ---
id:        f-ret64-output-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ret64
claim:     F$Ret64 produces no return value on success (None)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.8, line 5529
--- END ---

--- CARD ---
id:        f-ret64-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ret64
claim:     F$Ret64 deallocates a 64-byte block of memory as described in F$All64; the block is returned to the allocation pool
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.8, lines 5539-5540
--- END ---

--- CARD ---
id:        f-ret64-usage-li
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ret64
claim:     On LI, F$Ret64 is used to free path descriptors or process descriptors
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.8, lines 5542-5543
--- END ---

--- CARD ---
id:        f-ret64-usage-lii
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ret64
claim:     On LII, F$Ret64 is used to free only path descriptors
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.8, line 5544
--- END ---

--- CARD ---
id:        f-ret64-privileged
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ret64
claim:     F$Ret64 is a privileged system mode service request
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.8, line 5551
--- END ---

## F$SRqMem — System Memory Request (Allocate)

--- CARD ---
id:        f-srqmem-hex-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-srqmem
claim:     F$SRqMem machine code is 0x28 (in 103F 28 instruction format)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.9, line 5556
--- END ---

--- CARD ---
id:        f-srqmem-input-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-srqmem
claim:     F$SRqMem on entry takes D register containing byte count to allocate
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.9, line 5558
--- END ---

--- CARD ---
id:        f-srqmem-output-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-srqmem
claim:     F$SRqMem on success returns U=beginning address of allocated memory area and D=new memory size (rounded)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.9, lines 5560-5561
--- END ---

--- CARD ---
id:        f-srqmem-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-srqmem
claim:     F$SRqMem allocates a block of memory from the top of available RAM; requested size is rounded to the next 256-byte page boundary
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.9, lines 5568-5569
--- END ---

--- CARD ---
id:        f-srqmem-error
type:      FACT
target:    6809
verify:    from-manual
topic:     f-srqmem
claim:     F$SRqMem error is E$MemFul (insufficient memory available)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.9, line 5566
--- END ---

--- CARD ---
id:        f-srqmem-lii-system-only
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-srqmem
claim:     On LII, F$SRqMem allocates memory for system address space only
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.9, line 5571
--- END ---

--- CARD ---
id:        f-srqmem-privileged
type:      FACT
target:    6809
verify:    from-manual
topic:     f-srqmem
claim:     F$SRqMem is a privileged system mode service request
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.9, line 5580
--- END ---

## F$SRTMem — Return System Memory (Deallocate)

--- CARD ---
id:        f-srtmem-hex-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-srtmem
claim:     F$SRTMem machine code is 0x29 (in 103F 29 instruction format)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.10, line 5585
--- END ---

--- CARD ---
id:        f-srtmem-input-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-srtmem
claim:     F$SRTMem on entry takes U=beginning address of memory to return and D=number of bytes to return
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.10, lines 5589-5590
--- END ---

--- CARD ---
id:        f-srtmem-output-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-srtmem
claim:     F$SRTMem produces no return value on success (None)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.10, line 5592
--- END ---

--- CARD ---
id:        f-srtmem-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-srtmem
claim:     F$SRTMem deallocates a block of contiguous 256-byte pages; U register must point to an even page boundary
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.10, lines 5604-5605
--- END ---

--- CARD ---
id:        f-srtmem-error
type:      FACT
target:    6809
verify:    from-manual
topic:     f-srtmem
claim:     F$SRTMem error is E$BPAddr (bad page address — not even page boundary)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.10, line 5602
--- END ---

--- CARD ---
id:        f-srtmem-lii-system-only
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-srtmem
claim:     On LII, F$SRTMem deallocates memory for system address space only
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.10, line 5607
--- END ---

--- CARD ---
id:        f-srtmem-privileged
type:      FACT
target:    6809
verify:    from-manual
topic:     f-srtmem
claim:     F$SRTMem is a privileged system mode service request
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.10, line 5616
--- END ---

## F$VModul — Verify Module

--- CARD ---
id:        f-vmodul-hex-code
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-vmodul
claim:     F$VModul machine code is listed as 0x2B (in 103F 2B instruction format) but this appears to be an OCR error; F$IOQu already uses 0x2B; expected value likely 0x2E or 0x32
context:   OCR unclear: Line 5621 lists 103F 2B but this conflicts with F$IOQu (line 5413) also 103F 2B; sequential hex codes suggest missing value in the 0x2E/0x32 range
source:    OS-9 System Programmer's Manual, Chapter 11.2.11, line 5621
--- END ---

--- CARD ---
id:        f-vmodul-input-registers-li
type:      FACT
target:    6809
verify:    from-manual
topic:     f-vmodul
claim:     On LI, F$VModul on entry takes X=address of new module to verify
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.11, lines 5625-5626
--- END ---

--- CARD ---
id:        f-vmodul-input-registers-lii
type:      FACT
target:    6809
verify:    from-manual
topic:     f-vmodul
claim:     On LII, F$VModul on entry takes D=DAT image pointer and X=new module block offset
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.11, lines 5626-5627
--- END ---

--- CARD ---
id:        f-vmodul-output-registers
type:      FACT
target:    6809
verify:    from-manual
topic:     f-vmodul
claim:     F$VModul on success returns U=address of module directory entry
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.11, line 5629
--- END ---

--- CARD ---
id:        f-vmodul-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-vmodul
claim:     F$VModul verifies the module header parity and CRC bytes; if valid, searches module directory for a module with the same name; if duplicate name and type found, retains module with highest revision level; ties are broken in favor of the established (already-resident) module
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.11, lines 5637-5640
--- END ---

--- CARD ---
id:        f-vmodul-errors-li-lii
type:      FACT
target:    6809
verify:    from-manual
topic:     f-vmodul
claim:     F$VModul errors on both LI and LII include E$KwnMod, E$DirFul, E$BMID (bad module ID), E$BMCRC (bad module CRC)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.11, line 5634
--- END ---

--- CARD ---
id:        f-vmodul-errors-lii-only
type:      FACT
target:    6809
verify:    from-manual
topic:     f-vmodul
claim:     F$VModul error on LII only is E$BMHP (bad module header parity)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.11, line 5635
--- END ---

--- CARD ---
id:        f-vmodul-privileged
type:      FACT
target:    6809
verify:    from-manual
topic:     f-vmodul
claim:     F$VModul is a privileged system mode service request
context:   None
source:    OS-9 System Programmer's Manual, Chapter 11.2.11, line 5649
--- END ---
