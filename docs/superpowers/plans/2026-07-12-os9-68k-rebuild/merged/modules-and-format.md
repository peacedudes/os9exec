# Modules, format, and linking

Sifted and merged from `modules-and-format-RAW.md`: 185 raw cards in, 57 cards out.
Organized by sub-topic (core concepts; module directory & link-count lifecycle;
68k header byte layout; 6809 header divergence; program-module extended header
fields; CRC verification; permissions/ownership; trap library & subroutine
modules; data modules; system architecture & boot-time discovery; shell
interplay; linker/object-format specifics; BASIC09 packing) rather than by
source document. Cards purely about RBF/path-descriptor/file-manager I/O
mechanics, process scheduling, C-language/C-library specifics, and BASIC09
shell syntax were pruned as out-of-cluster; a couple dozen near-identical
restatements of the same fact across manuals were merged into one card each.

## Core concepts

--- CARD ---
id:        module-fundamental-unit
type:      CONCEPT
target:    all
verify:    authored
topic:     modules
claim:     In OS-9, the unit of loadable code and data is the memory module, not the file. Programs, device drivers, file managers, libraries, and data areas are all modules: a standard header (type, language, CRC, attributes) plus a body, tracked by name in an in-memory module directory. "Loading" means making a module known to the directory; "running" means forking a process from an already-known module. Modules are located and shared by name, the same way processes locate files by pathname, but modules live in memory rather than on disk.
context:   The single biggest mental shift from a Unix/ELF model: there is no fixed load address, no per-process private code copy (one resident module is shared reentrantly across every process running it, each with its own private data area), and a module can outlive the process that loaded it if flagged "sticky."
source:    authored; The OS-9 Guru §3.1 p.33; The OS-9 Primer ch.5 "OS-9 Memory Modules"; Using Professional OS-9 v2.4 "The Memory Module and Modular Software" p.1-6
--- END ---

--- CARD ---
id:        module-header-fields-general
type:      CONCEPT
target:    all
verify:    from-manual
topic:     modules
claim:     Every module carries a standard OS-9 module header: a type/language byte pair, an attribute/revision byte pair, an execution offset (where to start running), a storage size (initial memory allocation for data/stack/parameters), and a module name. The module name is what OS-9 uses to register/find the module in the module directory.
context:   The specific byte *values* used for type/language/attribute encodings are architecture-dependent — see the 6809-specific and 68k-specific header cards below for the concrete values.
source:    OS-9 C Compiler manual, "Module Header" / "Module Name", p. 2-1 to 2-2
--- END ---

--- CARD ---
id:        position-independence-concept
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     modules
claim:     All OS-9/68000 modules must be position-independent: a program cannot know or assume where it will be loaded in memory, and OS-9 determines the load address only when the program is run. OS-9 compilers and interpreters generate position-independent code automatically; hand-written 68000 assembly requires the programmer to avoid addressing modes that refer to absolute addresses. The 68000's PC-relative and register-indirect addressing modes are what make this practical.
context:   Prerequisite for ROM-ability and for a module's memory address being reassignable across successive loads.
source:    The OS-9 Guru §1.9 p.14; the independent 1985-era OS-9/68000 technical manual §1-3 "Module Requirements"; OS-9 v2.4 Technical Reference Manual "Memory Modules" p.1-7; The OS-9 Primer ch.5 "Address"
--- END ---

--- CARD ---
id:        module-reentrancy-concept
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     modules
claim:     A re-entrant module is non-self-modifying code whose single in-memory copy can be shared and run concurrently by multiple processes, each of which accesses its own independent data area via a base-address register (a6 by linker convention) rather than through code-embedded addresses. Because the code never changes, one user's execution cannot corrupt another's; if one process could write into shared code, every user of that module would see the change, so re-entrant modules must not modify themselves. Data modules are the one exception permitted to be non-self-modifying-restricted, since they exist specifically to hold mutable shared state.
context:   Re-entrance is indicated by module attribute bit 7 (M$Attr). Almost all OS-9 family software is re-entrant; this is why running the same program from many terminals costs one code copy, not one per user.
source:    The OS-9 Guru §1.9 p.15; the independent 1985-era OS-9/68000 technical manual §1-3 "Module Requirements", §4-1 "Process Memory Areas"; OS-9 v2.4 Technical Reference Manual "Memory Modules" p.1-7; The OS-9 Primer ch.5 "Attribute and Revisions"; OS-9 Insights §2.8.1 "Reentrant Modules"
--- END ---

--- CARD ---
id:        module-string-literal-text-section-reentrancy
type:      CONCEPT
target:    all
verify:    from-manual
topic:     modules
claim:     C string literals are placed in the module's TEXT (executable) section rather than its DATA section. Because a reentrant module's TEXT is shared read-only across all concurrent invocations, this avoids duplicating string-literal storage per invocation — at the cost that programmers must not alter a string literal in place (copy it out first). The one documented exception is a char array explicitly initialized from a string literal (`char msg[] = "hi";`), whose backing storage lands in DATA (one copy per invocation) and is safely mutable.
context:   A concrete illustration of what reentrancy costs a C programmer in practice, distinct from the general reentrancy concept.
source:    OS-9 C Compiler manual, "String Literals", p. 2-2 to 2-3
--- END ---

