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
 *    Revision 1.47  2007/02/24 14:15:13  bfo
 *    pWaitRead correctly handled for internal commands
 *
 *    Revision 1.46  2007/02/22 23:06:28  bfo
 *    Parameters reordered
 *
 *    Revision 1.45  2007/01/28 21:30:45  bfo
 *    'Regs_68k' introduced, 'built-in' renamed to 'native'
 *
 *    Revision 1.44  2007/01/07 13:23:21  bfo
 *    "trap0_call" introduced (for PLUGIN_DLL callback)
 *
 *    Revision 1.43  2006/12/16 22:14:57  bfo
 *    cp->oerr assigned correctly for internal commands
 *
 *    Revision 1.42  2006/12/02 12:12:13  bfo
 *    make cp->lastsyscall visible for internal commands as well
 *
 *    Revision 1.41  2006/12/01 20:00:23  bfo
 *    Enhanced (and shorter) "systime" display
 *
 *    Revision 1.40  2006/11/13 14:54:49  bfo
 *    (char*) type casting for GCC
 *
 *    Revision 1.39  2006/11/12 13:18:06  bfo
 *    Problem with changed <currentpid> (internal commands) fixed
 *
 *    Revision 1.38  2006/11/04 23:34:49  bfo
 *    <procName> => <intProcName>
 *
 *    Revision 1.37  2006/11/03 23:48:29  bfo
 *    Copy only used aX/dX registers for int commands
 *
 *    Revision 1.36  2006/11/01 11:42:08  bfo
 *    Copyback of <os9regs> for int commands no longer needed
 *    (because <os9regs> reference directly copies them to the right place)
 *
 *    Revision 1.35  2006/10/29 18:52:27  bfo
 *    <l2.hw_location> will be written only when changed
 *
 *    Revision 1.34  2006/10/12 19:48:49  bfo
 *    Show the correct registers for F$GPrDBT
 *
 *    Revision 1.33  2006/10/01 15:39:10  bfo
 *    <ptocMask> eliminated; os9exec_loop call rearranged
 *
 *    Revision 1.32  2006/09/04 14:52:22  bfo
 *    systime sorting corrected / -h added to -? help text
 *
 *    Revision 1.31  2006/09/03 20:56:09  bfo
 *    "systime -h" will show the highest tick count first
 *
 *    Revision 1.30  2006/08/29 22:37:13  bfo
 *    mid==0 test replaced by cp->isIntUtil
 *
 *    Revision 1.29  2006/08/26 23:57:31  bfo
 *    "START" for built-in cmds done correctly /
 *    A lot of old commented out stuff eliminated
 *
 *    Revision 1.28  2006/08/04 18:43:39  bfo
 *    Avoid negative ticks
 *
 *    Revision 1.27  2006/07/29 09:03:56  bfo
 *    "debug_return" at correct location now
 *
 *    Revision 1.26  2006/07/23 14:34:25  bfo
 *    os9exec_loop call <ptocMask> and <arbitrate> dependent
 *
 *    Revision 1.25  2006/07/21 07:10:42  bfo
 *    os9_to_xxx <name> parameter eliminated
 *
 *    Revision 1.24  2006/07/14 11:48:12  bfo
 *    os9_xxx kk<MAX_OS9PROGS-1 problem fixed (avoid memory overrun)
 *
 *    Revision 1.23  2006/07/10 10:02:14  bfo
 *    F_Exit special support for internal utilities
 *    "os9exec_loop" prepared, still inactive (because unstable)
 *
 *    Revision 1.22  2006/06/16 16:00:14  bfo
 *    Don't throw exceptions here anymore
 *
 *    Revision 1.21  2006/06/11 22:14:10  bfo
 *    Some printing comments eliminated
 *
 *    Revision 1.20  2006/02/19 16:31:45  bfo
 *    Thread support added
 *
 *    Revision 1.19  2005/07/15 22:20:13  bfo
 *    unistd.h implemented only here for MACH
 *
 *    Revision 1.18  2005/07/06 21:02:14  bfo
 *    defined UNIX
 *
 *    Revision 1.17  2005/07/02 14:17:12  bfo
 *    Adapted for Mach-O / break after switch for 'systime' options
 *
 *    Revision 1.16  2005/06/30 11:40:43  bfo
 *    Mach-O support / systime -t : more consistent support
 *
 *    Revision 1.15  2004/11/27 12:11:10  bfo
 *    _XXX_ introduced
 *
 *    Revision 1.14  2004/11/20 11:44:07  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.13  2004/10/22 22:51:12  bfo
 *    Most of the "pragma unused" eliminated
 *
 *    Revision 1.12  2003/08/01 11:13:42  bfo
 *    /L2 'Blue' support
 *
 *    Revision 1.11  2003/07/08 15:41:05  bfo
 *    Print hw name additionally
 *
 *    Revision 1.10  2003/05/17 10:27:14  bfo
 *    (remove some old unused comments)
 *
 *    Revision 1.9  2002/10/09 20:30:49  bfo
 *    "show_timing" call with additional parameter <ticksLim>
 *
 *    Revision 1.8  2002/10/02 18:42:38  bfo
 *    "systime -s?" bug fixed (no stop after -? text)
 *
 *    Revision 1.7  2002/08/13 21:24:17  bfo
 *    Some more variables defined at the real procid struct now.
 *
 *
 */


/* OS9 Service Request Dispatch table */
/* ================================== */

#include "os9exec_incl.h"

#ifdef linux
  #include <time.h>
#endif

#ifdef MACOSX
  #include <unistd.h>
#endif


/* externally used */
ulong   rw__idleticks;            /* set by read and write of console */
ulong   slp_idleticks;            /* set by sleep in arbitration */

/* system call timing */
ulong   sum_rw__idleticks;        /* total number of ticks spent idle (console input) */
ulong   sum_slp_idleticks;        /* total number of ticks spent idle (arbitrate sleep) */
ulong   arbticks;                 /* total number of ticks spent in arbitration */
ulong   arbnum;                   /* number of arbitration calls */
ulong   last_complete;            /* time of last completion */
char    systime_prog[OS9NAMELEN]; /* one specific program will be logged */
Boolean systime_prog_ok;          /* chosen name has been called ? */

time_t  timerstart;               /* time in seconds when timing was started */
ulong   fcall_time[NUMFCALLS+1];  /* ticks spent in F$xxx calls */
ulong   icall_time[NUMICALLS+1];  /* ticks spent in I$xxx calls */
ulong   fcall_num [NUMFCALLS+1];  /* number of F$xxx calls */
ulong   icall_num [NUMICALLS+1];  /* number of I$xxx calls */
/* ----------------------------------------------------------------------------------- */




static os9err OS9_TCP_Select( regs_type *rp, _pid_ )
/* OS9TCP specific select */
/* currently not in use, because the only the ISP system is supported */
{
    upe_printf( "select\n" );
 
    rp->d[0]= 0;
    rp->d[1]= 0;
    rp->d[2]= 0;
 
    return 0;
} /* OS9_TCP_Select */



/* dispatch table */
const funcdispatch_entry startcall =
    { /* start   */ OS9_F_SDummy, "START",  0, 0 };

const funcdispatch_entry invalidcall =
    { /* invalid */ OS9_F_UnImp,  "---",    0, 0 };

const funcdispatch_entry nocall =
    { /* no call */ OS9_F_SDummy, "<none>", 0, 0 };

