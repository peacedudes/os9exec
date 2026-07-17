--- CARD ---
id:        attr-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     attr
claim:     Syntax takes a path argument, followed by optional permission abbreviations (single characters or two-char pairs like pr/pw/pe for public read/write/execute). Permissions preceded by minus turn off; absence means no change. No arguments displays current attributes.
context:   Gimix 1983 manual, page 7-2; Level 1 command documented.
source:    Gimix OS-9 Users Manual 1983, ATTR section, line 2773
--- END ---

--- CARD ---
id:        backup-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     backup
claim:     Syntax accepts optional flags e, s, and -v; optional source and destination device names (default /do and /dl). Flag 'e' exits on read error, 's' prints single-drive prompt, '-v' skips verification. Memory allocator flag also mentioned as making backup faster.
context:   Gimix 1983 manual, page 7-3; docstring mentions "inK" (OCR unclear) as memory flag. Level 1 command.
source:    Gimix OS-9 Users Manual 1983, BACKUP section, lines 2817-2830
--- END ---

--- CARD ---
id:        binex-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     binex
claim:     Converts binary file (path1) to Motorola S-Record text format (path2). Prompts user for starting load address and program header name during conversion.
context:   Gimix 1983 manual, page 7-5; binary-to-srecord utility. Level 1 command.
source:    Gimix OS-9 Users Manual 1983, BINEX section, line 2894
--- END ---

--- CARD ---
id:        exbin-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     exbin
claim:     Inverse of BINEX; converts S-Record text file (path1) to pure binary (path2). Does not generate or validate OS-9 module headers or CRC values.
context:   Gimix 1983 manual, page 7-5; S-Record-to-binary utility. Level 1 command.
source:    Gimix OS-9 Users Manual 1983, EXBIN section, line 2895
--- END ---

--- CARD ---
id:        build-level1-syntax
type:      GOTCHA
target:    6809-level1
verify:    from-manual
topic:     build
claim:     Syntax documented as "BUILD <oath>" – OCR error for "path". Builds text file from stdin via interactive line-by-line prompting. Empty line (carriage return only) terminates.
context:   Gimix 1983 manual, page 7-6; OCR garble in original ("oath" vs "path"). Level 1 command.
source:    Gimix OS-9 Users Manual 1983, BUILD section, line 2941
--- END ---

--- CARD ---
id:        chd-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     chd
claim:     Changes working data directory. Syntax: chd <pathlist>. Described as shell built-in command (does not reside in CMDS directory).
context:   Gimix 1983 manual, page 7-7; built-in shell command. Level 1 command.
source:    Gimix OS-9 Users Manual 1983, CHD section, line 2979
--- END ---

--- CARD ---
id:        chx-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     chx
claim:     Changes working execution directory. Syntax: chx <pathlist>. Described as shell built-in command (does not reside in CMDS directory).
context:   Gimix 1983 manual, page 7-7; built-in shell command. Level 1 command.
source:    Gimix OS-9 Users Manual 1983, CHX section, line 2980
--- END ---

--- CARD ---
id:        cmp-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     cmp
claim:     Binary file comparison. Syntax: CMP <file1> <file2>. Displays byte offset and differing byte values in hexadecimal; ends at EOF of either file. Reports total bytes compared and count of differences.
context:   Gimix 1983 manual, page 7-8; binary comparison tool. Level 1 command.
source:    Gimix OS-9 Users Manual 1983, CMP section, line 3009
--- END ---

--- CARD ---
id:        cobbler-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     cobbler
claim:     Creates OS9Boot file from currently loaded modules in memory. Syntax: COBBLER <device name>. Level 1 only; Level 2 uses OS9GEN instead. Requires freshly formatted disk due to contiguous sector requirement.
context:   Gimix 1983 manual, page 7-9; Level 1-specific. Cannot be used with Exordisk controllers below 1 MHz.
source:    Gimix OS-9 Users Manual 1983, COBBLER section, line 3048
--- END ---

