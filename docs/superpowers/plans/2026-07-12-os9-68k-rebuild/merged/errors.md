# Error codes and error handling

118 cards in, 109 cards out. The error-code-table portion (103 cards) carries
forward already-cross-checked work from the `error-codes-pilot.md` pass — those
cards were previously cross-checked between the OS-9 v2.4 Technical Reference
Manual and an independent 1985-era OS-9/68000 technical manual, with genuine
cross-manual discrepancies already identified and preserved (e.g. the TRAP-range
numbering difference between editions, and error 000:225's description
specificity). That work is carried forward here, lightly reordered into strict
code-number order, not re-merged or re-litigated. The remaining ~15 cards came
from other source documents and received normal sift/merge/prune treatment; 6
survived as general error-handling concepts/gotchas placed after the table, 9
were pruned as duplicates or as mis-bucketed non-error-handling facts.

## Error code table (68k), in code-number order

--- CARD ---
id:        keyboard-quit-error
type:      FACT
target:    68k
verify:    from-manual
topic:     terminal-io
claim:     Error code 000:002 indicates keyboard quit — returned when the keyboard abort function (Control-E) is sent.
context:   -
source:    OS-9/68000 Operating System Technical Manual (July 1985, Revision S), Appendix C, Error Codes, "OTHER ERRORS"
--- END ---

--- CARD ---
id:        keyboard-interrupt-error
type:      FACT
target:    68k
verify:    from-manual
topic:     terminal-io
claim:     Error code 000:003 indicates keyboard interrupt — returned when the keyboard interrupt function (Control-C) is sent.
context:   -
source:    OS-9/68000 Operating System Technical Manual (July 1985, Revision S), Appendix C, Error Codes, "OTHER ERRORS"
--- END ---

--- CARD ---
id:        illegal-function-code-math
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:064 (E$IllFno) indicates illegal function code in a math trap handler error.
context:   -
source:    OS-9/68000 Operating System Technical Manual (July 1985, Revision S), Appendix C, Error Codes, "OTHER ERRORS"
--- END ---

--- CARD ---
id:        format-error-math
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:065 (E$FmtErr) indicates format error in a math trap handler.
context:   -
source:    OS-9/68000 Operating System Technical Manual (July 1985, Revision S), Appendix C, Error Codes, "OTHER ERRORS"
--- END ---

--- CARD ---
id:        number-not-found-math
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:066 (E$NotNum) indicates number not found in a math trap handler error.
context:   -
source:    OS-9/68000 Operating System Technical Manual (July 1985, Revision S), Appendix C, Error Codes, "OTHER ERRORS"
--- END ---

--- CARD ---
id:        illegal-argument-math
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:067 (E$IllArg) indicates illegal argument in a math trap handler error.
context:   -
source:    OS-9/68000 Operating System Technical Manual (July 1985, Revision S), Appendix C, Error Codes, "OTHER ERRORS"
--- END ---

--- CARD ---
id:        bus-error-exception
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:102 (E$BusErr) indicates a bus error exception occurred at the CPU level.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:102
--- END ---

--- CARD ---
id:        address-error-exception
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:103 (E$AdrErr) indicates an address error exception occurred.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:103
--- END ---

--- CARD ---
id:        illegal-instruction-exception
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:104 (E$IllIns) indicates an illegal instruction exception occurred.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:104
--- END ---

--- CARD ---
id:        zero-divide-exception
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:105 (E$ZerDiv) indicates an integer zero divide exception occurred.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:105
--- END ---

--- CARD ---
id:        chk-instruction-exception
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:106 (E$Chk) indicates a CHK or CHK2 instruction exception occurred.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:106
--- END ---

--- CARD ---
id:        trap-instruction-exception
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:107 (E$TrapV) indicates a TRAPV, TRAPcc, or FTRAPcc instruction exception occurred.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:107
--- END ---

--- CARD ---
id:        privilege-violation-exception
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:108 (E$Violat) indicates a privilege violation exception occurred.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:108
--- END ---

--- CARD ---
id:        uninitialized-trace-exception
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:109 (E$Trace) indicates an uninitialized trace exception occurred.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:109
--- END ---

