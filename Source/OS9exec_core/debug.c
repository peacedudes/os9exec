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
 *    Revision 1.29  2007/01/28 21:28:42  bfo
 *    <isPtoC> renamed to <isNative>
 *
 *    Revision 1.28  2007/01/07 13:07:32  bfo
 *    Adapted for the new memblock <mb> structure
 *
 *    Revision 1.27  2007/01/04 20:54:17  bfo
 *    Display memory size with 7 instead 6 digits
 *
 *    Revision 1.26  2006/11/18 09:58:16  bfo
 *    me!=NULL test added for <modsync> check
 *
 *    Revision 1.25  2006/11/12 13:15:56  bfo
 *    Some outdated comments eliminated
 *
 *    Revision 1.24  2006/11/06 22:59:32  MG
 *    size of allocated blocks will be displayed as well
 *
 *    Revision 1.23  2006/11/04 23:33:22  bfo/MG
 *    no more newline / stack remove problem (intProcName) fixed
 *
 *    Revision 1.22  2006/10/29 18:50:42  bfo
 *    Adapted texts for "-dh" output
 *
 *    Revision 1.21  2006/10/25 21:41:20  bfo
 *    div 0 will be masked with TRAP 5 instead of error 105
 *
 *    Revision 1.20  2006/10/25 19:29:29  bfo
 *    Suppress zerodiv logging and intutil disassembly
 *
 *    Revision 1.19  2006/10/13 10:24:45  MG
 *    "debug_procdump" (bus error reporting) added (by Martin Gregorie)
 *
 *    Revision 1.18  2006/10/01 15:25:23  bfo
 *    printf() for internal utilities, but not PtoC
 *
 *    Revision 1.17  2006/02/19 16:19:32  bfo
 *    use <isIntUtil>
 *
 *    Revision 1.16  2005/06/30 11:07:24  bfo
 *    Mach-O support
 *
 *    Revision 1.15  2005/01/22 15:47:40  bfo
 *    Renamed to ifdef MACOS9
 *
 *    Revision 1.14  2004/12/03 23:54:06  bfo
 *    MacOSX MACH adaptions
 *
 *    Revision 1.13  2004/11/27 12:01:31  bfo
 *    _XXX_ introduced
 *
 *    Revision 1.12  2004/11/20 11:44:06  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.11  2004/10/22 22:51:12  bfo
 *    Most of the "pragma unused" eliminated
 *
 *    Revision 1.10  2003/05/21 20:32:30  bfo
 *    Allocate 512k blocks / Additional parameter <mem_fulldisp> for "show_mem"
 *
 *    Revision 1.9  2003/05/17 10:43:38  bfo
 *    'show_mem' with <mem_unused> parameter
 *
 *    Revision 1.8  2003/04/20 23:00:37  bfo
 *    SpecialIO text: idbg for printing and networking
 *
 *    Revision 1.7  2003/04/12 21:48:24  bfo
 *    New codes SS_Send, SS_Recv, ... included
 *
 *    Revision 1.6  2002/09/22 20:46:48  bfo
 *    SS_206 (unused setstat call) included.
 *
 *    Revision 1.5  2002/07/30 16:47:19  bfo
 *    E-Mail adress bfo@synthesis.ch       is updated everywhere
 *
 *    Revision 1.4  2002/06/25 20:44:33  luz
 *    Added /lp printer support under windows. Not tested or even compiled for Mac
 *
 */

#include "os9exec_incl.h"
#include <ctype.h>

#ifdef USE_UAEMU
  #include "config.h"
  #include "options.h"
  #include "luzstuff.h"
  #include "memory.h"
  #include "readcpu.h"
  #include "newcpu.h"
  #include "compiler.h"
#endif

/* debugging support */
/* ================= */

/* Wrappers with correct variadic prototypes for passing to m68k_disasm.
   The dbg_func typedef is void(*)(void) which on arm64 causes the compiler
   to not pass variadic args through the pointer call; these wrappers carry
   the correct signature so va_start works inside the target printf. */
#ifdef USE_UAEMU
static void disasm_upe_out(const char *fmt, ...) {
    char buf[512]; va_list ap;
    va_start(ap, fmt); vsnprintf(buf, sizeof(buf), fmt, ap); va_end(ap);
    upe_printf("%s", buf);
}
static void disasm_upo_out(const char *fmt, ...) {
    char buf[512]; va_list ap;
    va_start(ap, fmt); vsnprintf(buf, sizeof(buf), fmt, ap); va_end(ap);
    upo_printf("%s", buf);
}
#endif

ushort debug[DEBUGLEVELS]= { 0,0,0 };   /* debug level array  */
ushort debughalt= 0;                    /* debug halt bitmask */

char triggername[TRIGNAMELEN] = "";     /* debug trigger name */

static ushort tempmask;
/* ---------------------------------------------------------- */

/* debug routines */
/* ============== */

#ifndef NODEBUG
Boolean debugcheck( ushort mask, ushort level )
/* debug condition check */
{
    tempmask=mask;
    return (debug[level & dbgLevelMask] & mask)!=0 && 
           (currentpid==0            || currentpid!= without_pid) &&
           (currentpid==justthis_pid || justthis_pid<1);
} /* debugcheck */
#endif

// check if address is outside process' allocated memory
Boolean out_of_mem( ushort pid, os9addr_t addr )
{
  uint32_t  base68k;
  pmem_typ* cm= &pmem[ pid ];

  ushort k;
  for  ( k=0; k<MAXMEMBLOCKS; k++ ) {
    if (cm->m[ k ].base &&
        (base68k= TO68K(cm->m[ k ].base)) <= addr &&
        addr < base68k + (uint32_t)cm->m[ k ].size) return false;
  } // for

  return true; // out of range
} // out_of_mem

// check if address is outside any OS9 module
Boolean out_of_mods( os9addr_t addr )
{
  ushort    k;
  mod_exec* mp;

  for (k=0; k<MAXMODULES; k++) {
        mp= os9mod( k );
    if (mp &&
        addr >= TO68K(mp) &&
        addr <  TO68K(mp) + os9_long( mp->_mh._msize )) return false;
  } // for

  return true; // out of range
} // out_of_mods