--- CARD ---
id:        copy-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     copy
claim:     Copies data between paths using large block reads/writes. Syntax: COpy <path> <path> [-s]. Option -s enables single-drive copy (user swaps media). Second path auto-created if mass-storage file. No data modification in transit.
context:   Gimix 1983 manual, page 7-10; Level 1 command. "COpy" capitalization as shown.
source:    Gimix OS-9 Users Manual 1983, COPY section, line 3079
--- END ---

--- CARD ---
id:        date-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     date
claim:     Displays system date; option 't' also shows time. Syntax: DATE [t]. Output format: month name, day, year (and time if 't' given).
context:   Gimix 1983 manual, page 7-11; Level 1 command.
source:    Gimix OS-9 Users Manual 1983, DATE section, line 3121
--- END ---

--- CARD ---
id:        dcheck-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     dcheck
claim:     Disk file structure integrity checker. Syntax: DCHECK [-opts] <devnam>. Options include -w= (work directory), -p (print questionable cluster paths), -m (save work files), -b (suppress unused cluster listing), -s (file/directory count only), -o (show valid options).
context:   Gimix 1983 manual, pages 7-12 to 7-15; Level 1 command. Creates bitmap work files in specified directory. Cannot handle >39 directory levels.
source:    Gimix OS-9 Users Manual 1983, DCHECK section, line 3147
--- END ---

--- CARD ---
id:        del-level1-syntax
type:      GOTCHA
target:    6809-level1
verify:    from-manual
topic:     del
claim:     Deletes files. Syntax documented as "DEL (-x] (path> {(path>} [-x]" – OCR garbles parentheses and brackets. Option -x assumes current execution directory. Cannot delete directory files unless attr removes directory attribute first.
context:   Gimix 1983 manual, page 7-16; OCR unclear on bracket/paren distinction. Level 1 command.
source:    Gimix OS-9 Users Manual 1983, DEL section, line 3321
--- END ---

--- CARD ---
id:        deldir-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     deldir
claim:     Recursively deletes directory and all contents. Syntax: DELDIR <directory name>. Interactive prompts before deletion: show contents, delete, or quit. Aborts on first permission denial. Calls DIR and ATTR automatically.
context:   Gimix 1983 manual, page 7-17; Level 1 command. Operates recursively on nested directories.
source:    Gimix OS-9 Users Manual 1983, DELDIR section, line 3354
--- END ---

--- CARD ---
id:        dir-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     dir
claim:     Lists directory contents. Syntax: DIR [e] [x] [<path>]. Option 'e' shows full file descriptions (size, address, owner, permissions, date/time); option 'x' shows execution directory instead of data directory. Default shows current data directory.
context:   Gimix 1983 manual, page 7-18; Level 1 command. Documented with capitalization "DIR" and lowercase examples "dir".
source:    Gimix OS-9 Users Manual 1983, DIR section, line 3392
--- END ---

--- CARD ---
id:        display-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     display
claim:     Converts hexadecimal bytes to ASCII characters and outputs them. Syntax: DISPLAY <hex> {<hex>}. Used to send cursor/screen control codes to terminals. Takes one or more hex values as parameters.
context:   Gimix 1983 manual, page 7-19; Level 1 command.
source:    Gimix OS-9 Users Manual 1983, DISPLAY section, line 3422
--- END ---

--- CARD ---
id:        dsave-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     dsave
claim:     Generates shell procedure file (not direct action) containing copy commands to backup directory tree. Syntax: dsave [-opts] [<devname>] [<path>]. Options: -b (add OS9Boot), -b=<path> (specify boot source), -i (indent), -L (non-recursive), -m (no makdir commands), -s<int> (copy size in K).
context:   Gimix 1983 manual, page 7-20; Level 1 command. Recursive by default; outputs procedure file to stdout for later execution.
source:    Gimix OS-9 Users Manual 1983, DSAVE section, line 3443
--- END ---

