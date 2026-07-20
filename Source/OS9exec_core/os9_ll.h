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

/* memcpy(), for the alignment-safe GET_OS9L/SET_OS9L/GET_OS9W/SET_OS9W
 * accessors below. os9exec_incl.h already pulls string.h in ahead of this
 * header, but state it here too so a direct include or a reordering can't
 * silently break them -- same reasoning as the endianness note further down. */
#include <string.h>

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

  /* __INTEL__ here means "little-endian host", NOT x86 -- the os9_word/os9_long
   * byte-swap macros below are gated on it and are needed on arm64 too. Define it
   * for EVERY little-endian host, and add __ARM64__ only as an extra platform ID.
   * This mirrors the canonical block in os9main_incl_precomp.h. The earlier
   * if/else here left __INTEL__ UNDEFINED on Apple Silicon, which would have
   * selected the no-swap (68k-native BE) path on a little-endian host -- wrong.
   * Dead today (os9exec_incl.h includes os9main_incl_precomp.h, which defines
   * `macintosh`, before this header, so this whole block is skipped), but kept
   * correct so a direct include or a reordering can't silently break endianness. */
  #ifndef __BIG_ENDIAN__
    #if defined __arm64__ || defined __aarch64__
      #define __ARM64__
    #endif
    #define __INTEL__
  #endif
#endif

/* mingw-w64 also runs the UAE 68k core -- and, like the linux/macOS
 * branches above, must NOT fall into the "#if !defined(USE_UAEMU)"
 * pragma pack(push, 2) below, which is a fallback for a hypothetical
 * non-UAEMU host and would silently repack every struct this codebase
 * relies on having natural alignment. */
#ifdef MINGW
  #define USE_UAEMU
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


/* Select on ACTUAL host byte order, not on __INTEL__ alone.
 *
 * Everything in this block -- the os9_word/os9_long swaps AND the
 * loword/hiword register-half offsets -- is endianness-dependent, so the whole
 * block switches together. __INTEL__ cannot be the discriminator by itself
 * because it is overloaded as a platform/type flag (see the OS9_HOST_BIG_ENDIAN
 * comment in os9main_incl_precomp.h): every Linux defines it regardless of
 * architecture. On Linux/s390x that selected the swapping macros on a host that
 * is already big-endian, reversing every OS-9 field so module headers failed
 * validation and even loading `shell` returned E_FNA. Confirmed against a
 * little-endian control build of the identical source, where the same disk and
 * commands worked. */
#if defined __INTEL__ && !defined OS9_HOST_BIG_ENDIAN
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
    /* (Intel needs byte reversal)
     *
     * CAUTION -- these mention their argument TWICE (os9_word) and FOUR TIMES
     * (os9_long). Never pass an expression with a side effect, and never pass a
     * call: `os9_long(DirSize(&sp))` really did run DirSize four times, and
     * `TO68K(++p)` (same hazard, since fixed by making TO68K a function) advanced
     * p twice and broke every one-character path name. Assign to a temp first.
     * They cannot be made functions like TO68K was: on the big-endian branch
     * below they are identity macros, so pinning a width here would silently
     * truncate any caller that passes something wider. */
    #define os9_byte(b)    (b)
    #define os9_word(w) ((((w)<<8) &0xFF00)    |(((w)>>8) &0x00FF))
    #define os9_long(l) ((((l)<<24)&0xFF000000)|(((l)>>24)&0x000000FF)|(((l)<<8)&0x00FF0000)|(((l)>>8)&0x0000FF00))
    /* no longer possible with backslash notation because of DOS file format of source !!! */