--- CARD ---
id:        memory-map-parameter-stack-data-order
type:      CONCEPT
target:    all
verify:    from-manual
topic:     modules
claim:     A running C program's memory, from high addresses to low, is laid out as: the parameter string passed by the parent process, the (downward-growing) stack, free/heap memory available for sbrk()/malloc(), uninitialized data, then initialized data. The overall size of this region comes from the "storage size" field recorded in the program's own module header, and can be overridden at invocation time via the OS-9 Shell's memory-size ("#") option.
context:   Exact low-address layout below initialized data (e.g. any CPU-specific fast-access area) is architecture-dependent.
source:    OS-9 C Compiler manual, "Typical C Program Memory Map", p. 2-4
--- END ---

--- CARD ---
id:        info-directive-module-strings
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     Strings preceded by the assembly-source directive "info" are placed into the module's information/description area of the header. A common use is embedding a version number and/or copyright notice into the compiled module. The `#asm` pre-compiler directive can be used from C source to emit this directive into the compiler-generated assembly.
context:   -
source:    OS-9 C Compiler manual, "Information", p. 2-2
--- END ---

## Module directory & link-count lifecycle

--- CARD ---
id:        module-directory-structure
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     The module directory is a kernel-maintained table with one entry per loaded module; each entry holds the module's memory address, link count, group identifier (address of the first module loaded from the same file), and parity check value. Module lookup compares the requested name against the module header's own name field.
context:   -
source:    The OS-9 Guru §3.1 p.33
--- END ---

--- CARD ---
id:        f-link-vs-f-load-module-directory-lifecycle
type:      CONCEPT
target:    all
verify:    authored
topic:     modules
claim:     `F$Link` (C library: `modlink()`) searches the in-memory module directory for a module matching a given name and, if found, increments its link count rather than reloading anything from disk. `F$Load` (`modload()`) instead reads a file from disk, adds whatever modules it contains to the module directory, and only then behaves like a link. `F$UnLink` (`munlink()`) decrements a module's link count and, once that count reaches zero, frees the module's memory and removes it from the directory — unless the module is flagged "sticky," in which case it survives until the count is explicitly driven to -1 or the kernel needs the memory back. Implicit links also happen: forking a program implicitly links it, just as an explicit load or link call would.
context:   This link-count model is how OS-9 shares one in-memory copy of a reentrant module (a program run by multiple processes at once, or a shared library) without duplicating it per process — the module directory is closer to a Unix dynamic linker's loaded-shared-object table than to anything in the ordinary process/file model. `F$Exit` only auto-unlinks a process's *primary* module and its trap handlers; any other module a process explicitly loaded or linked must be explicitly unlinked before `F$Exit`, or it leaks (stays resident with an inflated link count).
source:    authored; OS-9 C Compiler manual "Modload, Modlink"/"Munlink" p.3-24 to 3-25; The OS-9 Guru §3.3 p.48; the independent 1985-era OS-9/68000 technical manual §1-1 "Kernel Memory Management Functions", §14-12 "F$Exit Caveats"; OS-9 v2.4 Technical Reference Manual "Memory Management" p.2-6; The OS-9 Primer ch.5 "Link Count" (link count is not guaranteed accurate — commands like link/unlink can adjust it artificially)
--- END ---

--- CARD ---
id:        f-fork-module-resolution-and-type-check
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     modules
claim:     When forking or chaining to a named module, OS-9 first checks whether a module of that name is already in the module directory; if found but its type/language doesn't match what the caller requested, the kernel returns E_NEMOD rather than attempting to load a different (e.g. non-executable) module of the same name from disk. If no matching module is resident, the name is treated as a file path and the first module in that file is loaded from the execution directory. Both `os9fork()`/`chain()` (C library) and F$Fork itself take explicit type/language arguments checked against the target module's own header fields.
context:   This prevents a namespace collision (e.g. a data module and a program module sharing a name) from silently causing the wrong thing to load.
source:    OS-9 C Compiler manual "Chain" p.3-4 to 3-5, "Os9fork" p.3-29; The OS-9 Guru §3.3 p.48; the independent 1985-era OS-9/68000 technical manual §14-14 "F$Fork"
--- END ---

--- CARD ---
id:        module-revision-override-mechanism
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     modules
claim:     When a module with the same name, type, and language as an already-resident module is loaded, the kernel compares revision numbers (M$Revs): the higher revision overwrites the module directory entry. This does NOT wait for the old module's link count to drop to zero — existing processes already using the old module keep running against it (it isn't yanked out from under them), but any new lookup gets the new version.
context:   Allows updating modules in ROM from a boot file, but requires care to avoid a process ending up split across old and new behavior if it links again mid-run.
source:    The OS-9 Guru §3.2.8 p.42-43; the independent 1985-era OS-9/68000 technical manual §1-6 "Module Header Definitions"; The OS-9 Primer ch.5 "Attribute and Revisions"
--- END ---

--- CARD ---
id:        module-groups-memory-allocation
type:      CONCEPT
target:    all
verify:    from-manual
topic:     modules
claim:     Multiple modules loaded from the same file form a "module group"; the group identifier is the memory address of the first module in the group. All modules in a group remain in the directory until the entire group's combined link count reaches zero.
context:   Solves the problem of non-page-aligned module sizes preventing individual modules within a merged file from being freed independently — keeps the underlying contiguous memory block valid for memory management.
source:    The OS-9 Guru §3.3.1 p.49
--- END ---

--- CARD ---
id:        module-file-format
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     A file containing a module can hold one or more modules merged sequentially; the linker generates each module's header and CRC. Such a file can be a disk file or something already loaded to memory by a load utility — to the disk file manager, a module file is indistinguishable from any other file.
context:   -
source:    The OS-9 Guru §3.3 p.48
--- END ---

