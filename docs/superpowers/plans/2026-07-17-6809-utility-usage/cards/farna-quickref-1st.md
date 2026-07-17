--- CARD ---
id:        tmode-cannot-change-baud
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     TMODE displays but cannot modify baud rate, character length, parity, stop bits, or type parameters; these are for information only.
context:   Contrasts with XMODE which CAN change these same parameters permanently by deinit/xmode/reinit cycle.
source:    OS-9 Quick Reference (1st ed, Farna), TMODE section (line 951-958)
--- END ---

--- CARD ---
id:        xmode-can-change-baud
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     XMODE allows permanent changes to baud rate, character length (cs), parity, and stop bits if device is deinit/xmode/reinit'd.
context:   Explicit instruction: deiniz device, use xmode to change parameters, then iniz device for changes to take effect.
source:    OS-9 Quick Reference (1st ed, Farna), XMODE section (line 1074-1089)
--- END ---

--- CARD ---
id:        tmode-default-control-characters
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     TMODE documents specific default hex control characters: abort=$03 (Ctrl-C), eof=$1B (Escape), eor=$0D (CR), null=0 (no nulls after CR), bell=$07, bsp=$08 (Ctrl-H), del=$18 (Ctrl-X), dup=$01 (Ctrl-A).
context:   Provides actual defaults for terminal parameter configuration; useful for understanding default behavior.
source:    OS-9 Quick Reference (1st ed, Farna), TMODE parameters section (line 980-999)
--- END ---

--- CARD ---
id:        tsmon-device-limit
type:      FACT
target:    6809
verify:    from-manual
topic:     tsmon
claim:     TSMON can monitor up to 28 devices in a single process; multiple tsmon processes allowed for monitoring beyond 28 terminals.
context:   Hardware/system limit specific to timeshare terminal monitoring configuration.
source:    OS-9 Quick Reference (1st ed, Farna), TSMON section (line 1037-1050)
--- END ---

--- CARD ---
id:        pr-default-page-layout
type:      FACT
target:    6809
verify:    from-manual
topic:     pr
claim:     PR default page layout is 66 lines total: 1 header line, 5 blank lines, 55 text lines, 5 blank lines; right margin default is 79 characters.
context:   Precise formatting defaults for print output; changeable via -h, -p, -r options.
source:    OS-9 Quick Reference (1st ed, Farna), PR section (line 821-843)
--- END ---

--- CARD ---
id:        format-floppy-density-options
type:      FACT
target:    6809
verify:    from-manual
topic:     format
claim:     FORMAT supports floppy-specific density and side options: -sd (single density), -dd (double density), -ss (single sided), -ds (double sided).
context:   Floppy-specific disk formatting; standard on 6809 CoCo platforms.
source:    OS-9 Quick Reference (1st ed, Farna), FORMAT section (line 538-553)
--- END ---

--- CARD ---
id:        setime-flexible-separators
type:      FACT
target:    6809
verify:    from-manual
topic:     setime
claim:     SETIME accepts date/time with flexible separators: colons, semicolons, spaces, slashes, or commas; only space required between date and time portions.
context:   User-friendly time input; example given: "setime 940501 1330" or "setime 940501 130 pm" both work.
source:    OS-9 Quick Reference (1st ed, Farna), SETIME section (line 902-917)
--- END ---

--- CARD ---
id:        tape-option-execution-order
type:      FACT
target:    6809
verify:    from-manual
topic:     tape
claim:     TAPE executes options in fixed order: -z (read device), -f (skip tapemarks), -b (skip blocks), -w (write tapemarks), -e (erase), -r (rewind), -o (off-line).
context:   Multiple tape operations can be chained on one command line due to strict execution order.
source:    OS-9 Quick Reference (1st ed, Farna), TAPE section (line 927-943)
--- END ---

