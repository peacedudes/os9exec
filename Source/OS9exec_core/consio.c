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
/*         for Apple Macintosh, PC, Linux     */
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
 *    Revision 1.22  2006/10/29 18:47:49  bfo
 *    "ConsGetC" for MacOS9 repaired (again) /
 *    Some commented old things removed
 *
 *    Revision 1.21  2006/09/03 20:48:20  bfo
 *    Some more small <devReady> adaption for MacOS9/Windows
 *
 *    Revision 1.20  2006/09/01 15:19:03  bfo
 *    Problem with empty console reading for MacOS9 fixed
 *
 *    Revision 1.19  2006/08/29 22:08:23  bfo
 *    New version introduced again, this time w/o 0x7f -> 0x08 conv
 *
 *    Revision 1.18  2006/08/21 19:47:12  bfo
 *    Switched back to the older version for the moment due to a
 *    problem with MGR programs.
 *
 *    Revision 1.17  2006/08/20 18:36:33  MG
 *    several additions and enhancemenet for pagination and others
 *    (Changes done by Martin Gregorie)
 *
 *    Revision 1.16  2006/07/06 22:59:48  bfo
 *    devReady for MacOS9 is ok again
 *
 *    Revision 1.15  2006/06/26 22:11:42  bfo
 *    Adaptions for Ctrl-C
 *
 *    Revision 1.14  2006/06/11 22:06:00  bfo
 *    set_os9_state with 3rd param <callingProc>
 *
 *    Revision 1.13  2006/06/08 08:15:04  bfo
 *    Eliminate causes of signedness warnings with gcc 4.0
 *
 *    Revision 1.12  2006/06/01 15:17:48  bfo
 *    Signedness adaptions (for gcc 4.0)
 *
 *    Revision 1.11  2006/02/19 16:15:05  bfo
 *    Header changes for 2006
 *
 *    Revision 1.10  2005/06/30 11:05:48  bfo
 *    Mach-O support
 *
 *    Revision 1.9  2004/11/27 12:00:39  bfo
 *    _XXX_ introduced
 *
 *    Revision 1.8  2004/11/20 11:44:06  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.7  2004/10/22 22:51:12  bfo
 *    Most of the "pragma unused" eliminated
 *
 *    Revision 1.6  2003/08/01 11:12:44  bfo
 *    /L2 getstat support
 *
 *    Revision 1.5  2002/08/09 22:38:53  bfo
 *    New procedure set_os9_state introduced and adapted everywhere
 *
 *    Revision 1.4  2002/07/30 16:47:19  bfo
 *    E-Mail adress bfo@synthesis.ch       is updated everywhere
 *
 *    Revision 1.3  2002/06/25 20:44:33  luz
 *    Added /lp printer support under windows. Not tested or even compiled for Mac
 *
 */

/* Console I/O routines */
/* ==================== */

#include "os9exec_incl.h"
#include <ctype.h>

/* --- local procedure definitions for object definition ------------------- */
void   init_Cons ( fmgr_typ* f );
os9err pCopen    ( ushort pid, syspath_typ*, ushort  *modeP, char* pathname );
os9err pCclose   ( ushort pid, syspath_typ* );
os9err pConsIn   ( ushort pid, syspath_typ*, uint32_t *maxlenP, char* buffer );
os9err pConsInLn ( ushort pid, syspath_typ*, uint32_t *maxlenP, char* buffer );
os9err pConsOut  ( ushort pid, syspath_typ*, uint32_t *maxlenP, char* buffer );
os9err pConsOutLn( ushort pid, syspath_typ*, uint32_t *maxlenP, char* buffer );

os9err pCopt     ( ushort pid, syspath_typ*,                    byte* buffer );
os9err pCpos     ( ushort pid, syspath_typ*, uint32_t *posP );
os9err pCready   ( ushort pid, syspath_typ*, uint32_t *n );
os9err pCsetopt  ( ushort pid, syspath_typ*,                    byte* buffer );

void   init_NIL  ( fmgr_typ* f );
os9err pEOF      ( ushort pid, syspath_typ*, uint32_t *maxlenP, char* buffer );

void   init_SCF  ( fmgr_typ* f );
os9err pSopen    ( ushort pid, syspath_typ*, ushort  *modeP, char* pathname );
os9err pSclose   ( ushort pid, syspath_typ* );
os9err pSBlink   ( ushort pid, syspath_typ*, uint32_t   *d2 );
os9err pGBlink   ( ushort pid, syspath_typ*, uint32_t   *d2 );
/* ------------------------------------------------------------------------- */

