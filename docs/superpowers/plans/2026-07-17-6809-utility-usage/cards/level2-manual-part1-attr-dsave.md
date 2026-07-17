--- CARD ---
id:        attr-examine-or-change-permissions
type:      FACT
target:    6809
verify:    from-manual
topic:     attr
claim:     ATTR examines or changes a file's security permissions via command-line options.
context:   None
source:    OS-9 Level 2 Operating System Manual, ATTR section, line 5638
--- END ---

--- CARD ---
id:        attr-syntax-filename-and-permission
type:      FACT
target:    6809
verify:    from-manual
topic:     attr
claim:     Syntax is 'attr filename [permission]' where filename is the target and permission specifies which attributes to set.
context:   None
source:    OS-9 Level 2 Operating System Manual, ATTR section, line 5635
--- END ---

--- CARD ---
id:        attr-owner-read-flag
type:      FACT
target:    6809
verify:    from-manual
topic:     attr
claim:     Option 'r' restricts read access to the owner only.
context:   None
source:    OS-9 Level 2 Operating System Manual, ATTR section, line 5659
--- END ---

--- CARD ---
id:        attr-owner-write-flag
type:      FACT
target:    6809
verify:    from-manual
topic:     attr
claim:     Option 'w' restricts write access to the owner only.
context:   None
source:    OS-9 Level 2 Operating System Manual, ATTR section, line 5661
--- END ---

--- CARD ---
id:        attr-owner-execute-flag
type:      FACT
target:    6809
verify:    from-manual
topic:     attr
claim:     Option 'e' restricts execute access to the owner only.
context:   None
source:    OS-9 Level 2 Operating System Manual, ATTR section, line 5664
--- END ---

--- CARD ---
id:        attr-public-read-flag
type:      FACT
target:    6809
verify:    from-manual
topic:     attr
claim:     Option 'pr' grants read access to anyone (public).
context:   None
source:    OS-9 Level 2 Operating System Manual, ATTR section, line 5666
--- END ---

--- CARD ---
id:        attr-public-write-flag
type:      FACT
target:    6809
verify:    from-manual
topic:     attr
claim:     Option 'pw' grants write access to anyone (public).
context:   None
source:    OS-9 Level 2 Operating System Manual, ATTR section, line 5668
--- END ---

--- CARD ---
id:        attr-public-execute-flag
type:      FACT
target:    6809
verify:    from-manual
topic:     attr
claim:     Option 'pe' grants execute access to anyone (public).
context:   None
source:    OS-9 Level 2 Operating System Manual, ATTR section, line 5670
--- END ---

--- CARD ---
id:        attr-directory-flag
type:      FACT
target:    6809
verify:    from-manual
topic:     attr
claim:     Option 'ad' marks a file as a directory.
context:   OCR unclear: line 5655 reads "ad" but may be garbled; needs live verification if this is the actual option letter.
source:    OS-9 Level 2 Operating System Manual, ATTR section, line 5655
--- END ---

--- CARD ---
id:        attr-nonshareable-flag
type:      FACT
target:    6809
verify:    from-manual
topic:     attr
claim:     Option '8' marks a file as non-shareable, allowing only one user at a time.
context:   None
source:    OS-9 Level 2 Operating System Manual, ATTR section, line 5656
--- END ---

--- CARD ---
id:        attr-silent-mode-flag
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     attr
claim:     An option exists to suppress attribute display during change operations.
context:   OCR unclear: line 5672 shows '"a' which appears to be a quote character rendering artifact; exact option letter needs live verification.
source:    OS-9 Level 2 Operating System Manual, ATTR section, line 5672
--- END ---

--- CARD ---
id:        attr-minus-prefix-disables-option
type:      FACT
target:    6809
verify:    from-manual
topic:     attr
claim:     Prefix any permission option with a minus sign to disable (turn off) that attribute.
context:   None
source:    OS-9 Level 2 Operating System Manual, ATTR section, line 5689
--- END ---

--- CARD ---
id:        attr-no-params-shows-current
type:      FACT
target:    6809
verify:    from-manual
topic:     attr
claim:     Running ATTR with only a filename and no permission options displays the file's current attributes.
context:   None
source:    OS-9 Level 2 Operating System Manual, ATTR section, line 5693
--- END ---

--- CARD ---
id:        attr-ownership-required
type:      FACT
target:    6809
verify:    from-manual
topic:     attr
claim:     Only a file's owner can change its attributes (except user 0, who can change any file's attributes).
context:   None
source:    OS-9 Level 2 Operating System Manual, ATTR section, line 5696
--- END ---

