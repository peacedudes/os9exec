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
/* (c) 1993-2006 by Lukas Zeller, CH-Zuerich  */
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
 *    Revision 1.61  2007/04/10 22:11:51  bfo
 *    Internal utils are treated somewhat special
 *    => can't active nativce's father
 *
 *    Revision 1.60  2007/04/07 09:14:27  bfo
 *    - 'AssignNewChild' visible from outside now
 *    - Starting new processes in pStart state
 *    - 'AssignNewChild' now called at the 'correct' location
 *    - Text formatting beautified
 *
 *    Revision 1.59  2007/03/24 13:04:33  bfo
 *    "DoWait" is visible from outside
 *
 *    Revision 1.58  2007/03/10 12:39:21  bfo
 *    Params corrected: debug_return( &sigp->os9regs, spid, true )
 *
 *    Revision 1.57  2007/02/22 23:10:10  bfo
 *    sigp->plugElem->call_Intercept
 *
 *    Revision 1.56  2007/02/11 14:50:39  bfo
 *    DoWait() / <geCnt> balancer added
 *
 *    Revision 1.55  2007/01/28 21:58:51  bfo
 *    <isPtoC> renamed to <isNative>
 *
 *    Revision 1.54  2007/01/07 13:34:12  bfo
 *    New variable <isPlugin> initialized
 *
 *    Revision 1.53  2007/01/04 21:27:23  bfo
 *    'HandleOneEvent' MPW problem fixed
 *
 *    Revision 1.52  2006/12/16 22:17:03  bfo
 *    loop breaker for internal commands
 *
 *    Revision 1.51  2006/12/01 20:07:05  bfo
 *    <consoleSleep> param for "HandleOneEvent" (reduce MacClassic load)
 *
 *    Revision 1.50  2006/11/04 23:35:40  bfo
 *    <procName> => <intProcName>
 *
 *    Revision 1.49  2006/11/01 11:44:39  bfo
 *    Clean up proc desc no longer needed, because int commands do
 *    not overwrite FPU regs with bad values anymore
 *
 *    Revision 1.48  2006/10/30 00:07:21  bfo
 *    "show" crash problem of MacOS9 fixed (memset 0 for proc vars)
 *
 *    Revision 1.47  2006/10/25 19:21:23  bfo
 *    Slow response eliminated: wait_time.tv_nsec= 1000000
 *
 *    Revision 1.46  2006/10/20 08:33:04  bfo
 *    idle load of OS9exec nearly reduced to 0 %
 *
 *    Revision 1.45  2006/10/15 13:26:33  bfo
 *    icpt_signal assignment problem fixed for internal utilities,
 *    still a problem for "clock2"
 *
 *    Revision 1.44  2006/10/01 15:36:49  bfo
 *    <cp->isPtoC> introduced; MAX_SLEEP support;
 *    debugging the missing signal problem
 *
 *    Revision 1.43  2006/09/03 20:50:08  bfo
 *    No mask levels below 0 any more
 *
 *    Revision 1.42  2006/08/29 22:40:37  bfo
 *    arbitration improvements
 *
 *    Revision 1.41  2006/08/04 18:37:31  bfo
 *    Comment changes
 *
 *    Revision 1.40  2006/07/29 09:07:34  bfo
 *    nanosleep() introduced, according to input of Martin Gregorie
 *
 *    Revision 1.39  2006/07/23 14:28:30  bfo
 *    <pBlocked> introduced
 *
 *    Revision 1.38  2006/07/21 07:29:16  bfo
 *    Up to date
 *
 *    Revision 1.37  2006/07/21 07:23:43  bfo
 *    Child assignment strategy (intUtils) corrected
 *
 *    Revision 1.36  2006/07/14 11:46:20  bfo
 *    Longer usleep for MacOSX (but idle load is still up ...)
 *
 *    Revision 1.35  2006/07/10 09:59:40  bfo
 *    <allowIntUtil> param for "do_arbitrate",
 *    svid param for "callcommand"
 *
 *    Revision 1.34  2006/07/06 22:58:01  MG
 *    function 'is_super' added (by Marin Gregorie)
 *
 *    Revision 1.33  2006/06/25 22:17:40  bfo
 *    Up to date
 *
 *    Revision 1.32  2006/06/11 22:05:24  bfo
 *    set_os9_state with 3rd param <callingProc>
 *
 *    Revision 1.31  2006/06/10 10:22:55  bfo
 *    Some isIntUtil debugging made invisible
 *
 *    Revision 1.30  2006/06/01 21:03:29  bfo
 *    printf things commented out
 *
 *    Revision 1.29  2006/06/01 18:05:57  bfo
 *    differences in signedness (for gcc4.0) corrected
 *
 *    Revision 1.28  2006/02/19 16:38:37  bfo
 *    thread support added
 *
 *    Revision 1.27  2005/06/30 11:57:42  bfo
 *    sig_mask adaption
 *
 *    Revision 1.26  2004/11/20 11:42:31  bfo
 *    System load problem because of zombie fixed.
 *
 *    Revision 1.25  2003/06/09 21:55:06  bfo
 *    Signals can be sent only to process ids < MAXPROCESSES
 *
 *    Revision 1.24  2003/05/26 08:27:02  bfo
 *    buggy "list" V2.4 and "cmp" V3.0 diabled ( E_BADREV )
 *
 *    Revision 1.23  2003/01/10 21:01:09  bfo
 *    pWaitRead problems fixed
 *
 *    Revision 1.22  2003/01/09 21:58:55  bfo
 *    pWaitRead things adapted
 *
 *    Revision 1.21  2003/01/03 14:57:20  bfo
 *    Store the signals during read the same way as in real OS-9, add pwr_brk flag
 *
 *    Revision 1.20  2003/01/02 14:34:52  bfo
 *    Some type castinmg things fixed
 *
 *    Revision 1.19  2003/01/02 12:21:28  bfo
 *    RTE registers correctly saved and restored
 *
 *    Revision 1.18  2002/11/06 20:13:13  bfo
 *    lastsignal->pd._signal/icptroutine->pd._sigvec (directly defined at pd struct)
 *
 *    Revision 1.17  2002/10/27 23:50:41  bfo
 *    Sibling handling of internal commands corrected ("idevs!head" bug)
 *
 *    Revision 1.16  2002/10/16 17:50:36  bfo
 *    DevPak 68K OS-9 V1.2 _resvd1, _procstk conflict resolved
 *
 *    Revision 1.15  2002/09/11 17:25:53  bfo
 *     Definition is correct now: pd= &procs[k].pd
 *
 *    Revision 1.14  2002/09/01 20:15:39  bfo
 *    big/little endian bug with *idp fixed
 *
 *    Revision 1.13  2002/09/01 17:54:51  bfo
 *    some more variables of the real "procid" record used now
 *
 *    Revision 1.12  2002/08/13 21:55:46  bfo
 *    grp,usr and prior at the real prdsc now
 *
 *    Revision 1.11  2002/08/13 21:24:17  bfo
 *    Some more variables defined at the real procid struct now.
 *
 *    Revision 1.10  2002/08/09 22:39:21  bfo
 *    New procedure set_os9_state introduced and adapted everywhere
 *
 *    Revision 1.9  2002/08/08 21:54:43  bfo
 *    F$SetSys extended with D_PrcDBT support
 *
 *
 */


#include "os9exec_incl.h"

#if defined __EMSCRIPTEN__
  #include <emscripten.h>   /* emscripten_sleep: the browser's idle wait in DoWait() */
#endif

#if defined UNIX && !defined MINGW
  #include <sys/select.h>   /* select()/fd_set for the interactive idle wait in DoWait() */
  #include <signal.h>       /* sigprocmask(): the tick is held off around that select */
#endif

/* process routines */
/* ================ */

int is_super(ushort pid)
/* Returns true if process belongs to super user.
 *
 * Group ZERO alone, not 0.0 -- the user half is irrelevant. Microware
 * defines it twice: "A user with a group ID of 0 is referred to as a super
 * user. A super user can access and manipulate any file or directory on the
 * system regardless of the file's ownership" (Training, OS-9 Starter, Shell
 * chapter) and "A super user process is any process owned by group zero"
 * (Training, OS-9 Advanced, system globals chapter). The Starter volume then
 * makes it concrete with a password-file line, `amy,love,0.153,...` -- "if
 * user amy enters love at the password prompt, she will have super user
 * privileges" -- so 0.153 IS a super user and only the group half decides.
 * This used to demand BOTH halves zero; a group-0 account with a non-zero
 * user number was silently an ordinary user (live-verified before the fix:
 * 0.153 got E_FNA reading a 1.7-owned file with no public bits, where 0.0
 * read it fine).
 *
 * NOTE this is a wider test than it looks: it is the ONLY privilege check in
 * the codebase, gating RBF permission enforcement (file_rbf.c) and OS9STOP
 * (intcommand.c). Anything that lets a non-super process reach group 0 is
 * therefore a privilege escalation -- see pRsetFD, which must keep a
 * non-super owner from writing a zero group into a file descriptor's owner
 * word.
 *
 * Not to be confused with the TRM's F$SUser rule, which is about who may
 * CHANGE identity ("user number 0.0 may change their ID to anything without
 * restriction") rather than what an identity is privileged to do. */
{
   process_typ*   cp = &procs[pid];
   int            reply = 0;

   if (cp->pd._group == 0)
      reply = 1;

   return reply;
}

void show_processes(void)
/* show processes */
{
    int          k;
    char         sta;
    mod_exec*    mod;
    char         idstr[10], mIDs[10];
    char*        mName;
    process_typ* cp;
    
    upo_printf(" Id S PId SId CId MId  Module   Prior  MemStart  MemEnd   Last Syscall Name\n");
    upo_printf("--- - --- --- --- --- --------- ----- --------- --------- ------------ ----------------\n");     
    for (k=0; k<MAXPROCESSES; k++) {
            cp= &procs[k];
        if (cp->state!=pUnused) {
            mod= get_module_ptr(cp->mid);
            switch (cp->state) {
                case pActive   : sta='A'; break;
                case pDead     : sta='D'; break;
                case pSleeping : sta='S'; break;
                case pWaiting  : sta='W'; break;
                case pSysTask  : sta='T'; break;
                case pWaitRead : sta='S'; break;
                case pWaitWrite: sta='S'; break;
                default        : sta='?'; break;
            } // switch
            if (cp->isIntUtil)   sta='I';

            if (k==currentpid) mName= "iprocs";
            else {             mName= "<none>";
                if (mod!=NULL) mName= Mod_Name( mod );
            }

            snprintf( idstr,sizeof(idstr),"%c%d", k==currentpid ? '*' : ' ', k );
            if (cp->mid==MAXMODULES) strcpy( mIDs,"-" );
            else                    snprintf( mIDs,sizeof(mIDs), "%d", cp->mid );
            
            upo_printf("%3s %c %3d %3d %3d %3s $%08X %5d $%08X $%08X %-12s %s\n",
                        idstr,
                        sta,
               os9_word(cp->pd._pid),
               os9_word(cp->pd._sid),
               os9_word(cp->pd._cid),
                        mIDs,
                        (mod && !os9modules[cp->mid].isBuiltIn) ? TO68K(mod) : 0u,
               os9_word(cp->pd._prior),
                        cp->memstart,
                        cp->memtop,
                        get_syscall_name(cp->lastsyscall),
                        mName);
        }
    }
} /* show_processes */