--- CARD ---
id:        dump-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     dump
claim:     Formatted hex/ASCII dump of binary file contents. Syntax: DUMP [<path>]. Displays 16 bytes per line; non-printable bytes shown as periods. Addresses relative to file start (load address for modules). Default input is stdin.
context:   Gimix 1983 manual, page 7-21; Level 1 command.
source:    Gimix OS-9 Users Manual 1983, DUMP section, line 3493
--- END ---

--- CARD ---
id:        echo-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     echo
claim:     Echoes argument text to stdout. Syntax: ECHO <text>. Text must not include shell punctuation. Used for messages in procedure files or terminal initialization sequences.
context:   Gimix 1983 manual, page 7-22; Level 1 command.
source:    Gimix OS-9 Users Manual 1983, ECHO section, line 3535
--- END ---

--- CARD ---
id:        ex-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     ex
claim:     Replaces shell process with another program (overlay, no new process). Syntax: EX <module name> [<modifiers>] [<parameters>]. Built-in shell command; any following commands on same line are ignored. Conserves memory.
context:   Gimix 1983 manual, page 7-23; built-in shell command. Level 1 command.
source:    Gimix OS-9 Users Manual 1983, EX section, line 3562
--- END ---

--- CARD ---
id:        format-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     format
claim:     Initializes disk media and creates file structure. Syntax: FORMAT <devname> [<option list>]. Floppy disk options: S/D (single/double density, default S), 1/2 (sided, default 1). Hard disk options: R (inhibit ready prompt), number (track count), :number: (sector interleave), "name" (disk label, max 32 chars).
context:   Gimix 1983 manual, pages 7-24 to 7-25; Level 1 command. Verifies each sector; marks defective sectors for exclusion.
source:    Gimix OS-9 Users Manual 1983, FORMAT section, line 3592
--- END ---

--- CARD ---
id:        free-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     free
claim:     Displays free space on mass-storage device. Syntax: FREE <devname>. Reports unused 256-byte sectors, disk name, creation date, and cluster size. Small free space may not support proportionally many new files due to cluster allocation.
context:   Gimix 1983 manual, page 7-26; Level 1 command.
source:    Gimix OS-9 Users Manual 1983, FREE section, line 3678
--- END ---

--- CARD ---
id:        ident-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     ident
claim:     Displays OS-9 module headers and metadata. Syntax: IDENT [-opts] <path> [-opts]. Options: -m (path is module in memory), -v (skip CRC verification), -x (pathlist starts in execution directory), -s (single-line format showing edition, type/language, CRC, mark, module name).
context:   Gimix 1983 manual, pages 7-27 to 7-28; Level 1 command. Shows module size, CRC with verification, execution offset, permanent storage requirement, type/language, attribute/revision bytes.
source:    Gimix OS-9 Users Manual 1983, IDENT section, line 3713
--- END ---

--- CARD ---
id:        kill-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     kill
claim:     Sends abort signal to a process. Syntax: KILL <procID>. Built-in shell command. Only user who started process (or superuser) can kill it. Process waiting for I/O may not die immediately.
context:   Gimix 1983 manual, page 7-29; built-in shell command. Level 1 command.
source:    Gimix OS-9 Users Manual 1983, KILL section, line 3800
--- END ---

--- CARD ---
id:        link-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     link
claim:     Locks previously loaded module into memory. Syntax: LINK <memory module name>. Increments module link count; paired with UNLINK to release. On Level 2, maps system-memory module into user address space.
context:   Gimix 1983 manual, page 7-30; Level 1 command. Different semantics between Level 1 and Level 2.
source:    Gimix OS-9 Users Manual 1983, LINK section, line 3844
--- END ---

--- CARD ---
id:        list-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     list
claim:     Lists text file contents to stdout. Syntax: LIST <path> {<path>}. Multiple input files concatenated in order. Terminates at EOF of last file. No output line editing (line-feed not auto-added).
context:   Gimix 1983 manual, page 7-31; Level 1 command.
source:    Gimix OS-9 Users Manual 1983, LIST section, line 3871
--- END ---

