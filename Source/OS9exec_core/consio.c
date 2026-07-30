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

/* Forward declaration: pCopen calls this before its own definition (further
   down this file) is reached. Stays static -- both call sites are in this
   file. */
static ulong baud_bps( byte code );

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
   
    debugprintf(dbgTerminal,dbgDeep,("# stdwrite cnt=%d, from=%p\n",cnt,p));
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
  /* Put one character to a NAMED console. Split out from ConsPutc because the
     baud FIFO drains from the scheduler, where there is no "current" console
     to inherit -- see baud_drain_due. Every other caller legitimately runs
     inside an fmgr entry point that has just set gConsoleID from its path, and
     keeps using the ConsPutc wrapper below.
     Not declared in consio.h: ConsPutc's own declaration lives in
     filestuff.h, unconditionally (no TERMINAL_CONSOLE guard), not in
     consio.h -- so there is no matching spot to add ConsPutcTo to. Both of
     ConsPutcTo's only callers (ConsPutc and baud_drain_due) are in this
     file, so file-static is correct as well as consistent with that. */
  static void ConsPutcTo( int term_id, char c )
  {
      /* save this info in terminal interface system.
         0 = "nobody": emulator banner output is written while currentpid is the
         MAXPROCESSES "no process" sentinel, and that must not become a Ctrl-C
         signal target (KeyToBuffer) nor a procs[] index (lw_pid). */
      gLastwritten_pid= proc_slot( currentpid );

      if (term_id>=TTY_Base) {
          #ifdef PIP_SUPPORT
            WriteCharsToPTY( &c,1, term_id, false );
          #endif
          return;
      }

      if (hostterm_bound( term_id )) {
          hostterm_put( term_id, &c,1 );
          return;
      }

      write( 1,&c,1 );

      // not yet supported for Mac Classic/Carbon
      #ifdef win_unix
        lw_pid( &main_mco ); /* assign for later use */
      #endif
  } /* ConsPutcTo */

  /* put char to console and perform CR/LF expansion etc. */
  void ConsPutc( char c )
  {
      ConsPutcTo( gConsoleID, c );
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

    if (hostterm_bound( gConsoleID )) {
        /* Returns before the LF<->CR swap below on purpose. That swap exists
           for a genuine Unix terminal in cooked mode; a host endpoint is raw
           and 8-bit transparent, and rewriting CR would corrupt every binary
           transfer -- which is the entire point of this device. The existing
           PTY branch above returns early for the same reason. */
        n= hostterm_get( gConsoleID, c );
        devIsReady= n>0;
        return devIsReady;
    }

    #if defined windows32 || defined MINGW
      /* MINGW's HandleEvent() reads via ReadConsoleInput (telnetaccess.c),
       * same as native windows32 -- Enter arrives as a raw CR (0x0D)
       * already, with no tty-driver translation involved. Must return
       * early like windows32 does, skipping the CR/LF swap below: that
       * swap exists for genuine Unix terminals, where ICRNL turns Enter's
       * CR into LF before read() ever sees it. Falling through here would
       * flip the already-correct CR into LF, so ConsRead's endchar==CR
       * check (consio.c) never matches and the shell never sees a
       * completed line.
       */
      HandleEvent();
      n= ReadCharsFromTerminal( c,1, &main_mco );
      return (n>0) && devIsReady;

    #elif defined UNIX
      /* Shares windows32's plumbing above (HandleEvent() drains stdin into
       * main_mco.inBuf, skimming off Ctrl-C/Ctrl-E immediately as it goes —
       * see telnetaccess.c — and we consume from there instead of reading
       * stdin directly here, which would race with HandleEvent()'s own
       * read() of the same fd). Unlike windows32/MINGW's ReadConsoleInput,
       * raw Unix input arrives as LF, not CR, so this falls through to the
       * CR/LF swap below instead of returning early.
       */
      HandleEvent();
      n= ReadCharsFromTerminal( c,1, &main_mco );
      if (n<=0 || !devIsReady) return false;

    #elif defined MACOS9
          n= fread( c,1,1, stdin );
      if (n!=1 || !devIsReady) return false;

    #else
      n= fread( c,1,1, stdin );

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
          
            if (ot->_sgs_case && islower((unsigned char)c)) {
                /* lower case -> upper case */
                c = toupper((unsigned char)c);
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
            else if (!(edit && c!=NUL && c==ot->_sgs_bspch)) {
                /* backspace is handled below by shortening the line in
                   place; storing its raw byte here first would leave it
                   as stray data one slot past the new, shorter length */
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
                        *(buffer+cnt)= NUL; /* re-terminate at the shorter length */
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
    /* `id` is left deliberately uninitialised: every path that reaches the use
     * below now assigns it, because the no-match case returns an error instead
     * of falling through (see the end of the matcher block). Keeping it
     * uninitialised means the compiler/analyzers stay able to catch a FUTURE
     * branch that forgets to set it -- initialising it to a plausible default
     * would silence that warning forever and hide the next such bug.
     * Found by clang scan-build ("Assigned value is garbage or undefined");
     * GCC -fanalyzer did NOT report it. */
    int    id;

    while (true) {
        #ifdef TERMINAL_CONSOLE /* decide which terminal id has to be taken */
          if (ConsId( name,"/t",  MAX_CONSOLE,    0, &id )) break; /* Multi-Strout /t1   ../t99   */
        #endif
        
        if (ConsId( name,"/tty",MAXTTYDEV, TTY_Base, &id )) break; /* ttys for MGR /tty00../tty99 */

        if (ustrcmp(name,MainConsole)==0) { id=    Main_ID; break; } /* /term */
        if (ustrcmp(name,SerialLineA)==0) { id= SerialA_ID; break; } /* /ts1  */
        if (ustrcmp(name,SerialLineB)==0) { id= SerialB_ID; break; } /* /ts2  */

        /* Nothing matched -- REFUSE, rather than fall through with whatever
         * `id` happens to hold. This path is reachable: "/t999" matches the
         * "/t" family but fails ConsId's range check, so ConsId returns false
         * without writing *result. Defaulting to a console id here would make
         * an out-of-range or unknown device silently open the MAIN console and
         * report success, which is worse than the uninitialised read it
         * replaced -- pCopen returns os9err precisely so it can say no.
         * E_UNIT is the OS-9 error for a bad unit number. */
        return os9error(E_UNIT);
    } /* end exit part */

    spP->term_id= id;
    strcpy( spP->name,&name[1] );

    /* A /tN in the host-backed range is only a device if OS9T<n> names a
       WORKING endpoint. Unconfigured, or naming something the host refuses,
       it used to fall through and share the MAIN console's stdin/stdout --
       so `echo x >/t1` interleaved its bytes into the shell's own prompt,
       and `tsmon /t1` failed with a misleading E_NOTRDY much later instead
       of an honest refusal here. hostterm_open already returns E_UNIT when
       unconfigured, so this single call also covers the earlier gate. */
    if (hostterm_in_range( id )) {
        os9err herr= hostterm_open( id, spP );
        if    (herr) return herr;
    }

    /* for tty/pty pairs with the same name, the same pipe must be used */
    #ifdef PIP_SUPPORT
      if (spP->type==fTTY) {
          ConnectPTY_TTY( pid,spP );
          InstallTTY    ( spP,id );
      }
    #endif

    /* get the initialised path option table */
    pSCFopt( pid,spP, (byte*)&spP->opt ); /* no err returned */

    /* Not inside hostterm_open: that runs before pSCFopt above, so the option
       table -- and _sgs_bau with it -- is not populated yet there. */
    if (hostterm_bound( id )) {
        struct _sgs* ot= (struct _sgs*)&spP->opt;
        hostterm_setspeed( id, baud_bps( ot->_sgs_bau ) );
    }

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

     if (!RANGE_IN_ARENA(bb,8)) return os9error(E_BPADDR); /* the /L2 struct spans bb+0..bb+6 */
     /* The 16-bit fields go through GET_OS9W, not a ushort* aimed into the
        struct: d2 is the guest's pointer and may be ODD, so the wide load was
        undefined and faults on a strict-alignment host. GET_OS9W copies the
        bytes and byte-swaps, which is what os9_word() was doing separately. */
     l2.col1  =           *(bb+0); /* assign values as done in the "led_drv" */
     l2.ratio1= GET_OS9W(  bb,2 );
     l2.col2  =           *(bb+4);
     l2.ratio2= GET_OS9W(  bb,6 );
     return 0;
} /* pSBlink */

os9err pGBlink( _pid_, _spP_, uint32_t *d2 )
/* specific "/L2" blink command, as defined in "led_Drv" */
{
     byte*   bb= (byte  *)FROM68K(*d2);

     if (!RANGE_IN_ARENA(bb,8)) return os9error(E_BPADDR); /* the /L2 struct spans bb+0..bb+6 */
     /* See pSBlink: SET_OS9W rather than a store through a ushort* into a
        guest-supplied (possibly odd) address. */
     *(bb+0)=          l2.col1; /* assign values as done in the "led_drv" */
     SET_OS9W( bb,2,   l2.ratio1 );
     *(bb+4)=          l2.col2;
     SET_OS9W( bb,6,   l2.ratio2 );
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

    if (hostterm_in_range( spP->term_id )) hostterm_close( spP->term_id );

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

#define BAUD_FIFO_SIZE   256
#define MAXBAUDDEV         8

typedef struct {
    Boolean inUse;
    short   term_id;
    byte    buf[BAUD_FIFO_SIZE];
    ushort  head, tail, count;
    ulong   us_per_char;   /* 0 = unpaced; Task 3 fills this in for real baud rates */
    ulong   next_due_us;   /* host time next pop may happen; Task 3 makes this meaningful */
} baud_device_t;

static baud_device_t baud_devices[MAXBAUDDEV];

static ulong g_next_wake_us= 0; /* earliest next_due_us across all paced non-empty devices;
                                    0 is the sentinel for "nothing pending" -- relies on
                                    gettimeofday() never legitimately returning exactly
                                    epoch microsecond 0, which is true on any real system */

static ulong host_micros( void )
{
    struct timeval tv;
    gettimeofday( &tv, NULL );
    return (ulong)tv.tv_sec*1000000UL + (ulong)tv.tv_usec;
} /* host_micros */

static void recompute_next_wake( void )
{
    int   i;
    ulong earliest= 0;
    for (i=0; i<MAXBAUDDEV; i++) {
        baud_device_t* d= &baud_devices[i];
        if (d->inUse && d->count>0 && d->us_per_char>0) {
            if (earliest==0 || d->next_due_us<earliest) earliest= d->next_due_us;
        }
    }
    g_next_wake_us= earliest;
} /* recompute_next_wake */

/* find (or allocate) the simulated device for a physical console id */
static baud_device_t* baud_dev_for( short term_id )
{
    int i, free_slot= -1;
    for (i=0; i<MAXBAUDDEV; i++) {
        if ( baud_devices[i].inUse && baud_devices[i].term_id==term_id) return &baud_devices[i];
        if (!baud_devices[i].inUse && free_slot<0) free_slot= i;
    }
    if (free_slot<0) return NULL; /* out of device slots; caller falls back to unpaced */

    baud_devices[free_slot].inUse=       true;
    baud_devices[free_slot].term_id=     term_id;
    baud_devices[free_slot].head=
    baud_devices[free_slot].tail=
    baud_devices[free_slot].count=       0;
    baud_devices[free_slot].us_per_char= 0;
    baud_devices[free_slot].next_due_us= 0;
    return &baud_devices[free_slot];
} /* baud_dev_for */

static Boolean fifo_push( baud_device_t* d, byte c )
{
    if (d->count>=BAUD_FIFO_SIZE) return false;
    if (d->count==0 && d->us_per_char>0) {
        d->next_due_us= host_micros(); /* first queued char of a burst is due immediately */
    }
    d->buf[d->tail]= c;
    d->tail= (ushort)((d->tail+1) % BAUD_FIFO_SIZE);
    d->count++;
    if (d->us_per_char>0) recompute_next_wake();
    return true;
} /* fifo_push */

static Boolean fifo_pop( baud_device_t* d, byte* c )
{
    if (d->count==0) return false;
    *c= d->buf[d->head];
    d->head= (ushort)((d->head+1) % BAUD_FIFO_SIZE);
    d->count--;
    return true;
} /* fifo_pop */

/* pop+display everything currently due, across all devices. Unpaced
   devices (us_per_char==0) always drain in full immediately -- they
   shouldn't normally accumulate a backlog, but drain fully if they ever do. */
void baud_drain_due( void )
{
    int   i;
    byte  c;
    ulong now;

    for (i=0; i<MAXBAUDDEV; i++) {
        baud_device_t* d= &baud_devices[i];
        if (!d->inUse || d->count==0) continue;

        if (d->us_per_char==0) {
            /* Name the device explicitly: this function runs from the
               scheduler, not from an fmgr entry point, so there is no current
               console for ConsPutc to inherit -- it would send every device's
               backlog to whichever console was touched last. Only non-TTY
               devices are ever paced (ConsoleOut routes TTY_Base ids away from
               the FIFO entirely), so WriteCharsToPTY's own g_spP dependence
               cannot be reached from here. */
            while (fifo_pop(d,&c)) ConsPutcTo( d->term_id, c );
        }
    }

    if (g_next_wake_us==0) return;           /* nothing paced is queued anywhere */
    now= host_micros();
    if (now<g_next_wake_us) return;          /* not due yet */

    for (i=0; i<MAXBAUDDEV; i++) {
        baud_device_t* d= &baud_devices[i];
        if (!d->inUse || d->count==0 || d->us_per_char==0) continue;

        while (d->count>0 && d->next_due_us<=now) {
            fifo_pop( d,&c );
            ConsPutcTo( d->term_id, c );
            d->next_due_us += d->us_per_char;
        }
    }
    recompute_next_wake();
} /* baud_drain_due */

ulong baud_next_wake_delay_us( void )
{
    ulong now;
    if (g_next_wake_us==0) return ULONG_MAX; /* nothing pending: no deadline */
    now= host_micros();
    if (now>=g_next_wake_us) return 0;       /* already due */
    return g_next_wake_us-now;
} /* baud_next_wake_delay_us */

void baud_flush_device( short term_id )
{
    int i;
    for (i=0; i<MAXBAUDDEV; i++) {
        if (baud_devices[i].inUse && baud_devices[i].term_id==term_id) {
            baud_devices[i].head= baud_devices[i].tail= baud_devices[i].count= 0;
            recompute_next_wake(); /* this device may have been g_next_wake_us's source */
            return;
        }
    }
} /* baud_flush_device */

/* Called once, when the emulator is about to shut down (no more runnable
   OS-9 processes). Any console's baud ring buffer may still have queued
   output that real-time pacing hasn't caught up to yet -- block here until
   it's all drained, since there's no more cooperative scheduling to wait
   for and nothing else left running to stay responsive to. */
void baud_drain_all_pending( void )
{
    int     i;
    Boolean anyPending;
    ulong   delay;

    do {
        baud_drain_due();
        anyPending= false;
        for (i=0; i<MAXBAUDDEV; i++) {
            if (baud_devices[i].inUse && baud_devices[i].count>0) { anyPending= true; break; }
        }
        if (anyPending) {
            delay= baud_next_wake_delay_us();
            if (delay>0 && delay!=ULONG_MAX) {
                struct timespec ts;
                ulong capped= (delay>10000UL) ? 10000UL : delay; /* cap each nap at 10ms */
                ts.tv_sec = 0;
                ts.tv_nsec= (long)capped*1000L;
                nanosleep( &ts, NULL );
            }
        }
    } while (anyPending);
} /* baud_drain_all_pending */

/* SCF baud rate code (PD_BAU) -> bits per second.  Codes verified against
   tmode on this build; 0 = unknown/unsupported, meaning "don't throttle". */
static ulong baud_bps( byte code )
{
    static const ulong bps[]= {
          50,   75,  110,  134,  150,  300,  600, 1200, 1800, 2000, /*  0.. 9 */
        2400, 3600, 4800, 7200, 9600,19200,                         /* 10..15 */
       38400,    0,    0,    0,    0,57600,115200                   /* 16..22 */
    };
    return code < sizeof(bps)/sizeof(bps[0]) ? bps[code] : 0;
} /* baud_bps */

static os9err ConsoleOut( ushort pid, syspath_typ* spP,
                          uint32_t *maxlenP, char* buffer, Boolean wrln )
/* output to console */
{
    /* signed, and wide enough for every uint32_t it also receives: stdwrite()
     * returns a SIGNED long and uses a negative value to report a write
     * error, but cnt was unsigned, so the `if (cnt<0)` check below was always
     * false -- console write errors were silently swallowed, and on error
     * *maxlenP was handed back the huge unsigned reinterpretation of -1 as
     * the count of bytes written. Caught by GCC's -Wtype-limits; clang does
     * not diagnose it at any warning level. */
    long         cnt;
    char         c;
    ulong        outputticks= GetSystemTick();
    syspath_typ* spC=  spP;          /* default: no crossed path */
    struct _sgs* ot = (struct _sgs*)&spC->opt; /* path opt table */
    Boolean      do_lf= false;
    process_typ* cp= &procs[pid];
    Boolean      paced= false;
    baud_device_t* dev= NULL;

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
          /* decide once whether this write is paced or goes straight to the
             screen; guards pid==0 (system process) and the pid>=MAXPROCESSES
             sentinel (banner/system output) used elsewhere in this file */
          if (baud_throttle && pid>0 && pid<MAXPROCESSES && cp->state!=pSysTask) {
              ulong bps= baud_bps( ot->_sgs_bau );
              if (bps>0) {
                  dev= baud_dev_for( gConsoleID );
                  if (dev!=NULL) {
                      paced= true;
                      dev->us_per_char= (10UL*1000000UL)/bps; /* microseconds/char, 10 bits/char */
                  }
              }
          }

          cnt= 0;
          if (pid>0 && pid<MAXPROCESSES && cp->state==pWaitWrite) {
              set_os9_state( pid, cp->saved_state, "ConsoleOut" );
              cnt=                cp->saved_cnt;
          }

          while (cnt<*maxlenP) {
              Boolean needsLF; /* does this char carry a trailing auto-LF? */
              int     need;    /* FIFO slots this char needs (2 if CR+LF) */

              c= buffer[cnt];
              if (ot->_sgs_case && islower((unsigned char)c)) {
                  /* lower case -> upper case
                     NOTE: this may wreck alpha escape codes */
                  c = toupper((unsigned char)c);
              }

              /* A CR that gets an auto-LF and its LF must reach the FIFO
               * TOGETHER. The old code pushed the CR, then pushed the LF
               * best-effort and dropped it whenever the FIFO happened to be full
               * -- which under baud pacing it routinely is by the end of a long
               * write. The line was then left un-terminated (a bare CR), so the
               * next thing written -- the shell prompt after `login` -- landed on
               * top of it. Treat CR+LF as one atomic 2-byte unit: require room
               * for both, and if there isn't room, park BEFORE pushing the CR so
               * resume retries the pair (never a lone CR). Unpaced output goes
               * straight to the screen and can't drop anything, exactly as before. */
              needsLF= (wrln && c!=NUL && c==ot->_sgs_eorch && ot->_sgs_alf);
              need   = needsLF ? 2 : 1;

              if (paced) {
                  if (BAUD_FIFO_SIZE - dev->count < need) {
                      cp->saved_cnt  = cnt;
                      cp->saved_state= cp->state;
                      set_os9_state( pid, pWaitWrite, "ConsoleOut" );
                      arbitrate= true;
                      break;
                  }
                                fifo_push( dev, c  );
                  if (needsLF)  fifo_push( dev, LF );
              }
              else if (hostterm_bound( gConsoleID )
                       && pid>0 && pid<MAXPROCESSES && cp->state!=pSysTask) {
                  /* Same protection `paced` above relies on: pid==0 is the
                     system process and pid==MAXPROCESSES is the "no process
                     running" banner sentinel (see ConsPutc's own comment on
                     gLastwritten_pid) -- neither is a real process that can
                     be parked and later rescheduled, so a park here would
                     hang the emulator rather than one OS-9 process. A system-
                     state write instead falls through to the plain ConsPutc
                     arm below, best-effort, same as it was before this change.

                     CR and its auto-LF must reach the endpoint TOGETHER or not
                     at all -- a lone CR leaves an unterminated line and the
                     next output lands on top of it (the 75a8ea8 bug). Build
                     the pair, then write it as one unit. */
                  char pair[2];
                  int  len= 0;
                  int  w;

                  pair[len++]= c;
                  if (needsLF) pair[len++]= LF;

                  w= hostterm_put( gConsoleID, pair,len );

                  /* w<len covers both "would block" (hostterm_put returns 0)
                     and a genuine short write (0<w<len) -- either way, some
                     byte of the pair was not accepted and must not be
                     skipped. A hard error (w<0) parks too rather than
                     dropping: parking is never wrong, only potentially slow,
                     and the resumed retry will surface a persistent error
                     again on its own if the endpoint is truly gone. */
                  if (w<len) {
                      /* Park exactly as the paced branch does and retry from
                         this same character on resume -- never advance cnt
                         past a byte the endpoint did not take. A pWaitWrite
                         process is rescheduled periodically (procstuff.c), so
                         it comes back here and proceeds once the far end
                         drains. */
                      cp->saved_cnt  = cnt;
                      cp->saved_state= cp->state;
                      set_os9_state( pid, pWaitWrite, "ConsoleOut" );
                      arbitrate= true;
                      break;
                  }
              }
              else {
                  /* Plain screen output, and any hostterm-bound write this
                     bulk path cannot park (system-state, see above). ConsPutc
                     still owns the echo path, baud_drain_due and debug.c --
                     all single characters at low volume -- so its own
                     hostterm branch stays best-effort; only THIS bulk path
                     gained backpressure. */
                                ConsPutc( c  );
                  if (needsLF)  ConsPutc( LF );
              }
              cnt++;

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
                  break; /* end of record -- LF (if any) already delivered above */
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
/* On real OS-9, SS_Opt is a device-level operation: options set on any path
   to a terminal propagate to all open paths on the same device.  Propagate
   here so that, e.g., vi turning off _sgs_alf on path 0 also affects path 1. */
{
  int k;
  memcpy( &spP->opt, buffer, OPTSECTSIZE );
  for (k = 0; k < MAXSYSPATHS; k++) {
      syspath_typ* sp = &syspaths[k];
      if (sp != spP && sp->type == spP->type && sp->term_id == spP->term_id)
          memcpy( &sp->opt, buffer, OPTSECTSIZE );
  }

  /* SS_Opt is how `tmode baud=` reaches us, so a live port retunes. */
  if (hostterm_bound( spP->term_id )) {
      struct _sgs* ot= (struct _sgs*)&spP->opt;
      hostterm_setspeed( spP->term_id, baud_bps( ot->_sgs_bau ) );
  }

  return 0;
} /* pCsetopt */

os9err pCpos( _pid_, _spP_, uint32_t *posP )
{ *posP= 0; return 0;
} /* pCpos */

os9err pCready( _pid_, syspath_typ* spP, uint32_t* n )
/* check ready */
/* NOTE: is valid for outputs also, when using "dup" */
{
    long cnt;

    gConsoleID= spP->term_id;
    g_spP     = spP;

    if (gConsoleID>=TTY_Base) {
        if (DevReadyTTY( &cnt, gConsoleID )) {
            *n = (uint32_t)cnt;
            return 0;
        }
    }
    else if (hostterm_bound( gConsoleID )) {
        if (hostterm_ready( gConsoleID, &cnt )) {
            *n = (uint32_t)cnt;
            return 0;
        }
    }
    else {
        #ifdef TERMINAL_CONSOLE
          if  (DevReady( &cnt ))  {
              *n = (uint32_t)cnt;
              return 0;
          }
        #endif
    } // if

    return os9error(E_NOTRDY);
} /* pCready */

/* eof */
