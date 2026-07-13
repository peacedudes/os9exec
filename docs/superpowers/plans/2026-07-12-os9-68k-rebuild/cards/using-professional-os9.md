--- CARD ---
id:        module-foundation-architecture
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     modular-software
claim:     OS-9's fundamental unit of software organization is the memory module: pure program code loaded once and shared by multiple tasks/users simultaneously. Variable storage is allocated separately at runtime.
context:   This sharing model contrasts with systems that load multiple copies of the same program (OS-9 saves ~300% memory for multi-user execution of the same program).
source:    Using Professional OS-9 v2.4, "The Memory Module and Modular Software", p. 1-6
--- END ---

--- CARD ---
id:        library-module-math-example
type:      FACT
target:    68k
verify:    from-manual
topic:     modular-software
claim:     A standard Math module provides floating-point arithmetic for all programming languages and programs, eliminating per-program math packages and allowing hardware floating-point processor substitution without software recompilation.
context:   This is a concrete example of the shared library principle in OS-9's module system.
source:    Using Professional OS-9 v2.4, "The Memory Module and Modular Software", p. 1-6
--- END ---

--- CARD ---
id:        disk-sector-cluster-allocation
type:      FACT
target:    68k
verify:    from-manual
topic:     file-system-storage
claim:     Data is organized into sectors (pre-defined fixed-size byte groups, e.g., 256 bytes) and allocated in clusters (groupings of sectors whose size depends on device capacity and physical characteristics).
context:   The sector size is set during format; cluster size is device-dependent and shown by the free utility.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Free and Mfree section, p. 3-9
--- END ---

--- CARD ---
id:        file-pointer-movement-automatic
type:      FACT
target:    68k
verify:    from-manual
topic:     file-system-storage
claim:     When a file is opened or created, OS-9 maintains a file pointer tracking the next byte to read or write. Sequential reads/writes automatically advance the pointer; random access uses the I$SEEK system call (or seek language binding).
context:   This is fundamental to OS-9's I/O model; seek is described in the Technical Manual.
source:    Using Professional OS-9 v2.4, "OS-9 File Storage", p. 4-1 to 4-2
--- END ---

--- CARD ---
id:        boot-startup-os9boot-files
type:      FACT
target:    68k
verify:    from-manual
topic:     system-boot
claim:     Boot process reads system disk into memory; two key files: startup (shell procedure file executed immediately after boot, can contain any OS-9 command or program) and OS9Boot (contains OS-9 system modules loaded into memory).
context:   Both can be modified; full details on changing them are in the OS-9 system management chapter.
source:    Using Professional OS-9 v2.4, "Starting OS-9", Booting OS-9 section, p. 2-1 to 2-2
--- END ---

--- CARD ---
id:        setime-clock-startup-requirement
type:      FACT
target:    68k
verify:    from-manual
topic:     system-boot
claim:     After boot, setime utility must start the system clock; clock must be running for multi-tasking to function. setime prompts for year/month/day/hour/minute/second and optional am/pm; supports flexible delimiter entry (space, colon, semicolon, comma, slash).
context:   If system has battery-backed clock, use setime -s; otherwise setime prompts interactively.
source:    Using Professional OS-9 v2.4, "Starting OS-9", Booting OS-9 section, p. 2-2
--- END ---

--- CARD ---
id:        disk-format-parameters-density-sides
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-operations
claim:     format utility accepts command-line parameters: -sd (single density), -dd (double density), -ss (single sided), -ds (double sided); format varies by disk drive type and manufacturer, usually set to maximum capacity.
context:   Refer to hardware documentation and system disk label for correct format parameters.
source:    Using Professional OS-9 v2.4, "Starting OS-9", Formatting a Disk section, p. 2-3
--- END ---

--- CARD ---
id:        load-utility-memory-execution
type:      FACT
target:    68k
verify:    from-manual
topic:     process-execution
claim:     load utility puts a copy of a program into computer memory; once loaded, the program can be executed from memory without re-reading from disk. Any OS-9 utility can be loaded and executed this way.
context:   Useful for single-drive systems to avoid repeated disk swaps.
source:    Using Professional OS-9 v2.4, "Starting OS-9", Single Drive Format section, p. 2-4
--- END ---

--- CARD ---
id:        backup-utility-two-pass-verification
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-operations
claim:     backup utility makes exact disk copies in two passes: first pass reads source disk into memory buffer and writes to destination; second pass verifies all data copied correctly. Errors on first pass indicate source disk problem; errors on second pass indicate destination disk issue.
context:   If backup repeatedly fails on second pass, reformat destination disk to ensure no bad sectors.
source:    Using Professional OS-9 v2.4, "Starting OS-9", The Backup Procedure section, p. 2-5
--- END ---

--- CARD ---
id:        shell-prompt-dollar-default
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-interface
claim:     After successful boot and time/date setup, shell displays $ prompt (default system prompt) indicating OS-9 is active and ready for command input.
context:   The prompt can be customized; this manual uses $ for all examples.
source:    Using Professional OS-9 v2.4, "Starting OS-9", Booting OS-9 section, p. 2-2
--- END ---

--- CARD ---
id:        line-oriented-input-return-key
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-interface
claim:     Most input to OS-9, programming languages, and applications is line-oriented: characters are collected as typed but not sent to the program until <return> key is pressed, allowing error correction before submission.
context:   This applies to OS-9 itself, languages, and application programs uniformly.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Using the Keyboard section, p. 3-5
--- END ---

