/*
 *  sgstat_from_book.h  —  OS-9/68000 path descriptor and I/O status codes
 *
 *  Primary source:
 *    "The OS-9 Guru, Book 1: The Facts" (Galactic Industrial Ltd.)
 *    https://www.icdia.co.uk/books_os9/os9guru/index.html
 *    https://archive.org/details/galactic-industrial-the-os-9-guru-1-the-facts
 *
 *  Two SS_ values are book-explicit (§7.7.1):
 *    SS_Opt   = 0x0000  (book line 1635)
 *    SS_DevNm = 0x000E  (book line 1636)
 *
 *  Device type codes from book §7.7.2 (lines 1657–1666).
 *
 *  PD_ offsets derived from book Appendix B path descriptor tables.
 *  Note: SCF and RBF options sections start at the same offset (PD_OPT = 0x080).
 *  Many PD_ names share the same numeric offset — this is intentional:
 *  the same memory is interpreted differently depending on the file manager.
 *
 *  os9exec_nt.h also defines a subset of these PD_ constants.  Because this
 *  file is included first (via os9exec_incl.h), all defines here use #ifndef
 *  guards so they can be augmented but never silently overridden.
 */

#ifndef SGSTAT_FROM_BOOK_H
#define SGSTAT_FROM_BOOK_H

#include <stdint.h>
#include <stddef.h>

/* ── SCF options section struct ─────────────────────────────────────────── */
/* Layout derived from the initializer in utilstuff.c (init_consoleopts)    */
/* and the PD_ offset table.  Fields used by name in the codebase are given */
/* their canonical _sgs_ names; unused slots get placeholder names.         */
/* Total size = 128 bytes (OPTSECTSIZE).                                     */

struct _sgs {
    uint8_t  _sgs_dtp;    /* 0x00  PD_DTP   device type (0=SCF) */
    uint8_t  _sgs_case;   /* 0x01  PD_UPC   upper-case-only flag */
    uint8_t  _sgs_backsp; /* 0x02  PD_BSO   backspace output style */
    uint8_t  _sgs_dlo;    /* 0x03  PD_DLO   delete-line output style */
    uint8_t  _sgs_echo;   /* 0x04  PD_EKO   echo flag */
    uint8_t  _sgs_alf;    /* 0x05  PD_ALF   auto-linefeed flag */
    uint8_t  _sgs_nul;    /* 0x06  PD_NUL   null count after CR */
    uint8_t  _sgs_pause;  /* 0x07  PD_PAU   page-pause flag */
    uint8_t  _sgs_page;   /* 0x08  PD_PAG   page length in lines */
    uint8_t  _sgs_bspch;  /* 0x09  PD_BSP   backspace key character */
    uint8_t  _sgs_dlnch;  /* 0x0A  PD_DEL   delete-line key character */
    uint8_t  _sgs_eorch;  /* 0x0B  PD_EOR   end-of-record (CR) character */
    uint8_t  _sgs_eofch;  /* 0x0C  PD_EOF   end-of-file character */
    uint8_t  _sgs_rprch;  /* 0x0D  PD_RPR   reprint-line key */
    uint8_t  _sgs_dulnch; /* 0x0E  PD_DUP   duplicate-last-line key */
    uint8_t  _sgs_pscch;  /* 0x0F  PD_PSC   pause-scroll key */
    uint8_t  _sgs_kbich;  /* 0x10  PD_INT   keyboard interrupt character */
    uint8_t  _sgs_kbach;  /* 0x11  PD_QUT   keyboard quit/abort character */
    uint8_t  _sgs_bsech;  /* 0x12  PD_BSE   backspace echo character */
    uint8_t  _sgs_ovfch;  /* 0x13  PD_OVF   line-overflow character */
    uint8_t  _sgs_par;    /* 0x14  PD_PAR   parity/format code */
    uint8_t  _sgs_bau;    /* 0x15  PD_BAU   baud rate code */
    uint16_t _sgs_d2p;    /* 0x16  PD_D2P   offset to output device name */
    uint8_t  _sgs_xon;    /* 0x18  PD_XON   XON character */
    uint8_t  _sgs_xoff;   /* 0x19  PD_XOFF  XOFF character */
    uint8_t  _sgs_tab;    /* 0x1A  PD_TAB   tab character */
    uint8_t  _sgs_tabs;   /* 0x1B  PD_TABS  tab field size */
    uint32_t _sgs_tbl;    /* 0x1C  PD_TBL   device table address (os9exec use) */
    uint16_t _sgs_col;    /* 0x20  PD_Col   current column number */
    uint8_t  _sgs_err;    /* 0x22  PD_Err   last I/O error */
    uint8_t  _sgs_spare[93]; /* 0x23  spare to end of 128-byte options section */
};

typedef char _sgs_size_check[(sizeof(struct _sgs) == 128) ? 1 : -1];

/* ── Options section anchor ─────────────────────────────────────────────── */
/* PD_OPT is the absolute offset of the options section in the path         */
/* descriptor; PthOffs is the same value, used by os9exec_nt.h arithmetic.  */

