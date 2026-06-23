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



ulong WriteCharsToPTY( char* buffer, ulong n, int consoleID, Boolean do_lf )
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
      { uint32_t n32= (uint32_t)n;
        PutCharsToTTY( currentpid,spC, &n32,buffer, do_lf ); /* put it into pipe !! */
        n= n32; }
      if    (n>0) lw_pid( mco );                          /* assign for later use */
      return n;

    #else
      #ifndef linux
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
 
    /* if not yet ready handle other events */
    if (mco->holdScreen || (!DevReadyTTY( &cnt,consoleID ))) {      
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
    
  sprintf( title, "%s%s", OS9exec_Name(), p );
    
  #ifdef TERMINAL_CONSOLE
    if (vmod) sprintf( title, "%s  /vmod - display for  \"%s\"", 
                       title, gTitle );
    else {    
      #ifndef windows32
        Console_Name( gConsoleID, (char*)&cons );
        sprintf( title, "%s  /%s - terminal window", title, cons );
      #endif
    } // if
  #endif
} /* WindowTitle */



#if defined windows32
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
  if (!ok || n==0) return;

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


#if defined MPW || defined UNIX
void HandleEvent( void )
{
} /* empty implementation */
#endif


/* CheckInputBuffers */
void CheckInputBuffers(void)
{
    #if defined MACTERMINAL && defined USE_CLASSIC
      CheckInputBuffersSerial(); /* check characters from serial lines */
    #endif
    
    CheckInputBuffersTTY();      /*   "        "       "  ttys         */
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
    HandleEvent();

    #ifdef linux
      *count= 1; return true; /* %%% not yet as it should be */
    #endif

    #ifdef MACOSX
      *count= 1; return true; /* %%% not yet as it should be */
    #endif

    return DevReadyTerminal( count, &main_mco );
}    /* DevReady */
#endif



#ifdef TERMINAL_CONSOLE
long ReadCharsFromTerminal(char *buffer, long n, ttydev_typ* mco)
{    
    long cnt; /* this is the data base */
    
    if (mco->holdScreen) {
        devIsReady= false;
        *buffer= NUL;
        return 1;
    }
        
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

