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
 *    Revision 1.4  2003/05/17 11:13:12  bfo
 *    (CVS header included)
 *
 *
 */



/* OS-9 system call routines */
os9err OS9_F_Load   (regs_type *rp, ushort cpid);
os9err OS9_F_Link   (regs_type *rp, ushort cpid);
os9err OS9_F_UnLink (regs_type *rp, ushort cpid);
os9err OS9_F_Fork   (regs_type *rp, ushort cpid);
os9err OS9_F_DFork  (regs_type *rp, ushort cpid);
os9err OS9_F_DExec  (regs_type *rp, ushort cpid);
os9err OS9_F_DExit  (regs_type *rp, ushort cpid);
void   save_debug_regs( ushort pid ); /* write child registers into debugger's frame buffer (dbg_regsave_addr[pid]) */
os9err OS9_F_Wait   (regs_type *rp, ushort cpid);
os9err OS9_F_Chain  (regs_type *rp, ushort cpid);
os9err OS9_F_Exit   (regs_type *rp, ushort cpid);
os9err OS9_F_Send   (regs_type *rp, ushort cpid);
os9err OS9_F_Icpt   (regs_type *rp, ushort cpid);
os9err OS9_F_Sleep  (regs_type *rp, ushort cpid);
os9err OS9_F_ID     (regs_type *rp, ushort cpid);
os9err OS9_F_SPrior (regs_type *rp, ushort cpid);
os9err OS9_F_STrap  (regs_type *rp, ushort cpid);
os9err OS9_F_PErr   (regs_type *rp, ushort cpid);
os9err OS9_F_PrsNam (regs_type *rp, ushort cpid);
os9err OS9_F_CmpNam (regs_type *rp, ushort cpid);
os9err OS9_F_Time   (regs_type *rp, ushort cpid);
os9err OS9_F_STime  (regs_type *rp, ushort cpid);
os9err OS9_F_CRC    (regs_type *rp, ushort cpid);
os9err OS9_F_GPrDsc (regs_type *rp, ushort cpid);
os9err OS9_F_GBlkMp (regs_type *rp, ushort cpid);
os9err OS9_F_GModDr (regs_type *rp, ushort cpid);
os9err OS9_F_CpyMem (regs_type *rp, ushort cpid);
os9err OS9_F_SUser  (regs_type *rp, ushort cpid);
os9err OS9_F_UnLoad (regs_type *rp, ushort cpid);
os9err OS9_F_RTE    (regs_type *rp, ushort cpid);
os9err OS9_F_GPrDBT (regs_type *rp, ushort cpid);
os9err OS9_F_Julian (regs_type *rp, ushort cpid);
os9err OS9_F_TLink  (regs_type *rp, ushort cpid);
os9err OS9_F_DatMod (regs_type *rp, ushort cpid);
os9err OS9_F_SetCRC (regs_type *rp, ushort cpid);
os9err OS9_F_SetSys (regs_type *rp, ushort cpid);
os9err OS9_F_SRqMem (regs_type *rp, ushort cpid);
os9err OS9_F_SRtMem (regs_type *rp, ushort cpid);
os9err OS9_F_SSvc   (regs_type *rp, ushort cpid);
os9err OS9_F_Permit (regs_type *rp, ushort cpid);
os9err OS9_F_SysDbg (regs_type *rp, ushort cpid);
os9err OS9_F_Gregor (regs_type *rp, ushort cpid);
os9err OS9_F_Alarm  (regs_type *rp, ushort cpid);
os9err OS9_F_Sigmask(regs_type *rp, ushort cpid);
os9err OS9_F_Panic  (regs_type *rp, ushort cpid);
os9err OS9_F_Event  (regs_type *rp, ushort cpid);

os9err OS9_F_Dummy  (regs_type *rp, ushort cpid);
os9err OS9_F_SDummy (regs_type *rp, ushort cpid);
os9err OS9_F_UnImp  (regs_type *rp, ushort cpid);
os9err OS9_F_SUnImp (regs_type *rp, ushort cpid);
/* eof */

