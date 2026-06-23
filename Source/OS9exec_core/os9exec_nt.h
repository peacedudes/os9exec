// 
//    OS9exec,   OS-9 emulator for Mac OS, Windows and Linux 
//    Copyright (C) 2002 Lukas Zeller / Beat Forster
//    Available under http://www.synthesis.ch/os9exec
// 
//    This program is free software; you can redistribute it and/or 
//    modify it under the terms of the GNU General Public License as 
//    published by the Free Software Foundation; either version 2 of 
//    the License, or (at your option) any later version. 
// 
//    This program is distributed in the hope that it will be useful, 
//    but WITHOUT ANY WARRANTY; without even the implied warranty of 
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
//    See the GNU General Public License for more details. 
// 
//    You should have received a copy of the GNU General Public License 
//    along with this program; if not, write to the Free Software 
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 
//

/**********************************************/
/*             O S 9 E x e c / NT             */
/*  Cooperative-Multiprocess OS-9 emulation   */
/*         for Apple Macintosh and PC         */
/*                                            */
/* (c) 1993-2007 by Lukas Zeller, CH-Zuerich  */
/*                  Beat Forster, CH-Maur     */
/*                                            */
/* email: luz@synthesis.ch                    */
/*        bfo@synthesis.ch                    */
/**********************************************/

