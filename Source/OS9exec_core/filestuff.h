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
 *    Revision 1.15  2006/11/18 09:56:54  bfo
 *    "MountDev" params <sctSize>, <cluSize> added
 *
 *    Revision 1.14  2006/11/12 13:30:30  bfo
 *    "ReadFD" visible from outside
 *
 *    Revision 1.13  2006/02/19 15:42:23  bfo
 *    Header changed to 2006
 *
 *    Revision 1.12  2005/05/13 17:24:22  bfo
 *    Use <imgMode>
 *
 *    Revision 1.11  2005/04/15 11:48:38  bfo
 *    Reduced size of RBF images is supported now
 *
 *    Revision 1.10  2004/11/27 12:12:07  bfo
 *    "MyInetAddr" no longer implemented here
 *
 *    Revision 1.9  2004/11/20 11:44:07  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.8  2003/07/31 14:40:04  bfo
 *    Additional parameter for sectorsize
 *
 *    Revision 1.7  2003/04/20 22:56:37  bfo
 *    'MyInetAddr" supports now 2 DNS addresses
 *
 *
 */


/* main initialisation routine */
void init_fmgrs();

/* I/O routines */
void   show_files        ( ushort pid );
os9err parsepathext      ( ushort pid, char** inp, char* out, Boolean exedir, Boolean *ispath );
os9err parsepath         ( ushort pid, char** inp, char* out, Boolean exedir );
void   TwoCharDev        ( char* p, char** p3, char* tmp );

/* Generic */
os9err pUnimp            ( ushort pid, syspath_typ* );
os9err pUnimpOk          ( ushort pid, syspath_typ* );
os9err pBadMode          ( ushort pid, syspath_typ* );
os9err pNop              ( ushort pid, syspath_typ* );
os9err pNoModule         ( ushort pid, syspath_typ* );

/* SCF-like */
os9err pSCFnam           ( ushort pid, syspath_typ*, char* volname );


/* unified I/O */
os9err syspath_new                   ( ushort *sp, ptype_typ type );
os9err usrpath_new       ( ushort pid, ushort *up, ptype_typ type );
os9err syspath_open      ( ushort pid, ushort *sp, ptype_typ type, const char* pathname, ushort mode );
os9err usrpath_open      ( ushort pid, ushort *up, ptype_typ type, const char* pathname, ushort mode );
os9err syspath_close     ( ushort pid, ushort  sp );
os9err usrpath_close     ( ushort pid, ushort  up );
os9err usrpath_link      ( ushort pid, ushort  sp, const char* ident );
os9err syspath_write     ( ushort pid, ushort  sp, uint32_t *len, void* buffer, Boolean wrln );
os9err usrpath_write     ( ushort pid, ushort  up, uint32_t *len, void* buffer, Boolean wrln );

void usrpath_printf      ( ushort pid, ushort  up, const char* format, ... );
void     upo_printf                              ( const char* format, ... );
void    upho_printf                              ( const char* format, ... );
void     upe_printf                              ( const char* format, ... );
void    uphe_printf                              ( const char* format, ... );
void    main_printf                              ( const char* format, ... );
void copyright();

os9err syspath_read      ( ushort pid, ushort  sp, uint32_t *len, void* buffer, Boolean rdln );
os9err usrpath_read      ( ushort pid, ushort  up, uint32_t *len, void* buffer, Boolean rdln );
os9err syspath_seek      ( ushort pid, ushort  sp, uint32_t  pos );
os9err usrpath_seek      ( ushort pid, ushort  up, uint32_t  pos );

os9err syspath_getstat   ( ushort pid, ushort  sp, ushort func, ulong *a0,
                                                                uint32_t *d0, uint32_t *d1, uint32_t *d2, uint32_t *d3 );
os9err syspath_gs_size   ( ushort pid, ushort  sp, uint32_t *size );
os9err syspath_gs_devnm  ( ushort pid, ushort  sp, char*  name );
os9err syspath_gs_ready  ( ushort pid, ushort  sp, uint32_t *cnt  );

os9err usrpath_getstat   ( ushort pid, ushort  up, ushort func, ulong *a0,
                                                                uint32_t *d0, uint32_t *d1, uint32_t *d2, uint32_t *d3 );

os9err syspath_setstat   ( ushort pid, ushort  sp, ushort func, ulong *a0, ulong *a1,
                                                                uint32_t *d0, uint32_t *d1, uint32_t *d2, uint32_t *d3 );
os9err usrpath_setstat   ( ushort pid, ushort  up, ushort func, ulong *a0, ulong *a1,
                                                                uint32_t *d0, uint32_t *d1, uint32_t *d2, uint32_t *d3 );


os9err get_locations     ( ushort pid, ptype_typ type, const char* pathname,
                                                       Boolean doCreate, Boolean *asDir,
                                                       uint32_t *fdP, uint32_t *dfdP, uint32_t *dcpP, uint32_t *sSct );
                                                       
os9err change_dir        ( ushort pid, ptype_typ type, const char* pathname, ushort mode );
os9err delete_file       ( ushort pid, ptype_typ type, const char* pathname, ushort mode );
os9err make_dir          ( ushort pid, ptype_typ type, const char* pathname, ushort mode );


void init_syspaths       ( void );
void init_usrpaths       ( ushort pid );
char* fType              ( syspath_typ*, char* field );
syspath_typ* get_syspath ( ushort pid, ushort  sp);
syspath_typ* get_syspathd( ushort pid, ushort  sp);
void close_usrpaths      ( ushort pid );
void close_syspaths      ( void );




/* external consio support routines */
Boolean ConsGetc        ( char *c );
void    ConsPutc        ( char  c );
void    ConsPutcEdit    ( char  c, Boolean alf, char eorch );

/* baud pacing (docs/superpowers/specs/2026-07-10-baud-fifo-pacing-design.md) */
void  baud_drain_due   ( void );          /* pop+display whatever is ready right now, cheap no-op if nothing queued */
void  baud_flush_device( short term_id ); /* discard queued output for one device (Ctrl-C/E, kill) */

/* external RBF    support routines */
os9err  MountDev        ( ushort pid, char* name, char* mnt_dev, char* devCopy,
                          short  adapt, ushort scsibus, 
                          short scsiID, ushort scsiLUN, 
                          int  ramSize, int sctSize, int cluSize, Boolean wProtect, int imgMode );
                             
os9err  int_mount       ( ushort pid, int argc, char **argv );
os9err  int_unmount     ( ushort pid, int argc, char **argv );
void    Disp_RBF_Devs   ( Boolean statistic );
os9err  ReadFD          ( syspath_typ* spP );


/* external pipe   support routines */
os9err           getPipe( ushort pid, syspath_typ*, uint32_t buffsize );
os9err       releasePipe( ushort pid, syspath_typ* );
syspath_typ* crossedPath( ushort pid, syspath_typ* );
os9err    ConnectPTY_TTY( ushort pid, syspath_typ* );
void       PutCharsToTTY( ushort pid, syspath_typ*, uint32_t *lenP, char* buffer, Boolean wrln );
void    CheckInBufferTTY( ttydev_typ* mco );


/* external net    support routines */
/* os9err        MyInetAddr( ulong *inetAddr, ulong *dns1Addr,
                                           ulong *dns2Addr, char* domainName ); */
os9err             pNask( ushort pid, syspath_typ* );

/* eof */

