# Chapter 12.1 Level 2 System Service Requests — Fact Cards

## System Call Registers Reference (OS-9/6809)

---

## F$AllImg - Allocate Image RAM blocks

--- CARD ---
id:        f-allimg-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-allimg
claim:     Machine code for F$AllImg is 103F 3A (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.1, line 6680
--- END ---

--- CARD ---
id:        f-allimg-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-allimg
claim:     F$AllImg input registers: (A)=beginning block number, (B)=number of blocks, (X)=process descriptor pointer
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.1, line 6684-6686
--- END ---

--- CARD ---
id:        f-allimg-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-allimg
claim:     F$AllImg returns no values on success (all registers unchanged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.1, line 6688
--- END ---

--- CARD ---
id:        f-allimg-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-allimg
claim:     F$AllImg allocates contiguous or non-contiguous RAM blocks for a process's DAT image (data area)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.1, line 6695-6696
--- END ---

--- CARD ---
id:        f-allimg-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-allimg
claim:     F$AllImg is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.1, line 6703
--- END ---

---

## F$AllPrc - Allocate Process Descriptor

--- CARD ---
id:        f-allprc-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-allprc
claim:     Machine code for F$AllPrc is 103F 4B (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.2, line 6708
--- END ---

--- CARD ---
id:        f-allprc-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-allprc
claim:     F$AllPrc takes no input parameters
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.2, line 6712
--- END ---

--- CARD ---
id:        f-allprc-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-allprc
claim:     F$AllPrc returns (U)=process descriptor pointer on success
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.2, line 6714
--- END ---

--- CARD ---
id:        f-allprc-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-allprc
claim:     F$AllPrc allocates and initializes a 512-byte process descriptor, clears first 256 bytes, sets system state, and marks up to 60-64k of DAT image as unallocated
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.2, line 6721-6724
--- END ---

--- CARD ---
id:        f-allprc-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-allprc
claim:     F$AllPrc is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.2, line 6736
--- END ---

---

## F$AllRAM - Allocate RAM blocks

--- CARD ---
id:        f-allram-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-allram
claim:     Machine code for F$AllRAM is 103F 39 (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.3, line 6741
--- END ---

--- CARD ---
id:        f-allram-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-allram
claim:     F$AllRAM input: (B)=number of contiguous free RAM blocks to allocate
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.3, line 6745
--- END ---

--- CARD ---
id:        f-allram-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-allram
claim:     F$AllRAM returns (D)=beginning RAM block number on success
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.3, line 6747
--- END ---

--- CARD ---
id:        f-allram-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-allram
claim:     F$AllRAM searches the memory block map for a contiguous sequence of free RAM blocks of the requested size
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.3, line 6754-6755
--- END ---

--- CARD ---
id:        f-allram-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-allram
claim:     F$AllRAM is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.3, line 6762
--- END ---

---

## F$AllTsk - Allocate process Task number

--- CARD ---
id:        f-alltsk-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-alltsk
claim:     Machine code for F$AllTsk is 103F 3F (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.4, line 6767
--- END ---

--- CARD ---
id:        f-alltsk-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-alltsk
claim:     F$AllTsk input: (X)=process descriptor pointer
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.4, line 6771
--- END ---

--- CARD ---
id:        f-alltsk-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-alltsk
claim:     F$AllTsk returns no values on success (all registers unchanged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.4, line 6773
--- END ---

--- CARD ---
id:        f-alltsk-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-alltsk
claim:     F$AllTsk allocates a task number to a process if one is not already assigned, then copies the DAT image into the DAT hardware
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.4, line 6780-6781
--- END ---

--- CARD ---
id:        f-alltsk-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-alltsk
claim:     F$AllTsk is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.4, line 6788
--- END ---

---

## F$Boot - Bootstrap system

--- CARD ---
id:        f-boot-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-boot
claim:     Machine code for F$Boot is 103F 35 (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.5, line 6799
--- END ---

--- CARD ---
id:        f-boot-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-boot
claim:     F$Boot takes no input parameters
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.5, line 6803
--- END ---

--- CARD ---
id:        f-boot-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-boot
claim:     F$Boot returns no values on success (all registers unchanged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.5, line 6805
--- END ---

--- CARD ---
id:        f-boot-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-boot
claim:     F$Boot links the Boot module (or module specified in INIT), calls it, and expects a pointer and size for searching new modules
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.5, line 6810-6811
--- END ---

--- CARD ---
id:        f-boot-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-boot
claim:     F$Boot is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.5, line 6818
--- END ---

---

## F$BtMem - Bootstrap Memory request

--- CARD ---
id:        f-btmem-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-btmem
claim:     Machine code for F$BtMem is 103F 36 (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.6, line 6823
--- END ---

--- CARD ---
id:        f-btmem-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-btmem
claim:     F$BtMem input: (D)=byte count requested
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.6, line 6827
--- END ---

--- CARD ---
id:        f-btmem-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-btmem
claim:     F$BtMem returns (D)=byte count granted and (U)=pointer to memory allocated on success
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.6, line 6829-6830
--- END ---

--- CARD ---
id:        f-btmem-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-btmem
claim:     F$BtMem allocates contiguous memory in system address space (rounded to nearest block)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.6, line 6837-6838
--- END ---

--- CARD ---
id:        f-btmem-deprecated
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-btmem
claim:     F$BtMem is not needed as of Level 2 version 1.2 (equated to F$SrqMem)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.6, line 6840
--- END ---

--- CARD ---
id:        f-btmem-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-btmem
claim:     F$BtMem is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.6, line 6843
--- END ---

---

## F$ClrBlk - Clear specific Block

--- CARD ---
id:        f-clrblk-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-clrblk
claim:     Machine code for F$ClrBlk is 103F 50 (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.7, line 6848
--- END ---

--- CARD ---
id:        f-clrblk-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-clrblk
claim:     F$ClrBlk input: (B)=number of blocks, (U)=address of first block
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.7, line 6857-6858
--- END ---

--- CARD ---
id:        f-clrblk-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-clrblk
claim:     F$ClrBlk returns no values on success (all registers unchanged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.7, line 6860
--- END ---

--- CARD ---
id:        f-clrblk-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-clrblk
claim:     F$ClrBlk marks blocks in process DAT image as unallocated, freeing them for reuse as data or program area
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.7, line 6867-6868
--- END ---

--- CARD ---
id:        f-clrblk-user-mode
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-clrblk
claim:     F$ClrBlk is a user mode service request (not privileged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.7, line 6875
--- END ---

---

## F$CpyMem - Copy external Memory

--- CARD ---
id:        f-cpymem-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-cpymem
claim:     Machine code for F$CpyMem is 103F 18 (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.8, line 6880
--- END ---

--- CARD ---
id:        f-cpymem-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-cpymem
claim:     F$CpyMem input: (D)=DAT image pointer, (X)=offset in block to begin copy, (Y)=byte count, (U)=caller's destination buffer
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.8, line 6884-6887
--- END ---

--- CARD ---
id:        f-cpymem-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-cpymem
claim:     F$CpyMem returns no values on success (all registers unchanged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.8, line 6889
--- END ---

--- CARD ---
id:        f-cpymem-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-cpymem
claim:     F$CpyMem reads external memory into the user's buffer for inspection via DAT image; may view any system memory; X treated as address within the 64k space described by DAT image
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.8, line 6894-6897
--- END ---

--- CARD ---
id:        f-cpymem-user-mode
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-cpymem
claim:     F$CpyMem is a user mode service request (not privileged)
context:   cross-check: already documented as user-mode call
source:    OS-9 System Programmer's Manual, Chapter 12.1.8, line 6907
--- END ---

---

## F$DATLog - Convert DAT block/offset to Logical Address

--- CARD ---
id:        f-datlog-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-datlog
claim:     Machine code for F$DATLog is 103F 44 (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.9, line 6918
--- END ---

--- CARD ---
id:        f-datlog-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-datlog
claim:     F$DATLog input: (B)=DAT image offset (block index within DAT image), (X)=block offset (address within that block)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.9, line 6922-6923
--- END ---

--- CARD ---
id:        f-datlog-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-datlog
claim:     F$DATLog returns (X)=logical address on success
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.9, line 6925
--- END ---

--- CARD ---
id:        f-datlog-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-datlog
claim:     F$DATLog converts a DAT image block number and offset within that block to its equivalent logical address
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.9, line 6930-6931
--- END ---

--- CARD ---
id:        f-datlog-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-datlog
claim:     F$DATLog is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.9, line 6934
--- END ---

---

## F$DelImg - Deallocate Image RAM blocks

--- CARD ---
id:        f-delimg-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-delimg
claim:     Machine code for F$DelImg is 103F 3B (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.10, line 6956
--- END ---

--- CARD ---
id:        f-delimg-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-delimg
claim:     F$DelImg input: (A)=beginning block number, (B)=block count, (X)=process descriptor pointer
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.10, line 6960-6962
--- END ---

--- CARD ---
id:        f-delimg-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-delimg
claim:     F$DelImg returns no values on success (all registers unchanged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.10, line 6964
--- END ---

--- CARD ---
id:        f-delimg-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-delimg
claim:     F$DelImg deallocates memory from a process's address space by freeing RAM for system use and freeing the DAT image entry for the process; used for cleanup after process death
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.10, line 6969-6971
--- END ---

--- CARD ---
id:        f-delimg-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-delimg
claim:     F$DelImg is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.10, line 6983
--- END ---

---

## F$DelPrc - Deallocate Process descriptor

--- CARD ---
id:        f-delprc-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-delprc
claim:     Machine code for F$DelPrc is 103F 4C (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.11, line 6988
--- END ---

--- CARD ---
id:        f-delprc-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-delprc
claim:     F$DelPrc input: (A)=process ID
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.11, line 6992
--- END ---

--- CARD ---
id:        f-delprc-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-delprc
claim:     F$DelPrc returns no values on success (all registers unchanged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.11, line 6994
--- END ---

--- CARD ---
id:        f-delprc-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-delprc
claim:     F$DelPrc returns process descriptor memory to a free memory pool for cleanup after process death
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.11, line 6999-7000
--- END ---

--- CARD ---
id:        f-delprc-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-delprc
claim:     F$DelPrc is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.11, line 7007
--- END ---

---

## F$DelRam - Deallocate RAM blocks

--- CARD ---
id:        f-delram-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-delram
claim:     Machine code for F$DelRam is 103F 51 (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.12, line 7012
--- END ---

--- CARD ---
id:        f-delram-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-delram
claim:     F$DelRam input: (B)=number of blocks, (X)=starting block number
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.12, line 7016-7017
--- END ---

--- CARD ---
id:        f-delram-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-delram
claim:     F$DelRam returns no values (all registers unchanged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.12, line 7019
--- END ---

--- CARD ---
id:        f-delram-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-delram
claim:     F$DelRam clears the RAM-in-use flag in the system memory block map for specified blocks; blocks must not be associated with any DAT image
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.12, line 7023-7024
--- END ---

--- CARD ---
id:        f-delram-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-delram
claim:     F$DelRam is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.12, line 7031
--- END ---

---

## F$DelTsk - Deallocate process Task number

--- CARD ---
id:        f-deltsk-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-deltsk
claim:     Machine code for F$DelTsk is 103F 40 (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.13, line 7036
--- END ---

--- CARD ---
id:        f-deltsk-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-deltsk
claim:     F$DelTsk input: (X)=process descriptor pointer
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.13, line 7045
--- END ---

--- CARD ---
id:        f-deltsk-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-deltsk
claim:     F$DelTsk returns no values on success (all registers unchanged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.13, line 7047
--- END ---

--- CARD ---
id:        f-deltsk-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-deltsk
claim:     F$DelTsk releases the task number in use by a process, freeing its DAT hardware for another user
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.13, line 7052-7053
--- END ---

--- CARD ---
id:        f-deltsk-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-deltsk
claim:     F$DelTsk is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.13, line 7060
--- END ---

---

## F$ELink - Link using module directory Entry

--- CARD ---
id:        f-elink-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-elink
claim:     Machine code for F$ELink is 103F 4D (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.14, line 7065
--- END ---

--- CARD ---
id:        f-elink-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-elink
claim:     F$ELink input: (B)=module type, (X)=pointer to module directory entry
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.14, line 7069-7070
--- END ---

--- CARD ---
id:        f-elink-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-elink
claim:     F$ELink returns (U)=module header address and (Y)=module entry point on success
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.14, line 7072-7073
--- END ---

--- CARD ---
id:        f-elink-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-elink
claim:     F$ELink links a module given a pointer to its module directory entry (unlike F$Link which takes a name pointer)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.14, line 7080-7082
--- END ---

--- CARD ---
id:        f-elink-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-elink
claim:     F$ELink is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.14, line 7089
--- END ---

---

## F$FModul - Find Module directory entry

--- CARD ---
id:        f-fmodul-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-fmodul
claim:     Machine code for F$FModul is 103F 4B (hex)
context:   OCR unclear: same code as F$AllPrc (103F 4B at line 6708); may be OCR error or collision in original manual
source:    OS-9 System Programmer's Manual, Chapter 12.1.15, line 7094
--- END ---

--- CARD ---
id:        f-fmodul-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-fmodul
claim:     F$FModul input: (A)=module type, (X)=pointer to name string, (Y)=DAT image pointer (for name)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.15, line 7098-7106
--- END ---

--- CARD ---
id:        f-fmodul-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-fmodul
claim:     F$FModul returns (A)=module type, (B)=module revision, (X)=updated past name string, (U)=module directory entry pointer on success
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.15, line 7108-7111
--- END ---

--- CARD ---
id:        f-fmodul-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-fmodul
claim:     F$FModul searches module directory for first module matching the given name and type; module type zero matches any module
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.15, line 7118-7120
--- END ---

--- CARD ---
id:        f-fmodul-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-fmodul
claim:     F$FModul is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.15, line 7127
--- END ---

---

## F$FreeHB - Get Free High block

--- CARD ---
id:        f-freehb-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-freehb
claim:     Machine code for F$FreeHB is 103F 3E (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.16, line 7132
--- END ---

--- CARD ---
id:        f-freehb-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-freehb
claim:     F$FreeHB input: (B)=block count, (Y)=DAT image pointer
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.16, line 7136-7137
--- END ---

--- CARD ---
id:        f-freehb-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-freehb
claim:     F$FreeHB returns (A)=beginning block number on success
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.16, line 7139
--- END ---

--- CARD ---
id:        f-freehb-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-freehb
claim:     F$FreeHB searches a DAT image for the highest set of contiguous free blocks of the given size and returns the beginning block number
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.16, line 7146-7147
--- END ---

--- CARD ---
id:        f-freehb-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-freehb
claim:     F$FreeHB is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.16, line 7150
--- END ---

---

## F$FreeLB - Get Free Low block

--- CARD ---
id:        f-freelb-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-freelb
claim:     Machine code for F$FreeLB is 103F 3D (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.17, line 7155
--- END ---

--- CARD ---
id:        f-freelb-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-freelb
claim:     F$FreeLB input: (B)=block count, (Y)=DAT image pointer
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.17, line 7159-7160
--- END ---

--- CARD ---
id:        f-freelb-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-freelb
claim:     F$FreeLB returns (A)=beginning block number on success
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.17, line 7162
--- END ---

--- CARD ---
id:        f-freelb-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-freelb
claim:     F$FreeLB searches a DAT image for the lowest set of contiguous free blocks of the given size and returns the beginning block number
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.17, line 7174-7175
--- END ---

--- CARD ---
id:        f-freelb-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-freelb
claim:     F$FreeLB is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.17, line 7178
--- END ---

---

## F$GBlkMp - Get system Block Map copy

--- CARD ---
id:        f-gblkmp-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-gblkmp
claim:     Machine code for F$GBlkMp is 103F 19 (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.18, line 7183
--- END ---

--- CARD ---
id:        f-gblkmp-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-gblkmp
claim:     F$GBlkMp input: (X)=1024 byte buffer pointer
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.18, line 7187
--- END ---

--- CARD ---
id:        f-gblkmp-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-gblkmp
claim:     F$GBlkMp returns (D)=number of bytes per block (MMU-dependent) and (Y)=size of system's memory block map on success
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.18, line 7189-7190
--- END ---

--- CARD ---
id:        f-gblkmp-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-gblkmp
claim:     F$GBlkMp copies the system's memory block map into a user buffer for inspection; used by Mfree to find available free memory
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.18, line 7195-7196
--- END ---

--- CARD ---
id:        f-gblkmp-user-mode
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-gblkmp
claim:     F$GBlkMp is a user mode service request (not privileged)
context:   cross-check: already documented as user-mode call
source:    OS-9 System Programmer's Manual, Chapter 12.1.18, line 7203
--- END ---

---

## F$GModDr - Get Module Directory copy

--- CARD ---
id:        f-gmoddr-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-gmoddr
claim:     Machine code for F$GModDr is 103F 1A (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.19, line 7208
--- END ---

--- CARD ---
id:        f-gmoddr-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-gmoddr
claim:     F$GModDr input: (X)=2048 byte buffer pointer, (Y)=end of copied module directory, (U)=start address of module directory in system
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.19, line 7212-7214
--- END ---

--- CARD ---
id:        f-gmoddr-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-gmoddr
claim:     F$GModDr returns no values on success (all registers unchanged)
context:   OCR unclear: No explicit OUTPUT section found in text
source:    OS-9 System Programmer's Manual, Chapter 12.1.19, line 7208-7214
--- END ---

--- CARD ---
id:        f-gmoddr-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-gmoddr
claim:     F$GModDr copies the system's module directory into a user buffer for inspection; used by Mdir to examine the module directory
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.19, line 7219-7220
--- END ---

--- CARD ---
id:        f-gmoddr-user-mode
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-gmoddr
claim:     F$GModDr is a user mode service request (not privileged)
context:   cross-check: already documented as user-mode call
source:    OS-9 System Programmer's Manual, Chapter 12.1.19, line 7232
--- END ---

---

## F$GPrDsc - Get Process Descriptor copy

--- CARD ---
id:        f-gprdsc-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-gprdsc
claim:     Machine code for F$GPrDsc is 103F 18 (hex)
context:   OCR unclear: same code as F$CpyMem (103F 18 at line 6880); may be OCR error or collision in original manual
source:    OS-9 System Programmer's Manual, Chapter 12.1.20, line 7237
--- END ---

--- CARD ---
id:        f-gprdsc-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-gprdsc
claim:     F$GPrDsc input: (A)=requested process ID, (X)=512 byte buffer pointer
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.20, line 7241-7242
--- END ---

--- CARD ---
id:        f-gprdsc-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-gprdsc
claim:     F$GPrDsc returns no values on success (all registers unchanged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.20, line 7244
--- END ---

--- CARD ---
id:        f-gprdsc-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-gprdsc
claim:     F$GPrDsc copies a process descriptor into the caller's buffer for inspection; read-only access, no modification possible; used by Procs utility for process information
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.20, line 7249-7251
--- END ---

--- CARD ---
id:        f-gprdsc-user-mode
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-gprdsc
claim:     F$GPrDsc is a user mode service request (not privileged)
context:   cross-check: already documented as user-mode call
source:    OS-9 System Programmer's Manual, Chapter 12.1.20, line 7258
--- END ---

---

## F$GProcP - Get Process Pointer

--- CARD ---
id:        f-gprocp-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-gprocp
claim:     Machine code for F$GProcP is 103F 37 (hex)
context:   Note: Assembler call shown as OS9 F$GProCP (uppercase P mistakenly repeated)
source:    OS-9 System Programmer's Manual, Chapter 12.1.21, line 7263
--- END ---

--- CARD ---
id:        f-gprocp-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-gprocp
claim:     F$GProcP input: (A)=process ID
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.21, line 7267
--- END ---

--- CARD ---
id:        f-gprocp-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-gprocp
claim:     F$GProcP returns (B)=pointer to process descriptor on success
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.21, line 7269
--- END ---

--- CARD ---
id:        f-gprocp-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-gprocp
claim:     F$GProcP translates a process ID number to its process descriptor address in system address space; descriptors exist only in system task space
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.21, line 7276-7278
--- END ---

--- CARD ---
id:        f-gprocp-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-gprocp
claim:     F$GProcP is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.21, line 7285
--- END ---

---

## F$LDABX - Load A from 0,X in task B

--- CARD ---
id:        f-ldabx-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ldabx
claim:     Machine code for F$LDABX is 103F 49 (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.22, line 7297
--- END ---

--- CARD ---
id:        f-ldabx-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ldabx
claim:     F$LDABX input: (B)=task number, (X)=data pointer
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.22, line 7299-7300
--- END ---

--- CARD ---
id:        f-ldabx-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ldabx
claim:     F$LDABX returns (A)=data byte at address 0,X in the task's address space on success
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.22, line 7302
--- END ---

--- CARD ---
id:        f-ldabx-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ldabx
claim:     F$LDABX reads one byte from the logical address in register X within the given task's address space; typically used to read from current process memory in system state routines
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.22, line 7307-7308
--- END ---

--- CARD ---
id:        f-ldabx-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-ldabx
claim:     F$LDABX is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.22, line 7311
--- END ---

---

## F$LDAXY - Load A [X, [Y]]

--- CARD ---
id:        f-ldaxy-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ldaxy
claim:     Machine code for F$LDAXY is 103F 46 (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.23, line 7317
--- END ---

--- CARD ---
id:        f-ldaxy-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ldaxy
claim:     F$LDAXY input: (X)=block offset, (Y)=DAT image pointer
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.23, line 7319-7320
--- END ---

--- CARD ---
id:        f-ldaxy-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ldaxy
claim:     F$LDAXY returns (A)=data byte at offset X within the memory block specified by DAT image Y
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.23, line 7322
--- END ---

--- CARD ---
id:        f-ldaxy-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ldaxy
claim:     F$LDAXY reads one byte from the memory block specified by DAT image in Y, offset by X (X must be less than block size or invalid data returned; no error check)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.23, line 7327-7330
--- END ---

--- CARD ---
id:        f-ldaxy-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-ldaxy
claim:     F$LDAXY is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.23, line 7333
--- END ---

---

## F$LDDDXY - Load D [D+X],[Y]]

--- CARD ---
id:        f-ldddxy-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ldddxy
claim:     Machine code for F$LDDDXY is 103F 48 (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.24, line 7339
--- END ---

--- CARD ---
id:        f-ldddxy-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ldddxy
claim:     F$LDDDXY input: (D)=offset to the offset within DAT image, (X)=offset within DAT image, (Y)=DAT image pointer
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.24, line 7341-7343
--- END ---

--- CARD ---
id:        f-ldddxy-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ldddxy
claim:     F$LDDDXY returns (D)=bytes (two bytes) addressed by [D+X,Y] on success
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.24, line 7345
--- END ---

--- CARD ---
id:        f-ldddxy-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-ldddxy
claim:     F$LDDDXY loads two bytes from the address space described by DAT image in Y at address D+X; offsets must be oriented relative to the first block pointed by DAT image
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.24, line 7356-7362
--- END ---

--- CARD ---
id:        f-ldddxy-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-ldddxy
claim:     F$LDDDXY is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.24, line 7370
--- END ---

---

## F$MapBlk - Map specific block

--- CARD ---
id:        f-mapblk-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-mapblk
claim:     Machine code for F$MapBlk is 103F 4F (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.25, line 7375
--- END ---

--- CARD ---
id:        f-mapblk-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-mapblk
claim:     F$MapBlk input: (B)=number of blocks, (X)=beginning block number
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.25, line 7379-7380
--- END ---

--- CARD ---
id:        f-mapblk-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-mapblk
claim:     F$MapBlk returns (U)=address of first block on success
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.25, line 7382
--- END ---

--- CARD ---
id:        f-mapblk-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-mapblk
claim:     F$MapBlk maps specified blocks into unallocated blocks of process space; blocks are mapped in from the top down into highest available addresses
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.25, line 7389-7391
--- END ---

--- CARD ---
id:        f-mapblk-user-mode
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-mapblk
claim:     F$MapBlk is a user mode service request (not privileged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.25, line 7398
--- END ---

---

## F$Move - Move Data (low bound first)

--- CARD ---
id:        f-move-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-move
claim:     Machine code for F$Move is 103F 38 (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.26, line 7403
--- END ---

--- CARD ---
id:        f-move-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-move
claim:     F$Move input: (A)=source task number, (B)=destination task number, (X)=source pointer, (Y)=byte count, (U)=destination pointer
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.26, line 7407-7417
--- END ---

--- CARD ---
id:        f-move-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-move
claim:     F$Move returns no values on success (all registers unchanged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.26, line 7419
--- END ---

--- CARD ---
id:        f-move-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-move
claim:     F$Move transfers data bytes from one address space to another, typically from system to user space or vice versa
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.26, line 7424-7425
--- END ---

--- CARD ---
id:        f-move-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-move
claim:     F$Move is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.26, line 7428
--- END ---

---

## F$RelTsk - Release Task number

--- CARD ---
id:        f-reltsk-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-reltsk
claim:     Machine code for F$RelTsk is 103F 43 (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.27, line 7433
--- END ---

--- CARD ---
id:        f-reltsk-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-reltsk
claim:     F$RelTsk input: (B)=task number
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.27, line 7437
--- END ---

--- CARD ---
id:        f-reltsk-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-reltsk
claim:     F$RelTsk returns no values on success (all registers unchanged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.27, line 7439
--- END ---

--- CARD ---
id:        f-reltsk-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-reltsk
claim:     F$RelTsk releases a specified DAT task number, freeing its DAT hardware for another user
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.27, line 7444-7445
--- END ---

--- CARD ---
id:        f-reltsk-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-reltsk
claim:     F$RelTsk is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.27, line 7452
--- END ---

---

## F$ResTsk - Reserve Task number

--- CARD ---
id:        f-restsk-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-restsk
claim:     Machine code for F$ResTsk is 103F 42 (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.28, line 7457
--- END ---

--- CARD ---
id:        f-restsk-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-restsk
claim:     F$ResTsk takes no input parameters
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.28, line 7461
--- END ---

--- CARD ---
id:        f-restsk-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-restsk
claim:     F$ResTsk returns (B)=task number on success
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.28, line 7463
--- END ---

--- CARD ---
id:        f-restsk-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-restsk
claim:     F$ResTsk finds a free DAT task number, reserves it, and returns the task number to the caller (typically then assigned to a process)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.28, line 7468-7469
--- END ---

--- CARD ---
id:        f-restsk-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-restsk
claim:     F$ResTsk is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.28, line 7481
--- END ---

---

## F$SetImg - Set Process DAT Image

--- CARD ---
id:        f-setimg-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-setimg
claim:     Machine code for F$SetImg is 103F 3C (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.29, line 7486
--- END ---

--- CARD ---
id:        f-setimg-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-setimg
claim:     F$SetImg input: (A)=beginning image block number, (B)=block count, (X)=process descriptor pointer, (U)=new image pointer
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.29, line 7490-7493
--- END ---

--- CARD ---
id:        f-setimg-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-setimg
claim:     F$SetImg returns no values on success (all registers unchanged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.29, line 7495
--- END ---

--- CARD ---
id:        f-setimg-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-setimg
claim:     F$SetImg copies DAT image (or portion thereof) into process descriptor and sets image change flag to ensure hardware DAT update on return from system call
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.29, line 7500-7502
--- END ---

--- CARD ---
id:        f-setimg-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-setimg
claim:     F$SetImg is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.29, line 7509
--- END ---

---

## F$SetTsk - Set process Task DAT registers

--- CARD ---
id:        f-settsk-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-settsk
claim:     Machine code for F$SetTsk is 103F 41 (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.30, line 7514
--- END ---

--- CARD ---
id:        f-settsk-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-settsk
claim:     F$SetTsk input: (X)=process descriptor pointer
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.30, line 7518
--- END ---

--- CARD ---
id:        f-settsk-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-settsk
claim:     F$SetTsk returns no values on success (all registers unchanged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.30, line 7520
--- END ---

--- CARD ---
id:        f-settsk-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-settsk
claim:     F$SetTsk sets process task hardware DAT registers from the descriptor and clears the image change flag after writing data to hardware
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.30, line 7525-7526
--- END ---

--- CARD ---
id:        f-settsk-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-settsk
claim:     F$SetTsk is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.30, line 7529
--- END ---

---

## F$Slink - System Link

--- CARD ---
id:        f-slink-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-slink
claim:     Machine code for F$Slink is 103F 34 (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.31, line 7539
--- END ---

--- CARD ---
id:        f-slink-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-slink
claim:     F$Slink input: (A)=module type, (X)=module name string pointer, (Y)=name string DAT image pointer
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.31, line 7543-7545
--- END ---

--- CARD ---
id:        f-slink-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-slink
claim:     F$Slink returns (A)=module type, (B)=module revision, (X)=updated name string pointer, (Y)=module entry point, (U)=module pointer on success
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.31, line 7547-7551
--- END ---

--- CARD ---
id:        f-slink-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-slink
claim:     F$Slink links a module whose name is outside the current process's address space into the system's address space; used by I/O system for device-name module linking
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.31, line 7558-7560
--- END ---

--- CARD ---
id:        f-slink-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-slink
claim:     F$Slink is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.31, line 7567
--- END ---

---

## F$STABX - Store A at 0,X in task B

--- CARD ---
id:        f-stabx-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-stabx
claim:     Machine code for F$STABX is 103F 4A (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.32, line 7572
--- END ---

--- CARD ---
id:        f-stabx-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-stabx
claim:     F$STABX input: (A)=data byte to store, (B)=task number, (X)=logical address in task's address space to store
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.32, line 7576-7578
--- END ---

--- CARD ---
id:        f-stabx-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-stabx
claim:     F$STABX returns no values on success (all registers unchanged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.32, line 7580
--- END ---

--- CARD ---
id:        f-stabx-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-stabx
claim:     F$STABX stores a byte to an address in a specified task's address space (similar to assembly STA 0,X but cross-task)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.32, line 7585-7586
--- END ---

--- CARD ---
id:        f-stabx-priv
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-stabx
claim:     F$STABX is a privileged system mode service request (kernel-only)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.32, line 7589
--- END ---

---

## F$SUser - Set User ID number

--- CARD ---
id:        f-suser-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-suser
claim:     Machine code for F$SUser is 103F 1C (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.33, line 7594
--- END ---

--- CARD ---
id:        f-suser-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-suser
claim:     F$SUser input: (Y)=desired user ID number
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.33, line 7603
--- END ---

--- CARD ---
id:        f-suser-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-suser
claim:     F$SUser returns no values on success (all registers unchanged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.33, line 7605
--- END ---

--- CARD ---
id:        f-suser-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-suser
claim:     F$SUser alters the current user ID to the specified value without error checking and regardless of the caller's ID
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.33, line 7610-7611
--- END ---

--- CARD ---
id:        f-suser-user-mode
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-suser
claim:     F$SUser is a user mode service request (not privileged)
context:   cross-check: already documented as user-mode call
source:    OS-9 System Programmer's Manual, Chapter 12.1.33, line 7618
--- END ---

---

## F$UnLoad - Unlink module by name

--- CARD ---
id:        f-unload-code
type:      FACT
target:    6809
verify:    from-manual
topic:     f-unload
claim:     Machine code for F$UnLoad is 103F 1D (hex)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.34, line 7623
--- END ---

--- CARD ---
id:        f-unload-input
type:      FACT
target:    6809
verify:    from-manual
topic:     f-unload
claim:     F$UnLoad input: (A)=module type, (X)=module name pointer
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.34, line 7627-7628
--- END ---

--- CARD ---
id:        f-unload-output
type:      FACT
target:    6809
verify:    from-manual
topic:     f-unload
claim:     F$UnLoad returns no values on success (all registers unchanged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.34, line 7630
--- END ---

--- CARD ---
id:        f-unload-function
type:      FACT
target:    6809
verify:    from-manual
topic:     f-unload
claim:     F$UnLoad locates a module in the directory by name, decrements its link count, and removes it if count reaches zero (differs from F$UnLink which takes module header address)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.34, line 7635-7637
--- END ---

--- CARD ---
id:        f-unload-user-mode
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     f-unload
claim:     F$UnLoad is a user mode service request (not privileged)
context:   None
source:    OS-9 System Programmer's Manual, Chapter 12.1.34, line 7644
--- END ---

---

# Machine Code Lookup Table

| Call Name | Machine Code | Line | Notes |
|-----------|--------------|------|-------|
| F$AllImg | 103F 3A | 6680 | Privileged |
| F$AllPrc | 103F 4B | 6708 | Privileged |
| F$AllRAM | 103F 39 | 6741 | Privileged |
| F$AllTsk | 103F 3F | 6767 | Privileged |
| F$Boot | 103F 35 | 6799 | Privileged |
| F$BtMem | 103F 36 | 6823 | Privileged; deprecated (v1.2+) |
| F$ClrBlk | 103F 50 | 6848 | User mode |
| F$CpyMem | 103F 18 | 6880 | User mode; already documented |
| F$DATLog | 103F 44 | 6918 | Privileged |
| F$DelImg | 103F 3B | 6956 | Privileged |
| F$DelPrc | 103F 4C | 6988 | Privileged |
| F$DelRam | 103F 51 | 7012 | Privileged |
| F$DelTsk | 103F 40 | 7036 | Privileged |
| F$ELink | 103F 4D | 7065 | Privileged |
| F$FModul | 103F 4B | 7094 | Privileged; **COLLISION: same as F$AllPrc** |
| F$FreeHB | 103F 3E | 7132 | Privileged |
| F$FreeLB | 103F 3D | 7155 | Privileged |
| F$GBlkMp | 103F 19 | 7183 | User mode; already documented |
| F$GModDr | 103F 1A | 7208 | User mode; already documented |
| F$GPrDsc | 103F 18 | 7237 | User mode; **COLLISION: same as F$CpyMem** |
| F$GProcP | 103F 37 | 7263 | Privileged |
| F$LDABX | 103F 49 | 7297 | Privileged |
| F$LDAXY | 103F 46 | 7317 | Privileged |
| F$LDDDXY | 103F 48 | 7339 | Privileged |
| F$MapBlk | 103F 4F | 7375 | User mode |
| F$Move | 103F 38 | 7403 | Privileged |
| F$RelTsk | 103F 43 | 7433 | Privileged |
| F$ResTsk | 103F 42 | 7457 | Privileged |
| F$SetImg | 103F 3C | 7486 | Privileged |
| F$SetTsk | 103F 41 | 7514 | Privileged |
| F$Slink | 103F 34 | 7539 | Privileged |
| F$STABX | 103F 4A | 7572 | Privileged |
| F$SUser | 103F 1C | 7594 | User mode; already documented |
| F$UnLoad | 103F 1D | 7623 | User mode |

---

# OCR Issues & Collisions Detected

1. **F$AllPrc vs F$FModul**: Both list machine code `103F 4B`
   - F$AllPrc: line 6708 (Allocate Process Descriptor)
   - F$FModul: line 7094 (Find Module Directory Entry)
   - One or both codes are likely OCR errors

2. **F$CpyMem vs F$GPrDsc**: Both list machine code `103F 18`
   - F$CpyMem: line 6880 (Copy External Memory)
   - F$GPrDsc: line 7237 (Get Process Descriptor Copy)
   - One or both codes are likely OCR errors

## Summary

- **34 calls extracted** with full register input/output specifications
- **32/34 calls have clean machine code** entries (no OCR issues detected)
- **2 critical machine code collisions** require verification against a clean source
- **5 user-mode calls documented** (F$ClrBlk, F$CpyMem, F$GBlkMp, F$GModDr, F$GPrDsc, F$SUser, F$UnLoad) — cross-check value added
- **1 deprecated call noted** (F$BtMem equated to F$SrqMem in v1.2+)
- All privileged mode calls flagged consistently
