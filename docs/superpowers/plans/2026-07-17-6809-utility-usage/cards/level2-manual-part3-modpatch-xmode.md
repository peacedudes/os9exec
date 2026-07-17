--- CARD ---
id:        modpatch-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     modpatch
claim:     Modifies the contents of modules that are already loaded in memory, accepting a patchfile that contains single-letter commands to control the patching operation.
context:   Module patching is an in-memory operation; persistence requires saving the patched module.
source:    OS-9 Level 2 Operating System Manual, MODPATCH section, line 8674
--- END ---

--- CARD ---
id:        modpatch-flag-silent
type:      FACT
target:    6809
verify:    from-manual
topic:     modpatch
claim:     Silent mode (-s flag) suppresses the display of patchfile command lines as they execute.
context:   Default is to display commands.
source:    OS-9 Level 2 Operating System Manual, MODPATCH section, line 8690
--- END ---

--- CARD ---
id:        modpatch-flag-nowarnings
type:      FACT
target:    6809
verify:    from-manual
topic:     modpatch
claim:     The -w flag prevents display of any warnings that occur during patching.
context:   Default is to show warnings.
source:    OS-9 Level 2 Operating System Manual, MODPATCH section, line 8693
--- END ---

--- CARD ---
id:        modpatch-flag-compareonly
type:      FACT
target:    6809
verify:    from-manual
topic:     modpatch
claim:     Compare-only mode (-c flag) performs patch validation without actually modifying the module.
context:   Allows verification before committing changes.
source:    OS-9 Level 2 Operating System Manual, MODPATCH section, line 8695
--- END ---

--- CARD ---
id:        modpatch-cmd-link
type:      FACT
target:    6809
verify:    from-manual
topic:     modpatch
claim:     The link command (single letter l, followed by modulename) establishes access to the named module for patching.
context:   This is the patchfile syntax, not the shell syntax.
source:    OS-9 Level 2 Operating System Manual, MODPATCH section, line 8732
--- END ---

--- CARD ---
id:        modpatch-cmd-change
type:      FACT
target:    6809
verify:    from-manual
topic:     modpatch
claim:     The change command (c, offset, origval, newval) replaces a single byte at the given offset, validating that the original value matches before replacement.
context:   Offset is relative to module start (address 0). If original value does not match, an error is displayed.
source:    OS-9 Level 2 Operating System Manual, MODPATCH section, line 8735
--- END ---

--- CARD ---
id:        modpatch-cmd-verify
type:      FACT
target:    6809
verify:    from-manual
topic:     modpatch
claim:     The verify command (v) updates the module CRC checksum and is required before saving a patched module for system load.
context:   Needed to make the patched module loadable by the system.
source:    OS-9 Level 2 Operating System Manual, MODPATCH section, line 8743
--- END ---

--- CARD ---
id:        modpatch-cmd-maskiq
type:      FACT
target:    6809
verify:    from-manual
topic:     modpatch
claim:     The mask command (m) disables interrupt requests during patching, necessary when modifying service routines.
context:   Prevents IRQs from interfering with patching of interrupt-handling code.
source:    OS-9 Level 2 Operating System Manual, MODPATCH section, line 8749
--- END ---

--- CARD ---
id:        modpatch-cmd-unmaskiq
type:      FACT
target:    6809
verify:    from-manual
topic:     modpatch
claim:     The unmask command (u) re-enables interrupt requests after patching is complete.
context:   Counterpart to the mask command.
source:    OS-9 Level 2 Operating System Manual, MODPATCH section, line 8753
--- END ---

--- CARD ---
id:        modpatch-byte-addressing
type:      FACT
target:    6809
verify:    from-manual
topic:     modpatch
claim:     Module byte addresses in patchfiles are zero-indexed offsets from the module base, not absolute memory addresses.
context:   The offset parameter refers to position within the module's loaded image.
source:    OS-9 Level 2 Operating System Manual, MODPATCH section, line 8760
--- END ---

--- CARD ---
id:        modpatch-persistence
type:      FACT
target:    6809
verify:    from-manual
topic:     modpatch
claim:     To make patched modules persistent across boots, save them using the SAVE command to the MODULES directory and either re-run OS9GEN or COBBLER to create new boot files.
context:   In-memory patches do not survive reboot.
source:    OS-9 Level 2 Operating System Manual, MODPATCH section, line 8782
--- END ---

--- CARD ---
id:        modpatch-effect-timing
type:      FACT
target:    6809
verify:    from-manual
topic:     modpatch
claim:     Patching a module in memory may not take immediate effect; the patched module's initialization must be re-run for changes to become active.
context:   Depending on when the module was initialized, reboot or re-initialization may be needed.
source:    OS-9 Level 2 Operating System Manual, MODPATCH section, line 8776
--- END ---

--- CARD ---
id:        montype-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     montype
claim:     Configures the system for the type of display hardware attached (monitor or television).
context:   CoCo/Dragon hardware-specific; affects color rendering on compatible hardware.
source:    OS-9 Level 2 Operating System Manual, MONTYPE section, line 8856
--- END ---

--- CARD ---
id:        montype-composite
type:      FACT
target:    6809
verify:    from-manual
topic:     montype
claim:     The 'c' parameter selects composite monitor or color television display mode, and is the default when MONTYPE is not used.
context:   CoCo/Dragon hardware.
source:    OS-9 Level 2 Operating System Manual, MONTYPE section, line 8866
--- END ---