/*
 *  CVS:
 *    $Author$
 *    $Date$
 *    $Revision$
 *    $Source$
 *    $State$
 *    $Name$ (Tag)
 *    $Locker$ (who has reserved checkout)
 *  Log:
 *    $Log$
 *    Revision 1.81  2007/04/07 09:03:49  bfo
 *    pStart as new process state added
 *
 *    Revision 1.80  2007/03/31 12:15:07  bfo
 *    LINKED_HASH support with 16384 entries
 *
 *    Revision 1.79  2007/03/24 12:52:41  bfo
 *    - MAXDIR is now 65536
 *    - MAXDIRHIT = 60 and <hittable>, for "ihit" statistics
 *    - <readflag> for RW (fileaccess)
 *    - <svD_n> and <svD_dEnt> for fast directory access
 *    - <next> element for linked hash list
 *    - Improved TFS start conditions for directory searching
 *
 *    Revision 1.78  2007/03/10 13:52:30  bfo
 *    MPW adaptions
 *
 *    Revision 1.77  2007/03/10 12:19:26  bfo
 *    - number of dirs adaption
 *    - pending_typ / PENDING_MAX introduced (DeletePath problem)
 *    - readflag for r/w seek problem
 *    - "SearchDLLs": Don't break on error
 *
 *    Revision 1.76  2007/02/24 14:13:44  bfo
 *    - New <direntry> struct with <dirid> and fName>
 *    - pWaitRead recognized for internal commands
 *
 *    Revision 1.75  2007/02/22 23:03:15  bfo
 *    - <call_Intercept> added
 *    - Alphabetic sorting
 *    - Parameters reordered
 *
 *    Revision 1.74  2007/02/11 14:45:31  bfo
 *    <geCnt> added (MacOS9 reactiveness/idle load balance)
 *
 *    Revision 1.73  2007/02/04 20:02:58  bfo
 *    "plug_typ" with <pEnabled>,<pDisabled> and <fDLL>
 *
 *    Revision 1.72  2007/02/03 12:23:16  bfo
 *    <enabled> field added for "plug_typ"
 *
 *    Revision 1.71  2007/01/28 21:56:54  bfo
 *    - NATIVE_SUPPORT added
 *    - native interface definitions added
 *    - <with_dbgDLLs> added
 *    - 'init_plugins' and the full plugin support added
 *    - Callback routines for 'cclib' implemented here now
 *
 *    Revision 1.70  2007/01/07 13:41:00  bfo
 *    New <pmem> structure added
 *    Callback definitions for plugin DLLs added
 *    native/plugin additions
 *    prp_IProg/run_IProg DLL connection added
 *    call "Init_IProg"
 *
 *    Revision 1.69  2007/01/04 20:47:48  bfo
 *    <dllList> added
 *
 *    Revision 1.68  2007/01/02 11:11:54  bfo
 *    <includeList>/<excludeList> for PtoC added
 *
 *    Revision 1.67  2006/12/02 12:07:43  bfo
 *    global <lastsyscall> eliminated
 *
 *    Revision 1.66  2006/12/01 19:51:28  bfo
 *    <mnt_devCopy> added
 *
 *    Revision 1.65  2006/11/18 09:48:49  bfo
 *    <mnt_sctSize> and <mnt_cluSize> added
 *
 *    Revision 1.64  2006/11/12 13:20:21  bfo
 *    <catch_ctrlC> flag added
 *
 *    Revision 1.63  2006/11/04 23:30:55  bfo
 *    <procName> ( ptr ) renamed/changed to <intProcName> ( char[] )
 *
 *    Revision 1.62  2006/10/12 19:47:19  bfo
 *    Extended for ".DS_Store" support
 *
 *    Revision 1.61  2006/10/01 15:19:06  bfo
 *    <cp->isPtoC> introduced; <ptocMask> eliminated
 *
 *    Revision 1.60  2006/09/08 21:54:49  bfo
 *    <dbgPath> added
 *
 *    Revision 1.59  2006/08/26 23:51:11  bfo
 *    <arbitrate> is now with in "C" format section
 *
 *    Revision 1.58  2006/08/20 16:46:04  bfo
 *    line count for pagination added
 *
 *    Revision 1.57  2006/08/04 18:33:33  bfo
 *    Comment changes
 *
 *    Revision 1.56  2006/07/29 08:39:53  bfo
 *    <pBlocked> not really used now, removed again
 *
 *    Revision 1.55  2006/07/23 14:08:21  bfo
 *    element <pBlocked> and global var <ptocMask> added
 *
 *    Revision 1.54  2006/07/21 07:06:44  bfo
 *    <procName> the process' name (for internal utilities) added
 *
 *    Revision 1.53  2006/07/10 10:03:45  bfo
 *    os9exec_loop( unsigned short xErr, Boolean fromIntUtil ) added
 *
 *    Revision 1.52  2006/06/26 22:10:37  bfo
 *    Make some more things NET_SUPPORT dependent
 *    jmp_buf main_env added
 *
 *    Revision 1.51  2006/06/18 14:30:39  bfo
 *    DEFAULTPTYSZ is now exactly 1024 bytes ("clock2" hangs no longer)
 *
 *    Revision 1.50  2006/06/17 14:20:59  bfo
 *    DEFAULTPTYSZ is now exactly 512 bytes (seems to be ok for MGR)
 *
 *    Revision 1.49  2006/06/17 11:07:45  bfo
 *    Global var <withTitle> introduced
 *
 *    Revision 1.48  2006/06/16 15:57:57  bfo
 *    Make the PTY buffer larger again
 *
 *    Revision 1.47  2006/06/12 10:52:09  bfo
 *    MAX_PATH added
 *
 *    Revision 1.46  2006/06/10 19:14:31  bfo
 *    DEFAULTPTYSZ introduced
 *
 *    Revision 1.45  2006/06/07 16:07:03  bfo
 *    XCode / IntelMacs supported now
 *
 *    Revision 1.44  2006/06/02 19:00:25  bfo
 *    g_ipAddr ulong => char*
 *
 *    Revision 1.43  2006/06/01 21:02:26  bfo
 *    g_ipAddr added
 *
 *    Revision 1.42  2006/05/16 13:13:01  bfo
 *    pipeDirCnt / pipeTim added
 *
 *    Revision 1.41  2006/02/19 16:01:57  bfo
 *    ptoc vars and pthread support things added
 *
 *    Revision 1.40  2005/07/06 21:10:30  bfo
 *    defined UNIX
 *
 *    Revision 1.39  2005/07/02 14:21:30  bfo
 *    Adapted for Mach-O
 *
 *    Revision 1.38  2005/06/30 11:50:20  bfo
 *    Mach-O support
 *
 *    Revision 1.37  2005/05/13 17:25:59  bfo
 *    Use <imgMode>
 *
 *    Revision 1.36  2005/04/15 11:50:48  bfo
 *    Reduced size of RBF images is supported now, struct for SCSI
 *
 *    Revision 1.35  2005/01/22 16:15:40  bfo
 *    Renamed to ifdef MACOS9
 *
 *    Revision 1.34  2004/12/04 00:05:08  bfo
 *    MacOSX MACH adaptions / small transferBuffer for Linux
 *
 *    Revision 1.33  2004/11/27 12:02:21  bfo
 *    _XXX_ introduced
 *
 *    Revision 1.32  2004/11/20 11:44:08  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.31  2004/01/04 02:03:12  bfo
 *    #define MAXPROCESSES 129
 *    => this allows processes with id 1..128, id=0 is not used.
 *
 *    Revision 1.30  2003/08/01 11:11:40  bfo
 *    /L2 getstat support
 *
 *    Revision 1.29  2003/06/09 21:53:32  bfo
 *    Maximum number of modules changed from 512 -> 1024
 *
 *    Revision 1.28  2003/04/25 19:33:48  bfo
 *    MAXDIR/MAX_PATH now visible for Macintosh version
 *
 *    Revision 1.27  2003/04/20 23:02:55  bfo
 *    net->connected and wTimeout support
 *
 *    Revision 1.26  2003/04/12 21:56:03  bfo
 *    SS_Send, SS_Recv adaptions
 *
 *    Revision 1.25  2003/01/09 21:58:28  bfo
 *    Max number of signals increased
 *
 *    Revision 1.24  2003/01/03 14:57:02  bfo
 *    Store the signals during read the same way as in real OS-9, add pwr_brk flag
 *
 *    Revision 1.23  2003/01/02 12:16:41  bfo
 *    "MAXEVENTS" changed from 100 to 512
 *
 *    Revision 1.22  2002/11/06 20:12:12  bfo
 *    lastsignal->pd._signal/icptroutine->pd._sigvec (directly defined at pd struct)
 *
 *    Revision 1.21  2002/10/27 23:40:50  bfo
 *    memory and module structure definitions adapted
 *
 *    Revision 1.20  2002/10/15 18:16:46  bfo
 *    memtable included
 *
 *    Revision 1.19  2002/10/02 18:47:25  bfo
 *    <mdir_entry> / <mdirField> structures included
 *
 *    Revision 1.18  2002/09/22 20:48:23  bfo
 *    SS_206 (unused setstat call) included.
 *
 *    Revision 1.17  2002/09/11 17:28:30  bfo
 *    printerHandle definition disabled for Linux
 *
 *    Revision 1.16  2002/09/01 17:56:20  bfo
 *    some more variables of the real "procid" record used now
 *
 *    Revision 1.15  2002/08/13 21:55:46  bfo
 *    grp,usr and prior at the real prdsc now
 *
 *    Revision 1.14  2002/08/13 21:24:17  bfo
 *    Some more variables defined at the real procid struct now.
 *
 *    Revision 1.13  2002/08/09 22:39:21  bfo
 *    New procedure set_os9_state introduced and adapted everywhere
 *
 *    Revision 1.12  2002/08/08 21:54:30  bfo
 *    F$SetSys extended with D_PrcDBT support
 *
 *    Revision 1.11  2002/07/30 16:46:14  bfo
 *    E-Mail adress bfo@synthesis.ch       is updated everywhere
 *
 *    Revision 1.10  2002/07/24 22:33:28  bfo
 *    Timer synchronisation enhanced
 *
 *    Revision 1.9  2002/07/23 19:39:07  bfo
 *    introduce <syCorr> variable
 *
 *    Revision 1.8  2002/07/07 22:09:25  bfo
 *    take the RAM disk size from the descriptor (if available): define PD_SCT
 *
 *    Revision 1.7  2002/07/06 15:54:34  bfo
 *    mnt_ramSize included
 *
 *    Revision 1.6  2002/06/26 21:24:24  bfo
 *    Mac version can be compiled with "printer.c"
 *
 *    Revision 1.5  2002/06/25 20:44:33  luz
 *    Added /lp printer support under windows. Not tested or even compiled for Mac
 *
 */


/* OS9exec_nt.h */
#ifndef __os9exec_nt_h
#define __os9exec_nt_h

#include <stdio.h>

/* activate linked hash table */
//#if !defined MACOS9 && !defined windows32
#if defined MACOS9 && !defined powerc
  #undef  LINKED_HASH
#else
  #define LINKED_HASH 1
#endif

#ifdef THREAD_SUPPORT
  #include <types.h>
