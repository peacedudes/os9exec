--- CARD ---
id:        echo-basic-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     echo
claim:     ECHO writes its argument text to standard output (screen by default).
context:   
source:    OS-9 Level 2 Operating System Manual, ECHO section, line 7358-7361
--- END ---

--- CARD ---
id:        echo-text-restrictions
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     echo
claim:     Text argument should not include punctuation characters that the shell interprets as special syntax.
context:   Allows safe message generation in shell scripts; some punctuation may cause parsing errors.
source:    OS-9 Level 2 Operating System Manual, ECHO section, line 7374-7375
--- END ---

--- CARD ---
id:        echo-use-cases
type:      FACT
target:    6809
verify:    from-manual
topic:     echo
claim:     ECHO is used to generate messages in shell procedure files or send initialization character sequences to terminals.
context:   
source:    OS-9 Level 2 Operating System Manual, ECHO section, line 7372-7373
--- END ---

--- CARD ---
id:        echo-redirection
type:      FACT
target:    6809
verify:    from-manual
topic:     echo
claim:     ECHO output can be redirected to other devices (printer, terminal, etc.) using standard output redirection.
context:   Examples shown include >/p (to printer) and >/term (to specific terminal).
source:    OS-9 Level 2 Operating System Manual, ECHO section, line 7395-7407
--- END ---

--- CARD ---
id:        error-basic-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     error
claim:     ERROR takes one or more error code numbers as arguments and displays the corresponding text messages.
context:   Multiple error codes can be specified on one command line.
source:    OS-9 Level 2 Operating System Manual, ERROR section, line 7416
--- END ---

--- CARD ---
id:        error-code-range
type:      FACT
target:    6809
verify:    from-manual
topic:     error
claim:     ERROR accepts error numbers in the range 1 through 255.
context:   
source:    OS-9 Level 2 Operating System Manual, ERROR section, line 7426
--- END ---

--- CARD ---
id:        error-errmsg-location
type:      FACT
target:    6809
verify:    from-manual
topic:     error
claim:     ERROR retrieves error descriptions from a file called Errmsg located in the SYS directory.
context:   The file is arranged to provide rapid lookup of error codes.
source:    OS-9 Level 2 Operating System Manual, ERROR section, line 7433-7440
--- END ---

--- CARD ---
id:        format-basic-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     format
claim:     FORMAT initializes and verifies file structure on a disk device (floppy or hard disk).
context:   Requires devname (device name/path) and optional parameters for formatting options and disk name.
source:    OS-9 Level 2 Operating System Manual, FORMAT section, line 7532
--- END ---

--- CARD ---
id:        format-disk-name-parameter
type:      FACT
target:    6809
verify:    from-manual
topic:     format
claim:     FORMAT can accept a disk volume name (up to 32 characters, can include spaces and punctuation) enclosed in double quotes.
context:   The volume name is requested during the formatting process and can be displayed later with FREE command.
source:    OS-9 Level 2 Operating System Manual, FORMAT section, line 7621-7624
--- END ---

--- CARD ---
id:        format-option-r
type:      FACT
target:    6809
verify:    from-manual
topic:     format
claim:     The -r option causes FORMAT to proceed automatically without prompting the user.
context:   
source:    OS-9 Level 2 Operating System Manual, FORMAT section, line 7556-7557
--- END ---

--- CARD ---
id:        format-option-1-2-sides
type:      FACT
target:    6809
verify:    from-manual
topic:     format
claim:     The -1 option formats a single-sided diskette; the -2 option formats a double-sided diskette.
context:   Use -1 with single-sided drives or single-sided media in double-sided drives; use -2 with double-sided drives and media.
source:    OS-9 Level 2 Operating System Manual, FORMAT section, line 7559-7564
--- END ---

--- CARD ---
id:        format-option-cylinders
type:      FACT
target:    6809
verify:    from-manual
topic:     format
claim:     An option specifying the number of cylinders (in decimal) determines how many cylinders to format on the disk.
context:   OCR unclear: manual shows "'cylinders'" with unusual quoting; interpreting as a placeholder for a numeric argument.
source:    OS-9 Level 2 Operating System Manual, FORMAT section, line 7566-7567
--- END ---