--- CARD ---
id:        montype-rgb
type:      FACT
target:    6809
verify:    from-manual
topic:     montype
claim:     The 'r' parameter selects RGB monitor display mode for better color accuracy on RGB-compatible hardware.
context:   CoCo/Dragon hardware.
source:    OS-9 Level 2 Operating System Manual, MONTYPE section, line 8868
--- END ---

--- CARD ---
id:        montype-monochrome
type:      FACT
target:    6809
verify:    from-manual
topic:     montype
claim:     The 'm' parameter selects monochrome monitor or black-and-white television mode, producing a sharper image on compatible hardware.
context:   CoCo/Dragon hardware.
source:    OS-9 Level 2 Operating System Manual, MONTYPE section, line 8870
--- END ---

--- CARD ---
id:        os9gen-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     os9gen
claim:     Creates a bootable diskette by building an OS9Boot file and linking it to the target device's boot sector.
context:   Must be used on newly formatted diskettes to ensure OS9Boot remains contiguous on disk.
source:    OS-9 Level 2 Operating System Manual, OS9GEN section, line 8939
--- END ---

--- CARD ---
id:        os9gen-option-singledrive
type:      FACT
target:    6809
verify:    from-manual
topic:     os9gen
claim:     The -s option enables single-drive operation; OS9GEN prompts the user to swap diskettes between reads and writes.
context:   Reduces the number of diskette swaps compared to multi-drive setups.
source:    OS-9 Level 2 Operating System Manual, OS9GEN section, line 8956
--- END ---

--- CARD ---
id:        os9gen-option-memory
type:      FACT
target:    6809
verify:    from-manual
topic:     os9gen
claim:     The #n or #nK option reserves memory for OS9GEN's buffer; n is in 256-byte pages by default or kilobytes if K is appended.
context:   Larger buffers speed up processing and reduce diskette swaps on single-drive systems.
source:    OS-9 Level 2 Operating System Manual, OS9GEN section, line 8963
--- END ---

--- CARD ---
id:        os9gen-requirement-rename
type:      FACT
target:    6809
verify:    from-manual
topic:     os9gen
claim:     The RENAME utility must be present in the current execution directory or loaded in memory for OS9GEN to function correctly.
context:   OS9GEN uses RENAME internally to finalize the boot file.
source:    OS-9 Level 2 Operating System Manual, OS9GEN section, line 9004
--- END ---

--- CARD ---
id:        os9gen-contiguity-requirement
type:      FACT
target:    6809
verify:    from-manual
topic:     os9gen
claim:     OS9Boot files must occupy contiguous disk sectors; if fragmented, the system warns that the diskette should not be used for bootstrap.
context:   This is why OS9GEN should only be used with newly formatted diskettes.
source:    OS-9 Level 2 Operating System Manual, OS9GEN section, line 8976
--- END ---

--- CARD ---
id:        os9gen-process-workflow
type:      FACT
target:    6809
verify:    from-manual
topic:     os9gen
claim:     OS9GEN reads module filenames (either from keyboard or redirected from a file), copies each to a temporary file called Tempboot, then deletes any existing OS9Boot, renames Tempboot to OS9Boot, and records the boot file's starting address and size in the Identification Sector (LSN 0).
context:   LSN 0 contains boot metadata. OS-9 kernel will be written to Track 34.
source:    OS-9 Level 2 Operating System Manual, OS9GEN section, line 8989
--- END ---

--- CARD ---
id:        os9gen-kernel-location
type:      FACT
target:    6809
verify:    from-manual
topic:     os9gen
claim:     The OS-9 kernel is written to Track 34 of the boot diskette; if insufficient space remains, OS9GEN terminates with an error.
context:   Track numbering is hardware-dependent; specific to diskette layout.
source:    OS-9 Level 2 Operating System Manual, OS9GEN section, line 9011
--- END ---

--- CARD ---
id:        os9gen-config-alternative
type:      FACT
target:    6809
verify:    from-manual
topic:     os9gen
claim:     The CONFIG utility provides an alternative method for generating custom system diskettes, designed specifically for single- or multi-drive scenarios.
context:   CONFIG is easier to use than manual OS9GEN invocation for common tasks.
source:    OS-9 Level 2 Operating System Manual, OS9GEN section, line 9015
--- END ---

--- CARD ---
id:        procs-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     procs
claim:     Displays a snapshot list of currently executing processes on the system, automatically adapting output format for 32- or 80-column displays.
context:   List changes rapidly as processes switch states; represents an instant-in-time view.
source:    OS-9 Level 2 Operating System Manual, PROCS section, line 9138
--- END ---

--- CARD ---
id:        procs-option-all
type:      FACT
target:    6809
verify:    from-manual
topic:     procs
claim:     The 'e' option displays processes of all users; without it, PROCS shows only processes with the caller's user ID.
context:   Default behavior restricts view to current user.
source:    OS-9 Level 2 Operating System Manual, PROCS section, line 9147
--- END ---

--- CARD ---
id:        procs-output-fields
type:      FACT
target:    6809
verify:    from-manual
topic:     procs
claim:     PROCS displays user ID, process ID, priority level, process state, memory size in 256-byte pages, primary program module name, and standard input path for each process.
context:   All these fields are included in the output.
source:    OS-9 Level 2 Operating System Manual, PROCS section, line 9160
--- END ---

--- CARD ---
id:        pwd-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     pwd
claim:     Displays the complete path from the root directory to the process's current data directory.
context:   Shows where the process is currently located for file operations.
source:    OS-9 Level 2 Operating System Manual, PWD section, line 9229
--- END ---

--- CARD ---
id:        pxd-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     pxd
claim:     Displays the complete path from the root directory to the process's current execution directory.
context:   Shows where the process searches for executable programs.
source:    OS-9 Level 2 Operating System Manual, PXD section, line 9230
--- END ---