--- CARD ---
id:        backup-verify-default-on
type:      FACT
target:    6809
verify:    from-manual
topic:     backup
claim:     BACKUP performs verification by default; -v option description states "verify off", implying verification is enabled unless suppressed.
context:   Default safe behavior for backup operations; contrast with -r (continue on read error).
source:    OS-9 Quick Reference (1st ed, Farna), BACKUP section (line 234-241)
--- END ---

--- CARD ---
id:        expand-output-extension
type:      FACT
target:    6809
verify:    from-manual
topic:     expand
claim:     EXPAND with -n option creates decompressed output file with "_exp" suffix appended to original filename; default sends to stdout.
context:   File naming convention for expanded files matches COMPRESS's "_comp" suffix pattern.
source:    OS-9 Quick Reference (1st ed, Farna), EXPAND section (line 517-524)
--- END ---

--- CARD ---
id:        fixmod-packed-basic-option
type:      FACT
target:    6809
verify:    from-manual
topic:     fixmod
claim:     FIXMOD -ub option specifically fixes sys/rev field in packed BASIC09 subroutine modules; separate from general CRC update (-u).
context:   Specialized option for BASIC09 runtime modules; maintenance task for packed subroutines.
source:    OS-9 Quick Reference (1st ed, Farna), FIXMOD section (line 526-534)
--- END ---

--- CARD ---
id:        romsplit-default-16bit
type:      FACT
target:    6809
verify:    from-manual
topic:     romsplit
claim:     ROMSPLIT default assumes 16-bit ROM image, splitting into two 8-bit files named file.0 (even bytes) and file.L (odd bytes); -q option for 32-bit image into four files (.0, .L, .2, .3).
context:   ROM image splitting convention; .L extension for odd/low bytes, .0 for even.
source:    OS-9 Quick Reference (1st ed, Farna), ROMSPLIT section (line 882-888)
--- END ---

--- CARD ---
id:        date-julian-format-example
type:      FACT
target:    6809
verify:    from-manual
topic:     date
claim:     DATE -j (Julian) output format shown as "359,1995 2:30:00pm" for 25 DEC 1995; DATE -m (military) output shown as "December 25, 1995 Monday 14:30:00".
context:   Specific documented output formats for Julian vs military time display.
source:    OS-9 Quick Reference (1st ed, Farna), DATE section (line 349-354)
--- END ---

--- CARD ---
id:        tmode-parameter-reset-no-value
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     TMODE parameter with no value (no assignment) resets that parameter to its default; parameter set to 0 turns it off.
context:   Default reset mechanism; useful for restoring terminal settings to known state.
source:    OS-9 Quick Reference (1st ed, Farna), TMODE section (line 951-958)
--- END ---

--- CARD ---
id:        os9gen-oldboot-overwrite
type:      FACT
target:    6809
verify:    from-manual
topic:     os9gen
claim:     OS9GEN renames existing OS9Boot to OldBoot; if OldBoot already exists, it is overwritten without warning.
context:   File backup/overwrite behavior when creating bootable disks; no safety against accidental loss.
source:    OS-9 Quick Reference (1st ed, Farna), OS9GEN section (line 790-816)
--- END ---

--- CARD ---
id:        mdir-extended-format-columns
type:      FACT
target:    6809
verify:    from-manual
topic:     mdir
claim:     MDIR -e (extended) format displays columns: Addr, Size, Owner, Perm, Type, Revs, Ed#, Lnk, Module name; example shown for kernel module at address 002600.
context:   Specific output column order for module directory listing; useful for parsing or understanding memory layout.
source:    OS-9 Quick Reference (1st ed, Farna), MDIR section (line 737-754)
--- END ---

--- CARD ---
id:        make-option-case-sensitive
type:      FACT
target:    6809
verify:    from-manual
topic:     make
claim:     MAKE documentation states "Case dependant for directory and file names" when examining file dates and dependencies.
context:   Filesystem case sensitivity matters for make file dependency tracking; affects makefile portability.
source:    OS-9 Quick Reference (1st ed, Farna), MAKE section (line 722-735)
--- END ---