--- CARD ---
id:        a-line-emulator-exception
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:110 (E$1010) indicates a 1010 line emulator exception occurred (A Line).
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:110
--- END ---

--- CARD ---
id:        f-line-emulator-exception
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:111 (E$1111) indicates a 1111 line emulator exception occurred (F Line).
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:111
--- END ---

--- CARD ---
id:        coprocessor-protocol-violation
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:113 indicates a coprocessor protocol violation.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:113
--- END ---

--- CARD ---
id:        format-error-exception
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:114 indicates a format error exception.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:114
--- END ---

--- CARD ---
id:        uninitialized-interrupt-occurred
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:115 indicates an uninitialized interrupt occurred.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:115
--- END ---

--- CARD ---
id:        spurious-interrupt-occurred
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:124 indicates a spurious interrupt occurred.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:124
--- END ---

--- CARD ---
id:        uninitialized-user-trap
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error code 000:133–000:147 (E$Trap) indicates an uninitialized user TRAP 1–15 was executed.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:133-000:147
--- END ---

--- CARD ---
id:        user-trap-range-discrepancy-1985
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-cpu
claim:     Error codes 000:124–000:138 (E$Trap) indicate uninitialized user TRAP 0–14 was executed.
context:   DISCREPANCY: OS-9 v2.4 manual states range 000:133–000:147 for TRAP 1–15; this 1985 independent manual documents 000:124–000:138 for TRAP 0–14. Exact trap numbering and range differ between editions.
source:    OS-9/68000 Operating System Technical Manual (July 1985, Revision S), Appendix C, Error Codes, "UNINITIALIZED TRAP ERRORS"
--- END ---

--- CARD ---
id:        fpcp-unordered-condition-error
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-fpu
claim:     Error code 000:148 (E$FPUnordC) indicates an FPCP branch or set on unordered condition error.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:148
--- END ---

--- CARD ---
id:        fpcp-inexact-result
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-fpu
claim:     Error code 000:149 (E$FPInxact) indicates an FPCP inexact result error.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:149
--- END ---

--- CARD ---
id:        fpcp-divide-by-zero
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-fpu
claim:     Error code 000:150 (E$FPDivZer) indicates an FPCP divide by zero error.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:150
--- END ---

--- CARD ---
id:        fpcp-underflow-error
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-fpu
claim:     Error code 000:151 (E$FPUndrFl) indicates an FPCP underflow error.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:151
--- END ---

--- CARD ---
id:        fpcp-operand-error
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-fpu
claim:     Error code 000:152 (E$FPOprErr) indicates an FPCP operand error.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:152
--- END ---

--- CARD ---
id:        fpcp-overflow-error
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-fpu
claim:     Error code 000:153 (E$FPOverFl) indicates an FPCP overflow error.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:153
--- END ---

--- CARD ---
id:        fpcp-nan-signaled
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-fpu
claim:     Error code 000:154 (E$FPNotNum) indicates an FPCP NAN signaled error.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:154
--- END ---

--- CARD ---
id:        fpcp-unimplemented-data-type
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-fpu
claim:     Error code 000:155 indicates an FPCP unimplemented data type error.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:155
--- END ---

--- CARD ---
id:        pmmu-configuration-error
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-mmu
claim:     Error code 000:156 indicates a PMMU configuration error.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:156
--- END ---

--- CARD ---
id:        pmmu-illegal-operation
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-mmu
claim:     Error code 000:157 indicates a PMMU illegal operation error.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:157
--- END ---

--- CARD ---
id:        pmmu-access-level-violation
type:      FACT
target:    68k
verify:    from-manual
topic:     exceptions-mmu
claim:     Error code 000:158 indicates a PMMU access level violation error.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:158
--- END ---

--- CARD ---
id:        no-permission-error
type:      FACT
target:    68k
verify:    from-manual
topic:     access-control
claim:     Error code 000:164 (E$Permit) indicates no permission — the process or module must be owned by the superuser to perform the requested function.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:164
--- END ---

--- CARD ---
id:        different-arguments-error
type:      FACT
target:    68k
verify:    from-manual
topic:     validation
claim:     Error code 000:165 (E$Differ) indicates that F$ChkNam arguments do not match.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:165
--- END ---

