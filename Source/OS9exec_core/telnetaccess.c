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
 *    Revision 1.13  2007/01/04 21:08:08  bfo
 *    Avoid unused var (cons)
 *
 *    Revision 1.12  2007/01/04 20:52:59  bfo
 *    Shorter window title for Windows version
 *
 *    Revision 1.11  2006/12/01 19:56:44  bfo
 *    "HandleEvent": no more separate MACH implementation
 *
 *    Revision 1.10  2006/06/11 22:07:13  bfo
 *    set_os9_state with 3rd param <callingProc>
 *
 *    Revision 1.9  2006/06/01 15:17:15  bfo
 *    Signedness adaptions (for gcc 4.0)
 *
 *    Revision 1.8  2006/02/19 15:28:50  bfo
 *    Header changed to 2006
 *
 *    Revision 1.7  2005/06/30 11:55:40  bfo
 *    Mach-O support
 *
 *    Revision 1.6  2004/11/20 11:44:08  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.5  2003/05/17 10:14:41  bfo
 *    Disabled for USE_CARBON
 *
 *    Revision 1.4  2002/08/09 22:39:21  bfo
 *    New procedure set_os9_state introduced and adapted everywhere
 *
 *
 */


/* Telnet socket interface */
/* ======================= */

#include "os9exec_incl.h"

#ifdef MACTERMINAL
#include "serialaccess.h"
#endif

#if defined(linux) || defined(MACOSX)
#include <sys/ioctl.h>
#endif

#ifdef MINGW
#include <windows.h>  /* INPUT_RECORD, ReadConsoleInput, GetNumberOfConsoleInputEvents */
#endif



void InitTTYs()
/* initialize them all (bfo) */
{
    ttydev_typ* mco;
    int      k;
    
    for (k=0; k<MAXTTYDEV; k++) {
        mco= &ttydev[k];
        mco->installed = false;
        mco->inBufUsed =     0;
        mco->holdScreen= false;
        mco->pid       =     0;
    }
} /* InitTTYs */



short InstallTTY( syspath_typ* spP, int consoleID )
/* initialize access to TTY */
{
    ttydev_typ* mco= &ttydev[consoleID-TTY_Base];
    
    if (!mco->installed) {
         mco->spP= spP;
         mco->installed= true;
    }
    
    return 0;
} /* InstallTTY */



void RemoveTTY( int consoleID )
/* de-initalize TTY */
{
    ttydev_typ* mco= &ttydev[consoleID-TTY_Base];
                mco->installed= false;
} /* RemoveTTY */



uint32_t WriteCharsToPTY( char* buffer, uint32_t n, int consoleID, Boolean do_lf )
/* write characters to TTY */
{
    #ifdef PIP_SUPPORT
      ushort       pid= currentpid;
      process_typ* cp = &procs[pid];
      syspath_typ* spC= crossedPath( currentpid,g_spP );
      ttydev_typ*  mco= &ttydev[consoleID-TTY_Base];

      if (cp->state==pWaitRead) {
          set_os9_state( pid, cp->saved_state, "WriteCharsToPTY" );
          n=                  cp->saved_cnt;
      } // if

	  if (mco->holdScreen) {
          cp->saved_cnt  = n;
          cp->saved_state= cp->state;
          set_os9_state( pid, pWaitRead, "WriteCharsToPTY" );
          return n;
	  } // if

      spC->u.pipe.pchP->do_lf= do_lf;                     /* store it here also */
      PutCharsToTTY( currentpid,spC, &n,buffer, do_lf ); /* put it into pipe !! */
      if    (n>0) lw_pid( mco );                          /* assign for later use */
      return n;

    #else
      #ifndef __GNUC__ /* MPW-only pragma; GCC warns it is ignoring it. Gate on the COMPILER, not the OS -- "not linux" wrongly includes mingw. */
      #pragma unused( buffer,n,consoleID,do_lf )
      #endif
      
      return 0;   
    #endif
} /* WriteCharsToPTY */