--- CARD ---
id:        format-option-interleave
type:      FACT
target:    6809
verify:    from-manual
topic:     format
claim:     An option specifying sector interleave value (in decimal) controls the interleave pattern for disk sectors.
context:   OCR unclear: manual shows "interleave:" with trailing colon; interpreting as option name. Syntax not fully clear.
source:    OS-9 Level 2 Operating System Manual, FORMAT section, line 7569-7570
--- END ---

--- CARD ---
id:        format-write-protect-error
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     format
claim:     A write-protected disk generates OS-9 error code 242 and prevents formatting; the system returns to the OS-9 prompt.
context:   Must ensure disk is NOT write-protected before attempting FORMAT.
source:    OS-9 Level 2 Operating System Manual, FORMAT section, line 7583-7586
--- END ---

--- CARD ---
id:        format-hard-disk-screen-pause
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     format
claim:     When formatting a hard disk, disable screen pause with "tmode -pause" before FORMAT; re-enable after with "tmode pause".
context:   Without this, FORMAT stops when the screen fills during sector verification; pressing space bar continues the process.
source:    OS-9 Level 2 Operating System Manual, FORMAT section, line 7588-7603
--- END ---

--- CARD ---
id:        format-three-phase-process
type:      FACT
target:    6809
verify:    from-manual
topic:     format
claim:     FORMAT performs three steps: (1) physically initialize and divide disk into sectors, (2) read and verify each sector (excluding defective ones), (3) write allocation map, root directory, and identification sector to first few Track 0 sectors.
context:   Track 0 sectors must not be defective for this step to succeed.
source:    OS-9 Level 2 Operating System Manual, FORMAT section, line 7605-7619
--- END ---

--- CARD ---
id:        free-basic-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     free
claim:     FREE displays the number of unused 256-byte sectors available on a specified disk drive.
context:   
source:    OS-9 Level 2 Operating System Manual, FREE section, line 7675-7680
--- END ---

--- CARD ---
id:        free-parameter-drive
type:      FACT
target:    6809
verify:    from-manual
topic:     free
claim:     FREE requires a drive parameter specifying which disk drive to query for free space.
context:   If no drive is specified, FREE queries the disk containing the current data directory.
source:    OS-9 Level 2 Operating System Manual, FREE section, line 7694-7697
--- END ---

--- CARD ---
id:        free-display-includes-metadata
type:      FACT
target:    6809
verify:    from-manual
topic:     free
claim:     FREE output displays the disk name, creation date, cluster size, number of free sectors, and size of the largest contiguous free block.
context:   
source:    OS-9 Level 2 Operating System Manual, FREE section, line 7694-7697
--- END ---

--- CARD ---
id:        free-coco-cluster-size
type:      FACT
target:    6809
verify:    from-manual
topic:     free
claim:     On the Color Computer, cluster size is always one sector.
context:   
source:    OS-9 Level 2 Operating System Manual, FREE section, line 7699-7700
--- END ---

--- CARD ---
id:        help-basic-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     help
claim:     HELP displays the use and syntax information for OS-9 commands, taking one or more command names as arguments.
context:   
source:    OS-9 Level 2 Operating System Manual, HELP section, line 7749-7758
--- END ---

--- CARD ---
id:        help-helpmsg-file
type:      FACT
target:    6809
verify:    from-manual
topic:     help
claim:     HELP retrieves information from a file named Helpmsg located in the SYS directory.
context:   
source:    OS-9 Level 2 Operating System Manual, HELP section, line 7764-7765
--- END ---

--- CARD ---
id:        help-unknown-command
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     help
claim:     If HELP is asked for a command not in the Helpmsg file, it displays an error message stating "Help not available" for that command.
context:   No special flag or exit code is noted; the command continues with remaining arguments.
source:    OS-9 Level 2 Operating System Manual, HELP section, line 7782-7788
--- END ---

--- CARD ---
id:        ident-basic-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     ident
claim:     IDENT displays header information for memory modules, taking a module/file name and optional options as arguments.
context:   
source:    OS-9 Level 2 Operating System Manual, IDENT section, line 7807
--- END ---

--- CARD ---
id:        ident-option-m
type:      FACT
target:    6809
verify:    from-manual
topic:     ident
claim:     The -m option tells IDENT to assume the filename refers to a module already loaded in memory.
context:   
source:    OS-9 Level 2 Operating System Manual, IDENT section, line 7824-7825
--- END ---

