// 
//    OS9exec,   OS-9 emulator for Mac OS, Windows and Linux 
//    Copyright (C) 2002 Lukas Zeller / Beat Forster
//	  Available under http://www.synthesis.ch/os9exec
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
 *
 */

#ifndef _os9_ll_h
#define _os9_ll_h


/* this is required to allow the use of original */
/* "module.h" with its ulong definition on Linux */
#ifdef linux
  #undef  __USE_MISC
  typedef unsigned long int ulong;  /* native word; used for host pointers and OS-9 register values */
  typedef unsigned int uint;

  #ifdef __cplusplus
    typedef int OSErr;
  #endif
  
  #define USE_UAEMU
#endif

/* XCode does not know about macintosh */
#if !defined macintosh && defined __MACH__
  #define    macintosh
  #define    powerc
  #define    UNIX
  #define    USE_UAEMU
  
  // Support Intel Macs
  #ifndef __BIG_ENDIAN__
    #define __INTEL__
  #endif
#endif

#ifdef USE_UAEMU
  /* include file os9_uae.h */
  #include "sysconfig.h"
  #include "sysdeps.h"

  #ifdef _in_uae_c
    #define extern
  #endif
#else
  /* include file for empty interface */
   #include <time.h>
#endif

#ifdef linux
  #include <sys/types.h>   /* according to Martin Gregorie's proposal */
  #include <stdio.h>
  #include <stdarg.h>
  #include <time.h>
#endif


#define CARRY      0x0001
#define SUPERVISOR 0x2000


#if !defined(USE_UAEMU) && !defined(linux)
  /* struct alignment must be 68k for these structs! */
  #ifdef __clang__
    #pragma pack(push, 2)
  #else
    #pragma push
    #ifdef macintosh
      #ifndef powerc
      #pragma align=mac68k
      #endif
    #endif
  #endif
#endif


#ifdef __INTEL__
    /* access register parts */
    #define loword(reg) (*(((ushort*)&(reg))+0))
    #define hiword(reg) (*(((ushort*)&(reg))+1))
    #define lobyte(reg) (*((  (byte*)&(reg))+0))

    /* return word/byte values */
    #ifdef PARTIALRETURNREGS
        #define retword(reg) (*(((ushort*)&(reg))+0))
        #define retbyte(reg)  *((  (byte*)&(reg))+0))
    #else
        #define retword(reg) (reg)
        #define retbyte(reg) (reg)
    #endif

    /* access bytes/words/longs in OS9-byteorder */
    /* (Intel needs byte reversal) */
    #define os9_byte(b)    (b)
    #define os9_word(w) ((((w)<<8) &0xFF00)    |(((w)>>8) &0x00FF))
    #define os9_long(l) ((((l)<<24)&0xFF000000)|(((l)>>24)&0x000000FF)|(((l)<<8)&0x00FF0000)|(((l)>>8)&0x0000FF00))
    /* no longer possible with backslash notation because of DOS file format of source !!! */
#else
    /* access register parts */
    #define loword(reg) (*(((ushort*)&(reg))+1))
    #define hiword(reg) (*(((ushort*)&(reg))+0))
    #define lobyte(reg) (*((  (byte*)&(reg))+3))
    /* return word/byte values */
    #ifdef PARTIALRETURNREGS
        #define retword(reg) (*(((ushort*)&(reg))+1))
        #define retbyte(reg)  *((  (byte*)&(reg))+3))
    #else
        #define retword(reg) (reg)
        #define retbyte(reg) (reg)
    #endif

    /* access bytes/words/longs in OS9-byteorder */
    /* (Mac and OS9 have the same byte order) */
    #define os9_byte(b) (b)
    #define os9_word(w) (w)
    #define os9_long(l) (l)
#endif

/* Safe typed accessors for OS-9 big-endian fields embedded in byte arrays.
   Always read/write exactly 2 or 4 bytes regardless of host word size. */
#define GET_OS9L(buf, off)       os9_long( *(uint32_t*)&(buf)[off] )
#define SET_OS9L(buf, off, val) (*(uint32_t*)&(buf)[off] = os9_long((uint32_t)(val)))
#define GET_OS9W(buf, off)       os9_word( *(uint16_t*)&(buf)[off] )
#define SET_OS9W(buf, off, val) (*(uint16_t*)&(buf)[off] = (uint16_t)os9_word((uint16_t)(val)))


/* ---- 68k address <-> host pointer conversion ----
   A 68k address is an offset into the single RAM arena (emul_base, defined in
   memstuff.c). TO68K turns a host pointer into the offset stored in 68k
   registers / emulated memory; FROM68K turns an offset back into a host
   pointer for dereferencing. NULL <-> 0 is preserved both ways, so the 68000's
   reserved low memory keeps "address 0 means no pointer" intact. Conversion
   happens ONLY at this register/memory boundary -- in-world values are always
   offsets and are never stored as real addresses. */
extern unsigned char *emul_base;

/* a 32-bit 68k word: an in-world register or address. Exactly the width of a
   real 68k register, so it also matches UAE's uae_u32 regstruct fields. */