#endif

#include <setjmp.h>

/* XCode does not know about macintosh */
#if !defined macintosh && defined __MACH__
  #define    macintosh
  #define    powerc
#endif

#ifdef __GNUC__
  #define UNIX
#endif

             
/* constants */
/* ========= */

/* year 1904 offset for the date conversions */
#define OFFS_1904 126144000

/* day since 1.1.0000 + days between 1.1.0000 and 1.1.4713 B.C. */
/* question is still open whether year 0 exists ... */
#define DAYS_SINCE_0000 1721059L
#define SecsPerDay      (24*60*60)

/* 16 bit integer */
//#define MAXINT 32767

/* bytes */
#define KByte 1024  /* one     kByte */
#define BpB      8  /* bits per byte */

/* extra stack size: This is required because the user process'
 * stack must have room for MacOS interrupts
 */
#define EXTRAEMUSTACK 2048

/* number of ticks that must pass before SpinCursor is called */
#define DEFAULTSPININTERVAL 15



/* "system-wide" constants */
/* ----------------------- */
/* (that is, for all "processes" together) */

/* common string lenghts */
#define OS9NAMELEN       29 /* module,file,pipe name string */
#define OS9PATHLEN      255 /* path length */
#define OS9EVNAMELEN     12 /* event name length */
#define OS9MINSYSALLOC 2048 /* min system memory alloc block, a value that makes sense (bfo) */

#define RBF_MINALLOC      4 /* min sector allocation for RBF   */
#define DD__MINALLOC     32 /* min sector allocation for "/dd" */
#define DEFAULT_SCT      64 /* default number of sectors per track (RBF) */
#define MAXPRINTFLEN    500
#define ICMDPBUFF     20480 /* pipe buffer for internal commands */
#define MAX_PICTS         8 /* max number of picts to be displayed at /vmod display */
#define MAXTRIES_DEL    100 /* max tries to delete a file */

#ifndef windows32
  #define MAX_PATH     1024
#endif

// -------------------------------------------------------------------
/* number of "processes" OS9exec can handle. 
 * In the first versions of OS9exec "processes" are only chaining
 * each other, and no multitasking is possible.
 * The main purpose for this is to correctly handle chained makefiles
 * (which are heavily used in standard OS9 builds).
 * In the meantime multitasking is possible.
 */
#define MAXPROCESSES   129
/* this allows processes with id 1..128 / id=0 is not used */

/* maximum number of modules that can be linked (minus one for the main module itself */
#define MAXMODULES     1024

/* number of active events */
#define MAXEVENTS       512

/* max length of signal queue */
#define MAXSIGNALS      200

/* max length of signal queue */
#define MAXALARMS       512

/* number of open system paths */
#define MAXSYSPATHS     100

/* number of SCSI devices (total on all buses) */
#define MAXSCSI          20

/* TTY definitions: will be used for telnet and MGR */
#define MAXTTYDEV       100

/* max number of RBF devices */
#define MAXRBFDEV        64

/* path option section size */
#define OPTSECTSIZE     128

/* constants for the process(es) that run the OS9 code */
#define MYPRIORITY      128 /* priority for first process (defaults to IRQs enabled) */
#define IRQBLOCKPRIOR   250 /* minimal priority to have process execute with IRQs disabled */

/* number of dirs */
#if defined MACOS9 && !defined powerc
  #define MAXDIRS     10000
#else
  #ifdef LINKED_HASH
    #define MAXDIRS   16384
  #else
    #define MAXDIRS   65536
  #endif
#endif

#define MAXDIRHIT        60

/* number of include/exclude list elements */
#if defined NATIVE_SUPPORT || defined PTOC_SUPPORT
  #define MAXLIST       500
#endif

/* number of statistics entries */
#define MAX_OS9PROGS   1000

/* number of mem alloc entries */
#define MAX_MEMALLOC  10000

/* number of memory blocks than can be allocated per "process"
 * through F$SRqMem. For standard OS-9, this is limited to 32, but OS9 joins 
 * adjacent memory blocks if possible, while OS9exec does not. 
 * Therefore, the MAXMEMBLOCKS constant must be much larger than
 * 32 to avoid that F$SRqMem fails with E_NORAM after 32 calls.
 */
#define MAXMEMBLOCKS   512 

// total number of mem alloc entries
#define MAX_MEMALLOC 10000

// definition of an allocated memory block
typedef struct {
  void* base;
  ulong size;
} memblock_typ;

// per process memory blocks
typedef struct {
  memblock_typ m[ MAXMEMBLOCKS ]; // the process' allocated memory blocks
} pmem_typ;

#if defined MACOS9 && defined powerc
  #define PENDING_MAX 100
#endif
// 


/* "per-process" constants */
/* ----------------------- */
/* (that is, once for each "process") */


/* maximum number of open paths per "process" */
#define MAXUSRPATHS   32

/* number of exceptions for which a handler can be installed */
#define NUMEXCEPTIONS  7

/* number of the first exception (bus error) */
#define FIRSTEXCEPTION 2

/* special hi-byte flag for "silent" errors */
#define E_OKFLAG    0x4200

/* aging for pWaitRead state processes */
#define NewAge      30
#define Pwr_Signal  32      /* singals below go thru directly */

/* private error code */
#define E_PLINK     0x1234  /* returned by xOpen when *mode is the new syspath number */
#define SIG_SCRATCH 0x20    /* sigdat scratch area */

/* control characters */
#define NUL       0x00
#define CtrlA     0x01
#define CtrlC     0x03
#define CtrlE     0x05
#define LF        0x0A
#define CR        0x0D
#define XOn       0x11
#define XOff      0x13
#define keyRETURN '\r'
 
 
/* path delimiters */
/* for OS-9 */
#define   PSEP          '/'
#define   PSEP_STR      "/"

/* for the host system */
#if defined MACOS9
  #define PATHDELIM     ':'
  #define PATHDELIM_STR ":"
#elif defined windows32
  #define PATHDELIM     '\\'
  #define PATHDELIM_STR "\\"
#elif defined UNIX
  #define PATHDELIM     '/'
  #define PATHDELIM_STR "/"
#endif