void CheckInputBuffersTTY(void)
{
    #ifdef PIP_SUPPORT
      int  k;
      for (k=0; k<MAXTTYDEV; k++) CheckInBufferTTY( &ttydev[k] );
    #endif
} /* CheckInputBuffersTTY */




Boolean DevReadyTTY(long *cnt, int consoleID)
/* how many characters are ready at the TTY ? */
{
    Boolean     ok;
    ttydev_typ* mco= &ttydev[consoleID-TTY_Base];

    *cnt= 0; /* default */
    mco->pid= currentpid;
    CheckInputBuffersTTY();
    
    /* how many characters are currently ready in input buffer ? */
           ok=       mco->inBufUsed; 
    if    (ok) *cnt= mco->inBufUsed;
    return ok; 
} /* DevReadyTTY */



long ReadCharsFromPTY(char *buffer, long n, int consoleID)
/* read characters from TTY */
{
    long        cnt; /* this is the data base */
    ttydev_typ* mco= &ttydev[consoleID-TTY_Base];
 
    /* if not yet ready handle other events.
       holdScreen is deliberately NOT part of this test -- XOFF halts output
       only (see ReadCharsFromTerminal below for the full reasoning); the hold
       for a pty's output lives in WriteCharsToPTY. */
    if (!DevReadyTTY( &cnt,consoleID )) {
      devIsReady= false;
      *buffer= NUL; return 1; /* if 0, it would not return to caller */
    }

    devIsReady= true;
    if       (mco->inBufUsed) {   // got some chars to return...
        cnt= (mco->inBufUsed<n ? mco->inBufUsed : n); // return this many chars
        MoveBlk( buffer, mco->inBuf, cnt );
        if  (cnt<mco->inBufUsed) {
             // didn't return all chars; shift buffer contents down
             MoveBlk( mco->inBuf, mco->inBuf+cnt, mco->inBufUsed-cnt );
             mco->inBufUsed -= cnt; /* copy <inBuf>, and not <buffer> !!! */
        } 
        else mco->inBufUsed  = 0;
        return cnt;
    }
  
    return 0;
} /* ReadCharsFromPTY */
    


/* ---------------------------------------------------------------- */

void WindowTitle( char* title, Boolean vmod )
{
  const char* p;
    
  #if defined TERMINAL_CONSOLE && !defined windows32
    char cons[ 40 ];
  #endif
    
  #ifdef USE_UAEMU
    p= " (UAE)" ;
  #else
    p="";
  #endif
    
  /* Composed in a local buffer and appended by offset, rather than sprintf'ing
   * <title> into itself via "%s". Passing the destination as one of its own
   * source arguments is overlapping source/destination -- undefined behaviour,
   * which mingw-w64 flags as -Wrestrict. Both callers hand us a 255-byte buffer. */
  char buf[ 255 ];
  size_t used;

  snprintf( buf,sizeof(buf), "%s%s", OS9exec_Name(), p );

  #ifdef TERMINAL_CONSOLE
    used= strlen( buf );

    if (vmod) snprintf( buf+used, sizeof(buf)-used,
                        "  /vmod - display for  \"%s\"", gTitle );
    else {
      #ifndef windows32
        Console_Name( gConsoleID, (char*)&cons );
        snprintf( buf+used, sizeof(buf)-used, "  /%s - terminal window", cons );
      #endif
    } // if
  #endif

  strcpy( title, buf );
} /* WindowTitle */



