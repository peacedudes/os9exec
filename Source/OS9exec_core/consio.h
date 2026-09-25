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
 *    Revision 1.10  2006/12/01 19:54:47  bfo
 *    <consoleSleep> param for "HandleOneEvent" (reduce MacClassic load)
 *
 *    Revision 1.9  2006/02/19 15:51:07  bfo
 *    Header changed to 2006
 *
 *    Revision 1.8  2005/06/30 11:04:15  bfo
 *    win_linux => win_unix
 *
 *    Revision 1.7  2005/01/22 15:45:22  bfo
 *    Renamed to ifdef MACOS9
 *
 *    Revision 1.6  2004/12/03 23:53:12  bfo
 *    MacOSX MACH adaptions
 *
 *    Revision 1.5  2004/11/20 11:44:06  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.4  2003/05/17 11:03:48  bfo
 *    (CVS header included)
 *
 *
 */


/* Interface to the terminal console */
#ifdef TERMINAL_CONSOLE
  /* global control vars of console_stubs.c and serial.c */
  #define NO_CONSOLE 65535
  extern  short      gConsoleNLExpand;
  extern  char       gTitle[OS9NAMELEN];
  
  #ifdef MACOS9
    extern Rect*     gRect;
  #endif
#endif

extern  int          gLastwritten_pid;
extern  int          gConsoleID;
extern  syspath_typ* g_spP;
extern  ulong        gNetActive;
extern  ulong        gNetLast;


    
#ifdef TERMINAL_CONSOLE
  extern int     gConsoleQuickInput;
  
  #ifndef SERIAL_INTERFACE
    /* global control vars of console_stubs.c */
    extern short gConsoleEcho;
    extern int   gInBufUsed;
    
    /* global control vars of TermConsole.c */
    extern short TermWinScrolls;
  #endif

  #ifdef win_unix
    extern ttydev_typ main_mco;
  #endif
   
  #ifdef MACOS9
    extern Boolean gDocDone; /* synchronisation for <gFS> callback */
    extern FSSpec  gFS;
    
    extern void    Install_AppleEvents( void );
    extern int     HandleOneEvent     ( EventRecord* pEvent, int consoleSleep );
    extern long    WriteCharsToConsole( char *buffer, long n );
    extern ulong   SysPathWindow      ( syspath_typ* );
    extern ulong   CurrentWindow      ( void );
    extern void    UpdatePrms         ( ulong  wPtr,   ulong  wStore, ulong wSize, 
                                        ushort wIndex, ushort wTot );
    extern void    RemoveConsole      ( void );
  #endif
    
  extern   Boolean KeyToBuffer        ( ttydev_typ *mco, char key );
  extern   Boolean KeyAhead           ( ttydev_typ *mco, char key );
  extern   void    KeysAhead          ( ttydev_typ *mco );
  extern   int     KeyRoom            ( const ttydev_typ *mco );
  extern   Boolean console_page_release( short term_id );
  extern   void    console_owner_release( ushort pid );
  extern   void    console_owner_claim  ( ushort pid, short term );
  extern   Boolean DevReady           ( long *count );
#endif

/* eof */
