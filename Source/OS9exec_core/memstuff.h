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
 *    Revision 1.12  2006/06/13 22:23:05  bfo
 *    StrToInt/StrToShort added
 *
 *    Revision 1.11  2006/06/12 10:49:33  bfo
 *    BOOLEAN def added for GNUC
 *
 *    Revision 1.10  2006/03/12 19:31:22  bfo
 *    RealToStrN added
 *
 *    Revision 1.9  2006/02/19 16:33:20  bfo
 *    Some PtoC routines are implemented here now
 *
 *    Revision 1.8  2004/11/20 11:44:07  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.7  2003/05/21 20:31:39  bfo
 *    Allocate 512k blocks / Additional parameter <mem_fulldisp> for "show_mem"
 *
 *    Revision 1.6  2003/05/17 10:38:13  bfo
 *    'show_mem' with <mem_unused> parameter
 *
 *    Revision 1.5  2002/10/27 23:12:39  bfo
 *    get_mem/release_mem no longer with param <mac_asHandle>
 *
 *
 */


// the mem alloc table
extern memblock_typ memtable[ MAX_MEMALLOC ];

// memory blocks for all processes
extern pmem_typ pmem[ MAXPROCESSES ];

        
#ifdef REUSE_MEM
  typedef struct {
    memblock_typ  f[ MAX_MEMALLOC ];
    int           freeN;   // number of free segments
    ulong         freeMem; // total free memory
  } free_typ;
        
  // the free info
  extern free_typ freeinfo;
#endif


#if defined __cplusplus
  extern "C" {
#endif

void  init_all_mem( void );
void      init_mem( ushort pid );

ulong      max_mem( void );
void      show_mem( ushort pid, Boolean mem_unused, Boolean mem_fulldisp );
void   show_unused( void );

void*      get_mem( ulong  memsz   );
void   release_mem( void*  membase );
void      free_mem( ushort pid     ); // release all memory of process

void*   os9malloc ( ushort pid,                ulong memsz );
os9err  os9free   ( ushort pid, void* membase, ulong memsz );

Boolean RangeInProcMem( ushort pid, void* p, ulong cnt ); // may pid write [p,p+cnt)?


/* eof */