--- CARD ---
id:        pwd-pxd-separate-directories
type:      FACT
target:    6809
verify:    from-manual
topic:     pwd
claim:     OS-9 maintains separate current data directory and current execution directory for each process, allowing independent navigation of data and program locations.
context:   This is fundamental to OS-9's path management.
source:    OS-9 Level 2 Operating System Manual, PWD section, line 9237
--- END ---

--- CARD ---
id:        rename-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     rename
claim:     Changes the name of a file or directory from the old name to a new name.
context:   Simple file/directory renaming utility.
source:    OS-9 Level 2 Operating System Manual, RENAME section, line 9312
--- END ---

--- CARD ---
id:        rename-permission-requirement
type:      FACT
target:    6809
verify:    from-manual
topic:     rename
claim:     The user must have write permission for the file or directory being renamed.
context:   Write permission is enforced before rename is allowed.
source:    OS-9 Level 2 Operating System Manual, RENAME section, line 9323
--- END ---

--- CARD ---
id:        setime-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     setime
claim:     Sets the system date and time and activates the real-time clock.
context:   Once set, the clock maintains time across multitasking operations.
source:    OS-9 Level 2 Operating System Manual, SETIME section, line 9378
--- END ---

--- CARD ---
id:        setime-parameter-year
type:      FACT
target:    6809
verify:    from-manual
topic:     setime
claim:     Year parameter is specified as a two-digit decimal value (e.g., 86 for 1986).
context:   Y2K issues inherent to this time format.
source:    OS-9 Level 2 Operating System Manual, SETIME section, line 9385
--- END ---

--- CARD ---
id:        setime-parameter-month
type:      FACT
target:    6809
verify:    from-manual
topic:     setime
claim:     Month parameter is a one- or two-digit decimal value (1-12), where 01 or 1 is January and 12 is December.
context:   Flexible digit count for user convenience.
source:    OS-9 Level 2 Operating System Manual, SETIME section, line 9387
--- END ---

--- CARD ---
id:        setime-parameter-day
type:      FACT
target:    6809
verify:    from-manual
topic:     setime
claim:     Day parameter is a one- or two-digit decimal value (1-31) representing the day of the month.
context:   Flexible digit count.
source:    OS-9 Level 2 Operating System Manual, SETIME section, line 9390
--- END ---

--- CARD ---
id:        setime-parameter-hour
type:      FACT
target:    6809
verify:    from-manual
topic:     setime
claim:     Hour parameter is specified in 24-hour format as a one- or two-digit decimal value (00-23, e.g., 15 for 3 p.m.).
context:   Military time format.
source:    OS-9 Level 2 Operating System Manual, SETIME section, line 9393
--- END ---

--- CARD ---
id:        setime-parameter-minute
type:      FACT
target:    6809
verify:    from-manual
topic:     setime
claim:     Minute parameter is a one- or two-digit decimal value (00-59).
context:   Flexible digit count.
source:    OS-9 Level 2 Operating System Manual, SETIME section, line 9396
--- END ---

--- CARD ---
id:        setime-parameter-second
type:      FACT
target:    6809
verify:    from-manual
topic:     setime
claim:     Second parameter is optional; when omitted, the system uses a default value. When specified, it is a one- or two-digit decimal value (00-59).
context:   Seconds can be left off to simplify command entry.
source:    OS-9 Level 2 Operating System Manual, SETIME section, line 9407
--- END ---

--- CARD ---
id:        setime-delimiters
type:      FACT
target:    6809
verify:    from-manual
topic:     setime
claim:     Date and time parameters can be separated by space, colon, semicolon, or slash.
context:   Multiple delimiter styles are supported for user flexibility.
source:    OS-9 Level 2 Operating System Manual, SETIME section, line 9417
--- END ---

--- CARD ---
id:        setime-cc3go-autostart
type:      FACT
target:    6809
verify:    from-manual
topic:     setime
claim:     The CC3go module starts the system clock during boot, enabling multitasking without requiring explicit SETIME invocation.
context:   Automatic clock initialization happens at boot if CC3go is present.
source:    OS-9 Level 2 Operating System Manual, SETIME section, line 9420
--- END ---

--- CARD ---
id:        setime-file-timestamps
type:      FACT
target:    6809
verify:    from-manual
topic:     setime
claim:     If date and time are not set during boot, the system cannot accurately record "Last modified" timestamps for files.
context:   File timestamp integrity depends on correct system clock.
source:    OS-9 Level 2 Operating System Manual, SETIME section, line 9430
--- END ---

--- CARD ---
id:        setpr-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     setpr
claim:     Changes the CPU priority level of a specified process, affecting how much execution time the scheduler allots to it under multitasking.
context:   Priority affects task scheduling but is not process preemption; all runnable processes execute.
source:    OS-9 Level 2 Operating System Manual, SETPR section, line 9461
--- END ---

--- CARD ---
id:        setpr-priority-range
type:      FACT
target:    6809
verify:    from-manual
topic:     setpr
claim:     Priority values are decimal integers in the range 1 (lowest priority) to 255 (highest priority).
context:   1 gives minimal CPU time; 255 gives maximum relative to other processes.
source:    OS-9 Level 2 Operating System Manual, SETPR section, line 9477
--- END ---

--- CARD ---
id:        setpr-permission-own
type:      FACT
target:    6809
verify:    from-manual
topic:     setpr
claim:     Users can change priority only for processes that have the same user ID as the caller.
context:   Permission-based limitation on process control.
source:    OS-9 Level 2 Operating System Manual, SETPR section, line 9482
--- END ---