--- CARD ---
id:        a6-register-bias-for-data-area
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     modules
claim:     Register a6 (the static-storage/data-area base pointer set up for a new process) is deliberately biased by $8000, letting object code reach the full 64K data area using indexed addressing from a single base register. The linker automatically compensates for this bias when generating code, so programmers usually don't need to account for it manually.
context:   Relevant to understanding why disassembled/decompiled 68k OS-9 object code references data at addresses that look offset from a6 by a fixed $8000.
source:    OS-9 v2.4 Technical Reference Manual, Figure 2-3, p.1-24; the independent 1985-era OS-9/68000 technical manual §4-4 "Figure 7: New Process Initial Memory Map"
--- END ---

## Module header byte layout — 68k

--- CARD ---
id:        module-header-sync-word
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Every 68k module header begins with the constant sync bytes $4AFC (M$ID field, offset $00) — an illegal 68000 instruction, chosen so the kernel can scan ROM or memory word-by-word during coldstart and cheaply recognize module boundaries before spending time on header parity or CRC checks.
context:   -
source:    The OS-9 Guru §3.2.1 p.35; the independent 1985-era OS-9/68000 technical manual §1-4 "Module Header Definitions"; OS-9 Insights §2.1 "How Modules Are Identified"; OS-9 v2.4 Technical Reference Manual "Module Header Definitions" p.1-9
--- END ---

--- CARD ---
id:        module-header-size-and-sections
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Every module has three parts: a header (a minimum of 48 bytes, following the `modhcom` structure in `module.h`, standardized across all module types though some types extend it further), a type-specific body, and a trailing 3-byte CRC.
context:   -
source:    OS-9 Insights §2.1; The OS-9 Primer ch.5 "Module Format", "Module Header"; OS-9 v2.4 Technical Reference Manual "Basic Module Structure" p.1-8
--- END ---

--- CARD ---
id:        module-header-offset-table
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Standard 68k module header field offsets: $00=M$ID (sync bytes), $02=M$SysRev, $04=M$Size (overall module size in bytes, including header and CRC), $08=M$Owner, $0C=M$Name (offset to name string), $10=M$Accs, $12=M$Type, $13=M$Lang, $14=M$Attr, $15=M$Revs, $16=M$Edit, $18=M$Usage, $1C=M$Symbol, $20-$2D=reserved, $2E=M$Parity, $30+=module-type-specific fields.
context:   Resolve these offsets in assembly via the sys.l/usr.l libraries, which define the symbolic names. This project independently confirmed the M$Attr offset ($14) live against a real compiled 68k program, corroborating this table.
source:    OS-9 v2.4 Technical Reference Manual, Figure 1-4, p.1-12; the independent 1985-era OS-9/68000 technical manual §1-4 "Module Header Definitions" (M$Size)
--- END ---

--- CARD ---
id:        module-header-name-field
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     The M$Name field (offset $0C) holds not the name itself but an offset to the module name string, measured from the module's first sync byte. The name string itself can be located anywhere in the module body and is ASCII, NUL-terminated, and of arbitrary length.
context:   This indirection is why module names aren't a fixed-width field.
source:    the independent 1985-era OS-9/68000 technical manual §1-4 "Module Header Definitions"; The OS-9 Primer ch.5 "Name"
--- END ---

--- CARD ---
id:        module-type-codes-68k
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     M$Type (offset $12, high byte of the type/language word) values: 0=wildcard/not-used, 1=Prgm (program), 2=Sbrtn (subroutine), 3=Multi (multi-module), 4=Data, 5=CSDData, 6-10=reserved, 11=TrapLib (user trap library), 12=Systm (system component), 13=Flmgr (file manager), 14=Drivr (device driver), 15=Devic (device descriptor), 16-255=user-definable. The kernel validates the type code against intended usage — e.g. attempting to fork a non-program-type module returns E_NEMOD.
context:   -
source:    The OS-9 Guru §3.2.7 p.39; the independent 1985-era OS-9/68000 technical manual §1-5 "Module Header Definitions"; OS-9 v2.4 Technical Reference Manual "Module Header Definitions" p.1-9; The OS-9 Primer ch.5 "Type"
--- END ---

--- CARD ---
id:        module-language-codes-68k
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     M$Lang (offset $13, low byte of the type/language word) values: 0=wildcard/not-used, 1=Objct (68000 machine language), 2=ICode (compiled BASIC), 3=PCode (Pascal), 4=CCode (C), 5=CblCode (Cobol), 6=FrtnCode (Fortran), 7-15=reserved, 16-255=user-definable. The language code tells Shell/kernel which runtime or interpreter, if any, is required to execute the module.
context:   -
source:    The OS-9 Guru §3.2.7 p.39-40; the independent 1985-era OS-9/68000 technical manual §1-5 "Module Header Definitions"; OS-9 v2.4 Technical Reference Manual "Module Header Definitions" p.1-10
--- END ---

--- CARD ---
id:        module-attribute-byte-68k
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     M$Attr (offset $14) bit meanings: bit 7 = sharable/re-entrant — when set the kernel permits a link count greater than one; when clear, only one simultaneous link is allowed (on a device descriptor this means only one path can open the device at a time; on a program module it means the program is not re-entrant and may self-modify). Bit 6 = sticky — the module remains in memory even after its link count reaches zero, and is only actually removed by a further explicit unlink (driving the count to -1) or under memory pressure; used for frequently-used utilities to avoid reload overhead. Bit 5 = supervisor/system state — when set the module runs in 68000 supervisor state (all memory, privileged instructions); clear means user state; set for all OS components.
context:   Bits confirmed consistent across multiple manuals and this project's own live verification of the M$Attr field's offset.
source:    The OS-9 Guru §3.2.8 p.41-42; the independent 1985-era OS-9/68000 technical manual §1-6 "Module Header Definitions"; OS-9 v2.4 Technical Reference Manual "Module Header Definitions" p.1-10; The OS-9 Primer ch.5 "Attribute and Revisions"/"System State"
--- END ---

