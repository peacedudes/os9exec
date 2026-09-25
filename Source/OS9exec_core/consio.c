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
os9err pCopen    ( ushort pid, syspath_typ*, ushort  *modeP, const char* pathname );
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
os9err pSopen    ( ushort pid, syspath_typ*, ushort  *modeP, const char* pathname );
os9err pSclose   ( ushort pid, syspath_typ* );
os9err pSBlink   ( ushort pid, syspath_typ*, uint32_t   *d2 );
os9err pGBlink   ( ushort pid, syspath_typ*, uint32_t   *d2 );
/* ------------------------------------------------------------------------- */

/* -----------------------  /term pause control  --------------------------- */
/* Page pause (PD_PAU/PD_PAG): "output halts after each full screen until a key
   is pressed", per terminal. It used to be one line count for every terminal,
   and a full page only ended the write early -- output never stopped -- while
   the NEXT character typed, whatever it was for, was swallowed (`echo abc` ran
   as `cho abc`). Now a full page holds the terminal exactly as XOFF does: the
   writer parks and the paced queue stops, and the next key on that terminal
   releases it and is discarded (console_page_release, from KeyToBuffer). */
#define PAGE_TERMS 256
static int     pageLines  [ PAGE_TERMS ]; /* lines written since the terminal last read */
static Boolean pagePending[ PAGE_TERMS ]; /* a full page is waiting for a key */
/* ------------------------------------------------------------------------- */

/* Forward declaration: pCopen calls this before its own definition (further
   down this file) is reached. Stays static -- both call sites are in this
   file. */
static ulong baud_bps( byte code );

/* ConsRead echoes through this, not ConsPutc: see its definition. */
static void echo_putc( ushort pid, syspath_typ* spP, char c );

void init_Cons( fmgr_typ* f )
/* install all procedures of the console file manager */
{
    gs_typ* gs= &f->gs;
    ss_typ* ss= &f->ss;
    
    /* main procedures */
    f->open      = pCopen;
    f->close     = pCclose;
    f->read      = pConsIn;
    f->readln    = pConsInLn;
    f->write     = pConsOut;
    f->writeln   = pConsOutLn;
    /* I$Seek is not a valid request on SCF, and yet it must NOT report one.
       v2.4 Technical I/O Manual, SCF chapter: "The following I/O service
       requests are not valid for SCF: I$ChgDir I$Delete I$MakDir I$Seek",
       and immediately after -- "When an I$ChgDir, I$Delete, or I$MakDir is
       made to SCF, an appropriate error code is returned. I$Seek does not
       return an error." The general statement agrees (I$Seek's own entry):
       managers that "do not support random access usually do nothing during
       the I$Seek operation, and do not return an error". SBF is the stated
       exception that DOES error, and it is not this.
       This was pBadMode (E_BMODE), which matters because ported C calls
       fseek/ftell/rewind on a terminal as a matter of course. */
    f->seek      = pNop_num;      /* no-op, and no error */
    
    /* getstat */
    gs->_SS_Size = pUnimp_num;   /* -- not used */
    gs->_SS_Opt  = pCopt;
    gs->_SS_DevNm= pSCFnam;
    /* SCF handles SS_Opt and passes "all other GetStat calls ... directly to
       the driver" (v2.4 Technical I/O Manual, SCF I$GetStt); SS_Pos is scoped
       "(RBF, PIPE)" and no terminal driver implements it, so the answer is
       E$UnkSvc. This used to return 0 -- a position for a device that has
       none, which a caller cannot tell from a real one. Removing it is a
       deliberate behaviour REMOVAL: ported C calling ftell on a terminal now
       gets the error it would get on real OS-9. Pinned by CONF68K t46. */
    gs->_SS_Pos  = pUnimp_num;
    gs->_SS_EOF  = pNop;         /* ignored */
    gs->_SS_Ready= pCready;

    /* setstat */
    ss->_SS_Size = pNop_num;         /* ignored */
    ss->_SS_Opt  = pCsetopt;
    ss->_SS_Attr = pNop_num;         /* ignored */
} /* init_Cons */

void init_NIL( fmgr_typ* f )
{
    gs_typ* gs= &f->gs;
    ss_typ* ss= &f->ss;
    
    /* main procedures */
    f->open      = pNop_path;         /* ignored */
    f->close     = pNop;         /* ignored */
    f->read      = pEOF;      /* as in OS-9 */
    f->readln    = pEOF;      /* as in OS-9 */
    f->write     = pNop_data;         /* ignored */
    f->writeln   = pNop_data;         /* ignored */
    /* I$Seek is not a valid request on SCF, and yet it must NOT report one.
       v2.4 Technical I/O Manual, SCF chapter: "The following I/O service
       requests are not valid for SCF: I$ChgDir I$Delete I$MakDir I$Seek",
       and immediately after -- "When an I$ChgDir, I$Delete, or I$MakDir is
       made to SCF, an appropriate error code is returned. I$Seek does not
       return an error." The general statement agrees (I$Seek's own entry):
       managers that "do not support random access usually do nothing during
       the I$Seek operation, and do not return an error". SBF is the stated
       exception that DOES error, and it is not this.
       This was pBadMode (E_BMODE), which matters because ported C calls
       fseek/ftell/rewind on a terminal as a matter of course. */
    f->seek      = pNop_num;      /* no-op, and no error */

    /* getstat */
    gs->_SS_Size = pUnimp_num;   /* -- not used */
    gs->_SS_Opt  = pSCFopt;
    gs->_SS_DevNm= pSCFnam;
    /* E_BMODE was wrong for both of these, and wrong in a way that misleads:
       it says the PATH WAS OPENED IN THE WRONG ACCESS MODE, when what is
       actually true is that this manager does not implement the code. The
       unimplemented answer is E$UnkSvc -- the same principle CONF68K t32
       already pins from the SetStat side. SS_Pos is scoped to "(RBF, PIPE)"
       in the v2.4 Technical Reference heading, so SCF answering at all is
       out of scope; SS_Ready is "(RBF, SCF, PIPE)" and so IS in scope, but
       neither the manual nor anything else says what a null or virtual-module
       device should report as ready, and E$UnkSvc is the honest answer to a
       code this manager does not handle rather than an invented count.
       Both are pinned by CONF68K t46. */
    gs->_SS_Pos  = pUnimp_num;    /* E$UnkSvc, not E$BMode */
    gs->_SS_EOF  = pNop;         /* ignored */
    gs->_SS_Ready= pNotReady; /* in SCF's scope: E$NotRdy */

    /* setstat */
    ss->_SS_Size = pNop_num;         /* ignored */
    ss->_SS_Opt  = pNop_opt;         /* ignored */
    ss->_SS_Attr = pNop_num;         /* ignored */
} /* init_NIL */