--- CARD ---
id:        attr-dir-to-file-conversion
type:      FACT
target:    6809
verify:    from-manual
topic:     attr
claim:     After deleting all files from a directory, ATTR can convert the now-empty directory back to a regular file.
context:   None
source:    OS-9 Level 2 Operating System Manual, ATTR section, line 5699
--- END ---

--- CARD ---
id:        attr-cannot-convert-file-to-dir
type:      FACT
target:    6809
verify:    from-manual
topic:     attr
claim:     ATTR cannot convert a regular file into a directory; use MAKDIR for that instead.
context:   None
source:    OS-9 Level 2 Operating System Manual, ATTR section, line 5700
--- END ---

--- CARD ---
id:        backup-sector-by-sector-copy
type:      FACT
target:    6809
verify:    from-manual
topic:     backup
claim:     BACKUP performs a sector-by-sector copy, treating both disks as raw storage and ignoring file structure.
context:   None
source:    OS-9 Level 2 Operating System Manual, BACKUP section, line 5772
--- END ---

--- CARD ---
id:        backup-device-format-requirement
type:      FACT
target:    6809
verify:    from-manual
topic:     backup
claim:     Source and destination disks must have the same format (size, density, etc.) for BACKUP to succeed.
context:   None
source:    OS-9 Level 2 Operating System Manual, BACKUP section, line 5773
--- END ---

--- CARD ---
id:        backup-destination-defect-risk
type:      FACT
target:    6809
verify:    from-manual
topic:     backup
claim:     The destination disk must not contain defective sectors, or the backup operation will fail.
context:   None
source:    OS-9 Level 2 Operating System Manual, BACKUP section, line 5775
--- END ---

--- CARD ---
id:        backup-default-devices
type:      FACT
target:    6809
verify:    from-manual
topic:     backup
claim:     If no device names are provided, BACKUP defaults to copying from /D0 to /D1.
context:   None
source:    OS-9 Level 2 Operating System Manual, BACKUP section, line 5784
--- END ---

--- CARD ---
id:        backup-single-drive-option
type:      FACT
target:    6809
verify:    from-manual
topic:     backup
claim:     The -s option enables single-drive backup, reading source content into memory, then prompting for diskette swaps.
context:   None
source:    OS-9 Level 2 Operating System Manual, BACKUP section, line 5755
--- END ---

--- CARD ---
id:        backup-cancel-on-error-option
type:      FACT
target:    6809
verify:    from-manual
topic:     backup
claim:     An option exists to cancel the backup immediately if a read error is encountered.
context:   OCR unclear: line 5753 shows option but letter is not clearly visible; needs live verification.
source:    OS-9 Level 2 Operating System Manual, BACKUP section, line 5753
--- END ---

--- CARD ---
id:        backup-no-verify-option
type:      FACT
target:    6809
verify:    from-manual
topic:     backup
claim:     The -v option disables verification, telling BACKUP not to verify data written to the destination disk.
context:   None
source:    OS-9 Level 2 Operating System Manual, BACKUP section, line 5758
--- END ---

--- CARD ---
id:        backup-memory-allocation-option
type:      FACT
target:    6809
verify:    from-manual
topic:     backup
claim:     Option #nK or #n sets memory allocation: n is either pages of 256 bytes (without K) or kilobytes (with K suffix).
context:   None
source:    OS-9 Level 2 Operating System Manual, BACKUP section, line 5761
--- END ---

--- CARD ---
id:        backup-memory-speedup
type:      FACT
target:    6809
verify:    from-manual
topic:     backup
claim:     Increasing memory allocation to BACKUP speeds up the copy operation and reduces diskette swaps for single-drive backup.
context:   None
source:    OS-9 Level 2 Operating System Manual, BACKUP section, line 5763
--- END ---

--- CARD ---
id:        backup-memory-limit
type:      FACT
target:    6809
verify:    from-manual
topic:     backup
claim:     BACKUP can be allocated up to 56 kilobytes of memory; requesting more than available returns error 207.
context:   None
source:    OS-9 Level 2 Operating System Manual, BACKUP section, line 5821
--- END ---

--- CARD ---
id:        build-text-file-from-keyboard
type:      FACT
target:    6809
verify:    from-manual
topic:     build
claim:     BUILD creates a new text file by copying input typed at the keyboard, line by line.
context:   None
source:    OS-9 Level 2 Operating System Manual, BUILD section, line 5891
--- END ---

--- CARD ---
id:        build-syntax-filename-only
type:      FACT
target:    6809
verify:    from-manual
topic:     build
claim:     Syntax is 'build filename' with no options.
context:   None
source:    OS-9 Level 2 Operating System Manual, BUILD section, line 5888
--- END ---