#ifndef PthOffs
#define PthOffs     0x80
#endif
#ifndef PD_OPT
#define PD_OPT      0x0080  /* start of options section (absolute offset) */
#endif

/* ── Common path descriptor header (absolute offsets from PD base) ──────── */
/* Source: book Appendix B.  None of these appear in os9exec_nt.h.          */

#define PD_PD       0x0000  /* w  path number */
#define PD_MOD      0x0002  /* b  access mode */
#define PD_CNT      0x0003  /* b  open count */
#define PD_DEV      0x0004  /* l  device table entry pointer */
#define PD_CPR      0x0008  /* w  current process ID */
#define PD_RGS      0x000A  /* l  caller's register stack ptr */
#define PD_BUF      0x000E  /* l  data buffer pointer */
#define PD_USER     0x0012  /* l  user ID */
#define PD_Paths    0x0016  /* l  linked list of open paths */
#define PD_FST      0x002A  /* b[86] file manager storage area */

/* ── Options section offsets — relative to start of options (PthOffs) ───── */
/* These are the "PD_xxx = absolute - PthOffs" values used throughout.      */
/* Offsets listed here are relative to the options section start (0x80).    */

/* ── ALL device types: byte 0 of options ───────────────────────────────── */
#ifndef PD_DTP
#define PD_DTP      0x80 - PthOffs   /* b  device type code (DT_xxx) */
#endif

/* ── SCF (sequential character) path descriptor options ─────────────────── */
/* Absolute $080–$0A2; relative 0x00–0x22.                                  */

#ifndef PD_UPC
#define PD_UPC      0x81 - PthOffs   /* b  case: 0=both, 1=upper-only */
#endif
#ifndef PD_BSO
#define PD_BSO      0x82 - PthOffs   /* b  backspace output style */
#endif
#ifndef PD_DLO
#define PD_DLO      0x83 - PthOffs   /* b  delete-line output style */
#endif
#ifndef PD_EKO
#define PD_EKO      0x84 - PthOffs   /* b  echo flag */
#endif
#ifndef PD_ALF
#define PD_ALF      0x85 - PthOffs   /* b  auto-linefeed flag */
#endif
#ifndef PD_NUL
#define PD_NUL      0x86 - PthOffs   /* b  null count after CR */
#endif
#ifndef PD_PAU
#define PD_PAU      0x87 - PthOffs   /* b  page pause flag */
#endif
#ifndef PD_PAG
#define PD_PAG      0x88 - PthOffs   /* b  page length in lines */
#endif
#ifndef PD_BSP
#define PD_BSP      0x89 - PthOffs   /* b  backspace key character */
#endif
#ifndef PD_DEL
#define PD_DEL      0x8A - PthOffs   /* b  delete-line key character */
#endif
#ifndef PD_EOR
#define PD_EOR      0x8B - PthOffs   /* b  end-of-record (CR) character */
#endif
#ifndef PD_EOF
#define PD_EOF      0x8C - PthOffs   /* b  end-of-file character */
#endif
#ifndef PD_RPR
#define PD_RPR      0x8D - PthOffs   /* b  reprint-line key */
#endif
#ifndef PD_DUP
#define PD_DUP      0x8E - PthOffs   /* b  duplicate-last-line key */
#endif
#ifndef PD_PSC
#define PD_PSC      0x8F - PthOffs   /* b  pause-scroll key */
#endif
/* PD_INT / PD_QUT: also defined by os9exec_nt.h — use same expression     */
#ifndef PD_INT
#define PD_INT      0x90 - PthOffs   /* b  keyboard interrupt char (^C) */
#endif
#ifndef PD_QUT
#define PD_QUT      0x91 - PthOffs   /* b  keyboard quit char (^\) */
#endif
#ifndef PD_BSE
#define PD_BSE      0x92 - PthOffs   /* b  backspace echo character */
#endif
#ifndef PD_OVF
#define PD_OVF      0x93 - PthOffs   /* b  line-buffer overflow char */
#endif
#ifndef PD_PAR
#define PD_PAR      0x94 - PthOffs   /* b  parity/format code */
#endif
#ifndef PD_BAU
#define PD_BAU      0x95 - PthOffs   /* b  baud rate code */
#endif
/* 0x96–0x97: reserved for SCF */
/* PD_XON / PD_XOFF: also defined by os9exec_nt.h — use same expression    */
#ifndef PD_XON
#define PD_XON      0x98 - PthOffs   /* b  XON character (^Q) */
#endif
#ifndef PD_XOFF
#define PD_XOFF     0x99 - PthOffs   /* b  XOFF character (^S) */
#endif
#ifndef PD_TAB
#define PD_TAB      0x9A - PthOffs   /* b  tab character */
#endif
#ifndef PD_TABS
#define PD_TABS     0x9B - PthOffs   /* b  tab field size */
#endif
#ifndef PD_TBL
#define PD_TBL      0x9C - PthOffs   /* b  tab table length */
#endif
/* 0x9D–0x9F: tab table entries (variable) */
#ifndef PD_Col
#define PD_Col      0xA0 - PthOffs   /* w  current column number */
#endif
#ifndef PD_Err
#define PD_Err      0xA2 - PthOffs   /* b  last I/O error */
#endif