/* check for passing of bad register arguments */
#ifndef NODEBUG
void regcheck(ushort pid,char *nam,uint32_t reg,ushort mode)
{
    Boolean problem=false;
    
    if (debugcheck(dbgWarnings,dbgNorm)) {
        if (mode & RCHK_DRU) {
            /* check for unused data reg */
            if ((reg & 0xFFFFFFF0) == 0xDDDDDDD0) {
                problem=true;
                uphe_printf("regcheck: %s = $%08X seems to be uninitialized data reg (pid=%d)\n",nam,reg,pid);
            }
        }
        if (mode & RCHK_ARU) {
            /* check for unused addr reg */
            if ((reg & 0xFFFFFFF0) == 0xAAAAAAA0) {
                problem=true;
                uphe_printf("regcheck: %s = $%08X seems to be uninitialized address reg (pid=%d)\n",nam,reg,pid);
            }
        }
        if (mode & (RCHK_MEM+RCHK_MOD)) {
            /* check for violation of pointer target */ 
            problem=true;
            if (RCHK_MEM && !out_of_mem(pid,reg)) { problem=false; goto ok; }
            if (RCHK_MOD && !out_of_mods(reg)) { problem=false; goto ok; }
        ok:
            if (problem) uphe_printf("regcheck: %s = $%08X (pid=%d) is out of: %s %s\n",nam,reg,pid,mode & RCHK_MEM ? "[allocated memory]" : "",mode & RCHK_MOD ? "[all OS9 modules]" : "");
        }
        if (problem) {
            debug_halt(dbgWarnings);
        }
    }
} /* regcheck */
#endif

/* check for trigger name */
void trigcheck(char *message, char *name)
{
    if (triggername[0]!=0) {
        if (ustrncmp(name,triggername,strlen(triggername))==0) {
            /* triggered */
            uphe_printf("trigcheck [%s]: triggered to '%s'\n",message,name);
            debugwait();
        }
    }
} /* trigcheck */

/* debug printf */
#ifndef NODEBUG
void _debugprintf(char *format, ...)
{
    char buffer[MAXPRINTFLEN];
    process_typ* cp= &procs[ currentpid ];
    va_list vp;
    
    /* message enabled for that level */
    va_start    (vp,format);
    vsprintf(buffer,format,vp);
    va_end                (vp);
    
    /* "%s", buffer -- NOT buffer: it is already-formatted text, so any '%' that
     * survived into it (a filename, a module name, an error description) would
     * otherwise be read as a fresh specifier and consume varargs that were never
     * passed. The printf() branch just above always had this right. */
    if (cp->isIntUtil &&
       !cp->isNative) printf( "%s", buffer );
    else          upe_printf( "%s", buffer );

    /* look if also halt enabled for that class */
    debug_halt( tempmask );
} /* debugprintf */
#endif

#ifndef NODEBUG
  /* halt if enabled by debughalt */
  ushort debug_halt( ushort haltmask )
  {
      if (haltmask & debughalt) return debugwait();
      else                      return false;
  } /* debug_halt */
#endif

/* prepare debug leveling */
void debug_prep()
{
    #ifndef NODEBUG
      debug[dbgDetail] |= debug[dbgDeep];   /* deep detail also enables detail */
      debug[dbgNorm]   |= debug[dbgDetail]; /* detail also enables normal */
    #endif
} /* debug_prep */