--- CARD ---
id:        build-prompt-for-input
type:      FACT
target:    6809
verify:    from-manual
topic:     build
claim:     BUILD displays a question mark prompt and waits for the user to type a line; pressing ENTER records the line to disk.
context:   None
source:    OS-9 Level 2 Operating System Manual, BUILD section, line 5905
--- END ---

--- CARD ---
id:        build-empty-line-closes-file
type:      FACT
target:    6809
verify:    from-manual
topic:     build
claim:     Pressing ENTER on an empty line (no preceding text) closes the file and terminates BUILD.
context:   None
source:    OS-9 Level 2 Operating System Manual, BUILD section, line 5910
--- END ---

--- CARD ---
id:        chd-changes-data-directory
type:      FACT
target:    6809
verify:    from-manual
topic:     chd
claim:     CHD changes the current working (data) directory to the specified pathlist.
context:   None
source:    OS-9 Level 2 Operating System Manual, CHD/CHX section, line 5985
--- END ---

--- CARD ---
id:        chx-changes-execution-directory
type:      FACT
target:    6809
verify:    from-manual
topic:     chx
claim:     CHX changes the current execution directory to the specified pathlist.
context:   None
source:    OS-9 Level 2 Operating System Manual, CHD/CHX section, line 5986
--- END ---

--- CARD ---
id:        chd-chx-shell-built-in
type:      FACT
target:    6809
verify:    from-manual
topic:     chd
claim:     CHD and CHX are shell built-in commands and do not appear in the CMDS directory as external files.
context:   None
source:    OS-9 Level 2 Operating System Manual, CHD/CHX section, line 5999
--- END ---

--- CARD ---
id:        chd-chx-parent-directory
type:      FACT
target:    6809
verify:    from-manual
topic:     chd
claim:     Both CHD and CHX accept '..' as a pathlist to move to the parent directory.
context:   None
source:    OS-9 Level 2 Operating System Manual, CHD/CHX section, line 6016
--- END ---

--- CARD ---
id:        cmp-binary-byte-comparison
type:      FACT
target:    6809
verify:    from-manual
topic:     cmp
claim:     CMP opens two files and compares the binary values of corresponding bytes, reporting any differences found.
context:   None
source:    OS-9 Level 2 Operating System Manual, CMP section, line 6054
--- END ---

--- CARD ---
id:        cmp-displays-offset-and-values
type:      FACT
target:    6809
verify:    from-manual
topic:     cmp
claim:     For each byte difference, CMP displays the file offset (address) and the byte values from each file.
context:   None
source:    OS-9 Level 2 Operating System Manual, CMP section, line 6056
--- END ---

--- CARD ---
id:        cmp-stops-at-eof
type:      FACT
target:    6809
verify:    from-manual
topic:     cmp
claim:     CMP ends comparison when it encounters end-of-file in either file and displays a summary.
context:   None
source:    OS-9 Level 2 Operating System Manual, CMP section, line 6071
--- END ---

--- CARD ---
id:        cmp-summary-stats
type:      FACT
target:    6809
verify:    from-manual
topic:     cmp
claim:     CMP's summary displays total bytes compared and total differences found.
context:   None
source:    OS-9 Level 2 Operating System Manual, CMP section, line 6072
--- END ---

--- CARD ---
id:        cobbler-creates-boot-file
type:      FACT
target:    6809
verify:    from-manual
topic:     cobbler
claim:     COBBLER creates the OS9Boot file required on any bootable OS-9 diskette.
context:   Hardware-specific: CoCo/Dragon Level 2 utility.
source:    OS-9 Level 2 Operating System Manual, COBBLER section, line 6133
--- END ---

--- CARD ---
id:        cobbler-copies-loaded-modules
type:      FACT
target:    6809
verify:    from-manual
topic:     cobbler
claim:     COBBLER creates the new boot file using the same modules that were loaded during the most recent system bootstrap.
context:   Hardware-specific: CoCo/Dragon Level 2 utility.
source:    OS-9 Level 2 Operating System Manual, COBBLER section, line 6147
--- END ---

--- CARD ---
id:        cobbler-writes-kernel-track34
type:      FACT
target:    6809
verify:    from-manual
topic:     cobbler
claim:     COBBLER writes the OS-9 kernel on Track 34 and excludes those sectors from the disk's allocation map.
context:   Hardware-specific: CoCo/Dragon Level 2 utility.
source:    OS-9 Level 2 Operating System Manual, COBBLER section, line 6150
--- END ---