#else
    /* Access register parts on a BIG-ENDIAN host.
     *
     * Offsets are computed from sizeof(reg) rather than hardcoded, because on
     * big-endian the low half of a value lives at the END of the object, so a
     * fixed "+1 word / +3 bytes" silently assumes the operand is exactly 32
     * bits. Most operands are the 68k registers (ulong32, 32-bit) where that
     * held, but two are host `ulong` -- 64-bit under LP64 -- and there the
     * hardcoded offsets read the wrong halves entirely:
     *   - os9exec_nt.c's `resL`, which is where the trap vector and syscall
     *     function number are split out of llm_os9_go()'s result. Reading bits
     *     63..48 instead of 31..16 gives every system call a garbage vector.
     *   - modstuff.c's `b` in Update_MDir (module directory linkcount).
     * The little-endian branch above is immune by construction: there the low
     * half is at offset 0 whatever the width, which is why this never showed up
     * on any previously supported host.
     *
     * sizeof-relative indexing is correct for BOTH widths: for a 32-bit operand
     * it reproduces the original +1/+0/+3, and for a 64-bit one holding a
     * 32-bit value it lands on the same significant bits. Kept as lvalues --
     * three call sites assign through these macros. */
    #define loword(reg) (*(((ushort*)&(reg)) + (sizeof(reg)/sizeof(ushort) - 1)))
    #define hiword(reg) (*(((ushort*)&(reg)) + (sizeof(reg)/sizeof(ushort) - 2)))
    #define lobyte(reg) (*(((byte*)  &(reg)) + (sizeof(reg) - 1)))
    /* return word/byte values */
    #ifdef PARTIALRETURNREGS
        #define retword(reg) (*(((ushort*)&(reg)) + (sizeof(reg)/sizeof(ushort) - 1)))
        #define retbyte(reg) (*(((byte*)  &(reg)) + (sizeof(reg) - 1)))
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
   Always read/write exactly 2 or 4 bytes regardless of host word size.

   Go through memcpy() rather than casting the byte pointer to a uint32_t or
   uint16_t pointer and dereferencing it. OS-9/68k on-disk structures -- module headers, RBF file
   descriptors, directory entries -- are TIGHTLY PACKED big-endian, so <off> is
   routinely odd or merely 2-aligned; the old cast therefore formed a misaligned
   pointer and dereferenced it, which is undefined behaviour no matter how
   forgiving the hardware is. It survived on x86_64 (unaligned loads are a
   hardware feature there) and happens to survive on AArch64 too, since Windows
   leaves SCTLR_EL1.A clear so ordinary LDR/STR also tolerate misalignment -- but
   surviving is not the same as being correct: the compiler is entitled to ASSUME
   the pointer is aligned and emit something that isn't (e.g. widening a run of
   these into a vector or paired load), which is precisely the class of breakage
   that only shows up under a new optimiser or target.

   memcpy() with a constant size is not a function call at -O2 on either target;
   both compile to the same single unaligned load/store the cast produced, so
   this is free. Surfaced by clang's -Wcast-align on the native ARM64 build. */
static inline uint32_t os9_get_l( const void* p )
{ uint32_t v; memcpy( &v,p, sizeof(v) ); return (uint32_t)os9_long( v ); }

static inline void os9_set_l( void* p, uint32_t val )
{ uint32_t v= (uint32_t)os9_long( val ); memcpy( p,&v, sizeof(v) ); }

static inline uint16_t os9_get_w( const void* p )
{ uint16_t v; memcpy( &v,p, sizeof(v) ); return (uint16_t)os9_word( v ); }

static inline void os9_set_w( void* p, uint16_t val )
{ uint16_t v= (uint16_t)os9_word( val ); memcpy( p,&v, sizeof(v) ); }

#define GET_OS9L(buf, off)       os9_get_l( &(buf)[off] )
#define SET_OS9L(buf, off, val)  os9_set_l( &(buf)[off], (uint32_t)(val) )
#define GET_OS9W(buf, off)       os9_get_w( &(buf)[off] )
#define SET_OS9W(buf, off, val)  os9_set_w( &(buf)[off], (uint16_t)(val) )


/* ---- 68k address <-> host pointer conversion ----
   A 68k address is an offset into the single RAM arena (emul_base, defined in
   memstuff.c). TO68K turns a host pointer into the offset stored in 68k
   registers / emulated memory; FROM68K turns an offset back into a host
   pointer for dereferencing. NULL <-> 0 is preserved both ways, so the 68000's
   reserved low memory keeps "address 0 means no pointer" intact. Conversion
   happens ONLY at this register/memory boundary -- in-world values are always
   offsets and are never stored as real addresses. */
extern unsigned char *emul_base;
extern unsigned char *emul_end;   /* one past the end of the arena (memstuff.c) */
extern unsigned int   emul_arena_limit; /* arena size as a 32-bit offset bound (memstuff.c) */

/* a 32-bit 68k word: an in-world register or address. Exactly the width of a
   real 68k register, so it also matches UAE's uae_u32 regstruct fields. */
typedef unsigned int ulong32;

/* os9addr_t — 68k virtual address (arena offset, not a host pointer).
   Defined early in os9main_incl_precomp.h so os9defs adapted headers can use it. */

/* Convert between a host pointer and a 68k arena offset.
 *
 * These are FUNCTIONS, not macros, and must stay that way. As macros each
 * mentioned its argument twice (once in the null test, once in the arithmetic),
 * so any argument with a side effect was applied TWICE. That was not
 * hypothetical: F$PrsNam's `rp->a[0]=TO68K(++p)` advanced p by two, so parsing
 * "/dd/a" skipped the '/' AND the first name character. Every path whose final
 * component was one character then parsed as the empty name and came back
 * E$BNam, which the OS-9 shell reports as "^syntax error" -- so `echo x >/dd/a`
 * failed while `>/dd/ab` silently "worked" by parsing only its last character.
 * A function evaluates its argument exactly once, by construction. */
