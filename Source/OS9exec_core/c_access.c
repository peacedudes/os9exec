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
 *    Revision 1.8  2007/04/19 21:00:56  bfo
 *    Version changed to V3.38
 *
 *    Revision 1.7  2007/03/11 17:17:37  bfo
 *    Version changed to V3.37
 *
 *    Revision 1.6  2007/02/22 22:58:28  bfo
 *    Version changed to V3.36
 *
 *    Revision 1.5  2007/01/29 23:19:36  bfo
 *    import "Resource.h"
 *
 *    Revision 1.4  2007/01/29 23:12:08  bfo
 *    trying to make it clean for MPW (again)
 *
 *    Revision 1.3  2007/01/29 22:48:29  bfo
 *    'Handle' def imported
 *
 *    Revision 1.2  2007/01/28 21:47:57  bfo
 *    - __GLOBDEF added / Boolean => int /
 *    - 'nativeinfo_typ' added
 *    - getversion/lVersion implemented here now
 *    - 'ustrcmp' implemented here now
 *
 *    Revision 1.1  2007/01/07 13:25:33  bfo
 *    *** NEW MODULE ADDED ***
 *
 *
 */


// Don't import all other things ...
#if defined macintosh && !defined __MACH__
  #define MACOS9
  #include <ToolUtils.h>
  #include <Resources.h>
#endif

#include "c_access.h"
#include <ctype.h>
#include <stdio.h>


// the callback structure reference
callback_typ* cbP;


// obtain os9exec version
void getversion( unsigned short *ver,
                 unsigned short *rev )
{
  #ifdef MACOS9
    Handle versH;
  #endif
	
  *ver= 0; 
  *rev= 0;
	
  #ifdef MACOS9
    // obtain os9exec version
        versH= GetResource( 'vers',2 );
    if (versH!=NULL) {
      *ver= *( (byte*)*versH+0 );
      *rev= *( (byte*)*versH+1 );
      ReleaseResource( versH );
    } // if
  #else
    // simply hardwired for all other platforms
    *ver=    3;
    *rev= 0x40; /* V3.40 — arm64 port */
  #endif
} // getversion


// obtain os9exec version in long format
long lVersion()
{
  unsigned short ver,  rev;
  getversion  ( &ver, &rev );
  return   ( 256*ver + rev )*256*256;
} // lVersion



// -------------------------------------------------------------------------------------------------------
// copy the block with <size> form <src> to <dst
// forward and backward mode supported (in case of overlapping structures
// <size> is ulong (the header's type) rather than a spelled-out `unsigned long`:
// on Windows (LLP64) those are different widths, so writing it out here compiled
// this definition against a 32-bit size while every caller passed a 64-bit one.
//
// The reverse path walks the pointers directly. It used to route them through an
// `(unsigned long)` cast to do the arithmetic -- which is only pointer-width on
// LP64. On Windows it silently discarded the top half of both host addresses and
// then dereferenced the wreckage; mingw-w64 says so out loud ("cast from pointer
// to integer of different size"). Reached from winfiles.c's Conv_to_2e and from
// the free-list shuffles in memstuff.c, both of which copy with src<dst. Plain
// pointer arithmetic needs no cast at all and is correct on every platform.
void MoveBlk( void* dst, void* src, ulong size )
{
  ulong          n;
  unsigned char* s;
  unsigned char* d;

  /* src/dst often come from FROM68K(a guest register); a guest passing address
     0 yields NULL here.  A zero-length move is a no-op anyway, and moving to/from
     a NULL host pointer would crash the emulator, so skip it -- callers that pass
     a real length with a null pointer get an empty result instead of a host
     crash (F$CpyMem, F$GModDr, and the F$Fork/F$Chain parameter copy all rely on
     this; for a fork with no parameters a1 is legitimately 0 with size 0). */
  if (size==0 || dst==NULL || src==NULL) return;

  if (src>=dst) {    // condition for forward/backward copy
    s= (unsigned char*)src;   // normal forward copy
    d= (unsigned char*)dst;
    for ( n=0; n<size; n++ ) { *d= *s; s++; d++; }
  }
  else {             // reverse ordered copy
    s= (unsigned char*)src + size-1;
    d= (unsigned char*)dst + size-1;
    for ( n=0; n<size; n++ ) { *d= *s; s--; d--; }
  } // if
} // MoveBlk



// case insensitive version of strcmp
//  Input  : <s1>, <s2> = strings to be compared
//  Result : -1: s1<s2,
//            0: s1=s2,
//            1: s1>s2
int ustrcmp( const char *s1,const char *s2 )
{
  int  diff;
  char c;

  do {
                   c=           *(s1++);
    diff= toupper( c )-toupper( *(s2++) );

    if (diff!=0) return diff>0 ? 1 : -1;
  } while ( c!='\0' );

  return 0; // strings are equal
} // ustrcmp



/* eof */
