/*
 *  errno_from_docs.h  —  OS-9/68000 error codes, full range
 *
 *  ALTERNATIVE to errno_from_book.h, not a companion — the two define the
 *  same macro names at the same numeric values and must not both be
 *  #included in the same translation unit. errno_from_book.h derives ~93
 *  codes from "The OS-9 Guru" alone, using anchor values + a documented
 *  formula, and is kept as the more conservative, narrower-provenance
 *  version. This file is the more complete replacement: it draws on this
 *  project's own cross-referenced skill compilation
 *  (~/.claude/skills/os9-dev/references/common/error-codes.md), which was
 *  built by cross-checking the OS-9 v2.4 Technical Reference Manual against
 *  an independent 1985 OS-9/68000 Technical Manual, the OS-9 C Compiler
 *  manual, the BASIC09 Reference Manual, and the Technical I/O Manual.
 *  Prefer this file for new use; errno_from_book.h is kept for its
 *  independent, more narrowly-sourced derivation history.
 *
 *  Copyright note: only the numeric error codes and their short symbolic
 *  names are reproduced here — these are functional facts/identifiers (like
 *  any ABI constant or POSIX errno value), not the manuals' own descriptive
 *  prose. Every comment below is an independently-worded functional gloss
 *  (a handful of words, in the spirit of glibc's own errno.h comments, e.g.
 *  "No such file or directory" for ENOENT) — none of it is copied or
 *  lightly reworded from any manual's actual sentences. Where a code's
 *  symbolic name isn't confirmed by any source, no name is invented; the
 *  numeric value is presented as a comment only, not a #define, to avoid
 *  fabricating an identifier Microware never assigned.
 *
 *  Where the skill's own cross-check found the two manual editions
 *  disagree (marked with a dagger below), both readings are given rather
 *  than silently picking one — see the skill file's "Known cross-manual
 *  discrepancies" section for the full reasoning.
 *
 *  SECOND-HAND, AND NOT GUARANTEED MICROWARE-COMPATIBLE. Everything here was
 *  reconstructed from published books and documentation, never from Microware
 *  source. We tried to get it right and believe it is close, but if something
 *  built on this header disagrees with real OS-9, suspect the header at least
 *  as readily as the code using it.
 */

#ifndef ERRNO_FROM_DOCS_H
#define ERRNO_FROM_DOCS_H

/* ── Process, terminal, and math-trap errors (1-67) ──────────────────────── */

/* 000:001 — no confirmed symbol — process has aborted */
/* 000:002 — no confirmed symbol — Control-E (abort) sent to the process */
/* 000:003 — no confirmed symbol — Control-C (interrupt) sent to the process */
/* 000:004 — no confirmed symbol — modem/carrier hangup detected by a driver */

#define E_ILLFNC    0x40  /* 64 — invalid function code given to the math trap handler */
#define E_FMTERR    0x41  /* 65 — math trap handler format error */
#define E_NOTNUM    0x42  /* 66 — math trap handler could not find the expected number */
#define E_ILLARG    0x43  /* 67 — math trap handler received an illegal argument */

/* ── Processor exception errors (102-158): code = exception vector + 100 ── */

#define E_BUSERR    102  /* bus error (exception 2) */
#define E_ADRERR    103  /* address error (exception 3) */
#define E_ILLINS    104  /* illegal instruction (exception 4) */
#define E_ZERDIV    105  /* zero divide (exception 5) */
#define E_CHK       106  /* CHK/CHK2 instruction exception (exception 6) */
#define E_TRAPV     107  /* TRAPV/TRAPcc/FTRAPcc instruction exception (exception 7) */
#define E_VIOLAT    108  /* privilege violation (exception 8) */
#define E_TRACE     109  /* uninitialized trace exception (exception 9) */
#define E_1010      110  /* line-1010 emulator trap (exception 10) */
#define E_1111      111  /* line-1111 emulator trap (exception 11) */
#define E_COPROTO   113  /* coprocessor protocol violation (exception 13) */
#define E_FMTEXC    114  /* format error exception (exception 14) */
#define E_UNINTR    115  /* uninitialized interrupt occurred (exception 15) */
/* 000:124 — disputed between manual editions, see note below */
/* TRAP instructions: exceptions 32-47 -> 132-147 */
#define E_TRAP      132  /* uninitialized user TRAP handler; user traps: E_TRAP+n, see dagger note */
#define E_FPUNORDC  148  /* FP branch/set on unordered condition (exception 48) */
#define E_FPINXACT  149  /* FP inexact result (exception 49) */
#define E_FPDIVZER  150  /* FP zero divide (exception 50) */
#define E_FPUNDRFL  151  /* FP underflow (exception 51) */
#define E_FPOPRERR  152  /* FP invalid operand (exception 52) */
#define E_FPOVERFL  153  /* FP overflow (exception 53) */
#define E_FPNOTNUM  154  /* FP signaling NaN (exception 54) */
#define E_FPBADTYP  155  /* FP unimplemented data type */
#define E_PMMUCFG   156  /* PMMU configuration error */
#define E_PMMUOP    157  /* illegal PMMU operation */
#define E_PMMUACC   158  /* PMMU access-level violation */

/*
 * Dagger note (000:124, 000:133-147): the two source manuals disagree here.
 * One edition gives 133-147 for uninitialized user TRAP #1-15 and treats 124
 * as a separate "spurious interrupt" code. A second, independent edition
 * instead gives 124-138 for uninitialized user TRAP #0-14 (a different base
 * offset AND a different trap-numbering convention). The two ranges are not
 * reconcilable by a simple off-by-one — confirm against the actual
 * kernel/INIT build you're targeting rather than assuming either is
 * authoritative. E_TRAP=132 above follows the first edition; do not treat
 * 124 as available for reuse without checking which convention applies.
 */