// omitted parameters (does not really work with gcc)
#ifdef __GNUC__
  #define _modeP_   ushort*      modeP
  #define _rp_      regs_type*   rp
  #define _pid_     ushort       pid
  #define _spP_     syspath_typ* spP
  #define _rbf_     rbfdev_typ*  rbf
  #define _fmgr_    fmgr_typ*    f
  #define _maxlenP_ ulong*       maxlenP
  #define _buffer_  char*        buffer
  #define _bufferV_ void*        buffer
  #define _size_    ulong        size
  #define _txt_     char*        txt
  #define _txt2_    char*        txt2
  #define _argc_    int          argc
  #define _argv_    char**       argv
  #define _memPP_   void**       memoryPP
  #define _a1_      ulong*       a1
  #define _d0_      ulong*       d0
  #define _d2_      ulong*       d2
  #define _d3_      ulong*       d3
#else
  #define _modeP_   ushort*
  #define _rp_      regs_type*
  #define _pid_     ushort
  #define _spP_     syspath_typ*
  #define _rbf_     rbfdev_typ*
  #define _fmgr_    fmgr_typ*
  #define _maxlenP_ ulong*
  #define _buffer_  char*
  #define _bufferV_ void*
  #define _size_    ulong
  #define _txt_     char*
  #define _txt2_    char*
  #define _argc_    int
  #define _argv_    char**
  #define _memPP_   void**
  #define _a1_      ulong*
  #define _d0_      ulong*
  #define _d2_      ulong*
  #define _d3_      ulong*
#endif



/* global types */
/* ============ */

// OS9 error code */
// typedef ushort os9err;

/* an (internal) module directory entry */
typedef struct {
            void*   modulebase;            
            Boolean isBuiltIn; /* set, if module compiled into code (such as 'OS9exec' module) */
            short   linkcount;
         } module_typ;


/* a standard module directory entry — four 32-bit big-endian fields as the 68k expects */
typedef struct {
            os9addr_t m1;    /* big-endian 68k module address */
            os9addr_t m2;    /* big-endian 68k module address (group; mirrors m1 for now) */
            uint32_t  size;  /* big-endian module size */
            uint32_t  lnk;   /* big-endian link count */
         } mdir_entry;


/* a error trap handler */
typedef struct {
            os9addr_t handleraddr;   /* 68k virtual address of handler entry point */
            os9addr_t handlerstack;  /* 68k virtual address of handler stack pointer */
         } errortrap_typ;


/* OS9 simulated directory file constants */
#define IDMASK      0x001FFFFF /* one bit more because of problems ... (bfo) */
#define IDEXT       0xFFE00000
#define IDSIGN      0x00100000
#define IDSHIFT     21 /* bits to shift (bfo) */
#define VOLIDMASK   0x0007
#define VOLIDEXT    0xFFF8


/* #define VOLIDSIGN   0x0004 */
#define VOLIDMIN    -7
#define DIRENTRYSZ  32
#define DIRNAMSZ    28

/* a OS9 directory file entry */
typedef struct {
            char     name[DIRNAMSZ];
            uint32_t fdsect;       /* 4-byte big-endian FD sector number */
        } os9direntry_typ;
static_assert(sizeof(os9direntry_typ) == DIRENTRYSZ, "os9direntry_typ must be exactly 32 bytes");

/* pipe */
#define SAFETY           2
#define DEFAULTPIPESZ 4096+SAFETY
#define DEFAULTPTYSZ  1024
#define MINPIPESZ       90

#define MAX_SLEEP 0x7fffffff

/* attribute definitions */
typedef enum { poRead      =0x0001,
               poWrite     =0x0002,
               poUpdate    =0x0003,
               poExec      =0x0004,
               poDir       =0x0080,
               poCreateMask=0x8000 } poEnum;

/* sysPath 0 is reserved as nonexisting */
typedef enum { sysNonExisting=0, sysStdin=1, sysStdout=2, 
               sysStderr=3, sysStdnil=4, sysVMod=5  } syspathEnum;
              
typedef enum { usrStdin=0, usrStdout=1, usrStderr=2 } usrpathEnum;

typedef enum { fNone, fCons,fTTY,fNIL,fVMod,fSCF, fFile,fDir, 
                      fPipe,fPTY, fRBF, fNET, fPrinter, fARRSZ } ptype_typ;


#define  TOT_POS  0x00       /* Total number of sectors */
#define  TRK_POS  0x03       /* Number of sectors per track */
#define  MAP_POS  0x04       /* Number of bytes in allocation map */
#define  BIT_POS  0x06       /* Cluster size */
#define  DIR_POS  0x08       /* Position of root dir */
#define CRUZ_POS  0x60       /* Cruz position at RBF boot sector */
#define SECT_POS  0x68       /* Sector size */

#define Cruz_Str "Cruz"      /* the expected magic string at CRUZ_POS */
#define UNDEF_POS 0xFFFFFFFF /* invalid compare position for image raw access */

typedef struct {
            char   name[OS9NAMELEN];    /* device name */
            ushort ssize;               /* sector size */
            ushort sas;                 /* sector allocation size */
            byte   pdtyp;               /* PD_TYP according to descriptor */

            /* full SCSI Address */
            short  adapt;
            short  bus;
            short  id;                
            short  lun;
        } scsi_typ;


/* option section constants */
#define PthOffs    0x80           /* Offset to option section */
    
#define PD_TYP     0x83 - PthOffs /* RBF */
#define PD_SCT     0x8A - PthOffs
#define PD_SAS     0x8E - PthOffs
#define PD_SSize   0x94 - PthOffs
#define PD_CtrlID  0xA6 - PthOffs
#define PD_LUN     0x99 - PthOffs
#define PD_CtrlrID 0xA6 - PthOffs
#define PD_ATT     0xB5 - PthOffs
#define PD_FD      0xB6 - PthOffs
#define PD_DFD     0xBA - PthOffs
#define PD_DCP     0xBE - PthOffs
#define PD_DVT     0xC2 - PthOffs
#define PD_SctSiz  0xC8 - PthOffs
#define PD_NAME    0xE0 - PthOffs
    
#define PD_INT     0x90 - PthOffs /* SCF */
#define PD_QUT     0x91 - PthOffs
#define PD_XON     0x98 - PthOffs
#define PD_XOFF    0x99 - PthOffs


/* additional setstat/getstat codes */
#define SS_LBlink  0x080 + 32
#define SS_201     0x201
#define SS_204     0x204
#define SS_206     0x206
#define SS_Etc     0x3EE