/* Dump the process descriptor */
void debug_procdump( process_typ* cp, int cpid )
{
   char                       *code    = NULL;
   char                       *desc    = NULL;
   mod_exec                   *me      = get_module_ptr( cp->mid );
   ushort                     sync_id  = os9_word( 0x4afc );
   ushort                     modsync  = 0;
   Boolean                    sync_ok  = false;
   static int                 depth    = 0;
   
   int                        i;
   int                        j;
   int                        up;
   char                       *prefix  = "";
   syspath_typ                *spP;
   char                       *typename = "";
   char                       filename[OS9PATHLEN];
   char                       devname[OS9NAMELEN];
   char                       modelist[9];
   const funcdispatch_entry*  fdeP = getfuncentry(cp->lastsyscall);
   regs_type*                 rp;
   memblock_typ*              mb;
   errortrap_typ*             et;
   traphandler_typ*           th;
   mod_exec*                  tme;
   pmem_typ*                  cm= &pmem[ cpid ];

   /* trap segfaults within this function to avoid looping*/
   if (++depth > 1) {
      upo_printf("*** BUSERR during process dump: Emulation terminated\n");
      exit(0);
   } // if

   /* check that the sync bytes are $4afc */
   if (me!=NULL) {
     modsync=  me->_mh._msync;
     sync_ok= (modsync == sync_id);
   } // if
   
   /* output the module identity line */
   upo_printf("\nProcess   Pid: %d", cpid);
   if           (strcmp(cp->intProcName,"")!=0)
      upo_printf(" %s", cp->intProcName);
   else {
      if (sync_ok)
         upo_printf(", %s, edition %d",
                    Mod_Name(me),
                    os9_word(me->_mh._medit));
      else
         upo_printf(" Invalid sync bytes: corrupted module?");
   }

   if (cp->isIntUtil)
      upo_printf(" (internal)");

   upo_printf("\n");        /* End of the module identity line */

   /* Output the exit code */
   get_error_strings(cp->exiterr, &code, &desc);
   upo_printf("    Exit code: %s(%d) %s\n", code, cp->exiterr, desc);

   /* Show current and execution directories */
   upo_printf("  Directories: Current   - %s\n", cp->d.path);
   upo_printf("               Execution - %s\n", cp->x.path);

   /* List open files */
   prefix = "Files:";
   for (i = 0; i < MAXUSRPATHS; i++) {
      up = cp->usrpaths[i];
      if (up) {
         spP = &syspaths[up];
         typename = spP_TypeStr(spP);
         syspath_gs_devnm( cpid, spP->nr, devname );
         strcpy(filename, spP->name);         
         if (ustrcmp(filename, devname) !=0) {
            if (*filename == NUL)
               strcpy(filename, "''");
         }
         else
               strcpy(filename, "");
    
         upo_printf("        %6s %02d %-5s /%s",
                    prefix,
                    i,
                    typename,
                    devname);
         if (strlen(filename))
            upo_printf(" -> %s", filename);

         strcpy(modelist, "");
         if (spP->mode & 0x80) strcat(modelist, "d");
         if (spP->mode & 0x40) strcat(modelist, "n");
         if (spP->mode & 0x20) strcat(modelist, "i");
         if (spP->mode & 0x10) strcat(modelist, "4");
         if (spP->mode & 0x08) strcat(modelist, "3");
         if (spP->mode & 0x04) strcat(modelist, "e");
         if (spP->mode & 0x02) strcat(modelist, "w");
         if (spP->mode & 0x01) strcat(modelist, "r");
         if (strlen(modelist))
            upo_printf(" [%s]", modelist);

         upo_printf("\n");
      }

      prefix = "";
   }

   /* Report the last system call */
   upo_printf(" Last syscall: %s (0x%04x)\n", fdeP->name, cp->lastsyscall);

   /*
      Dump registers and failing instruction
      only if this isn't an internal command
   */
   if (!cp->isIntUtil) {
      /* Dump the registers */
      rp = &(cp->os9regs);
      for (i = 0; i < 2; i++) {
         if (i == 0)
            upo_printf("    Registers: Dn=");
         else
            upo_printf("                  ");
         
         for (j = 0; j < 4; j++)
            upo_printf("%08X ", rp->d[i * 4 + j]);

         upo_printf("\n");
      }

      for (i = 0; i < 2; i++) {
         if (i == 0) 
            upo_printf("               An=");
         else
            upo_printf("                  ");
         
         for (j = 0 ;j < 4; j++)
            upo_printf("%08X ", rp->a[i * 4 + j]);

         upo_printf("\n");
      }
   
      upo_printf("               PC=%08X SR=%04X\n", rp->pc, rp->sr);

      /* Show the failing instruction. */
      #ifdef USE_UAEMU
      {  uaecptr aa;
         regs.pc = rp->pc;
         regs.pc_p = regs.pc_oldp = get_real_address_safe(rp->pc);
         upo_printf(" Executing: -->");
         m68k_disasm(rp->pc, &aa, 1, disasm_upo_out);
         upo_printf("               ");
         m68k_disasm(aa,     &aa, 1, disasm_upo_out);
      }
      #endif
   }

   /* Static memory */
   upo_printf("       Memory: Static    -     %08X - %08X %7u bytes\n",
          cp->memstart,
          cp->memtop,
          (cp->memtop - cp->memstart));

   /* List allocated memory segments */
   prefix = "Allocated -";
   for (i = 0; i < MAXMEMBLOCKS; i++) {
       // mb= &cp->os9memblocks[i];
          mb= &cm->m[ i ];
      if (mb->base != 0) {
         upo_printf("              %12s %03d %08X - %08X %7lu bytes\n",
                    prefix,
                    i,
                    TO68K(mb->base),
                    TO68K(mb->base) + (uint32_t)mb->size,
                    (unsigned long)mb->size);
         prefix = "";
      }
   }

   /* List error traps */
   prefix = "Error trap:";
   for (i = 0; i < NUMEXCEPTIONS; i++) {
      et = &cp->ErrorTraps[i];
      if (et->handleraddr != 0) {
         upo_printf("   %11s #%01d addr=%08X stack=%08X\n",
                    prefix,
                    i + 2,
                    et->handleraddr,
                    et->handlerstack);
         prefix = "";
      }
   }

   /* Trap handlers */
   prefix = "Trap handler:";
   for (i = 0; i < NUMTRAPHANDLERS; i++) {
      th = &cp->TrapHandlers[i];
      tme = &th->trapmodule->progmod;
      if (tme != 0) {
         upo_printf(" %13s %02d #%02d %s, edition %d\n",
                    prefix,
                    i + 1,
                    i + 32,
                    Mod_Name(tme),
                    os9_word(tme->_mh._medit));
         prefix = "";
      }
   }
      
   /* Clear the segfault trap */
   depth = 0;
}		    

os9err debug_help( ushort pid, _argc_, _argv_ )
/* display debug help, allow internal and external access */
{
    upho_printf("Debug and Stop masks:\n");
    upho_printf("\n");
    upho_printf("  dbgAnomaly      0x0001  anomalies: exceptions and unimplemented/dummy system calls\n");
    upho_printf("  dbgSysCall      0x0002  call and return status of system calls\n");
    upho_printf("  dbgFMgrType     0x0004  the file manager type\n");
    upho_printf("  dbgProcess      0x0008  process creation/killing\n");
    upho_printf("  dbgTaskSwitch   0x0010  task switching/arbitration\n");
    upho_printf("  dbgModules      0x0020  module link/load/unlink\n");
    upho_printf("  dbgMemory       0x0040  memory allocation/deallocation\n");
    upho_printf("  dbgUtils        0x0080  internal utilities\n");
    upho_printf("  dbgTrapHandler  0x0100  trap handlers\n");
    upho_printf("  dbgFiles        0x0200  file operations\n");
    upho_printf("  dbgErrors       0x0400  error generation/translation\n");
    upho_printf("  dbgStartup      0x0800  os9exec/nt startup and initial argument preparation\n");
    upho_printf("  dbgPartial      0x1000  warnings for inaccurate/partial emulated functions\n");
    upho_printf("  dbgTerminal     0x2000  interactive I/O, output filtering\n");
    upho_printf("  dbgSpecialIO    0x4000  special I/O, such as printer and network\n");
    upho_printf("  dbgWarnings     0x8000  warnings\n");
    upho_printf("\n");
    upho_printf("Debug levels\n");
    upho_printf("  dbgNorm         0       normal   debug information\n");
    upho_printf("  dbgDetail       1       detailed debug information\n");
    upho_printf("  dbgDeep         2       deep   insider information\n");

    if (pid==0) upo_printf("\n");
    return 0; /* not really used, but same proc definition as int cmds */
} /* debug_help */

