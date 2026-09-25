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
 *    Revision 1.26  2007/01/07 14:01:04  bfo
 *    Fully rearranged
 *    No longer used as import file for PtoC system
 *    Parts moved to "c_access"
 *    New <pmem> structure introduced
 *
 *    Revision 1.25  2007/01/04 20:20:41  bfo
 *    Boolean return type added for 'IsZeroR' and 'IsZeroI'
 *
 *    Revision 1.24  2007/01/02 11:29:38  bfo
 *    Allow to release combined memory blocks (e.g. for "dir")
 *
 *    Revision 1.23  2006/08/06 22:42:43  bfo
 *    empty string causes <convErr>
 *
 *    Revision 1.22  2006/08/04 18:41:40  bfo
 *    MEM_SHIELD test environment added
 *
 *    Revision 1.21  2006/07/21 07:25:50  bfo
 *    UIntToStrN: Keep spaces for the remaining part of > 8
 *
 *    Revision 1.20  2006/07/14 11:43:30  bfo
 *    STRANGE/UNUSED BLOCK: Adapt it for easier debugging
 *
 *    Revision 1.19  2006/06/13 22:23:05  bfo
 *    StrToInt/StrToShort added
 *
 *    Revision 1.18  2006/03/12 19:30:24  bfo
 *    RealToStrN added
 *
 *    Revision 1.17  2006/02/19 16:33:21  bfo
 *    Some PtoC routines are implemented here now
 *
 *    Revision 1.16  2005/06/30 11:48:07  bfo
 *    Mach-O support
 *
 *    Revision 1.15  2004/11/20 11:44:07  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.14  2004/09/15 19:57:13  bfo
 *    Up to date
 *
 *    Revision 1.13  2003/05/25 10:35:20  bfo
 *    <totalMem> can't be any more smaller than <memsz>
 *
 *    Revision 1.12  2003/05/21 20:32:01  bfo
 *    Allocate 512k blocks / Additional parameter <mem_fulldisp> for "show_mem"
 *
 *    Revision 1.11  2003/05/17 10:41:39  bfo
 *    'show_mem' with <mem_unused> parameter /
 *    better deallocation strategy for the unused blocks
 *
 *    Revision 1.10  2002/10/28 22:19:21  bfo
 *    slightly different checkpoint:  STRANGE/UNUSED
 *
 *    Revision 1.9  2002/10/27 23:52:28  bfo
 *    REUSE_MEM handling introduced
 *
 *    Revision 1.8  2002/10/15 18:24:19  bfo
 *    memtable handling prepared
 *
 *    Revision 1.7  2002/09/01 20:15:07  bfo
 *    some debug messages reduced
 *
 *    Revision 1.6  2002/08/13 21:24:17  bfo
 *    Some more variables defined at the real procid struct now.
 *
 *    Revision 1.5  2002/08/09 22:39:20  bfo
 *    New procedure set_os9_state introduced and adapted everywhere
 *
 *
 */

#include "os9exec_incl.h"

/* Memory management for OS9 processes */
/* =================================== */


// the mem alloc table
memblock_typ memtable[ MAX_MEMALLOC ];
pmem_typ     pmem    [ MAXPROCESSES ];

#ifdef REUSE_MEM
  // the free info
  free_typ freeinfo;
#endif


/* ---- the 68k machine's RAM: one contiguous host arena ----
   A 68k address is simply an offset into this block; the host pointer is
   emul_base + addr (see get_real_address in the UAE memory layer). All OS-9
   memory is carved from here so every host pointer handed to the 68k world
   fits in a 32-bit offset on a 64-bit host. The low page is reserved and
   never handed out, so 68k address 0 can never alias a real block -- this
   mirrors the 68000 exception-vector region and keeps NULL == 0 valid.
   The reservation is demand-zero (untouched pages cost no real memory). */
#define EMUL_ARENA_DEFAULT (32u*1024u*1024u)  /* 32 MB default; covers real OS-9/68k machines */

ulong emul_arena_size = EMUL_ARENA_DEFAULT;  /* 68k arena size; can be overridden via -M option */

unsigned char* emul_base = NULL;             /* arena base (referenced by memory.h) */
static unsigned char* emul_next = NULL;      /* bump pointer for fresh allocations */
       unsigned char* emul_end  = NULL;      /* one past the end of the arena */
       ulong32        emul_arena_limit = 0;  /* arena size as a 32-bit offset limit;
                                                get_real_address()'s hot-path bound */

#ifdef USE_UAEMU
os9addr_t trapstack_isp = 0; /* 68k address of top of supervisor scratch stack */
#endif


static void* emul_alloc( ulong memsz )
/* bump-allocate a zeroed block from the 68k arena; NULL if exhausted */
{
    unsigned char* p= emul_next;
    if (p+memsz > emul_end) return NULL;     /* arena exhausted */
    emul_next= p+memsz;
    return p;                                /* pages are demand-zero, already cleared */
} /* emul_alloc */


ulong emul_arena_free( void )
/* Bytes still carve-able from the 68k arena in ONE contiguous piece.
 * Callers that must allocate a large block up front (a RAM disk is the whole
 * image at once) can use this to refuse an impossible request with a message
 * that names the real constraint, instead of letting get_mem fail late with a
 * bare "No more memory !!!" -- which is also what an exhausted memtable
 * prints, so the cause is genuinely ambiguous once you are down there.
 * Conservative on purpose: it ignores the REUSE_MEM free list, whose blocks
 * are small fragments that could not satisfy such a request anyway. */
{
    if (emul_base==NULL || emul_next==NULL) return 0;
    return (ulong)( emul_end - emul_next );
} /* emul_arena_free */


