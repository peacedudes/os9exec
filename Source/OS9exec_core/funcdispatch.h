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
 *    Revision 1.11  2007/01/28 21:30:45  bfo
 *    'Regs_68k' introduced, 'built-in' renamed to 'native'
 *
 *    Revision 1.10  2007/01/07 13:23:25  bfo
 *    "trap0_call" introduced (for PLUGIN_DLL callback)
 *
 *    Revision 1.9  2006/12/01 20:00:23  bfo
 *    Enhanced (and shorter) "systime" display
 *
 *    Revision 1.8  2006/09/03 20:55:00  bfo
 *     STIM_ORDERED 0x0020 added
 *
 *    Revision 1.7  2006/07/21 07:09:14  bfo
 *    os9_to_xxx <name> parameter eliminated
 *
 *    Revision 1.6  2006/02/19 16:31:45  bfo
 *    Thread support added
 *
 *    Revision 1.5  2004/11/20 11:44:07  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.4  2002/10/09 20:35:38  bfo
 *    "show_timing" call with additional parameter <ticksLim>
 *
 *
 */


/* os9 service request */
/* defines for register usage */
#define d_b(x) (0x01 << (x+x))
#define d_w(x) (0x02 << (x+x))
#define d_l(x) (0x03 << (x+x))
#define a_l(x) (0x03 << (x+x+16))
#define a_p(x) (0x02 << (x+x+16)) /* string name pointer (bfo) */
#define SFUNC_STATCALL  0x80000000
#define SEVENT_CALL     0x40000000
#define REGMASK_REGBITS 0x3FFFFFFF

typedef os9err os9func_typ(regs_type *rp, ushort cpid);

typedef struct {
            os9func_typ *func;
            char *name;
            uint32_t inregs;
            uint32_t outregs;
        } funcdispatch_entry;

#define NUMFCALLS   0x61
extern const funcdispatch_entry fcalltable[NUMFCALLS];
#define ICALLS_Offs 0x80
#define NUMICALLS   0x13
extern const funcdispatch_entry icalltable[NUMICALLS];

extern  ulong  rw__idleticks; /* set by read/write of console */
extern  ulong  slp_idleticks; /* set by arbitration sleep */
extern  ulong glob_callticks;

#define STIM_NONE           0
#define STIM_FCALLS    0x0001
#define STIM_ICALLS    0x0002
#define STIM_OS9       0x0004
#define STIM_S       ( STIM_ICALLS | STIM_FCALLS | STIM_OS9 ) /* combined for option -s */

#define STIM_TICKAVAIL 0x0008
#define STIM_PERCENT   0x0010
#define STIM_ORDERED   0x0020
#define STARTCALL      0xffff

ulong GetSystemTick(void);

/* The system tick (os9_tick.c). Off unless -q asks for it; started when the
 * guest first sets the time, as on a real system. */
void  os9_tick_start(void);
void  os9_tick_stop (void);

const funcdispatch_entry *getfuncentry(ushort func);

void os9_to_xxx             ( ushort pid );
void xxx_to_arb( ushort func, ushort pid );
void arb_to_os9( Boolean last_arbitrate  );


Boolean Dbg_SysCall ( regs_type* rp, ushort pid );
void    debug_comein( regs_type* rp, ushort pid );
void    debug_return( regs_type* rp, ushort pid, Boolean cwti );

// ---- accessible for the C++ world as well ----
#if defined __cplusplus
  extern "C" {
#endif

ushort pthread_pid();
os9err exec_syscall( ushort func, ushort pid, regs_type* rp, Boolean intUtil );
os9err trap0_call               ( ushort pid, Regs_68k*  regs );

#if defined __cplusplus
  } // end extern "C"
#endif
// -----------------------------------------------

void   init_L2(void);
void   init_syscalltimers(void);
ulong  show_timing( ushort mode,int ticksLim, Boolean doDisable );
os9err int_systime( ushort pid, int argc, char **argv );

/* eof */