--- CARD ---
id:        stack-overflow-error
type:      FACT
target:    68k
verify:    from-manual
topic:     validation
claim:     Error code 000:166 (E$StkOvf) indicates stack overflow; F$ChkNam can cause this if the pattern string is too complex.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:166
--- END ---

--- CARD ---
id:        illegal-event-id
type:      FACT
target:    68k
verify:    from-manual
topic:     event-mgmt
claim:     Error code 000:167 (E$EvntID) indicates an invalid or illegal event ID number is specified.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:167
--- END ---

--- CARD ---
id:        event-name-not-found
type:      FACT
target:    68k
verify:    from-manual
topic:     event-mgmt
claim:     Error code 000:168 (E$EvNF) indicates an attempt to link to or delete an event failed because the name is not found in the event table.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:168
--- END ---

--- CARD ---
id:        event-busy-error
type:      FACT
target:    68k
verify:    from-manual
topic:     event-mgmt
claim:     Error code 000:169 (E$EvBusy) indicates an attempt to delete an event with nonzero link count, or an attempt to create an already-existent named event.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:169
--- END ---

--- CARD ---
id:        impossible-event-parameter
type:      FACT
target:    68k
verify:    from-manual
topic:     event-mgmt
claim:     Error code 000:170 (E$EvParm) indicates impossible parameters were passed to F$Event.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:170
--- END ---

--- CARD ---
id:        system-damage-error
type:      FACT
target:    68k
verify:    from-manual
topic:     system-integrity
claim:     Error code 000:171 (E$Damage) indicates a system data structure has been corrupted.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:171
--- END ---

--- CARD ---
id:        incompatible-revision-error
type:      FACT
target:    68k
verify:    from-manual
topic:     system-integrity
claim:     Error code 000:172 (E$BadRev) indicates the software revision is incompatible with the operating system revision.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:172
--- END ---

--- CARD ---
id:        path-lost-error
type:      FACT
target:    68k
verify:    from-manual
topic:     io-paths
claim:     Error code 000:173 (E$PthLost) indicates the path became lost, typically when a network node goes down, a serial connection loses data carrier, or a pipe is broken via SS_Break SetStat.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:173
--- END ---

--- CARD ---
id:        bad-partition-error
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:174 (E$BadPart) indicates bad partition data or no active partition.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:174
--- END ---

--- CARD ---
id:        hardware-damage-detected
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:175 (E$Hardware) indicates hardware damage has been detected, usually when the driver fails to detect correct responses from the hardware.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:175
--- END ---

--- CARD ---
id:        invalid-sector-size
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:176 (E$SectSize) indicates an invalid sector size. RBF devices require binary multiples of 256 (256, 512, 1024, etc.) up to a maximum of 32768 bytes.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:176
--- END ---

--- CARD ---
id:        path-table-full-error
type:      FACT
target:    68k
verify:    from-manual
topic:     io-paths
claim:     Error code 000:200 (E$BPNum) indicates the path table is full — a process tried to open more than 32 I/O paths simultaneously, or there is insufficient contiguous memory to expand the table.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:200
--- END ---

--- CARD ---
id:        illegal-path-number
type:      FACT
target:    68k
verify:    from-manual
topic:     io-paths
claim:     Error code 000:201 (E$BPNum) indicates an illegal path number — the path number was too large or for a non-existent path.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:201
--- END ---

--- CARD ---
id:        interrupt-polling-table-full
type:      FACT
target:    68k
verify:    from-manual
topic:     irq-mgmt
claim:     Error code 000:202 (E$Poll) indicates the interrupt polling table is full; an attempt to install an IRQ service routine failed.
context:   The system INIT module specifies the maximum number of IRQ devices.
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:202
--- END ---

--- CARD ---
id:        illegal-mode-error
type:      FACT
target:    68k
verify:    from-manual
topic:     io-paths
claim:     Error code 000:203 (E$BMode) indicates an illegal mode — an I/O function was attempted on a device or file incapable of it (e.g., reading from an output file).
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:203
--- END ---

--- CARD ---
id:        device-table-full
type:      FACT
target:    68k
verify:    from-manual
topic:     device-mgmt
claim:     Error code 000:204 (E$DevOvf) indicates the device table is full; the specified device cannot be added to the system.
context:   The INIT module specifies the maximum number of devices that may be supported.
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:204
--- END ---