const funcdispatch_entry tcpselect =
    { /* $61     */ OS9_TCP_Select, "OS9_TCP_Select", 0, 0 }; /* OS9TCP specific call */



const funcdispatch_entry fcalltable[NUMFCALLS] =
{
    { /* 0x00 */ OS9_F_Link,   "F$Link",   d_w(0)+a_p(0),        d_w(0)+d_w(1)+a_l(0)+a_l(1)+a_l(2) }, /* Link to Module */
    { /* 0x01 */ OS9_F_Load,   "F$Load",   d_b(0)+a_p(0),        d_w(0)+d_w(1)+a_l(0)+a_l(1)+a_l(2) }, /* Load Module from File */
    { /* 0x02 */ OS9_F_UnLink, "F$UnLink", a_l(2),               0      }, /* Unlink Module */
    { /* 0x03 */ OS9_F_Fork,   "F$Fork",   d_w(0)+d_l(1)+d_l(2)+
                                           d_w(3)+d_w(4)+
                                           a_p(0)+a_l(1),        d_w(0)+a_l(0) }, /* Start New Process */
    { /* 0x04 */ OS9_F_Wait,   "F$Wait",   0,                    d_w(0)+d_w(1) }, /* Wait for Child Process to Die */
    { /* 0x05 */ OS9_F_Chain,  "F$Chain",  d_w(0)+d_l(1)+d_l(2)+
                                           d_w(3)+d_w(4)+
                                           a_p(0)+a_l(1),        0      }, /* Chain Process to New Module */
    { /* 0x06 */ OS9_F_Exit,   "F$Exit",   d_w(1),               0      }, /* Terminate Process */
    { /* 0x07 */ OS9_F_UnImp,  "F$Mem",    d_l(0),               d_l(0)+a_l(1) }, /* Set Memory Size */
    { /* 0x08 */ OS9_F_Send,   "F$Send",   d_w(0)+d_w(1),        0      }, /* Send Signal to Process */
    { /* 0x09 */ OS9_F_Icpt,   "F$Icpt",   a_l(0)+a_l(6),        0      }, /* Set Signal Intercept */
    { /* 0x0A */ OS9_F_Sleep,  "F$Sleep",  d_l(0),               d_l(0) }, /* Suspend Process */
    { /* 0x0B */ OS9_F_UnImp,  "F$SSpd",   d_w(0),               d_w(1) }, /* Suspend Process */
    { /* 0x0C */ OS9_F_ID,     "F$ID",     0,                    d_w(0)+d_l(1)+d_w(2) }, /* Return Process ID */
    { /* 0x0D */ OS9_F_SPrior, "F$SPrior", d_w(0)+d_w(1),        0      }, /* Set Process Priority */
    { /* 0x0E */ OS9_F_STrap,  "F$STrap",  a_l(0)+a_l(1),        0      }, /* Set Trap Intercept */
    { /* 0x0F */ OS9_F_PErr,   "F$PErr",   d_w(0)+d_w(1),        0      }, /* Print Error */
    { /* 0x10 */ OS9_F_PrsNam, "F$PrsNam", a_p(0),               d_b(0)+d_w(1)+a_l(0)+a_l(1) }, /* Parse Pathlist Name */
    { /* 0x11 */ OS9_F_CmpNam, "F$CmpNam", d_w(1)+a_p(0)+a_p(1), 0      }, /* Compare Two Name */
    { /* 0x12 */ OS9_F_UnImp,  "F$SchBit", d_w(0),               d_w(1) }, /* Search Bit Map */
    { /* 0x13 */ OS9_F_UnImp,  "F$AllBit", d_w(0),               d_w(1) }, /* Allocate in Bit Map */
    { /* 0x14 */ OS9_F_UnImp,  "F$DelBit", d_w(0),               d_w(1) }, /* Deallocate in Bit Map */
    { /* 0x15 */ OS9_F_Time,   "F$Time",   d_w(0),               d_l(0)+d_l(1)+d_w(2)+d_l(3) }, /* Get Current Time */
    { /* 0x16 */ OS9_F_STime,  "F$STime",  d_l(0)+d_l(1),        0      }, /* Set Current Time */
    { /* 0x17 */ OS9_F_CRC,    "F$CRC",    d_l(0)+d_l(1)+a_l(0), d_l(1) }, /* Generate CRC */
    { /* 0x18 */ OS9_F_GPrDsc, "F$GPrDsc", d_w(0)+d_w(1)+a_l(0), 0      }, /* get Process Descriptor copy */
    { /* 0x19 */ OS9_F_GBlkMp, "F$GBlkMp", d_w(0)+d_w(1)+a_l(0), d_l(0)+d_l(1)+d_l(2)+d_l(3)+a_l(0) }, /* get System Block Map copy */
    { /* 0x1A */ OS9_F_GModDr, "F$GModDr", d_l(1)+a_l(0),        d_l(1) }, /* get Module Directory copy */
    { /* 0x1B */ OS9_F_CpyMem, "F$CpyMem", d_w(0)+d_l(1)+a_l(0)+
                                           a_l(1),               0      }, /* Copy External Memory */
    { /* 0x1C */ OS9_F_SUser,  "F$SUser",  d_l(1),               0      }, /* Set User ID number */
    { /* 0x1D */ OS9_F_UnLoad, "F$UnLoad", d_w(0)+a_p(0),        a_l(0) }, /* Unlink Module by name */
    { /* 0x1E */ OS9_F_RTE,    "F$RTE",    0,                    0      }, /* Return from Intercept routine */
    { /* 0x1F */ OS9_F_GPrDBT, "F$GPrDBT", a_l(0)+d_l(1),        d_l(1) }, /* Get system global data copy */
    { /* 0x20 */ OS9_F_Julian, "F$Julian", d_l(0)+d_l(1),        d_l(0)+d_l(1) }, /* Convert gregorian to Julian date */
    { /* 0x21 */ OS9_F_TLink,  "F$TLink",  d_w(0)+d_l(1)+a_p(0), a_l(0)+a_l(1)+a_l(2) }, /* Link trap subroutine package */
    { /* 0x22 */ OS9_F_DFork,  "F$DFork",  d_w(0)+d_l(1)+d_l(2)+
                                           d_w(3)+d_w(4)+
                                           a_p(0)+a_l(1),        d_w(0)+a_l(0) }, /* Debugging Fork call */
    { /* 0x23 */ OS9_F_DExec,  "F$DExec",  d_w(0),               0      }, /* Debugging execution call (run child) */
    { /* 0x24 */ OS9_F_DExit,  "F$DExit",  d_w(0),               0      }, /* Debugging exit call (kill child) */
    { /* 0x25 */ OS9_F_DatMod, "F$DatMod", d_l(0)+d_w(1)+d_w(2)+
                                           d_w(3)+a_p(0),        d_w(0)+d_w(1)+a_l(0)+a_l(1)+a_l(2) }, /* Create data module */
    { /* 0x26 */ OS9_F_SetCRC, "F$SetCRC", a_l(0),               0      }, /* Generate valid header and CRC in module */
    { /* 0x27 */ OS9_F_SetSys, "F$SetSys", d_w(0)+d_l(1)+d_l(2), d_l(2) }, /* Set/examine system global variable */
    { /* 0x28 */ OS9_F_SRqMem, "F$SRqMem", d_l(0),               d_l(0)+a_l(2) }, /* System Memory Request */
    { /* 0x29 */ OS9_F_SRtMem, "F$SRtMem", d_l(0)+a_l(2),        0      }, /* System Memory Return */
    { /* 0x2A */ OS9_F_UnImp,  "F$IRQ",    d_w(0),               d_w(1) }, /* Enter IRQ Polling Table */
    { /* 0x2B */ OS9_F_UnImp,  "F$IOQu",   d_w(0),               d_w(1) }, /* Enter I/O Queue */
    { /* 0x2C */ OS9_F_UnImp,  "F$AProc",  d_w(0),               d_w(1) }, /* Enter Active Process Queue */
    { /* 0x2D */ OS9_F_UnImp,  "F$NProc",  d_w(0),               d_w(1) }, /* Start Next Process */
    { /* 0x2E */ OS9_F_UnImp,  "F$VModul", d_w(0),               d_w(1) }, /* Validate Module */
    { /* 0x2F */ OS9_F_UnImp,  "F$FindPD", d_w(0),               d_w(1) }, /* Find Process/Path Descriptor */
    { /* 0x30 */ OS9_F_UnImp,  "F$AllPD",  d_w(0),               d_w(1) }, /* Allocate Process/Path Descriptor */
    { /* 0x31 */ OS9_F_UnImp,  "F$RetPD",  d_w(0),               d_w(1) }, /* Return Process/Path Descriptor */
    { /* 0x32 */ OS9_F_SSvc,   "F$SSvc",   a_l(1)+a_l(3),        0      }, /* Service Request Table Initialization */
    { /* 0x33 */ OS9_F_UnImp,  "F$IODel",  d_w(0),               d_w(1) }, /* Delete I/O Module */
    { /* 0x34 */ OS9_F_UnImp,  "F$???_34", d_w(0),               d_w(1) }, /* not existing */
    { /* 0x35 */ OS9_F_UnImp,  "F$???_35", d_w(0),               d_w(1) }, /* not existing */
    { /* 0x36 */ OS9_F_UnImp,  "F$???_36", d_w(0),               d_w(1) }, /* not existing */
    { /* 0x37 */ OS9_F_UnImp,  "F$GProcP", d_w(0),               d_w(1) }, /* Get Process ptr */
    { /* 0x38 */ OS9_F_UnImp,  "F$Move",   d_w(0),               d_w(1) }, /* Move Data */
    { /* 0x39 */ OS9_F_UnImp,  "F$AllRAM", d_w(0),               d_w(1) }, /* Allocate RAM blocks */
    { /* 0x3A */ OS9_F_Permit, "F$Permit", d_l(0)+d_b(1)+a_l(2), d_w(1) }, /* Protect access to RAM block */
    { /* 0x3B */ OS9_F_UnImp,  "F$Protect",d_w(0),               d_w(1) }, /* Protect access of RAM block */
    { /* 0x3C */ OS9_F_UnImp,  "F$SetImg", d_w(0),               d_w(1) }, /* Set Process DAT Image */
    { /* 0x3D */ OS9_F_UnImp,  "F$FreeLB", d_w(0),               d_w(1) }, /* Get Free Low Block */
    { /* 0x3E */ OS9_F_UnImp,  "F$FreeHB", d_w(0),               d_w(1) }, /* Get Free High Block */
    { /* 0x3F */ OS9_F_UnImp,  "F$AllTsk", d_w(0),               d_w(1) }, /* Allocate Process Task number */
    { /* 0x40 */ OS9_F_UnImp,  "F$DelTsk", d_w(0),               d_w(1) }, /* Deallocate Process Task number */
    { /* 0x41 */ OS9_F_UnImp,  "F$SetTsk", d_w(0),               d_w(1) }, /* Set Process Task DAT registers */
    { /* 0x42 */ OS9_F_UnImp,  "F$ResTsk", d_w(0),               d_w(1) }, /* Reserve Task number */
    { /* 0x43 */ OS9_F_UnImp,  "F$RelTsk", d_w(0),               d_w(1) }, /* Release Task number */
    { /* 0x44 */ OS9_F_UnImp,  "F$DATLog", d_w(0),               d_w(1) }, /* Convert DAT Block/Offset to Logical */
    { /* 0x45 */ OS9_F_UnImp,  "F$DATTmp", d_w(0),               d_w(1) }, /* Make temporary DAT image */
    { /* 0x46 */ OS9_F_UnImp,  "F$LDAXY",  d_w(0),               d_w(1) }, /* Load A [X,[Y]] */
    { /* 0x47 */ OS9_F_UnImp,  "F$LDAXYP", d_w(0),               d_w(1) }, /* Load A [X+,[Y]] */
    { /* 0x48 */ OS9_F_UnImp,  "F$LDDDXY", d_w(0),               d_w(1) }, /* Load D [D+X,[Y]] */
    { /* 0x49 */ OS9_F_UnImp,  "F$LDABX",  d_w(0),               d_w(1) }, /* Load A from 0,X in task B */
    { /* 0x4A */ OS9_F_UnImp,  "F$STABX",  d_w(0),               d_w(1) }, /* Store A at 0,X in task B */
    { /* 0x4B */ OS9_F_UnImp,  "F$AllPrc", d_w(0),               d_w(1) }, /* Allocate Process Descriptor */
    { /* 0x4C */ OS9_F_UnImp,  "F$DelPrc", d_w(0),               d_w(1) }, /* Deallocate Process Descriptor */
    { /* 0x4D */ OS9_F_UnImp,  "F$ELink",  d_w(0),               d_w(1) }, /* Link using Module Directory Entry */
    { /* 0x4E */ OS9_F_UnImp,  "F$FModul", d_w(0),               d_w(1) }, /* Find Module Directory Entry */
    { /* 0x4F */ OS9_F_UnImp,  "F$MapBlk", d_w(0),               d_w(1) }, /* Map Specific Block */
    { /* 0x50 */ OS9_F_UnImp,  "F$ClrBlk", d_w(0),               d_w(1) }, /* Clear Specific Block */
    { /* 0x51 */ OS9_F_UnImp,  "F$DelRAM", d_w(0),               d_w(1) }, /* Deallocate RAM blocks */
    { /* 0x52 */ OS9_F_SysDbg, "F$SysDbg", 0,                    0      }, /* Invoke system level debugger */
    { /* 0x53 */ OS9_F_Event,  "F$Event",  SEVENT_CALL+d_l(0)+
                                           d_w(1)+d_w(2)+
                                           d_w(3)+a_l(0),        d_l(0)+a_l(0) }, /* Create/Link to named event */
    { /* 0x54 */ OS9_F_Gregor, "F$Gregor", d_l(0)+d_l(1),        d_l(0)+d_l(1) }, /* Convert julian date to gregorian date */
    { /* 0x55 */ OS9_F_UnImp,  "F$SysID",  d_w(0),               d_w(1) }, /* return system identification */
    { /* 0x56 */ OS9_F_Alarm,  "F$Alarm",  d_l(0)+d_w(1)+
                                           d_l(2)+d_l(3)+d_l(4), d_l(0) }, /* send alarm signal */
    { /* 0x57 */ OS9_F_Sigmask,"F$Sigmask",d_l(0)+d_l(1),        0      }, /* set signal mask */
    { /* 0x58 */ OS9_F_UnImp,  "F$ChkMem", d_w(0),               d_w(1) }, /* determine if user process may access memory area */
    { /* 0x59 */ OS9_F_UnImp,  "F$UAcct",  d_w(0),               d_w(1) }, /* inform user accounting of process status */
    { /* 0x5A */ OS9_F_SDummy, "F$CCtl",   d_l(0),               0      }, /* cache control */
    { /* 0x5B */ OS9_F_SUnImp, "F$GSPUMp", d_w(0),               d_w(1) }, /* get SPU map information for a process */
    { /* 0x5C */ OS9_F_SRqMem, "F$SRqCMem",d_l(0)+d_l(1),        d_l(0)+a_l(2) }, /* System Colored Memory Request */
    { /* 0x5D */ OS9_F_UnImp,  "F$POSK",   d_w(0),               d_w(1) }, /* execute svc request */
    { /* 0x5E */ OS9_F_Panic,  "F$Panic",  d_w(0),               d_w(1) }, /* Panic warning */
    { /* 0x5F */ OS9_F_UnImp,  "F$MBuf",   d_w(0),               d_w(1) }, /* Memory buffer manager */
    { /* 0x60 */ OS9_F_UnImp,  "F$Trans",  d_w(0),               d_w(1) }  /* Translate memory address to/from external bus */
};

