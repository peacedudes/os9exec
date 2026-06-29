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

#define  _in_uae_c
#include "os9exec_incl.h"

#include "config.h"
#include "options.h"
#include "luzstuff.h"
#include "memory.h"
#include "readcpu.h"
#include "newcpu.h"
#include "compiler.h"
#undef   _in_uae_c

#include <stddef.h> /* offsetof */

/* llm_os9_go bulk-copies registers between os9exec's regs_type and UAE's
   regstruct, so the two MUST share an identical layout through the last common
   field. These build-time checks fail loudly the moment the structs drift --
   cheaper and surer than a runtime probe. The ttP/membase tail of regs_type is
   never reached because the memcpy uses sizeof(struct regstruct). */
_Static_assert( offsetof(regs_type,d)        == offsetof(struct regstruct,regs),                    "d[]/regs[] base drift" );
_Static_assert( offsetof(regs_type,a)        == offsetof(struct regstruct,regs)+8*sizeof(uae_u32),  "a[] register drift" );
_Static_assert( offsetof(regs_type,sr)       == offsetof(struct regstruct,sr),                      "sr drift" );
_Static_assert( offsetof(regs_type,pc)       == offsetof(struct regstruct,pc),                      "pc drift" );
_Static_assert( offsetof(regs_type,fp)       == offsetof(struct regstruct,fp),                      "fp[] drift" );
_Static_assert( offsetof(regs_type,fpcr)     == offsetof(struct regstruct,fpcr),                    "fpcr drift" );
_Static_assert( offsetof(regs_type,prefetch) == offsetof(struct regstruct,prefetch),                "prefetch drift" );
_Static_assert( sizeof (regs_type)           >= sizeof (struct regstruct),                          "regs_type smaller than regstruct" );

/* universal interface to 68k low level magic */
/* ========================================== */

/* statics */
static Boolean needsflush;
static Boolean hasfpu;
static Boolean vmenabled;
static Boolean fullcache;
static Boolean usesusermode;


/* dummies for UAE */
void Flush68kCodeRange( _bufferV_, _size_ ) { }
void LockMemRange     ( _bufferV_, _size_ ) { }
void UnlockMemRange   ( _bufferV_, _size_ ) { }


/* current regs as per last call of llm_os9_go */
static regs_type *currentrp;

/* called by newcpu.c's Exception routine */
void handle_os9exec_exception(int nr, uaecptr oldpc)
{
    // in case of os9exec, we apply special treating to most exceptions
    if (nr==32) {
        // Trap0 : OS9 system call: Don't stack anything, just exit emulator
        //         for execution of system call
        m68_os9go_result=next_iword(); // get OS9 function word
     // consprintf( ">%2d %3d<", nr-32, m68_os9go_result );

        os9_running=0; // force exit from emulator loop
    } else if (nr>32 && nr<48) {
        uae_u16 func=next_iword();  // get TCALL function word
        traphandler_typ *th;
     // consprintf( ">%2d %3d<", nr-32, func ); /* JustInCase (bfo) */
      
        // Prepare OS9_go result in case no traphandler is installed
        m68_os9go_result=(nr << 16) + func;
        
        // Save callers PC
        m68k_areg(regs, 7) -= 4;        /* do not jump back into your own command */
        put_long (m68k_areg(regs, 7), oldpc+2 ); /* +2: is very important (bfo)   */
        // Save vector Offset
        m68k_areg(regs, 7) -= 2;
        put_word (m68k_areg(regs, 7), nr * 4);
        // Save function code
        m68k_areg(regs, 7) -= 2;
        put_word (m68k_areg(regs, 7), func);
        // Save caller's A6
        m68k_areg(regs, 7) -= 4;
        put_long (m68k_areg(regs, 7), m68k_areg(regs, 6));
        // now check if we have an installed trap handler
        th=&((currentrp->ttP)[nr-FIRSTTRAP]);
        if (th->trapentry!=0) {
            // there is an installed trap handler, just call it
            m68k_setpc(th->trapentry);
            m68k_areg(regs, 6) = th->trapmem + 0x8000; 
            // ...and keep emulator running
        }           
        else {
            // no traphandler installed...          
            // .. just let os9exec_nt do the installation of the traphandler
            //    (the stack frame is ready)
            os9_running=0; // force exit from emulator loop
        }           
    }
    else {
        // Must be error trap
        uae_u32 callers_stack=m68k_areg(regs, 7);
        short i;
        
        // prep os9_go result code
        m68_os9go_result=0xFAFA0000 + (nr * 4); // error tag and vector OFFSET
        // Build stack frame for user trap handler
        // - Save callers PC
        m68k_areg(regs, 7) -= 4;      /* do not jump back into your own command */
        put_long(m68k_areg(regs, 7), oldpc+2); /* +2: is very important (bfo)   */
        // - save SR
        m68k_areg(regs, 7) -= 2;
        put_word(m68k_areg(regs, 7), regs.sr);
        // - save stack pointer
        m68k_areg(regs, 7) -= 4;
        put_long(m68k_areg(regs, 7), callers_stack);
        // save A6..A0
        for (i=6; i>=0;i--) {
            m68k_areg(regs, 7) -= 4;
            put_long(m68k_areg(regs, 7), m68k_areg(regs, i)); // save A6..A0 
        }
        // save D7..D0
        for (i=7; i>=0;i--) {
            m68k_areg(regs, 7) -= 4;
            put_long(m68k_areg(regs, 7), m68k_dreg(regs, i)); // save D7..D0 
        }
        // now modify registers according to requirements of error trap handler 
        m68k_areg(regs,5) = m68k_areg(regs,7); // A5 is pointer to register stack
        m68k_areg(regs,1) = callers_stack; // A1 is caller's stack
        m68k_dreg(regs,7) = nr * 4; // D7 is vector OFFSET
        // now let os9exec_nt do the rest (set A6/PC according to installed error trap handler)
        os9_running=0; // force exit from emulator loop
    };
}