--- CARD ---
id:        illegal-module-header
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Error code 000:205 (E$BMID) indicates an illegal module header — the specified module cannot be loaded because its module sync code is incorrect.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:205
--- END ---

--- CARD ---
id:        module-directory-full
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Error code 000:206 (E$DirFul) indicates the module directory is full; the specified module cannot be added because there is not enough memory or the memory is too fragmented.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:206
--- END ---

--- CARD ---
id:        memory-full-error
type:      FACT
target:    68k
verify:    from-manual
topic:     memory-mgmt
claim:     Error code 000:207 (E$MemFul) indicates memory is full; there is not enough contiguous RAM free, or the process already has the maximum number of blocks allocated.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:207
--- END ---

--- CARD ---
id:        illegal-service-request
type:      FACT
target:    68k
verify:    from-manual
topic:     system-calls
claim:     Error code 000:208 (E$UnkSvc) indicates an illegal service request — the service code number is unknown or invalid, or a Getstat/Setstat call used an unknown status code.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:208
--- END ---

--- CARD ---
id:        module-busy-error
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Error code 000:209 (E$ModBsy) indicates a module is busy — an attempt was made to access a non-sharable module that is in use by another process.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:209
--- END ---

--- CARD ---
id:        boundary-error
type:      FACT
target:    68k
verify:    from-manual
topic:     memory-mgmt
claim:     Error code 000:210 (E$BPAddr) indicates a boundary error — a memory deallocation request used an invalid block address or attempted to deallocate memory not previously assigned.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:210
--- END ---

--- CARD ---
id:        end-of-file-error
type:      FACT
target:    68k
verify:    from-manual
topic:     io-paths
claim:     Error code 000:211 (E$EOF) indicates end of file — an EOF condition was encountered on a read operation.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:211
--- END ---

--- CARD ---
id:        vector-busy-error
type:      FACT
target:    68k
verify:    from-manual
topic:     irq-mgmt
claim:     Error code 000:212 (E$VctBsy) indicates a vector is busy — a device is trying to use an IRQ vector that is currently in use by another device.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:212
--- END ---

--- CARD ---
id:        non-existing-segment
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:213 (E$NES) indicates a non-existing segment — a disk file segment cannot be found, possibly due to damaged file structure.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:213
--- END ---

--- CARD ---
id:        file-not-accessible
type:      FACT
target:    68k
verify:    from-manual
topic:     file-access
claim:     Error code 000:214 (E$FNA) indicates file not accessible — an attempt was made to open a file or device without correct access permissions.
context:   Check the file's attributes and owner ID.
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:214
--- END ---

--- CARD ---
id:        bad-path-name
type:      FACT
target:    68k
verify:    from-manual
topic:     io-paths
claim:     Error code 000:215 (E$BPNam) indicates a bad path name — there is a syntax error in the specified pathlist (illegal character, etc.).
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:215
--- END ---

--- CARD ---
id:        path-name-not-found
type:      FACT
target:    68k
verify:    from-manual
topic:     io-paths
claim:     Error code 000:216 (E$PNNF) indicates path name not found — the specified pathlist cannot be found, possibly due to misspellings or incorrect directories.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:216
--- END ---

--- CARD ---
id:        segment-list-full
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:217 (E$SLF) indicates segment list is full — a file is too fragmented to be expanded further, usually due to expanding a file many times or insufficient free disk space.
context:   Copying the file or disk may solve the problem by moving it into contiguous areas.
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:217
--- END ---

--- CARD ---
id:        file-already-exists
type:      FACT
target:    68k
verify:    from-manual
topic:     file-access
claim:     Error code 000:218 (E$CEF) indicates file already exists — an attempt was made to create a file using a name that already appears in the current directory.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:218
--- END ---

--- CARD ---
id:        illegal-block-address
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:219 (E$IBA) indicates an illegal block address — an invalid pointer or block size was passed, or the device's file structure is damaged.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:219
--- END ---