const funcdispatch_entry icalltable[NUMICALLS] = {
    { /* 0x80 */ OS9_I_Attach, "I$Attach", d_b(0)+a_p(0),        a_l(2) }, /* Attach I/O Device */
    { /* 0x81 */ OS9_I_Detach, "I$Detach", a_l(2),               0      }, /* Detach I/O Device */
    { /* 0x82 */ OS9_I_Dup,    "I$Dup",    d_w(0),               d_w(0) }, /* Duplicate Path */
    { /* 0x83 */ OS9_I_Create, "I$Create", d_b(0)+d_w(1)+d_l(2)+
                                           a_p(0),               d_w(0)+a_l(0) }, /* Create New File */
    { /* 0x84 */ OS9_I_Open,   "I$Open",   d_b(0)+a_p(0),        d_w(0)+a_l(0) }, /* Open Existing File */
    { /* 0x85 */ OS9_I_MakDir, "I$MakDir", d_b(0)+d_w(1)+d_l(2)+
                                           a_p(0),               a_l(0) }, /* Make Directory File */
    { /* 0x86 */ OS9_I_ChgDir, "I$ChgDir", d_b(0)+a_p(0),        a_l(0) }, /* Change Default Directory */
    { /* 0x87 */ OS9_I_Delete, "I$Delete", d_b(0)+a_p(0),        a_l(0) }, /* Delete File */
    { /* 0x88 */ OS9_I_Seek,   "I$Seek",   d_w(0)+d_l(1),        0      }, /* Change Current Position */
    { /* 0x89 */ OS9_I_Read,   "I$Read",   d_w(0)+d_l(1)+a_l(0), d_l(1) }, /* Read Data */
    { /* 0x8A */ OS9_I_Write,  "I$Write",  d_w(0)+d_l(1)+a_l(0), d_l(1) }, /* Write Data */
    { /* 0x8B */ OS9_I_ReadLn, "I$ReadLn", d_w(0)+d_l(1)+a_l(0), d_l(1) }, /* Read Line of ASCII Data */
    { /* 0x8C */ OS9_I_WritLn, "I$WritLn", d_w(0)+d_l(1)+a_l(0), d_l(1) }, /* Write Line of ASCII Data */

    { /* 0x8D */ OS9_I_GetStt, "I$GetStt",SFUNC_STATCALL+d_w(0)+
                                           d_w(1)+d_l(2)+d_l(3)+
                                           a_l(0),               d_l(0)+d_l(1)+d_l(2)+d_l(3)+a_l(0) }, /* Get Path Status */
    { /* 0x8E */ OS9_I_SetStt, "I$SetStt",SFUNC_STATCALL+d_w(0)+
                                           d_w(1)+d_l(2)+d_l(3)+
                                           a_l(0),               d_l(0)+d_l(1)+d_l(2)+d_l(3)+a_l(0) }, /* Set Path Status */
                                           
    { /* 0x8F */ OS9_I_Close,  "I$Close",  d_w(0),               0      }, /* Close Path */
    { /* 0x90 */ OS9_F_UnImp,  "I$???_90", d_w(0),               d_w(1) }, /* not implemented */
    { /* 0x91 */ OS9_F_UnImp,  "I$???_91", d_w(0),               d_w(1) }, /* not implemented */

    { /* 0x92 */ OS9_I_SGetSt, "I$SGetSt",SFUNC_STATCALL+d_w(0)+
                                           d_w(1)+d_l(2)+d_l(3)+
                                           a_l(0),               d_l(0)+d_l(1)+d_l(2)+d_l(3)+a_l(0) }  /* Getstat using system path number */
};