void init_processes()
/* initialize process descriptors */
{
    short*  s;
    procid* pd;
    int     j, k;
    ulong   sz= sizeof(procid);
    
    for (k=0; k<MAXPROCESSES; k++) {   /* assign all the constant values which never change */
        pd= &procs[ k ].pd; 
        memset( pd,0, sz );            /* cleanup the whole proc descriptor */
        
        pd->_id    = os9_word(k);
        pd->_sp    = 0;                /* just to be sure */
        
        pd->_pagcnt= 0;
        pd->_age   = os9_word(128);    /* as in real OS-9 */
        pd->_task  = 0;
     // pd->_resvd1= os9_word(0xBD00); /* invisible at DevPak von 68K OS-9 V1.2 */
        pd->_deadlk= 0;                /* as in real OS-9 */
        /* Give this process its slice of the arena-resident signal scratch, then
         * publish its 68k address. Previously this took TO68K() of a field inside
         * the HOST procs[] global -- memory outside the arena entirely, so the
         * address handed to the guest's signal handler was garbage. */
        procs[k].sigdat= sigdat_arena + (size_t)k * SIG_SCRATCH;
        pd->_sigdat    = os9_long( TO68K(procs[k].sigdat) );

        /* clear all memory segments ... */
        for (j=0; j<32; j++) {
            pd->_memimg[j] = 0;
            pd->_blksiz[j] = 0;
        }

        pd->_frag  = 0;                /* don't use OS-9 V3.0 memory method */
        pd->_fragg = 0;

        pd->_data  = 0;
        pd->_datasz= 0;

        for (j=0; j<   7; j++) pd->FPExcpt [j]= 0;
        for (j=0; j<   7; j++) pd->FPExStk [j]= 0;
     // for (j=0; j<1168; j++) pd->_procstk[j]= NUL; /* invisible at DevPak von 68K OS-9 V1.2 */
        
        set_os9_state( k, pUnused, "init_process" ); /* invalidate this process  */
        procs[ k ].isIntUtil= false;
        procs[ k ].isPlugin = false;
        procs[ k ].plugElem = NULL;
        prDBT[ k ]= 0;                   /* and also the table entry */
    } /* for */
    
                                  s= (short*)prDBT;
    *s= (short)os9_word(MAXPROCESSES-1); s++; /* no process 0 */
    *s= os9_word((ushort)sz);     s++; /* the size of the real descriptor */
    
    currentpid= 0; /* earlier: MAXPROCESSES; no process is running */
} /* init_processes */



os9err new_process(ushort parentid, ushort *newpid, ushort numpaths)
/* prepare a new process
 * Note: pid passed is the parent ID
 *       numpaths will not be updated (was once, but not required any more)
 */
{
    os9err       err;
    ushort       npid, k;
    process_typ  *cp, *pap;
	int			 dayOfWk, currentTick;
	uint32_t     timbeg, datbeg;
    
    /* --- find empty process descriptor */
    debugprintf(dbgProcess,dbgNorm,("# new_process: parent=%d wants to create child\n",parentid));
    
    /* start searching with nr 2 (bfo) */
    /* OS-9: process 0 does not exist, process 1 is the kernel */
    for (npid=1;  npid<MAXPROCESSES; npid++) {
        if (procs[npid].state==pUnused) {
            /* this process descriptor is free, use it */
            cp= &procs[ npid ];
          //memset( cp,0, sizeof(procid) ); /* cleanup the whole proc descriptor */
            
            /* initialize link to traphandler table, first entry=TRAP 1 */
            cp->os9regs.ttP=&(cp->TrapHandlers[0]);
            /* initialize flags */
            cp->os9regs.flags=
                (llm_fpu_present()      ? FLAGS_FPU : 0) |
                (llm_runs_in_usermode() ? FLAGS_UM  : 0);
            /* initialize registers */
            for (k=0; k<8; k++) {
                cp->os9regs.d[k]=0xDDDDDDD0+k;
                cp->os9regs.a[k]=0xAAAAAAA0+k;
                cp->os9regs.pc=0xCCCCCCCC;
            }
            #ifdef USE_UAEMU
            /* make sure that ISP ist ready for exception stack frames */
            cp->os9regs.isp= trapstack_isp;
            #endif
            
            /* there was no last systemcall */
            cp->func       = STARTCALL;
            cp->lastsyscall= STARTCALL;
            cp->dbgfunc    = STARTCALL; /* nothing traced yet either */
            cp->dbgpending = false;
            
            /* reset statistics */
            cp->pd._uticks= 0;
            cp->fticks    = 0;
            cp->iticks    = 0;
            cp->upend     = 0;
            cp->stalled   = false; /* no parked pipe request from a slot's last owner */
            cp->unitRestLen= 0;    /* nor a line ending half written to a /tN */
            cp->pd._sticks= os9_long(cp->fticks + cp->iticks);
            
            /* julian time and date */
			Get_Time( &timbeg,&datbeg, &dayOfWk,&currentTick, false,false );
            cp->pd._datbeg= os9_long(datbeg);
            cp->pd._timbeg= os9_long(timbeg);
			
            cp->pd._fcalls= 0; /* reset counters */
            cp->pd._icalls= 0;
            cp->pd._rbytes= 0;
            cp->pd._wbytes= 0;
            
            strcpy( cp->intProcName, "" ); /* used for internal utilities only */
            cp->exiterr    = E_PRCABT;     /* process aborted if no other code is set (through F$Exit e.g.) */
            cp->pd._pid    = os9_word( parentid ); /* remember parent */
            cp->pd._sid    = 0;            /* has not yet siblings */
            cp->pd._cid    = 0;            /* has not yet children */
            cp->mid        = MAXMODULES;   /* no main module yet */
            cp->memstart   = cp->memtop=0; /* no memory yet */
            cp->pd._signal = 0;            /* no signal, rteregs invalid */
            cp->masklevel  = 0;            /* sigmask is disabled by default */
            cp->pwr_brk    = false;        /* pWaitRead break for signals <= 32 */
            cp->pd._sigvec = 0;            /* no intercept routine installed */
            cp->last_mco   = NULL;         /* last console buffer */
            cp->wakeUpTick = 0;            /* to be woken up */
            cp->pW_age     = 0;            /* sleep aging */

            /* Not waiting on any event. ev_id is what makes an event queue
               entry safe against a reused process slot -- the search honours a
               queued index only while that process still claims the same event
               -- so it has to be cleared HERE, where a slot becomes somebody
               new, and not merely when a wait ends. */
            cp->ev_id      = 0;
            cp->ev_next    = MAXPROCESSES;
            cp->ev_minV    = 0;
            cp->ev_maxV    = 0;
            cp->ev_woken   = false;
            cp->ev_wakeValue= 0;
            cp->ev_wakeErr = 0;

            init_mem(npid);                /* init memory block list */
            init_traphandlers(npid);       /* init traphandlers */
            init_usrpaths    (npid);       /* init user paths */
            init_exceptions  (npid);       /* init exception handlers */

            cp->stdoutfilter= NULL;        /* no stdout filter function yet */
            cp->filtermem   = NULL;        /* no memory allocated for filter yet */
            
            /* --- inherit from parent (if any) */
            if (parentid>0 &&
                parentid<MAXPROCESSES)
                /* --- inherit from parent */
                 pap= &procs[parentid];
            else pap= &procs[1];
            
            if (npid>1) { /* nothing to inherit for the kernel process */
                /* inherit default dirs */
                cp->d.type = pap->d.type;
                cp->d.dev  = pap->d.dev; 
                cp->d.lsn  = pap->d.lsn;
                /* memcpy of the WHOLE field, not strncpy: source and
                   destination are both exactly OS9PATHLEN and every producer
                   of a .path now NUL-terminates it, so copying the field
                   entire carries the terminator with it. strncpy here either
                   dropped the terminator (bound == size) or truncated by one
                   (bound == size-1); the field copy does neither. */
                memcpy( cp->d.path, pap->d.path, OS9PATHLEN );
            
                #ifdef macintosh
                  cp->d.volID= pap->d.volID; 
                  cp->d.dirID= pap->d.dirID;
                #endif
            
                cp->x.type = pap->x.type;
                cp->x.dev  = pap->x.dev;
                cp->x.lsn  = pap->x.lsn;
                memcpy( cp->x.path, pap->x.path, OS9PATHLEN );
            
                #ifdef macintosh
                  cp->x.volID= pap->x.volID; 
                  cp->x.dirID= pap->x.dirID;
                #endif
            
                if (parentid>0 &&
                    parentid<MAXPROCESSES) {
                
                //  strncpy( cp->d.path, pap->d.path, OS9PATHLEN );
                //  strncpy( cp->x.path, pap->x.path, OS9PATHLEN );
                
                    /* inherit standard paths */
                    if (numpaths>MAXUSRPATHS) return os9error(E_BPNUM); /* too many paths */
                    for (k=0; k<numpaths; k++) {
                        cp->usrpaths[k]= pap->usrpaths[k];
                        err=usrpath_link( npid, k, "fork " );
                
                        if (err && debugcheck(dbgProcess,dbgNorm)) {
                            uphe_printf("new_process: pid %d inherited inactive path from pid=%d: usrpath=%d (syspath=%d)\n",npid,parentid,k,cp->usrpaths[k]);
                            debug_halt( dbgProcess );
                        }
                        if (err) {
                            /* inheriting closed paths is allowed, simply pass them to child as unused */
                            cp->usrpaths[k]= 0; /* forget that one */
                        }
                    }
                }
                else {
                //  strncpy( cp->d.path, startPath, OS9PATHLEN );
                //  strncpy( cp->x.path, startPath, OS9PATHLEN );
                //
                //      p= egetenv("OS9DISK"); /* get path for default exe dir */
                //  if (p!=NULL) strncpy( cp->d.path,p, OS9PATHLEN );
                //
                //      p= egetenv("OS9CMDS"); /* get path for default exe dir */
                //  if (p!=NULL) strncpy( cp->x.path,p, OS9PATHLEN );
                //  else {
                //      strncpy( cp->x.path, cp->d.path,OS9PATHLEN-6);
                //      strcat ( cp->x.path, PATHDELIM_STR );
                //      strcat ( cp->x.path, "CMDS"        );
                //      strcat ( cp->x.path, PATHDELIM_STR );
                //  }

                    /* by default there is just one systempath */
                    cp->usrpaths[0]= sysStdin;
                    cp->usrpaths[1]= sysStdin;
                    cp->usrpaths[2]= sysStdin;
                
                    /* inherit stdin/stdout/stderr from MPW */
                    if (numpaths>1) cp->usrpaths[1]=sysStdout;
                    if (numpaths>2) cp->usrpaths[2]=sysStderr;
                }
            } /* if (npid>1) */
            
            /* --- process descriptor prepared ok */
            debugprintf(dbgProcess,dbgNorm,("# new_process: created pid=%d\n",npid));
            /* --- its only here where we activate the process */
            /* correct state must be set afterwards, or process will be dead from start */
            set_os9_state( npid, pStart, "new_process" );
          //printf( "toedlich %d\n", npid );
            *newpid=npid; /* return new PID */
            return 0;
        }
    }
    /* all processes used, can't prepare new process */
    return os9error(E_PRCFUL);
} /* new_process */



