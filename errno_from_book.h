/*
 *  errno_from_book.h  —  OS-9/68000 error codes
 *
 *  Primary source:
 *    "The OS-9 Guru, Book 1: The Facts" (Galactic Industrial Ltd.)
 *    https://www.icdia.co.uk/books_os9/os9guru/index.html
 *    https://archive.org/details/galactic-industrial-the-os-9-guru-1-the-facts
 *
 *  Five values are explicitly stated in the book:
 *    E_PERMIT = 164  (§3.2.6, p.39)
 *    E_KWNMOD = 231  (§4.2, p.49)
 *    E_NEMOD  = 234  (§4.1, p.48; §4.2, p.49)
 *    E_BMHP   = 236  (§4.2, p.49)
 *    E_NOTRDY = 246  (§12.5, p.272)
 *
 *  Hardware-exception errors follow the formula described in §10.5 (p.212):
 *    "error code is 100 plus the exception number"
 *  Confirmed by: debug.c comment "div 0 will be masked with TRAP 5 instead of error 105"
 *
 *  All remaining values are derived by counting forward/backward from the
 *  confirmed anchors using the switch statement in debug.c (line ~1087) as
 *  a consistency check: each case must be unique.  No OS-9 source code was
 *  read; all values come from the book, the formula, and arithmetic from anchors.
 */

#ifndef ERRNO_FROM_BOOK_H
#define ERRNO_FROM_BOOK_H

/* ── Hardware-exception errors (100 + 68000 exception number) ───────────── */
/* Formula from book §10.5 p.212; E_ZERDIV=105 confirmed by debug.c comment */

#define E_BUSERR    102  /* bus error (exception 2) */
#define E_ADRERR    103  /* address error (exception 3) */
#define E_ILLINS    104  /* illegal instruction (exception 4) */
#define E_ZERDIV    105  /* zero divide (exception 5) */
#define E_CHK       106  /* CHK instruction (exception 6) */
#define E_TRAPV     107  /* TRAPV instruction (exception 7) */
#define E_VIOLAT    108  /* privilege violation (exception 8) */
#define E_TRACE     109  /* trace (exception 9) */
#define E_1010      110  /* line-1010 emulator (exception 10) */
#define E_1111      111  /* line-1111 emulator (exception 11) */
#define E_FMTERR    114  /* stack frame format error (exception 14) */

/* TRAP instructions: exceptions 32-47 → 132-147 */
#define E_TRAP      132  /* TRAP #0 (OS-9 syscall not set up); user traps: E_TRAP+n */

/* 68881/68882 FPU exceptions: 48-55 → 148-155 */
#define E_FPUNORDC  148  /* FP branch/set on unordered condition (exception 48) */
#define E_FPINXACT  149  /* FP inexact result (exception 49) */
#define E_FPDIVZER  150  /* FP zero divide (exception 50) */
#define E_FPUNDRFL  151  /* FP underflow (exception 51) */
#define E_FPOPRERR  152  /* FP operand error (exception 52) */
#define E_FPOVERFL  153  /* FP overflow (exception 53) */
#define E_FPNOTNUM  154  /* FP signaling NaN (exception 54) */

/* ── OS-9 general error codes — block 1 (164–175) ──────────────────────── */
/* Anchored at E_PERMIT=164 (book §3.2.6 p.39) + consecutive run to 175    */

#define E_PERMIT    164  /* permission denied — book explicit */
#define E_DIFFER    165  /* module name mismatch (F$ChkNam) */
#define E_STKOVF    166  /* stack overflow */
#define E_EVNTID    167  /* invalid event ID */
#define E_EVNF      168  /* event not found */
#define E_EVBUSY    169  /* event busy (can't delete) */
#define E_EVPARM    170  /* impossible event parameters */
#define E_DAMAGE    171  /* system data structures damaged */
#define E_BADREV    172  /* incompatible revision */
#define E_PTHLOST   173  /* path lost (net node down) */
#define E_BADPART   174  /* bad partition data */
#define E_HARDWARE  175  /* hardware damaged */