/* enter OS9 context */
ulong llm_os9_go(regs_type *rp)
{
    ulong r;

    currentrp=rp; // save for use in exception handler
    memcpy(&regs,rp,sizeof(regs));
    regs.usp=rp->a[7];
    m68k_setpc(rp->pc); // make sure PC is set ok
    MakeFromSR(); // make sure condition flags are ok

    r=m68k_os9go();
    memcpy(rp,&regs,sizeof(regs));
    return r;
} /* llm_os9_go */


void llm_os9_copyback( regs_type *rp )
{
  memcpy( rp, &regs, sizeof(regs) );
} // Copy back registers in case of SEGV exception




#ifdef MACOS9
/* enter debugger with (most of) OS9 context in registers */
void llm_os9_debug( regs_type*, ConstStr255Param /* debuggerMsg */ )
{   upe_printf("# Using UAE-68k-emulator, can't go to debugger\n" );
} /* llm_os9_debug */
#endif


/* returns if FPU present */
Boolean llm_has_cache(void)
{   return needsflush;
} /* llm_fpu_present */


/* returns if FPU present */
Boolean llm_fpu_present(void)
{   return hasfpu;
} /* llm_fpu_present */


/* returns if virtual memory is enabled */
Boolean llm_vm_enabled(void)
{   return vmenabled;
} /* llm_fpu_present */


/* returns if 68k Mac Context runs in usermode */
/* (seems to be the case with newer G3/MacOS8.5 combinations) */
Boolean llm_runs_in_usermode(void)
{   return usesusermode;
} /* llm_runs_in_usermode */


/* prepare access to low-level code */
OSErr lowlevel_prepare(void)
{
    needsflush = false;
    hasfpu=true; // UAE has one? %%%
    fullcache=false; // no control over cache
    vmenabled=false; // no VM that might mess up UAE emulator
    usesusermode=false; // not of relevance with UAE
    /* now we are ready */
    debugprintf(dbgStartup,dbgDeep,("# lowlevel_prepare: Starting initialisation of UAE 68k emulator\n"));
    /* init UAE emulator */
    savestate_wanted = 0;
//  currprefs.cpu_level = 2; // 68020
    currprefs.cpu_level = 3; // 68020+68881
    init_m68k();
    compiler_init ();
    quit_program = 0; // no reset!
    /* now get info about usage of user mode */
    debugprintf(dbgStartup,dbgDeep,("# lowlevel_prepare: UAE 68k emulator initialized\n"));
    return(0);
} /* lowlevel_prepare */


/* uninstall low-level code */
void lowlevel_release(void)
{
} /* lowlevel_release */

/* eof */