--- CARD ---
id:        setpr-permission-superuser
type:      FACT
target:    6809
verify:    from-manual
topic:     setpr
claim:     Super User (user ID 0) can change priority for any process in the system.
context:   Administrative privilege override.
source:    OS-9 Level 2 Operating System Manual, SETPR section, line 9488
--- END ---

--- CARD ---
id:        setpr-builtin-notincmds
type:      FACT
target:    6809
verify:    from-manual
topic:     setpr
claim:     SETPR is a built-in shell command and does not appear as a separate executable in the CMDS directory.
context:   It is implemented directly in the shell, not as an external program.
source:    OS-9 Level 2 Operating System Manual, SETPR section, line 9485
--- END ---

--- CARD ---
id:        shell-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The shell is OS-9's command interpreter program that reads lines from standard input (keyboard or file) and executes them as a sequence of OS-9 commands and programs.
context:   Central command dispatcher for all user and script commands.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9556
--- END ---

--- CARD ---
id:        shell-termination
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The shell reads one line at a time until it reaches an end-of-file marker, then terminates itself.
context:   EOF signals the shell to exit.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9574
--- END ---

--- CARD ---
id:        shell-separator-semicolon
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The semicolon (;) is the sequential execution separator; commands are run one after the other in order.
context:   Default behavior at end-of-line as well.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9612
--- END ---

--- CARD ---
id:        shell-separator-ampersand
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The ampersand (&) is the concurrent execution separator; commands run in parallel (background).
context:   Allows multiple processes to execute simultaneously.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9614
--- END ---

--- CARD ---
id:        shell-separator-pipe
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The pipe (!) character connects the standard output of one command to the standard input of another.
context:   Allows command chaining.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9616
--- END ---

--- CARD ---
id:        shell-redirect-input
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The < modifier redirects a program's standard input from a file instead of the keyboard.
context:   Batch input redirection.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9622
--- END ---

--- CARD ---
id:        shell-redirect-output
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The > modifier redirects a program's standard output to a file instead of the display.
context:   Output capture redirection.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9624
--- END ---

--- CARD ---
id:        shell-redirect-error
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The >> modifier redirects standard error output to a file.
context:   Separate error stream redirection.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9626
--- END ---

--- CARD ---
id:        shell-redirect-inputoutput
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The <> modifier redirects standard input to standard output (bidirectional redirection).
context:   Unusual redirection mode.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9628
--- END ---

--- CARD ---
id:        shell-redirect-inputerror
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The <<>> modifier redirects standard input to standard error output.
context:   Unusual redirection mode.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9630
--- END ---

--- CARD ---
id:        shell-redirect-outputerror
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The >>> modifier redirects standard output to standard error output.
context:   Combines output streams.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9633
--- END ---

--- CARD ---
id:        shell-memory-size-pages
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The #n modifier sets the process memory allocation in 256-byte pages.
context:   Controls memory consumption of a single execution.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9636
--- END ---

--- CARD ---
id:        shell-memory-size-kilobytes
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The #nK or #nk modifier sets the process memory allocation in kilobytes (1024-byte units).
context:   Alternative to the page-based #n modifier.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9638
--- END ---

--- CARD ---
id:        shell-param-chd
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The chd (change data directory) parameter tells the shell to switch the current working directory for file operations.
context:   Shell built-in directory navigation.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9642
--- END ---

--- CARD ---
id:        shell-param-kill
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The kill parameter sends a termination signal to the specified process ID.
context:   Process termination control.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9644
--- END ---

--- CARD ---
id:        shell-param-setpr
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The setpr parameter allows priority adjustment of a specified process from within the shell.
context:   Shell built-in priority control.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9648
--- END ---

--- CARD ---
id:        shell-param-chx
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The chx (change execution directory) parameter tells the shell to switch the current program search directory.
context:   Affects where the shell searches for executable modules.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9650
--- END ---

--- CARD ---
id:        shell-param-i-device
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The i=devicename parameter creates an immortal process bound to the specified device.
context:   Prevents the process from being terminated under normal conditions.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9652
--- END ---

--- CARD ---
id:        shell-param-w
type:      FACT
target:    6809
verify:    from-manual
topic:     shell
claim:     The w parameter blocks the shell until any background process terminates.
context:   Waits for background job completion.
source:    OS-9 Level 2 Operating System Manual, SHELL section, line 9654
--- END ---

--- CARD ---
id:        tmode-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     Displays or changes the terminal initialization parameters for an open device path, including baud rate and control key definitions. Automatically adjusts output format for 32- or 80-column displays.
context:   TMODE works only on already-open paths and changes are temporary; use XMODE for permanent device descriptor changes.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9688
--- END ---

--- CARD ---
id:        tmode-path-default
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     If no path number is specified, TMODE affects the standard input path (.0).
context:   Standard path defaults.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9891
--- END ---

--- CARD ---
id:        tmode-param-upc
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'upc' parameter forces uppercase-only display mode; lowercase input characters are automatically converted to uppercase on display.
context:   Useful for terminals that lack lowercase capability.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9709
--- END ---

--- CARD ---
id:        tmode-param-noupc
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The '-upc' parameter enables display of both uppercase and lowercase characters.
context:   Default mode; reverses 'upc' setting.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9712
--- END ---

--- CARD ---
id:        tmode-param-bsb
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'bsb' parameter enables backspace character erasure; backspace echoes as the sequence backspace-space-backspace to visually erase characters. This is the system default.
context:   Standard terminal backspace behavior.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9714
--- END ---