ulong largest_free_block( void )
/* The largest block get_mem can hand out in one piece right now: the arena
 * still to be carved, rounded down to get_mem's 64-byte grain, or the biggest
 * block waiting on the free list, whichever is larger. F$SRqMem allocates this
 * when asked for -1, "the largest block of free memory". */
{
    ulong best= emul_arena_free() & ~(ulong)63;

    #ifdef REUSE_MEM
      int k;
      for (k=0; k<MAX_MEMALLOC; k++) {
          if (freeinfo.f[ k ].base!=NULL && freeinfo.f[ k ].size>best) best= freeinfo.f[ k ].size;
      }
    #endif
    return best;
} /* largest_free_block */


typedef struct { uint32_t addr, size; } freeblk_typ;

static int CompareFreeBlk( const void* a, const void* b )
{   uint32_t x= ((const freeblk_typ*)a)->addr, y= ((const freeblk_typ*)b)->addr;
    return x<y ? -1 : x>y ? 1 : 0;
} /* CompareFreeBlk */

static void PutBE32( byte* p, uint32_t v )
{   p[0]= (byte)(v>>24); p[1]= (byte)(v>>16); p[2]= (byte)(v>>8); p[3]= (byte)v;
} /* PutBE32 */

uint32_t free_block_map( uint32_t from, byte* buf, uint32_t bufsz,
                         uint32_t* totalFree, uint32_t* totalRam )
/* F$GBlkMp's map of free memory: the arena not yet carved, and every block on
 * the free list, as 68k address and size in address order. Those at or above
 * <from> go into <buf> as big-endian pairs, as many as fit ahead of the 0 that
 * ends the list. Returns how many free fragments there are in all; <totalFree>
 * gets their total size and <totalRam> the arena's, less the low page that is
 * never handed out. */
{
    freeblk_typ* list;
    uint32_t     n= 0, i, room, put= 0;
    int          k, count= 1;

    *totalFree= 0;
    *totalRam = emul_base!=NULL ? (uint32_t)(emul_end-emul_base) - EMUL_RESERVED : 0;
    if (bufsz>=4) PutBE32( buf, 0 );

    #ifdef REUSE_MEM
      for (k=0; k<MAX_MEMALLOC; k++) if (freeinfo.f[ k ].base!=NULL) count++;
    #endif
    list= malloc( (size_t)count*sizeof(freeblk_typ) ); if (list==NULL) return 0;

    if (emul_arena_free()>0) {
        list[ n ].addr= (uint32_t)(emul_next-emul_base);
        list[ n ].size= (uint32_t)emul_arena_free();
        n++;
    }
    #ifdef REUSE_MEM
      for (k=0; k<MAX_MEMALLOC && (int)n<count; k++) {
          if (freeinfo.f[ k ].base==NULL) continue;
          list[ n ].addr= (uint32_t)((unsigned char*)freeinfo.f[ k ].base-emul_base);
          list[ n ].size= (uint32_t)freeinfo.f[ k ].size;
          n++;
      }
    #endif
    qsort( list, n, sizeof(freeblk_typ), CompareFreeBlk );

    room= bufsz>=4 ? (bufsz-4)/8 : 0; /* pairs that fit ahead of the 0 */
    for (i=0; i<n; i++) {
        *totalFree+= list[ i ].size;
        if (list[ i ].addr<from || put>=room) continue;
        PutBE32( buf+8*put,   list[ i ].addr );
        PutBE32( buf+8*put+4, list[ i ].size );
        put++;
    }
    if (bufsz>=4) PutBE32( buf+8*put, 0 );

    free( list );
    return n;
} /* free_block_map */