--- CARD ---
id:        hangup-modem-carrier-lost
type:      FACT
target:    68k
verify:    from-manual
topic:     serial-io
claim:     Error code 000:220 (E$HangUp) indicates telephone (modem) data carrier was lost.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:220
--- END ---

--- CARD ---
id:        module-not-found
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Error code 000:221 (E$MNF) indicates module not found — a request to link to a module that is not found in the module directory, or whose header has been modified/corrupted.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:221
--- END ---

--- CARD ---
id:        no-clock-error
type:      FACT
target:    68k
verify:    from-manual
topic:     time-mgmt
claim:     Error code 000:222 (E$NoClk) indicates no clock — a request uses the system clock, but the system has no clock running (e.g., timed SLEEP request).
context:   Use SETIME to start the system clock.
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:222
--- END ---

--- CARD ---
id:        suicide-attempt-error
type:      FACT
target:    68k
verify:    from-manual
topic:     process-mgmt
claim:     Error code 000:223 (E$DelSP) indicates a suicide attempt — a user requested deallocation of memory where the user's own stack is located.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:223
--- END ---

--- CARD ---
id:        illegal-process-number
type:      FACT
target:    68k
verify:    from-manual
topic:     process-mgmt
claim:     Error code 000:224 (E$IPrcID) indicates an illegal process number — a system call was passed a process ID to a non-existent process or one the user may not access.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:224
--- END ---

--- CARD ---
id:        bad-parameter-error
type:      FACT
target:    68k
verify:    from-manual
topic:     validation
claim:     Error code 000:225 (E$Param) indicates a bad parameter — a service request was passed an illegal or impossible parameter.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:225
--- END ---

--- CARD ---
id:        bad-polling-parameter-1985
type:      FACT
target:    68k
verify:    from-manual
topic:     validation
claim:     Error code 000:225 (E$Param) is returned when an impossible vector number is passed to the IRQ polling system.
context:   DISCREPANCY/CLARIFICATION: OS-9 v2.4 manual describes this as generic "bad parameter"; this source clarifies it is specific to IRQ polling vector validation.
source:    OS-9/68000 Operating System Technical Manual (July 1985, Revision S), Appendix C, Error Codes, page C-5
--- END ---

--- CARD ---
id:        no-children-error
type:      FACT
target:    68k
verify:    from-manual
topic:     process-mgmt
claim:     Error code 000:226 (E$NoChld) indicates no children — an F$Wait request was made but the process has no child process to wait for.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:226
--- END ---

--- CARD ---
id:        illegal-trap-code
type:      FACT
target:    68k
verify:    from-manual
topic:     system-calls
claim:     Error code 000:227 (E$ITrap) indicates an illegal trap code — an unavailable (already in use) or invalid trap code was used in a TLINK call.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:227
--- END ---

--- CARD ---
id:        process-aborted
type:      FACT
target:    68k
verify:    from-manual
topic:     process-mgmt
claim:     Error code 000:228 (E$PrcAbt) indicates process aborted — a process is aborted by the kill signal code.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:228
--- END ---

--- CARD ---
id:        process-table-full
type:      FACT
target:    68k
verify:    from-manual
topic:     process-mgmt
claim:     Error code 000:229 (E$PrcFul) indicates the process table is full — too many processes are currently running, or there is insufficient contiguous memory to expand it.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:229
--- END ---

--- CARD ---
id:        illegal-fork-parameter-area
type:      FACT
target:    68k
verify:    from-manual
topic:     process-mgmt
claim:     Error code 000:230 (E$IForkP) indicates an illegal parameter area — ridiculous parameters were passed to a fork call.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:230
--- END ---

--- CARD ---
id:        known-module-error
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Error code 000:231 (E$KwnMod) indicates a known module — a call was made to install a module that is already in memory.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:231
--- END ---

--- CARD ---
id:        incorrect-module-crc
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Error code 000:232 (E$BMCRC) indicates an incorrect module CRC — the specified module has a bad CRC value.
context:   Use the FIXMOD utility to generate a valid CRC.
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:232
--- END ---

--- CARD ---
id:        unprocessed-signal-pending
type:      FACT
target:    68k
verify:    from-manual
topic:     signal-mgmt
claim:     Error code 000:233 (E$USigP) indicates an unprocessed signal is pending.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:233
--- END ---