--- CARD ---
id:        tmode-param-nobsb
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The '-bsb' parameter disables character erasure on backspace; only a single backspace character echoes.
context:   For terminals without visual erase capability.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9719
--- END ---

--- CARD ---
id:        tmode-param-bsl
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'bsl' parameter enables backspace-over-line functionality, sending backspace-space-backspace sequences to erase an entire line on video terminals. This is the system default.
context:   For video terminal line editing.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9722
--- END ---

--- CARD ---
id:        tmode-param-nobsl
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The '-bsl' parameter disables backspace-over-line; instead, a new line sequence is printed to delete lines, suitable for hardcopy terminals.
context:   For hardcopy/printer terminals.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9730
--- END ---

--- CARD ---
id:        tmode-param-echo
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'echo' parameter enables character echo; input characters are displayed on the terminal as typed. This is the system default.
context:   Normal interactive terminal behavior.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9734
--- END ---

--- CARD ---
id:        tmode-param-noecho
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The '-echo' parameter disables character echo; input is not displayed.
context:   Useful for password entry or silent input modes.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9737
--- END ---

--- CARD ---
id:        tmode-param-lf
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'lf' parameter enables auto line feed; line feeds automatically echo to the terminal on input and output carriage returns. This is the system default.
context:   Automatic CR-to-LF conversion.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9739
--- END ---

--- CARD ---
id:        tmode-param-nolf
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The '-lf' parameter disables auto line feed.
context:   For terminals that handle line feeds manually.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9744
--- END ---

--- CARD ---
id:        tmode-param-pause
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'pause' parameter enables screen pause mode; output suspends when the screen fills, resuming when the user presses the space bar. This is the system default.
context:   Prevents text from scrolling off the screen unread.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9746
--- END ---

--- CARD ---
id:        tmode-param-nopause
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The '-pause' parameter disables screen pause mode; output continues scrolling without interruption.
context:   For non-interactive output redirection.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9752
--- END ---

--- CARD ---
id:        tmode-param-null
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'null=n' parameter sets the null character count (number of $00 bytes transmitted after each carriage return for return delay). Default is 0.
context:   Provides delay for old hardcopy terminals.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9754
--- END ---

--- CARD ---
id:        tmode-param-pag
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'pag=n' parameter sets the video display page length to n lines, affecting screen pause behavior.
context:   Defines how many lines are "full" for pause mode purposes.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9759
--- END ---

--- CARD ---
id:        tmode-param-bsp
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'bsp=h' parameter sets the input backspace character to the hexadecimal value h. Default is 08 (ASCII backspace).
context:   Allows remapping of backspace key for alternate keyboards.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9763
--- END ---

--- CARD ---
id:        tmode-param-bse
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'bse=h' parameter sets the output backspace character to the hexadecimal value h. Default is 08 (ASCII backspace).
context:   Controls how backspace is echoed to the terminal.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9766
--- END ---

--- CARD ---
id:        tmode-param-del
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'del=h' parameter sets the delete-line character for input to the hexadecimal value h. Default is 18 (hex).
context:   Allows customization of line deletion key.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9769
--- END ---

--- CARD ---
id:        tmode-param-bell
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'bell=h' parameter sets the alert/bell output character to the hexadecimal value h. Default is 07 (ASCII bell).
context:   Customizes the alert signal.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9772
--- END ---

--- CARD ---
id:        tmode-param-eor
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'eor=h' parameter sets the end-of-record (carriage return) character for input to the hexadecimal value h. Default is 0D (hex).
context:   Allows customization of record terminator.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9775
--- END ---

--- CARD ---
id:        tmode-param-eof
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'eof=h' parameter sets the end-of-file character for input to the hexadecimal value h. Default is 1B (hex, ASCII ESC).
context:   Customizes the file terminator signal.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9787
--- END ---

--- CARD ---
id:        tmode-param-type
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'type=h' parameter sets ACIA (asynchronous communications interface adapter) initialization values in hexadecimal. Default is 00. Bits 5-7 control parity (no parity, MARK, SPACE, even, or odd); Bit 4 enables auto-answer modem support.
context:   External device serial configuration. TERM-VDG and TERM-WIN have special type= meanings.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9790
--- END ---

--- CARD ---
id:        tmode-param-reprint
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'reprint=h' parameter sets the reprint-line character to the hexadecimal value h.
context:   Allows user to request re-display of the current input line.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9834
--- END ---

--- CARD ---
id:        tmode-param-dup
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'dup=h' parameter sets the duplicate-last-line character to the hexadecimal value h. Default is 01 (hex).
context:   Allows shortcut to repeat the previous input line.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9839
--- END ---

--- CARD ---
id:        tmode-param-pse
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'pse=h' parameter sets the pause character to the hexadecimal value h. Default is 17 (hex, Ctrl-W).
context:   Character used to trigger screen pause.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9843
--- END ---

--- CARD ---
id:        tmode-param-abort
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'abort=h' parameter sets the terminate character (normally Ctrl-C) to the hexadecimal value h.
context:   Allows remapping of interrupt/abort key.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9846
--- END ---

--- CARD ---
id:        tmode-param-quit
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'quit=h' parameter sets the quit character (normally Ctrl-E) to the hexadecimal value h.
context:   Allows remapping of quit key.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9850
--- END ---