void AssignNewChild( ushort parentid, ushort pid )
{
  ushort  idp_sv;
  ushort* idp= &procs[parentid].pd._cid;
  int     n= 0;
  
  while (true) {
        idp_sv= os9_word( *idp );
    if (idp_sv==0) break;
      
    if (idp_sv==pid) {            
      *idp= procs[ pid ].pd._sid;
      debugprintf( dbgProcess,dbgNorm,( "# Assign new child: pid=%d\n",os9_word(*idp) ) );
      return;
    } // if
        
    idp= &procs[ idp_sv ].pd._sid;
    if ( os9_word( *idp )==idp_sv ) break; // no change
    
        n++;
    if (n>MAXPROCESSES) break;
  } // loop
} /* AssignNewChild */



/* The process os9exec was started with (prepLaunch), and whether it still
   runs. Once it has gone, os9exec ends as soon as every process left is only
   waiting for input -- see ShutdownDue. */
ushort  launch_pid  = 0;
Boolean launch_alive= false;

void spf_abort_request( ushort pid ); /* spfsock.c */

static void sig_queue_forget( ushort pid )
/* <pid> is gone: its queued signals go with it. Left in sig_queue they went
   to the NEXT process given its pid -- a fresh command killed by an old
   alarm's code -- and a queue kept full by them refused everyone else's
   F$Send (kernel review). sig_mask takes one entry per call, so a process
   that ends masked can leave many. */
{
    sig_typ* s= &sig_queue;
    int      i, j= 0;

    for (i=0; i<s->cnt; i++) {
        if (s->pid[ i ]==pid) continue;
        s->pid   [ j ]= s->pid   [ i ];
        s->signal[ j ]= s->signal[ i ];
        j++;
    }
    s->cnt= j;
    if (s->cnt<=0) async_pending= false;
} /* sig_queue_forget */

os9err kill_process( ushort pid )
/* kill a process
 * Note: exiterr must be set before calling kill_process (by F_Exit or F_Kill)
 */
{
    const        funcdispatch_entry *fdeP;
    process_typ* cp= &procs[pid];
    process_typ* kp;
    regs_type*   rp;
    ushort       parentid,k;
    
  //upe_printf( "kill id=%d %d\n", pid, cp->state );
    if (pid==launch_pid) launch_alive= false;
    cp->masklevel= 0;
    
    if (cp->state==pUnused) return os9error(E_IPRCID); /* process does not exist */
    debugprintf(dbgProcess,dbgNorm,("# kill_process: killing pid=%d, parentid=%d, exiterr=%d\n",
                                       pid, os9_word(cp->pd._pid),cp->exiterr));

    /* remove some more resources */
    spf_abort_request  ( pid ); /* a send it left part done is nobody's now */
    sig_queue_forget   ( pid ); /* and its queued signals nobody's either */
    close_usrpaths     ( pid );
    debugprintf(dbgProcess,dbgNorm,("# kill_process: usrpaths closed\n" ));
    unlink_traphandlers( pid );
    debugprintf(dbgProcess,dbgNorm,("# kill_process: traphandlers unlinked\n" ));
  //cp->isIntUtil= false;
    
    /* first orphan eventual children of the process */
    for (k=0; k<MAXPROCESSES; k++) {
    			   kp= &procs[k];
        if        (kp->state!=pUnused &&
          os9_word(kp->pd._pid)==pid) {
                   kp->pd._pid= 0; /* earlier: MAXPROCESSES */
            if    (kp->state==pDead) set_os9_state( k, pUnused, "kill_process" );
            debugprintf(dbgProcess,dbgNorm,("# kill_process: orphaned child pid=%d\n",k));
        }
    }

    /* A child this process was debugging (F$DFork) is its to end, with
       F$DExit; nobody else may. Left alone it slept forever, and it kept this
       pid as its debugger -- so whoever was given this pid next passed the
       parent checks of F$DExec and F$DExit and could drive it. It ends here,
       as its debugger's own F$DExit would have ended it. */
    for (k=0; k<MAXPROCESSES; k++) {
        if (k!=pid && dbg_parent_pid[k]==pid && procs[k].state!=pUnused) {
            dbg_parent_pid[k]  = 0;
            dbg_step_pending[k]= 0;
            if (procs[k].state!=pDead) kill_process( (ushort)k );
            if (procs[k].state==pDead) set_os9_state( (ushort)k, pUnused, "kill_process (debugger gone)" );
        }
    }
    if (cp->state==pDead) return os9error(E_IPRCID); /* avoid killing again, because double close is not good */
    debugprintf(dbgProcess,dbgNorm,("# kill_process: set to unused\n" ));

    /* A process killed while parked in Ev$Wait is still linked into that
       event's queue. The search would drop the entry by itself the next time
       it walked past -- it honours an entry only while the process still
       claims the event, and a reused slot starts with ev_id 0 -- so nothing
       depends on this call for correctness. It is here so the queue stops
       naming the dead at the moment they die, rather than whenever somebody
       next signals: state that is only ever cleaned up lazily is state that
       is wrong in every debugger dump taken in between. */
    evDequeue( pid );

    /* now kill the process */
        parentid= os9_word(cp->pd._pid); /* get parent ID */
    debugprintf(dbgProcess,dbgNorm,("# kill_process: parentid=%d\n", parentid ));
    
    if (parentid>0 &&
        parentid<MAXPROCESSES) {
    //printf( "dead id=%d mask=%d\n", pid, cp->masklevel );
      /* there is a parent process */
      set_os9_state( pid, pDead, "kill_process" ); /* keep it there until parent recognizes */
      if (pid==interactivepid) interactivepid= parentid; /* direct Cmd-'.' to parent now */
    //if (!cp->isIntUtil ||
    //     cp->isNative)  AssignNewChild( parentid,pid );
//    if (!cp->isIntUtil) AssignNewChild( parentid,pid );
    }
    else {
    //upe_printf( "unused id=%d\n", pid );
      set_os9_state( pid, pUnused, "kill_process" ); /* there's no parent => invalidate descriptor */
    } // if

    /* If a debug parent is waiting on this child, wake it now. Dying inside
       F$DExec is the call's error return: "If the child process terminates
       for any reason, the carry bit is set and returned" (page 1-14), E$PrcAbt
       being the one listed error that says so. The step it had armed goes with
       it -- left armed, the next instruction run was somebody else's. The
       child keeps its debug parent, which still owes it an F$DExit; the slot
       lets go of it when it is freed (set_os9_state). */
    if (dbg_parent_pid[pid] != 0 &&
        procs[dbg_parent_pid[pid]].state == pSleeping) {
        if (dbg_step_pending[pid]) {
            extern int m68k_os9singlestep;
            regs_type* prp= &procs[dbg_parent_pid[pid]].os9regs;

            dbg_step_pending[pid]= 0;
            m68k_os9singlestep   = 0;
            prp->sr|= CARRY;
            retword( prp->d[1] )= E_PRCABT;
        }
        set_os9_state(dbg_parent_pid[pid], pActive, "kill_process (dbg wake)");
    }
    
    debugprintf(dbgProcess,dbgNorm,("# kill_process: process killed\n" ));

    /* module can't be unlinked as long as process is not pDead or pUnused */
    unlink_module( cp->mid );
    cp->mid= MAXMODULES; /* invalidate it */
    
    if (pid==currentpid) {
        debugprintf(dbgProcess,dbgNorm,("# kill_process: switch to parentid=%d (%s)\n",
                                           parentid, PStateStr(&procs[parentid])));
        /* current process has gone, so activate its parent */
            currentpid= parentid; /* in case of wait, adapt d0+d1 */
        if (currentpid!=0) {
                fdeP= getfuncentry(procs[currentpid].func);
            if (fdeP->func==OS9_F_Wait) {
                debugprintf(dbgProcess,dbgNorm,("# kill_process: new d0=%d d1=%d\n",
                                               pid, cp->exiterr));
                rp= &procs[currentpid].os9regs;
                retword(rp->d[0])= pid;                /* ID of dead child */
                retword(rp->d[1])= cp->exiterr; /* exit error of child */
            }
            
            arbitrate= false; /* don't arbitrate, simply wake/unwait current if needed */
        }
        else
            arbitrate= true; /* not a valid ID -> search a new process */
    }

    /* now dispose all the process' resources */
    if (cp->last_mco!=NULL) {
        /* spP is NULL between a device's last path close and any reopen -- a
           /tN keeps its ttydev_typ (and this process's pointer to it) while
           hostterm_close() drops the syspath (hostterm.c). Killing a process
           that had written to such a device then dereferenced NULL here, which
           is exactly what shutdown does: close_syspaths runs before
           kill_processes, so ANY process still holding a last_mco for a /tN
           crashed cleanup. hostterm_poll already carries this same guard for
           the same reason; this was the other unguarded dereference.
           Nothing to disconnect when there is no path -- just drop the link. */
        if (cp->last_mco->spP!=NULL)
            cp->last_mco->spP->lastwritten_pid= 0; /* disconnect CtrlC/E signal */
        cp->last_mco= NULL;
    }
    debugprintf(dbgProcess,dbgNorm,("# kill_process: CtrlC/E signal disconnected\n" ));
    
    A_Kill  ( pid ); /* remove all alarms of this process */
    free_mem( pid ); /* IMPORTANT: use this only after all other resources are freed (because they might have memory allocated, such as traphandlers) */
    if (cp->stdoutfilter!=NULL) releasefilter(&cp->filtermem); /* release filter */
    debugprintf(dbgProcess,dbgNorm,("# kill_process: successfully killed pid=%d\n",pid));
    return 0;
} /* kill_process */


/* Set by sig_mask while it hands the OLDEST queued signal back to
   send_signal. When that signal cannot be delivered yet either, it goes back
   at the head of the queue, not the tail: appending it put it behind every
   signal sent after it, so two signals sent while masked reached the
   intercept routine last-in first-out. F$Send's queue is first-in first-out
   (CONF68K t69). */
static Boolean requeue_at_head= false;