/* show registers */
void dumpregs(ushort pid)
{
   int k;
    regs_type *rp;

    #ifdef USE_UAEMU
      /* Staging copy, rather than casting &regs straight to regs_type*.
       * `regs` is UAE's `struct regstruct`, which regs_type EXTENDS: the two
       * share a layout through the last common field (pinned by the
       * _Static_asserts in os9_uae.c) and regs_type then adds an os9exec-only
       * ttP/membase tail that no regstruct object has. Casting the smaller
       * object to the larger type is fine only for as long as every read stays
       * inside the common prefix -- true today (d/a/pc/sr below), but a future
       * line touching rp->ttP would read past the end of `regs` with nothing
       * to warn about it. Copying exactly sizeof(struct regstruct) into a real
       * regs_type, the same bounded-copy shape llm_os9_go already uses, makes
       * the tail well-defined instead of out of bounds.
       * (GCC -fanalyzer, -Wanalyzer-allocation-size.) */
      regs_type uaeStage;
    #endif

    if (pid>=MAXPROCESSES) {
        #ifdef USE_UAEMU
          memset( &uaeStage, 0, sizeof(uaeStage) );
          memcpy( &uaeStage, &regs, sizeof(struct regstruct) );
          rp= &uaeStage; /* UAE */
          uphe_printf("UAE current Register Dump:\n");
        #else
          uphe_printf("No current process to show regs for\n");
          return;
        #endif
    }
    else {
        rp= &procs[pid].os9regs;
        uphe_printf("OS-9 Register Dump for pid=%d:\n",pid);
    }
   
    uphe_printf(" Dn="); for (k=0;k<8;k++) upe_printf("%08X ",rp->d[k]); upe_printf("\n");
    uphe_printf(" An="); for (k=0;k<8;k++) upe_printf("%08X ",rp->a[k]); upe_printf("\n");
    uphe_printf(" PC=%08X SR=%04X\n",rp->pc,rp->sr);

    #ifdef USE_UAEMU
    if (pid < MAXPROCESSES && !procs[pid].isIntUtil) {
        uaecptr aa;
        regs.pc = rp->pc;
        regs.pc_p = regs.pc_oldp = get_real_address_safe(rp->pc);
        m68k_disasm(rp->pc, &aa, 2, disasm_upe_out);
    }
    #endif
} /* dumpregs */

/* show memory */
static void dumpmem(uint32_t *memptrP,int numlines)
{
    int k,i;

    for (k=0; k<numlines; k++) {
        uphe_printf("%08X: ",*memptrP);
        for (i=0;i<16;i++) {
            byte* hp= (byte*)FROM68K(*memptrP+i);
            upe_printf("%02X ", hp ? *hp : 0xEE);
        }
        upe_printf(" ");
        for (i=0;i<16;i++) {
            byte* hp= (byte*)FROM68K(*memptrP+i);
            char c= hp ? *hp : '?';
            upe_printf("%c",isprint((unsigned char)c) ? c : '.');
        }
        upe_printf("\n");
        (*memptrP)+=16;
    }
} /* dumpmem */

/* show regs in debugger — called only from MACOS9 context */
#ifdef MACOS9
static void regs_in_debugger( regs_type *rp )
{
    #ifdef MACOS9
      Str255 message;
    
      sprintf( &message[1], "%s called debugger, OS9 PC=%08X, OS9 A7=%08X\n",
                             OS9exec_Name(), rp->pc, rp->a[7]);
      message[0]=strlen(&message[1]);
      llm_os9_debug( rp, message );

    #else
      #ifndef __GNUC__ /* MPW-only pragma; GCC warns it is ignoring it. Gate on the COMPILER, not the OS -- "not linux" wrongly includes mingw. */
      #pragma unused(rp)
      #endif
    
      uphe_printf("Non-Macintosh: No low level debugger\n");
    #endif
} /* regs_in_debugger */
#endif /* MACOS9 */

#ifdef USE_UAEMU
extern int m68k_os9trace;
#endif

static uint32_t listbase=0;
static int disasm=0;

/* Returns true and prints an error if addr is outside the 68k arena. */
static Boolean bad_addr(uint32_t addr)
{
    if (emul_base + (uae_u32)addr >= emul_end) {
        upe_printf("Address $%08X is outside the OS-9 memory space\n", addr);
        return true;
    }
    return false;
}

/* True if the 68k opcode at addr is an unconditional flow terminator —
 * RTS/RTD/RTE/RTR, unconditional BRA, JMP, the ILLEGAL instruction, or an
 * A/F-line trap.  Execution never falls through to the next sequential byte
 * after any of these, so disassembling past them produces garbage output. */
static Boolean is_flow_terminator(uint32_t addr)
{
    if (emul_base + (uae_u32)addr + 1 >= emul_end) return true;
    uae_u8 *p  = get_real_address_safe(addr);
    uint16_t op = ((uint16_t)p[0] << 8) | p[1];
    switch (op) {
        case 0x4E73: case 0x4E74: case 0x4E75: case 0x4E77: /* RTE RTD RTS RTR */
        case 0x4AFC:                                          /* ILLEGAL */
            return true;
    }
    uint8_t hi = (uint8_t)(op >> 8);
    if (hi == 0x60)               return true;  /* BRA (all displacements) */
    if ((op & 0xFFC0) == 0x4EC0)  return true;  /* JMP (all EA modes) */
    if (hi >= 0xA0 && hi <= 0xAF) return true;  /* A-line (always illegal here) */
    if (hi >= 0xF0)               return true;  /* F-line (always illegal here) */
    return false;
}