--- CARD ---
id:        ident-option-v
type:      FACT
target:    6809
verify:    from-manual
topic:     ident
claim:     The -v option tells IDENT to skip module CRC (cyclic redundancy check) verification.
context:   By default, IDENT verifies the CRC; this option disables that check.
source:    OS-9 Level 2 Operating System Manual, IDENT section, line 7827-7828
--- END ---

--- CARD ---
id:        ident-option-x
type:      FACT
target:    6809
verify:    from-manual
topic:     ident
claim:     The -x option tells IDENT to assume the filename refers to a module in the execution directory.
context:   
source:    OS-9 Level 2 Operating System Manual, IDENT section, line 7830-7831
--- END ---

--- CARD ---
id:        ident-single-line-output
type:      FACT
target:    6809
verify:    from-manual
topic:     ident
claim:     When displaying with single-line format, IDENT shows edition byte, type/language byte, CRC, and module name on one line.
context:   A period (.) indicates the CRC verifies; a question mark (?) indicates the CRC does not verify.
source:    OS-9 Level 2 Operating System Manual, IDENT section, line 7833-7839
--- END ---

--- CARD ---
id:        ident-full-output
type:      FACT
target:    6809
verify:    from-manual
topic:     ident
claim:     IDENT displays module size, CRC bytes with verification status, and for program/device driver modules, the execution offset and permanent storage requirement bytes.
context:   
source:    OS-9 Level 2 Operating System Manual, IDENT section, line 7845-7848
--- END ---

--- CARD ---
id:        ident-type-language-interpretation
type:      FACT
target:    6809
verify:    from-manual
topic:     ident
claim:     IDENT interprets and displays the type/language byte and attribute revision byte; the edition byte (first byte after module name) is also displayed.
context:   Most Microware-supplied modules set the edition byte to indicate the module edition level.
source:    OS-9 Level 2 Operating System Manual, IDENT section, line 7858-7862
--- END ---

--- CARD ---
id:        ident-multiple-modules-in-file
type:      FACT
target:    6809
verify:    from-manual
topic:     ident
claim:     IDENT can display all modules contained within a single disk file.
context:   
source:    OS-9 Level 2 Operating System Manual, IDENT section, line 7864-7865
--- END ---

--- CARD ---
id:        iniz-basic-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     iniz
claim:     INIZ initializes one or more device drivers, taking device names as arguments.
context:   
source:    OS-9 Level 2 Operating System Manual, INIZ section, line 7959
--- END ---

--- CARD ---
id:        iniz-use-in-startup
type:      FACT
target:    6809
verify:    from-manual
topic:     iniz
claim:     INIZ can be used in the startup file or at system startup to initialize devices and allocate their static storage at the top of memory, reducing fragmentation.
context:   
source:    OS-9 Level 2 Operating System Manual, INIZ section, line 7973-7975
--- END ---

--- CARD ---
id:        iniz-attachment-and-allocation
type:      FACT
target:    6809
verify:    from-manual
topic:     iniz
claim:     INIZ attaches a device to OS-9, places its address in a new device table entry, allocates necessary memory, and calls the device driver's initialization routine.
context:   
source:    OS-9 Level 2 Operating System Manual, INIZ section, line 7977-7980
--- END ---

--- CARD ---
id:        iniz-no-reinit
type:      FACT
target:    6809
verify:    from-manual
topic:     iniz
claim:     INIZ does not reinitialize a device that has already been installed by the user or system.
context:   
source:    OS-9 Level 2 Operating System Manual, INIZ section, line 7980-7981
--- END ---

--- CARD ---
id:        iniz-non-shareable-printer
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     iniz
claim:     If changing the printer (/p) to a non-shareable (non-reentrant) device, do not initialize it with INIZ.
context:   INIZ is designed for shareable/reentrant devices; non-shareable devices require different initialization.
source:    OS-9 Level 2 Operating System Manual, INIZ section, line 7983-7984
--- END ---

--- CARD ---
id:        kill-basic-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     kill
claim:     KILL terminates a process specified by its process ID number.
context:   
source:    OS-9 Level 2 Operating System Manual, KILL section, line 8007
--- END ---