/* send a signal to a process */
os9err send_signal( ushort spid, ushort signal )
{
  #if defined NATIVE_SUPPORT || defined PTOC_SUPPORT
    os9err err= 0;
    short  sv;
  #endif
	
  process_typ* cp  = &procs[proc_slot(currentpid)]; /* ptr to my procs dsc */
  process_typ* sigp;                     /* ptr to procs dsc to which the signal will be sent */
  sig_typ*     s   = &sig_queue;
  save_type*   svd;
  int          k;
  Boolean      wRead, ptocOK;
  regs_type*   v;
  
  /* the broadcast (send to pid=0) is implemented here */
  if (currentpid!=0 && spid==0) {
    for (k=1; k<MAXPROCESSES; k++) {
      if (cp->pd._group==procs[k].pd._group &&
          cp->pd._user ==procs[k].pd._user  &&
        currentpid!=k) send_signal( k,signal );
    } /* for */

	return 0;
  } /* if */

  if (spid>=MAXPROCESSES) return E_IPRCID;         /* check the validity of <spid> first */
  sigp= &procs[spid];               /* ptr to procs dsc to which the signal will be sent */

  if (currentpid==0) cp= sigp;
  debugprintf(dbgProcess,dbgNorm,("# send signal=%d to pid=%d (%s) from currentpid=%d\n",
                                     signal,spid, PStateStr(sigp), currentpid ));

  if (sigp->state==pUnused || /* bad process */
      sigp->state==pDead) return os9error(E_IPRCID);

  if (signal==S_Kill) {
    /* unconditionally kill process */
    sigp->exiterr= os9error(E_PRCABT); /* process aborted */
    kill_process( spid );
    return 0;
  } // if

  /* A process waking ITSELF is a no-op only while it is running: there is
     nothing to wake, and the old unconditional test said just that. But an
     alarm is delivered in the context of the process that armed it
     (CheckAlarms -> send_signal, alarms.c), so a process that arms
     A$Set with S$Wake and then sleeps IS its own sender -- and its wake was
     being thrown away here while it lay in pSleeping. F$Sleep's page says
     "the process is activated before the full time interval if a signal (in
     particular S$Wake) is received" and recommends an indefinite sleep as
     the way to wait for one (page 1 - 53), which is the pattern this broke:
     the sleep ran its full course, or forever. Measured with a process trace:
     "send signal=1 to pid=3 (pSleeping) from currentpid=3", dropped here.
     CONF68K t92 is the regression. */
  if (signal==S_Wake && spid==currentpid &&
      sigp->state!=pSleeping) return 0; /* already awake: nothing to do */
   
//if (sigp->isIntUtil) return 0; /* ignore this as well for the moment */
    
  /* signal will be delayed, if receiving process in pWaitRead mode */
  wRead= ( sigp->state==pWaitRead && !sigp->pwr_brk );
    
  /* some of the signals must be stored in a queue */
  if  (cp->way_to_icpt || sigp->masklevel>0 || wRead || !async_area) {
    if (signal==S_Wake && sigp->masklevel>0) return 0; /* needn't to be stored in this case */
        
    if (wRead && signal<=Pwr_Signal) sigp->pwr_brk= true; /* special break condition */
        
    s= &sig_queue; /* store it, but don't do anything */

    /* Bounds check BEFORE the store, not after. The count used to be tested
       only when deciding whether to advance it, so with the queue already full
       the two stores below still ran at index MAXSIGNALS: pid[MAXSIGNALS] is
       signal[0] (the arrays are adjacent in sig_typ), and signal[MAXSIGNALS] is
       past the end of sig_queue altogether -- a global-buffer overflow that
       also silently corrupted the oldest queued entry. Every later signal then
       did it again, since the count could not advance.
       E$USigP is the documented answer, not an invented one: F$Send lists
       E$IPrcID and E$USigP as its only two possible errors, and "unprocessed
       signal pending" is exactly the situation. Refusing loudly beats
       overwriting somebody's queued signal and reporting success. */
    if (s->cnt>=MAXSIGNALS) {
        debugprintf(dbgProcess,dbgNorm,
                   ("# send signal=%d to pid=%d REFUSED: queue full (%d)\n",
                      signal, spid, s->cnt ));
        return os9error(E_USIGP);
    }

    if (requeue_at_head) {
        for (k=s->cnt; k>0; k--) {
            s->pid   [k]= s->pid   [k-1];
            s->signal[k]= s->signal[k-1];
        }
        s->pid   [0]= spid;
        s->signal[0]= signal;
    }
    else {
        s->pid   [s->cnt]= spid;
        s->signal[s->cnt]= signal;
    }
    
//  debugprintf(dbgSysCall,dbgNorm,("# STACK SIGNAL intUtil=%d spid=%d signal=%d lvl=%d\n", 
//                                     cp->isIntUtil, spid, signal, s->cnt ));
        
    s->cnt++; /* the full case returned above, so this can no longer saturate */
//  upe_printf("# send signal=%d to pid=%d queued (level=%d) %d\n",
//                                         signal,spid, s->cnt, sigp->pwr_brk );
    debugprintf(dbgProcess,dbgNorm,("# send signal=%d to pid=%d queued (level=%d)\n",
                                           signal,spid, s->cnt ));
    /* synchronise it (must be inside the os9exec area) */
  //if (!async_area && sigp->masklevel==0) async_pending= true;
    if (!async_area)                       async_pending= true;
    return 0;
  } /* if */
    
  /* now it's time to clear the flag again */
  if (signal<=Pwr_Signal) sigp->pwr_brk= false; /* switch it off again */
    
  cp->icpt_pid   = spid;    /* keep it save */
  cp->icpt_signal= signal;
  
  /* first, wakeup process, if sleeping */
  if (sigp->state==pSleeping) {
//if (sigp->state==pSleeping &&
//   !sigp->isIntUtil) {
    debugprintf(dbgProcess,dbgNorm,("# send_signal: waking pid=%d from sleep\n",spid));
    set_os9_state( spid, pActive, "send_signal" );
    /* F$Sleep's documented output: remaining ticks if woken prematurely
     * (68k Technical Manual / 6809 SPM). Clamped >=0 since a wake can
     * race the normal-expiry check in do_arbitrate(). */
    sigp->os9regs.d[0]= (sigp->wakeUpTick>GetSystemTick())
                           ? sigp->wakeUpTick-GetSystemTick() : 0;
    sigp->os9regs.sr &= ~CARRY; /* error-free return */
    
    if (!sigp->isIntUtil) {
      sigp->way_to_icpt = true;   /* activate both */
        cp->way_to_icpt = true;
    
      arbitrate= false;           /* don't arbitrate, continue with woken-up process */
    
      if (currentpid==0 && signal==S_Wake) {
      //if (sigp->isIntUtil) { 
      //  cp->way_to_icpt= false; return 0; /* don't switch */
      //} // if
      
        currentpid= spid;
        arbitrate = true;
      } // if
    } // if
  } // if

  if (signal!=S_Wake) {
    /* not wake, additional processing required */
    if (sigp->pd._signal!=0) return os9error(E_USIGP); /* behave like old OS9 <2.2 */
              /* should never be called, because now a signal queue is implemented */

    if (sigp->state==pWaiting) {
      debugprintf(dbgProcess,dbgDetail,("# send_signal: receiving pid=%d was waiting\n",spid));
      retword(sigp->os9regs.d[0])= 0; /* return 0 means that no process has died (pid=0's death can't be awaited) */
      retword(sigp->os9regs.d[1])= 0; /* no signal code !!! returning signal code is wrong (bfo) */
      sigp->os9regs.sr &= ~CARRY;     /* error-free return */
    } /* if */      
                
    arbitrate= false;
    sigp->pd._signal= os9_word(signal);      /* signal currently being processed */

    #if defined NATIVE_SUPPORT || defined PTOC_SUPPORT
      ptocOK= !sigp->isIntUtil || ( sigp->plugElem && 
                                    sigp->plugElem->call_Intercept );
    #else
      ptocOK= !sigp->isIntUtil;
    #endif
      
    /* now, check if target can catch signal */
    if   (sigp->pd._sigvec!=0 && ptocOK) {
          sigp->way_to_icpt= true;    /* activate both */
            cp->way_to_icpt= true;
      
      if (sigp->state==pWaitRead || sigp->state==pWaitWrite) { /* both park in savread */
                          svd= &sigp->savread; 
        sigp->rtevector=  svd->vector;   /* save original info */
        sigp->rtefunc  =  svd->func;
                      v= &svd->r;        /* all original regs */
      }
      else {
        sigp->rtevector=  sigp->vector;  /* save some additional info */
        sigp->rtefunc  =  sigp->func;
                      v= &sigp->os9regs; /* all regs */
      }
            
      memcpy( (void*)&sigp->rteregs, (void*)v, sizeof(regs_type) ); /* save all regs */
            
      /* signal causes wakeup */
      /* activate for all other cases -- but not a request parked as a system
         task (a pipe read or write waiting on the other end): F$RTE has to
         know it was one, to end it with the signal as its error or let it wait
         on. Recorded as pActive, the request was simply dropped: its I$Read
         "returned" whatever the registers held, and the pipe kept counting
         the reader as waiting. */
      /* A write parked on a busy terminal (pWaitWrite) likewise: made pActive,
         its I$Write came back carry clear with the count it had got to. */
      if (sigp->state!=pWaitRead && sigp->state!=pWaitWrite && sigp->state!=pSysTask)
          set_os9_state( spid, pActive, "send_signal" );
      sigp->rtestate= sigp->state;                                  /* save it, active after signal */
      /* and the system task it is parked in: an intercept routine that waits
         on a pipe of its own parks there too, and would leave F$RTE resuming
         (or cutting short) the intercept's request instead of this one */
      sigp->rtesystask     = sigp->systask;
      sigp->rtesystaskdataP= sigp->systaskdataP;
      sigp->rtesystask_offs= sigp->systask_offs;
      sigp->rtepipeDone    = sigp->pipeDone;
      /* ...and, the same way, a parked terminal write's place: an intercept
         routine whose own write parks overwrote it, and the outer write then
         resumed at the handler's count (pre-release review) */
      sigp->rtesaved_cnt   = sigp->saved_cnt;
      sigp->rtesaved_state = sigp->saved_state;
      sigp->rteunitRestLen = sigp->unitRestLen;
      memcpy( sigp->rteunitRest, sigp->unitRest, sizeof(sigp->unitRest) );
      sigp->rteparkedTerm  = sigp->parkedTerm;
      set_os9_state( spid, pActive, "send_signal" );                /* now activate it */
           
      sigp->os9regs.pc  = os9_long((ulong)sigp->pd._sigvec);
      sigp->os9regs.a[6]= sigp->icpta6;
      sigp->os9regs.d[1]= signal;
      sigp->icpt_signal = signal;  

      #if defined NATIVE_SUPPORT || defined PTOC_SUPPORT
        if (sigp->isIntUtil) {      /* prepare execution of intercept routine */
            sigp->way_to_icpt= false; /* don't handle it as intercept */
              cp->way_to_icpt= false;
              
          sv= currentpid;
              currentpid= spid;                       sigp->masklevel= 1;
          debug_return                             ( &sigp->os9regs, spid, true );
          err= sigp->plugElem->call_Intercept( (void*)sigp->os9regs.pc,
                                              signal, sigp->icpta6 );
                                              
          if (sigp->state!=pDead) {
            if (err) {
              sigp->exiterr= err; /* return signal as abort code */
              kill_process( spid );
            } 
            else {             sigp->func= F_RTE;
              debug_comein  ( &sigp->os9regs, spid );
              err= OS9_F_RTE( &sigp->os9regs, spid );
            } // if
          } // if
          
          currentpid= sv;          
          return 0;
        } // if
      #endif
      
    //debugprintf(dbgSysCall,dbgNorm,("# PREP SIGNAL intUtil=%d pid=%d spid=%d signal=%d\n", 
    //                                   cp->isIntUtil, sigp->pd._id, spid, signal ));

      if (currentpid==0) { /* for this special case we have to do it here */
      //if (sigp->isIntUtil) { 
      //  cp->way_to_icpt= false; return 0; /* don't switch */
      //}
        
        currentpid= spid;
        arbitrate = true;
      } // if
            
      debugprintf(dbgProcess,dbgNorm,("# send_signal=%d => continuing in icpt routine of pid=%d\n", 
                                               signal,spid ));
      /* execution will continue in intercept routine */
    }
    else {
    //if (sigp->isIntUtil) { 
    //  printf( "%d abortli\n", currentpid );  /* %bfo% */
    //} // if

      /* abort process */
      debugprintf(dbgProcess,dbgNorm,("# send_signal=%d => pid=%d has no icpt routine, so it will be killed\n",
                                               signal,spid ));
      sigp->exiterr= signal; /* return signal as abort code */
      kill_process( spid );            
      sigp->way_to_icpt= false; /* don't handle it as intercept */
        cp->way_to_icpt= false;
    }
  } // if  

  return 0; /* signal sent successfully */
} /* send_signal */