void init_SCF( fmgr_typ* f )
/* currently implemented for support of the /vmod driver only */
{
    gs_typ* gs= &f->gs;
    ss_typ* ss= &f->ss;
    
    /* main procedures */
    f->open       = pSopen;
    f->close      = pSclose;
    f->read       = pBadMode_data; /* not allowed */
    f->readln     = pBadMode_data; /* not allowed */
    f->write      = pBadMode_data; /* not allowed */
    f->writeln    = pBadMode_data; /* not allowed */
    /* I$Seek is not a valid request on SCF, and yet it must NOT report one.
       v2.4 Technical I/O Manual, SCF chapter: "The following I/O service
       requests are not valid for SCF: I$ChgDir I$Delete I$MakDir I$Seek",
       and immediately after -- "When an I$ChgDir, I$Delete, or I$MakDir is
       made to SCF, an appropriate error code is returned. I$Seek does not
       return an error." The general statement agrees (I$Seek's own entry):
       managers that "do not support random access usually do nothing during
       the I$Seek operation, and do not return an error". SBF is the stated
       exception that DOES error, and it is not this.
       This was pBadMode (E_BMODE), which matters because ported C calls
       fseek/ftell/rewind on a terminal as a matter of course. */
    f->seek       = pNop_num;      /* no-op, and no error */


    /* getstat */
    gs->_SS_Size  = pUnimp_num;   /* -- not used */
    gs->_SS_Opt   = pSCFopt;
    gs->_SS_DevNm = pSCFnam;
    /* E_BMODE was wrong for both of these, and wrong in a way that misleads:
       it says the PATH WAS OPENED IN THE WRONG ACCESS MODE, when what is
       actually true is that this manager does not implement the code. The
       unimplemented answer is E$UnkSvc -- the same principle CONF68K t32
       already pins from the SetStat side. SS_Pos is scoped to "(RBF, PIPE)"
       in the v2.4 Technical Reference heading, so SCF answering at all is
       out of scope; SS_Ready is "(RBF, SCF, PIPE)" and so IS in scope, but
       neither the manual nor anything else says what a null or virtual-module
       device should report as ready, and E$UnkSvc is the honest answer to a
       code this manager does not handle rather than an invented count.
       Both are pinned by CONF68K t46. */
    gs->_SS_Pos   = pUnimp_num;   /* E$UnkSvc, not E$BMode */
    gs->_SS_EOF   = pNop;         /* ignored */
    gs->_SS_Ready = pNotReady;/* in SCF's scope: E$NotRdy */
    gs->_SS_LBlink= pGBlink;     /* specific */
    gs->_SS_Undef = pVMod;

    /* setstat */
    ss->_SS_Size  = pNop_num;         /* ignored */
    ss->_SS_Opt   = pNop_opt;         /* ignored */
    ss->_SS_Attr  = pNop_num;         /* ignored */
    ss->_SS_LBlink= pSBlink;     /* specific */
    ss->_SS_Undef = pNop_buf;         /* ignored */
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
     file, so file-static is correct as well as consistent with that.
     Returns false ONLY when a bound /tN answered "would block": the byte was
     not taken, and the caller must keep it rather than move on. Every other
     outcome, a hard write error included, counts as taken -- a device that is
     gone must not wedge the drain that is waiting to hand it bytes. */
  static Boolean ConsPutcTo( int term_id, char c, ushort owner )
  {
      /* save this info in terminal interface system.
         0 = "nobody": emulator banner output is written while currentpid is the
         MAXPROCESSES "no process" sentinel, and that must not become a Ctrl-C
         signal target (KeyToBuffer) nor a procs[] index (lw_pid).
         The owner is passed in for the same reason term_id is: from the FIFO
         drain there is no ambient current process to inherit, and currentpid
         there is whoever holds the CPU, not whoever wrote these bytes. */
      gLastwritten_pid= proc_slot( owner );

      if (term_id>=TTY_Base) {
          #ifdef PIP_SUPPORT
            WriteCharsToPTY( &c,1, term_id, false );
          #endif
          return true;
      }

      if (hostterm_bound( term_id )) {
          if (hostterm_put( term_id, &c,1 )==0) return false; /* would block: not taken */
          hostterm_note_writer( term_id ); /* give this terminal's abort key a target */
          return true;
      }

      /* Retried, not ignored. glibc marks write() warn_unused_result and is
         right to: a console byte dropped on EINTR or a partial write is a
         character the guest printed that simply never appears, with nothing
         anywhere saying so. Only a real glibc Linux flags this -- neither
         macOS nor mingw annotates write() -- which is why it outlived every
         earlier sweep until one ran on Ubuntu. A hard failure means stdout
         itself is gone and there is nowhere left to report it, so it stops
         here deliberately rather than silently. */
      { ssize_t w;
        do { w= write( 1,&c,1 ); } while (w<0 && errno==EINTR);
        (void)w;
      }

      // not yet supported for Mac Classic/Carbon
      #ifdef win_unix
        lw_pid( &main_mco ); /* assign for later use */
      #endif
      return true;
  } /* ConsPutcTo */

  /* put char to console and perform CR/LF expansion etc. */
  void ConsPutc( char c )
  {
      /* currentpid is the right owner HERE and only here: this arm runs inside
         the writer's own fmgr entry point, so the running process is the one
         whose bytes these are. The FIFO drain has no such guarantee, which is
         why it carries the owner along instead. */
      ConsPutcTo( gConsoleID, c, currentpid );
  } /* ConsPutc */

  void ConsPutcEdit( char c, Boolean alf )
  /* put char to console and perform CR/LF expansion etc. */
  {
      /* PD_ALF is keyed on the CARRIAGE RETURN, not on PD_EOR, and the two
       * are only the same character because PD_EOR normally holds $0D.
       * v2.4 Technical I/O Manual, SCF path options: "PD_ALF Automatic line
       * feed -- If PD_ALF is not zero, carriage returns are automatically
       * followed by line-feeds." The Guru says the same thing from the other
       * side: the auto-linefeed "is implemented only if a Carriage Return
       * character is output, not an 'end of record' character", and the EOR
       * character "is echoed without converting it to a Carriage Return".
       *
       * This used to compare against PD_EOR, which is right until something
       * moves PD_EOR: `tmode eor=0A` then cost every echoed line its LF, so
       * the CR returned the cursor to column 0 and the next line was typed
       * over the last one. Terminating a record is PD_EOR's job (ConsRead's
       * endchar, ConsoleOut's wrln break); ending a display line is CR's. */
      ConsPutc ( c );
      if (alf && c==CR) ConsPutc( LF );
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

/* SCF's per-path line-editing buffer, and so the longest line I$ReadLn can
   return from a terminal -- terminator included. 512 since OS-9 v2.3; 256
   before it, which os9exec has no reason to emulate (it reports V4.00 and
   presents a v2.4 system). Citation and the evidence for it: pConsInLn. */
#define SCF_LINEBUF  512

static os9err ConsRead( ushort pid, syspath_typ* spP, uint32_t *maxlenP,
                        char* buffer, Boolean edit, char endchar,
                        Boolean reserveTerm )
/* reserveTerm says the LAST slot of *maxlenP belongs to the end-of-record
   character rather than to data, so data fills only *maxlenP-1. It is set
   when SCF's own line buffer is what bounds the read rather than the caller's
   count, because the two are bounded differently:
     - the CALLER's count binding: "SCF continues to input characters until
       the end-of-record character is received, DISCARDING any characters
       that exceed the number requested" -- the terminator exceeds it too, so
       the count comes back full of data and no terminator. Microware's word.
     - the BUFFER binding: 512 bytes is "the maximum length of a line typed
       in, INCLUDING the [CR]" -- so 511 data and the terminator in the last
       slot. The Guru's word, and the reason this flag exists. */
{
    os9err        err= 0; /* no err so far */
    long          cnt= 0;
    char          c;
    ulong         inputticks= GetSystemTick();
    process_typ*  cp= &procs[pid];
    struct _sgs*  ot= &spP->opt; /* path opt table */
    Boolean       alf= ot->_sgs_alf;
    Boolean       dupMode= false;
    pipechan_typ* k;
    
    interactivepid=pid; /* set focus to this process */
    fflush(stdout);     /* ensure all is written out */
    clearerr(stdin);    /* make sure we are not stuck with a Cmd-. */
    if (spP->term_id>=0 && spP->term_id<PAGE_TERMS) {
        /* a new page -- and a read is not output, so a pause still waiting
           for its key ends here rather than eating the answer typed to it */
        pageLines[ spP->term_id ]= 0;
        if (pagePending[ spP->term_id ]) {
            pagePending[ spP->term_id ]= false;
            console_hold_changed( spP->term_id, false );
        }
    }
    
    if (cp->state==pWaitRead) {
        set_os9_state( pid, cp->saved_state, "ConsRead" );
        cnt=                cp->saved_cnt;
    }

    /* I$Read stops when the requested count is reached; I$ReadLn does not
     * END there -- it TRUNCATES the line at the count, which is what keeps
     * it inside the caller's buffer, and the tail is lost rather than left
     * for a second read.
     * v2.4 Technical I/O Manual, SCF chapter: "If I$ReadLn has satisfied its
     * input byte count, SCF ignores any further input characters until an
     * end-of-record character (PD_EOR) is received. It echoes the PD_OVF
     * character for each byte ignored." The Guru states the same rule as a
     * warning to callers -- "input does not terminate when the requested
     * number of characters has been input" -- so an over-long line is
     * TRUNCATED here, not split into a second read.
     *
     * `full` is derived from cnt every pass rather than latched in a flag, so
     * it needs no extra state to survive the pWaitRead park-and-resume above.
     * Compared as uint32_t because cnt is a (never negative) long and
     * *maxlenP a uint32_t: on LP64 the promotion is to long and on ILP32 to
     * unsigned int, and the cast makes both models agree instead of leaving
     * the signedness to the target. */
    while (true) {
        /* the point at which DATA stops fitting; the terminator may still have
           a slot after it when reserveTerm is set */
        uint32_t datacap= *maxlenP - (reserveTerm ? 1:0);
        Boolean  full= ((uint32_t)cnt >= datacap);

        if (full && !edit) break; /* I$Read: done. I$ReadLn: keep going */

        if (!dupMode && !ConsGetc(&c)) { 
            err= E_READ;
            if (!devIsReady) {
                k=  spP->u.pipe.pchP;
                if (spP->type==fTTY &&
                            k!=NULL && k->broken) {
                    err= E_EOF; break; /* pipe is broken */
                }
                /* A redirected host stdin that has reached EOF (HandleEvent set
                   this): return E$EOF instead of parking forever waiting for
                   input that cannot come. cnt>0 means a final unterminated line
                   was already gathered -- hand it back first, EOF next read,
                   the way a partial last line behaves everywhere else.
                   Only for a terminal that READS host stdin: a pty's tty and a
                   host-bound /tN have sources of their own (ConsGetc), and
                   ending them here logged a telnet user out before a key was
                   typed, whenever os9exec was started from a pipe. */
                if (host_stdin_eof && gConsoleID<TTY_Base && !hostterm_bound( gConsoleID )) {
                    err= (cnt>0) ? 0 : E_EOF; break;
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
            /* PD_EOF ends the read only as the FIRST character, not wherever
               it appears. v2.4 Technical I/O Manual, SCF: the read terminates
               when "an end-of-file (PD_EOF) is detected as the first character
               of the read". The Guru sharpens "first" to first IN THE BUFFER,
               noting that other characters may be input "provided they are
               deleted before the end-of-file character is entered" -- so a
               line typed and then backspaced away is empty again and an EOF
               there still counts. `cnt==0` is exactly that test and satisfies
               both readings; they differ only for a line that was typed and
               erased, which the Guru covers and Microware does not mention.
               Typed anywhere else the character is ordinary data.
               NOTE this is the escape from an I$ReadLn with PD_EOR of zero,
               which the manual says never terminates: at cnt==0 it still
               works, and once bytes are buffered PD_QUT and PD_INT are what
               get you out -- neither is position-gated. */
            if (cnt==0 && c==ot->_sgs_eofch) { err= E_EOF; break; }
        
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
        
        {
            /* Both of these run BEFORE the dup expansion below, because that
               reads buffer[cnt] and buffer[cnt+1] -- one and two past the end
               of the caller's buffer once the count is reached. The old loop
               bound made that unreachable; `full` is now a state the loop
               stays in, so it has to be handled before the read. */
            if (dupMode && full) { dupMode= false; continue; } /* replay hit the count */

            if (full) {
                /* I$ReadLn past its count: the byte is neither stored nor
                   edited. PD_EOR still ends the read (and is discarded with
                   the rest, since there is nowhere to put it); every other
                   byte is answered with PD_OVF, the terminal's bell. Note
                   this is not gated on PD_EKO -- the manual describes PD_OVF
                   as an alert for a byte being thrown away, not as an echo of
                   input, and a path with echo off still overflows. */
                if (endchar!=0 && endchar==c) {
                    /* the reserved slot is what the terminator is for */
                    if (reserveTerm && (uint32_t)cnt < *maxlenP) {
                        *(buffer+cnt)= c;
                        cnt++;
                        if (ot->_sgs_echo) { echo_putc( pid,spP, c ); if (alf && c==CR) echo_putc( pid,spP, LF ); }
                    }
                    break;
                }
                if (ot->_sgs_ovfch)          echo_putc( pid,spP, ot->_sgs_ovfch );
                fflush(stdout);
                continue;
            }

            if (dupMode) {
                /* the last slot has no byte after it to look at: with no slot
                   reserved for the terminator it is the end of the buffer */
                c=  *(buffer+cnt);
                if ((uint32_t)cnt+1 >= *maxlenP || *(buffer+cnt+1)==NUL) dupMode= false;
            }
            else if (!(edit && c!=NUL && c==ot->_sgs_bspch)) {
                /* backspace is handled below by shortening the line in
                   place; storing its raw byte here first would leave it
                   as stray data one slot past the new, shorter length */
                *(buffer+cnt)= c;
            }
        

            if (edit) {
                /* basic line editing. There is no PD_EOF case here: it is
                   handled once, above, and only at cnt==0. This branch used to
                   repeat the test and was unreachable because the early one
                   caught every EOF first; leaving it would have quietly undone
                   the fix, firing for exactly the mid-line EOF that is now
                   supposed to be data. */
                if (c!=NUL && c==ot->_sgs_bspch) {
                    /* backspace */
                    if (cnt>0) {
                        cnt--;
                        *(buffer+cnt)= NUL; /* re-terminate at the shorter length */
                        if (ot->_sgs_echo) {
                            /* backspace echo */
                            echo_putc( pid,spP, ot->_sgs_bsech );
                            if (ot->_sgs_backsp) {
                                /* BSP-Space-BSP Sequence wanted */
                                echo_putc( pid,spP, ' ' );
                                echo_putc( pid,spP, ot->_sgs_bsech );
                            }
                        }
                    }
                }
                else
                if (c!=NUL && c==ot->_sgs_dlnch) {
                    /* clear line */
                    if (ot->_sgs_echo) {
                        while (cnt>0) {
                            echo_putc( pid,spP, ot->_sgs_bsech );
                            if (ot->_sgs_backsp) {
                                /* BSP-Space-BSP Sequence wanted */
                                echo_putc( pid,spP, ' ' );
                                echo_putc( pid,spP, ot->_sgs_bsech );
                            }
                            cnt--;                  
                        }
                    }
                    cnt=0; /* clear anyway */
                }
                else {
                    cnt++;
                    if (ot->_sgs_echo) { echo_putc( pid,spP, c ); if (alf && c==CR) echo_putc( pid,spP, LF ); }
                }
            }
            else {
                /* without Editing */
                cnt++;
                if (ot->_sgs_echo) echo_putc( pid,spP, c );
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
static Boolean ConsId( const char* name, const char* family, int range, int offs, int *result )
{
    int         flen= strlen(family);
    const char* nInd;
    int   ii;

    if (ustrncmp( name,family, flen )!=0) return false; /* family name correct ? */

    nInd= &name[flen]; if (strcmp(nInd,"00")==0) { *result= offs; return true; }
    ii  = atoi(nInd);                          
    if (ii>0 && ii<range) { *result= ii+offs; return true; }

    return false;
} /* ConsId */

os9err pCopen( ushort pid, syspath_typ* spP, _modeP_, const char* name )
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
    syspath_setname( spP,&name[1] );

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
        struct _sgs* ot= &spP->opt;
        hostterm_setspeed( id, baud_bps( ot->_sgs_bau ) );
    }

    debugprintf( dbgTerminal,dbgDetail,("# pCopen (%s): successful, pid=%d\n",
                                           name, pid ));
    return 0;
} /* pCOpen */

os9err pSopen( _pid_, syspath_typ* spP, _modeP_, const char* name )
/* routine for opening SCF devices */
{   
    int k;
    os9err reply = 0;
    if (*name != NUL) {
       	syspath_setname( spP, &name[1] );
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

    if (hostterm_in_range( spP->term_id )) hostterm_close( spP->term_id, spP );

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
      /* I$Read stops at the path's end-of-record character. d1.l is a
       * MAXIMUM, not a count to wait for -- v2.4 Technical I/O Manual, SCF
       * chapter: the read terminates when "the requested number of bytes has
       * been read", when "an end-of-record character is detected (PD_EOR)",
       * on PD_EOF as the first character, or on error. The same page names
       * zeroing PD_EOR as the supported way to ask for the count instead:
       * "De-select (set to zero) the end-of-record (PD_EOR) character ...
       * This prevents the read from terminating early".
       *
       * A literal 0 was passed here, and 0 means "no terminator" to
       * ConsRead, so every terminal read behaved as if PD_EOR were disabled
       * and waited for the full count: a program asking for more bytes than
       * were typed hung for ever.
       *
       * The freeware `ksh` is the program that found it, and it is worth
       * naming the RIGHT one. `-d 2` against the freeware disk: ksh reads its
       * command line with one `I$Read` of $100 = 256 bytes on its own dup of
       * stdin -- read(ttyfd,line,256) -- so with no terminator it waited for
       * 256 bytes that were never coming. Measured both ways: on the
       * pre-change build the shell never returns and the run has to be killed;
       * with PD_EOR honoured the same read comes back at the CR.
       *
       * Do NOT re-test this with /dd/CMDS/SHARE/sh on a Microware disk and
       * conclude the story is wrong -- that is a DIFFERENT pdksh build, it
       * reads the console ONE BYTE AT A TIME (`I$Read D1.l=$1`), and a
       * one-byte read is satisfied by its count no matter what the terminator
       * is. It runs fine on the broken build, which is exactly why the defect
       * survived a release. That mistake was made here once already.
       *
       * The other half of the contract, also live: pdksh and umacs zero
       * PD_EKO and PD_EOF on entry and deliberately LEAVE PD_EOR at $0D,
       * while tsmon zeroes PD_EOR and then reads a single byte.
       */
      struct _sgs* ot= &spP->opt; /* path opt table */

      gConsoleID= spP->term_id;
      g_spP     = spP;
      return ConsRead( pid,spP, maxlenP,buffer,false, ot->_sgs_eorch, false );

    #else
      return pUnimp  ( pid,spP );
    #endif
} /* pConsIn */

os9err pConsInLn( ushort pid, syspath_typ* spP, uint32_t *maxlenP, char* buffer )
/* input line from console */
{
	os9err err= 0;
	
    #ifdef TERMINAL_CONSOLE
      /* I$ReadLn ends the line at the path's end-of-record character, not at
       * a carriage return. v2.4 Technical I/O Manual, SCF chapter: the read
       * terminates when "an end-of-record character is detected (PD_EOR)",
       * on PD_EOF as the first character, or on error -- and, pointedly, not
       * on the byte count (ConsRead handles that half). The Guru puts the
       * same sentence the other way round: SCF "terminates when the
       * character read matches the 'end of record' character (PD_EOR),
       * rather than the Carriage Return character".
       *
       * A literal CR was passed here while the echo beside it already read
       * PD_EOR, so the two disagreed the moment anything moved PD_EOR: the
       * line still ended on CR but lost its auto-LF. `tmode eor=<h>` is a
       * documented v2.4 parameter and reaches this path, and tsmon zeroes
       * PD_EOR outright on /term at boot, so this is not hypothetical.
       *
       * PD_EOR of zero is passed straight through, which the manual is
       * explicit about and warns callers of: "If PD_EOR is set to zero,
       * SCF's I$ReadLn will never terminate, unless an EOF or error occurs."
       * PD_EOF (ESC by default) and PD_QUT still end the read, so a path put
       * in that state is recoverable rather than wedged.
       */
      struct _sgs* ot= &spP->opt; /* path opt table */
      Boolean      reserveTerm;

      /* SCF edits an input line in a buffer it allocates at I$Open, and that
       * buffer is the hard ceiling on a line however much the caller asks
       * for. `Guru` (Galactic Industrial, "The OS-9 Guru -- The Facts"),
       * which says it three times and consistently: SCF "allocates a buffer
       * of 512 bytes (256 bytes prior to OS-9 version 2.3) for input line
       * editing"; "the buffer is 512 bytes, so this is the maximum length of
       * a line typed in, including the [CR]"; input "is restricted to 512
       * bytes ... including the 'end of record' character". A separate buffer
       * per open path, so one path's line editing cannot affect another's.
       *
       * NOT Microware's word: no statement of a line-length limit appears in
       * the v2.4 Technical I/O Manual or the Technical Reference -- every
       * "input buffer" there is the DRIVER's hardware buffer, in the
       * flow-control sections. Two pieces of circumstantial support from
       * Microware's own binaries, measured by `-d 2` trace on a v2.4 disk:
       * the shell asks for exactly $200 (512) on a console I$ReadLn, and
       * BASIC09 for $1FF (511) -- 511 plus its terminator being 512 exactly.
       * Nothing on that disk asks for MORE, which is why the emulator went
       * four major versions without the ceiling mattering, and why the only
       * thing able to exercise it is test/68k-console/rdlnecho.
       */
      /* The buffer bounds the line only when the caller wanted more than it
         can hold. Asking for SCF_LINEBUF or more means the terminator gets
         the last slot; asking for less means the caller's own count binds
         first and the terminator is discarded with the rest of the excess. */
      reserveTerm= (*maxlenP >= SCF_LINEBUF);
      if (*maxlenP>SCF_LINEBUF) *maxlenP= SCF_LINEBUF;

      gConsoleID= spP->term_id;
      g_spP     = spP;
      err= ConsRead( pid,spP,maxlenP,buffer,true, ot->_sgs_eorch, reserveTerm );
    
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

static Boolean g_final_drain= false; /* see baud_drain_all_pending */

/* Is output to <term_id> currently halted by an XOFF typed on that terminal?
   "Output from a SCF device is halted immediately when PD_XOFF is received and
   will not be resumed until PD_XON is received" (Technical I/O Manual V2.4,
   PD_XOFF) -- per DEVICE, so a hold on /term must not touch /t1 or a TTY.
   KeyToBuffer (utilstuff.c) consumes the two characters and owns the flag; this
   only reads it, from whichever ttydev_typ backs the id:
     >= TTY_Base    a pty -- WriteCharsToPTY does its own hold, so false here
     bound /tN      that device's own flag, via hostterm.c (hostterms[] is private)
     anything else  the main console, including an unbound /tN whose output
                    falls back to it. */
static Boolean console_held( short term_id )
{
    if (g_final_drain)             return false;
    if (term_id>=0 && term_id<PAGE_TERMS && pagePending[ term_id ]) return true; /* page pause */
    if (term_id>=TTY_Base)         return false;
    if (hostterm_bound( term_id )) return hostterm_held( term_id );

    #ifdef win_unix
      return main_mco.holdScreen;
    #else
      return false;
    #endif
} /* console_held */

/* Room for the largest unit ConsoleOut queues whole -- a CR, its auto-LF and
   PD_NUL's 255 pad bytes, 257 -- with room to spare. At 256, a paced write
   with PD_NUL 255 waited for room that could never exist (found by the
   pre-release review). */
#define BAUD_FIFO_SIZE   512
typedef char baud_fifo_holds_a_whole_unit[ (BAUD_FIFO_SIZE >= 2+255) ? 1 : -1 ];
#define MAXBAUDDEV         8

typedef struct {
    Boolean inUse;
    short   term_id;
    byte    buf[BAUD_FIFO_SIZE];
    /* Who queued each byte. Ctrl-C/Ctrl-E target the process that wrote what is
       ON the screen (KeyToBuffer -> lastwritten_pid), but a paced byte reaches
       the screen long after its writer stopped running, and the drain happens in
       the scheduler where "currentpid" is simply whoever holds the CPU. Reading
       it there aimed the abort at an innocent bystander. Per BYTE, not per
       device: two processes writing one terminal genuinely interleave here. */
    ushort  owner[BAUD_FIFO_SIZE];
    ushort  head, tail, count;
    uint64_t us_per_char;  /* 0 = unpaced; Task 3 fills this in for real baud rates */
    uint64_t next_due_us;  /* host time next pop may happen; Task 3 makes this meaningful */
} baud_device_t;

static baud_device_t baud_devices[MAXBAUDDEV];

static uint64_t g_next_wake_us= 0; /* earliest next_due_us across all paced non-empty devices;
                                    0 is the sentinel for "nothing pending" -- relies on
                                    gettimeofday() never legitimately returning exactly
                                    epoch microsecond 0, which is true on any real system */

/* 64 bits on every host. ulong is 32 bits on i386, 32-bit Windows and the
   browser build, where microseconds since the epoch wrapped every 71.6 minutes:
   a deadline set just before the wrap then lay an hour in the future, and
   paced output stopped dead for that long (found by the pre-release review). */
static uint64_t host_micros( void )
{
    struct timeval tv;
    gettimeofday( &tv, NULL );
    return (uint64_t)tv.tv_sec*1000000U + (uint64_t)tv.tv_usec;
} /* host_micros */

static void recompute_next_wake( void )
{
    int      i;
    uint64_t earliest= 0;
    for (i=0; i<MAXBAUDDEV; i++) {
        baud_device_t* d= &baud_devices[i];
        /* A held device contributes no deadline. It must not: its next_due_us
           is already in the past, so leaving it in would pin g_next_wake_us to
           "due now" for the whole hold and turn DoWait()'s idle nap into a
           zero-timeout spin. console_hold_changed() recomputes on release. */
        if (d->inUse && d->count>0 && d->us_per_char>0 && !console_held(d->term_id)) {
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

static Boolean fifo_push( baud_device_t* d, byte c, ushort owner )
{
    if (d->count>=BAUD_FIFO_SIZE) return false;
    if (d->count==0 && d->us_per_char>0) {
        d->next_due_us= host_micros(); /* first queued char of a burst is due immediately */
    }
    d->buf  [d->tail]= c;
    d->owner[d->tail]= owner;
    d->tail= (ushort)((d->tail+1) % BAUD_FIFO_SIZE);
    d->count++;
    if (d->us_per_char>0) recompute_next_wake();
    return true;
} /* fifo_push */

static Boolean fifo_pop( baud_device_t* d, byte* c, ushort* owner )
{
    if (d->count==0) return false;
    *c    = d->buf  [d->head];
    *owner= d->owner[d->head];
    d->head= (ushort)((d->head+1) % BAUD_FIFO_SIZE);
    d->count--;
    return true;
} /* fifo_pop */

/* How long a paced device waits before offering a refused byte again. */
#define BAUD_REFUSED_RETRY_US 10000UL

/* A writer parked on a full queue is retried only every NewAge scheduler
   rounds (do_arbitrate), and every round ends in an idle wait: ~30 ms
   natively, but ~250 ms in Safari, whose timers are slower still -- output
   ran, stopped for a quarter of a second, and ran again. A driver wakes its
   writer when the buffer drains (SCF's low-water mark); this does the same,
   once the queue is half empty. */
static void wake_parked_writers( void )
{
    ushort k;
    for (k=1; k<MAXPROCESSES; k++)
        if (procs[ k ].state==pWaitWrite) procs[ k ].pW_age= 0;
} /* wake_parked_writers */

/* pop+display everything currently due, across all devices. Unpaced
   devices (us_per_char==0) always drain in full immediately -- they
   shouldn't normally accumulate a backlog, but drain fully if they ever do. */
void baud_drain_due( void )
{
    int    i;
    byte   c;
    ushort owner;
    uint64_t now;

    for (i=0; i<MAXBAUDDEV; i++) {
        baud_device_t* d= &baud_devices[i];
        if (!d->inUse || d->count==0) continue;
        if (console_held( d->term_id )) continue; /* XOFF: nothing leaves this device */

        if (d->us_per_char==0) {
            /* Name the device explicitly: this function runs from the
               scheduler, not from an fmgr entry point, so there is no current
               console for ConsPutc to inherit -- it would send every device's
               backlog to whichever console was touched last. Only non-TTY
               devices are ever paced (ConsoleOut routes TTY_Base ids away from
               the FIFO entirely), so WriteCharsToPTY's own g_spP dependence
               cannot be reached from here. */
            while (d->count>0 &&
                   ConsPutcTo( d->term_id, d->buf[d->head], d->owner[d->head] ))
                fifo_pop( d,&c,&owner );
        }
    }

    if (g_next_wake_us==0) return;           /* nothing paced is queued anywhere */
    now= host_micros();
    if (now<g_next_wake_us) return;          /* not due yet */

    for (i=0; i<MAXBAUDDEV; i++) {
        baud_device_t* d= &baud_devices[i];
        if (!d->inUse || d->count==0 || d->us_per_char==0) continue;
        if (console_held( d->term_id )) continue; /* XOFF: nothing leaves this device */

        while (d->count>0 && d->next_due_us<=now) {
            /* Pop only what the endpoint TOOK. A bound /tN whose reader has
               stalled answers "would block", and popping first threw that byte
               away: a paced listing to a pty nobody was reading arrived as
               exactly one pty buffer (1024 bytes) and the rest was gone, every
               run. Leave it queued and offer it again shortly. The FIFO then
               fills, and ConsoleOut parks the writer, exactly as it already did
               for an unpaced write to the same full endpoint. */
            if (!ConsPutcTo( d->term_id, d->buf[d->head], d->owner[d->head] )) {
                d->next_due_us= now + BAUD_REFUSED_RETRY_US;
                break;
            }
            fifo_pop( d,&c,&owner );
            d->next_due_us += d->us_per_char;
            if (d->count==BAUD_FIFO_SIZE/2) wake_parked_writers(); /* low water */
        }
    }
    recompute_next_wake();
} /* baud_drain_due */

ulong baud_next_wake_delay_us( void )
{
    uint64_t now, left;
    if (g_next_wake_us==0) return ULONG_MAX; /* nothing pending: no deadline */
    now= host_micros();
    if (now>=g_next_wake_us) return 0;       /* already due */
    left= g_next_wake_us-now;
    return left>=ULONG_MAX ? ULONG_MAX-1 : (ulong)left; /* a deadline, however far */
} /* baud_next_wake_delay_us */

/* A key arrived on <term_id>: if a page pause was waiting for it, the pause
   ends and the key is spent on it. True when it was. */
Boolean console_page_release( short term_id )
{
    if (term_id<0 || term_id>=PAGE_TERMS || !pagePending[ term_id ]) return false;
    pagePending[ term_id ]= false;
    pageLines  [ term_id ]= 0;
    console_hold_changed( term_id, false );
    return true;
} /* console_page_release */

void console_hold_changed( short term_id, Boolean held )
{
    int i;

    if (!held) {
        /* Released. The backlog queued before the XOFF is due at timestamps
           that are now long past, so an unmodified drain would empty the whole
           ring in one burst and lose the pacing the FIFO exists to provide.
           Restart the burst from now, exactly as fifo_push does for the first
           character of a fresh one. */
        for (i=0; i<MAXBAUDDEV; i++) {
            baud_device_t* d= &baud_devices[i];
            if (d->inUse && d->term_id==term_id && d->count>0 && d->us_per_char>0)
                d->next_due_us= host_micros();
        }
    }

    recompute_next_wake(); /* the held device just left, or rejoined, the deadline set */
} /* console_hold_changed */

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
   for and nothing else left running to stay responsive to.

   An XOFF hold is deliberately ignored from here on. A held device is skipped
   by baud_drain_due() and contributes no deadline, so honouring the hold would
   make this loop spin at full CPU and never terminate: count>0 forever,
   baud_next_wake_delay_us() ULONG_MAX so not even a nap between passes. No
   OS-9 process is left to receive the XON that would release it either, so the
   flag has no owner any more -- drop it and get the queued bytes out. */
void baud_drain_all_pending( void )
{
    int     i;
    Boolean anyPending;
    ulong   delay;
    ulong   queued, lastQueued= ULONG_MAX;
    int     stalled= 0;

    g_final_drain= true;
    recompute_next_wake(); /* devices excluded while held rejoin the schedule */

    do {
        baud_drain_due();
        anyPending= false;
        queued    = 0;
        for (i=0; i<MAXBAUDDEV; i++) {
            if (baud_devices[i].inUse && baud_devices[i].count>0) {
                anyPending= true;
                queued   += baud_devices[i].count;
            }
        }

        /* A refused byte now stays queued rather than being thrown away (see
           baud_drain_due), so a /tN whose reader never comes back would hold
           the emulator open forever. Bounded by LACK OF PROGRESS, like
           baud_make_room: a slow reader that is still taking bytes is waited
           for however long it takes, and only ~5s of nothing at all discards
           what is left -- the fate every refused byte used to meet at once. */
        if (anyPending && queued==lastQueued) {
            struct timespec ts;
            if (++stalled > 500) {
                for (i=0; i<MAXBAUDDEV; i++) baud_devices[i].head= baud_devices[i].tail=
                                              baud_devices[i].count= 0;
                recompute_next_wake();
                break;
            }
            ts.tv_sec = 0;
            ts.tv_nsec= 10L*1000L*1000L; /* 10ms: an unpaced refusal sets no deadline to nap on */
            nanosleep( &ts, NULL );
            continue;
        }
        stalled   = 0;
        lastQueued= queued;

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

/* Wait, in place, until the FIFO has room for <need> bytes -- for a writer that
   CANNOT be parked.

   ConsoleOut's normal backpressure is to park the writing process in pWaitWrite
   and resume it later, which loses nothing. An INTERNAL COMMAND cannot be parked:
   it is host C running straight through (mount_usage calls upe_printf eleven
   times in a row), so the park has no one to suspend, the C code carries on, and
   every byte past a full FIFO was silently dropped. `mount -?` printed 678 bytes
   and about 262 reached the screen -- cut mid-word, no newline, the next prompt
   landing on top of it, and the -k/-v lines gone. That is exactly the shape that
   gets read as "this build has no -k option", and it did: it put a wrong entry on
   the roadmap and survived a later "verification" that used -r (full speed, no
   pacing), which is the one condition that hides it.

   Pacing is preserved -- the bytes still leave at the configured rate, this just
   waits for them instead of discarding them. Bounded by LACK OF PROGRESS rather
   than by wall-clock, so a slow line (300 baud needs ~8.5s per FIFO-full) waits
   as long as it genuinely takes, while a device that is never going to drain --
   XOFF-held, with no OS-9 process left to send XON -- gives up after ~2s and
   drops, which is no worse than what happened before. A hang would be. */
static void baud_make_room( baud_device_t* d, int need )
{
    int   stalled= 0;
    ulong delay;
    ushort was;

    while (BAUD_FIFO_SIZE - d->count < need) {
        was= d->count;

        /* Pump INPUT as well as output. Without this the loop was deaf: it
           drained and slept, so an XOFF hold could never be lifted from here
           and the ~2s give-up below fired every single time. Measured: with
           `^S` typed at the prompt BEFORE running a built-in, `dhelp` produced
           11 bytes instead of 1184 -- 1173 dropped silently, and never
           delivered even after `^Q`. The give-up is meant for a device with
           nobody left to release it; an interactive user IS somebody, and this
           is what lets their XON be seen.

           This is not the `while (held) CheckInputBuffers()` spin warned about
           in ConsoleOut: that one stalled a whole cooperative system inside a
           GUEST process's write syscall. Here the caller is an internal
           command or emulator narration, both unparkable host C and already
           spinning in this loop -- pumping input only makes the spin productive. */
        CheckInputBuffers();

        baud_drain_due();
        if (BAUD_FIFO_SIZE - d->count >= need) return;

        /* An XOFF hold is not a stall -- it is the terminal doing exactly what
           it was asked. Counting it toward the give-up budget meant a user who
           held ^S for longer than two seconds lost the rest of the output, with
           no error, and did not get it back on ^Q. Hold the budget still while
           the device is held; it then measures only what it was meant to: a
           device making no progress for reasons nobody is going to fix.

           This cannot spin forever in practice. The holder is a person, who can
           release it, and CheckInputBuffers() above is what lets their XON be
           seen. At shutdown console_held() answers false unconditionally
           (g_final_drain), so the exit path still drains rather than waiting on
           a hold whose owner has gone. */
        if (console_held( d->term_id )) stalled= 0;               /* held: not a stall */
        else if (d->count>=was) { if (++stalled > 200) return; }  /* ~2s of no progress */
        else                      stalled= 0;

        /* A held device sets no deadline, so the delay is "none" -- and the
           loop then went round without sleeping at all: an echo waiting for
           room behind ^S ran a whole core until ^Q (found by the pre-release
           review). Nap the same 10ms whatever the reason for waiting. */
        delay= baud_next_wake_delay_us();
        if (delay>0) {
            struct timespec ts;
            ulong capped= (delay>10000UL) ? 10000UL : delay; /* cap each nap at 10ms */
            ts.tv_sec = 0;
            ts.tv_nsec= (long)capped*1000L;
            nanosleep( &ts, NULL );
        }
    } // while
} /* baud_make_room */

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

/* The paced device a write from <pid> on <spP> goes through, or NULL when it
   goes straight to the screen: pacing off (-r), a baud rate that is not
   throttled, a pty, or a writer that cannot be parked -- pid 0 (the system
   process), the MAXPROCESSES banner sentinel, system state. Sets the device's
   character time from the path's PD_BAU as a side effect. */
static baud_device_t* paced_device( ushort pid, syspath_typ* spP )
{
    ulong          bps;
    baud_device_t* dev;

    if (!baud_throttle || pid==0 || pid>=MAXPROCESSES) return NULL;
    if (procs[pid].state==pSysTask || spP->term_id>=TTY_Base) return NULL;
    bps= baud_bps( spP->opt._sgs_bau );
    if (bps==0) return NULL;
    dev= baud_dev_for( spP->term_id );
    if (dev!=NULL) dev->us_per_char= (10UL*1000000UL)/bps; /* 10 bits/char */
    return dev;
} /* paced_device */

/* SCF's echo of a character being read, and every other byte I$ReadLn sends
   back (backspace, PD_OVF). It goes out through the terminal's output queue,
   BEHIND whatever is already waiting there, as a driver's echo does. Straight
   to the screen it overtook paced output still in the FIFO: with commands
   piped in, the shell's echo of its next line landed inside the previous
   command's output ("$ Secho hi" / "eptember 19, ..."). A read cannot park for
   output, so it waits for room, as narration does; a device that has stopped
   taking bytes altogether gets the old best-effort direct write. */
static void echo_putc( ushort pid, syspath_typ* spP, char c )
{
    baud_device_t* dev= paced_device( pid, spP );

    if (dev!=NULL) {
        if (BAUD_FIFO_SIZE - dev->count < 1) baud_make_room( dev, 1 );
        if (fifo_push( dev, (byte)c, pid )) return;
    }
    ConsPutc( c );
} /* echo_putc */

/* The process whose write request holds each terminal: SCF gives one request
   the device until it is done, setting V_BUSY, and queues any other process
   that asks meanwhile ("check to see if the device is busy or not -- if so,
   queue the process; otherwise set V_BUSY, call driver, clear V_BUSY",
   Kevin Darling on Microware's Technical I/O Reference; Peter Dibble,
   OS-9 Insights). A write here parks partway whenever the terminal cannot
   take more, and another process's write used to go straight through while
   it was parked, so two programs writing one terminal came out mixed
   character by character. Indexed by console id; 0 is nobody. */
#define CONS_OWNERS 256
static ushort consOwner[ CONS_OWNERS ];

/* <pid>'s parked write was ended by a signal (F$RTE): it holds no terminal */
void console_owner_release( ushort pid )
{
    int i;
    if (pid<MAXPROCESSES) procs[ pid ].unitRestLen= 0; /* nor a line ending to finish */
    for (i=0; i<CONS_OWNERS; i++) if (consOwner[ i ]==pid) consOwner[ i ]= 0;
} /* console_owner_release */

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
    struct _sgs* ot = &spC->opt; /* path opt table */
    Boolean      do_lf= false;
    process_typ* cp= &procs[pid];
    Boolean      paced= false;
    Boolean      held = false;       /* XOFF on this terminal: park, don't write */
    Boolean      narration= false;   /* emulator narration: host C, never parked */
    Boolean      owned= false;       /* this write takes part in terminal ownership */
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
                    /* The record ends on PD_EOR; the auto-LF belongs to the
                       CARRIAGE RETURN -- see ConsPutcEdit for the citation.
                       Same character in every ordinary configuration. */
                    do_lf= (ot->_sgs_alf && c==CR);
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
          dev  = paced_device( pid, spP );
          paced= dev!=NULL;

          /* Emulator NARRATION -- a `-d` trace line, an allocator warning, any
             u*_printf -- is host C running inside some process's syscall. It
             arrives through usrpath_puts, which raises in_recursion for exactly
             that span, and a genuine guest write never does. Like an internal
             command it cannot be parked: the dispatcher resumes a parked
             process by re-running its whole call, which printed the narration
             again, filled the FIFO again and parked again, forever. `-d 2` never
             got past shell start-up under pacing, and each retry re-ran the
             call itself too. So narration waits for room instead, and never
             takes over the resume state of a write that genuinely is parked. */
          narration= in_recursion;

          cnt= 0;
          if (pid>0 && pid<MAXPROCESSES && cp->state==pWaitWrite && !narration) {
              set_os9_state( pid, cp->saved_state, "ConsoleOut" );
              cnt=                cp->saved_cnt;

              /* the tail of a line ending the terminal took only part of */
              if (cp->unitRestLen>0) {
                  int w= hostterm_bound( gConsoleID ) ?
                         hostterm_put( gConsoleID, cp->unitRest, cp->unitRestLen ) : cp->unitRestLen;
                  if (w>0) {
                      cp->unitRestLen-= (short)w;
                      memmove( cp->unitRest, cp->unitRest+w, (size_t)cp->unitRestLen );
                  }
                  if (cp->unitRestLen>0) {
                      cp->saved_cnt  = cnt;
                      cp->saved_state= cp->state;
                      set_os9_state( pid, pWaitWrite, "ConsoleOut (line ending)" );
                      arbitrate= true;
                      *maxlenP= cnt;
                      return 0;
                  }
              }
          }

          /* another process's request still holds this terminal: wait for it
             (see consOwner), before writing anything */
          owned= pid>0 && pid<MAXPROCESSES && !narration && !cp->isIntUtil &&
                 cp->state!=pSysTask && gConsoleID>=0 && gConsoleID<CONS_OWNERS;
          if (owned) {
              ushort own= consOwner[ gConsoleID ];
              if (own!=0 && own!=pid && own<MAXPROCESSES && procs[own].state==pWaitWrite) {
                  cp->saved_cnt  = cnt;
                  cp->saved_state= cp->state;
                  set_os9_state( pid, pWaitWrite, "ConsoleOut (terminal busy)" );
                  arbitrate= true;
                  *maxlenP= cnt;
                  return 0;
              }
          }

          /* XOFF on this terminal: halt its output until XON, and PARK the
             writer to do it. Parking is the whole point -- the scheduler is
             cooperative, so a `while (held) CheckInputBuffers()` spin inside
             the write syscall (what the classic-Mac console did) never lets
             the process leave the kernel, and stalls every OTHER process and
             every due F$Alarm along with the one that asked to be paused.
             A pWaitWrite process is rescheduled periodically (procstuff.c) and
             comes back through here, so release needs no wakeup of its own.
             pid 0 / the MAXPROCESSES sentinel / pSysTask cannot be parked (see
             the two branches below); their output goes out regardless, which is
             the same compromise those branches already make. */
          /* ...but an INTERNAL COMMAND cannot be parked either: it is host C
             running straight through, with no emulated PC to rewind, so the
             park above drops everything it had left to say. Measured before
             this: with `^S` typed at the prompt and then `dhelp`, 11 bytes of
             1184 arrived and the other 1173 were never delivered, not even
             after `^Q`. Wait for the release in place instead, pumping input
             so the XON can actually be seen -- the loop was otherwise deaf and
             the hold could never lift from here.

             Spinning here does NOT cost the concurrency the park exists to
             protect: an internal command already runs to completion as host C,
             so nothing else is running during it either way. Bounded so a hold
             whose owner has gone cannot wedge the emulator; at shutdown
             console_held() answers false anyway (g_final_drain). */
          if ((cp->isIntUtil || narration) && console_held( (short)gConsoleID )) {
              int spins= 0;
              while (console_held( (short)gConsoleID ) && ++spins <= 12000) {
                  struct timespec ts;
                  CheckInputBuffers();   /* so a typed XON is seen */
                  baud_drain_due();
                  ts.tv_sec= 0; ts.tv_nsec= 10L*1000L*1000L; /* 10ms */
                  nanosleep( &ts, NULL );
              }
          }

          held= console_held( (short)gConsoleID ) &&
                pid>0 && pid<MAXPROCESSES && cp->state!=pSysTask &&
                !cp->isIntUtil && !narration; /* handled above; parking either would drop or livelock */

          while (cnt<*maxlenP) {
              Boolean needsLF; /* does this char carry a trailing auto-LF? */
              int     nulls;   /* PD_NUL padding bytes that follow it */
              int     need;    /* FIFO slots this char and its tail need */
              int     q;

              if (held) {
                  cp->saved_cnt  = cnt;
                  cp->saved_state= cp->state;
                  set_os9_state( pid, pWaitWrite, "ConsoleOut" );
                  arbitrate= true;
                  break;
              }

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
              /* PD_ALF follows a CARRIAGE RETURN, not PD_EOR -- citation in
                 ConsPutcEdit. Ending the RECORD is still PD_EOR's job, and
                 that test is the `wrln` break at the bottom of this loop. */
              needsLF= (wrln && c==CR && ot->_sgs_alf);
              /* PD_NUL: "the number of NULL padding bytes to be sent after a
                 carriage return/line-feed character" (v2.4 Technical I/O
                 Manual); the Guru says the same, "[NUL] characters to send
                 after [CR] ... for slow devices that do not support flow
                 control handshaking, such as teletypes". So it keys on CR
                 exactly as PD_ALF does, and belongs to I$WritLn's line
                 editing, which is what `wrln` gates. It was read into
                 struct _sgs and never used. Defaults to zero, so a path that
                 has not asked for padding is byte-for-byte as before. */
              nulls  = (wrln && c==CR) ? ot->_sgs_nul : 0;
              need   = 1 + (needsLF ? 1:0) + nulls; /* always fits: see BAUD_FIFO_SIZE */

              if (paced) {
                  /* An internal command is host C and cannot be parked and
                     resumed -- parking it drops the rest of its output. Wait
                     for room instead. `paced` already excludes the other
                     unparkable writers (pid 0, the MAXPROCESSES sentinel,
                     pSysTask), so isIntUtil and narration are the cases left. */
                  if ((cp->isIntUtil || narration) && BAUD_FIFO_SIZE - dev->count < need)
                      baud_make_room( dev, need );

                  if (BAUD_FIFO_SIZE - dev->count < need) {
                      /* Narration still without room after baud_make_room gave
                         up -- a /tN nobody has read for seconds -- is DROPPED.
                         Parking it is the re-dispatch loop narration exists to
                         avoid; guest output parks and loses nothing. */
                      if (!narration) {
                          cp->saved_cnt  = cnt;
                          cp->saved_state= cp->state;
                          set_os9_state( pid, pWaitWrite, "ConsoleOut" );
                          arbitrate= true;
                          break;
                      }
                  }
                  else {
                  /* Stamp the WRITER, not whoever will be running when these
                     bytes finally reach the screen -- that is the whole point. */
                                fifo_push( dev, c,   pid );
                  if (needsLF)  fifo_push( dev, LF,  pid );
                  for (q=0; q<nulls; q++) fifo_push( dev, NUL, pid );
                  }
              }
              else if (hostterm_bound( gConsoleID ) && !narration
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
                  /* c, its optional LF, and up to 255 PD_NUL pad bytes --
                     kept one unit for the same reason the CR and its LF are:
                     a partially written tail is what leaves a line looking
                     terminated when it is not. */
                  char pair[2+255];
                  int  len= 0;
                  int  w;

                  pair[len++]= c;
                  if (needsLF) pair[len++]= LF;
                  for (q=0; q<nulls; q++) pair[len++]= NUL;

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
                         drains. When PART of the unit went, the rest waits in
                         unitRest instead: re-sending the whole unit on resume
                         wrote its CR, LF or pad bytes twice (pre-release
                         review). */
                      if (w>0) {
                          cp->unitRestLen= (short)(len-w);
                          memcpy( cp->unitRest, pair+w, (size_t)(len-w) );
                          cnt++;                   /* the character itself went */
                      }
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
                  for (q=0; q<nulls; q++) ConsPutc( NUL );
              }
              cnt++;

              if (cp->state==pSysTask) { /* should never go to here */
                  cp->systask_offs= cnt-1; /* store it here !! */
                  cnt= *maxlenP;
                  break; /* tty/pty break */
              }

              if (c==CR && ot->_sgs_pause && ot->_sgs_page>0 &&
                  spP->term_id>=0 && spP->term_id<PAGE_TERMS &&
                  ++pageLines[ spP->term_id ] >= ot->_sgs_page) {
                  /* a full page: hold here until a key, as XOFF would --
                     except for a writer that cannot be parked (a built-in,
                     narration), which carries on */
                  if (pid>0 && pid<MAXPROCESSES && cp->state!=pSysTask && !cp->isIntUtil && !narration) {
                      pagePending[ spP->term_id ]= true;
                      console_hold_changed( spP->term_id, true );
                      held= true;
                  }
                  else pageLines[ spP->term_id ]= 0;
              }

              if (wrln && c!=NUL && c==ot->_sgs_eorch) {
                  break; /* end of record -- LF (if any) already delivered above */
              }
          } /* while */

          /* parked partway: this request keeps the terminal; done: let it go */
          if (owned) {
              if      (cp->state==pWaitWrite)          consOwner[ gConsoleID ]= pid;
              else if (consOwner[ gConsoleID ]==pid) consOwner[ gConsoleID ]= 0;
          }

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
      struct _sgs* ot= &spP->opt;
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
