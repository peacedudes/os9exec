 /*
  * UAE - The Un*x Amiga Emulator
  *
  * memory management
  *
  * Copyright 1995 Bernd Schmidt
  */

/* Enabling this adds one additional native memory reference per 68k memory
 * access, but saves one shift (on the x86). Enabling this is probably
 * better for the cache. My favourite benchmark (PP2) doesn't show a
 * difference, so I leave this enabled. */

/*
//#if 1 || defined SAVE_MEMORY
//#define SAVE_MEMORY_BANKS
//#endif

//typedef uae_u32 (*mem_get_func)(uaecptr) REGPARAM;
//typedef void (*mem_put_func)(uaecptr, uae_u32) REGPARAM;
//typedef uae_u8 *(*xlate_func)(uaecptr) REGPARAM;
//typedef int (*check_func)(uaecptr, uae_u32) REGPARAM;

//extern char *address_space, *good_address_map;
//extern uae_u8 *chipmemory;

//extern uae_u32 allocated_chipmem;
//extern uae_u32 allocated_fastmem;
//extern uae_u32 allocated_bogomem;
//extern uae_u32 allocated_gfxmem;
//extern uae_u32 allocated_z3fastmem;
//extern uae_u32 allocated_a3000mem;
*/

#undef DIRECT_MEMFUNCS_SUCCESSFUL
#include "maccess.h"

/*
//#ifndef CAN_MAP_MEMORY
//#undef USE_COMPILER
//#endif

//#if defined(USE_COMPILER) && !defined(USE_MAPPED_MEMORY)
//#define USE_MAPPED_MEMORY
//#endif

//#define kickmem_size 0x080000

//#define chipmem_start 0x00000000
//#define bogomem_start 0x00C00000
//#define a3000mem_start 0x07000000
//#define kickmem_start 0x00F80000

//extern int ersatzkickfile;
*/

#ifdef NEVER_DEFINED /* %%% LuZ excludes almost everything */

typedef struct {
    /* These ones should be self-explanatory... */
    mem_get_func lget, wget, bget;
    mem_put_func lput, wput, bput;
    /* Use xlateaddr to translate an Amiga address to a uae_u8 * that can
     * be used to address memory without calling the wget/wput functions.
     * This doesn't work for all memory banks, so this function may call
     * abort(). */
    xlate_func xlateaddr;
    /* To prevent calls to abort(), use check before calling xlateaddr.
     * It checks not only that the memory bank can do xlateaddr, but also
     * that the pointer points to an area of at least the specified size.
     * This is used for example to translate bitplane pointers in custom.c */
    check_func check;
} addrbank;

extern uae_u8 filesysory[65536];

extern addrbank chipmem_bank;
extern addrbank kickmem_bank;
extern addrbank custom_bank;
extern addrbank clock_bank;
extern addrbank cia_bank;
extern addrbank rtarea_bank;
extern addrbank expamem_bank;
extern addrbank fastmem_bank;
extern addrbank gfxmem_bank;

extern void rtarea_init (void);
extern void rtarea_setup (void);
extern void expamem_init (void);
extern void expamem_reset (void);

extern uae_u32 gfxmem_start;
extern uae_u8 *gfxmemory;
extern uae_u32 gfxmem_mask;
extern int address_space_24;

/* Default memory access functions */

extern int default_check(uaecptr addr, uae_u32 size) REGPARAM;
extern uae_u8 *default_xlate(uaecptr addr) REGPARAM;

#define bankindex(addr) (((uaecptr)(addr)) >> 16)

#ifdef SAVE_MEMORY_BANKS
extern addrbank *mem_banks[65536];
#define get_mem_bank(addr) (*mem_banks[bankindex(addr)])
#define put_mem_bank(addr, b) (mem_banks[bankindex(addr)] = (b))
#else
extern addrbank mem_banks[65536];
#define get_mem_bank(addr) (mem_banks[bankindex(addr)])
#define put_mem_bank(addr, b) (mem_banks[bankindex(addr)] = *(b))
#endif

extern void memory_init(void);
extern void map_banks(addrbank *bank, int first, int count);

#endif // %%% LuZ


/* base of the single 68k RAM arena (defined in memstuff.c). A 68k address is
   an offset into this block; the host pointer is emul_base + addr. This
   replaces the original identity mapping, which only worked when host
   pointers were 32 bits wide (see memstuff.c for the full rationale).
   Declared here, before the access macros that call get_real_address. */
extern unsigned char *emul_base;
extern unsigned char *emul_end;
extern uae_u32        emul_arena_limit;  /* arena size; a 68k offset is valid iff < this */

/* Cold out-of-arena handler for the CPU path: raises a 68k bus error (vector 2)
   by longjmp'ing out of the running instruction back to m68k_os9go.  Declared
   to return a pointer only so the hot path below type-checks; it never returns. */