--- CARD ---
id:        non-executable-module
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Error code 000:234 (E$NEMod) indicates a non-executable module — a process tries to execute a module with a type other than program/object.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:234
--- END ---

--- CARD ---
id:        bad-name-error
type:      FACT
target:    68k
verify:    from-manual
topic:     validation
claim:     Error code 000:235 (E$BNam) indicates a bad name — there is a syntax error in the specified name.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:235
--- END ---

--- CARD ---
id:        bad-module-header-parity
type:      FACT
target:    68k
verify:    from-manual
topic:     modules
claim:     Error code 000:236 (E$BMHP) indicates bad parity — the specified module has bad module header parity.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:236
--- END ---

--- CARD ---
id:        ram-full-error
type:      FACT
target:    68k
verify:    from-manual
topic:     memory-mgmt
claim:     Error code 000:237 (E$NoRAM) indicates RAM is full — there is no free system RAM available, or insufficient contiguous memory to process a fork request.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:237
--- END ---

--- CARD ---
id:        directory-not-empty
type:      FACT
target:    68k
verify:    from-manual
topic:     file-access
claim:     Error code 000:238 (E$DNE) indicates directory not empty — an attempt was made to remove the directory attribute from a non-empty directory.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:238
--- END ---

--- CARD ---
id:        no-task-number-available
type:      FACT
target:    68k
verify:    from-manual
topic:     process-mgmt
claim:     Error code 000:239 (E$NoTask) indicates no task number available — all task numbers are currently in use and a new task execution/creation was requested.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:239
--- END ---

--- CARD ---
id:        illegal-drive-number
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:240 (E$Unit) indicates an illegal drive number.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:240
--- END ---

--- CARD ---
id:        bad-sector-error
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:241 (E$Sect) indicates a bad sector — bad disk sector number.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:241
--- END ---

--- CARD ---
id:        write-protect-error
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:242 (E$WP) indicates device is write protected.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:242
--- END ---

--- CARD ---
id:        crc-error
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:243 (E$CRC) indicates a CRC error on read or write verify.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:243
--- END ---

--- CARD ---
id:        read-error
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:244 (E$Read) indicates a read error — data transfer error during disk read operation, or SCF (terminal) input buffer overrun.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:244
--- END ---

--- CARD ---
id:        write-error
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:245 (E$Write) indicates a write error — hardware error during disk write operation.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:245
--- END ---

--- CARD ---
id:        not-ready-error
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:246 (E$NotRdy) indicates device is not ready.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:246
--- END ---

--- CARD ---
id:        seek-error
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:247 (E$Seek) indicates a seek error — physical seek to non-existent sector.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:247
--- END ---

--- CARD ---
id:        media-full-error
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:248 (E$Full) indicates media is full — insufficient free space on media.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:248
--- END ---

--- CARD ---
id:        wrong-type-error
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:249 (E$BTyp) indicates wrong type — attempt to read incompatible media (e.g., double-side disk on single-side drive).
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:249
--- END ---

--- CARD ---
id:        device-busy-error
type:      FACT
target:    68k
verify:    from-manual
topic:     device-mgmt
claim:     Error code 000:250 (E$DevBsy) indicates device is busy — non-sharable device is in use.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:250
--- END ---

--- CARD ---
id:        disk-id-change-error
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:251 (E$DIDC) indicates disk ID change — the disk media was changed with open files. RBF detects this mismatch between the disk ID number in the path descriptor and the driver's current disk ID.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:251
--- END ---

--- CARD ---
id:        record-locked-out
type:      FACT
target:    68k
verify:    from-manual
topic:     file-access
claim:     Error code 000:252 (E$Lock) indicates record is locked out — another process is accessing the requested record, or a timed lock wait expired.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:252
--- END ---

--- CARD ---
id:        non-sharable-file-busy
type:      FACT
target:    68k
verify:    from-manual
topic:     file-access
claim:     Error code 000:253 (E$Share) indicates non-sharable file is busy — the requested file or device has the single-user bit set, or was opened in single-user mode and another process is accessing it.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:253
--- END ---