--- CARD ---
id:        cobbler-contiguous-boot-file
type:      FACT
target:    6809
verify:    from-manual
topic:     cobbler
claim:     The new boot file must be contiguous on the diskette; COBBLER should only be used on newly-formatted diskettes.
context:   Hardware-specific: CoCo/Dragon Level 2 utility.
source:    OS-9 Level 2 Operating System Manual, COBBLER section, line 6156
--- END ---

--- CARD ---
id:        cobbler-corruption-risk
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     cobbler
claim:     Using COBBLER on a non-newly-formatted diskette can destroy the old boot file if no contiguous block is large enough, making the diskette unbootable.
context:   Hardware-specific: CoCo/Dragon Level 2 utility.
source:    OS-9 Level 2 Operating System Manual, COBBLER section, line 6158
--- END ---

--- CARD ---
id:        cobbler-use-xmode-first
type:      FACT
target:    6809
verify:    from-manual
topic:     cobbler
claim:     Use XMODE to change device attributes permanently before running COBBLER.
context:   Hardware-specific: CoCo/Dragon Level 2 utility.
source:    OS-9 Level 2 Operating System Manual, COBBLER section, line 6163
--- END ---

--- CARD ---
id:        config-custom-system-diskette
type:      FACT
target:    6809
verify:    from-manual
topic:     config
claim:     CONFIG creates a custom system diskette containing only the device drivers and commands selected by the user.
context:   Hardware-specific: CoCo/Dragon Level 2 utility; interactive menu-based.
source:    OS-9 Level 2 Operating System Manual, CONFIG section, line 6198
--- END ---

--- CARD ---
id:        config-no-parameters
type:      FACT
target:    6809
verify:    from-manual
topic:     config
claim:     CONFIG takes no command-line parameters; user establishes all parameters interactively during execution.
context:   Hardware-specific: CoCo/Dragon Level 2 utility.
source:    OS-9 Level 2 Operating System Manual, CONFIG section, line 6224
--- END ---

--- CARD ---
id:        config-display-adjustment
type:      FACT
target:    6809
verify:    from-manual
topic:     config
claim:     CONFIG automatically adjusts its screen display to work with either 32-column or 80-column terminals.
context:   Hardware-specific: CoCo/Dragon Level 2 utility.
source:    OS-9 Level 2 Operating System Manual, CONFIG section, line 6200
--- END ---

--- CARD ---
id:        config-module-extensions
type:      FACT
target:    6809
verify:    from-manual
topic:     config
claim:     Module files are identified by extension: .dd (Device Descriptor), .dr (Device Driver), .io (I/O subroutine), .hp (Help), .dw (Window Descriptor), .dt (Terminal Descriptor).
context:   Hardware-specific: CoCo/Dragon Level 2 utility.
source:    OS-9 Level 2 Operating System Manual, CONFIG section, line 6248
--- END ---

--- CARD ---
id:        config-no-fragmentation
type:      FACT
target:    6809
verify:    from-manual
topic:     config
claim:     CONFIG produces no diskette fragmentation, unlike using BACKUP plus selective deletion.
context:   Hardware-specific: CoCo/Dragon Level 2 utility.
source:    OS-9 Level 2 Operating System Manual, CONFIG section, line 6233
--- END ---

--- CARD ---
id:        copy-file-or-device-copy
type:      FACT
target:    6809
verify:    from-manual
topic:     copy
claim:     COPY transfers data from one file or device to another file or device.
context:   None
source:    OS-9 Level 2 Operating System Manual, COPY section, line 6413
--- END ---

--- CARD ---
id:        copy-creates-destination-file
type:      FACT
target:    6809
verify:    from-manual
topic:     copy
claim:     If the destination is a file, COPY automatically creates it; the file must not already exist.
context:   None
source:    OS-9 Level 2 Operating System Manual, COPY section, line 6455
--- END ---

--- CARD ---
id:        copy-preserves-data-unchanged
type:      FACT
target:    6809
verify:    from-manual
topic:     copy
claim:     COPY copies data without modifying it, regardless of data type.
context:   None
source:    OS-9 Level 2 Operating System Manual, COPY section, line 6456
--- END ---

--- CARD ---
id:        copy-no-line-feed-addition
type:      FACT
target:    6809
verify:    from-manual
topic:     copy
claim:     COPY does not add important codes such as line feeds to the output; use LIST instead when sending text to a terminal or printer.
context:   None
source:    OS-9 Level 2 Operating System Manual, COPY section, line 6460
--- END ---

--- CARD ---
id:        copy-single-drive-option
type:      FACT
target:    6809
verify:    from-manual
topic:     copy
claim:     The -s option enables single-drive copy mode, which requires a full pathlist for the destination and prompts for diskette swaps.
context:   None
source:    OS-9 Level 2 Operating System Manual, COPY section, line 6431
--- END ---