/* quickly get correct entry for function call */
const funcdispatch_entry* getfuncentry( ushort func )
{
    if         (func==STARTCALL)             
                                             return &startcall;
    if         (func             <NUMFCALLS) return &fcalltable[func];
    else { 
        if     (func>=ICALLS_Offs) {
            if (func -ICALLS_Offs<NUMICALLS) return &icalltable[func-ICALLS_Offs];
            else                             return &invalidcall; /* any other: invalid call */
        }
        else {
            if (func==0x7F) return &nocall;            /* consider this as "no call" */
            if (func==0x61) return &tcpselect; /* this is used as selector in OS9TCP */
                            /* %%% in real OS-9 this will be installed with a F$SSvc */

            upe_printf( "function out of range: %x\n", func );
            return &invalidcall; /* any other: invalid call */
        }
    }
} /* getfuncentry */



static ulong DiffTick( void )
/* get the difference since last ticks */
{
    process_typ* cp= &procs[1];
    ulong            t= GetSystemTick(); /* time spent since last call except idle part */
    ulong        r = last_complete + rw__idleticks + slp_idleticks;
    ulong        a = 0; if ( t>r ) a= t - r;
    last_complete  = t;
   
    cp->iticks       += rw__idleticks + slp_idleticks;
    cp->pd._sticks= os9_long(cp->fticks + cp->iticks);
    
    sum_rw__idleticks+= rw__idleticks;  rw__idleticks= 0;
    sum_slp_idleticks+= slp_idleticks;  slp_idleticks= 0;

    return a;
} /* DiffTick */



