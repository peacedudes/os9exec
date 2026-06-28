/*
 *  module_from_book.h  —  OS-9/68000 module header structures
 *
 *  Derived from:
 *    "The OS-9 Guru, Book 1: The Facts" (Galactic Industrial Ltd.)
 *    https://www.icdia.co.uk/books_os9/os9guru/index.html
 *    https://archive.org/details/galactic-industrial-the-os-9-guru-1-the-facts
 *
 *  All multi-byte fields are big-endian (68000 native byte order).
 *  Access via os9_word() / os9_long() from little-endian host code.
 *
 *  Requires os9addr_t (a uint32_t alias for 68k virtual addresses) to be
 *  in scope before including this file.  Also requires uint8_t / uint16_t /
 *  uint32_t from <stdint.h>.
 *
 *  Compile-time assertions at the bottom verify every key offset.
 */

#ifndef MODULE_FROM_BOOK_H
#define MODULE_FROM_BOOK_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>   /* ushort / ulong used by os9_ll.h that follows */

/* ── MODSYNC ────────────────────────────────────────────────────────────── */
#define MODSYNC  0x4AFC   /* module sync word — illegal 68000 instruction */

/* ── Module type codes (_mtylan high byte) — book §3.2.7, p.39 ─────────── */
#define MT_ANY      0   /* link to any type */
#define MT_PROGRAM  1   /* program */
#define MT_SUBROUT  2   /* subroutine */
#define MT_MULTI    3   /* multi-module */
#define MT_DATA     4   /* data module */
#define MT_CSDDATA  6   /* colored-memory data (OS-9 Professional extension) */
#define MT_TRAPLIB  11  /* trap handler */
#define MT_SYSTEM   12  /* OS component (non-I/O) */
#define MT_FILEMAN  13  /* file manager */
#define MT_DEVDRVR  14  /* device driver */
#define MT_DEVDESC  15  /* device descriptor */

/* ── Module language codes (_mtylan low byte) — book §3.2.7, p.40 ──────── */
#define ML_ANY      0   /* link to any language */
#define ML_OBJECT   1   /* 68000 object code */
#define ML_BASIC    2   /* compiled Basic intermediate code */
#define ML_PASCAL   3   /* compiled Pascal intermediate code */
#define ML_C        4   /* compiled C intermediate code */
#define ML_COBOL    5   /* compiled Cobol intermediate code */
#define ML_FORTRAN  6   /* compiled Fortran intermediate code */

/* ── Module attribute flags (_mattrev high byte) — book §3.2.8 ─────────── */
#define MA_REENT    0x80  /* module is re-entrant / sharable */
#define MA_GHOST    0x40  /* ghost (sticky) module */
#define MA_SUPER    0x20  /* must execute in supervisor state */

/* ══════════════════════════════════════════════════════════════════════════
 *  modhcom — universal module header ($000–$02F, 48 bytes)
 *  Present in every module type; ends with _mparity.
 * ══════════════════════════════════════════════════════════════════════════ */
typedef struct modhcom {

    /* ── Identification ─────────────────────────────────── $000–$003 ── */
    uint16_t  _msync;     /* $000  sync word — always $4AFC */
    uint16_t  _msysrev;   /* $002  OS-9 system revision ID (currently $0001) */

    /* ── Size and ownership ─────────────────────────────── $004–$00F ── */
    uint32_t  _msize;     /* $004  total module size including header and CRC */
    uint32_t  _mowner;    /* $008  creator: high word = group, low word = user */
    os9addr_t _mname;     /* $00C  offset from header start to module name string */

    /* ── Permissions and type ───────────────────────────── $010–$017 ── */
    uint16_t  _maccess;   /* $010  access permissions (12 flag bits) */
    uint16_t  _mtylan;    /* $012  high byte = type code, low byte = language code */
    uint16_t  _mattrev;   /* $014  high byte = attributes flags, low byte = revision */
    uint16_t  _medit;     /* $016  software edition number (not used by kernel) */

    /* ── Optional offsets ───────────────────────────────── $018–$01F ── */
    os9addr_t _musage;    /* $018  offset to usage/comments string (unused) */
    os9addr_t _msymbol;   /* $01C  offset to symbol table (unused) */

    /* ── Reserved / ident ───────────────────────────────── $020–$02D ── */
    uint16_t  _mident;    /* $020  ident code for ident utility (unused) */
    uint8_t   _mspare[12];/* $022  reserved — 12 bytes */

    /* ── Header parity ──────────────────────────────────── $02E–$02F ── */
    uint16_t  _mparity;   /* $02E  one's-complement XOR of all preceding words */

} modhcom;                /* sizeof = $030 = 48 bytes */

/* ══════════════════════════════════════════════════════════════════════════
 *  mod_exec — executable / subroutine / trap program module
 *  Extended header follows modhcom at $030.
 * ══════════════════════════════════════════════════════════════════════════ */
typedef struct mod_exec {
    modhcom   _mh;        /* $000  common header (48 bytes) */

    /* ── Program extended header ────────────────────────── $030–$047 ── */
    os9addr_t _mexec;     /* $030  offset to program entry point */
    os9addr_t _mexcpt;    /* $034  offset to default trap (exception) handler */
    uint32_t  _mdata;     /* $038  minimum static storage (data space) required */
    uint32_t  _mstack;    /* $03C  minimum stack size required */
    os9addr_t _midata;    /* $040  offset to data initialization table */
    os9addr_t _midref;    /* $044  offset to data pointer initialization tables */

} mod_exec;               /* sizeof = $048 = 72 bytes */

