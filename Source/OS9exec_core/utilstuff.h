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
 *    Revision 1.31  2007/03/24 12:26:26  bfo
 *    "FD_Name" introduced
 *
 *    Revision 1.30  2007/02/24 14:06:57  bfo
 *    - FD_ID: Additional params <dirid> and <fName>
 *    - MacOSX no longer based on Inodes (same as Linux)
 *    - Consistent m->ident key handling for MacOS9
 *
 *    Revision 1.29  2007/01/28 21:34:04  bfo
 *    'ustrcmp' made invisible
 *
 *    Revision 1.28  2007/01/07 13:17:07  bfo
 *    Use MACOS9 define
 *
 *    Revision 1.27  2006/06/11 22:01:49  bfo
 *    set_os9_state with 3rd param <callingProc>
 *
 *    Revision 1.26  2006/05/16 13:11:20  bfo
 *    Linux full path name adaption
 *
 *    Revision 1.25  2006/02/19 15:52:52  bfo
 *    Header changed to 2006
 *
 *    Revision 1.24  2005/07/06 21:04:38  bfo
 *    defined UNIX
 *
 *    Revision 1.23  2005/07/02 14:18:46  bfo
 *    Adapted for Mach-O
 *
 *    Revision 1.22  2005/06/30 11:44:15  bfo
 *    Mach-O support
 *
 *    Revision 1.21  2005/04/15 11:39:33  bfo
 *    RBF_ImgSize implemented here now
 *
 *    Revision 1.20  2004/12/04 00:08:18  bfo
 *    MacOSX MACH adaptions
 *
 *    Revision 1.19  2004/11/20 11:44:08  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.18  2004/10/22 23:35:07  bfo
 *    Up to date (for test)
 *
 *    Revision 1.17  2003/05/17 10:17:21  bfo
 *    'p2cstr' and 'c2pstr' prototype defined in the ".h" file now
 *
 *    Revision 1.16  2003/05/05 17:56:49  bfo
 *    Activate most of the ramDisk things even without RAM_SUPPORT
 *
 *    Revision 1.15  2003/04/25 19:29:17  bfo
 *    Handle Netatalk IDs out of range correctly now (using FD_ID)
 *
 *    Revision 1.14  2002/10/15 17:58:36  bfo
 *    'CutUp' visible again (for linuxfiles.c)
 *
 *    Revision 1.13  2002/10/02 18:52:29  bfo
 *    GetScreen function is now defined at "utilstuff"
 *
 *    Revision 1.12  2002/09/19 21:59:20  bfo
 *    "CutUp" invisible, "CutUp"/"Eatback" adapted for Win/Mac + OS9
 *
 *    Revision 1.11  2002/09/11 17:23:24  bfo
 *    os9_long_inc with unsigned int* param
 *
 *    Revision 1.10  2002/08/13 21:24:17  bfo
 *    Some more variables defined at the real procid struct now.
 *
 *
 */


/* General Utility routines */
char* nullterm     ( char* s1,const char* s2, ushort max );
//int ustrcmp( const char* s1,const char* s2 );
int  pustrcmp( const char* s1,const char* s2 );
int  ustrncmp( const char* s1,const char* s2, ushort n );


void os9_long_inc ( unsigned int* a, uint32_t increment );
void set_os9_state( ushort cpid, pstate_typ state, const char* callingProc );

os9err host2os9err( OSErr hosterr, ushort suggestion );
os9err c2os9err   ( int   cliberr, ushort suggestion );
os9err os9error   ( os9err err );


uint32_t j_date  (              int  d,  int  m,  int  y  );
void     g_date  ( uint32_t jdn, int *dp, int *mp, int *yp );

void   TConv   ( time_t u, struct tm* tim );
time_t UConv             ( struct tm* tim );
void   GetTim            ( struct tm* tim );

void   Get_Time( uint32_t *cTime, uint32_t *cDate, int *dayOfWk, int *currentTick,
                 Boolean asGregorian, Boolean withTicks );

ulong GetScreen( char mode );


#ifdef USE_CARBON
  char* p2cstr( unsigned char* s );
  unsigned char* c2pstr( char* s );
#endif

ulong Min( ulong a, ulong b );
ulong Max( ulong a, ulong b );

char* StrBlk_Pt( char* s, int n );

