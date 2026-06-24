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
 *    Revision 1.7  2005/04/15 11:14:53  bfo
 *    SCSI params are combined in one struct now
 *
 *    Revision 1.6  2004/11/20 11:44:07  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.5  2003/05/17 11:13:25  bfo
 *    (CVS header included)
 *
 *
 */

/* Low level SCSI interface */

os9err Set_SSize   ( scsi_dev* scsi,                    uint32_t  sctSize );
os9err Get_SSize   ( scsi_dev* scsi,                    uint32_t *sctSize );
os9err ReadCapacity( scsi_dev* scsi, uint32_t *totScts, uint32_t *sctSize );
os9err Get_DSize   ( scsi_dev* scsi, uint32_t *totScts );

os9err ReadFromSCSI( scsi_dev* scsi, uint32_t sectorNr, uint32_t nSectors, uint32_t len, byte* buffer );
os9err WriteToSCSI ( scsi_dev* scsi, uint32_t sectorNr, uint32_t nSectors, uint32_t len, byte* buffer );

#ifdef windows32
void scsiadaptor_help (void);
void scsi_finddefaults(void);
#endif
/* eof */

