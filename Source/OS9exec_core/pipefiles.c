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
 *    Revision 1.17  2006/10/01 15:23:36  bfo
 *    Debug output for signal sending when pipe full
 *
 *    Revision 1.16  2006/09/08 21:57:23  bfo
 *    signal sending will be cleared only w/o error
 *
 *    Revision 1.15  2006/06/17 14:22:13  bfo
 *    Debugging for default sizes
 *
 *    Revision 1.14  2006/06/11 22:09:49  bfo
 *    set_os9_state with 3rd param <callingProc>
 *    DEFAULTPTYSZ used for pty/tty system
 *    scroll bug fixed with <syW> checking
 *
 *    Revision 1.13  2006/06/10 10:23:26  bfo
 *    Some isIntUtil debugging made invisible
 *
 *    Revision 1.12  2006/06/01 18:05:57  bfo
 *    differences in signedness (for gcc4.0) corrected
 *
 *    Revision 1.11  2006/05/16 13:15:12  bfo
 *    Named pipes are supported now (including "dir -e /pipe)
 *
 *    Revision 1.10  2006/02/19 16:34:38  bfo
 *    use <isIntUtil>
 *
 *    Revision 1.9  2004/11/27 12:13:57  bfo
 *    _XXX_ introduced
 *
 *    Revision 1.8  2004/11/20 17:52:29  bfo
 *    ReadLn modified
 *
 *    Revision 1.7  2004/11/20 11:44:08  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.6  2004/10/22 22:51:12  bfo
 *    Most of the "pragma unused" eliminated
 *
 *    Revision 1.5  2002/10/27 23:46:47  bfo
 *    Release of pipes implemented more consistently
 *
 *    Revision 1.4  2002/08/09 22:39:21  bfo
 *    New procedure set_os9_state introduced and adapted everywhere
 *
 *
 */


#include "os9exec_incl.h"

/* Interprocess Pipes */
/* ================== */


/* --- local procedure definitions for object definition ------------------- */
/* --- pipes */
void   init_Pipe( fmgr_typ* f );
os9err pPopen   ( ushort pid, syspath_typ*, ushort *modeP,  char* pathname );
os9err pPclose  ( ushort pid, syspath_typ* );
os9err pPreadln ( ushort pid, syspath_typ*, uint32_t *n,      char* buffer   );
os9err pPread   ( ushort pid, syspath_typ*, uint32_t *n,      char* buffer   );
os9err pPwriteln( ushort pid, syspath_typ*, uint32_t *n,      char* buffer   );
os9err pPwrite  ( ushort pid, syspath_typ*, uint32_t *n,      char* buffer   );
os9err pPdelete ( ushort pid, syspath_typ*, ushort  *modeP,   char* pathname );

os9err pPsize   ( ushort pid, syspath_typ*, uint32_t *sizeP );
os9err pPopt    ( ushort pid, syspath_typ*,                    byte* buffer   );
os9err pPeof    ( ushort pid, syspath_typ* );
os9err pPready  ( ushort pid, syspath_typ*, uint32_t *n     );
os9err pPFDInf  ( ushort pid, syspath_typ*, uint32_t *maxbytP,
                                            uint32_t *fdinf,   byte* buffer   );
os9err pPsetsz  ( ushort pid, syspath_typ*, uint32_t *sizeP );


/* --- ptys */
void   init_PTY  ( fmgr_typ* f );
os9err pKopen    ( ushort pid, syspath_typ*, ushort *modeP, char* pathname );
os9err pKclose   ( ushort pid, syspath_typ* );
os9err pKread    ( ushort pid, syspath_typ*, uint32_t *n,     char* buffer   );
os9err pKreadln  ( ushort pid, syspath_typ*, uint32_t *n,     char* buffer   );
os9err pKwrite   ( ushort pid, syspath_typ*, uint32_t *n,     char* buffer   );
os9err pKwriteln ( ushort pid, syspath_typ*, uint32_t *n,     char* buffer   );

os9err pKopt     ( ushort pid, syspath_typ*,                  byte* buffer   );
os9err pKpos     ( ushort pid, syspath_typ*, uint32_t *posP );
os9err pKready   ( ushort pid, syspath_typ*, uint32_t *n    );
os9err pKlock    ( ushort pid, syspath_typ*, uint32_t *d0,    uint32_t *d1   );
/* ------------------------------------------------------------------------- */

void init_Pipe( fmgr_typ* f )
/* install all procedures of the pipe manager */
{
    gs_typ* gs= &f->gs;
    ss_typ* ss= &f->ss;

    /* main procedures */
    f->open      = (pathopfunc_typ)pPopen;
    f->close     = (pathopfunc_typ)pPclose;
    f->read      = (pathopfunc_typ)pPread;
    f->readln    = (pathopfunc_typ)pPreadln;
    f->write     = (pathopfunc_typ)pPwrite;
    f->writeln   = (pathopfunc_typ)pPwriteln;
    f->seek      = (pathopfunc_typ)pNop;      /* ignored */
    f->del       = (pathopfunc_typ)pPdelete;
    
    /* getstat */
    gs->_SS_Size = (pathopfunc_typ)pPsize;
    gs->_SS_Opt  = (pathopfunc_typ)pPopt;
    gs->_SS_DevNm= (pathopfunc_typ)pSCFnam;
    gs->_SS_Pos  = (pathopfunc_typ)pUnimpOk;      /* ??? */
    gs->_SS_EOF  = (pathopfunc_typ)pPeof;
    gs->_SS_Ready= (pathopfunc_typ)pPready;
    gs->_SS_FDInf= (pathopfunc_typ)pPFDInf;

    /* setstat */
    ss->_SS_Size = (pathopfunc_typ)pPsetsz;
    ss->_SS_Opt  = (pathopfunc_typ)pNop;      /* ignored */
    ss->_SS_Attr = (pathopfunc_typ)pNop;      /* ignored */
} /* init_Pipe */


void init_PTY( fmgr_typ* f )
/* install all procedures of the pseudo tty (packet manager) */
{
    gs_typ* gs= &f->gs;
    ss_typ* ss= &f->ss;
    
    /* main procedures */
    f->open      = (pathopfunc_typ)pKopen;
    f->close     = (pathopfunc_typ)pKclose;
    f->read      = (pathopfunc_typ)pKread;
    f->readln    = (pathopfunc_typ)pKreadln;
    f->write     = (pathopfunc_typ)pKwrite;
    f->writeln   = (pathopfunc_typ)pKwriteln;
    f->seek      = (pathopfunc_typ)pBadMode; /* not allowed */
    
    /* getstat */
    gs->_SS_Size = (pathopfunc_typ)pUnimp;      /* not used */
    gs->_SS_Opt  = (pathopfunc_typ)pKopt;
    gs->_SS_DevNm= (pathopfunc_typ)pSCFnam;
    gs->_SS_Pos  = (pathopfunc_typ)pKpos;
    gs->_SS_EOF  = (pathopfunc_typ)pNop;         /* ignored */
    gs->_SS_Ready= (pathopfunc_typ)pKready;
    gs->_SS_Undef= (pathopfunc_typ)pUnimp;      /* not used */

    /* setstat */
    ss->_SS_Size = (pathopfunc_typ)pNop;         /* ignored */
    ss->_SS_Opt  = (pathopfunc_typ)pNop;         /* ignored */
    ss->_SS_Attr = (pathopfunc_typ)pNop;         /* ignored */
    ss->_SS_Lock = (pathopfunc_typ)pKlock;
    ss->_SS_Undef= (pathopfunc_typ)pNop;         /* ignored */
} /* init_PTY */

/* -------------------------------------------------------- */




os9err getPipe( _pid_, syspath_typ* spP, uint32_t buffsize )
{
    pipechan_typ* p  = get_mem( sizeof(pipechan_typ) );
    byte*         buf= get_mem( buffsize             );
    if (buf==NULL) return os9error(E_NORAM);

    spP->u.pipe.pchP= p; /* assign pipe structure */
//  printf( "GET     PIPE     %08X %08X %d\n", p,spP->u.pipe.i_svd_pchP, buffsize );

    p->size      = buffsize; /* assign it */
    p->buf       = buf;
    p->prp       = buf;      /* nothing to read */
    p->pwp       = buf;      /* nothing written yet */
    p->bread     = 0;        /* defined to 0 at the beginning */
    p->bwritten  = 0;        /*    "     " "  "  "       "    */
    p->consumers = 0;        /* no waiting consumer yet */
    p->sp_lock   = 0;        /* to be used in pipe mode, not tty/pty */
//  p->sp_win    = 0;        /* no windows info at the beginning */
    p->do_lf     = false;    /* auto linefeed support */
    p->broken    = false;    /* not yet broken */
    p->pipeDirCnt= 0;        /* pipe dir count */
    
    debugprintf( dbgFiles,dbgDetail,("# getPipe: (name='%s') created with buffer[%u] @ $%lX\n",
                 spP->name, buffsize, (ulong)buf));
    return 0;
} /* getPipe */


os9err releasePipe( ushort pid, syspath_typ* spP )
{
    pipechan_typ* p= spP->u.pipe.pchP;
    pipechan_typ* pk;
    syspath_typ*  spK;
    ushort        k;

    for (k=0; k<MAXSYSPATHS; k++) {
            spK= &syspaths[k];
        if (spK!=spP &&                 /* not the own system path */
            spK->type!=fNone &&                      /* but in use */
            spK->u.pipe.pchP==p ) {   /* check/adapt backward lock */

                        spK= get_syspathd( pid, p->sp_lock );
            if         (spK!=NULL) {
                    pk= spK->u.pipe.pchP;
                if (pk->sp_lock==spP->nr) pk->sp_lock= k;   
                return 0;          /* still in use by another path */
            }
        } /* if */
    } /* for */

//  printf( "RELEASE PIPE OK  %08X %08X\n", p,spP->u.pipe.i_svd_pchP );
        spK= get_syspathd( pid, p->sp_lock );
    if (spK!=NULL && 
        spP->u.pipe.i_svd_pchP==NULL) { /* don't break if temporary pipe */
        spK->u.pipe.pchP->sp_lock= p->sp_lock;
        spK->u.pipe.pchP->broken = true;
    //  printf( "SEND %d %d\n", spK->signal_pid, spK->signal_to_send );

        if                               (spK->signal_to_send!=0) {
            send_signal( spK->signal_pid, spK->signal_to_send );
                                          spK->signal_to_send= 0;
        }
    }

    release_mem( p->buf );
    debugprintf( dbgFiles,dbgDetail,("# releasePipe: (name='%s') @ $%lX\n",
                 spP->name, p->buf ));

    release_mem( p );
    spP->u.pipe.pchP= NULL; /* and make it invisible */
    return 0;
} /* releasePipe */



static void releasePipe_svd( ushort pid, syspath_typ* spP, Boolean forced )
{
    pipe_typ*     pp= &spP->u.pipe;
    pipechan_typ* p =  pp->pchP;
//  printf( "RELEASE PIPE_SVD %08X %08X %d %d\n", p,spP->u.pipe.i_svd_pchP, p->pwp-p->prp, forced );
    
    if (pp->i_svd_pchP!=NULL && (forced || p->pwp==p->prp)) { /* svd buff empty */
//      printf( "RELEASE PIPE_SVD %08X %08X\n", p,spP->u.pipe.i_svd_pchP );
        releasePipe( pid, spP );          /* release and restore it */
        pp->pchP      = pp->i_svd_pchP;
        pp->i_svd_pchP= NULL; 
    }
} /* releasePipe_svd */



os9err pPopen(ushort pid, syspath_typ *spP, ushort *modeP, char* name)
/* open pipe, make it anonymous if name==NULL, named otherwise */
{
    uint32_t pipesz;
    ushort  k;
    Boolean cre= IsCrea(*modeP);
    
  //process_typ*  cp = &procs[pid];
  //printf( "pPopen='%s' %d\r\n", name, cp->cre_initsize );
        
    /* make sure that new descriptor has no pipe name yet */
    strcpy( spP->name,"" );      /* unnamed pipe: no name */
    
    if (name   !=NULL && 
        name[0]!=NUL) {
        /* first see if another descriptor exists for that pipe name */
        for (k=1; k<MAXSYSPATHS; k++) {
            if (syspaths[ k ].type==fPipe) {
                if (syspaths[ k ].name[ 0 ]!=NUL) {
                    /* this is a named pipe */
                    if (ustrcmp(name,syspaths[ k ].name)==0) {
                        /* pipe with specified name exists */
                        debugprintf(dbgFiles,dbgDetail,("# pPopen: named pipe '%s' already exists as sp=%d\n",name,k));
                        break;
                    }
                }
            }
        }
        
        if (k>0 && k<MAXSYSPATHS) {
            /* pipe with specified name exists */       
            if (cre) {
                /* creating existing named pipe is not possible */
                return os9error(E_CEF);
            }
            else {
                /* open existing named pipe, simply link path */
                debugprintf(dbgFiles,dbgDetail,("# pPopen: linking existing pipe '%s' (sp=%d), \n",name,k));
                syspaths[k].linkcount++;          /* simply link the path */
                *modeP=k; /* (mis-)use mode to return syspath number */
                return E_PLINK; /* special error signals usage of an existing path record */
            }
        }
        else {
            /* no pipe with specified name exists yet */
            if (cre) {
                /* ok, create new named pipe */
                strncpy( spP->name,name,OS9NAMELEN );
            }
            else {
                /* cannot open, pipe does not exist */
                return os9error(E_PNNF);
            }
        }
    }

    /* new pipe */
    pipesz= DEFAULTPIPESZ;
    if (cre && *modeP & 0x20) {
        /* special size */
            pipesz=procs[pid].os9regs.d[2];
        if (pipesz<MINPIPESZ) pipesz=MINPIPESZ;
    }
    
    /* allocate buffer */
                        spP->u.pipe.i_svd_pchP= NULL; /* initialize */
             pPopt( pid,spP, (byte*)&spP->opt ); /* no err returned */
    return getPipe( pid,spP, pipesz );
} /* pPopen */



os9err pPclose( ushort pid, syspath_typ* spP )
/* close pipe */
{       
  pipe_typ* pp= &spP->u.pipe;
  
  if (spP->name[ 0 ]!=NUL) { // named pipe and not yet empty ?
    if   (pp->i_svd_pchP!=NULL) {
      if (pp->i_svd_pchP->prp!=
          pp->i_svd_pchP->pwp) return 1;
    } // if
      
    if (pp->pchP->prp!=
        pp->pchP->pwp) return 1;
  } // if
  
  
  if (pp->i_svd_pchP!=NULL) releasePipe_svd( pid, spP, true );    
  releasePipe( pid,spP ); 
  return 0;
} /* pPclose */


os9err pPdelete ( _pid_, _spP_, ushort *modeP, char* pathname )
/* delete pipe */
{
  char pipename[ OS9NAMELEN ];
  int  k;
  syspath_typ* spK;
  
  debugprintf(dbgFiles,dbgNorm,("# pPdelete '%s' (%d)", pathname, *modeP ));
  nullterm( pipename,&pathname[ 6 ],OS9NAMELEN );
  if       (pipename[ 0 ]==NUL) return 0;
  
  /* first see if another descriptor exists for that pipe name */
  for (k=1; k<MAXSYSPATHS; k++) {
        spK= &syspaths[ k ];
    if (spK->type==fPipe &&
        spK->name[ 0 ]!=NUL) { /* this is a named pipe */
      if (ustrcmp( pipename, spK->name)==0) {
        /* pipe with specified name exists */
        debugprintf(dbgFiles,dbgDetail,("# pPopen: named pipe '%s' already exists as sp=%d\n", pipename, k ));
        if (spK->linkcount>0 ||
            spK->u.pipe.pchP->consumers>0) return E_SHARE; // its in use at the moment
        
        spK->type= fNone; // delete it really
        return 0;
      }
    } // if
  } // for

  return E_PNNF;
} /* pPdelete */


static void PipePutc( pipechan_typ* p, char c )
{
      *(p->pwp++)= c;
    if (p->pwp  >= p->buf+p->size) p->pwp= p->buf; /* wrap */
} /* PipePutC */


static void Reactivate( ushort pid, process_typ* cp, const char* callingProc )
{
  if (cp->state!=pWaitRead &&       /* this statement costed me 2 days debugging !! */
      cp->state!=pWaiting  &&       /* and this one another 1.5 days !!! */
      cp->state!=pWaitWrite)        /* new: don't repeat those mistakes for blocked writes either */
    set_os9_state( pid, pActive, callingProc );  /* re-activate */
} /* Reactvate */


static os9err pWriteSysTaskExe( ushort  pid, syspath_typ* spP,
                                uint32_t *lenP, char* buffer, Boolean wrln, systaskfunc_typ wr_func )
{
    os9err        err= 0;
    int           numfree, remaining, bytes, nn;
    byte*         buf;
    pipechan_typ* p= spP->u.pipe.pchP;
    char          c;
    struct _sgs*  ot; /* path opt table */
    char          eorch;
    Boolean       wrln_break;
    process_typ*  cp= &procs[pid];
    
    if (spP->name[ 0 ]!=NUL) {
      GetTim( &p->pipeTim );
    } // if

    /* find out how many bytes are free in the buffer; leave always one free */
  //if (p->prp<=p->pwp) numfree= p->prp-p->pwp + p->size-SAFETY; /* CR safety */
  //else                numfree= p->prp-p->pwp          -SAFETY; 
    numfree= p->size - Pipe_NReady( p ) - SAFETY;

    /* first, copy as much as possible into the buffer */
    remaining= *lenP - p->bwritten; /* remaining to be written */
    bytes= numfree < remaining ? numfree : remaining; /* what we can write now */
    debugprintf(dbgFiles,dbgDetail,("# pWriteSysTaskExe (ln=%s): %ld bytes still requested by pid=%ld, %ld already written, %ld free in pipe\n",
                                       wrln ? "true":"false", remaining, pid, p->bwritten, numfree));
        
    /* copy loop */
    buf= (byte*)buffer + p->bwritten; /* position the buffer pointer */
    debugprintf(dbgFiles,dbgDeep,("# pWriteSysTaskExe: buffer start=$%lX, writing now from $%lX\n",buffer,(ulong) buf));

    if (spP->type==fTTY) {
         ot   = (struct _sgs*)&spP->opt;
         eorch= ot->_sgs_eorch;
    }
    else eorch= CR;
    wrln_break= (wrln && eorch!=NUL);

          nn= bytes;
    while(nn>0) {
        c= *buf++;

    //  if (spP->type!=fPTY || c!=NUL) /* don't use this: it causes problems !!! */
        PipePutc( p,c ); /* simply echo */
        nn--;
        
        if (wrln_break && c==eorch) { /* last written was a CR */
            if (p->do_lf) {
                p->do_lf= false;
                PipePutc( p, LF );
            } // if
            
            remaining= 0;
            break; /* writeln aborts at first CR found */
        }
        
        remaining--;
    } /* while */

    if (debugcheck(dbgFiles,dbgDeep)) {
        char *dp= buffer + p->bwritten; /* at start of buffer */
        ushort kk;
        uphe_printf("pWriteSysTaskExe: written='");
        for (kk=0; kk<nn; kk++) putc(*dp++,stderr);
        upe_printf ("'\n");
    } // if
    
    p->bwritten+= bytes-nn; /* we have written so many now */
    if (remaining<0) remaining= 0;
    
    debugprintf(dbgFiles,dbgDetail,("# pWriteSysTaskExe: %ld bytes written, remaining now=%ld\n",nn,remaining));

    if (bytes>nn &&                      spP->signal_to_send!=0) {
    //debugprintf(dbgSysCall,dbgNorm,("# SEND SIGNAL isInt=%d: pid=%d => pid=%d sig=%d\n", 
    //                                   cp->isIntUtil, pid, spP->signal_pid, spP->signal_to_send ));
                                         
      err= send_signal( spP->signal_pid, spP->signal_to_send );
      if (!err)                          spP->signal_to_send= 0;
    } // if

    /* check how things go on */
  //if (remaining || cp->state==pSysTask) printf( "FULL remain=%d pid=%d state=%d\n", remaining, pid, cp->state ); 
    if (remaining) {
      //if (cp->isIntUtil)
      //  debugprintf(dbgSysCall,dbgNorm,("# isInt=%d remaining=%d: pid=%d state=%d bytes=%d nn=%d arbitrate=%d\n", 
      //                                     cp->isIntUtil, remaining, pid, cp->state, bytes, nn, arbitrate ));
    
        /* caller wants to write more... */
        if (spP->linkcount<2 && spP->name[0]==0) {
            /* ...but no one else will read it, so end things now (unnamed pipes only) */
            debugprintf(dbgFiles,dbgDetail,("# pWriteSysTaskExe: aborting pipe write with %ld total bytes written and E_WRITE\n",p->bwritten));
            Reactivate( pid, cp, "Reactivate pWriteSysTaskExe (disconnect)" );
            return os9error(E_WRITE);   /* pipe is broken */
        }
        else {
            /* ... so wait for some consumer to empty buffer */
            //if (procs[currentpid].isIntUtil)
            //  printf( "%d goto systemtask %d\n", currentpid, cp->state==pSysTask ); /* %bfo% */
            
            if (cp->state!=pSysTask) {
              //if (cp->isIntUtil || procs[pid].isIntUtil) 
              //  printf( "ALARM 1\n" );
                set_os9_state( pid, pSysTask, "pWriteSysTaskExe" ); /* goto systask */
                cp->systask_offs= 0; /* by default */
            } // if
                
            cp->systask= wr_func;
            cp->systaskdataP= (void *) spP;
            /* leave it as systask */
        }
    }
    else {
        /* done, return to caller */
        Reactivate( pid, cp, "Reactivate pWriteSysTaskExe" );

        *lenP= p->bwritten; /* number of bytes written to pipe in that I/O call */
        debugprintf(dbgFiles,dbgDetail,("# pWriteSysTaskExe: pipe write successful, %ld total bytes written\n",p->bwritten));
        /* go on */     
    }
    
    return 0;
} /* pWriteSysTaskExe */



/* system task routines to complete pipe write request */
static os9err pWriteSysTask( ushort pid, syspath_typ* spP, regs_type* rp )
{
    os9err   err;
    ulong    dd= procs[pid].systask_offs;
    char*    a0= (char*)(rp->a[0] + dd);
    uint32_t d1= (uint32_t)(rp->d[1] - dd);

    err= pWriteSysTaskExe( pid,spP, &d1,a0, false, (systaskfunc_typ)pWriteSysTask );
    rp->d[1]= d1 + dd;
    return err;
} /* pWriteSysTask */

static os9err pWriteSysTaskLn( ushort pid, syspath_typ* spP, regs_type* rp )
{
    os9err   err;
    ulong    dd= procs[pid].systask_offs;
    char*    a0= (char*)(rp->a[0] + dd);
    uint32_t d1= (uint32_t)(rp->d[1] - dd);

    err= pWriteSysTaskExe( pid,spP, &d1,a0, true,  (systaskfunc_typ)pWriteSysTaskLn );
    rp->d[1]= d1 + dd;
    return err;
} /* pWriteSysTaskLn */

    


/* write to pipe buffer */
os9err pPwrite( ushort pid, syspath_typ* spP, uint32_t *n, char* buffer )
{
    debugprintf( dbgFiles,dbgDetail,("# pPwrite: requests %ld bytes\n",*n ));
    spP->u.pipe.pchP->bwritten= 0; /* start of new write request */
    
    /* system task routine will do the rest */
    return pWriteSysTaskExe( pid,spP, n,buffer, false, (systaskfunc_typ)pWriteSysTask );
} /* pPwrite */


/* writeln to pipe buffer */
os9err pPwriteln( ushort pid, syspath_typ* spP, uint32_t *n, char* buffer )
{
    debugprintf( dbgFiles,dbgDetail,("# pPwriteln: requests %ld bytes\n",*n ));
    spP->u.pipe.pchP->bwritten= 0; /* start of new write request */

    /* system task routine will do the rest */
    return pWriteSysTaskExe( pid,spP, n,buffer, true,  (systaskfunc_typ)pWriteSysTaskLn );
} /* pPwriteln */


/* <syW>: if true, it is already in SysTask write mode */
static os9err pReadSysTaskExe( ushort  pid, syspath_typ *spP,
                               uint32_t *lenP, char* buffer, Boolean rdln, Boolean syW, systaskfunc_typ rd_func )
{
    uint32_t      numready,remaining,bytes,nn;
    byte*         buf;
    pipe_typ*     pp= &spP->u.pipe;
    pipechan_typ* p =  pp->pchP;
    process_typ*  cp= &procs[pid];
    
    if (pp->i_svd_pchP!=NULL) releasePipe_svd( pid, spP, false );    
    
    /* find out how many bytes are here to be read */
  //if (p->prp>p->pwp) numready= p->pwp-p->prp + p->size;
  //else               numready= p->pwp-p->prp;
    numready= Pipe_NReady( p );

    /* first, copy as much as wanted or ready */
    remaining= *lenP - p->bread; /* remaining to be read */
    bytes    = remaining>numready ? numready : remaining; /* what we can read now */
    debugprintf(dbgFiles,dbgDetail,("# pReadSysTaskExe (ln=%s): %ld bytes still requested by pid=%d, %ld already delivered, %ld ready in pipe\n",
                                       rdln ? "true":"false", remaining, pid, p->bread, numready));
    /* copy loop */
    nn = 0;
    buf= (byte*)buffer + p->bread; /* position the buffer pointer */
    debugprintf( dbgFiles,dbgDeep,("# pReadSysTaskExe: buffer start=$%lX, reading now to $%lX\n",
                                      buffer, (ulong)buf ));

    for (nn=0; nn<bytes; nn++) {
        *buf++=*(p->prp++);
        if (p->prp >= p->buf+p->size) p->prp= p->buf; /* wrap */
        if (rdln && *(buf-1)==CR) {
            /* last was a CR */
            remaining= ++nn; /* set remaining such that we're done now */
            break; /* readln aborts at first CR found */
        }
    } /* for */
    
    if (debugcheck(dbgFiles,dbgDeep)) {
        char* dp= (char*)buffer + p->bread; /* at start of buffer */
        ushort kk;
        uphe_printf("pReadSysTaskExe: read='");
        for (kk=0; kk<nn; kk++) putc( *dp++, stderr );
        upe_printf ("'\n");
    } // if
    
    p->bread  +=nn; /* we have read so many now */
    remaining -=nn; /* calc what we've left */
    debugprintf(dbgFiles,dbgDetail,("# pReadSysTaskExe: %ld bytes read, remaining %ld (link=%d, consumers=%d)\n",
                nn,remaining,spP->linkcount,p->consumers));

    if (!rdln && p->bread>0) remaining= 0; /* don't wait in normal read mode, when got at least 1 char */

    /* check how things go on */
  //if (remaining || cp->state==pSysTask) printf( "READ remain=%d pid=%d state=%d\n", remaining, pid, cp->state ); 
    if (remaining) {
        /* caller wants more... */
        if (spP->linkcount<=p->consumers || p->consumers<=0) { /* eof condition must be checked !! */
            /* ...but no one else will deliver, so end things now */
            debugprintf(dbgFiles,dbgDetail,("# pReadSysTaskExe: aborting pipe read with %ld total bytes read%s\n",
                                               p->bread, p->bread==0 ? " and E_EOF" : ""));
          //set_os9_state( pid, pActive ); /* re-activate, finish I$Read(Ln) now */
            if (!syW) Reactivate( pid, cp, "Reactivate pReadSysTaskExe (disconnect)" );
            p->consumers--; /* One waiting consumer less */
            if    (p->bread==0) return os9error(E_EOF); /* pipe is empty */
            *lenP= p->bread; /* there was still something in the pipe */
            /* next call will cause EOF */
        }
        else {
            /* ...so wait for writing end of pipe to send more data */
          //if (cp->isIntUtil || procs[pid].isIntUtil) 
          //  printf( "ALARM 2\n" );
            if (syW) printf( "ALARM double SysTask\n" );
            set_os9_state( pid, pSysTask, "pReadSysTaskExe" ); /* stay in (or enter) systask */
            cp->systask     = rd_func;
            cp->systaskdataP= (void*)spP;
            /* leave it as systask and as consumer */
        }
    }
    else {
        /* done, return to caller */
        if (!syW) Reactivate( pid, cp, "Reactivate pReadSysTaskExe" );
        
        *lenP= p->bread; /* number of bytes read from pipe in that I/O call */
        p->consumers--;  /* We are not a waiting consumer any more */
        debugprintf(dbgFiles,dbgDetail,("# pReadSysTaskExe: pipe read successful, %ld total bytes read\n",
                    p->bread));
        /* go on */     
    }
    
    return 0;
} /* pReadSysTaskExe */



/* system task routines to complete pipe read request */    
static os9err pReadSysTask( ushort pid, syspath_typ* spP, regs_type* rp )
{
    os9err   err;
    char*    a0= (char*)FROM68K(rp->a[0]);
    uint32_t d1= (uint32_t)rp->d[1];

    err= pReadSysTaskExe( pid,spP, &d1,a0, false, false, (systaskfunc_typ)pReadSysTask );
    rp->d[1]= d1;
    return err;
} /* pReadSysTask */

static os9err pReadSysTaskLn( ushort pid, syspath_typ* spP, regs_type* rp )
{
    os9err   err;
    char*    a0= (char*)FROM68K(rp->a[0]);
    uint32_t d1= (uint32_t)rp->d[1];

    err= pReadSysTaskExe( pid,spP, &d1,a0, true,  false, (systaskfunc_typ)pReadSysTaskLn );
    rp->d[1]= d1;
    return err;
} /* pReadSysTaskLn */


static os9err ShowPipeDir( syspath_typ* spP, char* buffer )
{
  os9direntry_typ os9dirent;
  int len, k, n= 0;
  Boolean found= false;
  syspath_typ* spK;
  
  for (k=1; k<MAXSYSPATHS; k++) {
        spK= &syspaths[ k ];
    if (spK->type==fPipe &&
        spK->name[ 0 ]!=NUL) { /* this is a named pipe */
      if (n>=spP->u.pipe.pchP->pipeDirCnt) { found= true; break; }
      n++;  
    } // if
  } // for
 
  if (!found) return E_EOF;
  
  strcpy( os9dirent.name, spK->name );
  spP->u.pipe.pchP->pipeDirCnt++;
  
  len= strlen( os9dirent.name );
  os9dirent.name[ len-1 ] |= 0x80; /* set old-style terminator */
  os9dirent.fdsect= os9_long( k );
  
  memcpy( buffer, (byte*)&os9dirent, DIRENTRYSZ );
  return 0;
} // ShowPipeDir


/* read from pipe buffer */
os9err pPread( ushort pid, syspath_typ* spP, uint32_t *n, char* buffer )
{
  Boolean       syW;
  process_typ*  cp= &procs[pid];
  pipechan_typ* p = spP->u.pipe.pchP;
    
  if (spP->mode & 0x80) { // treat it as directory ?
    if (*n<DIRENTRYSZ) return E_EOF;
    *n=    DIRENTRYSZ;
    return ShowPipeDir( spP, buffer );
  } // if
    
  if (p->broken) return E_EOF;
    
  debugprintf( dbgFiles,dbgDetail,("# pPread: requests %ld bytes\n",*n ));
  p->bread= 0;    /* start of new read request */
  p->consumers++; /* I'm now a consumer, too */

  /* system task routine will do the rest */
  syW= cp->state==pSysTask;
  return pReadSysTaskExe( pid,spP, n,buffer, false, syW, (systaskfunc_typ)pReadSysTask );
} /* pPread */
        
    
/* readln from pipe buffer */
os9err pPreadln( ushort pid, syspath_typ *spP, uint32_t *n, char* buffer )
{
  Boolean       syW;
  process_typ*  cp = &procs[pid];
  pipechan_typ* p  = spP->u.pipe.pchP;
  if           (p->broken) return E_EOF;
    
  debugprintf( dbgFiles,dbgDetail,("# pPreadln: requests %ld bytes\n",*n ));
  p->bread=0;     /* start of new read request */
  p->consumers++; /* I'm now a consumer, too */

  /* system task routine will do the rest */
  syW= cp->state==pSysTask;
  return pReadSysTaskExe( pid,spP, n,buffer, true, syW, (systaskfunc_typ)pReadSysTaskLn );
} /* pPreadln */
        



/* constant option sections for non-Console SCF I$GetStt */
const byte pipestdopts[OPTSECTSIZE]= 
                  { 2, /* PD_DTP 2=PIPE */
                    0, /* reserved  */
           0,0,0,0x90, /* PD_BufSiz */
 
                       0,0,  0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
              
           0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0 };


os9err pPopt( _pid_, _spP_, byte* buffer )
/* get options from console */
{   memcpy( buffer, pipestdopts, OPTSECTSIZE ); return 0;
} /* pPopt */



/* check for EOF in pipe */
os9err pPeof( _pid_, syspath_typ *spP )
{
    uint32_t numready;
    pipechan_typ* p= spP->u.pipe.pchP;

    /* number of chars ready to be read in the pipe */
  //if (p->prp>p->pwp) numready= p->pwp-p->prp + p->size;
  //else               numready= p->pwp-p->prp;
    numready= Pipe_NReady( p );

    debugprintf(dbgFiles,dbgDetail,("# pPeof: numready=%u, linkcount=%d\n",
                numready,spP->linkcount));
    /* if buffer empty and no writer left: E_EOF */
    if (numready==0 && spP->linkcount<2) return os9error(E_EOF);
    
    return 0; 
} /* pPeof */


/* check ready */
os9err pPready( ushort pid, syspath_typ* spP, uint32_t *n )
{
    pipe_typ*     pp= &spP->u.pipe;
    pipechan_typ* p =  pp->pchP;
    
  //*n= p->pwp-p->prp;
  //if (p->pwp<p->prp) *n+= p->size; /* wrapper */
    
        *n= Pipe_NReady( p );
    if (*n!=0) return 0; /* ready */

    /* release large pipe if allocated at an internal command */    
    /* do not cross over for reading */
    if (spP->type==fPTY) {
        if (p->broken) { *n= 1; return 0; } /* handle broken half-pipe correctly */
    }
        
    if (pp->i_svd_pchP!=NULL) releasePipe_svd( pid, spP, false );    
    return os9error(E_NOTRDY);
} /* pPready */


os9err pPFDInf( _pid_, _spP_, uint32_t *maxbytP,
                              uint32_t *fdinf,  byte* buffer )
{
  #define       FDS 16
  byte          fdbeg[FDS];                  /* buffer for preparing FD */
  syspath_typ*  spK= &syspaths[ *fdinf ];
  pipechan_typ* p;
  struct tm*    tim; 

  if  (spK==NULL) return E_UNKSVC;
  p  = spK->u.pipe.pchP; 
  tim= &p->pipeTim; 
  
  fdbeg[  0 ]= (byte)spK->fileAtt;
  fdbeg[  1 ]= 0;
  fdbeg[  2 ]= 0;
  fdbeg[  3 ]= tim->tm_year;
  fdbeg[  4 ]= tim->tm_mon+1; /* somewhat different month notation */
  fdbeg[  5 ]= tim->tm_mday;
  fdbeg[  6 ]= tim->tm_hour;
  fdbeg[  7 ]= tim->tm_min;
  fdbeg[  8 ]= 0;
  fdbeg[  9 ]= 0;
  fdbeg[ 10 ]= 0;
  fdbeg[ 11 ]= 0;
  fdbeg[ 12 ]= 0;
  fdbeg[ 13 ]= 0;
  fdbeg[ 14 ]= 0;
  fdbeg[ 15 ]= 0;
  fdbeg[ 13 ]= tim->tm_year;
  fdbeg[ 14 ]= tim->tm_mon+1;
  fdbeg[ 15 ]= tim->tm_mday;
  
  SET_OS9L(fdbeg, 9, Pipe_NReady( p ));
  
  memcpy( buffer, fdbeg, *maxbytP>FDS ? FDS : *maxbytP );
  return 0;
} /* pPFDInf */


/* get pipe size */
os9err pPsize( _pid_, syspath_typ* spP, uint32_t *sizeP )
{
    *sizeP= spP->u.pipe.pchP->size-1; /* return max available size of pipe buffer */
    return 0;
} /* pPsize */



/* set pipe size */
os9err pPsetsz( _pid_, syspath_typ* spP, uint32_t *sizeP )
{
    pipechan_typ* p= spP->u.pipe.pchP;

    if (*sizeP==0) {
        /* clear pipe buffer */
        p->pwp= p->buf;
        p->prp= p->buf;
    } /* if */

    return 0;
} /* pPsetsz */



/* ------------------------- packet manager routines --------- */
os9err pKopen( ushort pid, syspath_typ* spP, _modeP_, char* pathname )
{
    os9err        err;  
    syspath_typ*  spK;
    pipechan_typ* k;
    int           n;
    char          tty_cmp[OS9NAMELEN];
    
  //process_typ*  cp = &procs[pid];
  //printf( "pKopen='%s' %d\r\n", pathname, cp->cre_initsize );
        
    strcpy( spP->name,&pathname[1] );
    strcpy( tty_cmp, spP->name );
            tty_cmp[0]= 't';
        
    /* check whether this pty is already open by another syspath */
    for (n=0; n<MAXSYSPATHS; n++) {
        spK= get_syspathd( pid,n );
        if     (spK!=NULL       &&
                spK!=spP        &&
                spK->linkcount>0) {
            k=  spK->u.pipe.pchP;
            
            if (spK->type==fPTY &&
                ustrcmp( spP->name,spK->name )==0) return E_DEVBSY;
            if (spK->type==fTTY &&
                k!=NULL         &&
                k->broken       &&
                ustrcmp( tty_cmp,  spK->name )==0) return E_DEVBSY;
        }
    } /* for */
    
    err= pKopt         ( pid,spP, (byte*)&spP->opt ); /* no err returned */
    err= ConnectPTY_TTY( pid,spP );
    return err;
} /* pKopen */



os9err pKclose( ushort pid, syspath_typ* spP )
{
    syspath_typ*  spK;
    pipechan_typ* p= spP->u.pipe.pchP;
    pipechan_typ* k;
    int           i, j, n, lk;
//  int           snd;
    
    if (p==NULL) return 0; /* already disconnected ! */

        lk= p->sp_lock;
    if (lk==spP->nr) { releasePipe( pid,spP ); return 0; }
    
    for     (i=0; i<MAXPROCESSES; i++) {
        for (j=0; j<MAXUSRPATHS;  j++) {
                n= procs[i].usrpaths[j];
            if (n>0 && n<MAXSYSPATHS) {
                        spK= &syspaths[n];
                if     (spK!=NULL) {
                    k=  spK->u.pipe.pchP;
                    if (spK->type==fTTY &&
                        k!=NULL         &&
                       (k->sp_lock==spP->nr ||
                        k->sp_lock==lk)) {
                        send_signal( i, S_HangUp );
                        break; /* only one signal per process */
                    }
                }
            } /* if */
            
        } /* for */
    } /* for */
        
//  snd= 0;
//  for (n=0; n<MAXSYSPATHS; n++) {
//              spK= get_syspath( pid,n ); 
//      if     (spK!=NULL) {
//          k=  spK->u.pipe.pchP;
//          if (spK->type==fTTY &&
//              k!=NULL         &&
//             (k->sp_lock==spP->nr ||
//              k->sp_lock==lk) && snd==0) snd= spK->lastwritten_pid;
//      }
//  } /* for */
//  if (snd!=0 &&
//      snd!=pid) send_signal( snd, S_HangUp );

    for (n=0; n<MAXSYSPATHS; n++) {
                spK= get_syspath( pid,n ); 
        if     (spK!=NULL) {
            k=  spK->u.pipe.pchP;
            if (spK->type==fTTY &&
                k!=NULL         &&
                k->sp_lock==spP->nr) {
                k->broken = true;
                k->sp_lock= spK->nr; /* short-circuit them all */
            }
        }
    } /* for */

    return 0;
} /* pKclose */



/* read/readln will be done directly */
os9err pKread   ( ushort pid, syspath_typ* spP, uint32_t *n, char* buffer )
{   return pPread      ( pid, spP, n,buffer );
}

os9err pKreadln ( ushort pid, syspath_typ* spP, uint32_t *n, char* buffer )
{   return pPreadln    ( pid, spP, n,buffer );
}



/* write/writeln will be done with cross-over */
os9err pKwrite  ( ushort pid, syspath_typ* spP, uint32_t *n, char* buffer )
{   syspath_typ*              spC= crossedPath( pid,spP );
    return pPwrite     ( pid, spC, n,buffer );
} /* pKwrite */

os9err pKwriteln( ushort pid, syspath_typ* spP, uint32_t *n, char* buffer )
{   syspath_typ*              spC= crossedPath( pid,spP );
    return pPwriteln   ( pid, spC, n,buffer );
} /* pKwriteln */



/* emulate that a real pkman device is connected */ 
const byte pkstdopts[OPTSECTSIZE]= 
                  { 8, /* PD_DTP 8=PK */
                    0,
                 0x0f,
                    0,
                    0,
                 0x18,
                    0,
                 0x50,
                             0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
              
           0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0 };



os9err pKopt( _pid_, _spP_, byte *buffer )
/* get options from console */
{   memcpy( buffer, pkstdopts, OPTSECTSIZE ); return 0;
} /* pKopt */


os9err pKpos( _pid_, _spP_, uint32_t *posP )
/* get current file position */
{   *posP= 0; return 0;
} /* pKpos */



os9err pKready( ushort pid, syspath_typ *spP, uint32_t *n )
/* gs_ready will be done directly */
{   return pPready( pid,spP, n );
} /* pKready */



os9err pKlock( ushort pid, _spP_, uint32_t *d0, uint32_t *d1)
/* creates tty/pty system paths and locks them together */
/* they are named tty0,tty1,... and pty0,pty1,... */
/* the lowest free name will be taken */ 
{
    os9err       err;
    ushort       spX,  spY,  up;
    syspath_typ* spPX;
    syspath_typ* spPY;
    syspath_typ* spK; /* must be declared separately, don't know why ?! */
    char         ind[5];
    char         pty[OS9PATHLEN];
    char         tty[OS9PATHLEN];
    int          k, newPty= 0;
    Boolean      found;
    
    while (true) { /* search for an unused tty/pty pair */
        sprintf    ( ind, "%02d", newPty );
        strcpy( tty,"tty" );
        strcat( tty, ind  );
        strcpy( pty,"pty" );
        strcat( pty, ind  );
    
        found= true;
        for (k=0; k<MAXSYSPATHS; k++) {
                spK= get_syspath( pid,k ); 
            if (spK!=NULL &&
                spK->linkcount>0) {
                if (strcmp( spK->name,tty )==0 ||
                    strcmp( spK->name,pty )==0) { found= false; break; }
            }
        } /* for */
        
        if (found) break;
        newPty++;
    } /* while */
    
    /* use pipe structure for the tty/pty system */
    err= usrpath_new( pid,  &up, fPTY ); if (err) return err;
    spX= procs[pid].usrpaths[up];
                       spPX= get_syspath( pid,spX ); 
    strcpy           ( spPX->name, pty );
    getPipe     ( pid, spPX, DEFAULTPTYSZ );
    *d0= retword           ( up ); 
    
    err= usrpath_new( pid,  &up, fTTY ); if (err) return err;
    spY= procs[pid].usrpaths[up];
                       spPY= get_syspath( pid,spY ); 
    strcpy           ( spPY->name, tty );
    getPipe     ( pid, spPY, DEFAULTPTYSZ );
    err= pSCFopt( pid, spPY, (byte*)&spPY->opt );    /* default console options */
    spPY->term_id=          newPty+TTY_Base; /* compatible to serial sub system */
    InstallTTY       ( spPY,newPty+TTY_Base );
    *d1= retword           ( up );
    
    spPX->u.pipe.pchP->sp_lock= spY; /* cross over */
    spPY->u.pipe.pchP->sp_lock= spX;
    
    spPX->u.pipe.locked_open= true;
    spPY->u.pipe.locked_open= true;
    return 0;        
} /* pKlock */




/* ------------------------------------------------------------------------- */
syspath_typ* crossedPath( ushort pid, syspath_typ* spP )
{
    pipechan_typ *pcP, *pcC;
    syspath_typ  *spC, *spK, *spV;
    ushort        k;

        pcP= spP->u.pipe.pchP;
    if (pcP==NULL) return spP;

        spC= get_syspath( pid,pcP->sp_lock ); /* crossover */
    if (spC==NULL) return spP;
    
        pcC= spC->u.pipe.pchP;
    if (pcC==NULL) return spP;

    if (spC->linkcount>1 ||
        spP->linkcount>1) {
        pcC->sp_lock= spP->nr; return spC;
    }
    
    if (spC->type==fTTY) spV= spC;
    else                 spV= spP;
    
    for (k=1; k<MAXSYSPATHS; k++) {
            spK= &syspaths[k];
        if (spK->type==fTTY && 
            spK!=spV && ustrcmp(spK->name,spV->name)==0 &&
            spK->linkcount>0) { /* there is another path */
            pcC->sp_lock= spP->nr; return spC;
        }
    }

    if (spC->type==fTTY &&
        spC->u.pipe.locked_open) { 
        pcP->broken= true;
        return spP; /* no crossover */
    }
    
    return spC;
} /* crossedPath */



os9err ConnectPTY_TTY( ushort pid, syspath_typ* spP )
{
    syspath_typ*   spK;
    ushort         k;
    Boolean        found= false;
    ptype_typ      type_inv;    

    if (spP->type==fTTY) type_inv= fPTY;
    if (spP->type==fPTY) type_inv= fTTY;

    for (k=1; k<MAXSYSPATHS; k++) {
            spK= &syspaths[k]; /* is there already a tty with the same name ? */
        if (spK->type==spP->type && 
            spK!=spP             && 
            ustrcmp(spK->name,spP->name)==0) {
                spP->u.pipe.pchP= spK->u.pipe.pchP;      /* get existing pipe */
                
                spK= get_syspathd( pid, spP->u.pipe.pchP->sp_lock );
            if (spK!=NULL) {                             /* assign new number */
                spK->u.pipe.pchP->sp_lock= spP->nr;      /* for the feedback  */
                found= true; break;
            }
        }
    }

    if (!found) {
        getPipe( pid, spP, DEFAULTPTYSZ );

        for (k=1; k<MAXSYSPATHS; k++) {
                spK= &syspaths[k]; /* is there already a tty with the same name ? */
            if (spK->type==type_inv && ustrcmp(&spK->name[1],&spP->name[1])==0) {
                spK->u.pipe.pchP->sp_lock= spP->nr;      /* make the feedback  */
                spP->u.pipe.pchP->sp_lock= k;            /* in both directions */
                found= true; break;
            }
        }
    }

    if (!found) spP->u.pipe.pchP->sp_lock= spP->nr; /* short circuit */
    return 0;
} /* ConnectPTY_TTY */



void PutCharsToTTY( ushort  pid, syspath_typ* spP,
                    uint32_t *lenP, char* buffer, Boolean wrln )
/* write characters to TTY buffer */
{
     if (wrln) pPwriteln( pid,spP, lenP,buffer );
     else      pPwrite  ( pid,spP, lenP,buffer );
} /* PutCharsToTTY */



void CheckInBufferTTY( ttydev_typ* mco )
/* check if there is something at the input and put it into buffer <mco> */
{
    os9err   err;
    long     cnt, k, cmax;
    char     buffer[INBUFSIZE];
    
    if (mco->installed &&
        mco->pid!=0    &&
        mco->spP->type==fTTY) {
        uint32_t ucnt;
        err= pPready( mco->pid, mco->spP, &ucnt ); cnt= ucnt; if (cnt==0) return;

        cmax= INBUFSIZE-mco->inBufUsed-1;
        if (cnt>cmax) cnt= cmax;
        ucnt= (uint32_t)cnt;
        err= pPread ( mco->pid, mco->spP, &ucnt, buffer ); cnt= ucnt; if (err) return;
  
               k= 0;
        while (k<cnt) {
          if (!KeyToBuffer( mco, buffer[k] )) { err= E_READ; break; }
          k++;
        } /* while */
    } /* if */
} /* CheckInBufferTTY */


/* eof */