/* wait for debug confirmation */
ushort debugwait( void )
{
    #ifdef TERMINAL_CONSOLE
      char *cp;
    #endif

    #define    INPLEN 100
    char   inp[INPLEN];
    ushort temp,temp2;
    int    numitems;
    ushort extra= false;
//  ushort k= 0;  /* was used by commented-out Change_DbgPath call */
    
    #ifdef USE_UAEMU
      m68k_os9trace=false;
    #endif
    
    do {
        uphe_printf("Pid=%d%s: dbgmsk=$%04X,$%04X,$%04X stop=$%04X trigger='%s' (type ?<Enter> for hlp)\n",
                         currentpid, currentpid>=MAXPROCESSES ? " **NONE**" : "", debug[0],debug[1],debug[2], debughalt, triggername);
        clearerr(stdin); /* to make sure we don't get into an endless loop */
        
        #if defined(TERMINAL_CONSOLE) && defined(CON_SUPPORT)
          cp= inp;
          do {
              /* clearerr resets the EOF flag getchar() sets on each VTIME timeout,
               * allowing read() to be called again on the next ConsGetc iteration. */
              do { clearerr(stdin); ConsGetc(cp);
              } while (!devIsReady);
              ConsPutcEdit(*cp, true,CR); /* do echo */
              if          (*cp!=CR) cp++;
          } while         (*cp!=CR);
             *cp= NUL; /* string termination */
          if (cp==inp) continue;
        #else
          /* Non-CON_SUPPORT path: read() directly since stdio is unreliable here. */
          { int  n;
            char rc;
            cp= inp;
            do {
                n= read(0, &rc, 1);
                if (n==0) continue;
                if (n<0) break;
                if (rc=='\r' || rc=='\n') { write(1,"\r\n",2); break; }
                if ((rc==0x7f || rc=='\b') && cp>inp) { cp--; write(1,"\b \b",3); continue; }
                if (rc < 0x20) continue;
                *cp++= rc;
                write(1,&rc,1);
            } while (cp < inp+INPLEN-1);
            *cp= NUL;
            if (cp==inp) continue;
          }
        #endif
        
        switch (tolower((unsigned char)inp[0])) {
            case 'q' : stop_os9exec(); /* --- and never come back */
                                           
            case 'n' : if (inp[1]>0x20) {
                           strncpy(triggername,&inp[1],TRIGNAMELEN);
                                   triggername[strlen(triggername)-1]=0; /* cut CR away */
                       }
                       else triggername[0]=0;
                       
            case 'r' : if (tolower((unsigned char)inp[1])=='u') temp=MAXPROCESSES;
                       else if (sscanf(&inp[1],"%hu", &temp)<1) temp= currentpid;
                       dumpregs    ( temp ); break;
                       
            case 'v' : if      (sscanf(&inp[1],"%hu", &temp)<1) temp= MAXPROCESSES;
                       show_mem    ( temp, false,false ); break;
                       
            case 'p' : show_processes();     break;

            case 'f' : if      (sscanf(&inp[1],"%hu", &temp)<1) temp= MAXPROCESSES;
                       show_files  ( temp ); break;

            case 'm' : show_modules( NULL ); break;

            case 'k' : if      (sscanf(&inp[1],"%hu", &temp)<1) temp= currentpid;
                       procs[temp].exiterr=E_PRCABT;
                       kill_process(temp);
                       break;
                                         
            #ifdef MACOS9
              case 'b': if (tolower((unsigned char)inp[1])=='b') {
                            Debugger();
                            break;
                        }
                         
                        if (sscanf(&inp[1],"%hu", &temp)<1) {
                            temp=currentpid;
                        }
                         
                        regs_in_debugger(&procs[temp].os9regs); break;

              #ifndef USE_UAEMU
                case 't': Debugger(); goto goon;
              #endif
            #endif

            case 'd' : numitems=sscanf(&inp[1],"%hx,%hx", &temp,&temp2);
                          if (numitems<1) {
                              debug_help( 0,0,NULL ); /* show help */
                              break;
                          }
                          else if (numitems<2) {
                              temp2= temp; /* take first argument as debugmask */
                              temp = 0; /* default to level 0 */
                          }

                          if   (temp>=DEBUGLEVELS) temp=0;
                          debug[temp]=temp2;
                          debug_prep(); /* adjust debug masks */
                          break;

            case 's' : if (sscanf(&inp[1],"%hx", &debughalt)<1) {
                                uphe_printf("Error in hex number\n");
                          }
                          break;
            case 'g' :
            case 'c' : goto goon;
            case 'x' : extra=true; goto goon;

            #ifdef USE_UAEMU
              case 'i' : if (sscanf(&inp[1],"%x",&listbase)<1) {
                             if (currentpid<MAXPROCESSES && !procs[currentpid].isIntUtil)
                                 listbase= procs[currentpid].os9regs.pc;
                             else { upe_printf("No process PC available\n"); break; }
                         }
                         if (bad_addr(listbase)) break;
                         { int n; Boolean hit_term = false;
                           for (n = 0; n < 10; n++) {
                               Boolean term = is_flow_terminator(listbase);
                               regs.pc = listbase;
                               regs.pc_p = regs.pc_oldp = get_real_address_safe(listbase);
                               m68k_disasm(listbase,(uaecptr*)&listbase,1,disasm_upe_out);
                               if (term) { hit_term = true; break; }
                               if (bad_addr(listbase)) break;
                           }
                           if (hit_term && emul_base + (uae_u32)listbase < emul_end) {
                               upe_printf("# (flow ends — bytes that follow, not necessarily code:)\n");
                               uint32_t peek = listbase;
                               dumpmem(&peek, 2);
                           }
                         }
                         disasm=1;
                         break;

              case '.' : if (disasm) {
                             int n; Boolean hit_term = false;
                             for (n = 0; n < 10; n++) {
                                 Boolean term = is_flow_terminator(listbase);
                                 regs.pc = listbase;
                                 regs.pc_p = regs.pc_oldp = get_real_address_safe(listbase);
                                 m68k_disasm(listbase,(uaecptr*)&listbase,1,disasm_upe_out);
                                 if (term) { hit_term = true; break; }
                                 if (bad_addr(listbase)) break;
                             }
                             if (hit_term && emul_base + (uae_u32)listbase < emul_end) {
                                 upe_printf("# (flow ends — bytes that follow, not necessarily code:)\n");
                                 uint32_t peek = listbase;
                                 dumpmem(&peek, 2);
                             }
                         } else
                             dumpmem(&listbase,10);
                         break;
                         
              case 'e' : m68k_dumpstate( (uaecptr*)&listbase,false ); break;
              case 't' : m68k_os9trace=true; goto goon; /* %%% make this one better */

            #else
              case '.' : dumpmem(&listbase,8); break;
            #endif

            case 'l' : if (sscanf(&inp[1],"%x", &listbase)<1) {
                            #ifdef USE_UAEMU
                                listbase=m68k_areg(regs,7);
                            #else
                                listbase=procs[currentpid].os9regs.a[7];
                                #endif
                          }
                          if (bad_addr(listbase)) break;
                          disasm=0;
                          dumpmem(&listbase,8); 
                          break;
                          
          //case 'o' : Change_DbgPath( 0, NULL, &inp[ 1 ], &k ); break;
            
            case 'o' : if (sscanf(&inp[1],"%d",  &dbgOut)     <1) {  dbgOut     = 0; }; break;
            case 'w' : if (sscanf(&inp[1],"%d",  &without_pid)<1) {  without_pid= 0; }; break;
            case 'j' : if (sscanf(&inp[1],"%d", &justthis_pid)<1) { justthis_pid= 0; }; break;
            
            default  :
                          /* MACOS9, not macintosh: the [B]/[T] commands this text
                           * advertises are gated on MACOS9 (the case 'b'/'t'
                           * handlers above), which is never defined in a modern
                           * build -- but `macintosh` IS defined on live macOS, so
                           * the help used to promise B/T while pressing them just
                           * re-printed this help. Match the gate to the handlers. */
                          #ifdef MACOS9
                            upe_printf("[X]extra-[G]o, [T]continue in debugger (mac context), [K[xx]] Kill process [xx], [Q]uit emulation\n");
                            upe_printf("[B[xx|B]] Call MacsBug with OS9 regs [of pid=xx]/[B] directly, [R[xx]] Regs [of pid=xx]\n");
                          #else
                            upe_printf("[X]tra-[G]o, [K[xx]] Kill proc[xx], [Q]uit emulation\n");
                            upe_printf("[R[U][xx]] Regs[of pid=xx][of UAE]\n");
                          #endif

                          upe_printf("[L[xxx]] list 8 memory lines from xxx, default=A7, [.] continue list/disasm\n");

                          #ifdef USE_UAEMU
                            upe_printf("[I[xxx]] disassemble 10 instructions from xxx, default=PC\n");
                          #endif

                          upe_printf("[D]bg hlp, [D[y,]xx]] set dbg mask=xx (of level=y, def=0), [Sx] set stop mask=x\n");
                          upe_printf("[M]odules, [P]rocs, [F[xx]]iles [of pid=xx], [V[xx]] memory [of pid=xx]\n");
                          upe_printf("[O]ut (dbg hlp system path), [W]ithout pid dbg\n");
                          break;
        }
    } while (true);
    
goon:
    return extra;
} /* debugwait */