os9err sig_mask( ushort cpid, int level )
{
    process_typ* cp= &procs[cpid];  /* ptr to procs descriptor */
    int*   plv= &cp->masklevel;
    process_typ*  sigp;
    sig_typ*      s= &sig_queue;
    int    i, j;
    ushort pid, signal;
	    
    switch (level) {
        case  0 :  *plv= 0; break;
        /* "The signal masking level is an eight bit quantity; the system takes
           steps to insure that it does not wrap around in either direction"
           (Microware, OS-9 Intermediate training). Uncapped, 256 increments
           took 256 decrements to undo where OS-9 needs 255. */
        case  1 : if (*plv<255) (*plv)++; break;
        case -1 : (*plv)--; if (*plv<0) *plv= 0; break; // no mask levels below zero
    } /* switch */

  //debugprintf(dbgSysCall,dbgNorm, ("# masklevel=%d cpid=%d\n", cp->masklevel, cpid ));

    for (i=0; i<s->cnt; i++) {    /* remove one element from stack */
        pid   = s->pid   [i]; /* use it as a fifo, save them first */
        signal= s->signal[i];
        
            sigp= &procs[pid];
        if (sigp->masklevel<=0 &&
           (sigp->state!=pWaitRead ||
            sigp->pwr_brk)) {
                        s->cnt--;
            for (j=i; j<s->cnt; j++) { /* remove one element from stack */
                s->pid   [j]= s->pid   [j+1];
                s->signal[j]= s->signal[j+1];
            } /* for */
        
            if (s->cnt<=0) async_pending= false;
            requeue_at_head= true;   /* it was the oldest: if it waits again, it waits first */
            send_signal( pid,signal );
            requeue_at_head= false;
            break;
        } /* if */
    } /* for */

    return 0;
} /* sig_mask */



void kill_processes()
/* kill all running processes and cleanup their resources */
{
    int  k;
    for (k=0; k<MAXPROCESSES; k++) {
        if (procs[k].state!=pUnused) {
            kill_process(k);
        }
    }
} /* kill_processes */



static void wait_for_signal( ushort pid )
/* Let the host deliver whatever it has while <pid> waits.
 *
 * This polled every open ISP socket for incoming data and raised a signal for
 * it (pNask) until 2026-09-20. That stack is retired, and SPF sockets do not
 * want polling: a socket read parks the process and the answer arrives
 * through the path itself. What is left is the host event pump, which is what
 * makes a wait interruptible at all.
 */
{
    (void)pid;
    HandleEvent();
} /* wait_for_signal */



#if defined UNIX && !defined MINGW
  int hostterm_add_wait_fds( fd_set* rfds, int maxfd );   /* hostterm.c */
  int spf_add_wait_fds     ( fd_set* rfds, int maxfd );   /* spfsock.c  */
#endif

/* MINGW defines UNIX and takes the UNIX branch of DoWait(), so this has to
   exist there too -- it is the fd-set helpers above that do not, select()
   being Winsock-only there. Guarding this with them put the call in without
   the function and broke both Windows legs of `make warnings`; the host
   compiler never sees the difference, which is what `make warnings` is for. */
#ifdef UNIX

uint32_t RBF_WaitDeadline( ushort pid ); /* file_rbf.c */

/* How long the idle wait may sleep, in microseconds, before something the
 * emulator promised comes due: the next baud-pacing drain, the next sleeper's
 * wakeUpTick, the next alarm. ULONG_MAX when nothing at all is pending.
 *
 * Ticks are 10ms and GetSystemTick() reads a real clock (gettimeofday, see
 * funcdispatch.c), so a deadline in ticks converts straight to microseconds
 * and does not depend on how often the tick SIGNAL is delivered -- which is
 * what makes it safe for this wait to block the tick while it sleeps. */
static ulong idle_deadline_us( void )
{
    ulong    now  = GetSystemTick();
    ulong    best = baud_next_wake_delay_us();
    uint32_t due;
    ushort   k;

    /* Ticks are 10ms, so a delta beyond IDLE_FAR_TICKS would overflow the
       microsecond product on a 32-bit build -- `ulong` is pointer-width, which
       is 32 bits on the i686 and linux32 legs. Anything that far off is not
       the minimum we are looking for anyway (the caller caps at one tick), so
       it is reported as "far" rather than multiplied. Found auditing the whole
       push for size and endian errors, rdoggett's instruction, 2026-09-21. */
    #define IDLE_FAR_US     1000000UL          /* a second is already "not soon" */
    #define IDLE_FAR_TICKS  (IDLE_FAR_US/10000UL)

    for (k=1; k<MAXPROCESSES; k++) {
        process_typ* cp= &procs[k];
        ulong        us, ticks;
        uint32_t     when;

        if (cp->state==pWaitRead || cp->state==pWaitWrite) {
            when= RBF_WaitDeadline( k );  /* a record wait with an SS_Ticks limit */
            if (when==0)                continue;
        }
        else {
            if (cp->state!=pSleeping)       continue;
            if (cp->wakeUpTick>=MAX_SLEEP)  continue; /* sleeping until signalled */
            /* Zero is not a deadline, it is a field nobody set: the kernel process
               (pid 1) is parked in pSleeping for its whole life to be compliant
               with real OS-9, and never had a wake time. Read as "due at tick 0"
               it makes every idle wait return instantly -- measured, 94% of a core
               where the old fixed nap cost 1.6%. */
            if (cp->wakeUpTick==0)          continue;
            when= cp->wakeUpTick;
        }

        ticks= (when>now) ? (ulong)(when-now) : 0;
        us   = (ticks>=IDLE_FAR_TICKS) ? IDLE_FAR_US : ticks*10000UL;
        if (us<best) best= us;
    } /* for */

    if (A_NextDue( &due )) {
        ulong ticks= (due>now) ? (ulong)(due-now) : 0;
        ulong us   = (ticks>=IDLE_FAR_TICKS) ? IDLE_FAR_US : ticks*10000UL;
        if (us<best) best= us;
    } /* if */

    return best;
} /* idle_deadline_us */
#endif

/* Called wherever host input can end an idle wait: select() on UNIX, the
   console wait on MINGW, and a keystroke queued by the browser page. Guarded
   so that a build with none of those does not carry a function nobody calls,
   a warning there. -fsyntax-only does not show it -- -Wunused-function needs
   a real build -- so `make warnings` building every leg rather than
   syntax-checking them is what caught it once. */
#if defined UNIX
/* Something arrived from the host. Whoever is parked waiting to read or write
   is exactly who it arrived for, so retry them on the NEXT arbitration pass
   instead of making them wait out their rota.

   do_arbitrate retries a pWaitRead/pWaitWrite process only every NewAge (30)
   rounds. That rota was tuned for a loop that spun every millisecond, where it
   meant a 30ms retry; once the idle wait sleeps until something is due, the
   same 30 rounds can be a second or more, and a parked process's latency then
   tracks how long the emulator chose to sleep rather than when its data came.
   Measured: with a 50ms wait the suite's "XOFF halts output but input is still
   taken" fails, the shell needing several retries inside the test's window.
   Being woken by readiness and then still waiting out a rota is the worst of
   both; this is what makes the two agree. */
void retry_parked_now( void )
{
    ushort k;

    for (k=1; k<MAXPROCESSES; k++) {
        process_typ* cp= &procs[k];
        if (cp->state==pWaitRead || cp->state==pWaitWrite) cp->pW_age= 0;
    } /* for */
} /* retry_parked_now */
#endif