#if defined windows32 || defined MINGW
void HandleEvent( void )
{
  #define STARTVAL -200
  static int hvv= STARTVAL;

  char         c;
  Boolean      ok;
  INPUT_RECORD ir;
  DWORD        n;

  if (hvv<0) hvv++;
  else {
    hvv= STARTVAL;
    Sleep( 1 ); /* sleep in milliseconds */
  } // if

  /* is there any event ? */
       ok= GetNumberOfConsoleInputEvents( hStdin, &n );
  if (!ok) {
      /* GetNumberOfConsoleInputEvents fails whenever stdin isn't a real
       * console -- every non-interactive caller redirects it from/to a
       * pipe instead (the Swift integration-test harness among them, via
       * Foundation.Pipe). This function had no fallback for that case at
       * all, so bytes an automated caller writes to stdin just sit in the
       * pipe forever and anything reading through KeyToBuffer() (e.g. the
       * OS-9 shell's own input loop) hangs indefinitely even though real
       * data is sitting there ready to read -- confirmed live: os9exec.exe
       * produced zero output when driven over a plain pipe with no
       * console attached, even for the simplest possible test. Mirror the
       * UNIX branch below (ioctl FIONREAD + bounded read, feeding
       * KeyToBuffer()) using the Win32 pipe equivalent, PeekNamedPipe +
       * ReadFile. */
      DWORD avail= 0;
      int   room;

      if (PeekNamedPipe( hStdin, NULL,0, NULL, &avail, NULL ) && avail>0) {
          room= INBUFSIZE-1 - main_mco.inBufUsed;
          if ((int)avail>room) avail= (DWORD)room;

          while (avail-->0) {
              DWORD got= 0;
              if (!ReadFile( hStdin, &c,1,&got,NULL ) || got!=1) break;
              /* Unlike a real console's Enter key (already CR via
               * ReadConsoleInput below, no translation needed), an
               * automated caller writing to this pipe (the Swift test
               * harness, any non-interactive driver) sends genuine
               * LF-terminated lines. ConsGetc()'s MINGW branch (consio.c)
               * deliberately skips the LF<->CR swap it does for Unix,
               * assuming every MINGW byte already arrived as CR -- true
               * only for the console path, not this one. Without the swap
               * here, ConsRead's endchar==CR check (consio.c) never
               * matches, so a piped command line is never recognized as
               * complete and the shell never dispatches it. Mirror the
               * swap ConsGetc() already does for genuine Unix terminals. */
              if      (c==LF) c= CR;
              else if (c==CR) c= LF;
              if (c!=NUL) KeyToBuffer( &main_mco, c );
          } // while
      } // if
      return;
  }
  if (n==0) return;

  /* if yes, get it. If it keydown, put char into input buffer */
       ok= ReadConsoleInput( hStdin, &ir, 1, &n );
  if (!ok || n==0) return;

  if      (ir.EventType==KEY_EVENT &&
           ir.Event.KeyEvent.bKeyDown) {
        c= ir.Event.KeyEvent.uChar.AsciiChar;
    if (c!=NUL) KeyToBuffer( &main_mco, c );
  }
} /* HandleEvent */
#endif


#ifdef MPW
void HandleEvent( void )
{
} /* empty implementation */
#endif

#if defined UNIX && !defined MINGW
void HandleEvent( void )
/* Poll stdin for pending keystrokes and feed them through KeyToBuffer(),
 * the same way the windows32 branch above does via ReadConsoleInput.
 * This is what lets Ctrl-C/Ctrl-E interrupt a process immediately even
 * when nothing is currently blocked in a console read: real OS-9
 * hardware notices the abort character in the SCF driver's ISR, not
 * gated behind an active I$Read, and this restores that behaviour.
 */
{
    #if defined(linux) || defined(MACOSX)
      int  avail= 0;
      int  room;
      char c;

      if (ioctl(STDIN_FILENO, FIONREAD, &avail)==0) {
          /* Never read more raw bytes than inBuf has guaranteed space for --
           * KeyToBuffer() silently drops keys once inBuf is full, and unlike
           * a tty we can't push a byte back into a pipe once read() has taken
           * it. Special chars (Ctrl-C etc.) don't consume inBuf space at all,
           * so this is a conservative floor: worst case we defer a few plain
           * characters to the next call instead of losing any.
           */
          room= INBUFSIZE-1 - main_mco.inBufUsed;
          if (avail>room) avail= room;

          while (avail-->0) {
              if (read(STDIN_FILENO, &c,1)!=1) break;
              KeyToBuffer( &main_mco, c );
          } // while
      } // if
    #endif
} /* HandleEvent */
#endif