/* -----------------------  /term pause control  --------------------------- */
static int term_line = 0;
/* ------------------------------------------------------------------------- */

void init_Cons( fmgr_typ* f )
/* install all procedures of the console file manager */
{
    gs_typ* gs= &f->gs;
    ss_typ* ss= &f->ss;
    
    /* main procedures */
    f->open      = (pathopfunc_typ)pCopen;
    f->close     = (pathopfunc_typ)pCclose;
    f->read      = (pathopfunc_typ)pConsIn;
    f->readln    = (pathopfunc_typ)pConsInLn;
    f->write     = (pathopfunc_typ)pConsOut;
    f->writeln   = (pathopfunc_typ)pConsOutLn;
    f->seek      = (pathopfunc_typ)pBadMode; /* not allowed */
    
    /* getstat */
    gs->_SS_Size = (pathopfunc_typ)pUnimp;   /* -- not used */
    gs->_SS_Opt  = (pathopfunc_typ)pCopt;
    gs->_SS_DevNm= (pathopfunc_typ)pSCFnam;
    gs->_SS_Pos  = (pathopfunc_typ)pCpos;
    gs->_SS_EOF  = (pathopfunc_typ)pNop;         /* ignored */
    gs->_SS_Ready= (pathopfunc_typ)pCready;

    /* setstat */
    ss->_SS_Size = (pathopfunc_typ)pNop;         /* ignored */
    ss->_SS_Opt  = (pathopfunc_typ)pCsetopt;
    ss->_SS_Attr = (pathopfunc_typ)pNop;         /* ignored */
} /* init_Cons */

void init_NIL( fmgr_typ* f )
{
    gs_typ* gs= &f->gs;
    ss_typ* ss= &f->ss;
    
    /* main procedures */
    f->open      = (pathopfunc_typ)pNop;         /* ignored */
    f->close     = (pathopfunc_typ)pNop;         /* ignored */
    f->read      = (pathopfunc_typ)pEOF;      /* as in OS-9 */
    f->readln    = (pathopfunc_typ)pEOF;      /* as in OS-9 */
    f->write     = (pathopfunc_typ)pNop;         /* ignored */
    f->writeln   = (pathopfunc_typ)pNop;         /* ignored */
    f->seek      = (pathopfunc_typ)pBadMode; /* not allowed */

    /* getstat */
    gs->_SS_Size = (pathopfunc_typ)pUnimp;   /* -- not used */
    gs->_SS_Opt  = (pathopfunc_typ)pSCFopt;
    gs->_SS_DevNm= (pathopfunc_typ)pSCFnam;
    gs->_SS_Pos  = (pathopfunc_typ)pBadMode; /* not allowed */
    gs->_SS_EOF  = (pathopfunc_typ)pNop;         /* ignored */
    gs->_SS_Ready= (pathopfunc_typ)pBadMode; /* not allowed */

    /* setstat */
    ss->_SS_Size = (pathopfunc_typ)pNop;         /* ignored */
    ss->_SS_Opt  = (pathopfunc_typ)pNop;         /* ignored */
    ss->_SS_Attr = (pathopfunc_typ)pNop;         /* ignored */
} /* init_NIL */

void init_SCF( fmgr_typ* f )
/* currently implemented for support of the /vmod driver only */
{
    gs_typ* gs= &f->gs;
    ss_typ* ss= &f->ss;
    
    /* main procedures */
    f->open       = (pathopfunc_typ)pSopen;
    f->close      = (pathopfunc_typ)pSclose;
    f->read       = (pathopfunc_typ)pBadMode; /* not allowed */
    f->readln     = (pathopfunc_typ)pBadMode; /* not allowed */
    f->write      = (pathopfunc_typ)pBadMode; /* not allowed */
    f->writeln    = (pathopfunc_typ)pBadMode; /* not allowed */
    f->seek       = (pathopfunc_typ)pBadMode; /* not allowed */

    /* getstat */
    gs->_SS_Size  = (pathopfunc_typ)pUnimp;   /* -- not used */
    gs->_SS_Opt   = (pathopfunc_typ)pSCFopt;
    gs->_SS_DevNm = (pathopfunc_typ)pSCFnam;
    gs->_SS_Pos   = (pathopfunc_typ)pBadMode; /* not allowed */
    gs->_SS_EOF   = (pathopfunc_typ)pNop;         /* ignored */
    gs->_SS_Ready = (pathopfunc_typ)pBadMode; /* not allowed */
    gs->_SS_LBlink= (pathopfunc_typ)pGBlink;     /* specific */
    gs->_SS_Undef = (pathopfunc_typ)pVMod;

    /* setstat */
    ss->_SS_Size  = (pathopfunc_typ)pNop;         /* ignored */
    ss->_SS_Opt   = (pathopfunc_typ)pNop;         /* ignored */
    ss->_SS_Attr  = (pathopfunc_typ)pNop;         /* ignored */
    ss->_SS_LBlink= (pathopfunc_typ)pSBlink;     /* specific */
    ss->_SS_Undef = (pathopfunc_typ)pNop;         /* ignored */
} /* init_SCF */

