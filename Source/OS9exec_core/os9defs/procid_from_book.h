/*
 *  procid_from_book.h  —  OS-9/68000 process descriptor
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
 *  in scope before including this file.
 *
 *  Field names are kept compatible with the existing os9exec codebase.
 *  Total descriptor size: 2048 bytes ($000–$7FF).
 *
 *  Compile-time assertions at the bottom verify every key offset.
 */

#ifndef PROCID_FROM_BOOK_H
#define PROCID_FROM_BOOK_H

#include <stdint.h>
#include <stddef.h>

typedef struct {

    /* ── Basic process linkage ──────────────────────────── $000–$007 ── */
    uint16_t  _id;          /* $000  P$ID:    this process's ID */
    uint16_t  _pid;         /* $002  P$PID:   parent process ID */
    uint16_t  _sid;         /* $004  P$SID:   sibling process ID */
    uint16_t  _cid;         /* $006  P$CID:   child process ID */

    /* ── Stack pointers ─────────────────────────────────── $008–$00F ── */
    os9addr_t _sp;          /* $008  P$SP:    system stack pointer — saved after
                             *       registers are stacked on last system call;
                             *       points to the processor register save frame */
    os9addr_t _usp;         /* $00C  P$USP:   user-state stack pointer */

    /* ── Memory ─────────────────────────────────────────── $010–$013 ── */
    uint32_t  _pagcnt;      /* $010  P$Pag:   memory page / size accounting */

    /* ── Ownership and scheduling ───────────────────────── $014–$01F ── */
    uint16_t  _group;       /* $014  P$Group: owner group number */
    uint16_t  _user;        /* $016  P$User:  owner user number */
    uint16_t  _prior;       /* $018  P$Prior: execution priority */
    uint16_t  _age;         /* $01A  P$Age:   scheduling age counter */
    uint16_t  _state;       /* $01C  P$State: process state flags */
    uint16_t  _task;        /* $01E  P$Task:  task / MMU slot number */

    /* ── Miscellaneous single-byte fields ───────────────── $020–$023 ── */
    uint8_t   _queueid;     /* $020  P$QueuID: queue this process is in */
    uint8_t   _scall;       /* $021  P$SCall:  last system call code */
    uint8_t   _baked;       /* $022  P$Baked:  fork-complete flag */
    uint8_t   _pad0;        /* $023  (reserved — keeps $024 word-aligned) */

    /* ── Signals and IPC ────────────────────────────────── $024–$037 ── */
    uint16_t  _deadlk;      /* $024  P$DeadLk: ID of process to which I/O
                             *       deadlock has been lost */
    uint16_t  _signal;      /* $026  P$Signal: pending signal code (0 = none) */
    os9addr_t _sigvec;      /* $028  P$SigVec: signal handler function address */
    os9addr_t _sigdat;      /* $02C  P$SigDat: signal handler data address */

    /* ── Queue links ────────────────────────────────────── $030–$037 ── */
    os9addr_t _queuen;      /* $030  P$QueueN: next descriptor in queue (fwd link) */
    os9addr_t _queuep;      /* $034  P$QueueP: previous descriptor in queue (bk link) */

    /* ── Module ─────────────────────────────────────────── $038–$03B ── */
    os9addr_t _pmodul;      /* $038  P$PModul: address of the program module */

    /* ── Hardware exception handlers ────────────────────── $03C–$063 ── */
    os9addr_t except[10];   /* $03C  P$Except: handler addresses for bus error,
                             *       address error, illegal instruction, etc. [0..9]
                             *       Zero = kill process on that exception */

    /* ── Hardware exception stack areas ────────────────── $064–$08B ── */
    os9addr_t _exstk[10];   /* $064  P$ExStk:  static storage areas in which to
                             *       build the register save frame when the
                             *       corresponding hardware exception occurs [0..9]
                             *       Zero = use the user-state stack */

    /* ── Trap handlers ($08C–$13F) ─────────────────────────────────── */
    os9addr_t _traps[15];   /* $08C  PtTraps:  addresses of trap handler modules
                             *       for TRAP #1–#15 (index 0 = TRAP #1) */
    os9addr_t _trpmem[15];  /* $0C8  PtTrpMem: static storage addresses allocated
                             *       for each trap handler module */
    uint32_t  _trpsiz[15];  /* $104  PtTrpSiz: sizes of those static storage areas */

    /* ── System-state exception recovery ───────────────── $140–$147 ── */
    os9addr_t _excpsp;      /* $140  P$ExcpSP: SP to load if hardware exception
                             *       occurs while this process is in system state */
    os9addr_t _excppc;      /* $144  P$ExcpPC: handler address for same;
                             *       zero = fatal reboot */

    /* ── Directory information ──────────────────────────── $148–$167 ── */
    uint8_t   _dio[32];     /* $148  P$DIO:    current data dir (16 bytes) and
                             *       execution dir (16 bytes) device/path info */

    /* ── Open path table ────────────────────────────────── $168–$1A7 ── */
    uint16_t  _path[32];    /* $168  P$Path:   local path# → system path# map;
                             *       zero entry = path not open */

    /* ── Memory allocation tables ──────────────────────── $1A8–$2A7 ── */
    os9addr_t _memimg[32];  /* $1A8  P$MemImg: addresses of allocated memory areas,
                             *       ordered by address (low first) */
    uint32_t  _blksiz[32];  /* $228  P$BlkSiz: sizes of those areas (system-rounded) */

    /* ── Debugger support ───────────────────────────────── $2A8–$2B3 ── */
    os9addr_t _dbgreg;      /* $2A8  P$DbgReg: address of the register-frame buffer
                             *       in the parent (debugger) process's static storage.
                             *       After F$DFork or F$DExec the kernel copies the
                             *       child's D0-D7/A0-A6/PC/SR to this buffer so the
                             *       parent can inspect them. */
    os9addr_t _dbgpar;      /* $2AC  P$DbgPar: address of parent's process descriptor;
                             *       zero means this process is not being debugged */
    uint32_t  _dbgins;      /* $2B0  P$DbgIns: instruction count for current F$DExec */

    /* ── Statistics and accounting ──────────────────────── $2B4–$2D3 ── */
    uint32_t  _uticks;      /* $2B4  P$UTicks:  tick count while running in user state */
    uint32_t  _sticks;      /* $2B8  P$STicks:  tick count while running in system state */
    uint32_t  _datbeg;      /* $2BC  P$DatBeg:  Julian date when process was forked */
    uint32_t  _timbeg;      /* $2C0  P$TimBeg:  seconds since midnight when forked */
    uint32_t  _fcalls;      /* $2C4  P$FCalls:  non-I/O system call count */
    uint32_t  _icalls;      /* $2C8  P$ICalls:  I/O system call count */
    uint32_t  _rbytes;      /* $2CC  P$RBytes:  bytes read (I$Read/I$ReadLn, no error) */
    uint32_t  _wbytes;      /* $2D0  P$WBytes:  bytes written (I$Write/I$WritLn, no error) */

    /* ── I/O wait queue linkage ─────────────────────────── $2D4–$2D7 ── */
    uint16_t  _ioqp;        /* $2D4  P$IOQP:    prev process ID in I/O queue; 0=not queued */
    uint16_t  _ioqn;        /* $2D6  P$IOQN:    next process ID in I/O queue */

    /* ── Deprecated memory fragment list ────────────────── $2D8–$2DF ── */
    os9addr_t _frags[2];    /* $2D8  P$Frags:   not used (historical, pre-colour memory) */

    /* ── Scheduling and MMU ─────────────────────────────── $2E0–$2E7 ── */
    uint32_t  _sched;       /* $2E0  P$Sched:   scheduling constant (DActAge + priority) */
    os9addr_t _spumem;      /* $2E4  P$SPUMem:  SSM MMU memory map address; 0 if no SSM */

    /* ── Debugger breakpoint table ──────────────────────── $2E8–$30B ── */
    uint32_t  _bkptcnt;     /* $2E8  P$BkPtCnt: number of breakpoints set */
    uint16_t  _bkpts[16];   /* $2EC  P$BkPts:   saved instruction words at each
                             *       breakpoint location (restored when process halts) */

    /* ── User accounting module data ────────────────────── $30C–$32B ── */
    uint32_t  _acct[8];     /* $30C  P$Acct:    8 longwords for user accounting module */

    /* ── Primary static storage ─────────────────────────── $32C–$337 ── */
    os9addr_t _data;        /* $32C  P$Data:    address of process's static storage */
    uint32_t  _datasz;      /* $330  P$DataSz:  size of static storage (including stack) */
    os9addr_t _fpusave;     /* $334  P$FPUSave: FPU context save area; 0 if no FPU */

    /* ── FPU exception support ──────────────────────────── $338–$36F ── */
    os9addr_t FPExcpt[7];   /* $338  P$FPExcpt: FPU exception handler addresses */
    os9addr_t FPExStk[7];   /* $354  P$FPExStk: FPU exception register save areas */

    /* ── Signal masking and queue ───────────────────────── $370–$38F ── */
    uint8_t   _siglvl;      /* $370  P$SigLvl:  signal mask nesting level */
    uint8_t   _sigflg;      /* $371  P$SigFlg:  signal mechanism flags */
    uint16_t  _sigxs;       /* $372  P$Sigxs:   free entries in signal queue */
    uint32_t  _sigmask;     /* $374  P$SigMask: per-signal-code filter bitmask */
    uint32_t  _sigcnt;      /* $378  P$SigCnt:  number of pending signals */
    os9addr_t _sigque;      /* $37C  P$SigQue:  address of oldest pending signal entry */
    uint32_t  _defsig[4];   /* $380  P$DefSig:  pre-allocated signal queue structure */

    /* ── Thread and fragment list heads ─────────────────── $390–$39F ── */
    os9addr_t _thread[2];   /* $390  P$Thread:  first/last alarm/thread structure ptrs */
    os9addr_t _frag;        /* $398  P$frag:    first memory fragment structure ptr */
    os9addr_t _fragg;       /* $39C  P$frag+4:  last memory fragment structure ptr */

    /* ── Module owner ───────────────────────────────────── $3A0–$3AB ── */
    uint32_t  _mown;        /* $3A0  P$MOwn:    original owner of primary module */
    uint8_t   _res1[8];     /* $3A4  (reserved — pads to system stack at $3AC) */

    /* ── System state stack ─────────────────────────────── $3AC–$7FF ── */
    uint8_t   _sysstack[0x800 - 0x3AC]; /* $3AC: 1108 bytes — supervisor stack
                                          * used during system calls for this process */

} procid;