--- CARD ---
id:        control-A-repeat-previous-line
type:      FACT
target:    68k
verify:    from-manual
topic:     line-editing
claim:     <control>A redisplays the previous input line without executing it; cursor is at line end and can be edited by backspacing and typing over old characters.
context:   This is one of several line-editing control keys for error correction.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Using the Keyboard section, p. 3-5
--- END ---

--- CARD ---
id:        control-X-delete-entire-line
type:      FACT
target:    68k
verify:    from-manual
topic:     line-editing
claim:     <control>X deletes the entire current input line.
context:   Useful for abandoning a partially-typed command before pressing return.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Using the Keyboard section, p. 3-5
--- END ---

--- CARD ---
id:        control-S-xoff-halt-output
type:      FACT
target:    68k
verify:    from-manual
topic:     terminal-control
claim:     <control>S (X-off) halts input and output until <control>Q (X-on) is entered; used by serial I/O devices like printers to control output speed.
context:   The <control>S function name derives from hardware flow control conventions.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Using the Keyboard section, p. 3-5
--- END ---

--- CARD ---
id:        control-W-page-pause-output
type:      FACT
target:    68k
verify:    from-manual
topic:     terminal-control
claim:     <control>W temporarily halts output so user can read screen before data scrolls off; output resumes when any other key is pressed.
context:   Distinct from permanent page-pause mode set via tmode.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Using the Keyboard section, p. 3-5
--- END ---