--- CARD ---
id:        tmode-param-baud
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     The 'baud=h' parameter sets baud rate, word length, and stop bits for a software-controllable serial interface. Bits 0-3 select baud rate (codes 0-7), Bits 5-6 select word length (00=8-bit, 01=7-bit), Bit 7 selects stop bits (0=1, 1=2). Baud codes: 0=110, 1=300, 2=600, 3=1200, 4=2400, 5=4800, 6=9600, 7=19200 (ACIA) or 38400 (SIO).
context:   GOTCHA: OCR corruption in manual line 10445 shows "1=800" and "3=600" which contradict the standard RS-232 baud mapping; verified against XMODE section which repeats these values with similar OCR errors. Actual mapping is uncertain for codes 1 and 3 from this text alone.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9854
--- END ---

--- CARD ---
id:        tmode-procedure-file-note
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     When TMODE is used in a procedure file (shell script), the user must specify a standard output path (.1 or .2) because the procedure file redirection changes the shell's standard input path.
context:   TMODE can only operate on SCFMAN-type devices and requires an open path.
source:    OS-9 Level 2 Operating System Manual, TMODE section, line 9901
--- END ---

--- CARD ---
id:        tuneport-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     tuneport
claim:     Tests and adjusts delay loop values for a device's serial interface to optimize performance at the current baud rate. Allows interactive or command-line tuning.
context:   CoCo-specific for printer or terminal serial port tuning.
source:    OS-9 Level 2 Operating System Manual, TUNEPORT section, line 9940
--- END ---

--- CARD ---
id:        tuneport-device-printer
type:      FACT
target:    6809
verify:    from-manual
topic:     tuneport
claim:     The /p device parameter selects the printer port for testing and tuning.
context:   CoCo printer device.
source:    OS-9 Level 2 Operating System Manual, TUNEPORT section, line 9949
--- END ---

--- CARD ---
id:        tuneport-device-terminal
type:      FACT
target:    6809
verify:    from-manual
topic:     tuneport
claim:     The /t1 device parameter selects the terminal serial port for testing and tuning.
context:   CoCo serial terminal device.
source:    OS-9 Level 2 Operating System Manual, TUNEPORT section, line 9949
--- END ---

--- CARD ---
id:        tuneport-option-s
type:      FACT
target:    6809
verify:    from-manual
topic:     tuneport
claim:     The -s=value option directly sets a new delay loop value without interactive testing.
context:   Allows batch setting of tuned values.
source:    OS-9 Level 2 Operating System Manual, TUNEPORT section, line 9956
--- END ---

--- CARD ---
id:        tuneport-interactive-workflow
type:      FACT
target:    6809
verify:    from-manual
topic:     tuneport
claim:     In interactive mode, TUNEPORT displays the current baud rate, sends test data to the device, displays the current delay value, and prompts for a new value. The cycle repeats until the user presses Enter without entering a value.
context:   Allows iterative tuning until optimal value is found.
source:    OS-9 Level 2 Operating System Manual, TUNEPORT section, line 9968
--- END ---

--- CARD ---
id:        tuneport-persistence-cobbler
type:      FACT
target:    6809
verify:    from-manual
topic:     tuneport
claim:     To make tuned delay values persistent across system boots, the value can be set in the TUNEPORT command in the startup file or recorded via COBBLER when generating a new boot file.
context:   In-memory changes do not persist without boot file update.
source:    OS-9 Level 2 Operating System Manual, TUNEPORT section, line 9994
--- END ---

--- CARD ---
id:        unlink-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     unlink
claim:     Tells OS-9 that a module loaded in memory is no longer needed by the user, allowing the system to deallocate its memory if the link count reaches zero.
context:   Module reference counting mechanism.
source:    OS-9 Level 2 Operating System Manual, UNLINK section, line 10014
--- END ---

--- CARD ---
id:        unlink-linkcount-behavior
type:      FACT
target:    6809
verify:    from-manual
topic:     unlink
claim:     Each process that uses a module increments the module's link-count; each UNLINK command decrements it by 1. When link-count reaches 0, OS-9 deallocates and removes the module from memory.
context:   Reference counting allows safe multi-user module sharing.
source:    OS-9 Level 2 Operating System Manual, UNLINK section, line 10032
--- END ---

--- CARD ---
id:        unlink-multiple-modules
type:      FACT
target:    6809
verify:    from-manual
topic:     unlink
claim:     A single UNLINK command can unlink multiple modules by listing them on the same command line.
context:   Batch unlink operation.
source:    OS-9 Level 2 Operating System Manual, UNLINK section, line 10025
--- END ---

--- CARD ---
id:        unlink-recommendation
type:      FACT
target:    6809
verify:    from-manual
topic:     unlink
claim:     Users should unlink modules whenever possible to make efficient use of available memory resources.
context:   Resource management best practice.
source:    OS-9 Level 2 Operating System Manual, UNLINK section, line 10039
--- END ---

--- CARD ---
id:        unlink-warning-twice
type:      FACT
target:    6809
verify:    from-manual
topic:     unlink
claim:     Modules that the user has loaded and linked might need to be unlinked twice to remove them from memory (once to undo the link, once to undo the load).
context:   Reflects asymmetry between load and link operations.
source:    OS-9 Level 2 Operating System Manual, UNLINK section, line 10041
--- END ---

--- CARD ---
id:        unlink-warning-donot
type:      FACT
target:    6809
verify:    from-manual
topic:     unlink
claim:     Users should never attempt to unlink a module they did not load or link, and should never unlink a module that is currently in use by another program (check via PROCS).
context:   Attempting to unlink in-use or unowned modules can corrupt system state.
source:    OS-9 Level 2 Operating System Manual, UNLINK section, line 10052
--- END ---