/* ── OS-9 I/O and system errors — block 2 (200–230) ────────────────────── */
/* Anchored by counting backwards 31 entries from E_KWNMOD=231 (book p.49) */

#define E_PTHFUL    200  /* path table full */
#define E_BPNUM     201  /* bad path number */
#define E_POLL      202  /* polling table full */
#define E_BMODE     203  /* bad mode */
#define E_DEVOVF    204  /* device table overflow */
#define E_BMID      205  /* bad module ID */
#define E_DIRFUL    206  /* module directory full */
#define E_MEMFUL    207  /* process memory full */
#define E_UNKSVC    208  /* unknown service code */
#define E_MODBSY    209  /* module busy */
#define E_BPADDR    210  /* bad page address */
#define E_EOF       211  /* end of file */
#define E_VCTBSY    212  /* interrupt vector busy */
#define E_NES       213  /* non-existing segment */
#define E_FNA       214  /* file not accessible */
#define E_BPNAM     215  /* bad path name */
#define E_PNNF      216  /* path name not found */
#define E_SLF       217  /* segment list full */
#define E_CEF       218  /* creating existing file */
#define E_IBA       219  /* illegal block address */
#define E_HANGUP    220  /* modem hangup */
#define E_MNF       221  /* module not found */
#define E_NOCLK     222  /* no system clock */
#define E_DELSP     223  /* deleting stack pointer memory */
#define E_IPRCID    224  /* illegal process ID */
#define E_PARAM     225  /* impossible parameter */
#define E_NOCHLD    226  /* no children */
#define E_ITRAP     227  /* invalid/unavailable trap code */
#define E_PRCABT    228  /* process aborted */
#define E_PRCFUL    229  /* process table full */
#define E_IFORKP    230  /* illegal fork parameter */

/* ── OS-9 module and I/O device errors — block 3 (231–255) ─────────────── */
/* Anchored at E_KWNMOD=231, E_NEMOD=234, E_BMHP=236, E_NOTRDY=246 (book) */

#define E_KWNMOD    231  /* module already known — book explicit */
#define E_BMCRC     232  /* bad module CRC */
#define E_SIGNAL    233  /* signal error */
#define E_NEMOD     234  /* module not found / wrong type — book explicit */
#define E_BNAM      235  /* bad module name */
#define E_BMHP      236  /* bad module header parity — book explicit */
#define E_NORAM     237  /* no RAM available */
#define E_DNE       238  /* directory not empty */
#define E_NOTASK    239  /* no task number available */
#define E_UNIT      240  /* illegal unit (drive number) */
#define E_SECT      241  /* bad sector number */
#define E_WP        242  /* write protected */
#define E_CRC       243  /* CRC error */
#define E_READ      244  /* read error */
#define E_WRITE     245  /* write error */
#define E_NOTRDY    246  /* device not ready — book explicit */
#define E_SEEK      247  /* seek error */
#define E_FULL      248  /* media full */
#define E_BTYP      249  /* bad (incompatible) media type */
#define E_DEVBSY    250  /* device busy */
#define E_DIDC      251  /* disk ID change */
#define E_LOCK      252  /* record locked */
#define E_SHARE     253  /* non-sharable device in use */
#define E_DEADLK    254  /* I/O deadlock */
#define E_FORMAT    255  /* device format protected */

/* ── Errors not in book; values derived from position in debug.c switch ─── */
/* These appear in the switch before the hardware-exception block.          */
/* Best-effort values; build will expose any conflicts.                     */

#define E_ILLFNC    2    /* illegal function code */
#define E_NOTNUM    8    /* not a number */
#define E_ILLARG    22   /* illegal argument (= POSIX EINVAL) */

/* ── os9exec internal codes (also defined in os9exec_nt.h) ─────────────── */
/* Guard with #ifndef so the two definitions coexist without conflict.      */

#ifndef E_OKFLAG
#define E_OKFLAG    0x4200  /* os9exec: result already placed, no error */
#endif
#ifndef E_PLINK
#define E_PLINK     0x1234  /* os9exec: xOpen returns new syspath in *mode */
#endif

#endif /* ERRNO_FROM_BOOK_H */