--- CARD ---
id:        esc-ctrl-eof-file-termination
type:      FACT
target:    68k
verify:    from-manual
topic:     terminal-control
claim:     ESCAPE or <control>[ signals end-of-file; all OS-9 I/O devices including terminals are accessed as files, so this simulates reaching end of disk file.
context:   Allows programs to detect file termination uniformly across device types.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Using the Keyboard section, p. 3-6
--- END ---

--- CARD ---
id:        control-C-interrupt-foreground-background
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     process-control
claim:     <control>C sends interrupt signal to most recent program; behavior varies by program. If program handles interrupts, <control>C usually stops current function and returns to menu/command mode. In shell, <control>C can convert foreground program to background if program hasn't begun I/O to terminal.
context:   Behavior is program-dependent; not all programs support interrupts.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Using the Keyboard section, p. 3-6
--- END ---

--- CARD ---
id:        control-E-abort-process
type:      FACT
target:    68k
verify:    from-manual
topic:     process-control
claim:     <control>E sends program abort signal to currently running program; in most cases prematurely aborts current program and returns to shell.
context:   More forceful than <control>C; reliably terminates programs.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Using the Keyboard section, p. 3-6
--- END ---

--- CARD ---
id:        tmode-control-key-customization
type:      FACT
target:    68k
verify:    from-manual
topic:     system-configuration
claim:     tmode utility redefines the function of control keys; correspondence between control keys and their functions is changeable, allowing customization to specific computer keyboard layouts.
context:   tmode is described in the system management chapter and OS-9 Utilities section.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Using the Keyboard section, p. 3-6
--- END ---

--- CARD ---
id:        page-pause-feature-screen-halt
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     terminal-output
claim:     Page pause feature counts output lines until full screen is displayed, then halts output until any key is pressed; repeats for each screen. Can be fooled by lines longer than physical screen width (wrapped lines not counted properly).
context:   Can be toggled on/off and configured via tmode with pag parameter.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", The Page Pause Feature section, p. 3-6
--- END ---

--- CARD ---
id:        shell-option-echo-verbose-abort
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-configuration
claim:     Key shell option flags: -t/-nt (echo/don't echo input lines), -v/-nv (verbose mode displaying directory search messages), -x/-nx (abort/don't abort on error).
context:   Options can be set via command line or set command; affect shell behavior uniformly.
source:    Using Professional OS-9 v2.4, "The Shell", Shell Environment section, p. 5-1 to 5-2
--- END ---

--- CARD ---
id:        shell-environment-variables-four-automatic
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-environment
claim:     Four environment variables automatically set on timesharing login: PORT (terminal name, set by tsmon), HOME (home directory from password file), SHELL (first process executed on login), USER (user name typed at login).
context:   On single-user systems, these can be set manually with setenv command or procedure files.
source:    Using Professional OS-9 v2.4, "The Shell", The Shell Environment section, p. 5-3
--- END ---

--- CARD ---
id:        path-environment-colon-separated-search
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-environment
claim:     PATH environment variable specifies colon-separated list of directories. Shell searches PATH directories when executing commands if default commands directory doesn't contain the executable, continuing until file/module found or list exhausted.
context:   This is a critical system variable for command location.
source:    Using Professional OS-9 v2.4, "The Shell", The Shell Environment section, p. 5-3
--- END ---

--- CARD ---
id:        prompt-at-macro-shell-level
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-configuration
claim:     PROMPT environment variable specifies current prompt; using @ sign as replaceable macro for shell level number tracks nested shells (base level set by _sh variable); e.g., "@howdy: " with _sh=0 displays "howdy:" at level 0, "1.howdy:" at level 1, etc.
context:   Allows visual tracking of shell nesting depth.
source:    Using Professional OS-9 v2.4, "The Shell", The Shell Environment section, p. 5-4
--- END ---

--- CARD ---
id:        term-environment-terminal-type
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-environment
claim:     TERM environment variable specifies terminal type being used; allows word processors, screen editors, and screen-dependent programs to select appropriate terminal configuration.
context:   Essential for terminal-aware applications.
source:    Using Professional OS-9 v2.4, "The Shell", The Shell Environment section, p. 5-4
--- END ---

--- CARD ---
id:        environment-variable-inheritance-child-process
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     process-environment
claim:     Environment variables are inherited by any process called by the shell or descendant shells, allowing use as global variables. If a child shell redefines a variable, it's only redefined for that shell and its descendants, not the parent.
context:   This scoping model allows per-shell customization without affecting parent shell.
source:    Using Professional OS-9 v2.4, "The Shell", The Shell Environment section, p. 5-3
--- END ---

--- CARD ---
id:        setenv-declare-variable-storage
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-commands
claim:     setenv declares a variable and sets its value in the shell's environment storage area. Variable is known only to the defining shell and its descendant processes; does not change parent process environment.
context:   Example: setenv PATH ..:/h0/cmds:/d0/cmds:/dd/cmds
source:    Using Professional OS-9 v2.4, "The Shell", Changing the Shell Environment section, p. 5-4
--- END ---

--- CARD ---
id:        unsetenv-clear-remove-variable
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-commands
claim:     unsetenv clears the value and removes a variable from environment storage.
context:   Example: unsetenv PATH
source:    Using Professional OS-9 v2.4, "The Shell", Changing the Shell Environment section, p. 5-5
--- END ---

--- CARD ---
id:        printenv-display-variables
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-commands
claim:     printenv prints all environment variables and their values to standard output.
context:   Useful for inspecting current shell environment.
source:    Using Professional OS-9 v2.4, "The Shell", Changing the Shell Environment section, p. 5-5
--- END ---

--- CARD ---
id:        built-in-shell-commands-no-new-process
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-commands
claim:     Shell has special built-in commands executable without loading a program or creating a new process, and executable regardless of current execution directory. Examples: chd, chx, ex, kill, logout, profile, set, setenv, setpr, unsetenv, w, wait.
context:   Built-ins do not appear in CMDS directory; they're internal to shell.
source:    Using Professional OS-9 v2.4, "The Shell", Built-in Shell Commands section, p. 5-6
--- END ---

--- CARD ---
id:        chd-change-data-directory
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-commands
claim:     chd <path> built-in command changes current data directory; accepts full or relative path. With no argument, changes to directory specified by HOME environment variable if set.
context:   Relative paths from chd are relative to current data directory.
source:    Using Professional OS-9 v2.4, "The Shell", Built-in Shell Commands section, p. 5-6
--- END ---

--- CARD ---
id:        chx-change-execution-directory
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-commands
claim:     chx <path> built-in command changes current execution directory; accepts full or relative path. Relative paths used by chx are relative to current execution directory.
context:   Different from chd; affects which directories are searched for programs to execute.
source:    Using Professional OS-9 v2.4, "The Shell", Built-in Shell Commands section, p. 5-6
--- END ---

--- CARD ---
id:        ex-direct-execution-replace-shell
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-commands
claim:     ex <name> directly executes named program, replacing shell process with new execution module.
context:   Unlike normal program execution which creates a child process, ex replaces the current shell.
source:    Using Professional OS-9 v2.4, "The Shell", Built-in Shell Commands section, p. 5-6
--- END ---

--- CARD ---
id:        profile-procedure-file-source
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-commands
claim:     profile <path> reads input from named file and then returns to shell's original input source.
context:   Useful for executing batch command files.
source:    Using Professional OS-9 v2.4, "The Shell", Built-in Shell Commands section, p. 5-6
--- END ---

--- CARD ---
id:        shell-command-keyword-search-sequence
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     shell-execution
claim:     When shell processes non-built-in keyword, it searches in order: (1) memory module directory (check if already loaded), (2) current execution directory (try load, then procedure file interpretation, then next PATH directory), (3) current data directory (process as procedure file if found). Continues until successful execution or list exhausted; returns error if not found.
context:   Programs loaded once are reused without reloading; procedure files execute in child shell.
source:    Using Professional OS-9 v2.4, "The Shell", Shell Command Line Processing section, p. 5-7
--- END ---

--- CARD ---
id:        procedure-file-child-shell-execution
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     procedure-files
claim:     Procedure file (file containing shell command lines) is processed by newly created child shell as if commands were typed manually. After all procedure file commands execute, control returns to parent shell. Built-in commands (chd, chx) in procedure file only affect child shell, not parent.
context:   This scoping prevents data directory changes from affecting parent shell.
source:    Using Professional OS-9 v2.4, "The Shell", Shell Command Line Processing section, p. 5-7
--- END ---

--- CARD ---
id:        memory-modifier-pound-kilobytes
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-modifiers
claim:     # (pound sign) modifier followed by kilobyte number allocates additional working memory in 1K increments (e.g., #10k or #10). Memory increase only affects single command; must repeat modifier for subsequent commands.
context:   Module header specifies minimum working memory required; modifier only increases from that minimum. Programs written in C use additional memory for stack space only.
source:    Using Professional OS-9 v2.4, "The Shell", Shell Command Line Processing section, p. 5-9
--- END ---

--- CARD ---
id:        io-redirection-standard-paths
type:      FACT
target:    68k
verify:    from-manual
topic:     io-redirection
claim:     Programs normally receive input from terminal keyboard via standard input path, send output to terminal display via standard output path, and can use standard error path (by default same device as standard output) for status/error messages.
context:   Each child process inherits parent process's standard I/O paths.
source:    Using Professional OS-9 v2.4, "The Shell", I/O Redirection Modifiers section, p. 5-10
--- END ---

--- CARD ---
id:        redirection-modifier-less-greater-error
type:      FACT
target:    68k
verify:    from-manual
topic:     io-redirection
claim:     Three redirection modifiers: < (redirect input path), > (redirect output path), >> (redirect error path); must be immediately followed by path describing file or device.
context:   Modifiers are stripped from command line parameters and cannot be passed to programs unless quoted.
source:    Using Professional OS-9 v2.4, "The Shell", Shell Command Line Processing section, p. 5-8 to 5-10
--- END ---

--- CARD ---
id:        pipe-operator-filter-chain
type:      FACT
target:    68k
verify:    from-manual
topic:     pipes-filters
claim:     ! (exclamation mark) operator constructs pipes; allows output of one program to become input of next program in a chain (filter construction).
context:   Pipes enable complex data processing workflows.
source:    Using Professional OS-9 v2.4, "The Shell", Shell Command Line Processing section, p. 5-8
--- END ---

--- CARD ---
id:        wildcard-asterisk-any-characters
type:      FACT
target:    68k
verify:    from-manual
topic:     wildcards
claim:     Asterisk (*) wildcard matches any number of letters, numbers, or special characters; by itself expands to all files in directory.
context:   Example: dir d* lists all files and directories beginning with 'd'.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", Dir: Displaying Contents section, p. 4-11
--- END ---

--- CARD ---
id:        wildcard-question-single-character
type:      FACT
target:    68k
verify:    from-manual
topic:     wildcards
claim:     Question mark (?) wildcard matches a single letter, number, or special character.
context:   Example: dir prog_? lists all files with name prog_ followed by single character.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", Dir: Displaying Contents section, p. 4-11
--- END ---

--- CARD ---
id:        semicolon-sequential-execution
type:      FACT
target:    68k
verify:    from-manual
topic:     command-separators
claim:     Semicolon (;) separator specifies sequential command execution: each command waits for previous to complete before starting.
context:   Separator character stripped from parameters; must quote if passing as program argument.
source:    Using Professional OS-9 v2.4, "The Shell", Shell Command Line Processing section, p. 5-8
--- END ---

--- CARD ---
id:        ampersand-concurrent-execution
type:      FACT
target:    68k
verify:    from-manual
topic:     multitasking
claim:     & (ampersand) separator enables concurrent execution: multiple commands run simultaneously (background execution).
context:   Enables multi-tasking within shell; programs run as independent processes.
source:    Using Professional OS-9 v2.4, "The Shell", Shell Command Line Processing section, p. 5-8
--- END ---

--- CARD ---
id:        foreground-process-requires-interaction
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     process-types
claim:     Foreground process is a task requiring user interaction (e.g., editing file, program prompting for input).
context:   One foreground process can run while multiple background processes execute.
source:    Using Professional OS-9 v2.4, "An Overview of OS-9", Multi-tasking section, p. 1-4
--- END ---

--- CARD ---
id:        background-process-no-attention
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     process-types
claim:     Background process is a task not requiring user attention (e.g., printing file, sorting datafiles); frees computer from single-task limitation.
context:   Printing in background while editing foreground file exemplifies efficient multi-tasking.
source:    Using Professional OS-9 v2.4, "An Overview of OS-9", Multi-tasking section, p. 1-4
--- END ---

--- CARD ---
id:        multitasking-cpu-switching-appearance
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     multitasking
claim:     Multi-tasking/multi-processing rapidly switches CPU between programs many times per second, making programs appear to run simultaneously.
context:   OS-9 manages resource sharing (memory, CPU time, I/O) without conflict.
source:    Using Professional OS-9 v2.4, "An Overview of OS-9", Multi-tasking section, p. 1-4 to 1-5
--- END ---

--- CARD ---
id:        timesharing-multiuser-extension
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     multitasking
claim:     Timesharing/multi-user operation extends multi-tasking to allow several people to use computer simultaneously; OS-9 provides additional security-related functions to control access and ensure privacy.
context:   No difference in OS-9 itself or application software between single-user/multi-tasking and multi-user/multi-tasking systems.
source:    Using Professional OS-9 v2.4, "An Overview of OS-9", Multi-tasking section, p. 1-5
--- END ---

--- CARD ---
id:        absolute-pathlist-root-slash
type:      FACT
target:    68k
verify:    from-manual
topic:     pathlist
claim:     Absolute pathlist begins with slash (/) representing root directory and proceeds through file structure to specified file or directory; does not depend on current directory.
context:   Example: /h0/usr/nicholle specifies path from root /h0 through USR to NICHOLLE subdirectory.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", Accessing Files section, p. 4-9
--- END ---

--- CARD ---
id:        relative-pathlist-current-directory
type:      FACT
target:    68k
verify:    from-manual
topic:     pathlist
claim:     Relative pathlist begins at current directory and proceeds up or down through file structure; does not begin with slash (/). Slashes separate names within relative pathlist.
context:   Example: doc/letters/jim begins in current directory.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", Accessing Files section, p. 4-9
--- END ---

--- CARD ---
id:        period-parent-directory-naming
type:      FACT
target:    68k
verify:    from-manual
topic:     pathlist
claim:     Single period (.) refers to current directory; two periods (..) refer to parent directory; add one period for each higher level (e.g., three periods for two levels up, four periods for three levels).
context:   Example: ../chap/page accesses chap directory in parent, then page file. Useful for moving up directory tree.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", Accessing Files section, p. 4-9
--- END ---

--- CARD ---
id:        period-substitution-not-rename
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     pathlist
claim:     Using period naming conventions (., .., ...) does not change the actual directory's name; they are only substitutes for navigation.
context:   A directory's actual name remains unchanged even when accessed via period conventions.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", Accessing Files section, p. 4-10
--- END ---

--- CARD ---
id:        period-exceeding-hierarchy-root
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     pathlist
claim:     Specifying more period levels than actually exist doesn't return error; instead indicates root directory. dir ......... displays root directory if user unsure of depth.
context:   Useful for safely navigating when uncertain of directory nesting depth.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", Climbing Directory Trees section, p. 4-14
--- END ---

--- CARD ---
id:        dir-utility-lists-directory-contents
type:      FACT
target:    68k
verify:    from-manual
topic:     file-commands
claim:     dir utility displays contents of directories; typing dir alone displays current data directory contents; add pathlist to view other directories without changing current data directory.
context:   Can use wildcards and options (-e for extended listing, -r for recursive, -x for execution directory).
source:    Using Professional OS-9 v2.4, "The OS-9 File System", Dir section, p. 4-11
--- END ---

--- CARD ---
id:        dir-extended-listing-attributes-size-sector
type:      FACT
target:    68k
verify:    from-manual
topic:     file-commands
claim:     dir -e option gives extended directory listing: displays files with owner, last modified date, attributes, sector where file is stored, and byte count.
context:   Provides detailed file metadata.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", Dir Options section, p. 4-12
--- END ---

--- CARD ---
id:        dir-recursive-subdirectories
type:      FACT
target:    68k
verify:    from-manual
topic:     file-commands
claim:     dir -r option displays contents of specified directory and all files within its subdirectories; options -e and -r can be combined (-er) for extended recursive listing.
context:   Useful for viewing entire directory trees.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", Dir Options section, p. 4-12
--- END ---

--- CARD ---
id:        pd-display-current-path
type:      FACT
target:    68k
verify:    from-manual
topic:     file-commands
claim:     pd utility displays complete pathlist from root directory to current data directory. pd -x displays pathlist to current execution directory.
context:   Helps when confusion arises about location in file system.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", Using the Pd Utility section, p. 4-16
--- END ---

--- CARD ---
id:        free-disk-space-sectors-bytes
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-management
claim:     free utility displays unused disk space in sectors and bytes, plus disk name, creation date, and cluster size. Uses 4K buffer by default; can increase with -b option (e.g., -b=10 for 10K).
context:   Essential for capacity planning; output includes largest free block size.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Free and Mfree section, p. 3-9
--- END ---

--- CARD ---
id:        mfree-memory-availability
type:      FACT
target:    68k
verify:    from-manual
topic:     memory-management
claim:     mfree displays address and size of unused memory available for allocation. mfree -e shows detailed memory info including minimum allocation size, number of memory segments, total RAM at startup, current total free RAM, and segment-by-segment memory map.
context:   Used to assess system memory availability before loading programs.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Free and Mfree section, p. 3-9
--- END ---

--- CARD ---
id:        help-utility-online-reference
type:      FACT
target:    68k
verify:    from-manual
topic:     utilities
claim:     help utility is on-line quick reference manual; type help <utility name> to display function, syntax, and available options. Alternatively, use <utility name> -? option (every utility supports -?).
context:   Essential for quick lookup without printed documentation.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", The Help Utility section, p. 3-8
--- END ---

--- CARD ---
id:        utilities-three-groups-usage-frequency
type:      FACT
target:    68k
verify:    from-manual
topic:     utilities
claim:     OS-9 utilities organized into three groups: Group 1 (Basic - learn immediately), Group 2 (Programmer - learn as time permits), Group 3 (System Management - learn as needed for advanced work). Over 70 utilities provided; casual users need less than a dozen frequently, about a dozen more occasionally.
context:   Prevents information overload for new users.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Basic Utilities section, p. 3-7
--- END ---

--- CARD ---
id:        directory-special-file-containing-names
type:      FACT
target:    68k
verify:    from-manual
topic:     file-system
claim:     Directory is a special file containing names and locations of files and directories it contains; can contain files and subdirectories in tree structure (hierarchical organization).
context:   This modular organizational approach scales efficiently.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", OS-9 File Storage section, p. 4-1
--- END ---

--- CARD ---
id:        file-pointer-sequential-access-pattern
type:      FACT
target:    68k
verify:    from-manual
topic:     file-system
claim:     Successive read or write operations transfer data sequentially because file pointer is automatically moved after each operation.
context:   This enables natural streaming access pattern.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", OS-9 File Storage section, p. 4-1
--- END ---

--- CARD ---
id:        allocation-map-sector-usage-tracking
type:      FACT
target:    68k
verify:    from-manual
topic:     file-system
claim:     Allocation map at beginning of disk tracks whether each sector is in use; when file is created or expanded, data stored in sectors; when file shortened or deleted, sectors marked as unused and available for reuse.
context:   Enables disk space reuse and fragmentation tracking.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", OS-9 File Storage section, p. 4-1
--- END ---

--- CARD ---
id:        text-file-bytes-characters
type:      FACT
target:    68k
verify:    from-manual
topic:     file-types
claim:     Within text file, each byte contains one character; data written in order provided and read exactly as stored.
context:   Simple sequential encoding for text data.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", OS-9 File Storage section, p. 4-1
--- END ---

--- CARD ---
id:        module-header-program-metadata
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Every executable program converted to machine language during compilation has module header containing program name, size, memory requirements, and other metadata.
context:   Shell uses module header to determine memory allocation and other execution parameters.
source:    Using Professional OS-9 v2.4, "The Shell", Shell Command Line Processing section, p. 5-9
--- END ---

--- CARD ---
id:        root-directory-device
type:      FACT
target:    68k
verify:    from-manual
topic:     file-system
claim:     Root directory on system (typically /h0) serves as top-level directory containing system directories (CMDS, DEFS, SYSMODS, etc.) and user directories.
context:   All other directories and files are organized hierarchically under root.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", Basic File System Oriented Utilities section, p. 4-11
--- END ---

--- CARD ---
id:        home-directory-user-base
type:      FACT
target:    68k
verify:    from-manual
topic:     user-directories
claim:     Home directory specified in password file entry is initial current data directory when user logs on; directory where chd with no parameters changes to if HOME environment variable is set.
context:   Personal workspace for each user on timesharing system.
source:    Using Professional OS-9 v2.4, "The Shell", The Shell Environment section, p. 5-3
--- END ---

--- CARD ---
id:        backup-procedure-source-destination-distinction
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-operations
claim:     During backup, source disk is read from and destination disk written to; if errors on first pass source has problem, if errors on second pass destination has problem.
context:   Useful for diagnosing disk failures.
source:    Using Professional OS-9 v2.4, "Starting OS-9", The Backup Procedure section, p. 2-5
--- END ---

--- CARD ---
id:        write-protect-tab-backup-safety
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-operations
claim:     Write protect tab can be used on source disk during backup procedure to prevent accidental confusion and swapping of source and destination disks.
context:   Safety practice to avoid data loss.
source:    Using Professional OS-9 v2.4, "Starting OS-9", The Backup Procedure section, p. 2-5
--- END ---

--- CARD ---
id:        format-bad-sectors-backup-warning
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     disk-operations
claim:     Never backup system disk to disk with any bad sectors reported by format; backup will likely fail.
context:   Check format output carefully before proceeding.
source:    Using Professional OS-9 v2.4, "Starting OS-9", Formatting a Disk section, p. 2-4
--- END ---

--- CARD ---
id:        absolute-path-slash-prefix-requirement
type:      FACT
target:    68k
verify:    from-manual
topic:     pathlist
claim:     Absolute pathlist must begin with slash (/); relative pathlist must not begin with slash.
context:   This distinction is critical for correct path resolution.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", Accessing Files section, p. 4-9
--- END ---

--- CARD ---
id:        environment-variable-case-sensitive
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     shell-environment
claim:     Environment variables are case-sensitive; OS-9 does not recognize variable if proper case not used.
context:   Example: PATH and path are different variables.
source:    Using Professional OS-9 v2.4, "The Shell", The Shell Environment section, p. 5-4
--- END ---

--- CARD ---
id:        command-line-modifier-characters-stripped
type:      GOTCHA
target:    68k
verify:    from-manual
topic:     shell-modifiers
claim:     Characters comprising execution modifiers (#, ^, >, <, >>), separators (;, &, !), and wildcards (*, ?) are stripped from command line parts passed to program as parameters; cannot be passed to programs unless contained in quotes.
context:   Quoting allows literal passing of these special characters.
source:    Using Professional OS-9 v2.4, "The Shell", Shell Command Line Processing section, p. 5-8
--- END ---

--- CARD ---
id:        module-directory-memory-loaded-modules
type:      FACT
target:    68k
verify:    from-manual
topic:     memory-management
claim:     Module directory in memory tracks all currently loaded modules; when program is executed, shell first checks module directory to see if program already loaded (avoiding reload).
context:   OS-9 automatically manages module sharing without user intervention.
source:    Using Professional OS-9 v2.4, "The Shell", Shell Command Line Processing section, p. 5-7
--- END ---

--- CARD ---
id:        priority-modifier-caret-execution
type:      FACT
target:    68k
verify:    from-manual
topic:     process-management
claim:     Caret (^) modifier in command line modifies process priority during execution.
context:   Allows per-command priority adjustment without permanent changes.
source:    Using Professional OS-9 v2.4, "The Shell", Shell Command Line Processing section, p. 5-8
--- END ---

--- CARD ---
id:        setpr-builtin-change-priority
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-commands
claim:     setpr <proc ID> <priority> built-in shell command changes process's priority after creation.
context:   Complements priority modifier for post-launch adjustment.
source:    Using Professional OS-9 v2.4, "The Shell", Built-in Shell Commands section, p. 5-6
--- END ---

--- CARD ---
id:        kill-builtin-abort-process
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-commands
claim:     kill <proc ID> built-in shell command aborts process specified by proc ID.
context:   Used to terminate running processes.
source:    Using Professional OS-9 v2.4, "The Shell", Built-in Shell Commands section, p. 5-6
--- END ---

--- CARD ---
id:        w-builtin-wait-child-process
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-commands
claim:     w built-in shell command waits for a single child process to terminate.
context:   Allows synchronization with background processes.
source:    Using Professional OS-9 v2.4, "The Shell", Built-in Shell Commands section, p. 5-6
--- END ---

--- CARD ---
id:        wait-builtin-all-children
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-commands
claim:     wait built-in shell command waits for all child processes to terminate.
context:   Ensures all background tasks complete before continuing.
source:    Using Professional OS-9 v2.4, "The Shell", Built-in Shell Commands section, p. 5-6
--- END ---

--- CARD ---
id:        logout-builtin-shell-termination
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-commands
claim:     logout built-in command terminates current shell. If login shell is to be terminated, .logout file in home directory is executed first, then shell is terminated.
context:   Allows cleanup procedures before exiting.
source:    Using Professional OS-9 v2.4, "The Shell", Built-in Shell Commands section, p. 5-6
--- END ---

--- CARD ---
id:        parent-child-process-inheritance
type:      FACT
target:    68k
verify:    from-manual
topic:     process-creation
claim:     New process (child) can only be created by existing process (parent); child inherits parent's standard I/O paths.
context:   On login, shell's standard input is terminal keyboard, standard output/error go to terminal display; child processes inherit these paths unless redirected.
source:    Using Professional OS-9 v2.4, "The Shell", I/O Redirection Modifiers section, p. 5-10
--- END ---

--- CARD ---
id:        set-builtin-shell-options
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-commands
claim:     set <options> built-in command sets options for shell; hyphen (-) unnecessary before letter options (e.g., set np turns off prompt, equivalent to -np).
context:   Alternative to command-line option entry; affects shell behavior for session.
source:    Using Professional OS-9 v2.4, "The Shell", The Shell Environment section, p. 5-4
--- END ---

--- CARD ---
id:        comment-character-asterisk-procedure
type:      FACT
target:    68k
verify:    from-manual
topic:     procedure-files
claim:     * <text> indicates a comment; <text> is not processed. Especially useful in procedure files.
context:   Allows documenting script intent.
source:    Using Professional OS-9 v2.4, "The Shell", Built-in Shell Commands section, p. 5-6
--- END ---

--- CARD ---
id:        operating-system-master-supervisor
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     os9-overview
claim:     Operating system is master supervisor of resources (memory, CPU time, I/O devices) and functions (interface, loading/executing programs, file/directory management, timesharing, memory allocation).
context:   OS-9 provides these services for microcomputers.
source:    Using Professional OS-9 v2.4, "An Overview of OS-9", What Is an Operating System section, p. 1-1
--- END ---

--- CARD ---
id:        utility-programs-housekeeping-management
type:      FACT
target:    68k
verify:    from-manual
topic:     utilities
claim:     Over 70 utility programs provided with OS-9 are not part of basic operating system, but are application programs providing essential housekeeping, management, customization, and maintenance functions.
context:   Examples include µMACS text editor and many system administration tools.
source:    Using Professional OS-9 v2.4, "An Overview of OS-9", Using OS-9 as Operating System section, p. 1-2
--- END ---

--- CARD ---
id:        system-call-vs-utility-programs
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     programming-interface
claim:     OS-9 capabilities accessed two ways: (1) utility command set and shell (user-friendly), (2) system calls (within programs in assembler/high-level language for advanced programmers).
context:   System calls available for process creation, file operations, etc.; described in Technical Manual.
source:    Using Professional OS-9 v2.4, "An Overview of OS-9", Using OS-9 Functions section, p. 1-2
--- END ---

--- CARD ---
id:        tree-structure-hierarchical-files
type:      CONCEPT
target:    68k
verify:    from-manual
topic:     file-system
claim:     OS-9 organizes files into tree-structured (hierarchical) directory organization: directories contain files and subdirectories, subdirectories contain other files and subdirectories.
context:   Provides natural way to organize related files by topic or workgroup.
source:    Using Professional OS-9 v2.4, "An Overview of OS-9", Storing Information section, p. 1-3
--- END ---

--- CARD ---
id:        io-devices-uniform-file-access
type:      FACT
target:    68k
verify:    from-manual
topic:     io-model
claim:     All OS-9 I/O devices including terminals are accessed as files, allowing EOF (end-of-file) signaling to work uniformly across all device types.
context:   This unified model simplifies program logic.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Using the Keyboard section, p. 3-6
--- END ---

--- CARD ---
id:        procs-utility-process-listing
type:      FACT
target:    68k
verify:    from-manual
topic:     process-management
claim:     procs utility lists currently running processes.
context:   Used to monitor system activity and process status.
source:    Using Professional OS-9 v2.4, "Basic Commands and Functions", Basic Utilities section, p. 3-7
--- END ---

--- CARD ---
id:        cfp-command-file-processor
type:      FACT
target:    68k
verify:    from-manual
topic:     utilities
claim:     cfp (command file processor) creates temporary procedure file and invokes shell to execute it; asterisk (*) in procedure file is replaced by given pathlists unless preceded by tilde (~).
context:   Allows template-based batch processing of multiple files.
source:    Using Professional OS-9 v2.4, "The OS-9 Utilities", cfp section, p. 12-13
--- END ---

--- CARD ---
id:        build-utility-text-file-creation
type:      FACT
target:    68k
verify:    from-manual
topic:     utilities
claim:     build utility creates file from standard input; displays ? prompt for each input line; empty line or EOF (typically <escape>) terminates file creation.
context:   Useful for creating short text files interactively.
source:    Using Professional OS-9 v2.4, "The OS-9 Utilities", build section, p. 11
--- END ---

--- CARD ---
id:        binex-exbin-s-record-conversion
type:      FACT
target:    68k
verify:    from-manual
topic:     utilities
claim:     binex converts binary files to S-record format; exbin converts S-record back to binary format. Used for downloading programs to devices like PROM programmers.
context:   S-record is portable hex format for binary data transfer.
source:    Using Professional OS-9 v2.4, "The OS-9 Utilities", binex/exbin section, p. 7-8
--- END ---

--- CARD ---
id:        break-system-debugger-rom
type:      FACT
target:    68k
verify:    from-manual
topic:     debugging
claim:     break utility executes F$SysDbg system call, stops OS-9 and all user processes, returns control to ROM debugger; only super users can execute. If no ROM debugger or disabled, resets system.
context:   Used only for system debugging, not production systems; must be invoked from system console.
source:    Using Professional OS-9 v2.4, "The OS-9 Utilities", break section, p. 10
--- END ---

--- CARD ---
id:        cmp-binary-file-comparison
type:      FACT
target:    68k
verify:    from-manual
topic:     utilities
claim:     cmp compares two binary files; displays file offset (hex address), hexadecimal values, and ASCII characters for any differences encountered; outputs byte count and difference count summary.
context:   Can use -b option for buffer size, -s for silent mode.
source:    Using Professional OS-9 v2.4, "The OS-9 Utilities", cmp section, p. 15
--- END ---

--- CARD ---
id:        task-process-running-program
type:      FACT
target:    68k
verify:    from-manual
topic:     process-types
claim:     Task or process is a running program in OS-9; system allows one or more tasks running in background while task runs in foreground.
context:   Each process has its own process ID and state.
source:    Using Professional OS-9 v2.4, "An Overview of OS-9", Multi-tasking section, p. 1-4
--- END ---

--- CARD ---
id:        console-system-device-debugger
type:      FACT
target:    68k
verify:    from-manual
topic:     system-configuration
claim:     ROM debugger only communicates with system's console device; break utility should only be called from console.
context:   If invoked from other terminal, must use console to communicate with debugger.
source:    Using Professional OS-9 v2.4, "The OS-9 Utilities", break section, p. 10
--- END ---

--- CARD ---
id:        sector-pre-defined-byte-group
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-storage
claim:     Sector is pre-defined group of bytes (e.g., 256 bytes); ordered sequence of bytes in file is grouped into sectors.
context:   Sector size is fixed per device and set during formatting.
source:    Using Professional OS-9 v2.4, "The OS-9 File System", OS-9 File Storage section, p. 4-1
--- END ---

--- CARD ---
id:        device-naming-convention-slash-d
type:      FACT
target:    68k
verify:    from-manual
topic:     system-naming
claim:     Disk drives conventionally named /d0, /d1, etc.; /d0 typically first drive (usually floppy), /d1 second drive.
context:   Naming conventions used throughout OS-9 documentation.
source:    Using Professional OS-9 v2.4, "Starting OS-9", Backing Up System Disk section, p. 2-3
--- END ---

--- CARD ---
id:        volume-name-format-disk-identification
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-operations
claim:     During format procedure, user is prompted to enter volume name for disk; volume name identifies disk in system.
context:   free utility displays volume name in its output.
source:    Using Professional OS-9 v2.4, "Starting OS-9", Formatting a Disk section, p. 2-4
--- END ---

--- CARD ---
id:        format-track-verification-bad-sectors
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-operations
claim:     During format verification phase, hexadecimal number of each track displayed as verified; if bad sectors found, error message shown with sector number; output includes good sector count, unusable sector count, and total verified sectors.
context:   Bad sectors indicate disk problems; format can map out bad sectors.
source:    Using Professional OS-9 v2.4, "Starting OS-9", Formatting a Disk section, p. 2-4 to 2-5
--- END ---

--- CARD ---
id:        ident-module-validity-check
type:      FACT
target:    68k
verify:    from-manual
topic:     utilities
claim:     ident utility checks validity of modules and can examine S-record files; binex and exbin can also check module validity.
context:   Used to verify module integrity before loading or running.
source:    Using Professional OS-9 v2.4, "The OS-9 Utilities", binex/exbin section, p. 7-8
--- END ---

--- CARD ---
id:        prompt-file-entry-dollar-sign
type:      FACT
target:    68k
verify:    from-manual
topic:     shell-interface
claim:     Shell prompt $ (dollar sign) is default system prompt indicating OS-9 ready for command input; prompt can be customized.
context:   All examples in this manual use $ prompt.
source:    Using Professional OS-9 v2.4, "Starting OS-9", Booting OS-9 section, p. 2-2
--- END ---