--- CARD ---
id:        wcreate-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     wcreate
claim:     Initializes and creates a display window on a specified window device with user-defined size, position, and color attributes.
context:   CoCo-specific graphics/window capability.
source:    OS-9 Level 2 Operating System Manual, WCREATE section, line 10135
--- END ---

--- CARD ---
id:        wcreate-parameter-device
type:      FACT
target:    6809
verify:    from-manual
topic:     wcreate
claim:     The window device name parameter specifies which window to create (examples: W, W1, W2, W3, etc.).
context:   Multiple windows can be created on the same display.
source:    OS-9 Level 2 Operating System Manual, WCREATE section, line 10140
--- END ---

--- CARD ---
id:        wcreate-parameter-position
type:      FACT
target:    6809
verify:    from-manual
topic:     wcreate
claim:     The xpos and ypos parameters specify the decimal coordinates of the upper-left corner of the window on the display.
context:   Position is in character or pixel coordinates depending on display mode.
source:    OS-9 Level 2 Operating System Manual, WCREATE section, line 10143
--- END ---

--- CARD ---
id:        wcreate-parameter-size
type:      FACT
target:    6809
verify:    from-manual
topic:     wcreate
claim:     The xsize parameter specifies the horizontal window width in columns, range 1-80 (decimal); ysize specifies the vertical height in lines, range 1-24 (decimal).
context:   Allows window dimensions from minimal to full screen.
source:    OS-9 Level 2 Operating System Manual, WCREATE section, line 10149
--- END ---

--- CARD ---
id:        wcreate-parameter-colors
type:      FACT
target:    6809
verify:    from-manual
topic:     wcreate
claim:     The foreground and background parameters specify window foreground and background colors; border color is optional (defaults to black).
context:   Color values depend on display hardware capabilities.
source:    OS-9 Level 2 Operating System Manual, WCREATE section, line 10155
--- END ---

--- CARD ---
id:        wcreate-option-screentype
type:      FACT
target:    6809
verify:    from-manual
topic:     wcreate
claim:     The -s=type option selects the display screen type: 0 (40-column text), 1 (80-column text), 5 (640x192 2-color graphics), 6 (320x192 4-color graphics), 7 (640x192 4-color graphics), 8 (320x192 16-color graphics).
context:   CoCo display modes. When -s=type is used, border color must be specified in the command line.
source:    OS-9 Level 2 Operating System Manual, WCREATE section, line 10165
--- END ---

--- CARD ---
id:        wcreate-option-input
type:      FACT
target:    6809
verify:    from-manual
topic:     wcreate
claim:     The @ option directs WCREATE to accept window parameters from standard input (redirected from a file) instead of the command line.
context:   Allows batch window creation via input file.
source:    OS-9 Level 2 Operating System Manual, WCREATE section, line 10186
--- END ---

--- CARD ---
id:        wcreate-option-help
type:      FACT
target:    6809
verify:    from-manual
topic:     wcreate
claim:     The -? option displays a help message for the WCREATE command.
context:   On-line documentation.
source:    OS-9 Level 2 Operating System Manual, WCREATE section, line 10189
--- END ---

--- CARD ---
id:        xmode-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     Displays or changes initialization parameters of any SCF-type device (video, printer, RS-232 port, etc.) through its device descriptor. Changes persist across open/close cycles and survive path closures. Automatically adjusts output for 32- or 80-column displays.
context:   Unlike TMODE which affects only open paths, XMODE modifies permanent device configuration.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10263
--- END ---

--- CARD ---
id:        xmode-vs-tmode
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     TMODE operates only on open device paths and changes are temporary; XMODE updates the device descriptor and changes persist as long as the computer is running, even if the device is repeatedly opened and closed.
context:   TMODE affects current session only; XMODE is persistent until reboot or next XMODE command.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10462
--- END ---

--- CARD ---
id:        xmode-persistence-cobbler
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     Changes made via XMODE can be permanently written to a new system diskette by using the COBBLER program to re-make the boot file.
context:   XMODE changes + COBBLER make boot-persistent configuration.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10468
--- END ---

--- CARD ---
id:        xmode-require-devicename
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     XMODE requires that a device name be specified (e.g., /term, /w, /t2); if no parameters are given, XMODE displays the current values for all parameters.
context:   Device name is mandatory; path number is not sufficient.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10481
--- END ---

--- CARD ---
id:        xmode-param-upc
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'upc' parameter forces uppercase-only display mode; lowercase input characters are automatically converted to uppercase on display.
context:   Useful for terminals that lack lowercase capability.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10284
--- END ---

--- CARD ---
id:        xmode-param-noupc
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The '-upc' parameter enables display of both uppercase and lowercase characters.
context:   Default mode; reverses 'upc' setting.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10287
--- END ---

--- CARD ---
id:        xmode-param-bsb
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'bsb' parameter enables backspace character erasure; backspace echoes as the sequence backspace-space-backspace to visually erase characters. This is the system default.
context:   Standard terminal backspace behavior.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10289
--- END ---

--- CARD ---
id:        xmode-param-nobsb
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The '-bsb' parameter disables character erasure on backspace; only a single backspace character echoes.
context:   For terminals without visual erase capability.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10294
--- END ---

--- CARD ---
id:        xmode-param-bsl
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'bsl' parameter enables backspace-over-line functionality, sending backspace-space-backspace sequences to erase an entire line on video terminals. This is the system default.
context:   For video terminal line editing.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10297
--- END ---

--- CARD ---
id:        xmode-param-nobsl
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The '-bsl' parameter disables backspace-over-line; instead, a new line sequence is printed to delete lines, suitable for hardcopy terminals.
context:   For hardcopy/printer terminals.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10302
--- END ---