/* prepare the memory handling for use */
void init_all_mem(void)
{
    int k;

    totalMem= 0; /* initialize startup memory */

    if (emul_base==NULL) { /* allocate the 68k RAM arena once */
        emul_base= (unsigned char*)calloc( (size_t)emul_arena_size, 1 );
        if (emul_base==NULL) {
            /* Was a bare message that then fell through to the pointer
             * arithmetic below, leaving emul_next at 0x1000 and every
             * subsequent get_mem handing out addresses into nothing. Nothing
             * reached it while the arena was a fixed 32 MB compile-time
             * constant; -M makes the size a user input, so it is reachable now. */
            upe_printf( "Cannot allocate 68k memory arena (%u bytes) !!!\n", (uint32_t)emul_arena_size );
            exit( 1 );
        }
        emul_next= emul_base + EMUL_RESERVED; /* keep the low page out of circulation */
        emul_end = emul_base + emul_arena_size;
        emul_arena_limit = (ulong32)emul_arena_size; /* 32-bit bound for get_real_address */

        #ifdef USE_UAEMU
        {   /* reserve supervisor scratch stack for UAE exception frames */
            unsigned char* ts= (unsigned char*)emul_alloc( TRAPFRAMEBUFLEN );
            if (ts)
                trapstack_isp= TO68K( ts + TRAPFRAMEBUFLEN );
        }
        #endif
    } /* if */
    
    #ifdef REUSE_MEM
       freeinfo.freeN  = 0;
       freeinfo.freeMem= 0;
    #endif
     
    for (k=0;k<MAX_MEMALLOC;k++) {
        memtable[k].base= NULL;
        memtable[k].size= 0;

        #ifdef REUSE_MEM
          freeinfo.f[k].base= NULL;
          freeinfo.f[k].size= 0;
        #endif
    } /* for */

    /* allocate the module directory table in the 68k arena so D_ModDir can return TO68K(mdirField) */
    if (mdirField==NULL)
        mdirField= (mdir_entry*)get_mem( MAXMODULES * sizeof(mdir_entry) );

    init_intcmd_stub(); /* the module F$Link hands back for an internal command */

    /* allocate a zeroed I/O device table in the arena so D_DevTbl returns a valid 68k address */
    if (devtbl_arena==NULL)
        devtbl_arena= (byte*)get_mem( 0x0900 );

    /* The process-descriptor block table, the descriptor images it points at, the
     * path table, and the per-process signal scratch areas -- all in the arena,
     * for the same reason mdirField and devtbl_arena are: the guest is handed
     * their 68k ADDRESSES (F$SetSys D_PrcDBT/D_PthDBT, F$GPrDBT, P$SigDat), and
     * an address is only usable if it names memory the 68k side can reach. These
     * used to be plain host globals, so the "pointers" the guest got were
     * truncated host addresses -- meaningless, and (landing inside the arena by
     * chance) they read unrelated emulator memory instead of faulting.
     *
     * Allocated ONCE, here, and reused for the life of the run: the contents are
     * refreshed in place by Update_PrcDBT() on each call, never reallocated, so
     * there is nothing to leak. Fixed cost: 129 * 2048 = 258KB of descriptors
     * plus ~5KB of tables, out of a 32MB arena. */
    if (prDBT==NULL)
        prDBT       = (uint32_t*)get_mem( MAXPROCESSES * sizeof(uint32_t) );
    if (prcDsc==NULL)
        prcDsc      = (procid*)  get_mem( MAXPROCESSES * sizeof(procid)   );
    if (syspth==NULL)
        syspth      = (uint32_t*)get_mem( MAXSYSPATHS  * sizeof(uint32_t) );
    if (sigdat_arena==NULL)
        sigdat_arena= (byte*)    get_mem( MAXPROCESSES * SIG_SCRATCH      );
} /* init_all_mem */



#ifdef REUSE_MEM
static void MemLine( int *k, ulong value, const char* s  )
{
	ulong tot= 0;
	int   n  = 0;
    memblock_typ* f;

	while (*k>0) {
		          f= &freeinfo.f[(*k)-1];
        if       (f->base!=NULL) {
		    if   (f->size>value) break;
		    tot+= f->size;
		    n++;
		} /* if */
		
		(*k)--;
	} /* while */
	
	if (n>0) upo_printf( "%s  %4d  %9lu\n", s,  n,  (unsigned long)tot );
	else     upo_printf( "%s  %4s  %9s\n", s, "-", "-" );
} /* MemLine */
#endif

    

/* show memory blocks */
void show_mem( ushort npid, Boolean mem_unused, Boolean mem_fulldisp )
{
    memblock_typ *m;
    int k,pid;
    
    #ifdef REUSE_MEM
      #define UpL 0xffffffff
      void *nx, *nxMin;
      /* Both zeroed: they are assigned late in the loop and read on the NEXT
         pass, guarded by `nx!=NULL` -- true today, and not something to have
         to re-derive from the loop shape. */
      ulong nxSiz= 0, svSiz= 0;
      memblock_typ* f;
      int diff, i;
    #endif
      
    #ifdef MACOS9
      upo_printf("Macintosh Heap: MemFree=%ld, MaxBlock=%ld\n\n",FreeMem(),MaxBlock());
    #endif
    
    if (mem_unused) {
    	if (mem_fulldisp) {
   			upo_printf("  #     start       end      size   to next\n");
    		upo_printf("---  --------  --------  --------  --------\n");
    		
    	    #ifdef REUSE_MEM
    		  nxMin= 0; 
    		  nx   = 0; 
    		  i    = 0;
    		  
    		  while (true) {
   				  if (nx!=NULL) { /* not for the first time */
   				  	  diff= (int)((char*)nxMin-(char*)nx) - svSiz;
   				  	  upe_printf( "%3d  %p  %p  %8lu", i-1, nx, (void*)((char*)nx+svSiz), (unsigned long)svSiz );
   				  	  if (nxMin==(char*)UpL) { upe_printf( "\n" ); break; }
   				  	  upe_printf( "%8ld  %p\n", (long)diff, (void*)nxMin );
   				  } /* if */
   				  	  
   				  nx   = nxMin;
   				  svSiz= nxSiz;
    			  nxMin= (char*)UpL;
    			
    			  for (k=0; k<MAX_MEMALLOC; k++) {
		                  f= &freeinfo.f[k];
                      if (f->base==NULL) break;
                      if (f->base>nx && 
                          f->base<nxMin) {
                          nxMin= f->base;
                          nxSiz= f->size;
                      }
   				  } /* for */
   				  
   				  i++;
    		  } /* while */
    		#endif
    		
    		return;
    	}
    	
   		upo_printf("   size     #      total\n");
    	upo_printf("-------  ----  ---------\n");
    	
    	#ifdef REUSE_MEM
    	  			k= MAX_MEMALLOC;
    	  MemLine( &k,        128, "<=  128" );
    	  MemLine( &k,        256, "<=  256" );
    	  MemLine( &k,        512, "<=  512" );
     	  MemLine( &k,       1024, "<=  1kB" );
     	  MemLine( &k,       2048, "<=  2kB" );
     	  MemLine( &k,       5120, "<=  5kB" );
     	  MemLine( &k,      10240, "<= 10kB" );
     	  MemLine( &k,      20480, "<= 20kB" );
     	  MemLine( &k,      51200, "<= 50kB" );
     	  MemLine( &k,     102400, "<=100kB" );
     	  MemLine( &k,     204800, "<=200kB" );
     	  MemLine( &k,     512000, "<=500kB" );
     	  MemLine( &k,    1048576, "<=  1MB" );
     	  MemLine( &k, 0xffffffff, ">   1MB" );
		    upo_printf( "\n" );

    	  			k= MAX_MEMALLOC;
     	  MemLine( &k, 0xffffffff, "  total" );
   	  #endif
    	
    	return;
    } /* if mem_unused */
    
    upo_printf("Pid  Block   Start      Size       End (+1)\n");
    upo_printf("---  -----  ---------  ---------  ---------\n");
    
    for (pid=0; pid<MAXPROCESSES; pid++) {
        if (procs[pid].state!=pUnused) {
            if (npid==MAXPROCESSES || npid==pid) {
                for (k=0; k<MAXMEMBLOCKS; k++) {
                    //  m= &procs[pid].os9memblocks[k];
                        m= &pmem[ pid ].m[ k ];
                    if (m->base!=NULL) {
                        upo_printf("%2d%c  %5d  $%08X  $%08X  $%08X\n",
                                    pid,
                                    pid==currentpid ? '*' : ' ',
                                    k,
                                    TO68K(m->base),
                                    (uint32_t)m->size,
                                    TO68K(m->base) + (uint32_t)m->size);
                    }
                }
            }
        }
    }
} /* show_mem */