--- CARD ---
id:        module-edition-field
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     A module's edition (M$Edit, offset $16) is a 16-bit unsigned value assigned at module creation, distinct from the revision (M$Revs) field. It's a human-facing version identifier and is checked when loading a module update from disk.
context:   Don't confuse with M$Revs, which drives the kernel's automatic higher-revision-replaces-lower directory substitution.
source:    The OS-9 Primer ch.5 "Edition"
--- END ---

--- CARD ---
id:        module-header-parity-and-checksum
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     M$Parity (offset $2E) holds the one's complement of the word-by-word XOR of all preceding header words. Because parity is defined as the complement of that XOR, XOR-ing all header words together *including* the parity word itself yields $FFFF when the header is intact (a value X XOR'd with its own bitwise complement always produces all-one-bits). The kernel recalculates this on module link and returns E_BMHP on mismatch, protecting the header (though not the module body, which can be legitimately altered post-load — e.g. for debugger breakpoints) against corruption. In OS-9 v2.4+, some sources describe a refined version of this integrity check that additionally rotates the accumulating value right by (word-value mod 16) bits after each word is added, said to give better corruption detection than plain XOR; v2.2 reportedly used a simpler low-16-bits-of-sum scheme.
context:   One source (OS-9 Insights) describes the same parity formula but states the all-words-XOR'd verification result as 0 rather than $FFFF — that number is not internally consistent with the "one's complement of XOR" definition every other source agrees on, so $FFFF is treated as authoritative here.
source:    The OS-9 Guru §3.2.11 p.43-44; the independent 1985-era OS-9/68000 technical manual §1-6 "Module Header Definitions"; OS-9 Insights §2.2 "The Header Check" (conflicting result value, see context)
--- END ---

## Module header — 6809 divergence (do not merge with 68k values above)