--- CARD ---
id:        copy-memory-allocation-option
type:      FACT
target:    6809
verify:    from-manual
topic:     copy
claim:     Option #nK or #n allocates memory: n is either 256-byte pages (without K) or kilobytes (with K).
context:   None
source:    OS-9 Level 2 Operating System Manual, COPY section, line 6440
--- END ---

--- CARD ---
id:        copy-memory-limits
type:      FACT
target:    6809
verify:    from-manual
topic:     copy
claim:     COPY can be allocated up to 56 kilobytes of memory; more memory increases speed and reduces diskette swaps for single-drive copies.
context:   None
source:    OS-9 Level 2 Operating System Manual, COPY section, line 6479
--- END ---

--- CARD ---
id:        date-displays-current-date
type:      FACT
target:    6809
verify:    from-manual
topic:     date
claim:     DATE displays the current system date.
context:   None
source:    OS-9 Level 2 Operating System Manual, DATE section, line 6512
--- END ---

--- CARD ---
id:        date-time-option
type:      FACT
target:    6809
verify:    from-manual
topic:     date
claim:     Option 't' causes DATE to display the time along with the date.
context:   None
source:    OS-9 Level 2 Operating System Manual, DATE section, line 6517
--- END ---

--- CARD ---
id:        date-uses-setime
type:      FACT
target:    6809
verify:    from-manual
topic:     date
claim:     Use SETIME to set a new system date and time; use DATE to verify the setting.
context:   None
source:    OS-9 Level 2 Operating System Manual, DATE section, line 6524
--- END ---

--- CARD ---
id:        dcheck-checks-disk-structure
type:      FACT
target:    6809
verify:    from-manual
topic:     dcheck
claim:     DCHECK verifies the integrity of a disk's file structure by following the directory/file tree.
context:   None
source:    OS-9 Level 2 Operating System Manual, DCHECK section, line 6575
--- END ---

--- CARD ---
id:        dcheck-detects-orphaned-sectors
type:      FACT
target:    6809
verify:    from-manual
topic:     dcheck
claim:     DCHECK can detect orphaned sectors that are allocated but not part of the file structure (orphaned clusters).
context:   None
source:    OS-9 Level 2 Operating System Manual, DCHECK section, line 6610
--- END ---

--- CARD ---
id:        dcheck-verifies-descriptors
type:      FACT
target:    6809
verify:    from-manual
topic:     dcheck
claim:     DCHECK verifies the integrity of file descriptor sectors and reports discrepancies in directory/file links.
context:   None
source:    OS-9 Level 2 Operating System Manual, DCHECK section, line 6619
--- END ---

--- CARD ---
id:        dcheck-builds-allocation-map
type:      FACT
target:    6809
verify:    from-manual
topic:     dcheck
claim:     DCHECK builds a sector allocation map from the segment list in each file and compares it with the disk's stored allocation map.
context:   None
source:    OS-9 Level 2 Operating System Manual, DCHECK section, line 6620
--- END ---

--- CARD ---
id:        dcheck-detects-duplicate-clusters
type:      FACT
target:    6809
verify:    from-manual
topic:     dcheck
claim:     DCHECK ensures each disk cluster appears once and only once; duplicate allocations are detected and reported.
context:   None
source:    OS-9 Level 2 Operating System Manual, DCHECK section, line 6640
--- END ---

--- CARD ---
id:        dcheck-count-option
type:      FACT
target:    6809
verify:    from-manual
topic:     dcheck
claim:     Option -8 causes DCHECK to count directories and files and check file descriptors only, skipping full structural verification.
context:   None
source:    OS-9 Level 2 Operating System Manual, DCHECK section, line 5588
--- END ---

--- CARD ---
id:        dcheck-suppress-unused-option
type:      FACT
target:    6809
verify:    from-manual
topic:     dcheck
claim:     Option -b suppresses listing of unused clusters (sectors allocated but not part of the file structure).
context:   None
source:    OS-9 Level 2 Operating System Manual, DCHECK section, line 6594
--- END ---

--- CARD ---
id:        dcheck-pathlist-option
type:      FACT
target:    6809
verify:    from-manual
topic:     dcheck
claim:     Option -P causes DCHECK to print pathlists for clusters identified as questionable.
context:   None
source:    OS-9 Level 2 Operating System Manual, DCHECK section, line 6597
--- END ---

--- CARD ---
id:        dcheck-work-directory-option
type:      FACT
target:    6809
verify:    from-manual
topic:     dcheck
claim:     Option -w=pathlist specifies a directory where DCHECK will place its allocation map work files.
context:   None
source:    OS-9 Level 2 Operating System Manual, DCHECK section, line 6599
--- END ---