/* --------------------------------------------------------- */

/* standard output write. Used only when TERMINAL_CONSOLE is not defined. */
#ifndef TERMINAL_CONSOLE
static long stdwrite(ushort pid, byte *p, long cnt, FILE* stream, Boolean wrln)
{
    /* %%% not soooo nice stuff here, but will change with new filters anyway */
    #define MAXLINELEN 300
    static char linebuffer[MAXLINELEN+1];
    static char *lbp = linebuffer;
    long k,i;
    process_typ* cp= &procs[pid];
   
    debugprintf(dbgTerminal,dbgDeep,("# stdwrite cnt=%d, from=$%lX\n",cnt,(ulong)p));
    i=lbp-linebuffer; /* number of chars in linebuffer */
    for (k=0; k<cnt; k++) {
        if (i==MAXLINELEN) {
            /* we have a problem here with the buffer, write two separate lines */
            fprintf(stream,"# **** OS9exec/nt: Line longer than %d chars, truncated on console\n",MAXLINELEN);
            i++;
        }
        if (i<MAXLINELEN) {
          *lbp=*p; /* copy char */
        }
      
      if (*p==CR) {
         /* end of line: now call filter on a line-per line basis */
            *lbp= 0; /* terminate line */
         
            /* --- call appropriate filter routine */
            if   (cp->stdoutfilter==NULL) writeline( linebuffer,stream );
            else (cp->stdoutfilter)                ( linebuffer,stream, cp->filtermem );

            lbp= linebuffer; /* reset buffer pointer */
            if (wrln) {
                cnt= ++k;  /* just as much */
                break; /* don't go further */
            }
      }
      else if (i<MAXLINELEN) { i++; lbp++; }
      p++;
   }
   
   return cnt;
} /* stdwrite */
#endif /* TERMINAL_CONSOLE */

#ifdef TERMINAL_CONSOLE
  /* put char to console and perform CR/LF expansion etc. */
  void ConsPutc( char c )
  {   
      /* save this info in terminal interface system */
      gLastwritten_pid= currentpid;
    
      if (gConsoleID>=TTY_Base) {
          #ifdef PIP_SUPPORT
            WriteCharsToPTY( &c,1, gConsoleID, false );
          #endif
          return;
      }

      write( 1,&c,1 );

      // not yet supported for Mac Classic/Carbon
      #ifdef win_unix
        lw_pid( &main_mco ); /* assign for later use */
      #endif
  } /* ConsPutc */

  void ConsPutcEdit( char c, Boolean alf, char eorch )
  /* put char to console and perform CR/LF expansion etc. */
  {
      ConsPutc ( c );          /* Auto LF */
      if (alf && c!=NUL && c==eorch) ConsPutc( LF );
  } /* ConsPutcEdit */
#endif

#ifdef TERMINAL_CONSOLE
/* get char from console and perform CR/LF conversion */
Boolean ConsGetc( char* c )
{
    int n;
    
    // other terminal ?
    if (gConsoleID>=TTY_Base) {
              n= ReadCharsFromPTY( c,1, gConsoleID);        
      return (n>0) && devIsReady;
    } // if

    #if defined windows32
      HandleEvent();
      n= ReadCharsFromTerminal( c,1, &main_mco );
      return (n>0) && devIsReady;
    
    #elif defined MACOS9
          n= fread( c,1,1, stdin );
      if (n!=1 || !devIsReady) return false;
    
    #elif defined MACOSX
      n= getchar(); // problems with fread
           devIsReady= (n!=-1);
      if (!devIsReady) return false;
      *c= n;
      
    #else
      #ifdef linux
        n= read( 0, c, 1 );

        // printf( "nn=%d  c=0x%02X\n", n, (unsigned int)c[0] );
        // fflush(0);
      #else
        n= fread( c,1,1, stdin );
 
        //if (n!=1 || !devIsReady) return false;
      #endif
             
           devIsReady= n>0;
      if (!devIsReady) return false;
    
      debugprintf(dbgTerminal,dbgDetail,("# ConsGetc: returns=%X\n",*c));
    #endif

    /* now swap 0x0A and 0x0D */
    if      (*c==LF) *c= CR; /* convert LF to OS-9 style CR */
    else if (*c==CR) *c= LF; /* convert CR to LF, in case we need it */

    return true;
} /* ConsGetc */

