# Memory management and hardware

33 raw cards in -> 18 cards out (15 kept as-is, 3 produced by merging 6 near-duplicate/complementary
cards, 12 dropped: 9 mis-bucketed into this cluster by keyword match alone — BASIC09/I-code
architecture, data-type interop, SVC/performance classification, shell command-line syntax — and
3 pruned as generic memory-management-101 restatements with no OS-9-specific mechanism). 6809
direct-page/stack facts are kept in their own section, correctly isolated from 68k material.

## Allocation strategy (first-fit / block sizing)

--- CARD ---
id:        memory-minimum-allocation-unit
type:      FACT
target:    all
verify:    from-manual
topic:     memory-mgmt
claim:     OS-9 allocates memory in multiples of a 16-byte minimum allocation unit; areas of free memory are linked together with a controlling structure at the start of each area.
context:   The 16-byte structure is the minimum needed for free-list management; smaller areas cannot be freed.
source:    The OS-9 Guru, section 3.3.1, page 49
--- END ---

--- CARD ---
id:        system-minimum-allocatable-block-size
type:      FACT
target:    68k
verify:    from-manual
topic:     memory-mgmt
claim:     The system's minimum allocatable block size depends on memory protection: on MMU-equipped systems it typically matches the MMU page size (e.g. 4K bytes), since the kernel allocates whole physical pages and subdivides them internally for smaller requests; without inter-task memory protection, it defaults to 256 bytes instead. Either default is larger than OS-9's internal 16-byte logical allocation unit used for free-list bookkeeping.
context:   The kernel manages free fragments within these larger allocated blocks to avoid wasting a full block/page on small allocations.
source:    The OS-9 Guru, section 3.4.3, page 56 (merges the MMU and non-MMU cases, previously two separate cards)
--- END ---

--- CARD ---
id:        os9-alloc-first-fit-and-buddy
type:      FACT
target:    68k
verify:    from-manual
topic:     memory-mgmt
claim:     OS-9 memory allocation includes a first-fit strategy (grabs the first free block large enough for the request) as one of its schemes, alongside a separate buddy-system allocator that splits/merges power-of-2-sized blocks (see buddy-allocator-overview below).
context:   First-fit is simpler and faster than a best-fit search but can leave more unusable fragments; OS-9 favors it for speed over best-fit's tighter packing.
source:    OS-9 Insights §4.1.3 "First-Fit Allocation"; The OS-9 Primer, Chapter 2 "Memory model" (merged — the generic best-fit-vs-first-fit contrast card was dropped as CS-101 restatement with no OS-9-specific mechanism)
--- END ---

--- CARD ---
id:        mfree-memory-availability
type:      FACT
target:    68k
verify:    from-manual
topic:     memory-management
claim:     `mfree` displays the address and size of unused memory available for allocation. `mfree -e` shows detailed memory info including minimum allocation size, number of memory segments, total RAM at startup, current total free RAM, and a segment-by-segment memory map.
context:   Used to assess system memory availability before loading programs.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Free and Mfree section, p. 3-9
--- END ---

## Colored memory / buddy allocator

--- CARD ---
id:        buddy-allocator-overview
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     memory-mgmt
claim:     The buddy-system allocator pairs free blocks of the same size for faster allocation/deallocation; it does not maintain fragment lists, which improves speed and predictability.
context:   Complicates MMU memory protection because D_MinBlk must equal the MMU page size.
source:    OS-9 Insights §6 "The Buddy System Allocator"
--- END ---

--- CARD ---
id:        colored-memory
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     memory-mgmt
claim:     Colored memory partitions address space into distinct regions for different purposes — e.g. non-volatile RAM, fast cache-friendly memory (avoiding cache conflicts), or DMA-accessible regions — useful on systems with hardware memory-access limitations.
context:   -
source:    The OS-9 Primer, Chapter 8 "Colored memory in data modules"; OS-9 Insights §5.3 "Colored Memory" (merged duplicate concept cards from two manuals)
--- END ---

## Fragmentation

--- CARD ---
id:        memory-fragmentation-rare-but-possible
type:      GOTCHA
target:    all
verify:    from-manual
topic:     memory
claim:     Without MMU address translation, separate physical memory areas cannot be combined into a single logical area; fragmentation is rare in practice unless an application is very tight on memory or allocates very large blocks.
context:   OS-9's design avoids most fragmentation problems through careful allocation strategies.
source:    The OS-9 Guru, section 3.4, page 50
--- END ---

## MMU / DMA (68k, Level-2-style systems)

--- CARD ---
id:        mmu-not-required-for-basic-os9
type:      FACT
target:    all
verify:    from-manual
topic:     memory
claim:     OS-9 does not require a memory management unit (MMU); programs see actual physical memory addresses, and register-indirect addressing provides the necessary isolation.
context:   An MMU is optional and used for memory protection; it can be added via the System Security Module (ssm).
source:    The OS-9 Guru, section 1.9, page 14
--- END ---

--- CARD ---
id:        os9-memory-map-single-space
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     memory-management
claim:     OS-9 uses a software memory management system with a single memory map. All user tasks share a common address space (they are not isolated from one another).
context:   Recommendation: keep memory sections in contiguous reserved blocks, arranged for future expansion; physically contiguous RAM is preferred.
source:    OS-9 v2.4 Technical Reference Manual, "OS-9 Memory Map", p. 2-7
--- END ---