/* The pipe channel structure */
/* NOTE: several system paths can use the same pipe channel */
/* NOTE: this is structure is also used for the tty/pty system which is in fact   */
/* a cross connected pipe (writing will be done into (each other's) sp_lock pipe) */
typedef struct {
            ulong     size;               /* size of pipe buffer */
            byte*     buf;                /* pointer to pipe buffer */
            byte*     prp;                /* pipe read pointer */
            byte*     pwp;                /* pipe write pointer */
            ulong     bread;              /* number of bytes read so far */
            ulong     bwritten;           /* number of bytes read so far */
            ushort    consumers;          /* number of waiting consumers for this pipe */
            ushort    sp_lock;            /* if <> 0, tty/pty to this system path nr */
            Boolean   do_lf;
            Boolean   broken;             /* broken pipe or tty/pty */
            ushort    pipeDirCnt;         /* pipe dir count */
            struct tm pipeTim;
        } pipechan_typ;



/* variant for pipes */
typedef struct {
            pipechan_typ*       pchP;   /* (named or unnamed) pipe, tty/pty pairs */
            pipechan_typ* i_svd_pchP;   /* saved pipe during internal commands */
            Boolean       locked_open;
        } pipe_typ;


/* sub-variant for disk files */
typedef struct {
            Boolean moddate_changed;
            time_t  moddate;
            Boolean readFlag;           /* keep track for r/w changes */
            
            #ifdef MACFILES
              short refnum;             /* the path's refNum if it is a MACFILES fFile */
            #endif
        } file_typ;


/* sub-variant for disk directories */
typedef struct {
            uint32_t pos;               /* the emulated directory "file"'s position for fDir */

            #ifdef win_unix
              uint32_t fdcur;
            #endif
        } dir_typ;              


/* variant for disk objects */
typedef struct {
            #ifdef MACOS9
              FSSpec spec;              /* the HFS object's FSSpec */
            #endif

            union {
                file_typ file;          /* disk file */
                 dir_typ dir;           /* emulated directory file */                   
            } u;
        } disk_typ;


/* variant for RBF objects */
typedef struct {
            uint32_t  devnr;        // current device number
            byte      att;          // file's atttributes
            uint32_t  currPos;      // current file position
            uint32_t  lastPos;      // current file last position (seek<end)
            Boolean   wMode;        // opened in write mode
            Boolean   flushFDCache; // flush FD cache
            ushort    diskID;       // disk ID for comparison reasons
            uint32_t  fd_nr;        // current FD logical sector number
            uint32_t  fddir;        // current FD logical sector of the dir
            uint32_t  deptr;        // dir entry ptr
        } rbf_typ;

/* variant for SCF objects */
typedef struct {
            int       scrline;         /* line count, for pagination    */
        } scf_typ;


/* Mount modes for RBF images */
#define  Img_Unchanged 0 // as is
#define  Img_Reduced   1 // reduced;   mount -i
#define  Img_FullSize  2 // full size; mount -I


/* the SCSI device variables */
typedef struct {
			      int	    ID;               /* SCSI ID, -1 if NO_SCSI   */
			      short     adapt;            /* SCSI Adaptor, -1 if none */
			      short     bus;              /* SCSI Bus, normally 0     */
			      short     LUN;              /* SCSI LUN, normally 0     */
        } scsi_dev;
        
        
#ifdef NET_SUPPORT
  #ifdef MACOS9
    typedef EndpointRef SOCKET;  /* make it a common type for all platforms */
  #else
    #if defined UNIX
      typedef ulong     SOCKET;  /* make it visible for linux as std type */
    #endif
  
    #define nil   0
    typedef char* Ptr;
    typedef int   OSStatus;
  
    struct  InetAddress
    {
      ushort  fAddressType;   // always AF_INET
      ushort  fPort;          // Port number 
      ulong   fHost;          // Host address in net byte order
      byte    fUnused[8];     // Traditional unused bytes
    };

    typedef struct InetAddress InetAddress;
  #endif


  #ifdef UNIX
    #define kTransferBufferSize 256
  #else
    #define kTransferBufferSize 4096
  #endif


  /* variant for network objects */
  typedef struct {
              SOCKET      ep;             /* end point reference */
              Ptr         transferBuffer; /* OpenTransport's network buffer */
              Ptr         b_local;        /* local buffer */
              ulong       bsize;          /* local buffer size */     
              Ptr         bpos;           /* local buffer access */     
              InetAddress ipAddress;      /* my  own    host's address */
              InetAddress ipRemote;       /* the remote host's address */
              ushort      fAddT;
                
              #ifdef MACOS9
                TCall     call;
              #endif
            
              #ifdef windows32
                WSAEVENT  hEventObj;
              #endif
                
              Boolean     bound;          /* true, if binding was successful  */
              Boolean     accepted;       /* true, if connection is accepted  */
              Boolean     connected;      /* true, if connection is connected */
              Boolean     listen;         /* net listener */
              Boolean     check;          /* check if ready */
              Boolean     closeIt;        /* ready to be closed */
          } net_typ;
#endif

                    
/* the system path descriptor itself */         
#define STD_SECTSIZE 256        // (old) standard sector size for OS9

#define MIN_TMP_SCT_SIZE 2048   // minimu size of temp sector buffer (to read sector 0)