void show_unused(void)
{
  #ifdef REUSE_MEM
    int   k, n= 0;
    (void)n;   /* counted for the debug view only */
    memblock_typ* f;
    char s[10];
    
    upo_printf("Block   Start      Size          Size\n");
    upo_printf("-----  ---------  --------- ---------\n");

    for (k=0;k<MAX_MEMALLOC;k++) {
          f= &freeinfo.f[ k ];
      if (f->base!=NULL)    { upo_printf("%5d  %p  $%08lX  %8lu\n",
                                              k, f->base, (unsigned long)f->size,
                                                 (unsigned long)f->size ); n++;
      }
    } // for

    snprintf( s,sizeof(s), "(%d)", freeinfo.freeN );
    upo_printf("\nTOTAL %6s      $%08lX  %8lu\n", s, (unsigned long)freeinfo.freeMem, (unsigned long)freeinfo.freeMem );
  #endif
} // show_unused



/* initialize process' memory block list */
void init_mem(ushort pid)
{
  int  k;
  for (k=0; k<MAXMEMBLOCKS; k++) pmem[ pid ].m[ k ].base=NULL; // no memory yet
} /* init_mem */



#ifdef REUSE_MEM
  static Boolean release_ok( void* membase, ulong memsz )
  {
	int           k;
    memblock_typ* f;

  //#ifndef MEM_SHIELD
	void* qq;
    ulong svsize;
	
	/* check if piece appended at the end available */
    qq= (char*)membase + memsz;
	for (k=0;k<MAX_MEMALLOC;k++) {
            f= &freeinfo.f[k];
        if (f->base==NULL) break;
        if (f->base==qq) {
        	memsz += f->size; /* glue it together */
          	svsize = f->size;
            MoveBlk( f,&freeinfo.f[k+1], (freeinfo.freeN-k)*sizeof(memblock_typ) );

            #ifdef win_linux
              totalMem-= svsize; /* free memory is not really released */
            #endif
                  
            freeinfo.freeN--;
            freeinfo.freeMem-= svsize;
            break;
        } /* if */
    } /* for */
    
	/* check if piece appended at the beginning available */
	for (k=0;k<MAX_MEMALLOC;k++) {
            f= &freeinfo.f[k];
        if (f->base==NULL) break;
        if ((char*)f->base+f->size==membase) {
        	membase= f->base; /* the new starting point */
        	memsz += f->size; /* glue it together */
        	svsize=  f->size;
            MoveBlk( f,&freeinfo.f[k+1], (freeinfo.freeN-k)*sizeof(memblock_typ) );

            #ifdef win_linux
              totalMem-= svsize; /* free memory is not really released */
            #endif
                  
            freeinfo.freeN--;
            freeinfo.freeMem-= svsize;
            break;
        } /* if */
    } /* for */
  //#endif
    
	for (k=0;k<MAX_MEMALLOC;k++) { /* do not really release the memory */
            f= &freeinfo.f[k];
    	if (f->base==NULL || f->size<memsz) {
        	MoveBlk( &freeinfo.f[k+1],f, (freeinfo.freeN-k)*sizeof(memblock_typ) );
        	
        	f->base= membase;
        	f->size= memsz;
              
            #ifdef win_linux
              totalMem+= memsz; /* free memory is not really released */
            #endif
                  
            freeinfo.freeN++;
            freeinfo.freeMem+= memsz;
			return true;
		} /* if */
	} /* for */
	
	return false;
  } /* release_ok */
#endif


ulong max_mem()
/* the currently max available memory */
{
    ulong memsz;
    
    #if defined MACOS9
      memsz= MaxBlock();
      
    #elif defined win_unix
      memsz= 0;
      
    #else
      #error MaxBlock size must be defined here
    #endif
    
    #ifdef REUSE_MEM
      memsz+= freeinfo.freeMem;
    #endif
    
    return memsz;
} /* max_mem */