void DoWait( void )
{
  ulong ticks= GetSystemTick();

  #ifdef UNIX
    /* How long there is until the emulator owes somebody something. At 1ms --
     * what this was before -- an idle emulator woke a thousand times a second
     * and walked every ttydev slot each time, for 1.6% of a core measured.
     * Waking only when something is actually due, and being woken by the host
     * otherwise, costs 0.3%.
     *
     * The cap is ONE SYSTEM TICK, and the history of that number is the point.
     * It bounds how late anything NOT in the select set below can be noticed.
     *
     * Before retry_parked_now() existed, 50ms broke "XOFF halts output but
     * input is still taken" every single time, and 1, 2, 5 and 10ms passed.
     * That fix removed the deterministic failure, the single test went green,
     * and the cap was raised to 50ms on the strength of it -- which was one
     * run of one test standing in for repetition. At 50ms the full suite then
     * failed about ONE RUN IN THREE; at one tick it is 0 in 10, measured with
     * two suites running concurrently so the contention the failures preferred
     * was present. The pre-change build is 0 in 3.
     *
     * So this is not evidence that 50ms is wrong in principle. It is evidence
     * that at 50ms the emulator sits close enough to a latency cliff for a
     * known-marginal test to fall off it, and that one tick does not. The rota
     * fix keeps its own value either way; what was wrong was the parameter,
     * chosen without repeating the measurement that would have shown it.
     * A canary that fails one run in three looks exactly like a canary that
     * passes, if you only ask it once. */
    #define IDLE_CAP_US  10000UL
    ulong delay_us= idle_deadline_us();
    ulong cap;
    long  delay_ns;
    /* Is anything actually WATCHING host input during the wait? Only the
       waits below do, and only for an interactive stdin: select() on a tty,
       and on Windows a wait on the console handle, which is signalled while
       input is waiting in it. With stdin a pipe -- the test harness, any
       non-interactive driver -- select would return readable at EOF
       forever, and Windows has nothing to wait on. One source of truth for
       both decisions. */
    Boolean watching= false;
    #if defined __EMSCRIPTEN__
      watching= true;   /* the page queues keys while we sleep (see below) */
    #elif defined MINGW
      {
          DWORD mode;
          watching= (GetConsoleMode( hStdin, &mode )!=0);
      }
    #else
      watching= (isatty( STDIN_FILENO )!=0);
    #endif

    /* A longer wait is only allowed where something is watching. Where
       nothing is, the wait is a plain nap that looks at nothing, and how
       often it wakes IS the input latency -- measured as a real failure when
       it was let through: "XOFF halts output but input is still taken"
       stopped seeing its typed command in time. */
    cap= watching ? IDLE_CAP_US : 1000UL;
    if (delay_us>cap) delay_us= cap;
    /* Never shorter than the fixed nap this replaced. Something already due
       makes the deadline zero, and the arbitration loop will service it the
       moment this returns -- but if it ever could not, a zero wait would spin
       the host at full tilt. With the floor, the worst this can do is exactly
       what it did before: wake a thousand times a second. */
    if (delay_us<1000UL)      delay_us= 1000UL;
    delay_ns= (long)delay_us*1000L;

    /* Wait out the idle interval -- but on an interactive terminal, wake the
     * instant a keystroke arrives instead of napping the whole interval and
     * only then discovering the input. Parking in select() on stdin -- the very
     * fd CheckInputBuffers()/HandleEvent() drains (non-blocking, via FIONREAD)
     * -- makes console input event-driven rather than a fixed-rate poll: the
     * "park and be woken by host readiness" the console reader was missing. The
     * select timeout is the SAME interval as the nap it replaces, so F$Sleep,
     * F$Alarm and baud-pacing timing are unchanged; only wake-on-input latency
     * improves, and a spurious wake (EINTR / exceptional fd) just re-polls
     * below. Two deliberate exclusions stay on the plain nap: a pipe (the test
     * harness, any non-interactive driver) sits at EOF, where select() returns
     * readable forever and would spin; and MINGW's select() is Winsock-only
     * (sockets, not fd 0) with its own console plumbing. Dropping the residual
     * 1ms heartbeat entirely -- blocking until the next real deadline when
     * nothing at all is pending -- needs a next-deadline scan across
     * sleepers/alarms/baud and is left as a follow-up (see ROADMAP). */
    Boolean waited= false;
    #if defined __EMSCRIPTEN__
      /* A browser: there is no select() on a keyboard, and blocking would
       * freeze the page. Give the page its event loop for the interval
       * (Asyncify); anything typed meanwhile is queued, and HandleEvent,
       * via CheckInputBuffers below, takes it. */
      emscripten_sleep( (unsigned int)(delay_us/1000UL) );
      waited= true;
    #elif defined MINGW
      /* Windows' select() is for sockets only, but a console handle can be
         waited on directly, so a keystroke ends the wait just as it does on
         a UNIX tty. It used to be a blind 1ms nap, which is also how late a
         key was seen. */
      if (watching) {
          if (WaitForSingleObject( hStdin, (DWORD)(delay_us/1000UL) )==WAIT_OBJECT_0)
              retry_parked_now();
          waited= true;
      }
    #else
      if (watching) {
          fd_set         rfds;
          struct timeval tv;
          sigset_t       tick, before;
          int            maxfd= STDIN_FILENO;
          int            ready;
          FD_ZERO( &rfds );
          FD_SET ( STDIN_FILENO, &rfds );
          /* Every OTHER host source the millisecond poll used to ask about:
           * the bound /tN endpoints and any socket armed with SS_SSig. The
           * ttydev slots are deliberately not here -- their input arrives from
           * another GUEST process writing into a pipe, and a guest that can
           * run means this wait is not running at all. */
          maxfd= hostterm_add_wait_fds( &rfds, maxfd );
          maxfd= spf_add_wait_fds     ( &rfds, maxfd );
          tv.tv_sec =  delay_ns/1000000000L;
          tv.tv_usec= (delay_ns/1000L) % 1000000L;

          /* The system tick is installed with SA_RESTART (os9_tick.c), and
           * IRIX 6.5 restarts an interrupted select() with its WHOLE timeout:
           * the 1ms rounds up to a clock tick, the next SIGALRM always lands
           * first, and the select never returns until a key does. An idle
           * shell then needed ~30 keystrokes before its read was retried
           * (reported from an IRIX build, 2026-09-15; macOS and Linux return
           * EINTR instead, so they never showed it). Hold the tick off for
           * this one short wait: a tick due meanwhile is delivered when it is
           * unblocked, and it is only an arbitration hint. */
          sigemptyset( &tick );
          sigaddset  ( &tick, SIGALRM );
          sigprocmask( SIG_BLOCK, &tick, &before );
          ready= select( maxfd+1, &rfds, NULL,NULL, &tv );
          sigprocmask( SIG_SETMASK, &before, NULL );
          if (ready>0) retry_parked_now();
          waited= true;
      }
    #endif
    if (!waited) {
        struct timespec wait_time;
        wait_time.tv_sec =       0;
        wait_time.tv_nsec= delay_ns;
        nanosleep( &wait_time, NULL );
    }
  //slp_idleticks++;

    /* Mirror the windows32 branch's HandleEvent() call below: without this,
     * a process sleeping forever on a keypress-delivered signal (e.g. tsmon
     * waiting for the first keystroke on /term at boot, wakeUpTick pinned to
     * MAXINT) can never be woken once do_arbitrate finds no runnable process
     * at all -- that idle path only ever reaches DoWait(), never the main
     * loop's periodic CheckInputBuffers() spin-check, so stdin is never
     * polled and the wake signal is never delivered. Confirmed live via
     * lldb: procs[tsmon].state stayed pSleeping with wakeUpTick=2147483647
     * and pd._signal=0 even after keystrokes were sent into the pty. */
    CheckInputBuffers();

    /* Same reasoning, same fix shape, different symptom: a due F$Alarm was
     * never checked here either, so a process asleep specifically to be
     * woken BY its own alarm firing could sleep right through it -- confirmed
     * live, a 1-second alarm did not interrupt a 10-second F$Sleep. See
     * CheckAlarms()'s own comment (alarms.c) for the debug trace that found
     * this idle-wait loop as the actual root cause. */
    CheckAlarms();

  #elif defined windows32
  //ulong ticks= GetSystemTick();
    Sleep( 1 ); // sleep for a short time
    HandleEvent();
  //slp_idleticks+= GetSystemTick()-ticks;
                
  #elif defined MACOS9
    int   sWait;
  //ulong ticks= GetSystemTick();

    #ifndef MPW // is not available there
    //ulong len= 13;
    //char  s[ 14 ];
      sWait= ( geCnt / 20 )+1; if (sWait>10) sWait= 10;
      geCnt= HandleOneEvent( nil,  sWait );
      
    //sprintf( s, "%5d %5d\r\n", geCnt, sWait );
    //syspath_write( currentpid, 1, &len, &s, true );
    #endif
                
  //slp_idleticks+= GetSystemTick()-ticks;
  #else
    #error architecture not supported
  #endif

  baud_drain_due();
  slp_idleticks+= GetSystemTick()-ticks;
} // DoWait



/* arbitrate and prepare next process
 * Note: checks global flag "arbitrate" to see if global "currentpid"
 *       should be changed to next waiting/active process
 */
Boolean pipe_request_reads( ushort pid ); /* pipefiles.c */
Boolean module_readable_by( ushort grp, ushort usr, const mod_exec* m ); /* fcalls.c */
Boolean module_busy_for( ushort pid, ushort mid );                        /* fcalls.c */
Boolean pipe_task_stalled ( ushort pid ); /* pipefiles.c */

/* Whether os9exec should end now: the process it was started with has gone,
   and every process left is only waiting for input -- parked reading a
   socket, a terminal or a pipe, or waiting for a child. None of them can
   move until something arrives from outside, and there is no one left to
   send it. (rdoggett, 2026-09-19: end, and let the connections close, rather
   than keep the emulator open for them.) Whether a daemon kept os9exec open
   used to depend on a scheduling counter: about one run in twelve it did,
   forever. A process that is running, or sleeping, still keeps it open: a
   background job gets to finish, as it always did. */
static Boolean ShutdownDue( void )
{
    int k;

    if (launch_alive) return false;
    for (k=2; k<MAXPROCESSES; k++) {
        process_typ* p= &procs[ k ];
        if (p->state==pUnused || p->state==pDead)                 continue;
        if (p->state==pWaitRead || p->state==pWaiting)             continue;
        if (p->state==pSysTask && pipe_request_reads( (ushort)k )) continue;
        return false;
    }

    /* A parked process with an alarm armed is not stuck: the alarm is how a
       program aborts its own wait (F$Alarm's own example), and it comes from
       inside. Ending before it is due left that process's work undone. */
    { uint32_t due; if (A_NextDue( &due )) return false; }
    return true;
} /* ShutdownDue */