/* show one reg in specified length */
void showonereg(uint32_t value, Boolean isa, ushort regnum, ushort lenspec)
{
    char        *format;
    static char tmp[OS9PATHLEN]; /* static buffer to allow path display in error tracebacks */

    lenspec &= 0x03;
    if (lenspec) {
        if (!isa) {
            /* data register */
            switch (lenspec) {
                case 1 : format="D%d.b=$%02X "; value &= 0xFF;   break;
                case 2 : format="D%d.w=$%04X "; value &= 0xFFFF; break;
                /* lenspec is masked to 0..3 and 0 is excluded above, so this is
                   the case-3 long form -- but spell it `default` so `format` is
                   assigned on every path. As a bare `case 3` the compiler could
                   not prove the switch total, and any later change to the mask
                   or the guard above would hand printf an indeterminate char*. */
                default: format="D%d.l=$%X "  ;                  break;
            }
            upe_printf(format,     regnum,value);
        }
        else {
            /* address register */
            upe_printf("A%d=$%X ",regnum,value);
            if (lenspec==2) { /* not all incoming strings are null terminated */
                nullterm( (char*)&tmp,(char*)FROM68K(value), OS9PATHLEN );
                upe_printf("\"%s\" ", tmp );
            }
        }
    }
} /* showonereg */

/* show multiple regs according to bitmask */
void show_maskedregs(regs_type *rp, uint32_t regmask)
{
    ushort k;
    
    if (regmask & SEVENT_CALL) {
        upe_printf("%s ",  get_ev_name(loword(rp->d[1])));
    }
    if (regmask & SFUNC_STATCALL) {
        upe_printf("%s ",get_stat_name(loword(rp->d[1])));
    }
    
    regmask &= REGMASK_REGBITS;
    for (k=0; k<8; k++) {
        /* show data regs */
        showonereg(rp->d[k],false,k,regmask & 0x03);
        regmask >>= 2;
    }
    for (k=0; k<7; k++) {
        /* show address regs */
        showonereg(rp->a[k],true, k,regmask & 0x03);
        regmask >>= 2;
    }
} /* show_maskedregs */

/* setstat/getstat names for debugging */
char* get_stat_name(ushort stat)
{
    char *name;
    
    switch (stat) {
        case SS_Opt:     name="SS_Opt";        break;
        case SS_Ready:   name="SS_Ready";      break;
        case SS_Size:    name="SS_Size";       break;
        case SS_Reset:   name="SS_Reset";      break;
        case SS_WTrk:    name="SS_WTrk";       break;
        case SS_Pos:     name="SS_Pos";        break;
        case SS_EOF:     name="SS_EOF";        break;
        case SS_Link:    name="SS_Link";       break;
        case SS_ULink:   name="SS_ULink";      break;
        case SS_Feed:    name="SS_Feed";       break;
        case SS_Frz:     name="SS_Frz";        break;
        case SS_SPT:     name="SS_SPT";        break;
        case SS_SQD:     name="SS_SQD";        break;
        case SS_DCmd:    name="SS_DCmd";       break;
        case SS_DevNm:   name="SS_DevNm";      break;
        case SS_FD:      name="SS_FD";         break;
        case SS_Ticks:   name="SS_Ticks";      break;
        case SS_Lock:    name="SS_Lock";       break;
        case SS_DStat:   name="SS_DStat";      break;
        case SS_Joy:     name="SS_Joy";        break;
        case SS_BlkRd:   name="SS_BlkRd";      break;
        case SS_BlkWr:   name="SS_BlkWr";      break;
        case SS_Reten:   name="SS_Reten";      break;
        case SS_WFM:     name="SS_WFM";        break;
        case SS_RFM:     name="SS_RFM";        break;
        case SS_ELog:    name="SS_ELog";       break;
        case SS_SSig:    name="SS_SSig";       break;
        case SS_Relea:   name="SS_Relea";      break;
        case SS_Attr:    name="SS_Attr";       break;
        case SS_Break:   name="SS_Break";      break;
        case SS_RsBit:   name="SS_RsBit";      break;
        case SS_RMS:     name="SS_RMS";        break;
        case SS_FDInf:   name="SS_FDInf";      break;
        case SS_ACRTC:   name="SS_ACRTC";      break;
        case SS_IFC:     name="SS_IFC";        break;
        case SS_OFC:     name="SS_OFC";        break;
        case SS_EnRTS:   name="SS_EnRTS";      break;
        case SS_DsRTS:   name="SS_DsRTS";      break;
        case SS_DCOn:    name="SS_DCOn";       break;
        case SS_DCOff:   name="SS_DCOff";      break;
        case SS_Skip:    name="SS_Skip";       break;
        case SS_Mode:    name="SS_Mode";       break;
        case SS_Open:    name="SS_Open";       break;
        case SS_Close:   name="SS_Close";      break;
        case SS_Path:    name="SS_Path";       break;
        case SS_Play:    name="SS_Play";       break;
        case SS_HEADER:  name="SS_HEADER";     break;
        case SS_Raw:     name="SS_Raw";        break;
        case SS_Seek:    name="SS_Seek";       break;
        case SS_Abort:   name="SS_Abort";      break;
        case SS_CDDA:    name="SS_CDDA";       break;
        case SS_Pause:   name="SS_Pause";      break;
        case SS_Eject:   name="SS_Eject";      break;
        case SS_Mount:   name="SS_Mount";      break;
        case SS_Stop:    name="SS_Stop";       break;
        case SS_Cont:    name="SS_Cont";       break;
        case SS_Disable: name="SS_Disable";    break;
        case SS_Enable:  name="SS_Enable";     break;
        case SS_ReadToc: name="SS_ReadToc";    break;
        case SS_SM:      name="SS_SM";         break;
        case SS_SD:      name="SS_SD";         break;
        case SS_SC:      name="SS_SC";         break;
        case SS_SEvent:  name="SS_SEvent";     break;
        case SS_Sound:   name="SS_Sound";      break;
        case SS_DSize:   name="SS_DSize";      break;
        case SS_Net:     name="SS_Net";        break;
        case SS_Rename:  name="SS_Rename";     break;
        case SS_Free:    name="SS_Free";       break;
        case SS_Bind:    name="SS_Bind";       break;
        case SS_Listen:  name="SS_Listen";     break;
        case SS_Connect: name="SS_Connect";    break;
        case SS_Resv:    name="SS_Resv";       break;
        case SS_Accept:  name="SS_Accept";     break;
        case SS_Recv:    name="SS_Recv";       break;
        case SS_Send:    name="SS_Send";       break;
        case SS_GNam:    name="SS_GNam";       break;
        case SS_SOpt:    name="SS_SOpt";       break;
        case SS_SendTo:  name="SS_SendTo";     break;
        case SS_PCmd:    name="SS_PCmd";       break;
        
        case SS_201:     name="SS_201";        break;
        case SS_204:     name="SS_204";        break;
        case SS_206:     name="SS_206";        break;
        case SS_Etc:     name="SS_Etc";        break;
        default:         name="<<<unknown>>>"; break;
    } /* switch */
    return name;
} /* get_stat_name */