--- CARD ---
id:        dma-address-translation-mmu
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     dma-caching
claim:     On systems with an MMU, DMA transfers from user buffers require address translation because DMA hardware may not respect virtual-to-physical mappings. OS-9 provides address translation services for DMA.
context:   MMU-based (Level-2-style) systems must map user virtual addresses to physical addresses for DMA.
source:    Technical I/O Manual v2.4, "Address Translation and DMA Transfers", p. 1-42
--- END ---

## Direct memory access (BASIC)

--- CARD ---
id:        poke-peek-memory-access
type:      FACT
target:    68k
verify:    from-manual
topic:     low-level-access
claim:     POKE and PEEK statements allow direct memory access (POKE to write, PEEK to read). Dangerous for system stability; typically used only for OS-9 system globals or hardware access.
context:   -
source:    OS-9 BASIC User Manual (Revision G, 1991), Chapter 11, "Program Statements", command summary
--- END ---

## General / cross-target runtime memory facts

--- CARD ---
id:        edata-end-symbols
type:      FACT
target:    all
verify:    from-manual
topic:     memory-mgmt
claim:     Two linker-defined symbols, `edata` and `end`, mark addresses one byte past the end of a program's initialized data and one byte past the end of its uninitialized data, respectively. They are not variables — a C program must take their address with `&edata` / `&end` to get the values (in assembler, they're accessed as plain labels, e.g. `leax end,y`).
context:   Useful for a program that wants to know where its own static data region ends, e.g. before calling sbrk()/malloc()-style allocation. The assembler example is 6809 syntax; the symbol convention itself is a general linker facility.
source:    OS-9 C Compiler manual, "Memory Management", p. 2-5
--- END ---

## 6809-specific: direct page and stack facts (do not merge with 68k material)

--- CARD ---
id:        direct-storage-class
type:      FACT
target:    6809
verify:    from-manual
topic:     c-compiler
claim:     This compiler extends K&R's storage-class specifiers with three 6809-only additions: `direct`, `extern direct`, and `static direct`, which place a variable in the 6809's "direct page" — a 256-byte memory page addressable with fast 2-byte instructions via the CPU's direct-page register. Direct-class variables cannot be used to declare function arguments, and (like other uninitialized globals/statics) default to zero if not explicitly initialized.
context:   This entire storage class exists to exploit a 6809 hardware addressing mode; the manual itself warns "direct" is unique to this compiler and programs using it may not be portable to other environments even within the 6809 world, let alone to 68k.
source:    OS-9 C Compiler manual, "The 'Direct' Storage Class", p. 1-3
--- END ---

--- CARD ---
id:        direct-page-256-byte-limit
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     c-compiler
claim:     Total 6809 direct-page storage is capped at 255 usable bytes (the linker itself consumes 1 of the page's 256 bytes). If a program's declared `direct`-class variables would exceed this, the linkage editor reports an error and the programmer must move some variables out of direct storage to fit.
context:   Not applicable off the 6809 — a page-relative fast-addressing hardware feature of this specific CPU.
source:    OS-9 C Compiler manual, "The 'Direct' Storage Class", p. 1-3 to 1-4
--- END ---

--- CARD ---
id:        direct-page-min-one-byte
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     memory-mgmt
claim:     Even when a program declares no `direct`-class variables at all, the compiler/linker still reserves at least 1 byte for the direct-page variable area. This is deliberate: it guarantees no pointer to a direct-page variable can ever have the value 0 (NULL).
context:   6809-direct-page-specific; not meaningful on architectures without a direct-page addressing mode.
source:    OS-9 C Compiler manual, "Memory Management", p. 2-5
--- END ---

--- CARD ---
id:        stack-reservation-64-byte-overhead-6809
type:      FACT
target:    6809
verify:    from-manual
topic:     memory-mgmt
claim:     On each C function entry, a system-interface routine reserves stack space for that function's use plus a fixed additional 64 bytes, reserved for user-written assembly routines, the system interface, and arithmetic-support routines.
context:   The 64-byte figure is this 6809 runtime's specific constant; not confirmed for a 68k runtime, which would likely need a different (probably larger, given wider registers) reserve.
source:    OS-9 C Compiler manual, "Memory Management", p. 2-5
--- END ---

--- CARD ---
id:        stack-overflow-detection-error-message
type:      BEHAVIOR
target:    6809
verify:    from-manual
topic:     memory-mgmt
claim:     The runtime tracks the lowest stack address granted so far. If a function's stack request would push that watermark down far enough to overlap the data area, the program halts and prints "**** STACK OVERFLOW ****" to standard error rather than proceeding; otherwise the new (lower) limit is accepted and execution continues.
context:   Overflow detection can be disabled entirely with the -S compiler flag for time-critical code once the programmer is confident of correct stack usage.
source:    OS-9 C Compiler manual, "Memory Management", p. 2-5
--- END ---

--- CARD ---
id:        compile-time-memory-default-4k
type:      FACT
target:    6809
verify:    from-manual
topic:     memory-mgmt
claim:     Unless told otherwise (via -M=), this compiler's linker automatically allocates 4K bytes more than the total size of a program's variables and strings as its default runtime memory pool — intended to cover the parameter area, stack, and standard-library file buffers. Requests for less than 256 bytes via -M= are ignored by the linker.
context:   This specific 4K default and 256-byte floor are this compiler/linker's own defaults, not confirmed for a 68k linker.
source:    OS-9 C Compiler manual, "Compile Time Memory Allocation", p. 2-6
--- END ---