void do_arbitrate( ushort allowedIntUtil )
{
  ushort       cpid       = currentpid;
  ushort       pid, spid;
  process_typ* cp;
  process_typ* sprocess;
  process_typ* cpw;
  ushort       sleepingpid= MAXPROCESSES; /* assume none sleeping */
  ushort       deadpid    = MAXPROCESSES; /* assume none dead */
  Boolean      done       = false;
  Boolean      chkAll     = false;        /* 2nd run when all sleeping */
  Boolean      atLeast1;                  /* at least one process is sleeping */
  Boolean      pDone;
  Boolean      cOK;

  baud_drain_due();

  if (ShutdownDue()) { currentpid= MAXPROCESSES; return; } /* nothing can move: end */

  debugprintf(dbgTaskSwitch,dbgDetail,("# arbitrate: current pid=%d, arbitrate=%d\n",
                                          currentpid, arbitrate));
                                              
  /* first, check if current process is valid at all (if not, force arbitration) */
  if (cpid>=MAXPROCESSES) { 
    arbitrate = true;      /* arbitrate anyway, start at first process */
    currentpid= 0;
    cpid      = 0;
  } // if
    
  cp= &procs[cpid];

  if (!arbitrate) {
    if (cp->state==pSysTask   ||                 /* we need aritration or we'll get stuck in systasks */
        cp->state==pUnused    ||                 /* unused, arbitrating needed */
        cp->state==pWaitRead  ||
        cp->state==pWaitWrite) arbitrate= true; /* give a chance to other processes */
  } // if
    
  /* now arbitrate if needed */
  spid    =  currentpid;
  sprocess= &procs[spid];
  atLeast1= (sprocess->state==pSleeping);
    
  debugprintf(dbgTaskSwitch,dbgDetail,("# arbitrate: after correction: current pid=%d (state=%s), arbitrate=%d\n",
                                          cpid,PStateStr(cp),      arbitrate));
  do {
    debugprintf(dbgTaskSwitch,dbgDeep,("# arbitrate: checking pid=%d (state=%s), arbitrate=%d\n",
                                            spid,PStateStr(sprocess),arbitrate));
    if (arbitrate) {
      /* next process */
      pDone= false;
      do {  spid++;
      //if (spid>=MAXPROCESSES)  spid= 0; /* wrap */
        
        if (spid>=MAXPROCESSES) {
            spid= 2; /* process 0 and 1 do not exist */
          
          // search for any process that could run 
          while (true) {
                sprocess= &procs[spid];   /* do it in correct order */
            if (sprocess->state==pActive ||
               (sprocess->state==pSysTask && !pipe_task_stalled( spid ))) {
              if  (spid<=cpid) pDone= true; // it's immediately ok

              spid= 0;                      // do it later
              break;
            } // if
            
            if   (sprocess->state==pSleeping) {
              if (sprocess->wakeUpTick<=GetSystemTick()) {
              //  sprocess->os9regs.d[0]= 0;
              //  sprocess->os9regs.sr &= ~CARRY; // error-free return
              //set_os9_state( spid, pActive, "do_arbitrate" ); 
                if  (spid<=cpid) pDone= true; // it's immediately ok
                
                spid= 0;                      // do it later
                break;
              } // if
            } // if
            
            spid++;
            if (spid==MAXPROCESSES) { // no running process found => sleep a little bit !
              if (ShutdownDue()) { currentpid= MAXPROCESSES; return; } /* nothing can move: end */
              #ifdef THREAD_SUPPORT
                if (sprocess->isIntUtil && ptocThread) 
                  pthread_mutex_unlock( &sysCallMutex );
              #endif

              DoWait();
           
              #ifdef THREAD_SUPPORT
                if (sprocess->isIntUtil && ptocThread)
                  pthread_mutex_lock( &sysCallMutex );
              #endif
              
              spid= 0; /* process 0 and 1 do not exist */
              break;
            } // if
          } // loop
        } // if spid>=MAXPROCESSES
        
        if (pDone) break;
        
        if (spid==1)             spid++;  /* avoid process 1 */
                sprocess= &procs[spid];   /* do it in correct order */
      } while ((sprocess->state==pUnused ||
                sprocess->state==pDead)  && spid!=cpid); // break loop for sure
          
          sprocess= &procs[spid];   /* do it in correct order */
      if (sprocess->isIntUtil && 
          sprocess->state==pActive && spid==allowedIntUtil) break;
          
      /* --- test if all processes are tested already */
      if (spid==cpid) {
        /* -- no other process found to run */
        if (sprocess->state==pUnused || sprocess->state==pDead) {
          /* The search began at a slot nobody holds -- an orphan's exit leaves
             currentpid 0 -- so arriving back there is not "every process has
             had its turn": the ones waiting for input were never asked. Going
             on (the scan above already waited) keeps them; ending here shut
             os9exec down under a live shell. ShutdownDue decides the end. */
        }
        else if (!chkAll && (spid==allowedIntUtil || !sprocess->isIntUtil)) {
          if (atLeast1) chkAll= true;
          else          done  = true;
        }
        else {
          debugprintf(dbgTaskSwitch,dbgDetail,("# arbitrate: checked all, now pid=%d must be startable\n",spid));
          #ifdef THREAD_SUPPORT
            if (sprocess->isIntUtil && ptocThread) 
              pthread_mutex_unlock( &sysCallMutex );
          #endif

          if  (sprocess->state==pSleeping ||
               sprocess->isIntUtil) {
            DoWait(); 
            
            /*
            #ifdef UNIX
              wait_time.tv_sec =        0;
//            wait_time.tv_nsec= 10000000;
              wait_time.tv_nsec=  1000000;
              nanosleep( &wait_time, NULL );
              slp_idleticks++;
              
            #elif defined macintosh || defined windows32
              ulong ticks   = GetSystemTick();
              HandleEvent();
              slp_idleticks+= GetSystemTick()-ticks;
            #endif
            */
          } 
          else {
            done= true; /* don't stop if sleeping in slow mode */
          }
          
          #ifdef THREAD_SUPPORT
            if (sprocess->isIntUtil && ptocThread)
              pthread_mutex_lock( &sysCallMutex );
          #endif
        } /* if !chkAll */
      }
      else if (sprocess->state==pUnused) continue; /* fast forward */
    } /* if arbitrate */
        
    /* --- test if process can be run */
    if (sprocess->state==pWaiting /* && spid!=allowedIntUtil */) {
      // internal utilities are treated somewhat special
      cOK= true;
      for (pid=0; pid<MAXPROCESSES;  pid++) {
                     cpw=    &procs[ pid ];
        if (os9_word(cpw->pd._pid)==spid &&     
                     cpw->isNative       &&
                     cpw->state!=pDead   &&
                     cpw->state!=pUnused) {
          cOK= false;
          break;
        } // if
      } // for
    
      if (cOK) {
        // --- search if there is a dead child of that process
        for (pid=0; pid<MAXPROCESSES;  pid++) {
                       cpw=    &procs[ pid ];
          if (os9_word(cpw->pd._pid)==spid &&      
                       cpw->state==pDead) {
            deadpid= pid; 
            break;
          } // if
        } // for
        
        if (deadpid<MAXPROCESSES) break; /* yes, there is a dead child, we can unwait */
      } // if
    } /* if */
    
    if (sprocess->isIntUtil && spid!=allowedIntUtil) {
      arbitrate= true; continue;  /* don't break as internal utility */
    } // if
    
    /* --- check if process can be activated */
    if    (sprocess->state==pActive) break;               /* process can run, if active */

    if    (sprocess->state==pSysTask) {  /* process can run, if SysTask and not intUtil */
      if (!sprocess->isIntUtil) break;
      done= false;
    } // if

    if    (sprocess->state==pWaitRead ||
           sprocess->state==pWaitWrite) {        /* only every nth time for this mode */
      uint32_t lockDue= RBF_WaitDeadline( spid ); /* and when its SS_Ticks runs out */
      if  (sprocess->pW_age--<=0 || (lockDue!=0 && lockDue<=GetSystemTick())) {
           sprocess->pW_age= NewAge;  break;
      } // if
      done= false;
    } // if

    if (sprocess->state==pSleeping) {
      atLeast1= true;              /* at least one process is sleeping -> don't break ! */

      /* Slow down also here -- but never past the sleeper's own deadline. The
         rota counts arbitration rounds, and beside a process that computes
         without calls a round is a tick, so every F$Sleep overran by up to 30
         ticks: 0.3 s at 100 Hz, and over a second on a host whose timer runs
         slower (GitHub's macOS runner, about 25 Hz, where CONF68K t115's
         parent slept through its child's whole spin). Twenty 5-tick sleeps
         beside a spinner took 8.1 s; one second is right. */
      if (sprocess->pW_age--<=0 || chkAll ||
          sprocess->wakeUpTick<=GetSystemTick()) {
          sprocess->pW_age= NewAge;
            
        // --------------------------------------------
        // asynchronous signals are allowed here
        async_area= true; 
	    if (async_pending && sprocess->masklevel<=0) {
        //debugprintf(dbgSysCall,dbgNorm,("# SIGNAL HANDLED1 isInt=%d pid=%d sig9=%d d1=%d\n", 
        //            cp->isIntUtil, cpid, procs[ 9 ].icpt_signal, procs[ 9 ].os9regs.d[1] ));
	      sig_mask( spid, 0 ); /* pending signals */
        //debugprintf(dbgSysCall,dbgNorm,("# SIGNAL HANDLED2 isInt=%d pid=%d sig9=%d d1=%d\n", 
        //            cp->isIntUtil, cpid, procs[ 9 ].icpt_signal, procs[ 9 ].os9regs.d[1] ));
		} // if
			
        wait_for_signal( spid );
        
  	    async_area= false; 
        // asynchronous signals are no longer allowed
        // --------------------------------------------
                        
      //if (sprocess->wakeUpTick!=MAX_SLEEP &&
        if (sprocess->wakeUpTick<=GetSystemTick()) {
            sprocess->os9regs.d[0]= 0;      /* no remaining ticks */
            sprocess->os9regs.sr &= ~CARRY; /* error-free return */
          set_os9_state( spid, pActive, "do_arbitrate" ); break;
        } // if

        sleepingpid= spid; /* remember sleeping process */ 
      } /* if slow down */
    } /* if sleeping */
        
    if (sprocess->way_to_icpt) break;
    
//  /* --- b.t.w, remember sleeping processes */        
//  if (cp->state==pSleeping) sleepingpid=currentpid; /* remember sleeping process */   

    /* --- after one round, arbitration starts anyway */
    arbitrate= true; /* now advance anyway */
    if    (done) spid= MAXPROCESSES; /* should have exited via break by now */
  } while(!done);
  
//cp= &procs[ justthis_pid ];
//if (cp->state==pWaitRead) {
//   debugprintf(dbgTaskSwitch,dbgNorm,("# arbitrate spid=%d\n", spid ));
//} // if
  
//if (done && sprocess->isIntUtil)
//  printf( "%d ALLARM !!\n", currentpid ); /* %bfo% */
    
  /* assign */
  currentpid= spid;
//if (procs[currentpid].isIntUtil)
//  printf( "%d NOCHN ALLARM !!\n", currentpid ); /* %bfo% */
    
    
  /* now, if currentpid<MAXPROCESSES, there is a process to run or unwait-and-run
   * otherwise, only sleeping processes (if sleepingpid<MAXPROCESSES)
   * or no processes at all are left. 
   */
  if (currentpid<MAXPROCESSES) {
        cp= &procs[currentpid];
    if (cp->state==pWaiting && deadpid<MAXPROCESSES) {
      /* -- we need to terminate waiting first */
      retword(cp->os9regs.d[0])=        deadpid;          /* ID of dead child */
      retword(cp->os9regs.d[1])= procs[ deadpid ].exiterr; /* exit error of child */
      AssignNewChild      ( currentpid, deadpid );

      debugprintf(dbgTaskSwitch,dbgNorm,("# arbitrate: waiting pid=%d gets active because child pid=%d is dead (exiterr=%d)\n",currentpid,deadpid,procs[deadpid].exiterr));
      set_os9_state( deadpid,    pUnused, "do_arbitrate" );
      set_os9_state( currentpid, pActive, "do_arbitrate" ); /* process is now active */
      cp->os9regs.sr &= ~CARRY; /* error-free return */
    } // if           
  }
  else currentpid= sleepingpid; /* is there a sleeping process ? */
} /* do_arbitrate */