--- CARD ---
id:        xmode-param-echo
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'echo' parameter enables character echo; input characters are displayed on the terminal as typed. This is the system default.
context:   Normal interactive terminal behavior.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10314
--- END ---

--- CARD ---
id:        xmode-param-noecho
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The '-echo' parameter disables character echo; input is not displayed.
context:   Useful for password entry or silent input modes.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10317
--- END ---

--- CARD ---
id:        xmode-param-lf
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'lf' parameter enables auto line feed; line feeds automatically echo to the terminal on input and output carriage returns. This is the system default.
context:   Automatic CR-to-LF conversion.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10319
--- END ---

--- CARD ---
id:        xmode-param-nolf
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The '-lf' parameter disables auto line feed.
context:   For terminals that handle line feeds manually.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10324
--- END ---

--- CARD ---
id:        xmode-param-pause
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'pause' parameter enables screen pause mode; output suspends when the screen fills, resuming when the user presses the space bar. This is the system default.
context:   Prevents text from scrolling off the screen unread.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10326
--- END ---

--- CARD ---
id:        xmode-param-nopause
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The '-pause' parameter disables screen pause mode; output continues scrolling without interruption.
context:   For non-interactive output redirection.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10332
--- END ---

--- CARD ---
id:        xmode-param-null
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'null=n' parameter sets the null character count (number of $00 bytes transmitted after each carriage return for return delay). Default is 0.
context:   Provides delay for old hardcopy terminals.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10334
--- END ---

--- CARD ---
id:        xmode-param-pag
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'pag=n' parameter sets the video display page length to n lines, affecting screen pause behavior.
context:   Defines how many lines are "full" for pause mode purposes.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10339
--- END ---

--- CARD ---
id:        xmode-param-bsp
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'bsp=h' parameter sets the input backspace character to the hexadecimal value h. Default is 08 (ASCII backspace).
context:   Allows remapping of backspace key for alternate keyboards.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10343
--- END ---

--- CARD ---
id:        xmode-param-bse
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'bse=h' parameter sets the output backspace character to the hexadecimal value h. Default is 08 (ASCII backspace).
context:   Controls how backspace is echoed to the terminal.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10346
--- END ---

--- CARD ---
id:        xmode-param-del
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'del=h' parameter sets the delete-line character for input to the hexadecimal value h. Default is 18 (hex).
context:   Allows customization of line deletion key.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10349
--- END ---

--- CARD ---
id:        xmode-param-bell
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'bell=h' parameter sets the alert/bell output character to the hexadecimal value h. Default is 07 (ASCII bell).
context:   Customizes the alert signal.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10352
--- END ---

--- CARD ---
id:        xmode-param-eor
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'eor=h' parameter sets the end-of-record (carriage return) character for input to the hexadecimal value h. Default is 0D (hex).
context:   Allows customization of record terminator.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10355
--- END ---

--- CARD ---
id:        xmode-param-eof
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'eof=h' parameter sets the end-of-file character for input to the hexadecimal value h. Default is 1B (hex, ASCII ESC).
context:   Customizes the file terminator signal.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10359
--- END ---

--- CARD ---
id:        xmode-param-type
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'type=h' parameter sets ACIA (asynchronous communications interface adapter) initialization values in hexadecimal. Default is 00. Bits 5-7 control parity (no parity, MARK, SPACE, even, or odd); Bit 4 enables auto-answer modem support.
context:   External device serial configuration. TERM-VDG and TERM-WIN have special type= meanings.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10369
--- END ---

--- CARD ---
id:        xmode-param-reprint
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'reprint=h' parameter sets the reprint-line character to the hexadecimal value h.
context:   Allows user to request re-display of the current input line.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10413
--- END ---

--- CARD ---
id:        xmode-param-dup
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'dup=h' parameter sets the duplicate-last-line character to the hexadecimal value h. Default is 01 (hex).
context:   Allows shortcut to repeat the previous input line.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10416
--- END ---

--- CARD ---
id:        xmode-param-pse
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'pse=h' parameter sets the pause character to the hexadecimal value h. Default is 17 (hex, Ctrl-W).
context:   Character used to trigger screen pause.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10427
--- END ---

--- CARD ---
id:        xmode-param-abort
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'abort=h' parameter sets the terminate character (normally Ctrl-C) to the hexadecimal value h.
context:   Allows remapping of interrupt/abort key.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10431
--- END ---

--- CARD ---
id:        xmode-param-quit
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'quit=h' parameter sets the quit character (normally Ctrl-E) to the hexadecimal value h.
context:   Allows remapping of quit key.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10435
--- END ---

--- CARD ---
id:        xmode-param-baud
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     The 'baud=h' parameter sets baud rate, word length, and stop bits for a software-controllable serial interface. Bits 0-3 select baud rate (codes 0-7), Bits 5-6 select word length (00=8-bit, 01=7-bit), Bit 7 selects stop bits (0=1, 1=2). Baud codes: 0=110, 1=300, 2=600, 3=1200, 4=2400, 5=4800, 6=9600, 7=19200 (ACIA) or 38400 (SIO).
context:   GOTCHA: OCR corruption in manual line 10444 shows "1=800" and "3=600" which contradict the standard RS-232 baud mapping; line 10445 shows "7=32000" which does not match 19200 or 38400. These same errors appear in the TMODE section, suggesting consistent OCR errors rather than intentional differences. Actual mapping is uncertain from this text alone.
source:    OS-9 Level 2 Operating System Manual, XMODE section, line 10439
--- END ---