void os9_to_xxx( ushort pid )
/* Get systime ticks on the way from OS-9 to XXX command */
{
  char         *mn, *p, *q, *systime_end;
  int          mid, ii, jj, kk;
  mod_exec*    mod;
  st_typ       *s, *sj, *sj1;
  Boolean      eli= false; /* end of list */
  ulong        a= DiffTick();
  process_typ* cp= &procs[ pid ];
  procid*      pd= &cp->pd;
    
  /* try to measure ticks */
  os9_long_inc( &pd->_uticks, a ); /* info for F$GPrDsc */

      mid=     cp->mid;    
      mod= os9mod( mid );
  if (mid==0 || mod==NULL) mn= (char*)&cp->intProcName;
  else                     mn= Mod_Name( mod );

  /* question: must it be logged ? */
  systime_prog_ok= (*systime_prog==NUL)
        || (ustrcmp( systime_prog,mn )==0);

  // search for "xxx,yyy", 
  // "shell" <=> "ll" problem is fixed now
  if (!systime_prog_ok) {
                 p= systime_prog;
    systime_end= p + strlen( p );
        
    while (true) {
         p=  strstr( p,mn ); if (p==NULL) break;
      q= p + strlen  ( mn );
                              
      if ((p==systime_prog || *(p-1)==',') && // first or subsequent
          (q==systime_end  ||  *q   ==',')) { systime_prog_ok= true; break; }
              
      p= q;
    } // loop
  } // if

  if (!systime_prog_ok) { mn= "      (other progs)"; eli= true; }
    
  /* search for the module's name in the list, if not there add name to list */
  for (ii=0; ii<MAX_OS9PROGS; ii++) {
                            s= &statistics[ii];
    if                   ( *s->name==NUL            /* not yet in the list */
      || (!eli && ustrncmp( s->name," ", 1 )==0)
      || (!eli && ustrcmp ( s->name,mn     ) >0)) { /* sort them alphabetically */

             kk= ii; /* search for the last item, don't go OVER the limit !! */
      while (kk<MAX_OS9PROGS-1 && *statistics[kk].name!=NUL) kk++;

      for (jj= kk; jj>ii;  jj--) { /* shift up */
        sj = &statistics[jj  ];
        sj1= &statistics[jj-1];
        strcpy( sj->name,   sj1->name );
                sj->intern= sj1->intern;
                sj->ticks = sj1->ticks;
                sj->num   = sj1->num;
      } /* inner for */
        
      strncpy( s->name, mn, OS9NAMELEN-1 );  /* bounded: module name may exceed OS9NAMELEN */
      s->name[OS9NAMELEN-1]= NUL;
      s->intern= !eli && cp->isIntUtil;
              s->ticks = 0;
              s->num   = 0;
    } // if
        
    if (ustrcmp( mn,s->name )==0) {
                    s->ticks+= a;
                    s->num  ++; break;
    } // if
  } // outer for 
} // os9_to_xxx



void xxx_to_arb( ushort func, ushort pid )
/* Get systime ticks on the way from XXX command to arbitration */
{
    /* calc and save time spent */
    process_typ* cp= &procs[pid];
    ulong        a = DiffTick();
    ulong        f;
            
    /* func was within range, register */
    if (func<=NUMFCALLS) {
        if (systime_prog_ok) f= func;
        else                 f= NUMFCALLS;
        
        fcall_num [f]++;
        fcall_time[f]+= a;
        cp->fticks   += a; /* info for F$GPrDsc */
    }
    else {
        if (systime_prog_ok) f= func-0x80;
        else                 f= NUMICALLS;

        icall_num [f]++;
        icall_time[f]+= a;
        cp->iticks   += a; /* info for F$GPrDsc */
    }  
    
    cp->pd._sticks= os9_long(cp->fticks + cp->iticks);
} /* xxx_to_arb */



void arb_to_os9( Boolean last_arbitrate )
/* Get systime ticks on the way from arbitration to OS-9 */
{
    process_typ* cp= &procs[1];
    ulong a= DiffTick();  /* must be done after <b> calculation */
    
    arbticks  += a;
    cp->fticks+= a; /* info for F$GPrDsc */
    cp->pd._sticks= os9_long(cp->fticks + cp->iticks); /* update it */
    
    if (last_arbitrate) arbnum++;
} /* arb_to_os9 */



Boolean Dbg_SysCall( regs_type* rp, ushort pid )
{
  process_typ* cp= &procs[pid];
  
  if (!debugcheck(dbgSysCall,dbgNorm)) return false;
	 
  // OS9_I_WritLn, special masking provided for "Maloney" system
  return cp->func!=0x8c || loword(rp->d[0])<0x80 || debugcheck(dbgSysCall,dbgDetail);	
} /* Dbg_SysCall */


void debug_comein( regs_type* rp, ushort pid )
{
  process_typ* cp= &procs[pid];
  const funcdispatch_entry* fdeP= getfuncentry(cp->func);
  Boolean                   msk = cp->masklevel>0;

  if (!Dbg_SysCall( rp, pid )) return;
  if (cp->state==pWaitRead)    return; /* avoid dbg display in pWaitRead mode */

  /* (enclosed in another if to avoid get_syscall_name invocation */
  /* in nodebug case!) */
  uphe_printf( ">>>%cPid=%02d: OS9 %s : ",msk ? '*':' ', pid,fdeP->name );
  show_maskedregs( rp,fdeP->inregs );
  upe_printf ( "\n" );
} /* debug_comein */



void debug_return( regs_type* crp, ushort pid, Boolean cwti )
{	
  process_typ*              cp  = &procs[ pid ];
  const funcdispatch_entry* fdeP= getfuncentry(cp->func);
  
  char*     errnam;
  char*     errdesc;
  mod_exec* mod;
  char*     p;
  char      item[OS9NAMELEN];
  Boolean   msk= cp->masklevel  >0;
  Boolean   hdl= cp->pd._sigvec!=0;
  Boolean   strt;
  
  if (!Dbg_SysCall( crp, pid )) return;
  
  if (cwti) {
    uphe_printf( "<<<%cPid=%02d: OS9 INTERCEPT%s, state=%s, ", 
						msk ? '*':' ',pid,
						hdl ? "" :" (no handler)", PStateStr(cp) );
							 
    show_maskedregs( &cp->os9regs, d_w(1)+a_l(6) );
	upe_printf ( "\n" );
  }
  else {
	if ((cp->state==pActive  || // internal utilities can be active now as well
		 cp->oerr) &&
         cp->state!=pWaitRead) { /* avoid dbg display in pWaitRead mode */

      /* D1.w/carry error reporting will be done when process is active or syscall delivered error, */
      /* otherwise, d1.w will be updated when suspended process gets active again */
          strt= (ustrcmp(fdeP->name,"START")==0);
      if (strt) {
        if (cp->isIntUtil) {
          p= (char*)&cp->intProcName;
        }
        else {
          mod= (mod_exec *)FROM68K(cp->os9regs.a[3]);
          p  =  Mod_Name( mod );
        } // if
        
        strcpy( item,"\"" );
        strncat( item, p, OS9NAMELEN-3 );  /* -3 for quotes + null terminator */
        strncat( item,"\"", OS9NAMELEN-strlen(item)-1 );

        if (cp->isIntUtil)
          strncat( item, " (native)", OS9NAMELEN-strlen(item)-1 );
      } // if

      uphe_printf("<<<%cPid=%02d: OS9 %s %s",msk ? '*':' ',
                   pid, fdeP->name, strt ? item:"returns: " );

      if (cp->oerr) {
        get_error_strings(cp->oerr, &errnam,&errdesc);
        //  errnam= "XXX"; errdesc= "";
        upe_printf( "#%03d:%03d - %s\n", cp->oerr>>8,cp->oerr &0xFF,errnam );
      }
      else {
        show_maskedregs( &cp->os9regs,fdeP->outregs );
        upe_printf( "\n" );
      } // if
    } 
  } /* if (cwti) */
} /* debug_return */


ushort pthread_pid()
{
  #ifdef THREAD_SUPPORT
    pthread_t tid= pthread_self();
    int       i;

    /* pthread_equal, not ==: pthread_t is opaque and comparing it as an
       integer is not portable. The old code stashed it in a ulong and compared
       against an int field, which truncated on every LP64 host and matched
       nothing -- see the tid declaration in os9exec_nt.h. */
    for (i= 0; i<MAXPROCESSES; i++) {
      if (procs[ i ].tidValid && pthread_equal( procs[ i ].tid, tid )) return i;
    } // for
  #endif
  
  return currentpid;
} // pthread_pid