static os9err ConsRead( ushort pid, syspath_typ* spP,
                        uint32_t *maxlenP, char* buffer, Boolean edit, char endchar )
{
    os9err        err= 0; /* no err so far */
    long          cnt= 0;
    char          c;
    ulong         inputticks= GetSystemTick();
    process_typ*  cp= &procs[pid];
    struct _sgs*  ot= (struct _sgs*)&spP->opt; /* path opt table */
    Boolean       alf= ot->_sgs_alf;
    Boolean       dupMode= false;
    pipechan_typ* k;
    
    interactivepid=pid; /* set focus to this process */
    fflush(stdout);     /* ensure all is written out */
    clearerr(stdin);    /* make sure we are not stuck with a Cmd-. */
    
    if (cp->state==pWaitRead) {
        set_os9_state( pid, cp->saved_state, "ConsRead" );
        cnt=                cp->saved_cnt;
    }

    while (cnt<*maxlenP) {
        if (!dupMode && !ConsGetc(&c)) { 
            err= E_READ;
            if (!devIsReady) {
                k=  spP->u.pipe.pchP;
                if (spP->type==fTTY &&
                            k!=NULL && k->broken) {
                    err= E_EOF; break; /* pipe is broken */
                }
                    
                cp->saved_cnt  = cnt;
                cp->saved_state= cp->state;
                set_os9_state  ( pid, pWaitRead, "ConsRead" );
                arbitrate= true;
            }
            break; 
        }

        if (c!=NUL) { /* no check on NUL char for every sgs option */       
            /* check options */
            if (c==ot->_sgs_eofch) { err= E_EOF;    break; }
        
          //#ifdef MPW /* for the terminal consoles it is supported now */
            if (c==ot->_sgs_kbich) { err= S_Intrpt; break; };
            if (c==ot->_sgs_kbach) { err= S_Abort;  break; };
          //#endif

            /* xterm kludge: xterm consoles can only generate an escape
               sequence (default), 0x08 or 0x7f if the Delete key is hit.
               This kludge assumes that 0x7f has been configured for
               Delete and translates it to the OS-9 default.
            */
          /* 
          #ifdef UNIX
            if (c == 0x7f)
                c = 0x18;
          #endif
          */
          
            if (ot->_sgs_case && islower(c)) {
                /* lower case -> upper case */
                c = toupper(c);
            }
            
            if (edit && c==ot->_sgs_dulnch) {
                dupMode= true; /* duplicate last line */
            }

        } /* if not NUL char */
        
        if (ot->_sgs_pause && term_line >= ot->_sgs_page) {
            /* pause is ON and a screenful has been output,
               so the last character read means "go on".
               Reset the line count and swallow the character */
            term_line = 0;
        }
        else {
            if (dupMode) { 
                c=  *(buffer+cnt); 
                if (*(buffer+cnt+1)==NUL) dupMode= false;
            }
            else {
                *(buffer+cnt)= c;
            }
        

            if (edit) {
                /* basic line editing  */
                if (c!=NUL && c==ot->_sgs_eofch) {
                    ConsPutcEdit( CR, alf,ot->_sgs_eorch );
                    err= E_EOF;
                    break;
                }
                else
                if (c!=NUL && c==ot->_sgs_bspch) {
                    /* backspace */
                    if (cnt>0) {
                        cnt--;
                        if (ot->_sgs_echo) {
                            /* backspace echo */
                            ConsPutc(ot->_sgs_bsech);
                            if (ot->_sgs_backsp) {
                                /* BSP-Space-BSP Sequence wanted */
                                ConsPutc(' ');
                                ConsPutc(ot->_sgs_bsech);
                            }
                        }
                    }
                }
                else
                if (c!=NUL && c==ot->_sgs_dlnch) {
                    /* clear line */
                    if (ot->_sgs_echo) {
                        while (cnt>0) {
                            ConsPutc(ot->_sgs_bsech);
                            if (ot->_sgs_backsp) {
                                /* BSP-Space-BSP Sequence wanted */
                                ConsPutc(' ');
                                ConsPutc(ot->_sgs_bsech);
                            }
                            cnt--;                  
                        }
                    }
                    cnt=0; /* clear anyway */
                }
                else {
                    cnt++;
                    if (ot->_sgs_echo) ConsPutcEdit( c, alf,ot->_sgs_eorch );
                }
            }
            else {
                /* without Editing */
                cnt++;
                if (ot->_sgs_echo) ConsPutc( c );
            }
            
            fflush(stdout); /* ensure all is written out */
            if (endchar!=0 && endchar==c) break;
        }
    }
    
    *maxlenP = cnt;

    rw__idleticks+= GetSystemTick()-inputticks;
    return os9error(err);   
} /* ConsRead */
#endif