--- CARD ---
id:        kill-user-permission-restriction
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     kill
claim:     Non-superuser processes can only kill processes with the same user number; User Number 0 (superuser) can kill any process.
context:   Use PROCS command to obtain process ID numbers and user ownership.
source:    OS-9 Level 2 Operating System Manual, KILL section, line 8021-8024
--- END ---

--- CARD ---
id:        kill-io-wait-blocking
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     kill
claim:     A process waiting for I/O operations cannot be cancelled until its current I/O completes; if PROCS shows the process still exists after KILL, it is likely waiting for terminal input.
context:   This is expected behavior; the process will eventually terminate when its I/O completes.
source:    OS-9 Level 2 Operating System Manual, KILL section, line 8026-8029
--- END ---

--- CARD ---
id:        kill-builtin-command
type:      FACT
target:    6809
verify:    from-manual
topic:     kill
claim:     KILL is a built-in shell command and does not appear in the CMDS directory.
context:   
source:    OS-9 Level 2 Operating System Manual, KILL section, line 8031-8032
--- END ---

--- CARD ---
id:        link-basic-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     link
claim:     LINK locks a previously loaded module into memory, preventing it from being unloaded.
context:   
source:    OS-9 Level 2 Operating System Manual, LINK section, line 8087-8090
--- END ---

--- CARD ---
id:        link-requires-preloading
type:      FACT
target:    6809
verify:    from-manual
topic:     link
claim:     If a module is not already in memory, LOAD must be used before LINK can lock it.
context:   
source:    OS-9 Level 2 Operating System Manual, LINK section, line 8101-8102
--- END ---

--- CARD ---
id:        link-count-behavior
type:      FACT
target:    6809
verify:    from-manual
topic:     link
claim:     Each time a module is linked, its link count increases by one; use UNLINK to decrease the link count and unlock the module when no longer needed.
context:   A module remains in memory as long as its link count is greater than zero.
source:    OS-9 Level 2 Operating System Manual, LINK section, line 8103-8104
--- END ---

--- CARD ---
id:        load-basic-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     load
claim:     LOAD reads a module file from disk and loads the module(s) into memory, adding their names to the module directory.
context:   
source:    OS-9 Level 2 Operating System Manual, LOAD section, line 8222-8238
--- END ---

--- CARD ---
id:        load-revision-selection
type:      FACT
target:    6809
verify:    from-manual
topic:     load
claim:     If LOAD encounters a module with the same name and type as one already in memory but with a different revision level, it keeps the module with the highest revision.
context:   This ensures the most recent/stable version of a module is retained.
source:    OS-9 Level 2 Operating System Manual, LOAD section, line 8239-8241
--- END ---

--- CARD ---
id:        load-default-directory
type:      FACT
target:    6809
verify:    from-manual
topic:     load
claim:     If a pathlist for LOAD does not include a drive name, LOAD uses the current execution directory by default.
context:   To load from a different directory, specify a full pathlist beginning with a drive name if applicable.
source:    OS-9 Level 2 Operating System Manual, LOAD section, line 8243-8247
--- END ---

--- CARD ---
id:        makdir-basic-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     makdir
claim:     MAKDIR creates a new directory according to the specified pathname or directory name.
context:   
source:    OS-9 Level 2 Operating System Manual, MAKDIR section, line 8316-8319
--- END ---

--- CARD ---
id:        makdir-write-permission
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     makdir
claim:     The user must have write permission for the parent directory to create a new directory with MAKDIR.
context:   Attempting MAKDIR without proper permission will fail.
source:    OS-9 Level 2 Operating System Manual, MAKDIR section, line 8320-8321
--- END ---

--- CARD ---
id:        makdir-initial-contents
type:      FACT
target:    6809
verify:    from-manual
topic:     makdir
claim:     A newly created directory contains only two entries: "." (current directory) and ".." (parent directory).
context:   
source:    OS-9 Level 2 Operating System Manual, MAKDIR section, line 8334-8335
--- END ---

--- CARD ---
id:        makdir-permission-default
type:      FACT
target:    6809
verify:    from-manual
topic:     makdir
claim:     MAKDIR enables all permissions on the newly created directory.
context:   The directory is created with full read, write, and execute permission.
source:    OS-9 Level 2 Operating System Manual, MAKDIR section, line 8337-8338
--- END ---