/* perform system call */
os9err exec_syscall( ushort func, ushort pid, regs_type* rp, Boolean withinIntUtil )
{
//os9err  err;
  process_typ* cp= &procs[ pid ];
  procid* pd= &cp->pd;
  const   funcdispatch_entry* fdeP= getfuncentry(func);
//char*   fName= fdeP->name; /* debug aid: function name visible in debugger */
//char*   fSS  = "";
  
  cp->lastsyscall= func; // remember for error tracking (for process)
  
  #ifdef THREAD_SUPPORT
    if (withinIntUtil) {
      // make the debug logging prep for systemcalls within int commands here
    //memcpy( (void*)&cp->os9regs,   (void*)rp,       sizeof(regs_type) );
      memcpy( (void*)&cp->os9regs.d, (void*)&rp->d, 5*sizeof(ulong32) );
      memcpy( (void*)&cp->os9regs.a, (void*)&rp->a, 6*sizeof(ulong32) );
      
      if (ptocThread) pthread_mutex_lock( &sysCallMutex );
      currentpid= pid;
    } // if
  #endif
  
  if (logtiming) {
    os9_to_xxx( pid );
        
//  if (fdeP->inregs & SFUNC_STATCALL) /* get the getstat/setstat code as name for debugging */
//    fSS= get_stat_name(loword(rp->d[1]));
  } /* if (logtiming) */


  /* execute syscall */
  if (withinIntUtil) {
    // make the debug logging for systemcalls within int commands here
    cp->func= func;
    debug_comein( rp, pid );
  } // if
  
  cp->oerr= (fdeP->func)( rp,pid );
  
  if (withinIntUtil) {
    // make the debug logging prep for systemcalls within int commands here
  //memcpy( (void*)&cp->os9regs,   (void*)rp,       sizeof(regs_type) );
    memcpy( (void*)&cp->os9regs.d, (void*)&rp->d, 5*sizeof(uint32_t) );
    memcpy( (void*)&cp->os9regs.a, (void*)&rp->a, 6*sizeof(uint32_t) );

    if (func==F_Exit) { // for internal utilities, F$Exit returns until here !!
      rp->d[ 1 ]= cp->exiterr;
      
      #ifdef THREAD_SUPPORT
        if (ptocThread) pthread_mutex_unlock( &sysCallMutex );
      #endif
      
      return cp->oerr;
    } // if
  } // if

  if (logtiming) {
    xxx_to_arb( func, pid );
  } /* if (logtiming) */

  if (fdeP!=&invalidcall) {
    if     (func<NUMFCALLS) os9_long_inc( &pd->_fcalls, 1 ); /* "procs -a" uses it */
    else  { func-=0x80;
      if   (func<NUMICALLS) os9_long_inc( &pd->_icalls, 1 ); 
    }
  } // if
  
  if (withinIntUtil) {
    if (arbitrate) { 
      os9exec_loop( 0, true );
      arbitrate= false;
      currentpid= pid; // make sure it is not changed
    }
    else {
      debug_return( rp, pid, false );
    } // if
  } // if
  
  #ifdef THREAD_SUPPORT
    if (withinIntUtil) {
      if (ptocThread) pthread_mutex_unlock( &sysCallMutex );
    } // if
  #endif
  
  return cp->oerr;
} /* exec_syscall */


// Callback entry for plugin trap0 calls
os9err trap0_call( ushort code, Regs_68k* regs )
{
  const int RegsSize= 16*sizeof( uint32_t ); /* 16 × 32-bit 68k regs = 64 bytes */
  process_typ* cp= &procs[ currentpid ];
   
  os9err    err;
  regs_type rp;
  
  if (code==I_Write ||
      code==I_WritLn) {
    arbitrate= true;
  } // if

  do {
    MoveBlk( (void*)&rp.d[ 0 ], (void*)regs, RegsSize ); // copy forth ...
  
    err= exec_syscall( code, currentpid, &rp, true );
  } while (cp->state==pWaitRead);
  
  MoveBlk  ( (void*)regs, (void*)&rp.d[ 0 ], RegsSize ); // ... and back
  
  return err;
} // trap0_call



void init_L2(void)
/* support for the /L2 led blinking */
{
	l2.hw_location= 0;
	
	l2.col1  = L2_Off;
	l2.ratio1= L2_InitRatio;
	l2.col2  = L2_Off;
	l2.ratio2= L2_InitRatio;
			
	l2.lcount= 0;
	l2.phase = 1; /* blink phases: either 1 or 2 */
} /* init_L2 */


static void update_L2( ulong t )
{
    byte   ll;
    ushort v;
    ulong  diff;
    
    /* make the /L2 interrupt handling */
    	diff= t-lastTick; lastTick= t;
    if (diff==0 || l2.lcount>diff)
    		       l2.lcount-= diff;
    else {
    	if (l2.phase==1) {
    		l2.phase      = 2;
    		l2.lcount     = l2.ratio2;
    		ll            = l2.col2;
    	}
    	else {
    		l2.phase      = 1;
    		l2.lcount     = l2.ratio1;
     		ll            = l2.col1;
    	} // if
    	
    	    v= os9_word( ll );
    	if (v!=l2.hw_location) 
    	       l2.hw_location= v; 
    } /* if */
} /* update_L2 */



/* get system tick count and subtract the start tick */
ulong GetSystemTick(void)
{
    ulong t;

    #ifdef MACOS9
      t=  TickCount();
                  
    #elif defined windows32
      t= GetTickCount()/10; /* take 100 instead of 1000 on PC */

    #elif defined UNIX
      struct timeval  tv;

      gettimeofday( &tv, NULL );
      if (sec0==0) sec0= tv.tv_sec;
      t=         tv.tv_sec - sec0;
      t= 100*t + tv.tv_usec/10000;

    #else
      #error Not yet implemented GetSystemTick()
    #endif

    
    /* make the /L2 interrupt handling */
    update_L2( t );
    
    return t-startTick;
} /* GetSystemTick */



/* init syscall timing */
void init_syscalltimers(void)
{
    ushort       k;
    process_typ* cp;
    
    time( &timerstart );
    logtiming        = true;
    arbticks         = 0;
    arbnum           = 0;
    last_complete    = GetSystemTick();
    
        rw__idleticks= 0;   /* must be initialized too */
    sum_rw__idleticks= 0;
        slp_idleticks= 0;
    sum_slp_idleticks= 0;
    strcpy( systime_prog,"" );    /* invalidate it */
            systime_prog_ok= true;

    for (k=0; k<=NUMFCALLS;   k++) { fcall_time[k]= 0; fcall_num[k]= 0; }
    for (k=0; k<=NUMICALLS;   k++) { icall_time[k]= 0; icall_num[k]= 0; }

    for (k=0; k<MAXPROCESSES; k++) { cp= &procs[k];
                                     cp->pd._uticks= 0;
                                     cp->fticks    = 0;       cp->pd._fcalls= 0; 
                                     cp->iticks    = 0;       cp->pd._icalls= 0;
                                     cp->pd._sticks= os9_long(cp->fticks + cp->iticks); }

    for (k=0; k<MAX_OS9PROGS; k++) { strcpy( statistics[k].name,"" );
                                             statistics[k].ticks= 0;
                                             statistics[k].num  = 0; }
} /* init_syscalltimers */



