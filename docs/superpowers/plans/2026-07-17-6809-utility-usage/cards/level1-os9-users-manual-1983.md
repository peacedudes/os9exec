--- CARD ---
id:        attr-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     attr
claim:     Syntax: ATTR <path> [{ <permission-abbreviations> }]. Permission abbreviations are: dq (directory), Ss (sharable), r/w/e (read/write/execute for owner), pr/pw/pe (read/write/execute for public). Prefix with minus sign to turn permission off. ATTR can change directory file to non-directory file if empty.
context:   Level 2 should have similar structure; key feature is converting directory to non-directory via ATTR alone (Level 1 specific constraint).
source:    OS-9 Users Manual 1983, ATTR section, line 3784
--- END ---

--- CARD ---
id:        backup-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     backup
claim:     Syntax: BACKUP [e] [s] [-v] [<devname> [<devname>]]. Options: E=exit on read error, S=single-drive prompt, -V=skip verify. Defaults to /d0 to /d1 if device names omitted. #nK modifier for memory size.
context:   Level 2 likely similar; OCR has some garbled option notation but meaning is clear.
source:    OS-9 Users Manual 1983, BACKUP section, line 3843
--- END ---

--- CARD ---
id:        binex-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     binex
claim:     Syntax: BINEX <path1> <path2>. Converts binary file (path1) to S-Record text format (path2). Prompts for starting load address and program name. Level 1-only utility (not in Level 2 list).
context:   BINEX converts OS-9 binary to Motorola S-Record format for PROM programmers and serial transmission. Companion to EXBIN.
source:    OS-9 Users Manual 1983, BINEX section, line 3944
--- END ---

--- CARD ---
id:        exbin-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     exbin
claim:     Syntax: EXBIN <path2> <path1>. Converts S-Record text file (path2) to binary form (path1). Inverse of BINEX. Does not generate OS-9 module headers or CRC; IDENT or VERIFY can check resulting modules.
context:   Level 1-only utility. Note arg order is reversed vs BINEX (path2 input, path1 output).
source:    OS-9 Users Manual 1983, BINEX section, line 3945
--- END ---

--- CARD ---
id:        build-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     build
claim:     Syntax: BUILD <path>. Copies stdin line-by-line into file at path. Displays "?" prompt for each line. Terminates on blank line (carriage return only).
context:   Simple text file builder from keyboard. Supports I/O redirection (e.g., build <mytext /T2 copies file to terminal).
source:    OS-9 Users Manual 1983, BUILD section, line 4004
--- END ---

--- CARD ---
id:        chd-chx-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     chd
claim:     Syntax: chd <pathlist> or chx <pathlist>. Built-in shell commands (not in CMDS directory). CHD changes working data directory; CHX changes working execution directory.
context:   These are shell built-ins, not external commands. No options or modifiers documented.
source:    OS-9 Users Manual 1983, CHD/CHX section, line 4058
--- END ---

--- CARD ---
id:        cmp-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     cmp
claim:     Syntax: CMP <filel> <file2>. Compares binary values of corresponding bytes. Displays file offset and byte values in hexadecimal for each difference. Ends at EOF on either file. Reports summary: bytes compared and bytes different.
context:   Binary file comparison only (no text-aware mode). Output format shows hex address and hex byte values.
source:    OS-9 Users Manual 1983, CMP section, line 4102
--- END ---

--- CARD ---
id:        cobbler-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     cobbler
claim:     Syntax: COBBLER <device-name>. Creates OS9Boot bootstrap file on specified device. Level 1-only; Level 2 uses OS9GEN instead. Requires contiguous block on disk; normally used on freshly formatted media.
context:   Level 1-only utility. Cannot be used with Exordisk controllers at speeds other than 1 MHz. Note differs from Level 2 approach (OS9GEN mandatory on Level 2).
source:    OS-9 Users Manual 1983, COBBLER section, line 4163
--- END ---