--- CARD ---
id:        load-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     load
claim:     Loads one or more modules from file into memory. Syntax: LOAD <path>. Module names added to module directory. If duplicate name/type exists, highest revision level is kept. All loads assign non-overlapping addresses.
context:   Gimix 1983 manual, page 7-32; Level 1 command.
source:    Gimix OS-9 Users Manual 1983, LOAD section, line 3914
--- END ---

--- CARD ---
id:        login-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     login
claim:     Timesharing system login utility. Syntax: LOGIN (no arguments). Prompts for username and password; validates against /dO/SYS/PASSWORD file. Initializes user ID, priority, working directories, and runs initial program (usually SHELL). Allows 3 login attempts before abort.
context:   Gimix 1983 manual, pages 7-33 to 7-34; Level 1 timesharing feature. Password file format: username, password, user ID, priority, exec dir, data dir, initial program.
source:    Gimix OS-9 Users Manual 1983, LOGIN section, line 3954
--- END ---

--- CARD ---
id:        makdir-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     makdir
claim:     Creates new directory file. Syntax: MAKDIR <path>. Pathlist must refer to parent directory with write permission. New directory initially empty except for "." and ".." pointers. Customary to capitalize directory names.
context:   Gimix 1983 manual, page 7-35; Level 1 command.
source:    Gimix OS-9 Users Manual 1983, MAKDIR section, line 4041
--- END ---

--- CARD ---
id:        mdir-level1-syntax
type:      GOTCHA
target:    6809-level1
verify:    from-manual
topic:     mdir
claim:     Displays module directory (resident modules in memory). Syntax documented as "HDIR [e]" – OCR misread MDIR as HDIR. Option 'e' shows extended info (address, size, type, revision, user count) in hexadecimal. Level 2 adds extended physical address (block number, offset).
context:   Gimix 1983 manual, page 7-36; OCR error. Level 1 command. Not all listed modules are executable processes.
source:    Gimix OS-9 Users Manual 1983, MDIR section, line 4068
--- END ---

--- CARD ---
id:        merge-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     merge
claim:     Concatenates multiple input files to stdout. Syntax: MERGE <path> { <path> }. Files copied in order given; no output line editing. Typically redirected to file or device. Combines file contents without modification.
context:   Gimix 1983 manual, page 7-37; Level 1 command.
source:    Gimix OS-9 Users Manual 1983, MERGE section, line 4100
--- END ---

--- CARD ---
id:        mfree-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     mfree
claim:     Displays free RAM memory blocks. Syntax: MFREE (no arguments). Level 1: shows address and size (in 256-byte pages) of each free block. Level 2: shows block number, extended physical addresses, size in blocks and K bytes (used to detect fragmentation).
context:   Gimix 1983 manual, pages 7-38; Level 1 command. Different output format between Level 1 and Level 2. Level 1 size in pages; Level 2 size in blocks and KB.
source:    Gimix OS-9 Users Manual 1983, MFREE section, line 4123
--- END ---

--- CARD ---
id:        os9gen-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     os9gen
claim:     Creates/links OS9Boot file for bootstrapping. Syntax: OS9GEN <device name>. Reads file names (pathlists) from stdin, one per line, until EOF or blank line. Copies each file to temporary "TempBoot", then renames to "OS9Boot" and links address/size into Identification Sector (LSN 0).
context:   Gimix 1983 manual, pages 7-39 to 7-40; Level 1 command; Level 2 mandatory method. Boot file must be contiguous; normally used on freshly formatted disks.
source:    Gimix OS-9 Users Manual 1983, OS9GEN section, line 4170
--- END ---