--- CARD ---
id:        dcheck-directory-depth
type:      FACT
target:    6809
verify:    from-manual
topic:     dcheck
claim:     DCHECK -d=(#) option controls directory traversal depth when printing file structure; without -d, full depth is printed.
context:   Useful for limiting output verbosity during disk structure checks.
source:    OS-9 Quick Reference (1st ed, Farna), DCHECK section (line 356-366)
--- END ---

--- CARD ---
id:        echo-newline-option-wording
type:      GOTCHA
type:      FACT
target:    6809
verify:    from-manual
topic:     echo
claim:     ECHO documentation for -n option states "separate text with carriage return" which is ambiguous but appears to mean "add CR separator between echo calls".
context:   OCR or manual wording unclear; intended meaning may be "do not separate with carriage return" as default behavior sends CR.
source:    OS-9 Quick Reference (1st ed, Farna), ECHO section (line 467-476)
--- END ---

--- CARD ---
id:        copy-abort-on-error-option
type:      FACT
target:    6809
verify:    from-manual
topic:     copy
claim:     COPY -a option "abort copy on error"; without -a, command displays "continue (y/n)" prompt on error unless -a also specified.
context:   Error handling behavior for file copy operations; -w option also triggers prompt unless -a suppresses it.
source:    OS-9 Quick Reference (1st ed, Farna), COPY section (line 324-336)
--- END ---

--- CARD ---
id:        pr-columns-per-page
type:      FACT
target:    6809
verify:    from-manual
topic:     pr
claim:     PR -k=(#) option sets number of columns for multi-column output; -m option prints multiple files side by side in columns.
context:   Multi-column output formatting available via combination of -k and -m options.
source:    OS-9 Quick Reference (1st ed, Farna), PR section (line 821-843)
--- END ---

--- CARD ---
id:        tmode-xon-xoff-defaults
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     TMODE documents xon/xoff defaults: xon=$11 (Ctrl-Q, DC1 resume), xoff=$13 (Ctrl-S, DC2 stop); both can be disabled by setting to 0.
context:   Software flow control defaults; example given: "tmode xon=0 xoff=0 bell=0" disables these features.
source:    OS-9 Quick Reference (1st ed, Farna), TMODE parameters section (line 998-1003)
--- END ---

--- CARD ---
id:        tmode-pause-character
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     TMODE pause parameter default is hex $17 (Ctrl-W) to temporarily halt video scrolling; pag parameter sets number of display lines for pause behavior.
context:   Terminal video control; interrelated with pause and pag (page lines) parameters.
source:    OS-9 Quick Reference (1st ed, Farna), TMODE parameters section (line 990-991)
--- END ---

--- CARD ---
id:        tmode-tab-stops-default
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     TMODE tabs parameter sets characters between tab stops; default is 4 characters per tab.
context:   Terminal tab formatting default; used in conjunction with tabc parameter (tab character, default $09/Ctrl-I).
source:    OS-9 Quick Reference (1st ed, Farna), TMODE parameters section (line 1001-1002)
--- END ---

--- CARD ---
id:        dir-recursive-depth-option
type:      FACT
target:    6809
verify:    from-manual
topic:     dir
claim:     DIR -r=(#) option recursively displays directories and filenames to specified depth; -r alone shows all depths; -r=-1 or similar not documented.
context:   Depth-limiting option for recursive directory traversal; similar to DCHECK -d.
source:    OS-9 Quick Reference (1st ed, Farna), DIR section (line 407-422)
--- END ---

--- CARD ---
id:        devs-output-column-order
type:      FACT
target:    6809
verify:    from-manual
topic:     devs
claim:     DEVS displays: device descriptor, driver, file manager, data ptr, link count; example shows "term" device with scf driver/file manager.
context:   Output order for system device table inspection; useful for understanding device configuration.
source:    OS-9 Quick Reference (1st ed, Farna), DEVS section (line 398-405)
--- END ---

