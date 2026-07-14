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
 *    Revision 1.8  2006/10/13 10:24:45  bfo
 *    "debug_procdump" (bus error reporting) added (by Martin Gregorie)
 *
 *    Revision 1.7  2006/02/19 15:37:20  bfo
 *    Header changed to 2006
 *
 *    Revision 1.6  2004/11/20 11:44:06  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.5  2003/05/17 11:06:20  bfo
 *    (up to date)
 *
 *    Revision 1.4  2002/07/30 16:47:19  bfo
 *    E-Mail address bfo@synthesis.ch       is updated everywhere
 *
 *    Revision 1.3  2002/06/25 20:44:33  luz
 *    Added /lp printer support under windows. Not tested or even compiled for Mac
 *
 */

/* debugging aids */
/* debug mask definitions */

#define dbgAnomaly      0x0001   /* anomalies such as unimplemented/dummy system calls (default) */
#define dbgSysCall      0x0002   /* call and return status of system calls */
#define dbgFMgrType     0x0004   /* call of traphandler calls */
#define dbgProcess      0x0008   /* Process creation/killing, signals */
#define dbgTaskSwitch   0x0010   /* task switching/arbitration */
#define dbgModules      0x0020   /* module link/load/unlink & CRC */
#define dbgMemory       0x0040   /* memory allocation/deallocation */
#define dbgUtils        0x0080   /* internal utilities */
#define dbgTrapHandler  0x0100   /* traphandlers and exception handlers */
#define dbgFiles        0x0200   /* file operations */
#define dbgErrors       0x0400   /* error generation/translation */
#define dbgStartup      0x0800   /* os9exec/nt startup and initial argument preparation */
#define dbgPartial      0x1000   /* warnings for inaccurate/partial emulated functionality */
#define dbgTerminal     0x2000   /* interactive I/O, output filtering */
#define dbgSpecialIO    0x4000   /* special I/O such as printer */

#define dbgWarnings     0x8000   /* warnings */

#define dbgAllInfo      0xFFFE   /* all except anomalies */
#define dbgAll          0xFFFF   /* all debug info */


/* debug levels/options */
#define DEBUGLEVELS 3
#define dbgNorm              0       /* normal debug info */
#define dbgDetail            1       /* detailed debug info */
#define dbgDeep              2       /* deep detail debug info */
#define dbgLevelMask        0x000F   /* mask for debug level index */

#define dbgNohalt           0x8000   /* suppress halt here, even if enabled */


/* debugging masks */
extern ushort debug[];                          /* debug level array */
extern ushort debughalt;                        /* debug halt bitmask */
#define TRIGNAMELEN 30
extern char triggername[TRIGNAMELEN];           /* debug trigger name */


/* routines */

#define RCHK_DRU 0x01
#define RCHK_ARU 0x02
#define RCHK_MEM 0x04
#define RCHK_MOD 0x08

Boolean out_of_mods(os9addr_t addr);
Boolean out_of_mem(ushort pid, os9addr_t addr);


#if defined NODEBUG
  /* avoid code generation (and argument preparation) at all */
  #define debugcheck(a,b) (false)
  #define debugprintf(a,b,c) 
  #define debug_halt(a) (false)
  #define regcheck(r,s,p,m)
  #define trigcheck(m,s)
#else
  /* real debug routines */
  Boolean debugcheck ( ushort mask, ushort level );
  Boolean debugprintf( ushort mask, ushort level, char *format, ... );
  #define debugprintf( a,b,c ) if (debugcheck(a,b)) _debugprintf c;

  ushort  debug_halt ( ushort haltmask );
  void    regcheck   ( ushort pid, char *nam, uint32_t reg, ushort mode );
  void    trigcheck  ( char  *msg, char *name );
#endif

/* PRINTF_FMT: let the compiler type-check the format string against the args.
 * These are real vararg printf calls, so a wrong specifier is undefined
 * behaviour, not a cosmetic slip -- and the mistakes are invisible on LP64 while
 * being wrong on LLP64 (or vice versa), which is exactly the sort of thing a
 * human should never be asked to police by eye. With this attribute the compiler
 * catches every one, on both toolchains. Keep it on: 124 latent mismatches were
 * found the day it was first switched on. */
#if defined __GNUC__
  #define PRINTF_FMT( fmtArg,firstVararg ) \
          __attribute__(( format( printf, fmtArg,firstVararg ) ))
#else
  #define PRINTF_FMT( fmtArg,firstVararg )
#endif

void  _debugprintf( char *format, ... ) PRINTF_FMT( 1,2 );
void   debug_prep ( void );
void   debug_procdump(process_typ *cp, int cpid);
os9err debug_help ( ushort pid, int argc, char **argv );
void   dumpregs   ( ushort pid );
ushort debugwait  ( void );

void showonereg(uint32_t value, Boolean isa, ushort regnum, ushort lenspec);
void show_maskedregs(regs_type *rp, uint32_t regmask);

char* get_stat_name(ushort stat);
char* get_ev_name  (ushort ev);
void  get_error_strings(os9err err, char **nameP, char **descP);
char* get_syscall_name(ushort syscall); 

/* eof */