// --------------------------------------------------------------------------------------
// install a memory area as memory block
static ushort install_memblock(ushort pid, void *base, ulong size)
{
  pmem_typ*     cm= &pmem[ pid ];
  memblock_typ* m;
  
  int  k;
  for (k=0;k<MAXMEMBLOCKS;k++) {
        m= &cm->m[ k ];
    if (m->base==NULL) {
        m->base= base;             // save pointer
        m->size= size;             // save block size
        LockMemRange( base,size ); // keep always paged in !!
        return k;
    } // if
  } // for
   
  return MAXMEMBLOCKS;
} /* install_memblock */


static Boolean BlockHolds( const memblock_typ* m, const byte* b, ulong cnt )
/* True if [b, b+cnt) lies wholly within allocated block m (overflow-safe, as
 * RangeInProcMem explains). */
{
  const byte* base= (const byte*)m->base;
  const byte* end;
  if (base==NULL) return false;
  end= base + m->size;
  return b>=base && b<end && cnt<=(ulong)(end-b);
} /* BlockHolds */


/* The block that last matched, tried first: under -W every write a C program
 * makes to its heap asks, and the table is MAXMEMBLOCKS long. BlockHolds
 * re-reads the slot each time, so a freed or reused one never grants a stale
 * range. */
static ushort lastPid  = MAXPROCESSES;
static int    lastBlock= 0;

Boolean RangeInProcMem( ushort pid, void* p, ulong cnt )
/* True if the byte range [p, p+cnt) lies wholly within process <pid>'s own
 * writable memory: its static-storage data area [memstart,memtop) -- which the
 * emulator treats as the whole data area, stack and parameter block included
 * (os9exec_nt.c warns when A7 leaves that span) -- or one of its F$SRqMem
 * blocks. This is the process-owned half of F$ChkMem's write-access test (the
 * other half, loaded RAM modules, is RangeInAnyModule) -- the check F$CpyMem
 * applies to its DESTINATION: OS-9 lets a user-state caller READ any address but
 * only WRITE where it has permission. Each span is tested as cnt<=(top-b) rather
 * than b+cnt<=top, so it never forms an out-of-range pointer -- a b+cnt overflow
 * check is undefined and the compiler folds it (the RANGE_IN_ARENA note). It
 * also skips a zero-length data area (memstart==memtop==0 for a process with no
 * memory), whose NULL bound a relational compare may not touch. */
{
  process_typ* cp= &procs[ pid ];
  byte*        b = (byte*)p;
  int          k;

  if (cnt==0) return true;   /* an empty write touches nothing */
  if (pid>=MAXPROCESSES) return false; /* the no-process sentinel owns nothing */

  if (cp->memtop > cp->memstart) { /* has a data area at all */
    byte* lo= (byte*)FROM68K( cp->memstart );
    byte* hi= (byte*)FROM68K( cp->memtop   );
    if (b>=lo && b<hi && cnt<=(ulong)(hi-b)) return true;
  } // if

  if (lastPid==pid && BlockHolds( &pmem[ pid ].m[ lastBlock ], b, cnt )) return true;

  for (k=0; k<MAXMEMBLOCKS; k++) { /* or one of its allocated memory blocks */
    if (BlockHolds( &pmem[ pid ].m[ k ], b, cnt )) {
      lastPid  = pid;
      lastBlock= k;
      return true;
    } // if
  } // for

  return false;
} /* RangeInProcMem */



void release_mem( void* membase )
/* process independent part of memory deallocation */
{
    ulong memsz= 1;
    int  k;
    memblock_typ* m;
    
    for (k=0;k<MAX_MEMALLOC;k++) {
            m= &memtable[k];
        if (m->base==membase) { /* search for a segment to be released */
            memsz= m->size;
       
            #ifdef win_linux
              totalMem-= memsz;
            #endif
            
            m->base= NULL; /* and release the segment */
            m->size= 0;
            break;
        } /* if */
    } /* for */
    
    #ifdef REUSE_MEM
      /* Diagnostics for os9exec's own bookkeeping, so they go where -d output
         goes. They were plain printf: a host pointer in the guest's output. */
      if (memsz==0) {
        debugprintf(dbgMemory,dbgNorm,("# release_mem: STRANGE BLOCK (size 0) at %p\n", membase ));
        return;
      } // if
      if (memsz==1) {
        debugprintf(dbgMemory,dbgNorm,("# release_mem: UNUSED BLOCK (not allocated) at %p\n", membase ));
        return;
      } // if
      
      if (release_ok( membase,memsz )) {
        debugprintf(dbgMemory,dbgNorm,("# release_mem: release block     at %p (size=%5u) %8u\n",
                                          membase,(uint32_t)memsz, (uint32_t)totalMem ));
        return;
      } /* if */
    #endif

    debugprintf(dbgMemory,dbgNorm,("# release_mem: release block     at %p (size=%5u) %8u\n",
                                      membase,(uint32_t)memsz, (uint32_t)totalMem ));
      
    #ifdef MACMEM
      DisposePtr( membase );

      if (MemError()!=noErr) {
          debugprintf(dbgMemory+dbgAnomaly,dbgDeep,
                 ("# release_memblock: DisposePtr returned Mac OS9 MemError=%d\n",MemError()));
      }
    #else
      /* arena memory is never returned to the host: membase points inside the
         single 68k arena, not at an individually malloc'd block, so it must
         not be passed to free(). Without REUSE_MEM the block simply stays
         out of circulation; enabling REUSE_MEM recycles it via release_ok. */
      (void)membase;
    #endif
} /* release_mem */