/* returns index for numbered descriptors like tty00,01,02... */
static Boolean ConsId( char* name, char* family, int range, int offs, int *result )
{
    int   flen= strlen(family);
    char* nInd;
    int   ii;

    if (ustrncmp( name,family, flen )!=0) return false; /* family name correct ? */

    nInd= &name[flen]; if (strcmp(nInd,"00")==0) { *result= offs; return true; }
    ii  = atoi(nInd);                          
    if (ii>0 && ii<range) { *result= ii+offs; return true; }

    return false;
} /* ConsId */

os9err pCopen( ushort pid, syspath_typ* spP, _modeP_, char* name )
/* routine for opening serial devices */
{
    int    id;

    while (true) {
        #ifdef TERMINAL_CONSOLE /* decide which terminal id has to be taken */
          if (ConsId( name,"/t",  MAX_CONSOLE,    0, &id )) break; /* Multi-Strout /t1   ../t99   */
        #endif
        
        if (ConsId( name,"/tty",MAXTTYDEV, TTY_Base, &id )) break; /* ttys for MGR /tty00../tty99 */

        if (ustrcmp(name,MainConsole)==0) { id=    Main_ID; break; } /* /term */
        if (ustrcmp(name,SerialLineA)==0) { id= SerialA_ID; break; } /* /ts1  */
        if (ustrcmp(name,SerialLineB)==0) { id= SerialB_ID; break; } /* /ts2  */

        break;
    } /* end exit part */

    spP->term_id= id;
    strcpy( spP->name,&name[1] );

    /* for tty/pty pairs with the same name, the same pipe must be used */
    #ifdef PIP_SUPPORT
      if (spP->type==fTTY) {
          ConnectPTY_TTY( pid,spP );
          InstallTTY    ( spP,id );
      }
    #endif

    /* get the initialised path option table */
    pSCFopt( pid,spP, (byte*)&spP->opt ); /* no err returned */
    
    debugprintf( dbgTerminal,dbgDetail,("# pCopen (%s): successful, pid=%d\n",
                                           name, pid ));
    return 0;
} /* pCOpen */

os9err pSopen( _pid_, syspath_typ* spP, _modeP_, char* name )
/* routine for opening SCF devices */
{   
    int k;
    os9err reply = 0;
    if (*name != NUL) {
       	strcpy(spP->name, &name[1]);
    	k = link_mod_id( spP->name);
    	if (k!=MAXMODULES)  spP->mh = os9mod( k );
    } /* if */

    return reply;
} /* pSopen */

os9err pSBlink( _pid_, _spP_, uint32_t *d2 )
/* specific "/L2" blink command, as defined in "led_Drv" */
{
     byte*   bb= (byte  *)FROM68K(*d2);
     ushort* ww= (ushort*)FROM68K(*d2);
        	
     l2.col1  =           *(bb+0); /* assign values as done in the "led_drv" */
     l2.ratio1= os9_word( *(ww+1) );
     l2.col2  =           *(bb+4);
     l2.ratio2= os9_word( *(ww+3) );
     return 0;
} /* pSBlink */

