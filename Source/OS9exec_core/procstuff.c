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

/* process routines */
/* ================ */

int is_super(ushort pid)
/* Returns true if process belongs to super user */
{
   process_typ*   cp = &procs[pid];
   int            reply = 0;

   if (cp->pd._user == 0 && cp->pd._group == 0)
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
                default        : sta='?'; break;
            } // switch
            if (cp->isIntUtil)   sta='I';

            if (k==currentpid) mName= "iprocs";
            else {             mName= "<none>";
                if (mod!=NULL) mName= Mod_Name( mod );
            }

            sprintf( idstr,"%c%d", k==currentpid ? '*' : ' ', k );
            if (cp->mid==MAXMODULES) strcpy( mIDs,"-" );
            else                    sprintf( mIDs, "%d", cp->mid );
            
            upo_printf("%3s %c %3d %3d %3d %3s $%08lX %5d $%08lX $%08lX %-12s %s\n",
                        idstr,
                        sta,
               os9_word(cp->pd._pid),
               os9_word(cp->pd._sid),
               os9_word(cp->pd._cid),
                        mIDs,
                        (ulong) mod,
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
        pd->_sigdat= os9_long( TO68K(&procs[k].sigdat) );

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
	ulong        timbeg, datbeg;
    
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
            cp->os9regs.isp= (ulong) &trapframebuf[TRAPFRAMEBUFLEN];
            #endif
            
            /* there was no last systemcall */
            cp->func       = STARTCALL;
            cp->lastsyscall= STARTCALL;
            
            /* reset statistics */
            cp->pd._uticks= 0;
            cp->fticks    = 0;
            cp->iticks    = 0;
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
                strncpy( cp->d.path, pap->d.path, OS9PATHLEN );
            
                #ifdef macintosh
                  cp->d.volID= pap->d.volID; 
                  cp->d.dirID= pap->d.dirID;
                #endif
            
                cp->x.type = pap->x.type;
                cp->x.dev  = pap->x.dev;
                cp->x.lsn  = pap->x.lsn;
                strncpy( cp->x.path, pap->x.path, OS9PATHLEN );
                fprintf(stderr,"# new_process: pid=%d inherits x.dev=%d x.lsn=%lu x.path='%s' from pap=%d\n",
                        npid, cp->x.dev, (unsigned long)cp->x.lsn, cp->x.path, (int)(pap-procs));
            
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
    cp->masklevel= 0;
    
    if (cp->state==pUnused) return os9error(E_IPRCID); /* process does not exist */
    debugprintf(dbgProcess,dbgNorm,("# kill_process: killing pid=%d, parentid=%d, exiterr=%d\n",
                                       pid, os9_word(cp->pd._pid),cp->exiterr));

    /* remove some more resources */
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
    if (cp->state==pDead) return os9error(E_IPRCID); /* avoid killing again, because double close is not good */
    debugprintf(dbgProcess,dbgNorm,("# kill_process: set to unused\n" ));

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


/* send a signal to a process */
os9err send_signal( ushort spid, ushort signal )
{
  #if defined NATIVE_SUPPORT || defined PTOC_SUPPORT
    os9err err= 0;
    short  sv;
  #endif
	
  process_typ* cp  = &procs[currentpid]; /* ptr to my procs dsc */
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

  if (signal==S_Wake && spid==currentpid) return 0; /* ignore this */
   
//if (sigp->isIntUtil) return 0; /* ignore this as well for the moment */
    
  /* signal will be delayed, if receiving process in pWaitRead mode */
  wRead= ( sigp->state==pWaitRead && !sigp->pwr_brk );
    
  /* some of the signals must be stored in a queue */
  if  (cp->way_to_icpt || sigp->masklevel>0 || wRead || !async_area) {
    if (signal==S_Wake && sigp->masklevel>0) return 0; /* needn't to be stored in this case */
        
    if (wRead && signal<=Pwr_Signal) sigp->pwr_brk= true; /* special break condition */
        
    s= &sig_queue; /* store it, but don't do anything */
    s->pid   [s->cnt]= spid;
    s->signal[s->cnt]= signal;
    
//  debugprintf(dbgSysCall,dbgNorm,("# STACK SIGNAL intUtil=%d spid=%d signal=%d lvl=%d\n", 
//                                     cp->isIntUtil, spid, signal, s->cnt ));
        
    if (s->cnt<MAXSIGNALS) s->cnt++;
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
    sigp->os9regs.d[0]= 0;      /* %%% return # of remaining ticks! */
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
      
      if (sigp->state==pWaitRead) {
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
      /* activate for all other cases */
      if (sigp->state!=pWaitRead) set_os9_state( spid, pActive, "send_signal" );
      sigp->rtestate= sigp->state;                                  /* save it, active after signal */
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
        case  1 : (*plv)++; break;
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
            send_signal( pid,signal );
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
/* this routine looks for incoming data from any of the connected */
/* network sockets */
{
    #ifdef NET_SUPPORT
    os9err       sig= 0;
    #endif
    syspath_typ* spP;
    int          k, sp;
    int          sv= currentpid;
    process_typ* cp= &procs[pid]; /* ptr to procs descriptor */
    
    HandleEvent();
    
    currentpid= 0;/* switch off */
    for (k=0; k<MAXUSRPATHS; k++) {
            sp= cp->usrpaths[k];
        if (sp>0 && sp<MAXSYSPATHS) {
                spP= get_syspath( pid,sp );
            if (spP!=NULL && 
                spP->type==fNET) {
                #ifdef NET_SUPPORT
                      sig= pNask( pid, spP );
                  if (sig) send_signal( pid,sig );
                #endif
            }
        } /* if */
    }
    currentpid= sv;
} /* wait_for_signal */


void DoWait( void )
{
  ulong ticks= GetSystemTick();
  
  #ifdef UNIX
    struct timespec wait_time;
    
    wait_time.tv_sec =       0;
    wait_time.tv_nsec= 1000000; // = 1 millisecond 
    nanosleep( &wait_time, NULL );
  //slp_idleticks++;
                
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
  
  slp_idleticks+= GetSystemTick()-ticks;
} // DoWait



/* arbitrate and prepare next process
 * Note: checks global flag "arbitrate" to see if global "currentpid"
 *       should be changed to next waiting/active process
 */
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
    if (cp->state==pSysTask ||                 /* we need aritration or we'll get stuck in systasks */
        cp->state==pUnused  ||                 /* unused, arbitrating needed */
        cp->state==pWaitRead) arbitrate= true; /* give a chance to other processes */
  } // if
    
  /* now arbitrate if needed */
  spid    =  currentpid;
  sprocess= &procs[spid];
  atLeast1= (sprocess->state==pSleeping);
    
  debugprintf(dbgTaskSwitch,dbgDetail,("# arbitrate: after correction: current pid=%d (state=%d), arbitrate=%d\n",
                                          cpid,PStateStr(cp),      arbitrate));
  do {
    debugprintf(dbgTaskSwitch,dbgDeep,("# arbitrate: checking pid=%d (state=%d), arbitrate=%d\n",
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
                sprocess->state==pSysTask) {
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
        if (!chkAll && (spid==allowedIntUtil || !sprocess->isIntUtil)) {
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

    if    (sprocess->state==pWaitRead) {           /* only every nth time for this mode */
      if  (sprocess->pW_age--<=0) { 
           sprocess->pW_age= NewAge;  break;
      } // if
      done= false;
    } // if

    if (sprocess->state==pSleeping) {
      atLeast1= true;              /* at least one process is sleeping -> don't break ! */

      if (sprocess->pW_age--<=0 || chkAll) {                     /* slow down also here */
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
    
    /* -- get pointer to main module */
    theModule= os9mod( mid );

    /* -- prepare data area, for internal commands as well */
    debugprintf(dbgProcess,dbgDetail,("# prepFork: extra memory=%ld (= paramsiz:%ld + memplus:%ld)\n",
                                    memplus+paramsiz, paramsiz,memplus));
    err= prepData( newpid,theModule,memplus+paramsiz, &memsiz, &mp ); if (err) return err; /* no room for data */

    /* -- copy parameter area, for internal commands as well */
    p= paramptr;        p2= mp+memsiz-paramsiz;
    cp->my_args= TO68K(p2); /* parameter area start (68k offset) */

    regcheck( newpid,"Param writing start",TO68K(p2),            RCHK_MEM );
    regcheck( newpid,"Param writing end",  TO68K(p2)+paramsiz-1, RCHK_MEM );
    for (cnt=0; cnt<paramsiz; cnt++) *(p2++)= *(p++);
    
    cp->isIntUtil= false; /* no internal command by default */
    
    #ifdef INT_CMD
          cp->isIntUtil= isintcommand( mpath, &cp->isNative, &modBase )>=0;
      if (cp->isIntUtil) {
        set_os9_state( newpid, pActive, "prepFork" );
        if (!cp->isNative) cp->mid= 0; // assign "OS9exec" module, for non-PtoC modules

        /* prepare args */
        prepArgs( (char*)paramptr, &argc,&arguments );
        arguments[0]= (char*)mpath;  /* set module name */
        
      //if  (!cp->isNative) {            
        if (pp->pd._cid!=0 &&
            pp->pd._cid!=newpid)       /* already children available */
            cp->pd._sid= pp->pd._cid;  /* take child as sibling */
      //} // if
        
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
          
        //pp->pd._cid= cp->pd._sid; /* restore former child id */
        //cp->exiterr= 0;
          cp->exiterr= err;
          kill_process( newpid );
          err= 0;
        } // if
         
        return err; /* internal-tool "fork" return value */
      } /* if isintcommand */
    #endif

    /* check if module is executeable and */
    /* check if this module is not in the "black list" of OS9exec */
        mty= os9_word( theModule->_mh._mtylan )>>BpB;
    if (mty!=MT_PROGRAM) err= E_NEMOD;
    else                 err= os9exec_compatible( theModule ); 

    if (err) { unlink_module( mid ); return err; }

    debugprintf(dbgProcess,dbgNorm,("# prepFork: Module mid=%d, address=$%08lX\n",mid,(ulong) theModule));

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
    debugprintf(dbgProcess,dbgNorm,("# prepFork: stdoutfilter=$%lX, filtermem=$%lX\n",cp->stdoutfilter,cp->filtermem));
    debugprintf(dbgProcess,dbgNorm,("# prepFork: Everything's ready for launch!\n"));

    return 0; /* ok */
} /* prepFork */


/* eof */