Boolean IsRead ( ushort  mode );
Boolean IsWrite( ushort  mode );
Boolean IsRW   ( ushort  mode );
Boolean IsDir  ( ushort  mode );
Boolean IsExec ( ushort  mode );
Boolean IsCrea ( ushort  mode );
Boolean IsTrDir( ushort umode ); /* transparent file system: unix style */

os9err  pSCFopt     ( ushort pid, syspath_typ*, byte* buffer );
os9err  pRBFopt     ( ushort pid, syspath_typ*, byte* buffer );
void    LastCh_Bit7 ( char* name, Boolean setIt);
void    Console_Name( int term_id, char* consname );
char*   OS9exec_Name( void );
uint32_t Pipe_NReady ( pipechan_typ* p );

Boolean AbsPath      ( const char* pathname );
void    GetOS9Dev    ( const char* pathname, char* cmp_entry );
Boolean IsWhat       ( const char* pathname, Boolean isRoot );
Boolean IsRoot       ( const char* pathname );
Boolean IsRaw        ( const char* pathname );
Boolean VolInfo      ( const char* pathname, char* volname );
Boolean     OpenTDir ( const char* pathname, DIR** d );
dirent_typ* ReadTDir                       ( DIR*  d );
Boolean PathFound    ( const char* pathname );
Boolean FileFound    ( const char* pathname );
Boolean HostPathWithinConfiguredDevice( const char* hostpath );
Boolean HostPathDeviceName( const char* hostpath, char* nameOut );
Boolean IsHostDeviceRoot( const char* hostpath );
Boolean FindConfiguredDeviceRoot( const char* hostpath, char* rootOut );

void    CutUp              ( char* pathname, const char* prev );
void    EatBack            ( char* pathname );

os9err  FD_ID        ( const char* pathname, dirent_typ* dEnt,
                       uint32_t   *fdID,     dirtable_entry** mH );
os9err  FD_Name      ( uint32_t    fdID,     char* *pathnameP );

os9err  Flush_Dir    ( ushort cpid, ushort* pathP, const char* nmS );
os9err  Flush_Entry  ( ushort cpid,                const char* nmS );
void    Flush_FDCache( const char* pathname );

#ifdef win_unix
  os9err DirNthEntry       ( syspath_typ*, int n, dirent_typ** dEnt );
  os9err RemoveAppledouble ( syspath_typ* );
  void   seekD0            ( syspath_typ* );
  uint32_t DirSize         ( syspath_typ* );
#endif


int     stat_        ( const char* pathname, struct stat *buf );
Boolean DirName      ( const char* pathname, uint32_t fdsect, char* result, Boolean useInodes );
uint32_t My_FD       ( const char* pathname );
void    MakeOS9Path  (       char* pathname );

Boolean SamePathBegin( const char* pathname, const char* cmp );
Boolean InstalledDev ( const char* os9path,  
                       const char* curpath,  Boolean fullsearch, ushort *cdv );
Boolean IsDesc       ( const char* dvn,      mod_dev** mod, char** p );
Boolean SCSI_Device  ( const char* os9path,
                       short  *scsiAdapt, short  *scsiBus, int  *scsiId,    short   *scsiLUN,
                       ushort *scsiSsize, ushort *scsiSas, byte *scsiPDTyp, ptype_typ *typeP );

// #ifdef RAM_SUPPORT
  Boolean RAM_Device ( const char* os9path );
// #endif

#if defined windows32 || defined UNIX
  os9err AdjustPath ( const char* pathname, char* adname, Boolean creFile );
#endif

#if defined win_linux || defined UNIX
  void   GetEntry( dirent_typ* dEnt, char* name, Boolean do_2e_conv );
#endif

Boolean RBF_ImgSize( long size );
/* Returns true, if it is a valid RBF Image size */

#ifdef MACOS9
  os9err RBF_Rsc   ( FSSpec  *fs );
  os9err GetRBFName( char* os9path, ushort mode, Boolean *isFolder, 
                     FSSpec  *fs, FSSpec *afs );
#elif defined UNIX
  os9err GetRBFName( char* os9path, ushort mdoe, Boolean *isFolder, 
                     char* rbfname );
#endif


char*    Mod_TypeStr(    mod_exec* mod  );
char*      PStateStr( process_typ* cp   );
char*        TypeStr(   ptype_typ  type );
char*    spP_TypeStr( syspath_typ* );
ptype_typ IO_Type   ( ushort  pid, char* os9path, ushort mode );


/* eof */