--- CARD ---
id:        dcheck-save-workfiles-option
type:      FACT
target:    6809
verify:    from-manual
topic:     dcheck
claim:     Option -m preserves DCHECK's allocation map work files after the check completes.
context:   None
source:    OS-9 Level 2 Operating System Manual, DCHECK section, line 6601
--- END ---

--- CARD ---
id:        dcheck-help-option
type:      FACT
target:    6809
verify:    from-manual
topic:     dcheck
claim:     Option -0 prints DCHECK's valid options.
context:   None
source:    OS-9 Level 2 Operating System Manual, DCHECK section, line 6603
--- END ---

--- CARD ---
id:        dcheck-workfile-names
type:      FACT
target:    6809
verify:    from-manual
topic:     dcheck
claim:     DCHECK creates work files named DCHECK<pp>0 (allocation bitmap) and optionally DCHECK<pp>1 (questionable clusters), where <pp> is the process number in hexadecimal.
context:   None
source:    OS-9 Level 2 Operating System Manual, DCHECK section, line 6680
--- END ---

--- CARD ---
id:        dcheck-exclusive-access
type:      FACT
target:    6809
verify:    from-manual
topic:     dcheck
claim:     DCHECK requires exclusive disk access for best results; concurrent access can fool the check.
context:   None
source:    OS-9 Level 2 Operating System Manual, DCHECK section, line 6693
--- END ---

--- CARD ---
id:        dcheck-directory-depth-limit
type:      FACT
target:    6809
verify:    from-manual
topic:     dcheck
claim:     DCHECK cannot process disks with more than 39 levels of nested directories.
context:   None
source:    OS-9 Level 2 Operating System Manual, DCHECK section, line 6697
--- END ---

--- CARD ---
id:        deiniz-detaches-device
type:      FACT
target:    6809
verify:    from-manual
topic:     deiniz
claim:     DEINIZ deinitializes and detaches one or more devices from the system.
context:   None
source:    OS-9 Level 2 Operating System Manual, DEINIZ section, line 6821
--- END ---

--- CARD ---
id:        deiniz-performs-detach-call
type:      FACT
target:    6809
verify:    from-manual
topic:     deiniz
claim:     DEINIZ performs an OS-9 I$Detach system call for each specified device.
context:   None
source:    OS-9 Level 2 Operating System Manual, DEINIZ section, line 6841
--- END ---

--- CARD ---
id:        deiniz-pairs-with-iniz
type:      FACT
target:    6809
verify:    from-manual
topic:     deiniz
claim:     DEINIZ is used in conjunction with INIZ; after creating a window or device with INIZ, use DEINIZ to remove it and return its memory.
context:   None
source:    OS-9 Level 2 Operating System Manual, DEINIZ section, line 6834
--- END ---

--- CARD ---
id:        del-deletes-files
type:      FACT
target:    6809
verify:    from-manual
topic:     del
claim:     DEL deletes one or more specified files.
context:   None
source:    OS-9 Level 2 Operating System Manual, DEL section, line 6868
--- END ---

--- CARD ---
id:        del-write-permission-required
type:      FACT
target:    6809
verify:    from-manual
topic:     del
claim:     Write permission on a file is required before DEL can delete it.
context:   None
source:    OS-9 Level 2 Operating System Manual, DEL section, line 6888
--- END ---

--- CARD ---
id:        del-execution-directory-option
type:      FACT
target:    6809
verify:    from-manual
topic:     del
claim:     Option -x causes DEL to assume the file is in the current execution directory instead of the data directory.
context:   None
source:    OS-9 Level 2 Operating System Manual, DEL section, line 6880
--- END ---

--- CARD ---
id:        del-directory-deletion-methods
type:      FACT
target:    6809
verify:    from-manual
topic:     del
claim:     To delete a directory with DEL: delete all files inside, convert the directory to a non-directory with ATTR, then delete it; or use DELDIR instead.
context:   None
source:    OS-9 Level 2 Operating System Manual, DEL section, line 6891
--- END ---

--- CARD ---
id:        deldir-recursive-deletion
type:      FACT
target:    6809
verify:    from-manual
topic:     deldir
claim:     DELDIR recursively deletes all subdirectories and files within a directory, then deletes the directory itself.
context:   None
source:    OS-9 Level 2 Operating System Manual, DELDIR section, line 6953
--- END ---