--- CARD ---
id:        copy-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     copy
claim:     Syntax: COPY <path> <path> [-s]. Copies data from first to second file (block transfers, no output processing). -s option enables single-drive copy (second path must be full pathlist).
context:   Block-based copy (not text-aware like LIST). Shell memory size modifier (#nK) can accelerate single-drive copies and reduce media swaps.
source:    OS-9 Users Manual 1983, COPY section, line 4206
--- END ---

--- CARD ---
id:        date-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     date
claim:     Syntax: DATE [t]. Displays system date; optional [t] flag adds time display. No parameters for setting date (use SETIME instead). Output format: "Month DD, YYYY" and optionally "HH:MM:SS".
context:   Display-only command. SETIME is the separate command to set date/time.
source:    OS-9 Users Manual 1983, DATE section, line 4263
--- END ---

--- CARD ---
id:        dcheck-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     dcheck
claim:     Syntax: DCHECK [-opts] <devname>. Options: -w=<path> (work file directory), -p (print pathlists), -m (save map files), -b (suppress unused clusters), -s (count only), -O (show options). Checks disk file structure integrity, builds allocation map, reports missing/duplicate clusters.
context:   Extensive options for diagnostics. Work files created as DCHECKpp0, DCHECKpp1 in specified directory (default /DO). Cannot process directory depth >39 levels.
source:    OS-9 Users Manual 1983, DCHECK section, line 4303
--- END ---

--- CARD ---
id:        del-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     del
claim:     Syntax: DEL [-x] <path> {<path>} [-x]. Deletes one or more files. -x option assumes current execution directory instead of data directory. Cannot delete directory files (must use ATTR to change type first).
context:   Multiple file arguments supported. -x flag can appear at start or end of argument list.
source:    OS-9 Users Manual 1983, DEL section, line 4521
--- END ---

--- CARD ---
id:        deldir-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     deldir
claim:     Syntax: DELDIR <directory-name>. Deletes directory and all contained files recursively. Prompts: "List directory, delete directory, or quit? (l/d/q)". Requires write permission on all files/dirs encountered. Calls DIR and ATTR internally (must be in execution directory).
context:   Level 1-only utility (convenience wrapper around DIR/DEL/ATTR). Not in Level 2 command list.
source:    OS-9 Users Manual 1983, DELDIR section, line 4573
--- END ---

--- CARD ---
id:        dir-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     dir
claim:     Syntax: DIR [e] [x] [<path>]. Displays formatted list of files in directory (data dir by default). [e] option shows full details (size, address, owner, permissions, date/time). [x] option uses execution directory instead of data directory. [<path>] argument specifies specific directory.
context:   Options can be combined (e.g., dirxe). Works on current data directory if no path given.
source:    OS-9 Users Manual 1983, DIR section, line 4625
--- END ---

--- CARD ---
id:        display-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     display
claim:     Syntax: DISPLAY <hex> {<hex>}. Reads one or more hexadecimal numbers, converts each to ASCII character, writes to stdout. Used for sending special control codes to terminals/printers.
context:   Level 1-only utility. Simple hex-to-ASCII converter (not in Level 2 list).
source:    OS-9 Users Manual 1983, DISPLAY section, line 4670
--- END ---

--- CARD ---
id:        dsave-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     dsave
claim:     Syntax: dsave [-opts] [<devname>] {[<path>]}. Generates procedure file (shell script) with copy/makdir/chd commands. Options: -b (include OS9Boot), -b=<path> (alt boot source), -i (indent per level), -L (no recursion), -M (skip makdir), -Ss<int> (set copy size in K). Default device /DO.
context:   Does NOT directly copy files; generates commands for later execution. Handles recursive directory structures. Output typically redirected to file then executed separately.
source:    OS-9 Users Manual 1983, DSAVE section, line 4699
--- END ---

--- CARD ---
id:        dump-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     dump
claim:     Syntax: DUMP [<path>]. Produces formatted hex/ASCII dump of file (16 bytes per line, relative addresses shown). If no path given, reads from stdin. Non-displayable bytes shown as periods in ASCII column.
context:   Level 1-only utility (or perhaps in Level 2, but here it is documented as standalone). Used to examine non-text file contents.
source:    OS-9 Users Manual 1983, DUMP section, line 4765
--- END ---

--- CARD ---
id:        echo-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     echo
claim:     Syntax: ECHO <text>. Echoes argument text to stdout. Text should not include shell punctuation. Commonly used in procedure files or to send initialization sequences to terminals.
context:   Simple text output command. Supports I/O redirection (e.g., echo >/T2 <text> sends to terminal T2).
source:    OS-9 Users Manual 1983, ECHO section, line 4821
--- END ---

--- CARD ---
id:        ex-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     ex
claim:     Syntax: EX <module-name> [<modifiers>] [<parameters>]. Built-in shell command (not in CMDS directory). Replaces shell with another program without creating new process. Must be last command on line (subsequent commands ignored).
context:   Level 1-only utility (shell overlay mechanism). EX differs from normal command execution by reusing shell's process memory instead of forking.
source:    OS-9 Users Manual 1983, EX section, line 4864
--- END ---

--- CARD ---
id:        format-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     format
claim:     Syntax: FORMAT <devname> [<option-list>]. Options include: density mode (default single), track density (default single-sided), R= (inhibit ready prompt), number= (track count in decimal), snumber: (sector interleave value), name= (disk name, max 32 chars). Initializes, verifies, and establishes file structure on disk media.
context:   Option notation is OCR-corrupted in places but intent is clear. Includes prompt-based option entry if not given on command line.
source:    OS-9 Users Manual 1983, FORMAT section, line 4909
--- END ---

--- CARD ---
id:        free-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     free
claim:     Syntax: FREE <devname>. Displays free space (number of unused 256-byte sectors) on mass-storage device. Also shows disk name, creation date, cluster size. Clusters are smallest allocation unit; small amounts of free space may not be divisible into full clusters.
context:   Single device argument required. Reports help users understand storage fragmentation (8-sector cluster example given).
source:    OS-9 Users Manual 1983, FREE section, line 5041
--- END ---

--- CARD ---
id:        ident-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     ident
claim:     Syntax: IDENT [-opts] <path> [-opts]. Options: -m (assume path is memory module, not file), -v (skip CRC verification), -x (pathlist in execution directory), -s (single-line output format). Displays module size, CRC (with verification), execution offset, data size, type/language, and edition byte.
context:   Can display all modules in file. Single-line mode (-s) shows: edition, type/language, CRC, status, and module name.
source:    OS-9 Users Manual 1983, IDENT section, line 5089
--- END ---

--- CARD ---
id:        kill-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     kill
claim:     Syntax: KILL <procID>. Built-in shell command (not in CMDS directory). Sends abort signal to process. Process must have same user ID as caller. Can only kill own processes except user 0 (superuser).
context:   Process ID obtained from PROCS command. Note: if process is waiting for I/O, it may not terminate until I/O completes.
source:    OS-9 Users Manual 1983, KILL section, line 5194
--- END ---

--- CARD ---
id:        link-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     link
claim:     Syntax: LINK <memory-module-name>. Locks a previously loaded module into memory. Link count of module increments by one each time LINK is called. UNLINK decrements and may release module when count reaches zero.
context:   Level 1 behavior described; Level 2 maps modules into user address space instead. Module must already be in memory (use LOAD first if needed).
source:    OS-9 Users Manual 1983, LINK section, line 5265
--- END ---

--- CARD ---
id:        list-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     list
claim:     Syntax: LIST <path> {<path>}. Copies text lines from one or more files to stdout. Terminates at EOF of last input file. Multiple file arguments processed in order.
context:   Text-aware output (unlike COPY which uses block transfers). Commonly used to display or print text files. Supports I/O redirection.
source:    OS-9 Users Manual 1983, LIST section, line 5305
--- END ---

--- CARD ---
id:        load-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     load
claim:     Syntax: LOAD <path>. Opens file and loads one or more OS-9 modules into memory. Module names added to module directory. If module with same name/type already in memory, keeps the one with highest revision level.
context:   Single path argument. Works with multi-module files. Modules must have valid OS-9 module headers.
source:    OS-9 Users Manual 1983, LOAD section, line 5361
--- END ---

--- CARD ---
id:        login-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     login
claim:     Syntax: LOGIN. No arguments. Used in timesharing systems. Prompts for user name and password (checked against /d0/SYS/PASSWORD file). Sets up user ID, priorities, directories, and executes initial program (usually SHELL). Allows three attempts; aborts after third failure. Password file has comma-delimited fields: name, password, user-ID, priority, execution-dir, data-dir, initial-program.
context:   Level 1 timesharing feature. Can be invoked via TSMON or manually. Optional motd file displayed after successful login. EX command can overlay shell with LOGIN.
source:    OS-9 Users Manual 1983, LOGIN section, line 5418
--- END ---

--- CARD ---
id:        makdir-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     makdir
claim:     Syntax: MAKDIR <path>. Creates new directory file. Pathlist must reference parent directory with write permission. New directory initialized with "." and ".." pointers. All access permissions enabled except sharable. Customary (not required) to capitalize directory names.
context:   Simple single-argument command. Creates minimal directory structure ready for use.
source:    OS-9 Users Manual 1983, MAKDIR section, line 5541
--- END ---

--- CARD ---
id:        mdir-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     mdir
claim:     Syntax: MDIR [e]. Displays module names currently resident in system module directory (in memory). Optional [e] flag shows full listing: physical address, size, type, revision level, user count (all in hexadecimal). WARNING: not all modules listed are executable as processes; check type code before executing.
context:   Level 1 memory module display. Related command in Level 2 likely similar; user count tracking present in both.
source:    OS-9 Users Manual 1983, MDIR section, line 5581
--- END ---

--- CARD ---
id:        merge-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     merge
claim:     Syntax: MERGE <path> {<path>}. Copies multiple input files to stdout in order given. No output line editing (no auto line feed). Data is combined as-is. Standard output typically redirected to file or device.
context:   Multiple file concatenation. Text-aware (unlike COPY). No buffering or formatting applied.
source:    OS-9 Users Manual 1983, MERGE section, line 5627
--- END ---

--- CARD ---
id:        mfree-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     mfree
claim:     Syntax: MFREE. No arguments. Displays list of free (unallocated) memory areas. Level 1 format: address and size in 256-byte pages. Useful for detecting memory fragmentation.
context:   Diagnostic command. Level 1 output format differs from Level 2 (which uses block numbers and extended addresses for MMU systems). Shows total pages free.
source:    OS-9 Users Manual 1983, MFREE section, line 5662
--- END ---

--- CARD ---
id:        os9gen-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     os9gen
claim:     Syntax: OS9GEN <device-name>. Reads file names (pathlists) from stdin, one per line. Copies each to "TempBoot" on specified device. End with blank line or EOF. Renames TempBoot to OS9Boot and updates boot info in disk Identification Sector (LSN 0).
context:   Creates/updates OS9Boot bootstrap file. Must be on freshly formatted disk or boot file will fragment (prints warning). All component modules required per section 6.1. On Level 1, COBBLER is simpler alternative for exact copy.
source:    OS-9 Users Manual 1983, OS9GEN section, line 5727
--- END ---

--- CARD ---
id:        printerr-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     printerr
claim:     Syntax: PRINTERR. No arguments. Loads system-wide error printer module. Replaces basic error printing with full text messages from /d0/SYS/errmsg file. One-time installation; cannot be undone. Module should not be unlinked after installation. Uses user stack for I/O buffer.
context:   Level 1-only utility. Error message file is editable text with variable-length lines. Error codes in ASCII, followed by delimiter (space or char <$20), then message text. Multi-line messages supported via leading-delimiter continuation lines.
source:    OS-9 Users Manual 1983, PRINTERR section, line 5835
--- END ---

--- CARD ---
id:        procs-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     procs
claim:     Syntax: PROCS [e]. Displays list of running processes. By default, shows only user's processes; [e] flag shows all processes (all users). Level 1 output: Usr, #, Id, pty, state, Mem (pages), Primary module, stdin-path. Level 2 output differs (includes parent ID, stack pointer, block number).
context:   Snapshot at execution time; processes change state rapidly. Process state values: active, waiting, sleeping. Memory shown in 256-byte pages.
source:    OS-9 Users Manual 1983, PROCS section, line 5879
--- END ---

--- CARD ---
id:        pwd-pxd-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     pwd
claim:     Syntax: PWD or PXD. No arguments. PWD displays pathlist from root to current data directory. PXD displays pathlist from root to current execution directory. Used to discover physical location of files or navigate file system.
context:   Simple display-only commands. Example shows "pwd" after chd, then pxd showing execution directory.
source:    OS-9 Users Manual 1983, PWD/PXD section, line 5938
--- END ---

--- CARD ---
id:        rename-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     rename
claim:     Syntax: RENAME <path> <new-name>. Renames mass-storage file. User must have write permission for the file. Cannot rename devices, ".", or ".." entries.
context:   Simple renaming utility. New name can be relative or absolute pathlist. Two arguments only.
source:    OS-9 Users Manual 1983, RENAME section, line 5978
--- END ---

--- CARD ---
id:        save-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     save
claim:     Syntax: SAVE <path> <modname> {<modname>}. Creates new file and writes copies of one or more memory modules to it. Module names must exist in module directory. New file gets all access permissions except public write. Executable modules should be saved in default execution directory.
context:   Level 1 utility. Default directory is current data directory (override with path argument). Modules must be in memory before saving.
source:    OS-9 Users Manual 1983, SAVE section, line 6018
--- END ---

--- CARD ---
id:        setime-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     setime
claim:     Syntax: SETIME [y,m,d,h,m,s]. Sets system date/time and activates real-time clock. Parameters optional; if omitted, prompts appear. Delimiters: space, colon, semicolon, or slash. Uses 24-hour clock (1520 = 3:20 PM). IMPORTANT: must run before multitasking. For battery-backed clocks, only year needed.
context:   Critical system initialization command. Parameters all decimal. Examples show various delimiter/spacing styles.
source:    OS-9 Users Manual 1983, SETIME section, line 6050
--- END ---

--- CARD ---
id:        setpr-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     setpr
claim:     Syntax: SETPR <procID> <number>. Built-in shell command (not in CMDS directory). Changes CPU priority of process. Priority is decimal 1 (lowest) to 255 (highest). Can only change processes with caller's user ID. Process ID obtained from PROCS.
context:   Only affects own processes. Priority changes take effect immediately.
source:    OS-9 Users Manual 1983, SETPR section, line 6095
--- END ---

--- CARD ---
id:        sleep-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     sleep
claim:     Syntax: SLEEP <tick-count>. Puts process to sleep for specified number of clock ticks. Tick duration is system-dependent: typically 100 ms on Level 1, 10 ms on Level 2. Tick count of 1 yields time slice; tick count of 0 sleeps indefinitely (awakened by signal).
context:   Level 1-only utility (not in Level 2 list, or perhaps present but less emphasized). Used for time delays and CPU load distribution.
source:    OS-9 Users Manual 1983, SLEEP section, line 6154
--- END ---

--- CARD ---
id:        shell-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     shell
claim:     Syntax: SHELL <arglist>. OS-9 command interpreter. Reads stdin line-by-line as command sequences. Built-in parameters: ex (overlay), chd/chx (change dir), kill/setpr (process control), W (wait), p/-p (prompt), t/-t (echo), -x/x (error handling), * (comment). Separators: ; (sequential), & (concurrent), ! (pipeline), <CR> (end-of-line).
context:   Formal grammar for shell input specified in manual. Modifiers include I/O redirection (<,>,>>), memory size (#nK). Built-in commands do not appear in CMDS directory.
source:    OS-9 Users Manual 1983, SHELL section, line 6182
--- END ---

--- CARD ---
id:        tee-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     tee
claim:     Syntax: TEE {<path>}. Filter command. Copies all text lines from stdin to stdout and to any number of additional output paths (given as parameters). Used in pipelines to send output to multiple destinations simultaneously.
context:   Level 1-only utility (or standard UNIX command present in Level 1). Works with pipelines (!) to broadcast output to multiple files/devices.
source:    OS-9 Users Manual 1983, TEE section, line 6289
--- END ---

--- CARD ---
id:        tmode-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     tmode
claim:     Syntax: TMODE [.<pathnum>] [<arglist>]. Displays or changes terminal operating parameters. Optional .<pathnum> specifies path (0/1/2 for stdin/stdout/stderr). Flags: upe/-upe, bsb/-bsb, bsl/-bsl, echo/-echo, lf/-lf, pause/-pause. Numeric parameters: null=n (null count), pag=n (page length), bsp=h (backspace char), bse=h (backspace-echo char), del=h (delete-line char), bell=h (alert char), eor=h (carriage-return char), eof=h (end-of-file char), type=h (ACIA init), reprint=h, dup=h, psc=h (pause char), abort=h (normally Ctrl-C), quit=h (normally Ctrl-Q), xon=h (normally Ctrl-Q), noff=h (normally Ctrl-S).
context:   Many terminal parameters available. No arguments displays current values. Path number required in procedure files (use .1 or .2). Changes remain in effect until path closed; permanent changes require device descriptor modification.
source:    OS-9 Users Manual 1983, TMODE section, line 6324
--- END ---

--- CARD ---
id:        tsmon-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     tsmon
claim:     Syntax: TSMON [<pathlist>]. Timesharing monitor for idle terminals. If pathlist given, opens Standard I/O paths for device. When carriage return typed, TSMON calls LOGIN command. If LOGIN fails, returns to TSMON. Requires LOGIN command and /d0/SYS/PASSWORD file to work.
context:   Level 1 timesharing feature (companion to LOGIN). Typically used with terminal device paths (e.g., TSMON /t1).
source:    OS-9 Users Manual 1983, TSMON section, line 6481
--- END ---

--- CARD ---
id:        unlink-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     unlink
claim:     Syntax: UNLINK <modname> {<modname>}. Tells OS-9 modules are no longer needed by user. May or may not destroy modules/reassign memory depending on use by other processes/users and residency (ROM vs RAM). Best practice to unlink when done. WARNING: never unlink a module you did not load or link to.
context:   Multiple module names supported. Decrements reference count; module released when count reaches zero (if not in use elsewhere).
source:    OS-9 Users Manual 1983, UNLINK section, line 6530
--- END ---

--- CARD ---
id:        verify-syntax-level1
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     verify
claim:     Syntax: VERIFY [U]. Verifies module header parity and CRC from stdin. Messages sent to stderr. U (update) option: copies modules to stdout with corrected header parity and CRC values, reporting whether originals matched. Without U: no stdout copy, only verification message.
context:   Can process multiple modules from file. Output reports header parity and CRC match status for each module.
source:    OS-9 Users Manual 1983, VERIFY section, line 6590
--- END ---

--- CARD ---
id:        level1-exclusive-utilities
type:      GOTCHA
target:    6809-level1
verify:    from-manual
topic:     level1-utilities
claim:     The following utilities appear in Level 1 manual but were not listed in Level 2 command set: BINEX, EXBIN, COBBLER, DISPLAY, DUMP, EX, LIST, LOGIN, PRINTERR, SAVE, SLEEP, TEE, TSMON, VERIFY. These may be Level 1-only or may have moved to other mechanisms in Level 2.
context:   Comprehensive list of potential Level 1-specific utilities extracted from manual scan. Each has been documented separately. Further cross-check against Level 2 manual recommended to confirm true Level 1-only status.
source:    OS-9 Users Manual 1983, Command Summary (page A-1), lines 6656-6753
--- END ---

--- CARD ---
id:        level1-syntax-vs-level2-variance
type:      ROSETTA
target:    6809-level1
verify:    from-manual
topic:     syntax-differences
claim:     Key potential syntax differences between Level 1 and Level 2 (to be verified against Level 2 manual): MDIR and PROCS output format differs between levels (Level 1: Usr/#/Id/pty/state/Mem/module/stdin; Level 2: Id/ParntID/UserIndex/Pty/Mem/StackPtr/module). MFREE output format differs (Level 1: address in pages; Level 2: block number/extended address). Some commands have options tuned differently (e.g., FORMAT, DCHECK). COBBLER exists only in Level 1 (OS9GEN handles it in Level 2). Shell-built-in commands (CHD, CHX, KILL, SETPR, EX) present in both but implementation may differ.
context:   Represents cross-level variance discovered during manual review. Exact Level 2 syntax must be verified against Level 2 manual to confirm. This card flags areas known to differ.
source:    OS-9 Users Manual 1983, various command descriptions and examples
--- END ---
