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
 *    Revision 1.5  2004/11/20 11:44:06  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.4  2003/05/17 11:03:34  bfo
 *    (CVS header included)
 *
 *
 */


/* OS-9 alarms */
void   init_alarms( void );
void   A_Insert   ( alarm_typ* aa );
void   A_Remove   ( alarm_typ* aa );
os9err A_Make     ( ushort pid, uint32_t *aId, ushort aCode, uint32_t aTicks, Boolean cyclic );
void   A_Kill     ( ushort pid );

os9err Alarm( ushort pid, uint32_t *aId, short aFunc, ushort aCode, uint32_t aTime, uint32_t aDate );

/* eof */