/* time display */
static char *time_disp(ulong t)
{
    static char tbuf[24]; /* worst case is 19 incl. NUL -- see the snprintf below */
    ulong ms, secs, mins;
    
    
    /* calc milliseconds */
    ms  =(ulong)((double)t/(double)TICKS_PER_SEC*1000.0);   
    secs= ms/1000;
    mins= secs/60;
    
    /* (uint32_t) casts, not "%llu": these are `ulong`, which is `unsigned long`
     * on LP64 but `unsigned long long` on Windows -- so no single length modifier
     * fits both. The values are elapsed-time components, comfortably 32-bit.
     * (Only mingw ever complained; on LP64 `%lu` happens to line up.) */
    /* snprintf, and a buffer sized for the real worst case. The minutes field
     * has NO width limit, so "%0u'%02u.%03u\"" needs up to
     * 10 digits + ' + 2 + . + 3 + " + NUL = 19 bytes -- the old tbuf[13]
     * overflowed once minutes reached 5 digits, i.e. after roughly 10000
     * minutes (~7 days) of accumulated time, which a long-running emulator
     * session reaches for real. Caught by GCC's -Wformat-overflow on a 32-BIT
     * build; no 64-bit target diagnosed it, the value-range analysis differs. */
    if (ms==0) tbuf[0]= NUL; /* was sprintf(tbuf,"") -- an empty format string */
    else {
        if (mins==0) snprintf( tbuf, sizeof(tbuf),     "%2u.%03u\"",
                               (uint32_t)(secs % 60), (uint32_t)(ms % 1000) );
        else         snprintf( tbuf, sizeof(tbuf), "%0u'%02u.%03u\"", (uint32_t)mins,
                               (uint32_t)(secs % 60), (uint32_t)(ms % 1000) );
    }

    return tbuf;
} /* time_disp */



static void show_title( Boolean show, char* name )
{
    char* n= name; 
    char* u= "-------------------";
    
    if (show) {
        if (*n==NUL) n= "Name";
        
        upho_printf("\n");
        upho_printf(" %-19s       ticks  # of calls       %%        time \n", n );
        upho_printf(" %-19s  ----------  ----------  ------  -----------\n", u );
    }
} /* show_title */



/* <n> is int64_t, not int: it carries the -1 "no count to show" sentinel (so it
   must stay SIGNED), while every caller passes a ulong counter -- as an int
   those were truncated to 32 bits, and a count past 2^31 would have printed as
   negative or matched the sentinel by accident. */
static void show_line( Boolean show, ushort mode, 
                       char c, char* name, ulong t, int64_t n, ulong active, int ticksLim )
{
    #define Lim  0.051
    #define GLim 1
    char    nnnn[20], perc[20];
    float   f;
    
    if (!show || (mode!=0 && n==0)) return;

    if (active==0) f= 0; /* no division by zero */
    else           f= (float)100*t/active;

    /* ticksLim is signed and t is not: comparing them directly converted a
       NEGATIVE limit into a huge unsigned one, which filtered out every line
       instead of none. Guard the sign, then compare like with like. */
    if ((mode & STIM_TICKAVAIL) && ticksLim>0 && t<(ulong)ticksLim) return;
    if ((mode & STIM_PERCENT  ) && f<GLim    ) return;
    
    /* snprintf: both fit today (an int is at most 11 chars, and `f` is a
     * percentage of tick counts so it stays well inside 20), but that safety
     * rests on reasoning about value ranges rather than on the call itself.
     * Bound them structurally so a future change to `f`'s computation cannot
     * turn this into an overflow. */
    if (n==-1) strcpy  ( nnnn,"" );
    else       snprintf( nnnn, sizeof(nnnn), "%lld", (long long)n );
    if (f<Lim) strcpy  ( perc,"-" );
    else       snprintf( perc, sizeof(perc), "%1.1f%%", f );
        
    /* t is `ulong` (a tick count) -- cast, don't re-spell the format: see time_disp */
    upo_printf("  %c%-19s  %10u  %10s %7s %12s\n",
               c,name, (uint32_t)t, nnnn, perc, time_disp(t) );
} /* show_line */


static void show_sline( char* name,  ulong t, int64_t n, ulong active )
{           show_line ( true,0, ' ', name, t,n, active, 1 );
} /* show_sline */



/* show system call timers */
/* <t>/<n> are ulong OUT params, matching the ulong counters they read. They
   used to be int*, which truncated a 64-bit tick total to 32 bits and, once
   bit 31 was set, produced a NEGATIVE count that the ulong comparisons below
   then read as enormous -- so the "pick the largest" sort selected the wrong
   entry. Same LP64 truncation class as the earlier host-pointer fixes. */
static void Get_FI_tn( ushort k, Boolean icalls, ulong *t, ulong *n )
{
  *t= icalls ? icall_time[ k ] : fcall_time[ k ];
  *n= icalls ? icall_num [ k ] : fcall_num [ k ];
} // Get_FI_tn


static void show_syscalltimers( Boolean icalls, ushort mode, ulong active, 
                                                ulong *call, ulong *num, int ticksLim )
{
  ushort   k, j, kInd;
  ulong    mxTicks, mxNum;
  ulong    t, n;
  char*    name;
  Boolean  kDone[ NUMFCALLS ]; // NUMFALLS is larger than NUMICALLS
  Boolean  sort= mode & STIM_ORDERED;
  
  Boolean  show=    icalls ? mode & STIM_ICALLS : mode & STIM_FCALLS;
  ushort   max =    icalls ?          NUMICALLS :          NUMFCALLS;
  show_title( show, icalls ?            "I$XXX" :            "F$XXX" );

  *call= 0;
  *num = 0;

  if (sort) {
    for ( k=0; k<max; k++ ) kDone[ k ]= false;
  } // if

  for   ( k=0; k<=max; k++ ) {
    kInd= k;

    if (sort && k<max) {
      mxTicks= 0; mxNum= 0;
           
      for ( j=0; j<max; j++ ) {
        if  (!kDone[ j ]) {
          Get_FI_tn( j, icalls, &t, &n );
          
          if (t==mxTicks && // do this first
              n>=mxNum  ) {             mxNum= n; kInd= j; }
          if (t >mxTicks) { mxTicks= t; mxNum= n; kInd= j; }
        } // if
      } // for
          
      kDone[ kInd ]= true;
    } // if

  //t= icalls ? icall_time[k] : fcall_time[k];
  //n= icalls ? icall_num [k] : fcall_num [k];
        
    Get_FI_tn( kInd,     icalls, &t, &n );
    if (kInd==max) name= icalls ? "I$xxx (other progs)":"F$xxx (other progs)";
    else           name= icalls ?  icalltable[kInd].name  : fcalltable[kInd].name;
        
    show_line( show,mode, ' ',name, t,n, active, ticksLim );
    if (n>0) { *call+= t; *num+= n; }
  } /* for */
} /* show_syscalltimers */



/* show system call timers */
static void Get_Statistics_tn( ushort k, st_typ** s, ulong *t, ulong *n ) /* see Get_FI_tn */
{
       *s= &statistics[ k ];
  *t= (*s)->ticks;
  *n= (*s)->num;
} // GetStatistics_tn