os9err pGBlink( _pid_, _spP_, uint32_t *d2 )
/* specific "/L2" blink command, as defined in "led_Drv" */
{
     byte*   bb= (byte  *)FROM68K(*d2);
     ushort* ww= (ushort*)FROM68K(*d2);
        	
     *(bb+0)=          l2.col1; /* assign values as done in the "led_drv" */
     *(ww+1)= os9_word(l2.ratio1);
     *(bb+4)=          l2.col2;
     *(ww+3)= os9_word(l2.ratio2);
     return 0;
} /* pGBlink */

os9err pCclose( ushort pid, syspath_typ* spP )
{
    int           k, pb;
    syspath_typ   *spK, *spB;
    pipechan_typ* p= spP->u.pipe.pchP;
    pipechan_typ* b;

    g_spP     = spP;
    gConsoleID= spP->term_id;

    if (spP->type!=fTTY) {
        #ifdef MACTERMINAL
          /* now it can really be de-initialized */
          RemoveConsole();
        #endif
          
        return 0;
    } /* if */
    
                    if (p==NULL) return 0; /* no pipe available => return */          
    pb= p->sp_lock; if (pb==0)   return 0;

       spB= get_syspath( pid,pb );  /* take the according PTY */
    b= spB->u.pipe.pchP;

    /* paths shouldn't be closed if same console is still open */
    /* searching if there is a console with the same name */
    for (k=0; k<MAXSYSPATHS; k++) {
            spK= get_syspath( pid,k ); 
        if (spK!=NULL                     && 
            spK->linkcount>0              && /* still open by another path */
            spK->type==fTTY               &&
            spK->u.pipe.pchP!=NULL        &&
            spK->u.pipe.pchP->sp_lock==pb && b!=NULL) { b->sp_lock= spK->nr; return 0; }
    } /* for */

    /* syspath no longer in use, disconnect the pipe */
    if (b!=NULL) {
        b->broken= true;
        b->sp_lock= spB->nr;
        
        #ifdef PIP_SUPPORT
          releasePipe( pid, spP );
        #endif
    }
    
    RemoveTTY( gConsoleID ); 
    return 0;
} /* pCclose */

/* Close an SCF device, resetting the terminal modes to pre-open values */
os9err pSclose( _pid_, _spP_ )
{
  return 0;
} /* pSclose */

/* input character wise from console */
os9err pConsIn( ushort pid, syspath_typ* spP, uint32_t *maxlenP, char* buffer )
{
    #ifdef TERMINAL_CONSOLE
      gConsoleID= spP->term_id;
      g_spP     = spP;
      return ConsRead( pid,spP, maxlenP,buffer,false, 0);
    
    #else
      return pUnimp  ( pid,spP );
    #endif
} /* pConsIn */

os9err pConsInLn( ushort pid, syspath_typ* spP, uint32_t *maxlenP, char* buffer )
/* input line from console */
{
	os9err err= 0;
	
    #ifdef TERMINAL_CONSOLE
      gConsoleID= spP->term_id;
      g_spP     = spP;
      err= ConsRead( pid,spP,maxlenP,buffer,true,CR );
    
    #else
      long cnt,i;
      #define RDBUFLEN 500
      static char readbuf[RDBUFLEN];
      int cerr;
      ulong inputticks= GetSystemTick();

      /* interactive input line from stdin */
      cnt=*maxlenP; /* max number of chars to get */
      interactivepid=pid; /* set focus to this process */
      stdwrite(pid,"\n",1,stdout,false); /* begin input on a free line */
      fflush(stdout); /* ensure all is written out */
      clearerr(stdin); /* make sure we are not stuck with a Cmd-. */

      if (fgets(readbuf,RDBUFLEN,spP->stream)==NULL) {
          cerr= errno;
          clearerr(stdin);
          debugprintf(dbgTerminal,dbgNorm,("# pConsInLn: fgets returned NULL, errno=%d\n",cerr));   

          if      (cerr==22)    return          S_Abort;
          else if (feof(stdin)) return os9error(E_EOF);
          return                       os9error(E_READ);
      }

      i= strlen(readbuf);
      if (i>0) if (*(readbuf+i-1) =='\n') i--; /* forget newline if there is one */
      i=i<cnt ? i : cnt-1;
      *(readbuf+i++)='\n'; /* at end of string returned, there will be a CR anyway */

      debugprintf( dbgTerminal,dbgNorm,("# pConsIn len=%d, string='%s'\n",i,readbuf ));
      debug_halt ( dbgTerminal );
      strncpy( buffer,readbuf,i );
      *maxlenP=i;

      rw__idleticks+= GetSystemTick()-inputticks;
    #endif
    
    arbitrate= true;
    return err;
} /* pConsInLn */