typedef struct {
    /* common for all types */
    ptype_typ type;             /* the path's type */
    ushort    mode;             /* the path's mode */
    ushort    fileAtt;          /* file attributes */
    ushort    nr;               /* the own syspath number */        
    ushort    linkcount;        /* the link count */
    char      name[OS9NAMELEN]; /* file name */
    mod_exec* mh;				        /* according module, if available */
    byte      opt[OPTSECTSIZE]; /* path's option section */
    ushort    signal_to_send;   /* send signal on data ready */
    ushort    signal_pid;       /* signal has to be sent to this process */
    ulong     set_evId;         /* set  event  on data ready */
    int       lastwritten_pid;  /* connection for signal handler */
    Boolean   rawMode;          /* raw mode /xx@ */
    ulong     rawPos;           /* the (emulated) sector 0 position */
        
    byte*     fd_sct;           /* FD  sector buffer */
    byte*     rw_sct;           /* R/W sector buffer */
    ulong     rw_nr;            /* current R/W sector nr */
    ulong     mustW;            /* current sector to be written */
    Boolean   fullsearch;       /* recursive mode for "babushka" */

    // %%%luz: many of these should be in the union below...
    #if !defined MACFILES || defined MPW
      FILE*   stream;           /* the associated stream */
    #endif
        
    char      fullName[OS9PATHLEN]; /* direct access makes life easier */

    #if defined win_unix
      DIR*        dDsc;
      int         svD_n;
      dirent_typ* svD_dEnt;
    #endif
        
    int       term_id;          /* terminal/port id number, console|pipe|printer used together */

    /* no errors for mac/linux definition */
    #ifdef windows32        
      HANDLE    printerHandle;  /* handle for printer devices */
    #endif


    /* variants for different types */
    union {
      pipe_typ pipe;            /* Pipe          object */
      disk_typ disk;            /* Disk file/dir object */
       rbf_typ rbf;             /* RBF  file/dir object */
       scf_typ scf;             /* SCF           object */
         
      #ifdef NET_SUPPORT
        net_typ net;            /* Network       object */
      #endif
    } u;
} syspath_typ;



/* path operation function def */
// Originally variadic (...) to accept different 3rd/4th arg types across file managers.
// arm64: variadic call through fn pointer misaligns registers vs non-variadic callees.
// Call sites must cast to an explicit non-variadic type before dispatching.
typedef os9err(*pathopfunc_typ) ( ushort pid, syspath_typ*, ... );

                
/* system task function def */
typedef os9err(*systaskfunc_typ)( ushort pid, void *dataptr, regs_type* rp );


/* a output filter procedure */
typedef void (*filterfunc)( char *linebuf, FILE *stream, void *filtermem );


/* debug_out handling */
typedef void (*dbg_func)(void);



/* the file manager system */
typedef struct {
            pathopfunc_typ _SS_Size,
                           _SS_Opt,
                           _SS_DevNm,
                           _SS_Pos,
                           _SS_EOF,
                           _SS_Ready,
                           _SS_FD,
                           _SS_FDInf,
                           _SS_DSize,
                           
                           _SS_PCmd,   /* network specific function */

                           _SS_LBlink,                /* L2 support */
                           
                           _SS_Undef; /* any other getstat function */
        } gs_typ;


typedef struct {
            pathopfunc_typ _SS_Size,
                           _SS_Opt,
                           _SS_Attr,
                           _SS_FD,
                           _SS_Lock,
                           _SS_WTrk,

                           _SS_Bind,    /* network spefic functions */
                           _SS_Listen,
                           _SS_Connect,
                           _SS_Accept,
                           _SS_Recv,
                           _SS_Send,
                           _SS_GNam,
                           _SS_SOpt,
                           _SS_SendTo,
                           _SS_PCmd,
                           
                           _SS_LBlink,                /* L2 support */
                           
                           _SS_Undef; /* any other setstat function */
        } ss_typ;


typedef struct {
            pathopfunc_typ open,
                           close,
                           write,
                           writeln,
                           read,
                           readln,
                           seek,
                           
                           chd,
                           del,
                           makdir;
                           
            gs_typ         gs;
            ss_typ         ss;
        } fmgr_typ;


/* an "event" */
typedef struct {
            ulong   id;
            char    name[OS9EVNAMELEN];
            int     value;
            short   wInc;      
            short   sInc;
            ushort  e_linkcount;
                
            void*   next;
            void*   prev;
        } event_typ;            

/* a "process" */
typedef enum { 
            pUnused,
            pStart,
            pActive, 
            pDead, 
            pSleeping, 
            pWaiting, 
            pSysTask, 
            pWaitRead 
        } pstate_typ;


typedef struct {                /* saved structure during read */
            regs_type r;        /* the registers */
            ushort    vector;   /* vector */
            ushort    func;     /* and function */
        } save_type;



typedef struct {
            ptype_typ type;             /* current dir's device type */
            ushort    dev;              /* RBF: device */
            ulong     lsn;              /* RBF: current lsn */
            char      path[OS9PATHLEN]; /* current dir path */
                                
            #ifdef macintosh
              short volID;              /* current dir's volume id */
              long  dirID;              /* current dir's directory id */
            #endif
        } dir_type;



/* signal type */
typedef struct {
            int    cnt;
            ushort pid   [MAXSIGNALS];
            ushort signal[MAXSIGNALS];
        } sig_typ;



/* alarm type */
typedef struct {
			ushort  pid;
			ushort  signal;
			ulong   ticks;
			ulong   due;
			Boolean cyclic;
		} alarm_typ;
		


/* record structure for multi console access */
#define INBUFSIZE  256
typedef struct {
            Boolean      installed;
            ulong        consoleTermPtr;    /* used for the mac terminals */
            short        rIn, rOut;         /* used for the serial lines */
            char         inBuf[INBUFSIZE];
            int          inBufUsed;
            Boolean      holdScreen;
            ushort       pid;
            syspath_typ* spP;
        
            #ifdef MACOS9
              char       pict_title[OS9NAMELEN]; /* vmod window stores this info */
              PicHandle  pict_hdl   [MAX_PICTS];
              ushort     pict_tot;
            #endif
        } ttydev_typ;


/* the L2 led structure */
#define L2_Off        4
#define L2_InitRatio 10

typedef struct {
			ushort       hw_location;
			
			byte           col1;
			ushort       ratio1;
			byte           col2;
			ushort       ratio2;
			
			ushort        phase;
			ushort       lcount;
		} l2_typ;



//#if defined NATIVE_SUPPORT || defined PTOC_SUPPORT
  typedef int    ( *Next_NativeProg_Typ)( int* i, char* progName, char*   callMode );
  typedef int    (   *Is_NativeProg_Typ)(   const char* progName, void**   modBase );
  typedef os9err (*Start_NativeProg_Typ)(   const char* progName, void* nativeinfo );
  typedef os9err (  *Call_Intercept_Typ)( void* routine,    short code, ulong ptr  );
    
  // The plugin element
  typedef struct {
    char*   name;           // the plugin's name
    void*   fDLL;           // the plugin's reference
    long    pVersion;       // the plugin's version
    int     numNativeProgs; // number of internal progs
    
    Boolean pEnabled;       // the plugin is explicitely  enabled
    Boolean pDisabled;      // the plugin is explicitely disabled
    
     Next_NativeProg_Typ  next_NativeProg;
       Is_NativeProg_Typ    is_NativeProg;
    Start_NativeProg_Typ start_NativeProg;
      Call_Intercept_Typ  call_Intercept;
  } plug_typ;