/* free an allocated memory block */
static void release_memblock( ushort pid, ushort memblocknum )
{
  pmem_typ*         cm= &pmem[ pid ];
  memblock_typ* m= &cm->m[ memblocknum ];
    
  if (m->base==NULL) return;
    
  debugprintf(dbgMemory,dbgNorm,("# release_memblock:    block #%-2d at %p (size=%5u) %8u pid=%d\n",
                                    memblocknum, m->base, (uint32_t)m->size, (uint32_t)totalMem, pid ));
    
  #ifndef MACMEM     
    UnlockMemRange( m->base, m->size );
  #endif

  release_mem( m->base );
    
  m->base= NULL; // free block
  m->size= 0;
} // release_memblock



/* free process' allocated memory blocks */
void free_mem(ushort pid)
{
   int  k;
   for (k=0; k<MAXMEMBLOCKS; k++) release_memblock( pid,k );   
} /* free_mem */



/* Allocation failures belong to the PROGRAM, which gets E$NoRAM or E$MemFul
   exactly as on OS-9 and is told nothing else: real OS-9 refuses silently.

   This used to print "No more memory" onto the current process's stderr. The
   line dates from the Mac original, where get_mem asked the HOST for memory
   (NewPtrClear), so a failure meant the emulator itself had run out -- a
   classic Mac OS application partition set too small -- and only the operator
   could act on it. It reached the guest's stderr in 2000, when "everything"
   moved to the user-path printers. By then the arena was the emulator's own,
   and in practice the line almost always announced a program asking for
   nonsense (an unset register), interleaved with that program's output. One
   runaway allocator printed it 439,689 times.

   What is still the operator's is an ARENA too small for a sane request: that
   one is said once per run, on the emulator's console (dbg_printf, never a
   guest path), naming -M. A request larger than the whole arena is not that --
   no -M would help -- and stays silent. Everything is in the -d 0x0040 trace. */
static Boolean arenaFullSaid= false;

static void alloc_failed( ulong memsz, const char* why )
{
    debugprintf(dbgMemory,dbgNorm,("# alloc_failed: %lu-byte request refused%s, %lu bytes free in a %lu-byte arena\n",
                                    (unsigned long)memsz, why,
                                    (unsigned long)emul_arena_free(), (unsigned long)emul_arena_size));

    if (arenaFullSaid || memsz>emul_arena_size || memsz<=emul_arena_free()) return;

    arenaFullSaid= true;
    dbg_printf( "# os9exec: the %lu MB 68k memory arena is full; -M <size> makes it larger\n",
                (unsigned long)(emul_arena_size/(1024*1024)) );
} /* alloc_failed */


static void* get_mem_once( ulong memsz )
/* process independent part of memory allocation */
{
    void* pp= NULL;
    int   k;
    
    #define MBlk 64
    ulong sz;
    
    #ifdef REUSE_MEM
      #define FullBlk 524288
      memblock_typ*   f;
      void*           qq;
      ulong           sv_size;
      Boolean         cond;
    #endif
      
    /* A size within a block of 4 GB would wrap to 0 below and be handed a
       minimum block while the caller believes it has what it asked for.
       Nothing that large fits a 68k address space anyway. */
    if (memsz > 0xFFFFFFFFUL-(MBlk-1)) return NULL;
    memsz= (memsz+MBlk-1) & 0xFFFFFFC0; /* round up to next boundary */

    /* A zero-byte request (F$SRqMem with d0=0 is one) still gets a block of
       its own. Recorded at size 0 it could share its address with the next
       allocation, and blocks are released by address -- releasing one could
       then take the other's entry. The guest is still told what it asked for:
       this is only how much of the arena stands behind it. */
    if (memsz==0) memsz= MBlk;
    
    #ifdef REUSE_MEM
      for (k= MAX_MEMALLOC-1; k>=0; k--) { /* try to get it from the free list */
            f= &freeinfo.f[ k ];      
        if (f->base!=NULL) {
              cond= f->size==  memsz ||
                    f->size>=2*memsz ||
                    f->size>=  memsz+8*MBlk;
          if (cond) {     
                    pp= f->base;
            memset( pp, 0, memsz ); /* some programs need a clean block !!! */
              
//          printf( "- %d %d %d %d\n", freeinfo.freeN, k, (freeinfo.freeN-k)*sizeof(memblock_typ), memsz );
//          show_unused();
              
            sv_size= f->size; /* save it before overwritten */            
            MoveBlk( f,&freeinfo.f[k+1], (freeinfo.freeN-k)*sizeof(memblock_typ) );
            if (sv_size>memsz) {
                          qq= (char*)pp + memsz;
              release_ok( qq,   sv_size - memsz );
            } // if
                  
            #ifdef win_linux
              totalMem-= sv_size; /* memory was not really free */
            #endif

            freeinfo.freeN--;
            freeinfo.freeMem-= sv_size;                                    

//          printf( "+ %d %d %d %d\n", freeinfo.freeN, k, (freeinfo.freeN-k)*sizeof(memblock_typ), memsz );
//          show_unused();
            break;
          } // if    
        } // if 
      } // for
    #endif
        
    sz= memsz;
    if (pp==NULL) { /* not yet found */
      #ifdef MACMEM
      	#ifdef REUSE_MEM
      	  if (sz>FullBlk)
          	  pp= NewPtrClear( sz );
          else {
      	      pp= NewPtrClear( FullBlk );
                          qq= (char*)pp + sz;
              release_ok( qq,  FullBlk  - sz );
          }
        #else
          pp= NewPtrClear( sz );
        #endif
                
        if (pp==NULL) {
          debugprintf(dbgMemory,dbgNorm,("# get_mem: %s returned MacOS MemError=%d\n",
                                         "NewPtrClear", MemError()));
        }
      #else
        pp= emul_alloc( sz ); /* carve a zeroed block from the 68k arena */
      #endif
    } // if
 
    if (pp!=NULL) {
        for (k=0;k<MAX_MEMALLOC;k++) {
            if (memtable[k].base==NULL) { /* search for a free segment */
                #ifdef win_linux
                  totalMem+= memsz;
                #endif
            
                memtable[k].base= pp;
                memtable[k].size= memsz;
                LockMemRange( pp, memsz ); /* keep always paged in !! */ 
               
                debugprintf(dbgMemory,dbgNorm,("# get_mem:    allocate block     at %p (size=%5u) %8u\n",
                                                  pp, (uint32_t)memsz, (uint32_t)totalMem ));
                return pp;
            } /* if */
        } /* for */
    } /* if */
    
    return NULL;
} /* get_mem_once */