char* get_ev_name(ushort ev)
{
    char *name;
    
    switch (ev) {
        case Ev_Link:    name="Ev_Link";       break;
        case Ev_UnLnk:   name="Ev_UnLnk";      break;
        case Ev_Creat:   name="Ev_Creat";      break;
        case Ev_Delet:   name="Ev_Delet";      break;
        case Ev_Wait:    name="Ev_Wait";       break;
        case Ev_WaitR:   name="Ev_WaitR";      break;
        case Ev_Read:    name="Ev_Read";       break;
        case Ev_Info:    name="Ev_Info";       break;
        case Ev_Pulse:   name="Ev_Pulse";      break;
        case Ev_Signl:   name="Ev_Signl";      break;
        case Ev_Set:     name="Ev_Set";        break;
        case Ev_SetR:    name="Ev_SetR";       break;
        default:         name="<<<unknown>>>"; break;
    } /* switch */
    
    return name;
} /* get_ev_name */

/* error names for debugging and F$PErr */
void get_error_strings(os9err err, char **nameP, char **descP)
{   
    char *name,*desc;
        
    switch (err) {
        case 0          : name="<none>";    desc=""; break;
        case S_Wake     : name="S_Wake";    desc="wake"; break;
        case S_Abort    : name="S_Abort";   desc="User abort (Cmd-'.')"; break;
        case S_Intrpt   : name="S_Intrpt";  desc="User interrupt"; break;
        case S_HangUp   : name="S_HangUp";  desc="modem hangup"; break;
        case S_Activate : name="S_Active";  desc="activate"; break;
        case E_ILLFNC   : name="E_ILLFNC";  desc="illegal function code"; break;
        case E_FMTERR   : name="E_FMTERR";  desc="format error"; break;
        case E_NOTNUM   : name="E_NOTNUM";  desc="number not found/not a number"; break;
        case E_ILLARG   : name="E_ILLARG";  desc="illegal argument"; break;
        
        case E_BUSERR   : name="E_BUSERR";  desc="bus error TRAP 2 occurred"; break;
        case E_ADRERR   : name="E_ADRERR";  desc="address error TRAP 3 occurred"; break;
        case E_ILLINS   : name="E_ILLINS";  desc="illegal instruction TRAP 4 occurred"; break;
        case E_ZERDIV   : name="E_ZERDIV";  desc="zero divide TRAP 5 occurred"; break;
        case E_CHK      : name="E_CHK";     desc="CHK instruction TRAP 6 occurred"; break;
        case E_TRAPV    : name="E_TRAPV";   desc="TrapV instruction TRAP 7 occurred"; break;
        case E_VIOLAT   : name="E_VIOLAT";  desc="privelage violation TRAP 8 occurred"; break;
        case E_TRACE    : name="E_TRACE";   desc="Uninitialized Trace TRAP 9 occurred"; break;
        case E_1010     : name="E_1010";    desc="Uninitialized 1010 TRAP 10 occurred"; break;
        case E_1111     : name="E_1111";    desc="Uninitialized 1111 TRAP 11 occurred"; break;
        case E_TRAP     : name="E_TRAP";    desc="user traps 1-15 E_TRAP+n"; break;
        case E_FPUNORDC : name="E_FPUNORDC";desc="FP unordered condition error"; break;
        case E_FPINXACT : name="E_FPINXACT";desc="FP inexact result"; break;
        case E_FPDIVZER : name="E_FPDIVZER";desc="FP zero divisor"; break;
        case E_FPUNDRFL : name="E_FPUNDRFL";desc="FP underflow"; break;
        case E_FPOPRERR : name="E_FPOPRERR";desc="FP operand error"; break;
        case E_FPOVERFL : name="E_FPOVERFL";desc="FP overflow"; break;
        case E_FPNOTNUM : name="E_FPNOTNUM";desc="FP not a number"; break;
        
        case E_PERMIT   : name="E_PERMIT";  desc="You must be super user to do that"; break;
        case E_DIFFER   : name="E_DIFFER";  desc="Arguments to F$ChkNam are different"; break;
        case E_STKOVF   : name="E_STKOVF";  desc="Stack overflow (ChkNam)"; break;
        case E_EVNTID   : name="E_EVNTID";  desc="Invalid or Illegal event ID number "; break;
        case E_EVNF     : name="E_EVNF";    desc="Event (name) not found"; break;
        case E_EVBUSY   : name="E_EVBUSY";  desc="The event is busy (can't be deleted) "; break;
        case E_EVPARM   : name="E_EVPARM";  desc="Impossible Event parameters supplied "; break;
        case E_DAMAGE   : name="E_DAMAGE";  desc="System data structures are damaged "; break;
        case E_BADREV   : name="E_BADREV";  desc="Incompatible revision"; break;
        case E_PTHLOST  : name="E_PTHLOST"; desc="Path Lost (net node was down)"; break;
        case E_BADPART  : name="E_BADPART"; desc="Bad partition data or no active"; break;
        case E_HARDWARE : name="E_HARDWARE";desc="Hardware is damaged"; break;
        
        case E_PTHFUL   : name="E_PTHFUL";  desc="Path Table full"; break;
        case E_BPNUM    : name="E_BPNUM";   desc="Bad Path Number"; break;
        case E_POLL     : name="E_POLL";    desc="Polling Table Full"; break;
        case E_BMODE    : name="E_BMODE";   desc="Bad Mode"; break;
        case E_DEVOVF   : name="E_DEVOVF";  desc="Device Table Overflow"; break;
        case E_BMID     : name="E_BMID";    desc="Bad Module ID"; break;
        case E_DIRFUL   : name="E_DIRFUL";  desc="Module Directory Full"; break;
        case E_MEMFUL   : name="E_MEMFUL";  desc="Process Memory Full"; break;
        case E_UNKSVC   : name="E_UNKSVC";  desc="Unknown Service Code"; break;
        case E_MODBSY   : name="E_MODBSY";  desc="Module Busy"; break;
        case E_BPADDR   : name="E_BPADDR";  desc="Bad Page Address"; break;
        case E_EOF      : name="E_EOF";     desc="End of File"; break;
        case E_VCTBSY   : name="E_VCTBSY";  desc="IRQ Vector is already used"; break;
        case E_NES      : name="E_NES";     desc="Non-Existing Segment"; break;
        case E_FNA      : name="E_FNA";     desc="File Not Accessible"; break;
        case E_BPNAM    : name="E_BPNAM";   desc="Bad Path Name"; break;
        case E_PNNF     : name="E_PNNF";    desc="Path Name Not Found"; break;
        case E_SLF      : name="E_SLF";     desc="Segment List Full"; break;
        case E_CEF      : name="E_CEF";     desc="Creating Existing File"; break;
        case E_IBA      : name="E_IBA";     desc="Illegal Block Address"; break;
        case E_HANGUP   : name="E_HANGUP";  desc="Phone hangup occurred (modem)"; break;
        case E_MNF      : name="E_MNF";     desc="Module Not Found"; break;
        case E_NOCLK    : name="E_NOCLK";   desc="System has no clock running"; break;
        case E_DELSP    : name="E_DELSP";   desc="Deleting Stack Pointer memory"; break;
        case E_IPRCID   : name="E_IPRCID";  desc="Illegal Process ID"; break;
        case E_PARAM    : name="E_PARAM";   desc="Impossible polling parameter specified"; break;
        case E_NOCHLD   : name="E_NOCHLD";  desc="No Children"; break;
        case E_ITRAP    : name="E_ITRAP";   desc="Invalid or unavailable Trap code"; break;
        case E_PRCABT   : name="E_PRCABT";  desc="Process Aborted"; break;
        case E_PRCFUL   : name="E_PRCFUL";  desc="Process Table Full"; break;
        case E_IFORKP   : name="E_IFORKP";  desc="Illegal Fork Parameter"; break;
        case E_KWNMOD   : name="E_KWNMOD";  desc="Known Module"; break;
        case E_BMCRC    : name="E_BMCRC";   desc="Bad Module CRC"; break;
        case E_SIGNAL   : name="E_SIGNAL";  desc="Signal Error (replaces E_USIGP)"; break;
        case E_NEMOD    : name="E_NEMOD";   desc="Non Existing Module"; break;
        case E_BNAM     : name="E_BNAM";    desc="Bad Name"; break;
        case E_BMHP     : name="E_BMHP";    desc="bad module header parity"; break;
        case E_NORAM    : name="E_NORAM";   desc="No Ram Available"; break;
        case E_DNE      : name="E_DNE";     desc="Directory not empty"; break;
        case E_NOTASK   : name="E_NOTASK";  desc="No available Task number"; break;
        case E_UNIT     : name="E_UNIT";    desc="Illegal Unit (drive)"; break;
        case E_SECT     : name="E_SECT";    desc="Bad SECTor number"; break;
        case E_WP       : name="E_WP";      desc="Write Protect"; break;
        case E_CRC      : name="E_CRC";     desc="Bad Check Sum"; break;
        case E_READ     : name="E_READ";    desc="Read Error"; break;
        case E_WRITE    : name="E_WRITE";   desc="Write Error"; break;
        case E_NOTRDY   : name="E_NOTRDY";  desc="Device Not Ready"; break;
        case E_SEEK     : name="E_SEEK";    desc="Seek Error"; break;
        case E_FULL     : name="E_FULL";    desc="Media Full"; break;
        case E_BTYP     : name="E_BTYP";    desc="Bad Type (incompatible) media"; break;
        case E_DEVBSY   : name="E_DEVBSY";  desc="Device Busy"; break;
        case E_DIDC     : name="E_DIDC";    desc="Disk ID Change"; break;
        case E_LOCK     : name="E_LOCK";    desc="Record is busy (locked out)"; break;
        case E_SHARE    : name="E_SHARE";   desc="Non-sharable file busy"; break;
        case E_DEADLK   : name="E_DEADLK";  desc="I/O Deadlock error"; break;
        case E_FORMAT   : name="E_FORMAT";  desc="Device is format protected"; break;
        default         : name="E_???";     desc="<<unknown error code>>"; break;
    } /* switch */
    
    if (nameP!=NULL) *nameP= name;
    if (descP!=NULL) *descP= desc;
} /* get_error_strings */

/* syscall names for debugging */
char* get_syscall_name(ushort syscall)
{   return getfuncentry(syscall)->name;
} /* get_syscall_name */
 

/* eof */
