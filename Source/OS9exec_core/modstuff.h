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
 *    Revision 1.8  2006/02/19 16:14:12  bfo
 *    void* for MoveBlk
 *
 *    Revision 1.7  2004/11/20 11:44:08  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.6  2002/10/02 19:10:21  bfo
 *    Update_MDir implementation at "modstuff" (taken partly from OS9_F_GModDr)
 *
 *
 */



/* Memory modules */

mod_exec* os9mod      ( int k );
char*     Mod_Name    ( mod_exec* mod );
void      Update_MDir ( void );

void      MoveBlk     ( void* dst, void* src, ulong size );
Boolean   SameBlk     ( byte* a,   byte* b,   ulong size );
uint32_t  DatMod_Size ( uint32_t namsize, uint32_t datsize );
void      FillTemplate( mod_exec* m, short access, short tylan, short attrev );

int   NextFreeModuleId( char* name );
void      show_modules( char* cmp  );
void      init_modules( void );
void   release_module ( ushort mid, Boolean modOK );

mod_exec*  get_module_ptr( int   mid     );

Boolean RangeInAnyModule( void* p, ulong cnt ); // does [p,p+cnt) lie in a loaded module?
int        get_mid       ( void *modptr );

int       find_mod_id             ( const char* name );
int       link_mod_id                   ( char* name );
os9err    load_module ( ushort pid,       char* name, ushort *midP, Boolean exedir );
os9err    link_module ( ushort pid, const char* name, ushort *midP );
os9err    link_load   ( ushort pid,       char* name, ushort *midP );
os9err    load_OS9Boot( ushort pid );
void    unlink_module ( ushort mid );
void      free_modules();


void init_exceptions  ( ushort pid );
void init_traphandlers( ushort pid );

ushort calc_parity( ushort*   p, ushort numwords );
uint32_t calc_crc ( byte*     p, uint32_t size, uint32_t accum );
void    mod_crc   ( mod_exec* m );

os9err prepData(ushort pid, mod_exec* theModule, uint32_t memplus, uint32_t *msiz, byte **mp);

os9err install_traphandler(ushort pid, ushort trapidx, char *mpath, uint32_t addmem, traphandler_typ **traphandler);
os9err release_traphandler(ushort pid, ushort trapidx);
void   unlink_traphandlers(ushort pid);

/* eof */