void* get_mem( ulong memsz )
/* A block of <memsz> bytes, or NULL. Sticky modules kept at link count 0 are
   the memory OS-9 gives back "when memory is required for another use" (M$Attr
   bit 6), so a request that fails first releases them and tries once more. */
{
    void* pp= get_mem_once( memsz );

    if (pp==NULL && release_sticky_modules()>0) pp= get_mem_once( memsz );
    if (pp==NULL) alloc_failed( memsz, "" );
    return pp;
} /* get_mem */


/* memory allocation for OS-9 */
void* os9malloc( ushort pid, ulong memsz, os9err* whyP )
/* <whyP> receives the reason on failure, because the two ways this can fail are
   different errors and the manual distinguishes them. E$NoRAM (237) is "no free
   RAM, or not enough contiguous memory"; E$MemFul (207) is "MEMORY FULL ... This
   can ALSO occur if a process has already been allocated the maximum number of
   blocks permitted by the system" -- which is exactly what running out of
   MAXMEMBLOCKS is. Both used to surface as E$NoRAM, telling a program the
   machine was out of memory when what it had really hit was its own block
   limit. */
{
    void *pp;
    int   k;

    if (whyP!=NULL) *whyP= 0;

        pp= get_mem( memsz );
    if (pp==NULL) {
        if (whyP!=NULL) *whyP= os9error(E_NORAM);
        return NULL; /* no memory */
    } // if
     
        k= install_memblock( pid, pp, memsz );
    if (k>=MAXMEMBLOCKS) {
        /* memory block list is full */
        release_mem( pp ); pp= NULL;
        if (whyP!=NULL) *whyP= os9error(E_MEMFUL);
        
        alloc_failed( memsz, " at the per-process block limit" );
    } // if
   
    debugprintf(dbgMemory,dbgNorm,("# os9malloc:  allocate block #%-2d at %p (size=%5u) %8u pid=%d\n",
                                      k, pp, (uint32_t)memsz, (uint32_t)totalMem, pid ));
    return pp;
} /* os9malloc */


/* memory deallocation for OS-9 */
os9err os9free( ushort pid, void* membase, ulong memsz )
{
  os9err        err;
  pmem_typ*     cm= &pmem[ pid ];
  memblock_typ* m;
  int           k;

  debugprintf(dbgMemory,dbgDetail, ( "# os9free:      free request   at %p (size=%u) pid=%d\n",
                                        membase, (uint32_t)memsz, pid ) );
  if (membase!=NULL) {
    for (k=0; k<MAXMEMBLOCKS; k++) {
            m= &cm->m[ k ];
      if   (m->base==membase) {
        if (m->size==memsz  ) {
          release_memblock( pid, k );
          return 0; /* freed ok */
        } // if
        
        /* A block recorded as zero bytes (F$SRqMem with d0=0 makes one) has
           no pieces to walk: freeing it "in smaller pieces" below recursed on
           the same address for ever and overflowed the host's stack -- two
           system calls crashed the emulator. It is the caller's block, so it
           goes, whatever size it is given back with. */
        if (m->size==0) {
          release_memblock( pid, k );
          return 0; /* freed ok */
        } // if

        // try to free it in smaller pieces ...
        // NOTE: e.g. OS-9 "dir" is doing it this way !
        if (memsz > m->size) { // recursive call
               err= os9free( pid, (void*)( (uintptr_t)membase + m->size ), memsz - m->size );
          if (!err) {
            release_memblock( pid, k ); // only release it, if all of them are fitting
            return 0; /* freed ok */
          } // if
          
          debugprintf(dbgMemory,dbgNorm, ( "# os9free:     release block #%-2d at %p, but wrong size=%u (specified=%4u) %8u\n",
                                              k, membase, (uint32_t)m->size, (uint32_t)memsz, (uint32_t)totalMem ) );
          break;
        } // if
            
        return 0; /* freed ok */
      } // if
    } /* for */
  } // if
  
//upe_printf( "bad block %08X size=%d\n", membase, memsz );
  debugprintf(dbgMemory+dbgAnomaly,dbgNorm, ("# os9free: Block at %p (size=%u) not found in pid=%d's memory list\n",
                                                membase, (uint32_t)memsz, pid ) );
  return os9error(E_BPADDR); /* no memory was allocated here */
} /* os9free */


