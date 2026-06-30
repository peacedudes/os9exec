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
 *    Revision 1.14  2007/02/04 20:08:29  bfo
 *    - DLL connect/disconnect implemented here now
 *    - "Plugin_Possible"/"Native_Possible" enhanced
 *
 *    Revision 1.13  2007/01/28 22:14:00  bfo
 *    - Native program handling, <modBaseP> param added
 *    - int command 'native' added
 *
 *    Revision 1.12  2007/01/07 13:48:17  bfo
 *    "Init_IProg" added
 *    Callback for "StrToFload"/"StrToDouble" placed locally
 *    Internal command "plugin" ( -d/-e ) added
 *    DLL jump implemented here (in a first working version)
 *
 *    Revision 1.11  2007/01/04 20:27:19  bfo
 *    Up to date
 *
 *    Revision 1.10  2007/01/02 11:19:01  bfo
 *    "ChangeElement" visible from outside
 *
 *    Revision 1.9  2006/08/29 22:10:38  bfo
 *    isintcommand with additional <isPtoC> parameter
 *
 *    Revision 1.8  2006/08/26 23:46:49  bfo
 *    "Is_PtoC" no longer visible, "isintcommand" with const char*
 *
 *    Revision 1.7  2006/07/10 09:55:58  bfo
 *    ushort parentid added for "callcommand"
 *
 *    Revision 1.6  2006/02/19 16:28:56  bfo
 *    'Is_PtoC' visible from ouside / PtoC + thread support added
 *
 *    Revision 1.5  2004/11/20 11:44:07  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.4  2003/05/17 11:25:21  bfo
 *    (CVS header included)
 *
 *
 */


/* OS9exec/nt internal commands */
#ifndef __intcommand_h
#define __intcommand_h

/* public vars */
extern ushort icmpid; /* current internal command's PID */

/* internal commands (in separate files) */
os9err int_rename( ushort pid, int argc, char **argv );
os9err int_move  ( ushort pid, int argc, char **argv );
os9err int_help  ( ushort pid, int argc, char **argv );

#ifdef windows32
os9err int_wincmd( ushort pid, int argc, char **argv );
#endif

/* Native program handling */
Boolean Native_Possible( Boolean hardCheck );
Boolean Plugin_Possible( Boolean hardCheck );

#if defined NATIVE_SUPPORT || defined PTOC_SUPPORT
  const char* DLL_Suffix();
  int         NumberOfNativeProgs( plug_typ* p );
  os9err      ConnectDLL         ( plug_typ* p );
  void        display_pluginList ( Boolean dispTitle, Boolean atStartup );
  void        ChangeNativeElement( const char* s,     Boolean addIt     );
#endif


/* utility */
void    Change_DbgPath( int argc, char** argv, char** pp, ushort* kp );

os9err  _errmsg( os9err err, char* format, ...);

int     isintcommand( const char* name, Boolean *isNative, void** modBaseP );

os9err  prepArgs    ( char* arglist, ushort *argcP, char*** argP);
os9err  callcommand ( char* name,    ushort pid, ushort parentid,
                                                 int argc,     char** argv, Boolean *asThread );
os9err  call_hostcmd( char* cmdline, ushort pid, int moreargs, char** argv );

#endif

/* eof */