extern uae_u8 *os9exec_oob_fault(uaecptr addr);

/* Guest-address -> host-pointer translation for the CPU (fetch and data access).
   The arena is the guest's entire RAM, so any offset >= its size is a wild
   pointer -- a genuine bus error, exactly as real 68k hardware would raise.
   Hot path is a single 32-bit bound compare (independent of the base+offset add,
   so the two issue in parallel) plus a predicted-not-taken branch; the fault
   path is out-of-line in os9exec_oob_fault(). */
static __inline__ uae_u8 *get_real_address(uaecptr addr)
{
    if (__builtin_expect((uae_u32)addr >= emul_arena_limit, 0))
        return os9exec_oob_fault(addr);
    return emul_base + (uae_u32)addr;
}

/* Non-faulting variant for the debugger/disassembler, which deliberately peeks
   at addresses computed from garbage (e.g. ShowEA dereferencing a memory-
   indirect EA past an RTS).  Out-of-arena clamps to the arena base instead of
   faulting -- a tool peek must never disturb guest execution. */
static __inline__ uae_u8 *get_real_address_safe(uaecptr addr)
{
    return ((uae_u32)addr < emul_arena_limit) ? emul_base + (uae_u32)addr : emul_base;
}

//#ifndef NO_INLINE_MEMORY_ACCESS
//
//#define longget(addr)   (call_mem_get_func(get_mem_bank(addr).lget, addr))
//#define wordget(addr)   (call_mem_get_func(get_mem_bank(addr).wget, addr))
//#define byteget(addr)   (call_mem_get_func(get_mem_bank(addr).bget, addr))
//#define longput(addr,l) (call_mem_put_func(get_mem_bank(addr).lput, addr, l))
//#define wordput(addr,w) (call_mem_put_func(get_mem_bank(addr).wput, addr, w))
//#define byteput(addr,b) (call_mem_put_func(get_mem_bank(addr).bput, addr, b))
//
// %%% arm64: 68k addr is a 32-bit offset into the arena; must translate via
// get_real_address before casting to a host pointer (identity mapping fails
// on 64-bit hosts where the low 4 GB is unmapped __PAGEZERO).

#define longget(addr)   (do_get_mem_long((uae_u32 *)get_real_address(addr)))
#define wordget(addr)   (do_get_mem_word((uae_u16 *)get_real_address(addr)))
#define byteget(addr)   (do_get_mem_byte((uae_u8  *)get_real_address(addr)))
#define longput(addr,l) (do_put_mem_long((uae_u32 *)get_real_address(addr), l))
#define wordput(addr,w) (do_put_mem_word((uae_u16 *)get_real_address(addr), w))
#define byteput(addr,b) (do_put_mem_byte((uae_u8  *)get_real_address(addr), b))

/*
//%%% LuZ: This case does not seem to be a valid choice any more, as
//         alongget/awordget is not implemented anywhere!
//#else */

/*	extern uae_u32 alongget(uaecptr addr);
	extern uae_u32 awordget(uaecptr addr);
	extern uae_u32 longget(uaecptr addr);
	extern uae_u32 wordget(uaecptr addr);
	extern uae_u32 byteget(uaecptr addr);
	extern void longput(uaecptr addr, uae_u32 l);
	extern void wordput(uaecptr addr, uae_u32 w);
	extern void byteput(uaecptr addr, uae_u32 b);
#endif */


#ifndef MD_HAVE_MEM_1_FUNCS

#define longget_1 longget
#define wordget_1 wordget
#define byteget_1 byteget
#define longput_1 longput
#define wordput_1 wordput
#define byteput_1 byteput

#endif

static __inline__ uae_u32 get_long(uaecptr addr)
{
    return longget_1(addr);
}
static __inline__ uae_u32 get_word(uaecptr addr)
{
    return wordget_1(addr);
}
static __inline__ uae_u32 get_byte(uaecptr addr)
{
    return byteget_1(addr);
}
static __inline__ void put_long(uaecptr addr, uae_u32 l)
{
    longput_1(addr, l);
}
static __inline__ void put_word(uaecptr addr, uae_u32 w)
{
    wordput_1(addr, w);
}
static __inline__ void put_byte(uaecptr addr, uae_u32 b)
{
    byteput_1(addr, b);
}

static __inline__ int valid_address( uaecptr addr, uae_u32 size )
{
  /* #pragma unused is an MPW/CodeWarrior-ism; GCC just warns that it is
   * ignoring it (clang quietly accepts it). Gate on the COMPILER, not on the
   * OS -- "not linux" wrongly included mingw. */
  #ifndef __GNUC__
    #pragma unused(addr,size)
  #endif
 
 /* return get_mem_bank(addr).check(addr, size); */
    return 1; /* all addresses can be translated (as there is no translation! %%% */
}