os9err pEOF( _pid_, _spP_, _maxlenP_, _buffer_ )
/* read operation for the nil device is alway EOF */
{   return E_EOF;
} /* pEOF */

static os9err ConsoleOut( ushort pid, syspath_typ* spP,
                          uint32_t *maxlenP, char* buffer, Boolean wrln )
/* output to console */
{
    uint32_t     cnt;
    char         c;
    ulong        outputticks= GetSystemTick();
    syspath_typ* spC=  spP;          /* default: no crossed path */
    struct _sgs* ot = (struct _sgs*)&spC->opt; /* path opt table */
    Boolean      do_lf= false;
    process_typ* cp= &procs[pid];

    gConsoleID=  spP->term_id;
    g_spP     =  spP;

    /* go directly if tty */
    if (gConsoleID>=TTY_Base) {
        if (wrln) {
                   cnt= 0; /* search if there is any CR */
            while (cnt<*maxlenP) {
                    c= buffer[cnt++];
                if (c!=NUL && c==ot->_sgs_eorch) {
                    *maxlenP= cnt;
                    do_lf= ot->_sgs_alf;
                    break;
                }
            } /* while */
        } /* if */

        cnt=  WriteCharsToPTY( buffer,*maxlenP, gConsoleID, do_lf );
        if (cp->state==pSysTask) cnt= *maxlenP;
    }
    else {
        /* interactive output to console */
        #ifdef TERMINAL_CONSOLE
                 cnt=0;
          while (cnt<*maxlenP) {
              c= buffer[cnt++];
              if (ot->_sgs_case && islower(c)) {
                  /* lower case -> upper case
                     NOTE: this may wreck alpha escape codes */
                  c = toupper(c);
              }
            
              ConsPutc( c );

              if (cp->state==pSysTask) { /* should never go to here */
                  cp->systask_offs= cnt-1; /* store it here !! */
                  cnt= *maxlenP;
                  break; /* tty/pty break */
              }

              if (c == CR && ot->_sgs_pause) {
                  term_line++;
                  if (term_line >= ot->_sgs_page) {
                      break;
                  }
              }
              
              if (wrln && c!=NUL && c==ot->_sgs_eorch) {
                  if (ot->_sgs_alf) ConsPutc( LF );
                  break;
              }
          } /* while */

        #else   
          cnt= stdwrite(pid,buffer,*maxlenP,spP->stream,false);
        #endif
    }

    rw__idleticks+= GetSystemTick()-outputticks;
    if (cnt<0) return c2os9err(errno,E_WRITE); /* default: general write error */

    *maxlenP= cnt;
    return 0;
} /* ConsoleOut */

os9err pConsOut  ( ushort pid, syspath_typ* spP, uint32_t *maxlenP, char* buffer )
/* output to console */
{ return ConsoleOut( pid,spP, maxlenP, buffer, false );    
} /* pConsOut */

os9err pConsOutLn( ushort pid, syspath_typ* spP, uint32_t *maxlenP, char* buffer)
/* output line to console */
{   
	os9err err= ConsoleOut( pid,spP, maxlenP, buffer, true );
	arbitrate= true; /* create smoother output */
	return err;
} /* pConsOutLn */

os9err pCopt( _pid_, syspath_typ* spP, byte* buffer )
/* get options from console */
{
  memcpy( buffer,&spP->opt, OPTSECTSIZE);
  return 0;
} /* pCopt */

os9err pCsetopt( _pid_, syspath_typ* spP, byte* buffer )
/* set console options */
{
  memcpy( &spP->opt,buffer, OPTSECTSIZE );
  return 0;
} /* pCsetopt */

os9err pCpos( _pid_, _spP_, uint32_t *posP )
{ *posP= 0; return 0;
} /* pCpos */

os9err pCready( _pid_, syspath_typ* spP, uint32_t* n )
/* check ready */
/* NOTE: is valid for outputs also, when using "dup" */
{
    gConsoleID= spP->term_id;
    g_spP     = spP;

    if (gConsoleID>=TTY_Base) {
        if (DevReadyTTY( (long*)n, gConsoleID )) return 0;
    }
    else { 
        #ifdef TERMINAL_CONSOLE
          if  (DevReady( (long*)n ))             return 0;
        #endif
    } // if
    
    return os9error(E_NOTRDY);
} /* pCready */

/* eof */