static void show_os9timers( ushort mode, ulong active, ulong *call, ulong *num, int ticksLim )
{
  ushort  k, j, kInd;
  ulong   mxTicks, mxNum;
  st_typ* s;
  ulong   t, n;
  Boolean kDone[ MAX_OS9PROGS ];
  Boolean sort= mode & STIM_ORDERED;
  Boolean show= mode & STIM_OS9;
    
  show_title( show,"OS-9" );
  *call= 0;
  *num = 0;
    
  if (sort) {
    for ( k=0; k<MAX_OS9PROGS; k++ ) kDone[ k ]= false;
   } // if

  for   ( k=0; k<MAX_OS9PROGS; k++ ) {
    kInd= k;
        
    if (sort) {
      mxTicks= 0; mxNum= 0;
           
      for ( j=0; j<MAX_OS9PROGS; j++ ) {
        if (!kDone[ j ]) {
          Get_Statistics_tn( j, &s, &t, &n );
          
          if (t==mxTicks && // do this first
              n>=mxNum  ) {             mxNum= n; kInd= j; }
          if (t >mxTicks) { mxTicks= t; mxNum= n; kInd= j; }
        } // if
      } // for
          
      kDone[ kInd ]= true;
    } // if
     
    Get_Statistics_tn( kInd, &s, &t, &n );   
        
    show_line( show,mode, s->intern ? '>':' ', s->name, t,n, active, ticksLim );
    if (n>0) { *call+= t; *num+= n; }
  } /* for */
} /* show_os9timers */



/* show timing summary */
ulong show_timing( ushort mode, int ticksLim, Boolean doDisable )
{
    ulong   tfcall, ticall, oscall;
    ulong   tfnum,  tinum,  osnum;
    ulong   t_call, active;
    ulong   realtime;
    time_t  now;
    Boolean sOpt= mode & STIM_S;

    time(&now);
    upho_printf( "%s timing (in 1/%d sec)\n", OS9exec_Name(),TICKS_PER_SEC );
  //upho_printf( "%s\n", hw_name );
    
    if (!logtiming && !doDisable) {
        upo_printf("**** WARNING: timing log is currently DISABLED!\n");
    }

    show_syscalltimers( false, 0,   1,       &tfcall,&tfnum, ticksLim );
    show_syscalltimers( true,  0,   1,       &ticall,&tinum, ticksLim );
    show_os9timers    (        0,   1,       &oscall,&osnum, ticksLim );

    t_call= tfcall+ticall;
    active= t_call+oscall + arbticks + sum_rw__idleticks + sum_slp_idleticks;
    
    if (active==0) active=1; /* prevent division by zero */
    
    show_syscalltimers( false, mode, active, &tfcall,&tfnum, ticksLim );
    show_syscalltimers( true,  mode, active, &ticall,&tinum, ticksLim );
    show_os9timers    (        mode, active, &oscall,&osnum, ticksLim );
        
    if (!logtiming) timerstart= now+1;  /* prevent uninitialized <timerstart> */
    if (now<timerstart) now= timerstart;
    
        realtime= (now-timerstart)*TICKS_PER_SEC;         /* avoid lost ticks */
    if (realtime<active) realtime= active;       /* avoid negative lost ticks */


    show_title  ( true,  hw_name );
    show_sline  ( "TOTAL F$xxx",      tfcall,         tfnum, active );
    show_sline  ( "TOTAL I$xxx",      ticall,         tinum, active );
    show_sline  ( "TOTAL SysCall",    t_call,   tfnum+tinum, active );
    show_sline  ( "TOTAL OS-9",       oscall,         osnum, active );
    show_sline  ( "TOTAL arbitration",arbticks,      arbnum, active );
    
    if (sOpt) {
      show_sline( "TOTAL idle sleep", sum_slp_idleticks, -1, active );
      show_sline( "TOTAL idle (R/W)", sum_rw__idleticks, -1, active );
    }
    else {
      show_sline( "TOTAL idle",       sum_slp_idleticks
                                    + sum_rw__idleticks, -1, active );
    } // if
    
    show_sline  ( "TOTAL active",     active,         osnum, active );

    if (sOpt && realtime>active) {
      show_sline( "TOTAL realtime",   realtime,          -1,      0 );
      show_sline( "lost  ticks",      realtime-active,   -1,      0 );
    } // if
    
    return ticall+tfcall;
} /* show_timing */



/* internal command for timing management */

static void usage( char *name, _pid_ )
{
    upe_printf("Usage:    %s [options]\n",name);
    upe_printf("Function: OS9exec emulator timing utility\n");
    upe_printf("Options:  -r          switch on timing measurement & reset counters\n");
    upe_printf("          -d          disable   timing measurement\n");
    upe_printf("          -e          re-enable timing measurement (but no reset)\n");
    upe_printf("          -n=[<prog>] display info of OS-9 <prog> only\n");
    upe_printf("          -f          show F$xxx call timing list\n");
    upe_printf("          -i          show I$xxx call timing list\n");
    upe_printf("          -o          show OS-9 progs timing list\n");
    upe_printf("          -s          show I$xxx + F$xxx calls + OS-9 progs (same as -fio)\n");
    upe_printf("          -h          display highest tickcnt first\n" );
    upe_printf("          -t=[<n>]    a minimum of <n> ticks must have been counted\n");
    copyright ();
} /* usage */


os9err int_systime(ushort pid, int argc, char **argv)
{
    ushort  mode= STIM_NONE; /* set default options */
    char    *p;              /* command line scanning */
    char    opt;
    ushort  h;
    /* Initialised: only set when the tick-threshold option is parsed, but
     * show_timing() below is reached whenever ANY option set `mode`, so a
     * combination that sets mode without the threshold passed a garbage
     * limit into the `t<ticksLim` filter. 0 means "filter nothing", the
     * benign default. (clang scan-build, "Uninitialized argument value".) */
    int     ticksLim= 0;
    Boolean doDisable= false;
    
    /* get arguments and options, multiple options allowed (bfo) */
    for ( h=1; h<argc; h++ ) {
        p= argv[h];    
        if (*p=='-') { 
            p++;
            
            /* multiple options allowed now */
            while (*p!=NUL) {
                        opt= p[0];
                switch (opt) { /* no longer case sensitive */
                    case '?': usage(argv[0],pid); return 0;
                    
                    case 'r': init_syscalltimers();
     						  upho_printf("Activated and resetted timing measurement\n"); break;
                    case 'd': logtiming= false; doDisable= true;
                              upho_printf("Disabled timing measurement\n");   break;
                    case 'e': logtiming=  true;
                              upho_printf("Re-enabled timing measurement\n"); break;
                               
                    case 'n': switch (p[ 1 ]) {
                                  case '=': strncpy( systime_prog, &p[2], OS9NAMELEN-1 );  /* bounded: user input */
                                            systime_prog[OS9NAMELEN-1]= NUL;
                                            p= p+strlen(p)-1; /* skip rest */ break;
                                  default : strcpy( systime_prog,"" );
                              } // switch
                              break;
                              
                    case 'f': mode |= STIM_FCALLS; break;
                    case 'i': mode |= STIM_ICALLS; break;
                    case 'o': mode |= STIM_OS9;    break;
                    case 's': mode |= STIM_S;      break;

                    case 't': mode |= STIM_TICKAVAIL; /* >=1 ticks */
                    
                              switch (p[ 1 ]) {
                                case '=': if (sscanf( &p[2],"%d", &ticksLim )<1) ticksLim= 1;
                                          p= p+strlen(p)-1; /* skip rest */   break;
                                default : ticksLim= 1;
                              } // switch
                              break;
                    
                    case 'p': mode |= STIM_PERCENT; break; /* >=1 percent */       
                    case 'h': mode |= STIM_ORDERED; break; /* highest cnt 1st */   
                    
                    default : upe_printf("Error: unknown option '%c'!\n",*p); 
                              usage(argv[0],pid); return 1;
                }
                
                p++;
            } /* while */
        }
        else {
            upe_printf("Error: no arguments allowed\n"); return 1;
        }
    } // for
       
    if (mode!=STIM_NONE) show_timing( mode, ticksLim, doDisable );
    return 0;
} // int_systime

    
/* eof */