typedef unsigned int ulong32;

/* os9addr_t — 68k virtual address (arena offset, not a host pointer).
   Defined early in os9main_incl_precomp.h so os9defs adapted headers can use it. */

#define TO68K(hostptr)  ((ulong32)( (hostptr)==NULL ? 0 : (unsigned char*)(hostptr) - emul_base ))
#define FROM68K(addr)   ( (addr)==0 ? NULL : (void*)( emul_base + (addr) ) )


/* floating point register */
#ifdef USE_UAEMU
  typedef  double        fp_typ;     
#else
  typedef  unsigned char fp_typ[12];
#endif


#ifdef USE_UAEMU
  /* buffer for interupt stack */
  #define TRAPFRAMEBUFLEN 128 /* should be fairly enough for all stack frames */
  extern ulong trapframebuf[TRAPFRAMEBUFLEN];
#endif


/* definitions for user traphandlers */
#define NUMTRAPHANDLERS 15
#define FIRSTTRAP       33 /* TRAP 1 */
#define FIRSTUSERTRAP    1


/* definition of the header of a OS-9 traphandler module */
typedef  struct {
            mod_exec progmod;
            long _mtrapinit;
            long _mtrapterm;
         } mod_trap;


/* traphandler description */
typedef  struct {
            os9addr_t trapentry;   /* 68k entry point of the trap handler */
            os9addr_t trapmem;     /* 68k arena offset of trap handler static storage */

            /* other entries are used by high-level code only */
            ulong trapmemsz;
            mod_trap *trapmodule;
            ushort mid;
            ushort dummy;
         } traphandler_typ;

#define TRAPENTRYSIZE sizeof(traphandler_typ)

/* os9regs.flags: */
#define FLAGS_FPU 0x0001   /* set if FPU is used */
#define FLAGS_IRQ 0x0002   /* allow IRQs for this process */
#define FLAGS_UM  0x0004   /* Mac context runs in user mode */


#ifdef USE_UAEMU
  #ifndef flagtype
    typedef char flagtype;
  #endif
#endif

/* register context:
   !!!!!!!! must be STRUCTURALLY EQUAL to regstruct in newcpu.h !!!!!!!!
   (through the last common field; the ttP/membase tail is os9exec-only and is
   never reached by the llm_os9_go memcpy. _Static_assert in os9_uae.c enforces
   this.) The `flags` field below is mirrored by a matching field in regstruct. */
#ifdef USE_UAEMU
  typedef struct {
#else
  typedef volatile struct {
#endif

    /* common in UAE and os9exec */
    ulong32 d[8]; /* data registers (32-bit, matches UAE regs[]) */
    ulong32 a[8]; /* address registers (32-bit, matches UAE regs[]) */

    #ifdef USE_UAEMU
      /* UAE only */
      uae_u32 usp,isp,msp;
    #endif
    
    /* common */
    unsigned short sr;   /* status register */
    unsigned short flags;/* divs OS9exec specific flags */

    #ifdef USE_UAEMU
      /* UAE only */
      flagtype t1;
      flagtype t0;
      flagtype s;
      flagtype m;
      flagtype x;
      flagtype stopped;
      int intmask;
    #endif

    /* common */
    ulong32 pc;             /* program counter (32-bit) */

    #ifdef USE_UAEMU
    /* UAE only */
    uae_u8 *pc_p;
    uae_u8 *pc_oldp;
    uae_u32 vbr,sfc,dfc;
    #endif
    
    /* common */                
    fp_typ fp[8];        /* FPU data registers */
    ulong32 fpcr;
    ulong32 fpsr;
    ulong32 fpiar;          /* FPU control registers (32-bit) */

    #ifdef USE_UAEMU
      /* UAE only */             
      uae_u32 prefetch;
    #endif

    /* --- link to traphandler entry of TRAP #1 */
    traphandler_typ* ttP;
    /* --- base address of process' static storage */
    void *membase;          /* used to fool StackSniffer via modified ApplLimit */
}   regs_type;



/* high level interface */
/* -------------------- */

#ifdef USE_UAEMU
extern void handle_os9exec_exception(int nr, uaecptr oldpc);
#endif


ulong llm_os9_go      ( regs_type *rp );
void  llm_os9_copyback( regs_type *rp );

#if defined macintosh && !defined __MACH__
  void llm_os9_debug( regs_type *rp, ConstStr255Param debuggerMsg );
#endif

Boolean llm_fpu_present     (void);
Boolean llm_vm_enabled      (void);
Boolean llm_has_cache       (void);
Boolean llm_runs_in_usermode(void);

void  Flush68kCodeRange(void *address, uint32_t size);
void  LockMemRange     (void *address, uint32_t size);
void  UnlockMemRange   (void *address, uint32_t size);

OSErr lowlevel_prepare (void);
void  lowlevel_release (void);


#ifdef USE_UAEMU
#undef extern
#endif

#if !defined(USE_UAEMU) && !defined(linux)
  #ifdef __clang__
    #pragma pack(pop)
  #else
    #pragma pop
  #endif
#endif


#endif

/* eof */
