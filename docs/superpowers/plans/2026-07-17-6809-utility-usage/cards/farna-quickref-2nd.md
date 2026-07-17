--- CARD ---
id:        attr-permission-flags
type:      FACT
target:    6809
verify:    from-manual
topic:     attr
claim:     File permission options use single-letter codes (a=display, d=directory, e=owner-execute, r=owner-read, w=owner-write, s=non-shareable). Prefix any of e/r/w with 'p' to grant public access. Prefix with minus sign to revoke a permission.
context:   Page 5; confirms standard 6809 Level 2 attr flags, CoCo-specific application
source:    OS-9 Quick Reference (2nd ed, Farna), ATTR section (line 159-171)
--- END ---

--- CARD ---
id:        backup-verify-option
type:      FACT
target:    6809
verify:    from-manual
topic:     backup
claim:     The -v option to BACKUP turns verify off (double-negative); default is verify on.
context:   Line 179 states "-v - verify off"; this is the opposite of typical flag semantics
source:    OS-9 Quick Reference (2nd ed, Farna), BACKUP section (line 173-181)
--- END ---

--- CARD ---
id:        backup-buffer-size
type:      FACT
target:    6809
verify:    from-manual
topic:     backup
claim:     BACKUP accepts a buffer size parameter up to 56K in kilobytes (syntax #nK) to speed the backup process.
context:   Line 180 shows example "-v /d1 /d0 #40K"
source:    OS-9 Quick Reference (2nd ed, Farna), BACKUP section (line 180-181)
--- END ---

--- CARD ---
id:        copy-buffer-size
type:      FACT
target:    6809
verify:    from-manual
topic:     copy
claim:     COPY command accepts a buffer size parameter up to 56K in kilobytes (syntax #nK) to speed copying.
context:   Line 214 shows example "#40K" buffer
source:    OS-9 Quick Reference (2nd ed, Farna), COPY section (line 209-215)
--- END ---

--- CARD ---
id:        date-time-display
type:      FACT
target:    6809
verify:    from-manual
topic:     date
claim:     DATE command with 't' argument (syntax DATE t) displays both current system date and time together.
context:   Line 217; distinguishes bare DATE (date only) from DATE t (date and time)
source:    OS-9 Quick Reference (2nd ed, Farna), DATE section (line 217-218)
--- END ---

--- CARD ---
id:        dcheck-options-combined
type:      FACT
target:    6809
verify:    from-manual
topic:     dcheck
claim:     DCHECK options (prefixed with single dash) can be combined without spaces after the first dash. Example: -bpsw=/d1 combines -b, -p, -s, and -w=/d1.
context:   Line 229-230 explains combined syntax
source:    OS-9 Quick Reference (2nd ed, Farna), DCHECK section (line 229-233)
--- END ---

--- CARD ---
id:        dmode-stp-milliseconds
type:      FACT
target:    6809
verify:    from-manual
topic:     dmode
claim:     DMODE step rate parameter (stp) sets drive access time in milliseconds using hex values: 00=35ms, 01=20ms, 02=12ms, 03=6ms.
context:   CoCo-specific timing values; line 264
source:    OS-9 Quick Reference (2nd ed, Farna), DMODE section (line 259-275)
--- END ---

--- CARD ---
id:        dmode-typ-coco-format
type:      FACT
target:    6809
verify:    from-manual
topic:     dmode
claim:     DMODE drive type parameter (typ) uses hex value 20 to indicate 5.25-inch CoCo format disk drive.
context:   CoCo-specific designation; line 265
source:    OS-9 Quick Reference (2nd ed, Farna), DMODE section (line 265)
--- END ---

--- CARD ---
id:        dmode-dns-tpi
type:      FACT
target:    6809
verify:    from-manual
topic:     dmode
claim:     DMODE density parameter (dns) encodes tracks-per-inch: 1=40 tpi (single density), 3=96 tpi (double density).
context:   CoCo disk format specifics; line 266
source:    OS-9 Quick Reference (2nd ed, Farna), DMODE section (line 266)
--- END ---

--- CARD ---
id:        dmode-cyl-track-counts
type:      FACT
target:    6809
verify:    from-manual
topic:     dmode
claim:     DMODE cylinder parameter (cyl) encodes track counts: 23=35 tracks, 28=40 tracks, 50=80 tracks.
context:   CoCo disk geometry; line 267
source:    OS-9 Quick Reference (2nd ed, Farna), DMODE section (line 267)
--- END ---

--- CARD ---
id:        dmode-sct-standard-value
type:      FACT
target:    6809
verify:    from-manual
topic:     dmode
claim:     DMODE sectors-per-track parameter (sct) uses hex value 12 to specify 18 sectors per track (standard CoCo value).
context:   CoCo standard disk format; line 270
source:    OS-9 Quick Reference (2nd ed, Farna), DMODE section (line 270)
--- END ---

--- CARD ---
id:        dmode-t0s-track-zero
type:      FACT
target:    6809
verify:    from-manual
topic:     dmode
claim:     DMODE alternate parameter name tos (or t0s) specifies sectors-per-track for track 0 only, allowing asymmetric layouts.
context:   Undocumented variant; line 271 lists both "tos OR t0s"
source:    OS-9 Quick Reference (2nd ed, Farna), DMODE section (line 271)
--- END ---

--- CARD ---
id:        dmode-ilv-default
type:      FACT
target:    6809
verify:    from-manual
topic:     dmode
claim:     DMODE interleave factor parameter (ilv) normally defaults to 3 for CoCo floppy disks.
context:   Default value noted in parens; line 272
source:    OS-9 Quick Reference (2nd ed, Farna), DMODE section (line 272)
--- END ---

--- CARD ---
id:        dmode-sas-default
type:      FACT
target:    6809
verify:    from-manual
topic:     dmode
claim:     DMODE sectors-allocated-at-once parameter (sas) normally defaults to 8 for CoCo disk allocation.
context:   Default value noted in parens; line 273
source:    OS-9 Quick Reference (2nd ed, Farna), DMODE section (line 273)
--- END ---

--- CARD ---
id:        dmode-default-drives-warning
type:      FACT
target:    6809
verify:    from-manual
topic:     dmode
claim:     DMODE parameter changes to /d0 and /dd (default drive designator) require separate modifications to persist across boot; changes to /d1 alone do not affect defaults.
context:   Line 276 emphasizes that /d0 & /dd must be changed separately
source:    OS-9 Quick Reference (2nd ed, Farna), DMODE section (line 274-276)
--- END ---

--- CARD ---
id:        display-hex-special-values
type:      FACT
target:    6809
verify:    from-manual
topic:     display
claim:     DISPLAY hex values for screen control: hex 32=text mode, hex 33=background color, hex 34=border color. Foreground color hex codes can be used interchangeably for background; OS-9 translates automatically.
context:   Lines 281-288 show DISPLAY 1b commands for screen manipulation
source:    OS-9 Quick Reference (2nd ed, Farna), DISPLAY section (line 278-289)
--- END ---

--- CARD ---
id:        dsave-boot-backup
type:      FACT
target:    6809
verify:    from-manual
topic:     dsave
claim:     DSAVE option -b copies the source disk's boot file (OS9Boot) if present. Variant -b=path specifies an alternate source path for the boot file.
context:   Lines 318-319; boot file backup
source:    OS-9 Quick Reference (2nd ed, Farna), DSAVE section (line 315-330)
--- END ---

--- CARD ---
id:        dsave-verify-fork
type:      FACT
target:    6809
verify:    from-manual
topic:     dsave
claim:     DSAVE option -v verifies the copy by forking to CMP command after each file transfer.
context:   Line 324
source:    OS-9 Quick Reference (2nd ed, Farna), DSAVE section (line 324-325)
--- END ---

--- CARD ---
id:        del-exec-dir-flag
type:      FACT
target:    6809
verify:    from-manual
topic:     del
claim:     DEL command with -x flag assumes the file resides in the current execution directory (chx) rather than data directory.
context:   Line 239
source:    OS-9 Quick Reference (2nd ed, Farna), DEL section (line 238-241)
--- END ---

--- CARD ---
id:        dir-extended-listing
type:      FACT
target:    6809
verify:    from-manual
topic:     dir
claim:     DIR option 'e' displays extended details: file size, load address, owner ID, permissions, and last modification date/time.
context:   Line 253-254
source:    OS-9 Quick Reference (2nd ed, Farna), DIR section (line 251-257)
--- END ---

--- CARD ---
id:        dir-exec-dir-flag
type:      FACT
target:    6809
verify:    from-manual
topic:     dir
claim:     DIR option 'x' lists the current execution directory (chx) instead of data directory.
context:   Line 255
source:    OS-9 Quick Reference (2nd ed, Farna), DIR section (line 251-257)
--- END ---

--- CARD ---
id:        format-interleave-coco-default
type:      FACT
target:    6809
verify:    from-manual
topic:     format
claim:     Standard CoCo floppy disk interleave value is 3; FORMAT command uses this by default.
context:   Line 369 explicitly states this CoCo-specific value
source:    OS-9 Quick Reference (2nd ed, Farna), FORMAT section (line 359-371)
--- END ---

--- CARD ---
id:        format-track-count-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     format
claim:     FORMAT track count option uses bare numeric value in single quotes (syntax '#' where # is track count). Example: '35' for 35 tracks.
context:   Line 364 shows format: '#' - format # of tracks. Line 370 example: '35'
source:    OS-9 Quick Reference (2nd ed, Farna), FORMAT section (line 359-371)
--- END ---

--- CARD ---
id:        format-interleave-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     format
claim:     FORMAT interleave option uses colon delimiters (syntax ':#:' where # is the interleave value).
context:   Line 365 shows format: ':#:' - sector interleave value #
source:    OS-9 Quick Reference (2nd ed, Farna), FORMAT section (line 365)
--- END ---

--- CARD ---
id:        ident-module-crc-verify
type:      FACT
target:    6809
verify:    from-manual
topic:     ident
claim:     IDENT option -s displays edition byte, type/language byte, module CRC, and module name on one line. CRC verification shows period (.) if valid, question mark (?) if invalid.
context:   Line 383-384
source:    OS-9 Quick Reference (2nd ed, Farna), IDENT section (line 380-388)
--- END ---

--- CARD ---
id:        ident-skip-crc
type:      FACT
target:    6809
verify:    from-manual
topic:     ident
claim:     IDENT option -v skips module CRC verification.
context:   Line 386
source:    OS-9 Quick Reference (2nd ed, Farna), IDENT section (line 386)
--- END ---

--- CARD ---
id:        mdir-extended
type:      FACT
target:    6809
verify:    from-manual
topic:     mdir
claim:     MDIR option 'e' displays extended module information: physical address, size, type, revision level, re-entrant attribute, link count, and name. All values shown in hexadecimal.
context:   Line 448-449
source:    OS-9 Quick Reference (2nd ed, Farna), MDIR section (line 447-449)
--- END ---

--- CARD ---
id:        montype-display-modes
type:      FACT
target:    6809
verify:    from-manual
topic:     montype
claim:     MONTYPE command sets display output mode: 'r' for RGB, 'c' for composite color, 'm' for composite monochrome.
context:   Line 459-460; CoCo display option
source:    OS-9 Quick Reference (2nd ed, Farna), MONTYPE section (line 459-460)
--- END ---

--- CARD ---
id:        os9gen-input-redirection
type:      FACT
target:    6809
verify:    from-manual
topic:     os9gen
claim:     OS9GEN accepts module list input via stdin redirection (syntax: os9gen /dx /dx <textfile). Alternatively, can accept modules interactively one per line, terminated by BREAK.
context:   Lines 468-475 show both modes
source:    OS-9 Quick Reference (2nd ed, Farna), OS9GEN section (line 462-475)
--- END ---

--- CARD ---
id:        procs-extended
type:      FACT
target:    6809
verify:    from-manual
topic:     procs
claim:     PROCS option 'e' displays processes for all users (not just current user). Without option, shows only calling user's processes.
context:   Line 478
source:    OS-9 Quick Reference (2nd ed, Farna), PROCS section (line 477-478)
--- END ---

--- CARD ---
id:        pcdos-conversion
type:      FACT
target:    6809
verify:    from-manual
topic:     pcdos
claim:     PCDOS automatically converts text files between OS-9 and MS-DOS/Atari ST DOS formats during transfer. Use -raw option to prevent conversion.
context:   Lines 494, 503
source:    OS-9 Quick Reference (2nd ed, Farna), PCDOS section (line 490-506)
--- END ---

--- CARD ---
id:        rsdos-put-type-flags
type:      FACT
target:    6809
verify:    from-manual
topic:     rsdos
claim:     RSDOS -put command type options encode DECB file format: -b (BASIC binary, type 0, default), -d (BASIC data, type 1), -m (executable M/L, type 2), -t (text editor source, type 3), -a (ASCII text), -f=x (custom type 0-255).
context:   Lines 520-526; -b is default
source:    OS-9 Quick Reference (2nd ed, Farna), RSDOS section (line 511-529)
--- END ---

--- CARD ---
id:        setime-flexible-separators
type:      FACT
target:    6809
verify:    from-manual
topic:     setime
claim:     SETIME date/time separator flexibility: accepts colons, spaces, or slashes between date and time components, but NOT commas. No separators needed if just using space between date and time.
context:   Line 541-542; examples show "910501 1330" and "yy/mm/dd hh:mm(:ss)"
source:    OS-9 Quick Reference (2nd ed, Farna), SETIME section (line 539-543)
--- END ---

--- CARD ---
id:        tuneport-device-param
type:      FACT
target:    6809
verify:    from-manual
topic:     tuneport
claim:     TUNEPORT accepts device parameter for printer (/p) or terminal (/t1) to test and adjust serial port delay loop for proper baud rate.
context:   Line 556
source:    OS-9 Quick Reference (2nd ed, Farna), TUNEPORT section (line 555-561)
--- END ---

--- CARD ---
id:        tuneport-persistent-change
type:      FACT
target:    6809
verify:    from-manual
topic:     tuneport
claim:     TUNEPORT temporary changes to delay loop are lost on reboot. Use cobbler command to make changes permanent.
context:   Line 561
source:    OS-9 Quick Reference (2nd ed, Farna), TUNEPORT section (line 561)
--- END ---

--- CARD ---
id:        undel-case-sensitivity
type:      FACT
target:    6809
verify:    from-manual
topic:     undel
claim:     UNDEL command's first-letter recovery input is case-sensitive: uppercase letter undeletes a directory, lowercase undeletes a file. Using uppercase for a file incorrectly marks it with directory attribute; requires disk editor to fix.
context:   Lines 565-570; warning emphasized
source:    OS-9 Quick Reference (2nd ed, Farna), UNDEL section (line 563-571)
--- END ---

--- CARD ---
id:        unlink-merged-files
type:      FACT
target:    6809
verify:    from-manual
topic:     unlink
claim:     Modules in a merged file cannot be unlinked individually except for the "master" module (first in the group). Unlinking the master decrements the entire group's count. All merged modules show link count 0 except the master.
context:   Lines 577-580
source:    OS-9 Quick Reference (2nd ed, Farna), UNLINK section (line 573-580)
--- END ---

--- CARD ---
id:        tmode-path-numbers
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     TMODE operates on standard path numbers: .0 (input), .1 (output), .2 (error output).
context:   Line 589
source:    OS-9 Quick Reference (2nd ed, Farna), TMODE section (line 588-629)
--- END ---

--- CARD ---
id:        tmode-baud-encoding
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     TMODE baud parameter encodes rate, word length, and stop bits. Bits 0-3: baud rate (0=110, 1=300, 2=600, 3=1200, 4=2400, 5=4800, 6=9600, 7=19200 with ACIAPAK/32000 with SIO). Bits 5-6: word length (00=8, 01=7). Bit 7: stop bits (0=1, 1=2).
context:   Lines 605-610; technical bit layout
source:    OS-9 Quick Reference (2nd ed, Farna), TMODE section (line 605-610)
--- END ---

--- CARD ---
id:        tmode-type-acia-window
type:      FACT
target:    6809
verify:    from-manual
topic:     tmode
claim:     TMODE type parameter controls ACIA initialization and window mode. Bit 0: true lowercase for TERM-VDG (1=yes, 0=no). Set to 80 for window device (TERM-WIN). Bits 5-7: parity marking (101=MARK, 111=SPACE, 000=none, 011=even, 001=odd) for ACIA.
context:   Lines 623-628
source:    OS-9 Quick Reference (2nd ed, Farna), TMODE section (line 623-628)
--- END ---

--- CARD ---
id:        wcreate-screen-type-values
type:      FACT
target:    6809
verify:    from-manual
topic:     wcreate
claim:     WCREATE -s=type parameter defines screen resolution: 1=40-column text, 2=80-column text, 3=640x192 two-color graphics, 4=320x192 four-color graphics, 5=640x192 four-color graphics, 6=320x192 sixteen-color graphics.
context:   Lines 642-644; CoCo display modes
source:    OS-9 Quick Reference (2nd ed, Farna), WCREATE section (line 636-655)
--- END ---

--- CARD ---
id:        wcreate-size-constraints
type:      FACT
target:    6809
verify:    from-manual
topic:     wcreate
claim:     WCREATE xsize parameter ranges 1-80 columns. ysize parameter ranges 1-24 lines.
context:   Lines 647-648
source:    OS-9 Quick Reference (2nd ed, Farna), WCREATE section (line 647-648)
--- END ---

--- CARD ---
id:        wcreate-border-requires-screentype
type:      FACT
target:    6809
verify:    from-manual
topic:     wcreate
claim:     WCREATE border color must be specified only when using -s=type option. Default border is black if not specified with screen type.
context:   Lines 651-652
source:    OS-9 Quick Reference (2nd ed, Farna), WCREATE section (line 651-652)
--- END ---

--- CARD ---
id:        wmode-screen-type-params
type:      FACT
target:    6809
verify:    from-manual
topic:     wmode
claim:     WMODE uses same parameters as WCREATE but operates on already-initialized window. WMODE does NOT change items specified under XMODE or TMODE; those require separate commands.
context:   Lines 657-661
source:    OS-9 Quick Reference (2nd ed, Farna), WMODE section (line 657-661)
--- END ---

--- CARD ---
id:        xmode-tmode-difference
type:      FACT
target:    6809
verify:    from-manual
topic:     xmode
claim:     XMODE parameter changes persist for the current session only. TMODE changes also require the path to stay open to persist; once path closes, TMODE settings are lost.
context:   Lines 666-669
source:    OS-9 Quick Reference (2nd ed, Farna), XMODE section (line 663-669)
--- END ---

--- CARD ---
id:        keyboard-codes-ocr-garble
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     keyboard-codes
claim:     Line 738 shows a strange dash character that OCR likely corrupted. Original intent appears to be dash code "-2D" but character is garbled.
context:   OCR error; reading says "—2D" with em-dash, should verify against source
source:    OS-9 Quick Reference (2nd ed, Farna), Keyboard Codes section (line 738)
--- END ---

--- CARD ---
id:        attr-permission-word-typo
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     attr
claim:     Section text line 163 contains spelling error "driectory" (should be "directory"). Does not affect command functionality but indicates OCR transcription issue.
context:   OCR or original typo; appears in description of 'd' permission flag
source:    OS-9 Quick Reference (2nd ed, Farna), ATTR section (line 163)
--- END ---

--- CARD ---
id:        dcheck-typo-ommitted
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     dcheck
claim:     Line 228 contains spelling error "ommitted" (should be "omitted") in DCHECK -w parameter description.
context:   OCR or original typo
source:    OS-9 Quick Reference (2nd ed, Farna), DCHECK section (line 228)
--- END ---

--- CARD ---
id:        fsleep-sleep-zero-semantics
type:      FACT
target:    6809
verify:    from-manual
topic:     fsleep
claim:     FSleep system call with X=0 puts process into indefinite sleep (no wake-up without external signal). X=1 sleeps through current time slice only.
context:   Lines 1059-1060; special values for sleep duration parameter
source:    OS-9 Quick Reference (2nd ed, Farna), FSleep section (line 1056-1063)
--- END ---

--- CARD ---
id:        icreat-attribute-bits
type:      FACT
target:    6809
verify:    from-manual
topic:     icreat
claim:     ICreate attribute bits encode permissions: bit 0=read, 1=write, 2=execute, 3=public-read, 4=public-write, 5=public-execute, 6=shareable.
context:   Lines 1152-1156; file creation attribute encoding
source:    OS-9 Quick Reference (2nd ed, Farna), ICreate section (line 1147-1157)
--- END ---

--- CARD ---
id:        imakdir-attribute-bits
type:      FACT
target:    6809
verify:    from-manual
topic:     imakdir
claim:     IMakDir attribute bits encode directory permissions: bit 0=read, 1=write, 2=execute, 3=public-read, 4=public-write, 5=public-execute, 6=single-user, 7=any-user/type.
context:   Lines 1197-1201; directory creation differs from file attributes at bits 6-7
source:    OS-9 Quick Reference (2nd ed, Farna), IMakDir section (line 1193-1201)
--- END ---

--- CARD ---
id:        iopen-access-mode-codes
type:      FACT
target:    6809
verify:    from-manual
topic:     iopen
claim:     IOpen access mode parameter uses letter codes: D=directory, E=execute, R=read, S=not-shareable, PE=public-execute, PR=public-read, PW=public-write.
context:   Lines 1221-1224; access mode mnemonics
source:    OS-9 Quick Reference (2nd ed, Farna), IOpen section (line 1216-1224)
--- END ---

--- CARD ---
id:        fallo64-block-allocation
type:      FACT
target:    6809
verify:    from-manual
topic:     fallo64
claim:     FAll64 allocates 64-byte memory blocks; parameter X=0 means no page table allocated yet. Returns block number in A and block address in Y.
context:   Lines 1277-1279
source:    OS-9 Quick Reference (2nd ed, Farna), FAll64 section (line 1274-1281)
--- END ---

--- CARD ---
id:        gcmdir-internal-only-warning
type:      FACT
target:    6809
verify:    from-manual
topic:     fgcmdir
claim:     FGCMDir (compact module directory) is marked "FOR INTERNAL OS-9 USE ONLY! DO NOT CALL FROM PROGRAM!" indicating this system call should never be invoked by user programs.
context:   Lines 1431-1432; strong warning
source:    OS-9 Quick Reference (2nd ed, Farna), FGCMDir section (line 1430-1432)
--- END ---

--- CARD ---
id:        firq-polling-table
type:      FACT
target:    6809
verify:    from-manual
topic:     firq
claim:     FIRQ manages device polling table. Device packet at address X contains: flip byte at X, mask byte at X+1, priority byte at X+2. Set X=0 to remove device from polling.
context:   Lines 1460-1463; polling packet structure
source:    OS-9 Quick Reference (2nd ed, Farna), FIRQ section (line 1458-1463)
--- END ---

--- CARD ---
id:        fstabx-task-address-space
type:      FACT
target:    6809
verify:    from-manual
topic:     fstabx
claim:     FSTABX stores byte A in task B's address space at offset X (similar to M/L STA 0,X but X is resolved in target task's space, not current).
context:   Lines 1577-1579
source:    OS-9 Quick Reference (2nd ed, Farna), FSTABX section (line 1577-1579)
--- END ---

--- CARD ---
id:        ss-mouse-port-selection
type:      FACT
target:    6809
verify:    from-manual
topic:     ss-mouse
claim:     SS.Mouse function code 89 accepts Y parameter for port selection: 0=automatic, 1=right port, 2=left port.
context:   Lines 1624-1627; port selection encoding
source:    OS-9 Quick Reference (2nd ed, Farna), SS.Mouse section (line 1612-1627)
--- END ---

--- CARD ---
id:        ss-mouse-packet-valid-flag
type:      FACT
target:    6809
verify:    from-manual
topic:     ss-mouse
claim:     SS.Mouse data packet Pt.Valid field: 0=info not valid, 1=info valid. When Pt.Valid=0, Pt.Stat is 0 and not accurate.
context:   Lines 1630, 1659; validity checking
source:    OS-9 Quick Reference (2nd ed, Farna), SS.Mouse section (line 1630-1659)
--- END ---

--- CARD ---
id:        ss-mouse-button-time-tracking
type:      FACT
target:    6809
verify:    from-manual
topic:     ss-mouse
claim:     SS.Mouse button state tracking uses two time counters: Pt.TTSx = clock ticks in current state (starts at 1, increments while state unchanged), Pt.TLSx = ticks in opposite state. Allows button event classification (click/drag/hold) independent of state-change timing.
context:   Lines 1644-1648; allows detecting button events even if state changed between queries
source:    OS-9 Quick Reference (2nd ed, Farna), SS.Mouse section (line 1644-1648)
--- END ---

--- CARD ---
id:        ss-mouse-resolution-encoding
type:      FACT
target:    6809
verify:    from-manual
topic:     ss-mouse
claim:     SS.Mouse Pt.Res resolution field: 0=low (X resolution 0-639 with 1:10 ratio, Y 1:3 ratio), 1=high (X,Y 0-191, 1:1 ratio). High-resolution requires Hi-Res adapter or keyboard mouse.
context:   Lines 1660-1661; resolution is device-dependent
source:    OS-9 Quick Reference (2nd ed, Farna), SS.Mouse section (line 1660-1661)
--- END ---

--- CARD ---
id:        ss-mouse-window-relative-coords
type:      FACT
target:    6809
verify:    from-manual
topic:     ss-mouse
claim:     SS.Mouse window-relative coordinates (Pt.WRX, Pt.WRY) are calculated as mouse position minus current window's top-left coordinates. Divide by 8 to get absolute character positions.
context:   Lines 1666-1668
source:    OS-9 Quick Reference (2nd ed, Farna), SS.Mouse section (line 1666-1668)
--- END ---