--- CARD ---
id:        deldir-convenient-alternative
type:      FACT
target:    6809
verify:    from-manual
topic:     deldir
claim:     DELDIR provides a convenient alternative to manually deleting all files and subdirectories before removing the directory.
context:   None
source:    OS-9 Level 2 Operating System Manual, DELDIR section, line 6968
--- END ---

--- CARD ---
id:        deldir-confirmation-prompt
type:      FACT
target:    6809
verify:    from-manual
topic:     deldir
claim:     DELDIR displays a prompt with three options: 'l' (list directory contents), 'd' (start deletion), 'q' (cancel).
context:   None
source:    OS-9 Level 2 Operating System Manual, DELDIR section, line 6972
--- END ---

--- CARD ---
id:        deldir-list-option-shows-files
type:      FACT
target:    6809
verify:    from-manual
topic:     deldir
claim:     Pressing 'l' at the DELDIR prompt runs DIR to display all files before they are deleted.
context:   None
source:    OS-9 Level 2 Operating System Manual, DELDIR section, line 6981
--- END ---

--- CARD ---
id:        deldir-delete-from-bottom-up
type:      FACT
target:    6809
verify:    from-manual
topic:     deldir
claim:     DELDIR processes nested directories bottom-up, starting with the deepest subdirectories and working toward the root.
context:   None
source:    OS-9 Level 2 Operating System Manual, DELDIR section, line 6990
--- END ---

--- CARD ---
id:        deldir-write-permission-required
type:      FACT
target:    6809
verify:    from-manual
topic:     deldir
claim:     Write permission is required for all files and directories in the deletion hierarchy; DELDIR terminates if permission is missing.
context:   None
source:    OS-9 Level 2 Operating System Manual, DELDIR section, line 6993
--- END ---

--- CARD ---
id:        deldir-requires-dir-and-attr
type:      FACT
target:    6809
verify:    from-manual
topic:     deldir
claim:     DELDIR automatically invokes DIR and ATTR; both commands must reside in the current execution directory.
context:   None
source:    OS-9 Level 2 Operating System Manual, DELDIR section, line 7005
--- END ---

--- CARD ---
id:        dir-displays-formatted-filenames
type:      FACT
target:    6809
verify:    from-manual
topic:     dir
claim:     DIR displays a formatted list of filenames in a directory.
context:   None
source:    OS-9 Level 2 Operating System Manual, DIR section, line 7022
--- END ---

--- CARD ---
id:        dir-adjusts-for-column-width
type:      FACT
target:    6809
verify:    from-manual
topic:     dir
claim:     DIR's output format automatically adjusts to the terminal's display width (32-column or 80-column).
context:   None
source:    OS-9 Level 2 Operating System Manual, DIR section, line 7023
--- END ---

--- CARD ---
id:        dir-default-current-data-directory
type:      FACT
target:    6809
verify:    from-manual
topic:     dir
claim:     If no parameters are given, DIR displays the current data directory.
context:   None
source:    OS-9 Level 2 Operating System Manual, DIR section, line 7038
--- END ---

--- CARD ---
id:        dir-execution-directory-option
type:      FACT
target:    6809
verify:    from-manual
topic:     dir
claim:     Option 'x' displays the current execution directory instead of the data directory.
context:   None
source:    OS-9 Level 2 Operating System Manual, DIR section, line 7041
--- END ---

--- CARD ---
id:        dir-extended-details-option
type:      FACT
target:    6809
verify:    from-manual
topic:     dir
claim:     Option 'e' displays full details for each file: size, sector address, owner, permissions, and date/time of last modification.
context:   None
source:    OS-9 Level 2 Operating System Manual, DIR section, line 7043
--- END ---

--- CARD ---
id:        display-hex-to-ascii-conversion
type:      FACT
target:    6809
verify:    from-manual
topic:     display
claim:     DISPLAY reads one or more hexadecimal numbers provided as parameters and outputs the corresponding ASCII characters.
context:   None
source:    OS-9 Level 2 Operating System Manual, DISPLAY section, line 6173
--- END ---

--- CARD ---
id:        display-special-characters
type:      FACT
target:    6809
verify:    from-manual
topic:     display
claim:     DISPLAY is used to send special characters such as cursor control codes and screen control codes to terminals and other I/O devices.
context:   None
source:    OS-9 Level 2 Operating System Manual, DISPLAY section, line 6187
--- END ---

--- CARD ---
id:        dsave-generates-procedure-file
type:      FACT
target:    6809
verify:    from-manual
topic:     dsave
claim:     DSAVE does not directly copy files; instead, it generates a procedure file (shell script) that must be executed separately.
context:   None
source:    OS-9 Level 2 Operating System Manual, DSAVE section, line 7273
--- END ---