--- CARD ---
id:        module-header-6809-object-code-byte
type:      FACT
target:    6809
verify:    from-manual
topic:     modules
claim:     For the OS-9/6809 C compiler, the module header's type/language byte is set to $4, documented as "C Program — 6809 Object Code," and the attribute/revision byte is set to $81 (Reentrant + revision level 1).
context:   These specific byte values are this 6809 compiler's own encoding — a 68k C compiler's module header uses a different type/language byte value (see module-type-codes-68k / module-language-codes-68k). Do not treat $4 as a general OS-9 constant, and note the 6809 module header is a shorter, differently-laid-out structure than the 68k one documented above (16-bit addressing throughout vs. 68k's longer, offset-table-driven header).
source:    OS-9 C Compiler manual, "Module Header", p. 2-1
--- END ---

--- CARD ---
id:        module-lang-nibble-6809-value
type:      FACT
target:    6809
verify:    from-manual
topic:     modules
claim:     When calling os9fork() or chain() for this compiler's programs, the "lang" argument must equal 1, documented as "6809 machine code," matching the language nibble recorded in that compiler's module header.
context:   This specific nibble value (1) is this compiler/architecture's own encoding. It happens to coincide numerically with the 68k header's M$Lang=1 ("Objct", 68000 machine language) but the two values describe different, mutually incompatible machine-code formats — they are not the same fact and must not be merged.
source:    OS-9 C Compiler manual, "Chain", p. 3-5; "Os9fork", p. 3-29
--- END ---

--- CARD ---
id:        data-text-data-data-reference-tables
type:      CONCEPT
target:    6809
verify:    from-manual
topic:     modules
claim:     Because no absolute addresses are known at compile time under OS-9, the 6809 C compiler's runtime resolves pointer-valued initializers (e.g. `char *p = "string";`) at load time using two tables recorded in the module: a "data-text reference" table (offsets needing the base address of the TEXT/code section added) and a "data-data reference" table (offsets needing the base address of the DATA section added). The startup routine walks both tables once, after first copying all initializer data into the DATA section, to patch these pointer values into absolute addresses.
context:   This is the 6809 compiler's specific implementation strategy for achieving position-independence, and its direct counterpart is the 68k header's M$IRefs/M$IRela table (see the program-module-extended-header-fields and data-pointers-initialization-table-format cards) — a genuine architectural divergence in mechanism, not just field values, so the two are kept as separate cards.
source:    OS-9 C Compiler manual, "Data References", p. 2-3
--- END ---

--- CARD ---
id:        rma-psect-vsect-csect-6809-mnemonics
type:      CONCEPT
target:    6809
verify:    from-manual
topic:     assembler
claim:     The bundled Relocating Macro Assembler (RMA, invoked as c.asm) organizes a 6809 source file into sections: PSECT (code, where any actual 6809 instruction mnemonic is legal, plus a handful of directives), VSECT (initialized/uninitialized data, optionally the direct page), and CSECT (an offset-only counter for assigning label offsets without EQUs). This replaces the older Microware Interactive Assembler's single MOD/EMOD directive pair; PSECT records the module type/lang/attr/edition/stack-size/entry-point info that used to live in MOD, but for the *linker* (c.link) rather than for OS-9 directly.
context:   PSECT/VSECT/CSECT could be a Microware-linker-family convention worth checking against the 68k assembler, but this manual explicitly restricts PSECT's body to real 6809 opcodes, so the exact directive set and legal-mnemonic list is 6809-specific until independently confirmed for 68k.
source:    OS-9 C Compiler manual, "Relocating Macro Assembler Reference" / "Differences between RMA and MIA", p. D-1 to D-4
--- END ---

## Program-module extended header fields (68k)

--- CARD ---
id:        program-module-extended-header-fields
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Program, file-manager, and system modules carry additional header fields beyond the universal 48-byte header, starting around offset $30: M$Exec (offset to the entry point, relative to module start — set by the linker from the "root" psect in the linked files; need not be at module start), M$Excpt (default trap entry point — called if the program executes a TRAP instruction with no handler installed for that trap number), M$Mem (required size of the program's data area), M$Stack (minimum required stack size), M$IData (offset to the initialized-data table), M$IRefs/M$IRela (offset to the pointer-relocation table). The kernel computes an absolute entry-point address by adding M$Exec to the module's runtime memory address.
context:   See the dedicated cards below for the internal byte-format of the M$IData and M$IRefs tables — those are drill-downs, not duplicates, of this field list.
source:    The OS-9 Guru §3.2.12 p.44; the independent 1985-era OS-9/68000 technical manual §1-7/1-8 "Additional Header Fields For Individual Modules"; OS-9 v2.4 Technical Reference Manual "Additional Header Fields For Individual Modules" p.1-13
--- END ---

--- CARD ---
id:        module-minimum-stack-size-field
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     modules
claim:     M$Stack is set by the linker to an assumed maximum stack depth — it cannot know the true maximum since recursion is unbounded — defaulting to 3K bytes and overridable via a linker option. At fork time, the kernel sums this stack size, the module's data size (M$Mem), the parameter string size, and any additional caller-requested stack, to determine the total contiguous memory needed for the new process.
context:   -
source:    The OS-9 Guru §3.2.15 p.45; the independent 1985-era OS-9/68000 technical manual §1-8 "Additional Header Fields For Individual Modules"
--- END ---

--- CARD ---
id:        data-initialization-table-format
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     The data initialization table (built by the linker, pointed to by M$IData) is a series of entries, each: a 4-byte start offset into the process's static (data) storage, a 4-byte size in bytes, followed by that many bytes of initialization data. On fork, the kernel copies this data verbatim into the newly allocated data area at the given offsets — this is how C static-variable initializers get their startup values.
context:   Values declared in "vsects" are placed here by the linker.
source:    The OS-9 Guru §3.2.16 p.45; the independent 1985-era OS-9/68000 technical manual §1-8 "Additional Header Fields For Individual Modules"
--- END ---

--- CARD ---
id:        data-pointers-initialization-table-format
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     The pointer-relocation table (M$IRefs/M$IRela) is two sub-tables, each formatted as: a word holding the most-significant word of an offset into the data area, followed by a count word, followed by that many least-significant-word offsets, terminated by a zero count. The first sub-table locates data-area slots holding pointers into the code/TEXT area (fixed up by adding the module's absolute base address); the second locates slots holding pointers into the DATA area itself (fixed up by adding the data area's absolute base address). F$Fork walks both sub-tables to patch these pointers at process-creation time — this is the 68k mechanism for position-independent initialized pointer data.
context:   This is the 68k architectural counterpart to the 6809 C compiler's data-text/data-data reference tables (see data-text-data-data-reference-tables) — same underlying problem, differently structured solution; kept as a separate card deliberately.
source:    The OS-9 Guru §3.2.17 p.46; the independent 1985-era OS-9/68000 technical manual §1-8 "Additional Header Fields For Individual Modules"
--- END ---

## CRC verification

--- CARD ---
id:        module-crc-integrity-mechanism
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Every module ends with a 24-bit CRC computed over the entire module (header through the byte just before the CRC field itself); the accumulator is initialized to $FFFFFF and the final value is one's-complemented before being stored. Because the 68000 requires all instruction words on even byte addresses, the linker inserts a zero padding byte at the end of the module body (before the CRC) whenever needed to keep the module's length even. The kernel checks this CRC via the F$CRC system call before a module is entered into the module directory — a module whose computed CRC doesn't match its stored CRC is refused rather than loaded. The Fixmod utility recomputes and updates a module's CRC after it has been hand-patched.
context:   A specific worked value: with CRC bytes included in the computation, a valid module's full-module CRC accumulation lands on $800FE3.
source:    The OS-9 Guru §3.2.18 p.47-48; OS-9 v2.4 Technical Reference Manual "The CRC Value" p.1-8; OS-9 Insights §2.3; The OS-9 Primer ch.5 "Module CRC"
--- END ---

--- CARD ---
id:        module-not-reverified-after-load
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     modules
claim:     Once a module is validated and placed in the module directory, OS-9 does not re-verify its CRC on every subsequent use — the check happens exactly once, at ROM bootstrap or at RAM load time.
context:   Relevant if reasoning about whether an in-memory module can drift from its on-disk CRC-verified state without detection.
source:    OS-9 Insights §2.5 "Circumventing CRC Protection"
--- END ---

--- CARD ---
id:        crc-calculation-processor-cost
type:      GOTCHA
target:    all
verify:    from-manual
topic:     modules
claim:     CRC calculation requires significant processor effort. The time cost is imperceptible during a single module load, but high-performance/time-critical applications should pre-load needed modules before the time-critical portion of the application starts rather than triggering their CRC-checked load mid-operation.
context:   Also generally good practice to reduce disk access in time-critical apps regardless of CRC cost.
source:    The OS-9 Guru §3.2.18 p.48
--- END ---

--- CARD ---
id:        crc-accumulation-helper
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     The `crc()` library call accumulates a running CRC into a caller-supplied 3-byte array, over "count" bytes starting at a given address; the 3-byte accumulator must be initialized to 0xFFFFFF before the first call, but repeated calls can extend the same running CRC across an entire module. If the result is to be used as an OS-9 module's own CRC check field, its bytes must be complemented before insertion.
context:   Documents the general OS-9 module-CRC algorithm's calling convention (init to all-ones, complement before storing), independent of CPU architecture — matches the kernel-side algorithm described in module-crc-integrity-mechanism.
source:    OS-9 C Compiler manual, "Crc", p. 3-10
--- END ---

## Module permissions & ownership

--- CARD ---
id:        module-access-permissions-bit-layout
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Module access permissions (M$Accs, offset $10) occupy the low 12 bits of a word: bits 0-2 = private (owner) permissions, bits 4-6 = group permissions, bits 8-10 = public permissions; bits 3, 7, 11 are reserved. Within each 3-bit group, bit 0 = read, bit 1 = write, bit 2 = execute. Read/execute permission is what gates load/link/unlink/fork of the module; write permission affects write-to-module capability only on systems with memory protection hardware. The same permission scheme is often described as three octal digits (owner/group/world rwx), directly analogous to Unix file permission bits.
context:   -
source:    The OS-9 Guru §3.2.6 p.37; The OS-9 Primer ch.5 "Perm"
--- END ---

--- CARD ---
id:        module-security-check-order-and-superuser
type:      FACT
target:    all
verify:    from-manual
topic:     modules
claim:     Group zero is the super-user group: any user whose group ID is zero may link/load/access any module regardless of that module's permission bits, and can make certain privileged system calls unavailable to other groups. For a non-super-user, module access is checked in order: same user-ID and group → private permission bits apply; same group only → group permission bits apply; different group → public permission bits apply (the fallback case). Programmers can compile privileged programs to delegate controlled resource access to non-privileged users without making them super-user.
context:   -
source:    The OS-9 Guru §3.2.4 p.37
--- END ---

--- CARD ---
id:        privileged-module-ownership-requirement
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     modules
claim:     I/O-system modules (file managers, drivers, device descriptors) must be owned by a super-user (group 0) and have the system-state attribute bit set in the module header; more generally, any module file owned by super-user must remain owned by super-user or OS-9 refuses to load it.
context:   The system-state-bit requirement for I/O modules may become more strictly enforced in future OS-9 revisions per the source manual's own note.
source:    Technical I/O Manual v2.4, "File Managers", p. 1-17; Disk File Organization manual, "File Security", p. 7-11
--- END ---

## Trap library & subroutine modules

--- CARD ---
id:        trap-handler-module-type
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     A trap handler is a module of type TrapLib (code 11), language Objct, containing subroutines called via the 68000 TRAP instruction; a program specifies which trap handler module(s) to use, then calls functions in it by trap number rather than by resolving an address through explicit module linking. Trap handler modules have exactly three defined entry points: trap execution (invoked via tcall/TRAP), trap initialization (invoked once per handler per program, on F$TLink), and trap termination (reserved; not implemented in this era of OS-9).
context:   TRAP #0 is reserved for OS-9 system calls proper; user trap handlers occupy TRAP #1-#15.
source:    The OS-9 Guru §1.8.7 p.13, §10.4 p.202; OS-9 v2.4 Technical Reference Manual "Trap Handlers" p.5-1 to 5-2
--- END ---

--- CARD ---
id:        trap-handler-installation-lifecycle
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     modules
claim:     F$TLink installs a trap handler: the kernel links to the trap module, allocates and initializes its own private static storage (separate per use, unlike a subroutine module), and executes the module's initialization entry point. A process may link to at most 15 trap modules simultaneously (matching the 15 available user trap vectors, 33-47 in the exception table), but has no such limit on ordinary subroutine-module links (bounded only by system memory).
context:   Initialization entry is a natural place for one-time setup like opening files or linking to other modules.
source:    OS-9 v2.4 Technical Reference Manual "Installing and Executing Trap Handlers" p.5-3; the independent 1985-era OS-9/68000 technical manual §4-9 "User Traps"; The OS-9 Primer ch.13 "Trap Modules"
--- END ---

--- CARD ---
id:        trap-lazy-binding-via-exception
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     modules
claim:     A trap handler can be bound lazily: if a tcall is made before F$TLink has installed the handler, the kernel checks the target module's M$Excpt (default trap entry point) offset. If zero, the call aborts; otherwise execution jumps to that exception routine, which installs the handler and re-executes the original tcall. This lets a program make a trap handler's linkage conditional on whether it's actually needed at runtime.
context:   -
source:    OS-9 v2.4 Technical Reference Manual "OS9 and tcall: Equivalent Assembly Language Syntax" p.5-3
--- END ---

--- CARD ---
id:        subroutine-module-no-static-storage
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     modules
claim:     Subroutine modules cannot access static/global variables by name, because they are not linked at compile time and have no static storage area of their own — all such state must be passed in as parameters or reached via pointers supplied by the calling program. A subroutine module instead exposes its callable routines via an index table: an array of integer offsets, measured from the module header, one per callable routine.
context:   Contrast with trap modules (below), which do get private static storage.
source:    The OS-9 Primer ch.13 "Subroutine modules", "Index table"
--- END ---

--- CARD ---
id:        trap-module-private-storage-and-execution-state
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Unlike subroutine modules, a trap module is given its own private static storage area by the system, kept separate per use. Trap module code always executes in the CPU state (user vs. system) declared by its own module attributes, regardless of the calling process's state — unlike subroutine code, which runs in the caller's context.
context:   -
source:    The OS-9 Primer ch.13 "Trap Modules"
--- END ---

--- CARD ---
id:        math-module-example
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     The standard Math module (installed as a trap-15 handler) is a concrete example of the shared-library principle: it provides floating-point arithmetic (single/double precision), extended integer ops, type conversions, and transcendental functions to every language and program on the system, so no program needs its own math package, and a hardware floating-point coprocessor can be substituted (Math881.l for the 68881/882 FPCP vs. plain Math.l) without recompiling client software. Both files register the module under the same name, "Math". 32-bit floats are internally promoted to 64-bit doubles for computation (no speed advantage from using float), and there's no support for denormalized numbers or negative zero. Compilers auto-link/call it; assembly programmers call it via F$TLink + trap.
context:   Can be pre-loaded via the Load command for quick access, though not generally recommended to bake into OS9Boot.
source:    OS-9 v2.4 Technical Reference Manual "Standard Function Library Module" p.6-1, "Calling Standard Function Module Routines" p.6-3; Using Professional OS-9 v2.4 "The Memory Module and Modular Software" p.1-6
--- END ---

## Data modules

--- CARD ---
id:        data-module-concept-and-creation
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     modules
claim:     A data module is OS-9's preferred mechanism for inter-process shared memory: processes reference shared data by module name rather than a hardcoded address. It's created via `_os_mkmodule()`/`_os_datmod()` (C) or the F$DatMod system call — the first process to create the module specifies its size and attributes, the data area is cleared automatically, and the module is entered into the directory with a valid CRC; subsequent processes simply link to the existing module by name to reach the same shared memory.
context:   -
source:    The OS-9 Primer ch.8 "Data Modules", "Creating a Data Module"; OS-9 v2.4 Technical Reference Manual "Creating Data Modules" p.4-17
--- END ---

--- CARD ---
id:        data-module-nonreentrant-and-sync
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     modules
claim:     Unlike program modules, a data module can be non-re-entrant — i.e. it can be modified in place — since that's the whole point of using one for shared, mutable state. OS-9 itself provides no synchronization for concurrent writers; coordinating multiple processes updating the same data module is entirely up to explicit events/signals in user code, and the internal layout/organization of the data area is not restricted by OS-9 at all.
context:   -
source:    OS-9 v2.4 Technical Reference Manual "Data Modules" p.4-16
--- END ---

--- CARD ---
id:        data-module-link-count-sticky
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     modules
claim:     A data module's link count behaves like an ordinary module's: it's removed when the count reaches zero, unless the sticky bit was set at creation time, in which case it remains resident until the count is driven to -1 or the kernel needs the memory back.
context:   -
source:    OS-9 v2.4 Technical Reference Manual "The Link Count" p.4-17
--- END ---

--- CARD ---
id:        data-module-crc-invalidated-on-modify
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     modules
claim:     A data module that's been modified since creation has a stale (now-invalid) CRC if saved to disk in that state, and cannot be reloaded until fixed. Call F$SetCRC (`_setcrc()` in C) before writing it out, or run the fixmod utility on the saved file afterward. The `dump` utility can be used to examine a data module's format/contents directly.
context:   A modification-tracking issue unique to data modules, since program modules are never legitimately self-modifying.
source:    OS-9 v2.4 Technical Reference Manual "Saving to Disk" p.4-17
--- END ---

## System architecture & boot-time module discovery

--- CARD ---
id:        four-levels-of-modularity
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     modules
claim:     OS-9's modularity is organized in four levels, each a different module type: Level 1 — the kernel, clock module, and INIT module (basic system services); Level 2 — file managers (device-class-independent: RBF for disk, SCF for character devices, PIPEMAN for interprocess communication); Level 3 — device drivers (hardware-controller-specific); Level 4 — device descriptor modules (non-executable tables associating a logical device name with a driver, a file manager, and hardware configuration). Because a device descriptor is just a small table, only one copy of a given driver is needed system-wide no matter how many logical devices of that type exist — descriptors are cheap and swappable at runtime.
context:   This hierarchy is what lets a system be reconfigured (new device, new file manager) by swapping small modules rather than rebuilding anything monolithic.
source:    the independent 1985-era OS-9/68000 technical manual, Introduction "Four Levels of Modularity"; OS-9 v2.4 Technical Reference Manual "The OS-9 Unified Input/Output System" p.3-1
--- END ---

--- CARD ---
id:        init-and-kernel-customization-modules
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     modules
claim:     The INIT module is a Level-1 initialization table read by the kernel at startup, specifying initial table sizes, initial system device names, and other startup configuration. Kernel customization modules are separately listed within this INIT configuration; the kernel calls each one's initialization function at startup, letting new system calls be added or existing ones replaced without rebuilding the kernel itself. The Clock module, also Level 1, is the hardware-specific software handler for the platform's real-time clock, providing timing services to the rest of the OS.
context:   -
source:    the independent 1985-era OS-9/68000 technical manual, Introduction "Level 1 - The Kernel..."; The OS-9 Guru §1.8.8 p.14
--- END ---

--- CARD ---
id:        module-discovery-at-boot
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     modules
claim:     On reset/coldstart, the kernel scans ROM (and the boot file) word-by-word looking for the $4AFC sync code. When found, it verifies the header parity, reads the module's size from the header, and computes its CRC; a module that passes both checks is entered into the module directory. This is how ROMed modules end up automatically linked and present in the system module directory at boot — including non-system, user-supplied modules placed in ROM for discovery.
context:   Only after this module-population phase does coldstart link the INIT config module, initialize kernel tables/paths/directories, and fork the initial program.
source:    OS-9 v2.4 Technical Reference Manual "ROMed Memory Modules" p.1-9; The OS-9 Guru §2.1 p.18
--- END ---

## Shell & the module directory

--- CARD ---
id:        shell-command-resolution-via-module-directory
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     modules
claim:     When Shell processes a non-built-in command word, it first checks the in-memory module directory for an already-loaded module of that name (avoiding a reload if found), then falls back to searching the current execution directory — trying a module load, then procedure-file interpretation — and finally the PATH environment variable's colon-separated directory list, continuing until the command is found or the list is exhausted. If still not found, it's tried as a procedure file against the current data directory. A program loaded once this way is reused from the module directory on subsequent invocations without touching disk again.
context:   This is the module directory's role in everyday interactive use, not just in explicit F$Link/F$Load calls.
source:    Using Professional OS-9 v2.4, "The Shell", "Shell Command Line Processing" p.5-7, "The Shell Environment" p.5-3
--- END ---

## Linker & object-format specifics

--- CARD ---
id:        l68-error-taxonomy-corrupt-object-vs-link-failure
type:      GOTCHA
target:    68k
verify:    VERIFIED
topic:     modules
claim:     `l68`'s two early errors say very different things, and the distinction is diagnostic. `file 'x.r' is not a relocatable module` means the object did not *parse* at all — the file is corrupt or is not a ROF. `no root psect found` means the object parsed perfectly well and the linker simply found no entry point in it. Running `l68` on a *single* object file is therefore a cheap integrity check: a healthy object reports `no root psect found`; a corrupt one reports `not a relocatable module`.
context:   Worth reaching for early, because a corrupt object masquerades convincingly as a linker bug. In this project a single bad `.r` file — one of eleven, with a perfectly plausible ROF header and a creation timestamp one second apart from its known-good siblings — was blamed across two sessions on an `l68` size limit and on a toolchain-generation mismatch, when the actual fix was to recompile that one file. General lesson: when exactly one input out of many is rejected, rebuild it before theorizing about the tool.
source:    authored
--- END ---

--- CARD ---
id:        rof-header-carries-a-decodable-build-timestamp
type:      FACT
target:    68k
verify:    VERIFIED
topic:     modules
claim:     A ROF relocatable object (`.r`) begins with the sync longword `$DEADFACE`, and bytes 12..17 of the header hold its creation date as six bytes: year−1900, month, day, hour, minute, second. The module's code size sits at offset 0x1C and its name is a NUL-terminated string starting at offset 0x38.
context:   Directly useful for forensics on a suspect object, without needing any tool: decoding the timestamp of two `.r` files is enough to prove or disprove "these were built by different generations of the compiler" — a theory that can otherwise absorb a lot of effort. In this project two objects whose build dates differed by one second settled that question immediately.
source:    authored
--- END ---

--- CARD ---
id:        rma-library-merge-forward-reference-order
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     modules
claim:     When several separately-compiled modules (ROFs) are merged into one library file for c.link to search, the linker resolves each unresolved external reference on a first-found basis, using whichever ROF's matching symbol it encounters first in search order. Consequently, if procedure A in a library calls procedure B also in that library, B's ROF must be merged into the library *after* A's — i.e. all intra-library references should be forward references only.
context:   This ordering requirement is a consequence of this specific linker's single-pass, first-found symbol resolution; not necessarily true of a different (e.g. two-pass) 68k linker.
source:    OS-9 C Compiler manual, "Using and Linking to User Defined Libraries", p. B-5
--- END ---

## BASIC09 module packing & linking

--- CARD ---
id:        basic09-module-linking-and-packing
type:      CONCEPT
target:    all
verify:    from-manual
topic:     modules
claim:     BASIC09 can link to external procedures stored as standard OS-9 memory modules (in ROM or RAM): if a called procedure isn't found in the workspace, BASIC09 asks OS-9 to link to an external module of that name, falling back to loading it from a file if the link fails. The RUN statement's resolution order is: search the workspace, then the current data directory, then the current execution directory, before attempting LINK/LOAD. The PACK command compiles a BASIC09 procedure into this same OS-9 module format — suitable for placement in ROM or RAM and loadable through the standard LINK mechanism — trading away in-workspace editability/debuggability for smaller, faster, IP-protected distribution.
context:   This lets BASIC09 library procedures (written in BASIC09 or machine language) be shared by multiple users/processes exactly like any other reentrant OS-9 module.
source:    BASIC09 Reference Manual (Rev H), "RUN command" p.3-7, "PACK command" p.3-6; "OS-9 BASIC User Manual (Revision G, 1991)" ch.5 "RUN" command p.76-77
--- END ---

## Module inspection utilities

--- CARD ---
id:        module-inspection-utilities
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     The `mdir` command displays all modules currently in memory; `mdir -e` additionally shows address, size, owner, permissions, type, revision, edition, link count, and name for each. The `ident` utility checks the validity of modules (and can also examine S-record files); `binex`/`exbin` can likewise check module validity as a side effect of format conversion.
context:   Useful first-response tools when a module fails to load or behaves unexpectedly, before reaching for `dump` or `fixmod`.
source:    The OS-9 Primer ch.5 "Module Header"; Using Professional OS-9 v2.4, "The OS-9 Utilities", "binex/exbin section" p.7-8
--- END ---