--- CARD ---
id:        printerr-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     printerr
claim:     Installs system-wide textual error message printer. Syntax: PRINTERR (no arguments). Replaces basic F$PERR routine. Reads messages from /dO/SYS/errmsg file. Once loaded, cannot be unlinked; uses current user stack for I/O buffer.
context:   Gimix 1983 manual, page 7-41; Level 1 command. Permanent installation; error message file format: error number (ASCII) + delimiter + text. Multi-line messages supported.
source:    Gimix OS-9 Users Manual 1983, PRINTERR section, line 4257
--- END ---

--- CARD ---
id:        procs-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     procs
claim:     Displays running processes. Syntax: PROeS [e]. Default shows current user's processes only; 'e' option shows all users' processes. Level 1 output: user ID, process ID, priority, state, memory (256-byte pages), primary module, stdin path. Level 2 output adds parent process ID and stack pointer.
context:   Gimix 1983 manual, pages 7-42; OCR typo "PROeS" for PROCS. Levels 1 and 2 differ in output format; snapshot taken at execution time.
source:    Gimix OS-9 Users Manual 1983, PROCS section, line 4290
--- END ---

--- CARD ---
id:        pwd-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     pwd
claim:     Prints path from root to current data directory. Syntax: PWD (no arguments, documented as "PVJD" – OCR error). Shows full pathlist of working data directory. Used by programs to discover file location or by users navigating file system.
context:   Gimix 1983 manual, page 7-43; OCR garble "PVJD" vs "PWD". Level 1 command.
source:    Gimix OS-9 Users Manual 1983, PWD section, line 4342
--- END ---

--- CARD ---
id:        pxd-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     pxd
claim:     Prints path from root to current execution directory. Syntax: PXD (no arguments). Companion to PWD; shows working execution directory instead of data directory.
context:   Gimix 1983 manual, page 7-43; Level 1 command.
source:    Gimix OS-9 Users Manual 1983, PXD section, line 4343
--- END ---

--- CARD ---
id:        rename-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     rename
claim:     Changes file name. Syntax: RENAME <path> <neVI name> (OCR shows "neVI" for "new"). User must have write permission for file. Cannot rename devices or directory pointers "." or "..".
context:   Gimix 1983 manual, page 7-44; OCR garble "neVI" vs "new name". Level 1 command.
source:    Gimix OS-9 Users Manual 1983, RENAME section, line 4374
--- END ---

--- CARD ---
id:        save-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     save
claim:     Saves memory module(s) to disk file. Syntax: SAVE <path> <modname> {<modname>}. Creates new file with all-permissions-except-public-write. Module names must exist in module directory at time of save. Default directory is current data directory for data files, execution directory for executable modules.
context:   Gimix 1983 manual, page 7-45; Level 1 command.
source:    Gimix OS-9 Users Manual 1983, SAVE section, line 4404
--- END ---

--- CARD ---
id:        setime-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     setime
claim:     Sets system date/time and activates real-time clock. Syntax: SETIME [y,m,d,h,m,s]. Parameters optional; no args triggers interactive prompt. Accepts space, colon, semicolon, or slash as delimiters. Uses 24-hour time format. Mandatory before multitasking; run for date-only on battery-backed clocks.
context:   Gimix 1983 manual, page 7-46; Level 1 command. Documented with non-standard syntax showing parameters as [y,m,d,h,m,s] separated by commas or delimiters.
source:    Gimix OS-9 Users Manual 1983, SETIME section, line 4427
--- END ---

--- CARD ---
id:        setpr-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     setpr
claim:     Changes process CPU priority. Syntax: SETPR <procID> <number>. Priority range 1 (lowest) to 255. Built-in shell command; only user who owns process (or superuser) can change priority. Documented as not appearing in CMDS directory.
context:   Gimix 1983 manual, page 7-47; built-in shell command. Level 1 command.
source:    Gimix OS-9 Users Manual 1983, SETPR section, line 4458
--- END ---