--- CARD ---
id:        dsave-backs-up-directory
type:      FACT
target:    6809
verify:    from-manual
topic:     dsave
claim:     DSAVE copies all files from one or more directories, recursively including subdirectories unless -l is specified.
context:   None
source:    OS-9 Level 2 Operating System Manual, DSAVE section, line 7220
--- END ---

--- CARD ---
id:        dsave-default-device
type:      FACT
target:    6809
verify:    from-manual
topic:     dsave
claim:     If devname is not specified, DSAVE defaults to /D0 as the source drive.
context:   None
source:    OS-9 Level 2 Operating System Manual, DSAVE section, line 7228
--- END ---

--- CARD ---
id:        dsave-output-redirection
type:      FACT
target:    6809
verify:    from-manual
topic:     dsave
claim:     DSAVE writes the procedure file by output redirection (> pathlist); the pathlist must not already exist.
context:   None
source:    OS-9 Level 2 Operating System Manual, DSAVE section, line 7217
--- END ---

--- CARD ---
id:        dsave-requires-chd-first
type:      FACT
target:    6809
verify:    from-manual
topic:     dsave
claim:     Before running DSAVE, use CHD to change to the source directory to be copied.
context:   None
source:    OS-9 Level 2 Operating System Manual, DSAVE section, line 7285
--- END ---

--- CARD ---
id:        dsave-chd-to-destination
type:      FACT
target:    6809
verify:    from-manual
topic:     dsave
claim:     After generating the procedure file with DSAVE, use CHD to change to the destination directory, then execute the procedure file.
context:   None
source:    OS-9 Level 2 Operating System Manual, DSAVE section, line 7291
--- END ---

--- CARD ---
id:        dsave-auto-makdir-commands
type:      FACT
target:    6809
verify:    from-manual
topic:     dsave
claim:     DSAVE automatically includes MAKDIR and CHD commands in the procedure file when it encounters subdirectories.
context:   None
source:    OS-9 Level 2 Operating System Manual, DSAVE section, line 7295
--- END ---

--- CARD ---
id:        dsave-full-disk-backup
type:      FACT
target:    6809
verify:    from-manual
topic:     dsave
claim:     If the current data directory is the ROOT directory, DSAVE creates a procedure file that backs up the entire disk file by file.
context:   None
source:    OS-9 Level 2 Operating System Manual, DSAVE section, line 7301
--- END ---

--- CARD ---
id:        dsave-boot-copy-option
type:      FACT
target:    6809
verify:    from-manual
topic:     dsave
claim:     Option -b copies the OS9Boot file from the source disk to make the destination disk bootable.
context:   None
source:    OS-9 Level 2 Operating System Manual, DSAVE section, line 7244
--- END ---

--- CARD ---
id:        dsave-indent-option
type:      FACT
target:    6809
verify:    from-manual
topic:     dsave
claim:     Option -i causes DSAVE to indent the procedure file commands to reflect directory nesting levels.
context:   None
source:    OS-9 Level 2 Operating System Manual, DSAVE section, line 7248
--- END ---

--- CARD ---
id:        dsave-no-subdirectories-option
type:      FACT
target:    6809
verify:    from-manual
topic:     dsave
claim:     Option -l tells DSAVE not to process directories below the current level.
context:   None
source:    OS-9 Level 2 Operating System Manual, DSAVE section, line 7250
--- END ---

--- CARD ---
id:        dsave-no-makdir-option
type:      FACT
target:    6809
verify:    from-manual
topic:     dsave
claim:     Option -m tells DSAVE not to include MAKDIR commands in the procedure file it creates.
context:   None
source:    OS-9 Level 2 Operating System Manual, DSAVE section, line 7253
--- END ---

--- CARD ---
id:        dsave-memory-allocation-option
type:      FACT
target:    6809
verify:    from-manual
topic:     dsave
claim:     Option -s<integer> sets memory allocation for the copy operation to the specified number of kilobytes.
context:   None
source:    OS-9 Level 2 Operating System Manual, DSAVE section, line 7256
--- END ---

--- CARD ---
id:        dsave-verify-option
type:      FACT
target:    6809
verify:    from-manual
topic:     dsave
claim:     Option -v enables verification by forking to CMP after copying each file to ensure data integrity.
context:   None
source:    OS-9 Level 2 Operating System Manual, DSAVE section, line 7259
--- END ---

--- CARD ---
id:        dsave-pipe-to-shell
type:      FACT
target:    6809
verify:    from-manual
topic:     dsave
claim:     DSAVE output can be piped directly into a shell for immediate execution instead of saving to a file.
context:   None
source:    OS-9 Level 2 Operating System Manual, DSAVE section, line 7328
--- END ---