static inline ulong32 TO68K( const void* hostptr )
{
    return (ulong32)( hostptr==NULL ? 0 : (const unsigned char*)hostptr - emul_base );
}

static inline void* FROM68K( ulong32 addr )
{
    return addr==0 ? NULL : (void*)( emul_base + addr );
}

/* Validate a resolved guest pointer (a FROM68K result) as a real host pointer
   into the 68k arena.  Because the arena is one contiguous block, a range test
   catches an out-of-range address (offset >= arena size -> at or past
   emul_end).  IN_ARENA checks a single pointer; RANGE_IN_ARENA checks that
   a whole [p, p+len) span fits, with an overflow guard on p+len.  A syscall that
   is handed a bad pointer should reject it with E_BPADDR rather than dereference
   it and crash the host -- there is no MMU here to fault the guest instead.

   The NULL test is explicit and must stay that way.  Guest address 0 resolves to
   NULL (see FROM68K), and these macros used to lean on the range test alone to
   reject it, reasoning that NULL sorts below emul_base.  That is true of every
   real allocator, but it is not something the language guarantees: relationally
   comparing a null pointer against an unrelated object is undefined, so a
   compiler is entitled to assume it cannot happen and fold the guard away.  It
   also blinded static analysis -- GCC's -fanalyzer reported eight NULL
   dereferences across fcalls.c/icalls.c, every one of them a call site that DID
   guard correctly but whose guard the analyzer could not see through. */
#define IN_ARENA(p) \
    ( (const unsigned char*)(p) != NULL && \
      (const unsigned char*)(p) >= emul_base && (const unsigned char*)(p) < emul_end )
#define RANGE_IN_ARENA(p,len) \
    ( (const unsigned char*)(p) != NULL && \
      (const unsigned char*)(p) >= emul_base && \
      (const unsigned char*)(p) + (len) <= emul_end && \
      (const unsigned char*)(p) + (len) >= (const unsigned char*)(p) )


/* floating point register */
#ifdef USE_UAEMU
  typedef  double        fp_typ;     
#else
  typedef  unsigned char fp_typ[12];
#endif


#ifdef USE_UAEMU
  /* scratch supervisor stack for UAE exception frames; allocated inside the
     68k arena so ISP holds a valid 32-bit arena offset, not a host pointer */
  #define TRAPFRAMEBUFLEN 128
  extern os9addr_t trapstack_isp; /* 68k address of top of supervisor scratch stack */
#endif


/* definitions for user traphandlers */
#define NUMTRAPHANDLERS 15
#define FIRSTTRAP       33 /* TRAP 1 */
#define FIRSTUSERTRAP    1


/* definition of the header of a OS-9 traphandler module.
 * This struct is overlaid directly on a module header in the 68k arena, so
 * every field must be exactly the 32 bits the header actually holds -- hence
 * os9addr_t, as in every other module-header struct (see module_from_book.h).
 * `long` was 8 bytes wide on LP64, which pushed _mtrapterm to $050 instead of
 * $04C and made os9_long() shift a signed 64-bit value (undefined behaviour;
 * it only produced the right answer on little-endian hosts, by accident of the
 * macro's 0xFF000000 masks). The checks below pin the layout. */
typedef  struct {
            mod_exec  progmod;    /* $000  program module header (72 bytes) */
            os9addr_t _mtrapinit; /* $048  offset to trap handler init routine */
            os9addr_t _mtrapterm; /* $04C  offset to trap handler terminate routine */
         } mod_trap;              /* sizeof = $050 = 80 bytes */

#define MOD_TRAP_CHECK(field, expected_offset) \
    typedef char mod_trap_offset_check_##field \
        [ (offsetof(mod_trap, field) == (expected_offset)) ? 1 : -1 ]

MOD_TRAP_CHECK(progmod,    0x000);
MOD_TRAP_CHECK(_mtrapinit, 0x048);
MOD_TRAP_CHECK(_mtrapterm, 0x04C);

typedef char mod_trap_size_check[(sizeof(mod_trap) == 0x050) ? 1 : -1];


/* traphandler description */
typedef  struct {
            os9addr_t trapentry;   /* 68k entry point of the trap handler */
            os9addr_t trapmem;     /* 68k arena offset of trap handler static storage */

            /* other entries are used by high-level code only */
            uint32_t trapmemsz;
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