/* ── RBF (random block) path descriptor options ─────────────────────────── */
/* Share PD_DTP at $080; RBF-specific fields start at $081.                  */
/* Many offsets OVERLAP SCF offsets — same memory, different interpretation. */

#ifndef PD_DRV
#define PD_DRV      0x81 - PthOffs   /* b  logical drive number */
#endif
#ifndef PD_STP
#define PD_STP      0x82 - PthOffs   /* b  step rate */
#endif
/* PD_TYP: already defined by os9exec_nt.h */
#ifndef PD_TYP
#define PD_TYP      0x83 - PthOffs   /* b  media type flags */
#endif
#ifndef PD_DNS
#define PD_DNS      0x84 - PthOffs   /* b  density */
#endif
#ifndef PD_CYL
#define PD_CYL      0x85 - PthOffs   /* w  number of cylinders */
#endif
#ifndef PD_SID
#define PD_SID      0x87 - PthOffs   /* b  number of sides */
#endif
#ifndef PD_VFY
#define PD_VFY      0x88 - PthOffs   /* b  verify writes (0=yes) */
#endif
#ifndef PD_TOS
#define PD_TOS      0x89 - PthOffs   /* w  track-zero offset in sectors */
#endif
/* PD_SCT: already defined by os9exec_nt.h */
#ifndef PD_SCT
#define PD_SCT      0x8A - PthOffs   /* w  sectors per track */
#endif
#ifndef PD_ILV
#define PD_ILV      0x8F - PthOffs   /* b  sector interleave factor */
#endif
#ifndef PD_TFM
#define PD_TFM      0x90 - PthOffs   /* b  DMA transfer mode */
#endif
#ifndef PD_TOffs
#define PD_TOffs    0x91 - PthOffs   /* b  track-to-track offset */
#endif
#ifndef PD_SOffs
#define PD_SOffs    0x93 - PthOffs   /* b  sector offset */
#endif
/* PD_SSize: already defined by os9exec_nt.h */
#ifndef PD_SSize
#define PD_SSize    0x94 - PthOffs   /* w  sector size in bytes */
#endif
#ifndef PD_Cntl
#define PD_Cntl     0x96 - PthOffs   /* w  drive control flags */
#endif
#ifndef PD_Trys
#define PD_Trys     0x98 - PthOffs   /* b  number of retries */
#endif
#ifndef PD_LUN
#define PD_LUN      0x99 - PthOffs   /* b  SCSI logical unit number */
#endif
#ifndef PD_WPC
#define PD_WPC      0x9A - PthOffs   /* w  write precompensation cylinder */
#endif
#ifndef PD_RWR
#define PD_RWR      0x9C - PthOffs   /* w  reduced-write cylinder */
#endif
#ifndef PD_Park
#define PD_Park     0x9E - PthOffs   /* w  head park cylinder */
#endif
#ifndef PD_LSNOffs
#define PD_LSNOffs  0xA0 - PthOffs   /* l  logical sector number offset */
#endif
#ifndef PD_TotCyls
#define PD_TotCyls  0xA4 - PthOffs   /* w  total cylinders */
#endif
/* PD_CtrlID / PD_CtrlrID: already defined by os9exec_nt.h */
#ifndef PD_CtrlID
#define PD_CtrlID   0xA6 - PthOffs   /* b  controller ID */
#endif
#ifndef PD_Rate
#define PD_Rate     0xA7 - PthOffs   /* b  transfer rate */
#endif
/* PD_SAS: already defined by os9exec_nt.h */
#ifndef PD_SAS
#define PD_SAS      0x8E - PthOffs   /* b  segment allocation size */
#endif
/* PD_ATT, PD_FD, PD_DFD, PD_DCP, PD_DVT, PD_SctSiz, PD_NAME:            */
/* all defined by os9exec_nt.h.                                             */

/* ── Device type codes — book §7.7.2 ───────────────────────────────────── */

#define DT_SCF      0   /* sequential character file manager */
#define DT_RBF      1   /* random block file manager */
#define DT_Pipe     2   /* pipe file manager */
#define DT_SBF      3   /* sequential block (tape) file manager */
#define DT_NFM      4   /* Microware network file manager */
#define DT_CDFM     5   /* compact disc (CD-I) file manager */
#define DT_UCM      6   /* user-interface communications (CD-I) */
#undef  DT_SOCK           /* macOS dirent.h defines DT_SOCK=12 for a different purpose */
#define DT_SOCK     7   /* logical socket (ISP) */
#define DT_PTTY     8   /* pseudo-keyboard (ISP) */
#define DT_INET     9   /* internet protocol (ISP) */
#define DT_NRF      10  /* non-volatile memory (CD-I) */
#define DT_GFM      11  /* graphics display (CD-I) */

/* SS_ SetStat/GetStat codes are defined in os9funcs.h with verified values. */
/* The two book-explicit values (SS_Opt=0x00, SS_DevNm=0x0E) match exactly. */

#endif /* SGSTAT_FROM_BOOK_H */
