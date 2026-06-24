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
 *    Revision 1.12  2007/03/24 13:04:33  bfo
 *    "DoWait" is visible from outside
 *
 *    Revision 1.11  2006/07/29 08:54:19  bfo
 *    <allowedIntUtil> is now the pid of the int util (0 for none)
 *
 *    Revision 1.10  2006/07/21 07:29:54  bfo
 *    Up to date
 *
 *    Revision 1.9  2006/07/10 09:58:27  bfo
 *    <allowIntUtil> param for "do_arbitrate"
 *
 *    Revision 1.8  2006/07/06 22:57:10  MG
 *    function 'is_super' added (by Marin Gregorie)
 *
 *    Revision 1.7  2006/02/19 16:38:37  bfo
 *    thread support added
 *
 *    Revision 1.6  2005/06/30 11:56:37  bfo
 *    sig_mask adaption
 *
 *    Revision 1.5  2004/11/20 11:44:08  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.4  2003/05/17 11:20:32  bfo
 *    (CVS header included)
 *
 *
 */



/* process routines */
void   show_processes(void);
void   init_processes(void);
void   kill_processes(void);
int    is_super(ushort pid);

os9err  new_process( ushort parentid, ushort *newpid, ushort numpaths );
os9err kill_process( ushort pid );
os9err send_signal ( ushort pid, ushort signal );
os9err sig_mask    ( ushort pid, int    level  );

void do_arbitrate( ushort allowedIntUtil );
void DoWait( void );
void AssignNewChild( ushort parentid, ushort pid );

void stop_os9exec(void);
void lw_pid( ttydev_typ* mco );

os9err setprior( ushort pid, ushort newprior);
os9err prepFork( ushort pid,      char* mpath,    ushort mid,
                 byte*  paramptr, uint32_t paramsiz, uint32_t memplus,
                 ushort numpaths, ushort grp, ushort usr, ushort prior );

/* eof */