--- CARD ---
id:        makdir-naming-convention
type:      FACT
target:    6809
verify:    from-manual
topic:     makdir
claim:     OS-9 convention dictates that directory names should be capitalized.
context:   
source:    OS-9 Level 2 Operating System Manual, MAKDIR section, line 8340
--- END ---

--- CARD ---
id:        mdir-basic-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     mdir
claim:     MDIR displays the names of all modules currently resident in memory, automatically adjusting its output format for 32- or 80-column displays.
context:   
source:    OS-9 Level 2 Operating System Manual, MDIR section, line 8373-8378
--- END ---

--- CARD ---
id:        mdir-option-e
type:      FACT
target:    6809
verify:    from-manual
topic:     mdir
claim:     The -e option produces a full extended listing showing block number, offset within block, module size, type, revision level, re-entrant attribute, user count, and module name, all displayed in hexadecimal.
context:   Display adjusts for 80- or 32-column terminal width.
source:    OS-9 Level 2 Operating System Manual, MDIR section, line 8385-8390
--- END ---

--- CARD ---
id:        mdir-not-all-executable
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     mdir
claim:     Many modules displayed by MDIR are OS-9 system modules that are not executable as programs; always check the module type code before attempting to run an unfamiliar module.
context:   Running a non-executable system module will cause errors or system instability.
source:    OS-9 Level 2 Operating System Manual, MDIR section, line 8397-8400
--- END ---

--- CARD ---
id:        merge-basic-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     merge
claim:     MERGE copies files to standard output, and with output redirection can combine multiple files into one or direct them to a device like a printer.
context:   
source:    OS-9 Level 2 Operating System Manual, MERGE section, line 8568-8570
--- END ---

--- CARD ---
id:        merge-order-preservation
type:      FACT
target:    6809
verify:    from-manual
topic:     merge
claim:     MERGE copies data in the order in which filenames are specified on the command line.
context:   
source:    OS-9 Level 2 Operating System Manual, MERGE section, line 8582-8584
--- END ---

--- CARD ---
id:        merge-no-line-editing
type:      FACT
target:    6809
verify:    from-manual
topic:     merge
claim:     MERGE does not output line editing characters such as automatic line feeds; raw data is copied as-is.
context:   This differs from LIST which may insert line editing.
source:    OS-9 Level 2 Operating System Manual, MERGE section, line 8586-8587
--- END ---

--- CARD ---
id:        merge-typical-usage
type:      FACT
target:    6809
verify:    from-manual
topic:     merge
claim:     MERGE is typically used with output redirection to send combined or copied file data to another file or device.
context:   Examples include merging files into a single output file or sending multiple files to a printer.
source:    OS-9 Level 2 Operating System Manual, MERGE section, line 8589-8593
--- END ---

--- CARD ---
id:        mfree-basic-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     mfree
claim:     MFREE displays a list of free (unassigned) memory areas available in the system.
context:   Takes no arguments or options.
source:    OS-9 Level 2 Operating System Manual, MFREE section, line 8621-8625
--- END ---

--- CARD ---
id:        mfree-display-content
type:      FACT
target:    6809
verify:    from-manual
topic:     mfree
claim:     MFREE displays for each free area: block number, physical (extended) beginning and ending addresses, size in blocks, and size in kilobytes.
context:   
source:    OS-9 Level 2 Operating System Manual, MFREE section, line 8631-8634
--- END ---

--- CARD ---
id:        mfree-block-size
type:      FACT
target:    6809
verify:    from-manual
topic:     mfree
claim:     The block size is 8 kilobytes per block for memory allocation.
context:   
source:    OS-9 Level 2 Operating System Manual, MFREE section, line 8634
--- END ---

--- CARD ---
id:        mfree-mmu-contiguity
type:      FACT
target:    6809
verify:    from-manual
topic:     mfree
claim:     Free memory for user data areas does not need to be physically contiguous because the MMU (Memory Management Unit) can map scattered free blocks to be logically contiguous.
context:   This allows flexible memory allocation despite potential fragmentation.
source:    OS-9 Level 2 Operating System Manual, MFREE section, line 8635-8637
--- END ---