/* mod_trap is defined in os9_ll.h (which extends mod_exec with _mtrapinit
 * and _mtrapterm).  We don't redefine it here to avoid a redefinition error.
 * The book documents only _mtrapinit at $048; os9exec adds _mtrapterm. */

/* ══════════════════════════════════════════════════════════════════════════
 *  mod_dev — device descriptor module ($030 onward from book §7.3)
 * ══════════════════════════════════════════════════════════════════════════ */
typedef struct mod_dev {
    modhcom  _mh;         /* $000  common header (48 bytes) */

    /* ── Hardware configuration ─────────────────────────── $030–$037 ── */
    os9addr_t _mport;     /* $030  M$Port:    hardware interface port address */
    uint8_t  _mvect;      /* $034  M$Vector:  interrupt vector number */
    uint8_t  _mirqlvl;    /* $035  M$IRQLvl:  interrupt request level (1–7) */
    uint8_t  _mprior;     /* $036  M$Prior:   interrupt software polling priority */
    uint8_t  _mmode;      /* $037  M$Mode:    device capability flags */

    /* ── Module name offsets ────────────────────────────── $038–$03D ── */
    uint16_t _mfmgr;      /* $038  M$FMgr:    offset to file manager name string */
    uint16_t _mpdev;      /* $03A  M$PDev:    offset to device driver name string */
    uint16_t _mdevcon;    /* $03C  M$DevCon:  offset to optional extra device info */

    /* ── Reserved ───────────────────────────────────────── $03E–$045 ── */
    uint8_t  _mres[8];    /* $03E  reserved */

    /* ── Options section ────────────────────────────────── $046–$047+ ─ */
    uint16_t _mopt;       /* $046  M$Opt:     size of options section in bytes */
    uint8_t  _mdtype[128];/* $048  options section (copied to path descriptor on open) */

} mod_dev;

/* ══════════════════════════════════════════════════════════════════════════
 *  mod_driver — device driver module (stub — only modhcom accessed)
 * ══════════════════════════════════════════════════════════════════════════ */
typedef struct mod_driver {
    modhcom   _mh;        /* $000  common header */
    os9addr_t _mexec;     /* $030  entry table offset */
    os9addr_t _mexcpt;    /* $034  exception handler offset */
    uint32_t  _mdata;     /* $038  static storage size */
} mod_driver;

/* ══════════════════════════════════════════════════════════════════════════
 *  mod_config — configuration / init module (stub)
 * ══════════════════════════════════════════════════════════════════════════ */
typedef struct mod_config {
    modhcom   _mh;        /* $000  common header */
    os9addr_t _mexec;     /* $030  init routine offset */
} mod_config;

/* ── Compile-time layout verification ──────────────────────────────────── */

#define MODHCOM_CHECK(field, expected_offset) \
    typedef char modhcom_offset_check_##field \
        [ (offsetof(modhcom, field) == (expected_offset)) ? 1 : -1 ]

MODHCOM_CHECK(_msync,   0x000);
MODHCOM_CHECK(_msysrev, 0x002);
MODHCOM_CHECK(_msize,   0x004);
MODHCOM_CHECK(_mowner,  0x008);
MODHCOM_CHECK(_mname,   0x00C);
MODHCOM_CHECK(_maccess, 0x010);
MODHCOM_CHECK(_mtylan,  0x012);
MODHCOM_CHECK(_mattrev, 0x014);
MODHCOM_CHECK(_medit,   0x016);
MODHCOM_CHECK(_musage,  0x018);
MODHCOM_CHECK(_msymbol, 0x01C);
MODHCOM_CHECK(_mident,  0x020);
MODHCOM_CHECK(_mspare,  0x022);
MODHCOM_CHECK(_mparity, 0x02E);

typedef char modhcom_size_check[(sizeof(modhcom) == 0x030) ? 1 : -1];

#define MOD_EXEC_CHECK(field, expected_offset) \
    typedef char mod_exec_offset_check_##field \
        [ (offsetof(mod_exec, field) == (expected_offset)) ? 1 : -1 ]

MOD_EXEC_CHECK(_mh,     0x000);
MOD_EXEC_CHECK(_mexec,  0x030);
MOD_EXEC_CHECK(_mexcpt, 0x034);
MOD_EXEC_CHECK(_mdata,  0x038);
MOD_EXEC_CHECK(_mstack, 0x03C);
MOD_EXEC_CHECK(_midata, 0x040);
MOD_EXEC_CHECK(_midref, 0x044);

typedef char mod_exec_size_check[(sizeof(mod_exec) == 0x048) ? 1 : -1];

#define MOD_DEV_CHECK(field, expected_offset) \
    typedef char mod_dev_offset_check_##field \
        [ (offsetof(mod_dev, field) == (expected_offset)) ? 1 : -1 ]

MOD_DEV_CHECK(_mh,      0x000);
MOD_DEV_CHECK(_mport,   0x030);
MOD_DEV_CHECK(_mvect,   0x034);
MOD_DEV_CHECK(_mirqlvl, 0x035);
MOD_DEV_CHECK(_mprior,  0x036);
MOD_DEV_CHECK(_mmode,   0x037);
MOD_DEV_CHECK(_mfmgr,   0x038);
MOD_DEV_CHECK(_mpdev,   0x03A);
MOD_DEV_CHECK(_mdevcon, 0x03C);
MOD_DEV_CHECK(_mres,    0x03E);
MOD_DEV_CHECK(_mopt,    0x046);
MOD_DEV_CHECK(_mdtype,  0x048);

#undef MODHCOM_CHECK
#undef MOD_EXEC_CHECK
#undef MOD_DEV_CHECK

#endif /* MODULE_FROM_BOOK_H */
