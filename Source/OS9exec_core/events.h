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
 *    Revision 1.4  2003/05/17 11:04:04  bfo
 *    (CVS header included)
 *
 *
 */


/* OS-9 events */
/* the definitions */
#define Ev_Link     0x00
#define Ev_UnLnk    0x01
#define Ev_Creat    0x02
#define Ev_Delet    0x03
#define Ev_Wait     0x04
#define Ev_WaitR    0x05
#define Ev_Read     0x06
#define Ev_Info     0x07
#define Ev_Signl    0x08
#define Ev_Pulse    0x09
#define Ev_Set      0x0A
#define Ev_SetR     0x0B

/* evWait()-internal: value not yet in the requested range, distinct from a
   genuinely invalid event ID (E_EVNTID). Never returned to guest code -- the
   OS9_F_Event dispatcher (fcalls.c) uses it only to decide whether to park
   the process and retry. Chosen well outside the real 0-255 OS-9 error-code
   range (os9errno.h) so it can never collide with one. */
#define EV_NOTYET   0xFFFF



/* the routines */
void   init_events(void);

os9err evLink ( char* evName,                                           uint32_t *evId );
os9err evUnLnk                                                        ( uint32_t  evId );
os9err evCreat( char* evName, int evValue, short wInc, short sInc,     uint32_t *evId );
os9err evDelet( char* evName );
os9err evWait ( uint32_t evId, int minV, int maxV, int  *evValue );
os9err evSignl( uint32_t evId );
os9err evSet  ( uint32_t evId, int evValue,         int *prvValue );

/* eof */
