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
#define EMUL_ARENA_SIZE  (256u*1024u*1024u)  /* generous; bigger than any real OS-9/68k machine */
#define EMUL_RESERVED    0x1000u             /* low page never allocated */

unsigned char* emul_base = NULL;             /* arena base (referenced by memory.h) */
static unsigned char* emul_next = NULL;      /* bump pointer for fresh allocations */
static unsigned char* emul_end  = NULL;      /* one past the end of the arena */


static void* emul_alloc( ulong memsz )
/* bump-allocate a zeroed block from the 68k arena; NULL if exhausted */
{
    unsigned char* p= emul_next;
    if (p+memsz > emul_end) return NULL;     /* arena exhausted */
    emul_next= p+memsz;
    return p;                                /* pages are demand-zero, already cleared */
} /* emul_alloc */


/* prepare the memory handling for use */
void init_all_mem(void)
{
    int k;

    totalMem= 0; /* initialize startup memory */

    if (emul_base==NULL) { /* allocate the 68k RAM arena once */
        emul_base= (unsigned char*)calloc( (size_t)EMUL_ARENA_SIZE, 1 );
        if (emul_base==NULL) {
            upe_printf( "Cannot allocate 68k memory arena (%u bytes) !!!\n", EMUL_ARENA_SIZE );
        }
        emul_next= emul_base + EMUL_RESERVED; /* keep the low page out of circulation */
        emul_end = emul_base + EMUL_ARENA_SIZE;
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
	
	if (n>0) upo_printf( "%s  %4d  %9d\n", s,  n,  tot );
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
      ulong nxSiz, svSiz;
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
   				  if (nx>(char*)0) { /* not for the first time */
   				  	  diff= (int)((char*)nxMin-(char*)nx) - svSiz;
   				  	  upe_printf( "%3d  %08X  %08X  %8d", i-1, nx, (char*)nx+svSiz, svSiz );
   				  	  if (nxMin==(char*)UpL) { upe_printf( "\n" ); break; }
   				  	  upe_printf( "%8d  %08X\n", diff, nxMin );
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
                        upo_printf("%2d%c  %5d  $%08lX  $%08lX  $%08lX\n",
                                    pid,
                                    pid==currentpid ? '*' : ' ',
                                    k,      m->base,
                                                      m->size,
                                    (ulong) m->base + m->size);
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
    memblock_typ* f;
    char s[10];
    
    upo_printf("Block   Start      Size          Size\n");
    upo_printf("-----  ---------  --------- ---------\n");
    
    for (k=0;k<MAX_MEMALLOC;k++) {
          f= &freeinfo.f[ k ];      
      if (f->base!=NULL)    { upo_printf("%5d  $%08lX  $%08lX  %8d\n",
                                              k, f->base, f->size, f->size ); n++;
      }
    } // for
 
    sprintf( s, "(%d)", freeinfo.freeN );
    upo_printf("\nTOTAL %6s      $%08lX  %8d\n", s, freeinfo.freeMem,freeinfo.freeMem );
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
      if (memsz==0) { 
        printf( "STRANGE BLOCK at %08X\n", membase ); return;
      } // if
      if (memsz==1) { 
        printf( "UNUSED  BLOCK at %08X\n", membase ); return;
      } // if
      
      if (release_ok( membase,memsz )) {
        debugprintf(dbgMemory,dbgNorm,("# release_mem: release block     at $%08lX (size=%5u) %8d\n",
                                          membase,memsz, totalMem ));
        return;
      } /* if */
    #endif  
            
    debugprintf(dbgMemory,dbgNorm,("# release_mem: release block     at $%08lX (size=%5u) %8d\n",
                                      membase,memsz, totalMem ));
      
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
    
  debugprintf(dbgMemory,dbgNorm,("# release_memblock:    block #%-2d at $%08lX (size=%5u) %8d pid=%d\n",
                                    memblocknum, m->base, m->size, totalMem, pid ));
    
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



void* get_mem( ulong memsz )
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
      
    memsz= (memsz+MBlk-1) & 0xFFFFFFC0; /* round up to next boundary */
    
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
               
                debugprintf(dbgMemory,dbgNorm,("# get_mem:    allocate block     at $%08X (size=%5u) %8d\n",
                                                  (ulong)pp, memsz, totalMem ));
                return pp;
            } /* if */
        } /* for */
    } /* if */
    
  //#ifndef PLUGIN_DLL
    upe_printf( "No more memory !!!\n" );
  //#endif
    
    return NULL;
} /* get_mem */


/* memory allocation for OS-9 */
void* os9malloc( ushort pid, ulong memsz )
{
    void *pp;
    int   k;

        pp= get_mem( memsz );
    if (pp==NULL) return NULL; /* no memory */
     
        k= install_memblock( pid, pp, memsz );
    if (k>=MAXMEMBLOCKS) {
        /* memory block list is full */
        release_mem( pp ); pp= NULL;
        
      //#ifndef PLUGIN_DLL
        upe_printf( "No more memory (MAXMEMBLOCKS) !!!\n" );
      //#endif
    } // if
   
    debugprintf(dbgMemory,dbgNorm,("# os9malloc:  allocate block #%-2d at $%08X (size=%5u) %8d pid=%d\n",
                                      k, (ulong)pp, memsz, totalMem, pid ));
    return pp;
} /* os9malloc */


/* memory deallocation for OS-9 */
os9err os9free( ushort pid, void* membase, ulong memsz )
{
  os9err        err;
  pmem_typ*     cm= &pmem[ pid ];
  memblock_typ* m;
  int           k;

  debugprintf(dbgMemory,dbgDetail, ( "# os9free:      free request   at $%08lX (size=%lu) pid=%d\n",
                                        (ulong)membase, memsz, pid ) );
  if (membase!=NULL) {
    for (k=0; k<MAXMEMBLOCKS; k++) {
            m= &cm->m[ k ];
      if   (m->base==membase) {
        if (m->size==memsz  ) {
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
          
          debugprintf(dbgMemory,dbgNorm, ( "# os9free:     release block #%-2d at $%08lX, but wrong size=%lu (specified=%4u) %8d\n",
                                              k, membase, m->size, memsz, totalMem ) );
          break;
        } // if
            
        return 0; /* freed ok */
      } // if
    } /* for */
  } // if
  
//upe_printf( "bad block %08X size=%d\n", membase, memsz ); 
  debugprintf(dbgMemory+dbgAnomaly,dbgNorm, ("# os9free: Block at $%08lX (size=%lu) not found in pid=%d's memory list\n",
                                                membase, memsz, pid ) );
  return os9error(E_BPADDR); /* no memory was allocated here */
} /* os9free */



/* eof */