//#endif
  
  

/* the process descriptor */
typedef struct {
                regs_type       os9regs;    /* the process' register stack */
                save_type       savread;    /* saved variables during read */

                procid pd;                  /* this is the 1:1 process descriptor image */
                
                /* general state */
                pstate_typ        state;    /* process' state */
                Boolean       isIntUtil;    /* acting as internal utility */
                Boolean        isNative;    /*    "   "  native program */
                Boolean        isPlugin;    /*    "   "  plugin program */
                plug_typ*      plugElem;    /* plugin type reference    */
                ushort              mid;    /* the process' primary module ID */
                
                char intProcName[OS9NAMELEN]; /* the process' name (for internal utilities) */

                os9err          exiterr;    /* process' exit OS9 error */
                systaskfunc_typ systask;    /* the system task function if state=pSysTask */
                void      *systaskdataP;    /* system task data pointer */
                ulong      systask_offs;    /* offset for rewrite call (from tty/pty) */
                ushort      lastsyscall;    /* last system call issued by this process */
                int            wTimeOut;    /* timeout counter for pWaitRead */

                /* statistics */
                ulong            fticks;    /* number of ticks at fcalls */
                ulong            iticks;    /* number of ticks at icalls */
                                  
                /* memory */
                os9addr_t memstart;         /* the process' static storage start addr (68k arena offset) */
                os9addr_t memtop;           /* the process' static storage top pointer (68k arena offset) */
             // memblock_typ    os9memblocks[MAXMEMBLOCKS]; /* the process' allocated memory blocks */
                byte sigdat[SIG_SCRATCH];
                
                /* exceptions */
                errortrap_typ     ErrorTraps[NUMEXCEPTIONS];    /* BusErr .. Privilege (Vector #2..#8) */
                traphandler_typ TrapHandlers[NUMTRAPHANDLERS];  /* TRAP 1..15 = vector #32..#47 */

                /* paths */
                ushort usrpaths[MAXUSRPATHS]; /* system path number of user paths */

                /* create init size */
                ushort    fileAtt;          /* here because I$Create is not able */
                uint32_t  cre_initsize;     /* to call with these params (from rp->d[2]) */

                dir_type d;                 /* current      directory */
                dir_type x;                 /* current exec directory */

                ushort vector;              /* current TRAP vector */
                ushort func;                /* current function code or exception vector */
                os9err oerr;                /* OS9 error */
    
                /* signal handling */
                regs_type  rteregs;         /* saved main thread registers during intercept processing, valid if lastsignal!=0 */
                pstate_typ rtestate;        /* saved process' state */
                ushort     rtevector;       /* saved vector */
                ushort     rtefunc;         /* and function */
                
                int     masklevel;
                Boolean pwr_brk;            /* pWaitRead break for signals <= 32 */
                os9addr_t icpta6;           /* 68k A6 value to pass to intercept routine */

                Boolean way_to_icpt;        /* is true on the way to icpt */
                ushort  icpt_pid,           /* keep pid and signal save */
                        icpt_signal;

                ulong   wakeUpTick;
                int		pW_age;				/* aging for pWaitRead */

                ttydev_typ* last_mco;
                /* Stdout filtering %%% needs some work... */
                filterfunc stdoutfilter;    /* the process' filter function handle, if any */
                void* filtermem;            /* the process' filter function's static storage */

                /* Stdin saved buffer */
                int        saved_cnt;
                pstate_typ saved_state;     /* saved process' state */
                
                os9addr_t my_args;          /* 68k arena offset of process argument area */
                int   tid;                  /* thread ID */
            } process_typ;
            


/* the statistics type */               
typedef struct {
            char    name[OS9NAMELEN]; /* name of the program */
            Boolean intern;           /* internal command */
            ulong   ticks;            /* number of ticks */
            ulong   num;              /* number of calls */
        } st_typ;


// ---- global vars ---------------------------------------------
// id of current process
extern  ushort currentpid; 

/* the "module directory" */
extern  module_typ  os9modules[MAXMODULES];
extern  mod_exec*   init_module;
extern  ulong       totalMem;
extern  mdir_entry *mdirField;

/* the system paths */
extern  syspath_typ syspaths [MAXSYSPATHS];
extern  uint32_t    syspth   [MAXSYSPATHS]; /* big-endian 32-bit path table entries */


/* the RBF and SCSI devices */
extern  scsi_typ    scsi[MAXSCSI];

/* definitions for TTY */
extern  ttydev_typ  ttydev[MAXTTYDEV];


/* the processes */
extern  process_typ procs[MAXPROCESSES];
extern  uint32_t    prDBT[MAXPROCESSES]; /* big-endian 32-bit process descriptor offsets */

/* the signal queue */
extern  sig_typ     sig_queue;

/* the alarms */
extern  alarm_typ   alarms     [MAXALARMS];
extern  alarm_typ*  alarm_queue[MAXALARMS];


/* the dir table */
typedef struct {
  char* ident;
  long  dirid;
  
  #ifdef MACOS9
  char* fName;
  #endif
  
  #ifdef LINKED_HASH
  void* next;
  #endif
} dirtable_entry;

extern dirtable_entry dirtable[MAXDIRS];
extern int            hittable[MAXDIRHIT];

#if defined MACOS9 && defined powerc && !defined MPW
  typedef struct {
    FSRef   newRef;
    Boolean toBeDeleted;
  } pending_typ;
  
  extern pending_typ dPending[ PENDING_MAX ];
#endif

#if defined NATIVE_SUPPORT || defined PTOC_SUPPORT
  /* the include/exclude list for internal commands */
  extern char*    includeList[ MAXLIST ];
  extern char*    excludeList[ MAXLIST ];
  extern plug_typ  pluginList[ MAXLIST ];
  
  extern callback_typ g_cb;
#endif

/* the OS-9 statistics table */
extern  st_typ      statistics[MAX_OS9PROGS];

/* I/O device table */
extern  byte        devs[0x0900];       