--- CARD ---
id:        io-deadlock-error
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:254 (E$DeadLk) indicates I/O deadlock — two processes are attempting to use the same two disk areas simultaneously, each locking out the other.
context:   -
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:254
--- END ---

--- CARD ---
id:        format-protected-device
type:      FACT
target:    68k
verify:    from-manual
topic:     disk-io
claim:     Error code 000:255 (E$Format) indicates device is format protected — an attempt was made to format a disk that is format protected.
context:   A bit in the device descriptor may be changed to allow formatting; formatting is usually inhibited on hard disks to prevent erasure.
source:    OS-9 v2.4 Technical Reference Manual, Error Codes section, 000:255
--- END ---

## General error-handling concepts and gotchas

--- CARD ---
id:        errno-system-call-convention
type:      FACT
target:    all
verify:    from-manual
topic:     error-handling
claim:     OS-9 C system calls signal failure by returning -1; the specific error code is left in the predefined variable `errno`, with definitions supplied by `<errno.h>`. `errno` retains the value from the most recently *failed* call — it is not reset on success.
context:   Because errno isn't cleared on success, code must check the call's own return value first, then consult errno only when an error was indicated.
source:    OS-9 C Compiler manual, "Introduction to C System Calls", p. 3-1
--- END ---

--- CARD ---
id:        arithmetic-error-signal-codes-6809
type:      FACT
target:    6809
verify:    from-manual
topic:     error-handling
claim:     This compiler's runtime adds three arithmetic error codes to <errno.h>: `#define EFPOVR 40` (floating point overflow or underflow), `#define EDIVERR 41` (division by zero), `#define EINTERR 42` (overflow converting a floating point value to a long integer). When one of these conditions occurs, the running program sends *itself* a signal carrying that error number as its value; if uncaught, the program terminates with an error return to its parent.
context:   These are runtime-generated conditions caught via signal()/intercept(), distinct from the OS-9 kernel's own error-number space; specific numeric values (40/41/42) and the self-signal mechanism are this compiler's own design and not confirmed identical on a 68k compiler.
source:    OS-9 C Compiler manual, "Run-Time Arithmetic Error Handling", p. 1-8
--- END ---

--- CARD ---
id:        basic09-err-function
type:      FACT
target:    all
verify:    from-manual
topic:     basic09-error-handling
claim:     The ERR function returns the error code of the most recent error and automatically resets to zero when referenced. This allows interrogation of what error occurred.
context:   Error codes above 80 are OS-9 or external errors; codes 0-80 are BASIC09-specific.
source:    BASIC09 Reference Manual (Rev H), Functions section, p. 8-5
--- END ---

--- CARD ---
id:        driver-subroutine-error-return
type:      FACT
target:    68k
verify:    from-manual
topic:     device-drivers
claim:     All driver subroutines end with RTS. Error status returned via CCR carry bit; error code in (d1.w). For IRQ routine, only CCR carry is meaningful.
context:   -
source:    Technical I/O Manual v2.4, "Driver Module Format", p. 1-23
--- END ---

--- CARD ---
id:        f-exit-process-termination
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     process-management
claim:     F$Exit terminates the calling process by releasing all resources: open paths are closed, the data memory is deallocated, and both the primary module and any installed user trap handlers are unlinked. A parent process waiting on the terminating child via F$Wait receives the status code passed in d1.w. By convention, this code should be an OS-9 error code or zero to indicate success.
context:   The parent must perform an F$Wait before the process descriptor is returned to the system; if the parent is dead, the process descriptor is freed immediately. Input: d1.w = status code to be returned to parent. Output: process is terminated.
source:    the independent 1985-era OS-9/68000 technical manual, §14-12 "F$Exit"
--- END ---

--- CARD ---
id:        pipe-write-error-unnamed-named
type:      BEHAVIOR
target:    68k
verify:    from-manual
topic:     ipc
claim:     Unnamed pipes: I$Write returns E$Write error when all full-pipe writers attempt write (pipe remains full); each writer gets error. Named pipes: no E$Write error; writer sleeps until reader opens/drains pipe.
context:   Difference allows named pipes to work as self-destructing temporary RAM disk files (non-empty, no open paths).
source:    OS-9 v2.4 Technical Reference Manual, "Write/WritLn", p. 4-13
--- END ---