/* Resize one of a process's blocks IN PLACE, for F$Mem.
 *
 * A process's data area is one block (prepData allocates it with os9malloc),
 * and F$Mem's contract is that it grows "contiguously upward" and shrinks
 * "downward from the old highest address" -- its base never moves, because
 * everything the program owns is addressed from A6.  So this is not a
 * realloc: the block stays where it is, and the question is only whether the
 * arena directly above it can be claimed.
 *
 * Two places that memory can come from, and both are checked: the REUSE_MEM
 * free list, when a released block happens to start exactly at our end, and
 * the arena's bump pointer, when this block was the last one carved.  Anything
 * else is E$MemFul ("not enough contiguous RAM free"), which the manual
 * explicitly permits for an expansion even when plenty of memory is free:
 * "the data area must always be contiguous".  E$NoRAM is kept for the arena
 * itself being exhausted.
 *
 * Sizes.  The guest's size (<newsz>, what F$Mem reports) lives in the
 * per-process table; the arena's size, rounded to get_mem's MBlk granularity,
 * lives in memtable -- the same split os9malloc already leaves behind, since
 * F$SRqMem rounds to 16 and get_mem to 64.  A shrink returns only whole
 * arena-granularity units above the new top, and a growth that still fits in
 * that slack allocates nothing.
 */
os9err os9resize( ushort pid, void* membase, ulong newsz )
{
  pmem_typ*     cm= &pmem[ pid ];
  memblock_typ* m = NULL;
  memblock_typ* t = NULL;
  byte*         end;
  ulong         old64, new64, need;
  int           k;

  for (k=0; k<MAXMEMBLOCKS; k++) if (cm->m[k].base==membase) { m= &cm->m[k]; break; }
  for (k=0; k<MAX_MEMALLOC; k++) if (memtable[k].base==membase) { t= &memtable[k]; break; }
  if (m==NULL || t==NULL) return os9error(E_BPADDR); /* not a block of this process */

  /* A size bigger than the whole arena can never fit, wherever the block
     sits: that is the arena being exhausted, E$NoRAM, not E$MemFul.  Refusing
     it here also keeps the rounding below from wrapping where ulong is 32 bits
     (i386, wasm32, mingw): a size within a block of 4 GB rounded to 0, read
     as a shrink, and put the whole data area on the free list while the
     process still owned it. */
  if (newsz > (ulong)(emul_end-emul_base)) return os9error(E_NORAM);

  old64= t->size;
  new64= (newsz+MBlk-1) & ~(ulong)(MBlk-1);
  end  = (byte*)membase + old64;

  if (new64 < old64) {           /* shrink: give the tail back, whole units only */
    #ifdef REUSE_MEM
      #ifdef win_linux
        totalMem-= old64-new64;  /* the same dance release_mem does before release_ok */
      #endif
      UnlockMemRange( (byte*)membase+new64, old64-new64 );
      if (!release_ok( (byte*)membase+new64, old64-new64 )) {
        #ifdef win_linux
          totalMem+= old64-new64;
        #endif
        new64= old64;            /* free list full: keep the tail, nothing is lost */
      } /* if */
    #else
      new64= old64;              /* without a free list the tail cannot be recycled */
    #endif
  } /* if */
  else if (new64 > old64) {      /* grow: only if the arena right above us is free */
    need= new64-old64;
    if (end==emul_next) {        /* we were the last block carved: carve on, if the arena has it */
      if (need > (ulong)(emul_end-end)) { alloc_failed( need, " above the data area" ); return os9error(E_NORAM); }
      emul_next= end+need;
    } /* if */
    else {
      /* Somebody else's block is above us unless the free list says otherwise.
         That is the manual's E$MemFul -- "not enough contiguous RAM free" --
         rather than E$NoRAM, which is the arena being exhausted outright. */
      #ifdef REUSE_MEM
        memblock_typ* f= NULL;
        ulong         fsz;
        for (k=0; k<MAX_MEMALLOC; k++) {
          if (freeinfo.f[k].base==NULL) break;
          if (freeinfo.f[k].base==end && freeinfo.f[k].size>=need) { f= &freeinfo.f[k]; break; }
        } /* for */
        if (f==NULL) { alloc_failed( need, ": the block above the data area is in use" ); return os9error(E_MEMFUL); }
        fsz= f->size;
        MoveBlk( f,&freeinfo.f[k+1], (freeinfo.freeN-k)*sizeof(memblock_typ) );
        freeinfo.freeN--;
        freeinfo.freeMem-= fsz;
        #ifdef win_linux
          totalMem-= fsz;
        #endif
        if (fsz>need) release_ok( end+need, fsz-need );
      #else
        alloc_failed( need, ": the block above the data area is in use" ); return os9error(E_MEMFUL);
      #endif
    } /* else */
    LockMemRange( end, need );
    #ifdef win_linux
      totalMem+= need;
    #endif
  } /* else if */

  /* Every byte the guest gains is clean, as at fork -- the arena slack an
     earlier shrink kept included, which still holds what was written there. */
  if (newsz > m->size) memset( (byte*)membase + m->size, 0, newsz - m->size );

  t->size= new64;
  m->size= newsz;
  debugprintf(dbgMemory,dbgNorm,("# os9resize:  block at %p now size=%u (arena %u) pid=%d\n",
                                    membase, (uint32_t)newsz, (uint32_t)new64, pid ));
  return 0;
} /* os9resize */



/* eof */