static os9err os9exec_compatible( mod_exec* mod )
/* this is the list of modules, which can't run under OS9exec */
{
    char* p;
    short ed;

    p = Mod_Name( mod );
    ed= os9_word( mod->_mh._medit );
    
    /* deldir: pRdelete zeros the parent slot and frees target sectors; no parent-shrink attempted */
    if (ustrcmp(p,"sysdbg" )==0 && ed<=100) return E_BADREV; /* crashes right at the beginning */
    if (ustrcmp(p,"mnt"    )==0 && ed<=100) return E_BADREV; /* no "/mt" device available */

    if (ustrcmp(p,"list"   )==0 && ed== 16) return E_BADREV; /* V2.4 version is bugy */
    if (ustrcmp(p,"cmp"    )==0 && ed== 23) return E_BADREV; /* V3.0 version is bugy */
    
    return 0;
} /* os9exec_compatible */



void stop_os9exec(void)
/* print a messsage and stop the OS9 emulator */
{
	/* write message to main system path and stop emulator directly */
	quitFlag= true;
    usrpath_printf( 0,MAXUSRPATHS, "\n" );
    usrpath_printf( 0,MAXUSRPATHS, "# OS9 emulation ends here.\n" ); 
    fflush(stdout);

    exit(0); /* never come back */
} /* stop-os9exec */



void lw_pid( ttydev_typ* mco )
/* set the last written pid */
{
    if   (mco->spP->lastwritten_pid==gLastwritten_pid ) return;
    procs[mco->spP->lastwritten_pid].last_mco= NULL;   /* switch off old one */
          mco->spP->lastwritten_pid= gLastwritten_pid; /* assign for later use */
    procs[mco->spP->lastwritten_pid].last_mco= mco;    /* activate new */
} /* lw_pid */



os9err setprior(ushort pid, ushort newprior)
/* set priority (and update IRQblock flag */
{
    process_typ* cp= &procs[pid]; /* ptr to procs descriptor */

    if  (pid>=MAXPROCESSES)  return os9error(E_IPRCID);
    if  (cp->state==pUnused) return os9error(E_IPRCID);
         cp->pd._prior= os9_word(newprior);
        
    if (newprior<IRQBLOCKPRIOR)
         cp->os9regs.flags |=   FLAGS_IRQ;  /* allow IRQs (and VBL) during OS9 code execution */
    else cp->os9regs.flags &= (~FLAGS_IRQ); /* disallow any IRQs during OS9 code execution */
    
    return 0;
} /* setprior */



/* prepare OS9 program module for fork
 * Note: procs[pid] must be already prepared (by new_process())
 */
os9err prepFork( ushort newpid,   char*  mpath,    ushort mid,
                 byte*  paramptr, uint32_t paramsiz, uint32_t memplus,
                 ushort numpaths, ushort grp, ushort usr, ushort prior )
{
    byte         *mp,*p,*p2;
    uint32_t     memsiz, cnt;
    ushort       err, mty;
    mod_exec*    theModule;
    process_typ* cp= &procs[newpid];
    regs_type*   rp= &cp->os9regs;

    #ifdef INT_CMD
      ushort       svid;
      process_typ* pp= &procs[os9_word(cp->pd._pid)];
      Boolean      asThread;
      void*        modBase;
      ushort       argc;
      char**       arguments;
    #endif

    /* save main module ID */
    cp->mid=  mid;
    setprior( newpid,prior );

    /* inherit group and user */
    cp->pd._group= os9_word( grp );
    cp->pd._user = os9_word( usr );

    cp->isIntUtil= false; /* no internal command by default */

    /* Check for internal commands BEFORE touching the module or allocating 68k memory.
       Internal commands run as C functions and need neither prepData nor 68k register setup.
       This also handles commands with no OS-9 binary (ihelp, icmds, iprocs, ...) that
       were previously unreachable because link_load failed before prepFork was ever called. */
    #ifdef INT_CMD
        /* A genuine resident module must win over a same-named internal command
         * (same guard as OS9_F_Link/link_module, 881f05f): without it, mid!=0
         * here (link_load already resolved a real module -- e.g. a packed
         * BASIC09 procedure named "move") is still overridden by the internal
         * command, so F$Fork/F$Chain never launch the real module at all. */
          cp->isIntUtil= isintcommand( mpath, &cp->isNative, &modBase )>=0
                         && find_mod_id( mpath )>=MAXMODULES;
      if (cp->isIntUtil) {
        set_os9_state( newpid, pActive, "prepFork" );
        if (!cp->isNative) cp->mid= 0; // assign "OS9exec" module, for non-PtoC modules

        /* prepare args */
        prepArgs( (char*)paramptr, &argc,&arguments );
        arguments[0]= (char*)mpath;  /* set module name */

        if (pp->pd._cid!=0 &&
            pp->pd._cid!=os9_word(newpid)) /* already children available */
            cp->pd._sid= pp->pd._cid;  /* take child as sibling */

        svid= currentpid;
        pp->pd._cid= os9_word(newpid); /* this is the child */
        currentpid =          newpid;  /* use the correct identification */

        /* execute command */
        err= callcommand( mpath,newpid,svid, argc,arguments, &asThread );
        release_mem                             ( arguments );

        if (asThread)
          currentpid= svid; // don't change current pid for threads
        else {
          /* simulate successful F$Exit of internal command */
          cp->exiterr= err;
          kill_process( newpid );
          err= 0;
        } // if

        return err; /* internal-tool "fork" return value */
      } /* if isintcommand */
    #endif

    /* mid==0 here means link_load failed and isintcommand didn't catch it either.
       os9modules[0] is the OS9exec identification module (a built-in C struct outside
       the arena); using it would produce a garbage PC.  Return E_MNF cleanly. */
    if (mid==0) return os9error(E_MNF);

    /* get pointer to main module — must be valid for real OS-9 binaries */
    theModule= os9mod( mid );
    if (theModule==NULL) return os9error(E_MNF);

    /* Guard: built-in modules live outside the 68k arena; TO68K of their pointer
       is garbage.  This shouldn't normally be reached, but catch it defensively. */
    if (os9modules[mid].isBuiltIn) return os9error(E_MNF);

    /* "To be loaded, the module must be program object code" (F$Fork, v2.4
       Technical Reference Manual, page 1-30), else E$NEMod. Checked before the
       data area is built, which reads a program's initialised-data table: a
       packed BASIC09 module ($0202, Subroutine, I-code) has none, and failed there as a
       "bad module ID" instead; a Program in any language but object code would
       have been run as 68000 code, since only the type was checked, further
       down. Handing I-code to RunB is the
       shell's job, not the kernel's (Using Professional OS-9, shell). */
    { ushort tylan= os9_word( theModule->_mh._mtylan );
      if ((tylan>>BpB)!=MT_PROGRAM || (tylan & 0xFF)!=ML_OBJECT) {
          /* and forget it: F$Chain's failure path kills this process, and
             kill_process unlinks cp->mid -- a second unlink for the one
             link F$Chain made, so a module still in use could be freed */
          unlink_module( mid ); cp->mid= MAXMODULES; return os9error(E_NEMOD);
      }
    }

    /* -- prepare data area */
    debugprintf(dbgProcess,dbgDetail,("# prepFork: extra memory=%u (= paramsiz:%u + memplus:%u)\n",
                                    memplus+paramsiz, paramsiz,memplus));
    err= prepData( newpid,theModule,memplus+paramsiz, &memsiz, &mp );
    if (err) { unlink_module( mid ); cp->mid= MAXMODULES; return err; } /* no room for data; as below */

    /* -- copy parameter area */
    p= paramptr;        p2= mp+memsiz-paramsiz;
    cp->my_args= TO68K(p2); /* parameter area start (68k offset) */

    regcheck( newpid,"Param writing start",TO68K(p2),            RCHK_MEM );
    regcheck( newpid,"Param writing end",  TO68K(p2)+paramsiz-1, RCHK_MEM );
    /* paramptr is FROM68K(a1) from F$Fork/F$Chain; a1=0 with paramsiz>0 is a bad
       call that would deref NULL here.  Skip the copy rather than crash the host
       (a1=0 with paramsiz=0 -- a fork with no parameters -- is legitimate and
       already copies nothing). */
    if (paramptr!=NULL) for (cnt=0; cnt<paramsiz; cnt++) *(p2++)= *(p++);

    /* check if module is executeable and */
    /* check if this module is not in the "black list" of OS9exec */
        mty= os9_word( theModule->_mh._mtylan )>>BpB;
    if      (mty!=MT_PROGRAM)                            err= E_NEMOD;
    else if (!module_readable_by( grp,usr, theModule ))  err= E_PERMIT; /* F$Fork links it: fcalls.c */
    else if (module_busy_for( newpid, mid ))             err= E_MODBSY; /* the same link's other rule */
    else                                                 err= os9exec_compatible( theModule ); 

    if (err) { unlink_module( mid ); cp->mid= MAXMODULES; return err; } /* as above */

    debugprintf(dbgProcess,dbgNorm,("# prepFork: Module mid=%d, address=%p\n",mid,(void*)theModule));

    /* -- prepare registers */
    rp->sr=0; /* everything cleared, USER state */
    rp->pc  = TO68K(theModule)+os9_long(theModule->_mexec); /* entry point */
    rp->a[3]= TO68K(theModule); /* primary module pointer */
    rp->d[0]= newpid;    /* assign process ID */
    rp->d[1]= os9_word(cp->pd._group)<<(2*BpB)|
              os9_word(cp->pd._user ); /* inherited group/user */
    rp->d[2]= prior;     /* priority */
    rp->d[3]= numpaths;  /* number of paths inherited */
   
    cp->memstart= TO68K(mp);        /* save static storage start address (68k offset) */
    rp->a[6]    = TO68K(mp)+0x8000; /* biased A6 */
    rp->membase =        mp;        /* unbiased static storage pointer (host ptr) */

    /* set up parameter area regs */
    rp->a[5]= cp->my_args;
    rp->a[7]= rp->a[5];                        /* top of stack */
    rp->a[1]= cp->memtop= TO68K(mp+memsiz);  /* memory end (68k offset) */
    rp->d[5]= paramsiz; /* parameter size */
    rp->d[6]= memsiz; /* total initial memory allocation */

    /* prepare sigdat content */
    SET_OS9L(cp->sigdat, 0x12, cp->memtop - paramsiz);

    /* check for stdout filter and init */
    cp->stdoutfilter= initfilterfunc( Mod_Name( theModule ), (char*)paramptr, (void**)&cp->filtermem);
    debugprintf(dbgProcess,dbgNorm,("# prepFork: stdoutfilter=%p, filtermem=%p\n",(void*)(uintptr_t)cp->stdoutfilter,(void*)cp->filtermem));
    debugprintf(dbgProcess,dbgNorm,("# prepFork: Everything's ready for launch!\n"));

    return 0; /* ok */
} /* prepFork */


/* eof */