--- CARD ---
id:        sleep-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     sleep
claim:     Suspends process for duration (in clock ticks). Syntax: StEEP <tick count> (OCR capitalization "StEEP" vs "SLEEP"). Tick count 1 yields CPU slice; tick count 0 sleeps indefinitely (awaiting signal). Tick duration system-dependent: ~100 ms Level 1, ~10 ms Level 2.
context:   Gimix 1983 manual, page 7-48; OCR mixed-case typo "StEEP". Level 1 command.
source:    Gimix OS-9 Users Manual 1983, SLEEP section, line 4495
--- END ---

--- CARD ---
id:        shell-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     shell
claim:     OS-9 command interpreter. Syntax: SHELL <arglist>. Reads stdin line-by-line as command sequence; terminates at EOF. If called from program, takes parameter area as first input line. Built-in commands (chd, chx, kill, setpr, ex, w, p, -p, t, -t, -x, x, *comment) processed before program load.
context:   Gimix 1983 manual, pages 7-49 to 7-50; Level 1 command. Shell syntax formally defined with BNF-like notation; filters options before passing to executed programs.
source:    Gimix OS-9 Users Manual 1983, SHELL section, line 4517
--- END ---

--- CARD ---
id:        tee-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     tee
claim:     Copies stdin to stdout and multiple output paths. Syntax: Tee {<path>}. Filter for pipelines; broadcasts input to multiple destinations simultaneously. Commonly used to send output to terminal, printer, and disk file at once.
context:   Gimix 1983 manual, page 7-51; Level 1 command (filter utility).
source:    Gimix OS-9 Users Manual 1983, TEE section, line 4599
--- END ---

--- CARD ---
id:        tmode-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     tmode
claim:     Configures terminal operating parameters. Syntax: TMODE [.<pathnum>] [<arglist>]. Optional path number (0/1/2) specifies target; default stdin. Flags: upc/-upc, bsb/-bsb, bsl/-bsl, echo/-echo, lf/-lf, pause/-pause. Numeric parameters: null=n, pag=n, bsp=h, bse=h, del=h, bell=h, eor=h, eof=h, type=h, reprint=h, dup=h, psc=h, abort=h, quit=h, xon=h, xoff=h.
context:   Gimix 1983 manual, pages 7-52 to 7-54; Level 1 command. Device-descriptor permanence requires modification of descriptor, not TMODE. SCFMAN-type devices only.
source:    Gimix OS-9 Users Manual 1983, TMODE section, line 4623
--- END ---

--- CARD ---
id:        tsmon-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     tsmon
claim:     Timesharing monitor for unattended terminal supervision. Syntax: TSMON [<pathlist>]. Monitors idle terminals; calls LOGIN on carriage return. Returns to TSMON on login failure. Requires LOGIN command and /dO/SYS/PASSWORD file.
context:   Gimix 1983 manual, page 7-55; Level 1 timesharing feature.
source:    Gimix OS-9 Users Manual 1983, TSMON section, line 4744
--- END ---

--- CARD ---
id:        unlink-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     unlink
claim:     Releases (unlinks) memory module(s). Syntax: UNLINK <modname> { <modname>}. Module may/may not be destroyed depending on use by other processes, residency (ROM/RAM), etc. Good practice to unlink when done to avoid fragmentation. Warning: never unlink modules you did not load or link.
context:   Gimix 1983 manual, page 7-56; Level 1 command.
source:    Gimix OS-9 Users Manual 1983, UNLINK section, line 4777
--- END ---

--- CARD ---
id:        verify-level1-syntax
type:      FACT
target:    6809-level1
verify:    from-manual
topic:     verify
claim:     Verifies/updates module header parity and CRC. Syntax: VERIFY [U]. Reads modules from stdin; reports to stderr. Option U (update) recomputes and writes correct parity/CRC to stdout; no option means verification only (no output copy).
context:   Gimix 1983 manual, page 7-57; Level 1 command. Used with redirected I/O to check/fix module validity.
source:    Gimix OS-9 Users Manual 1983, VERIFY section, line 4818
--- END ---