/* and the file managers routine tables */
extern  fmgr_typ    fmgr_none,
                    fmgr_cons,
                    fmgr_nil,
                    fmgr_scf,
                    fmgr_file,
                    fmgr_dir,
                    fmgr_pipe,
                    fmgr_pty,
                    fmgr_rbf,
                    fmgr_net,
                    fmgr_printer;

/* the events */
extern  event_typ   events  [MAXEVENTS];
extern  ulong  newEventId;
#define EvOffs 0x00010001

/* the currently executing process, MAXPROCESSES if none */
#if defined __cplusplus
  extern "C" {
#endif

//extern ushort currentpid; // id of current process
extern short arbitrate;  // set if arbitrate() should switch away from one running process to next

#if defined __cplusplus
  } // end extern "C"
#endif

extern ushort   interactivepid;        /* process that will get keyboard abort signals */
extern char     startPath[OS9PATHLEN]; /* start path */
extern char     strtUPath[OS9PATHLEN]; /* next higher than start path */
extern dir_type mdir;                  /* current module dir */

#ifdef MACOS9
  /* the MPW-level default directory */
  extern short    startVolID;	          // startup dir's volume    id
  extern long     startDirID;	          // startup dir's directory id
  extern char     callPath[OS9PATHLEN];
  
  extern short    applVolID;	          // app's   dir's volume    id
  extern long     applDirID;	          // app's   dir's directory id
  extern char     applName[OS9PATHLEN];
  extern int      geCnt;
 
#else
  /* the default module load directory OS9MDIR */
  extern    char   mdirPath[MAX_PATH];    /* current mdir path */
#endif


/* stdin and out for console */
#ifdef windows32
  extern  HANDLE  hStdin; /* the windows console definition */
#endif

#ifdef win_linux
  extern  ttydev_typ main_mco;
#endif

#ifdef MPW
  extern  int mpwsignal; /* the MPW signal flag */
#endif

/* flag which allows empty read functionality for multiple terminal support */
extern Boolean devIsReady;       /* it is true by default or if char has been read */

/* break the recursion loops variable */
extern Boolean in_recursion;

/* global settings */
extern int     dbgOut;
extern ushort  dbgPath;
extern int      without_pid;
extern int     justthis_pid;
extern Boolean quitFlag;
extern Boolean userOpt;
extern Boolean catch_ctrlC;

extern Boolean nativeActive;
extern Boolean pluginActive;
extern Boolean ptocThread;
extern Boolean fullArb;
extern int     withTitle;

extern Boolean logtiming;
extern Boolean logtiming_disp;
extern Boolean async_area;
extern Boolean async_pending;
extern short   defSCSIAdaptNo;
extern short   defSCSIBusNo;
extern l2_typ  l2;
extern ulong   my_inetaddr;

/* jump back environment for SEGV exceptions */
extern jmp_buf main_env;

/* tickCount at start of the program */
extern ulong   startTick;
extern ulong    lastTick;
extern int        syCorr;

#ifdef UNIX
  extern ulong sec0;
#endif

/* error traceback */
extern ushort  errpid;          /* PID of process that generated that error */
//extern ushort  lastsyscall;   /* last system call */
extern char*   lastpathparsed;  /* last pathstring parsed (NULL if none) */

/* the software's version and revision */
//extern ushort  appl_version;
//extern ushort  appl_revision;
extern ushort  exec_version;
extern ushort  exec_revision;
extern char*   hw_site;
extern char*   hw_name;
extern char*   sw_name;
extern char*   platform;

/* screen size */
extern ulong   screenW;
extern ulong   screenH;
extern char*   g_ipAddr;


/* external definitions */
/* ==================== */

/* fork mode */
extern int dummyfork; /* if set, forks are done as in os9exec 1.14 (only command printout to stdout) */

/* Fetch FTP name translation */
extern int fetchnames; /* if set, OS9-filenames beginning with '.' will be translated to begin with '/' on the mac */

/* output filter flag */
extern int disablefilters; /* if set, output filters will be disabled */

/* internal commands */
extern Boolean with_intcmds; // if set, int commands will be used
extern Boolean with_dbgDLLs; // if set, '*_dbg' DLLs will be preferred

/* Cursor spinning interval */
extern ulong spininterval;

/* mount device name and write protection */
extern  char*   mnt_name;
extern  int     mnt_ramSize;
extern  int     mnt_sctSize;
extern  int     mnt_cluSize;
extern  char*   mnt_devCopy;

#define NO_SCSI -1
extern  int     mnt_scsiID;
extern  short   mnt_scsiAdapt;
extern  short   mnt_scsiBus;
extern  short   mnt_scsiLUN;

extern  Boolean mnt_wProtect;
extern  Boolean mnt_imgMode;

/* additional memory for all processes */
extern ulong memplusall;

#ifdef THREAD_SUPPORT
extern  pthread_mutex_t sysCallMutex;
#endif


/* externally defined routines */
void    writeline    ( char* linebuf, FILE *stream );
char*   dgetenv( const char* name );
char*   egetenv( const char* name );
void    eSpinCursor  ( short incr );
void    eAdvanceCursor(void);

// console names
#define MainConsole "/term"
#define SerialLineA "/ts1"
#define SerialLineB "/ts2"

#define Main_ID       0
#define SerialA_ID   -1
#define SerialB_ID   -2
#define VModBase     50
#define TTY_Base    100

#define MAX_CONSOLE 100 /* half of them are VMod windows */

// printer names
#define MainPrinter "/lp"
#define LptPrinterFamilyName "/lpt"

#define MAX_PRINTER 5 /* Com0..com4 */


#ifdef win_unix
  #define AppDo ".AppleDouble"   /* specific netatalk file names */
  #define AppDt ".AppleDesktop"
  #define L_P   ":2e"            /* specific netatalk way for "." files */
  #define DsSto ".DS_Store"      /* specific desktop info store file */
#endif

/* OS9exec builtin module definitons */
#define OS9exec_name "OS9exec"   /* the name of the built-in OS9exec module */


/* routines */
/* ======== */

// Get HW info
void   get_hw();

/* Entry into OS9 emulation */
void   os9exec_globinit(void);
void   os9exec_loop( unsigned short xErr, Boolean fromIntUtil );

ushort os9exec                         ( int argc, char** argv, char** envp );
ushort os9exec_nt( const char* toolname, int argc, char** argv, char** envp, ulong memplus, ushort prior );


#endif
/* eof */