/* ── Miscellaneous errors (164-176) ──────────────────────────────────────── */

#define E_PERMIT    164  /* operation requires superuser ownership */
#define E_DIFFER    165  /* name-check arguments do not match */
#define E_STKOVF    166  /* pattern/expression too complex — internal stack overflow */
#define E_EVNTID    167  /* invalid or illegal event ID */
#define E_EVNF      168  /* named event not found */
#define E_EVBUSY    169  /* event still in use, or a duplicate create attempted */
#define E_EVPARM    170  /* invalid parameters given to the event system call */
#define E_DAMAGE    171  /* a system data structure has been corrupted */
#define E_BADREV    172  /* software revision incompatible with the running OS */
#define E_PTHLOST   173  /* network/serial/pipe connection lost */
#define E_BADPART   174  /* bad or missing partition data */
#define E_HARDWARE  175  /* driver detected a hardware fault */
#define E_SECTSIZE  176  /* sector size must be a binary multiple of 256, up to 32768 */

/* ── Operating system errors (200-239) ───────────────────────────────────── */

/* 000:200 — no confirmed symbol — path table exhausted (too many open paths) */
#define E_BPNUM     201  /* path number out of range, or refers to a closed path */
#define E_POLL      202  /* IRQ polling table exhausted */
#define E_BMODE     203  /* operation not valid for this device/file's open mode */
#define E_DEVOVF    204  /* device table exhausted */
#define E_BMID      205  /* module sync/header ID is wrong */
#define E_DIRFUL    206  /* module directory exhausted or too fragmented */
#define E_MEMFUL    207  /* not enough contiguous memory, or process at its limit */
#define E_UNKSVC    208  /* unknown service/GetStat/SetStat code */
#define E_MODBSY    209  /* non-sharable module already in use elsewhere */
#define E_BPADDR    210  /* invalid or previously-unassigned memory block address */
#define E_EOF       211  /* end of file reached on read */
#define E_VCTBSY    212  /* IRQ vector already claimed by another device */
#define E_NES       213  /* file segment missing — possible file-structure damage */
#define E_FNA       214  /* access denied — wrong permissions or ownership */
#define E_BPNAM     215  /* syntax error in a path name */
#define E_PNNF      216  /* path name not found */
#define E_SLF       217  /* file too fragmented to grow further */
#define E_CEF       218  /* create attempted, name already exists */
#define E_IBA       219  /* invalid block address or size */
#define E_HANGUP    220  /* modem/carrier lost */
#define E_MNF       221  /* module missing from the directory, or its header is damaged */
#define E_NOCLK     222  /* system clock not running */
#define E_DELSP     223  /* attempted to free the memory holding the caller's own stack */
#define E_IPRCID    224  /* process ID does not exist or isn't accessible to the caller */
#define E_PARAM     225  /* illegal or impossible parameter given to a service, see dagger note */
#define E_NOCHLD    226  /* wait requested with no child process to wait for */
#define E_ITRAP     227  /* trap code already in use, or invalid for this call */
#define E_PRCABT    228  /* process aborted via a kill signal */
#define E_PRCFUL    229  /* process table exhausted */
#define E_IFORKP    230  /* invalid parameters given to a fork call */

/*
 * Dagger note (000:225): one manual describes this generically ("an illegal
 * or impossible parameter was passed to a service request"). An independent
 * edition gives a narrower, concrete case for the same code: an impossible
 * IRQ-vector number. These aren't contradictory — the narrower case reads as
 * one instance of the general one — but since the second manual presents it
 * as *the* meaning rather than *an* example, both readings are preserved.
 */

/* ── Module and I/O device errors (231-255) ──────────────────────────────── */

#define E_KWNMOD    231  /* attempted to install a module already resident */
#define E_BMCRC     232  /* module CRC mismatch */
#define E_SIGNAL    233  /* an unprocessed signal is pending */
#define E_NEMOD     234  /* module isn't the executable type this call requires */
#define E_BNAM      235  /* syntax error in a module name */
#define E_BMHP      236  /* module header parity mismatch */
#define E_NORAM     237  /* no free RAM, or not enough contiguous memory for a fork */
#define E_DNE       238  /* attempted to un-mark a non-empty directory */
#define E_NOTASK    239  /* no task number available */
#define E_UNIT      240  /* invalid drive/unit number */
#define E_SECT      241  /* invalid disk sector number */
#define E_WP        242  /* device is write-protected */
#define E_CRC       243  /* data CRC mismatch on read or write-verify */
#define E_READ      244  /* hardware read error, or input buffer overrun */
#define E_WRITE     245  /* hardware write error */
#define E_NOTRDY    246  /* device reports not ready */
#define E_SEEK      247  /* physical seek to a non-existent location */
#define E_FULL      248  /* media has no free space left */
#define E_BTYP      249  /* media incompatible with this drive */
#define E_DEVBSY    250  /* non-sharable device already in use */
#define E_DIDC      251  /* media changed while a path was still open on it */
#define E_LOCK      252  /* record locked by another process, or a lock wait expired */
#define E_SHARE     253  /* file/device restricted to single-user access, already in use */
#define E_DEADLK    254  /* two processes each hold a resource the other needs */
#define E_FORMAT    255  /* device is format-protected */

/* ── 6809 C runtime arithmetic signals — separate numbering, not confirmed */
/* identical on a 68k compiler; the running program signals itself on these */

#define E_FPOVR     40   /* floating-point overflow/underflow */
#define E_DIVERR    41   /* division by zero */
#define E_INTERR    42   /* overflow converting float to long */

#endif /* ERRNO_FROM_DOCS_H */