/* CheckInputBuffers */
void CheckInputBuffers(void)
{
    #if defined MACTERMINAL && defined USE_CLASSIC
      CheckInputBuffersSerial(); /* check characters from serial lines */
    #endif
    
    CheckInputBuffersTTY();      /*   "        "       "  ttys         */
    hostterm_poll();             /* and every host-backed /tN          */
    HandleEvent();               /* and check also the Mac events      */
} /* CheckInputBuffers */



Boolean DevReadyTerminal( long *count, ttydev_typ* mco )
/* true, if next character(s) can be read */
/* used by "ReadCharsFromConsole" and "GetStat.SS_Ready" */
{
    Boolean ok=         mco->inBufUsed; 
    if     (ok) *count= mco->inBufUsed;
    return  ok;
} /* DevReadyTerminal */
    

#if defined win_unix
Boolean DevReady( long *count )
{
    /* HandleEvent() now drains stdin into main_mco.inBuf (see above) —
     * check that buffer via DevReadyTerminal() rather than re-polling
     * the raw fd, which would race with HandleEvent()'s own read().
     */
    HandleEvent();
    return DevReadyTerminal( count, &main_mco );
}    /* DevReady */
#endif



#ifdef TERMINAL_CONSOLE
long ReadCharsFromTerminal(char *buffer, long n, ttydev_typ* mco)
{
    long cnt; /* this is the data base */

    /* NO holdScreen test here. XOFF halts a device's OUTPUT and nothing else
       ("Output from a SCF device is halted immediately when PD_XOFF is
       received" -- Technical I/O Manual V2.4, PD_XOFF); input keeps being
       taken, which is why a real terminal lets you type ahead through a pause.
       This used to return not-ready whenever the device was held, so keystrokes
       piled up in inBuf and every one of them ran at once on XON.
       SS_Ready never had the test (DevReadyTerminal above), so the device also
       reported characters available while the read refused to hand them over.

       Consequence, accepted deliberately: SCF echoes through ConsPutc, which is
       best-effort and cannot park, so a character typed during a hold appears
       on the held screen. That is how a terminal with local echo behaves, and
       the alternative -- freezing input to keep the screen perfectly still --
       is the divergence this comment exists to record the removal of. */
    devIsReady= true; // the default value for devIsReady
    if       (mco->inBufUsed) {   // got some chars to return...
        cnt= (mco->inBufUsed<n ? mco->inBufUsed : n); // return this many chars
        MoveBlk( buffer,mco->inBuf, cnt );

        if (cnt<mco->inBufUsed) {
             // didn't return all chars; shift buffer contents down
             MoveBlk( mco->inBuf,mco->inBuf+cnt, mco->inBufUsed-cnt );
             mco->inBufUsed -= cnt; /* copy <inBuf>, and not <buffer> !!! */
        } 
        else mco->inBufUsed  = 0;
        return cnt;
    }
    
    // no chars in the buffer... enter an event loop till we get some (or a RETURN)
    while (true) {
        HandleEvent();
        if (gConsoleQuickInput) {
               // terminate read immediately (non-blocking)
            if (mco->inBufUsed) 
               return ReadCharsFromTerminal( buffer,n, mco );

            devIsReady= false; // break the loop
            *buffer= NUL;
            return 1; /* if 0, it will not return to caller */
        } 
        else { // terminate read only when the RETURN key is pressed
            for (cnt=0; cnt<mco->inBufUsed; cnt++) {
                if (mco->inBuf[cnt] == keyRETURN) 
                    return ReadCharsFromTerminal( buffer,n, mco );
            }
        }
    } /* while (true) */
} /* ReadCharsFromTerminal */
#endif


/* eof */