/* ── Compile-time layout verification ──────────────────────────────────── */
/* Any mismatch here means the struct above is wrong, not the assertion.    */

#define PROCID_CHECK(field, expected_offset) \
    typedef char procid_offset_check_##field \
        [ (offsetof(procid, field) == (expected_offset)) ? 1 : -1 ]

PROCID_CHECK(_id,      0x000);
PROCID_CHECK(_pid,     0x002);
PROCID_CHECK(_sid,     0x004);
PROCID_CHECK(_cid,     0x006);
PROCID_CHECK(_sp,      0x008);
PROCID_CHECK(_usp,     0x00C);
PROCID_CHECK(_pagcnt,  0x010);
PROCID_CHECK(_group,   0x014);
PROCID_CHECK(_user,    0x016);
PROCID_CHECK(_prior,   0x018);
PROCID_CHECK(_age,     0x01A);
PROCID_CHECK(_state,   0x01C);
PROCID_CHECK(_task,    0x01E);
PROCID_CHECK(_queueid, 0x020);
PROCID_CHECK(_scall,   0x021);
PROCID_CHECK(_baked,   0x022);
PROCID_CHECK(_deadlk,  0x024);
PROCID_CHECK(_signal,  0x026);
PROCID_CHECK(_sigvec,  0x028);
PROCID_CHECK(_sigdat,  0x02C);
PROCID_CHECK(_queuen,  0x030);
PROCID_CHECK(_queuep,  0x034);
PROCID_CHECK(_pmodul,  0x038);
PROCID_CHECK(except,   0x03C);
PROCID_CHECK(_exstk,   0x064);
PROCID_CHECK(_traps,   0x08C);
PROCID_CHECK(_trpmem,  0x0C8);
PROCID_CHECK(_trpsiz,  0x104);
PROCID_CHECK(_excpsp,  0x140);
PROCID_CHECK(_excppc,  0x144);
PROCID_CHECK(_dio,     0x148);
PROCID_CHECK(_path,    0x168);
PROCID_CHECK(_memimg,  0x1A8);
PROCID_CHECK(_blksiz,  0x228);
PROCID_CHECK(_dbgreg,  0x2A8);
PROCID_CHECK(_dbgpar,  0x2AC);
PROCID_CHECK(_dbgins,  0x2B0);
PROCID_CHECK(_uticks,  0x2B4);
PROCID_CHECK(_sticks,  0x2B8);
PROCID_CHECK(_datbeg,  0x2BC);
PROCID_CHECK(_timbeg,  0x2C0);
PROCID_CHECK(_fcalls,  0x2C4);
PROCID_CHECK(_icalls,  0x2C8);
PROCID_CHECK(_rbytes,  0x2CC);
PROCID_CHECK(_wbytes,  0x2D0);
PROCID_CHECK(_ioqp,    0x2D4);
PROCID_CHECK(_ioqn,    0x2D6);
PROCID_CHECK(_frags,   0x2D8);
PROCID_CHECK(_sched,   0x2E0);
PROCID_CHECK(_spumem,  0x2E4);
PROCID_CHECK(_bkptcnt, 0x2E8);
PROCID_CHECK(_bkpts,   0x2EC);
PROCID_CHECK(_acct,    0x30C);
PROCID_CHECK(_data,    0x32C);
PROCID_CHECK(_datasz,  0x330);
PROCID_CHECK(_fpusave, 0x334);
PROCID_CHECK(FPExcpt,  0x338);
PROCID_CHECK(FPExStk,  0x354);
PROCID_CHECK(_siglvl,  0x370);
PROCID_CHECK(_sigflg,  0x371);
PROCID_CHECK(_sigxs,   0x372);
PROCID_CHECK(_sigmask, 0x374);
PROCID_CHECK(_sigcnt,  0x378);
PROCID_CHECK(_sigque,  0x37C);
PROCID_CHECK(_defsig,  0x380);
PROCID_CHECK(_thread,  0x390);
PROCID_CHECK(_frag,    0x398);
PROCID_CHECK(_fragg,   0x39C);
PROCID_CHECK(_mown,    0x3A0);
PROCID_CHECK(_res1,    0x3A4);
PROCID_CHECK(_sysstack,0x3AC);

typedef char procid_size_check[(sizeof(procid) == 2048) ? 1 : -1];

#undef PROCID_CHECK

#endif /* PROCID_FROM_BOOK_H */
