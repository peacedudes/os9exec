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
 *    Revision 1.30  2007/01/28 22:07:07  bfo
 *    Support for linking the native resource files
 *
 *    Revision 1.29  2007/01/07 13:56:32  bfo
 *    "MoveBlk" is now defined at "c_access.c"
 *
 *    Revision 1.28  2007/01/04 20:41:26  bfo
 *    Partly unused label 'modulefound' hidden
 *
 *    Revision 1.27  2006/11/13 14:55:12  bfo
 *    type adaptions for GCC
 *
 *    Revision 1.26  2006/11/12 13:25:31  bfo
 *    "load_OS9Boot" implemented /
 *    "load_module_local" extended for the OS9Boot requirements
 *
 *    Revision 1.25  2006/08/29 22:38:54  bfo
 *    isintcommand adaptions
 *
 *    Revision 1.24  2006/08/26 23:48:55  bfo
 *    Formatting beautified
 *
 *    Revision 1.23  2006/06/08 08:15:04  bfo
 *    Eliminate causes of signedness warnings with gcc 4.0
 *
 *    Revision 1.22  2006/06/01 18:05:57  bfo
 *    differences in signedness (for gcc4.0) corrected
 *
 *    Revision 1.21  2006/02/19 16:14:12  bfo
 *    void* for MoveBlk
 *
 *    Revision 1.20  2005/06/30 16:33:38  bfo
 *    Up to date
 *
 *    Revision 1.19  2005/06/30 11:48:59  bfo
 *    68k adaption (NET_SUPPORT)
 *
 *    Revision 1.18  2005/01/22 16:11:06  bfo
 *    Renamed to ifdef MACOS9
 *
 *    Revision 1.17  2004/12/04 00:01:25  bfo
 *    MacOSX MACH adaptions
 *
 *    Revision 1.16  2004/11/27 12:14:37  bfo
 *    "net_platform" used
 *
 *    Revision 1.15  2004/11/20 11:44:07  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.14  2004/09/15 19:48:44  bfo
 *    "Flush68kCodeRange" now activated for ALL module loads.
 *
 *    Revision 1.13  2003/07/08 15:42:24  bfo
 *    /socket and /le0 are built-in modules now
 *
 *    Revision 1.12  2003/05/26 08:25:49  bfo
 *    "double" big/little endian problem module sizes fixed
 *
 *    Revision 1.11  2003/04/20 23:08:45  bfo
 *    "inetdb" adaptions and other manipulations
 *
 *    Revision 1.10  2003/04/12 21:52:26  bfo
 *    "inetdb" will be adapted automatically now
 *
 *    Revision 1.9  2002/11/06 19:59:48  bfo
 *    zero is zero for os9_word/os9_long
 *
 *    Revision 1.8  2002/10/27 23:20:14  bfo
 *    module system on Mac no longer implemented with handles
 *
 *    Revision 1.7  2002/10/02 19:03:13  bfo
 *    Update_MDir implementation at "modstuff" (taken partly from OS9_F_GModDr)
 *
 *    Revision 1.6  2002/09/11 17:20:30  bfo
 *    Make an internal copy of the built-in "init" module: ref to var, not to const
 *
 *
 */


#include "os9exec_incl.h"
_Static_assert(sizeof(((mod_exec*)0)->_mh._msize)==4, "modhcom._msize must be 32-bit for OS-9 binary layout");

/* additional memory for all processes */
ulong memplusall;


/* Memory modules */
/* ============== */




/* OS9exec builtin module */
/* the assembler source:  */

// TYPELANG EQU (Prgrm<<8)+Objct
// ATTREV EQU (ReEnt<<8)+0
// 
//  PSECT OS9exec,TYPELANG,ATTREV+0,1,512,DUMMYENT
// 
// 
//  VSECT
// * none
//  ENDS
//  
// USETEXT: DC.B "Dummy program as OS9exec/nt placeholder",C$CR
// ULEN: EQU *-USETEXT
// 
// * entry point
// DUMMYENT:
//  MOVEQ #2,D0
//  MOVEQ #ULEN,D1
//  LEA USETEXT(PC),A0
//  OS9 I$WritLn
//  MOVE.W #0,D1
//  OS9 F$Exit
// 
//  ENDS
 
/* Each builtin module image below is cast to mod_exec* (see the *_ptr
 * definitions after each array) and the emulator then reads its 32-bit
 * big-endian header fields straight off that pointer -- for socket/le0 the array
 * IS the live module (theModuleP points at it), not a copy. As a plain
 * `const byte[]` the array only carries alignment 1, and the linker packs these
 * back-to-back: confirmed with objdump on the native ARM64 build, Init_mod
 * landed at .rodata+0xa2, i.e. 2-aligned, so every 4-byte header read through
 * Init_ptr was a misaligned access. Give them the alignment mod_exec actually
 * requires. Surfaced by clang's -Wcast-align; same undefined-behaviour family as
 * the GET_OS9L/SET_OS9L fix in os9_ll.h. */
#if defined __GNUC__
  #define MODALIGN __attribute__((aligned(4)))
#else
  #define MODALIGN /* MPW/CodeWarrior: 68k/PPC aligned these by default anyway */
#endif

/* OS9exec builtin module, defined as constant array */
const byte OS9exec_mod[] MODALIGN = {
    0x4a,0xfc,0x00,0x01,0x00,0x00,0x00,0xa2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x48,  //  J|.....".......H
    0x05,0x55,0x01,0x01,0x80,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  //  .U..............
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x31,0xbd,  //  ..............1=
    0x00,0x00,0x00,0x7a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,  //  ...z............
    0x00,0x00,0x00,0x8e,0x00,0x00,0x00,0x96,0x4f,0x53,0x39,0x65,0x78,0x65,0x63,0x00,  //  ........OS9exec.
    0x00,0x00,0x44,0x75,0x6d,0x6d,0x79,0x20,0x70,0x72,0x6f,0x67,0x72,0x61,0x6d,0x20,  //  ..Dummy program 
    0x61,0x73,0x20,0x4f,0x53,0x39,0x65,0x78,0x65,0x63,0x2f,0x6e,0x74,0x20,0x70,0x6c,  //  as OS9exec/nt pl 
    0x61,0x63,0x65,0x68,0x6f,0x6c,0x64,0x65,0x72,0x0d,0x70,0x02,0x72,0x28,0x41,0xfa,  //  aceholder.p.r(Az
    0xff,0xd2,0x4e,0x40,0x00,0x8c,0x32,0x3c,0x00,0x00,0x4e,0x40,0x00,0x06,0x00,0x00,  //  .RN@..2<..N@....
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x5d,  //  ...............]
    0xc9,0x72                                                                         //  lr
};
mod_exec* OS9exec_ptr= (mod_exec*)OS9exec_mod;   /* ptr to OS9exec module */

#define sizeof_OS9exec_mod 162
/* The size constant is hand-written and the array beside it is 100+ lines of
   hex; nothing tied the two together. A constant left behind by an edit
   would hand F$Link a length that does not match the module. */
_Static_assert( sizeof(OS9exec_mod)==sizeof_OS9exec_mod,
                "sizeof_OS9exec_mod does not match the array" );

/* The same bytes, copied into the 68k arena at startup, as the module F$Link
   answers with for an INTERNAL command (fcalls.c). The array above lives in
   host memory, which has no 68k address, so it cannot be handed to a guest. */
byte* intcmd_stub= NULL;

void init_intcmd_stub( void )
{
    if (intcmd_stub!=NULL) return;
    intcmd_stub= (byte*)get_mem( sizeof_OS9exec_mod );
    if (intcmd_stub!=NULL) memcpy( intcmd_stub, OS9exec_mod, sizeof_OS9exec_mod );
} /* init_intcmd_stub */
    


/* "init" builtin module, defined as constant array */
const byte Init_mod[] MODALIGN = {
    0x4a,0xfc,0x00,0x01,0x00,0x00,0x01,0x6e,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x64,  //  J|.....n.......d
    0x05,0x55,0x0c,0x00,0x80,0x00,0x00,0x1d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  //  .U..............
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3c,0x42,  //  ..............<B
    0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0xad,0x00,0xa7,  //  .....@.......-.'
    0x00,0xaf,0x00,0xb3,0x00,0xbf,0x00,0xb9,0x00,0x02,0x00,0x00,0x4d,0x61,0x63,0x00,  //  ./.3.?.9....Mac.
    0x00,0x84,0x00,0x01,0x09,0xc8,0x01,0x02,0x22,0x00,0x00,0x95,0x00,0x80,0x00,0x00,  //  .....H..".......
    0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x8c,0x01,0x00,0x04,0x00,0x00,0x00,  //  ................
    0x00,0x00,0x00,0x00,0x01,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  //  .....@..........
    0x00,0x00,0x00,0x00,0x41,0x70,0x70,0x6c,0x65,0x20,0x4d,0x61,0x63,0x69,0x6e,0x74,  //  ....Apple Macint
    0x6f,0x73,0x68,0x00,0x00,0x4f,0x53,0x2d,0x39,0x2f,0x36,0x38,0x6b,0x20,0x45,0x6d,  //  osh..OS-9/68k Em
    0x75,0x6c,0x61,0x74,0x6f,0x72,0x00,0x73,0x79,0x73,0x67,0x6f,0x00,0x0d,0x00,0x2f,  //  ulator.sysgo.../
    0x64,0x64,0x00,0x2f,0x74,0x65,0x72,0x6d,0x00,0x63,0x6c,0x6f,0x63,0x6b,0x00,0x73,  //  dd./term.clock.s
    0x79,0x73,0x63,0x61,0x63,0x68,0x65,0x20,0x46,0x50,0x55,0x20,0x46,0x50,0x53,0x50,  //  yscache FPU FPSP
    0x20,0x4f,0x53,0x39,0x50,0x32,0x20,0x4f,0x53,0x39,0x41,0x43,0x43,0x20,0x53,0x53,  //   OS9P2 OS9ACC SS
    0x4d,0x20,0x73,0x79,0x73,0x74,0x72,0x61,0x70,0x20,0x73,0x65,0x6c,0x65,0x63,0x74,  // M systrap select
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  //  ................
    0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x02,0x00,0x00,0x02,0x00,0x00,0x00,  //  ................
    0x01,0x24,0x00,0x00,0xc0,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // .$..@...........
    0x00,0x00,0x00,0x00,0x6f,0x6e,0x2d,0x62,0x6f,0x61,0x72,0x64,0x20,0x52,0x41,0x4d,  //  ....on-board RAM
    0x00,0x52,0x41,0x4d,0x20,0x6f,0x6e,0x20,0x56,0x4d,0x45,0x62,0x75,0x73,0x00,0x00,  //  .RAM on VMEbus..
    0x00,0x02,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00,  //  ........... ....
    0x02,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,  //  ...........@....
    0xff,0xff,0xff,0xff,0x69,0x6e,0x69,0x74,0x00,0x00,0x00,0xd0,0x58,0x33             //  ....init...PX3
};
mod_exec* Init_ptr= (mod_exec*)Init_mod;   /* ptr to Init module */
#define sizeof_Init_mod 366
/* The size constant is hand-written and the array beside it is 100+ lines of
   hex; nothing tied the two together. A constant left behind by an edit
   would hand F$Link a length that does not match the module. */
_Static_assert( sizeof(Init_mod)==sizeof_Init_mod,
                "sizeof_Init_mod does not match the array" );


/* "socket" builtin module, defined as constant array */
const byte Socket_mod[] MODALIGN = {
    0x4a,0xfc,0x00,0x01,0x00,0x00,0x01,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf3,  //  J|.............s
    0x05,0x55,0x0f,0x00,0x80,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  //  .U.....H........
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3e,0x74,  //  ..............>t
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0xeb,0x00,0xe3,0x00,0x00,0x00,0x00,  //  .........k.c....
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  //  ................
    0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x40,0x00,0x00,0x00,  //  ........@...@...
    0x00,0x80,0x00,0xc8,0x00,0xae,0x00,0x68,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x02,  //  ...H...h........
    0x00,0xfa,0x00,0x02,0x00,0x03,0x00,0x00,0x00,0x01,0x01,0x07,0x00,0x02,0x00,0x02,  //  .z..............
    0x00,0x11,0x00,0x01,0x01,0x03,0x00,0x02,0x00,0x01,0x00,0x06,0x00,0x01,0x00,0xff,  //  ................
    0x00,0x0c,0x00,0x03,0x00,0x00,0x00,0x01,0x00,0xd2,0x00,0x01,0x00,0x01,0x00,0x00,  //  .........R......
    0x00,0x01,0x00,0xdb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2f,0x6c,  //  ...[........../l
    0x6f,0x30,0x20,0x2f,0x6c,0x65,0x30,0x20,0x2f,0x6c,0x65,0x31,0x20,0x2f,0x73,0x6c,  //  o0 /le0 /le1 /sl
    0x31,0x20,0x2f,0x6c,0x62,0x70,0x30,0x00,0x4c,0x6f,0x63,0x61,0x6c,0x48,0x6f,0x73,  //  1 /lbp0.LocalHos
    0x74,0x00,0x61,0x66,0x5f,0x65,0x74,0x68,0x65,0x72,0x00,0x61,0x66,0x5f,0x75,0x6e,  //  t.af_ether.af_un
    0x69,0x78,0x00,0x73,0x6f,0x63,0x6b,0x64,0x76,0x72,0x00,0x73,0x6f,0x63,0x6b,0x6d,  //  ix.sockdvr.sockm
    0x61,0x6e,0x00,0x73,0x6f,0x63,0x6b,0x65,0x74,0x00,0x69,0x6e,0x65,0x74,0x00,0x74,  //  an.socket.inet.t
    0x63,0x70,0x00,0x75,0x64,0x70,0x00,0x69,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  //  cp.udp.ip.......
    0x00,0x00,0x00,0x00,0x00,0xb2,0xc9,0x87                                           //  .....2I.
};
mod_exec* Socket_ptr= (mod_exec*)Socket_mod;   /* ptr to socket module */
#define sizeof_Socket_mod 280
/* The size constant is hand-written and the array beside it is 100+ lines of
   hex; nothing tied the two together. A constant left behind by an edit
   would hand F$Link a length that does not match the module. */
_Static_assert( sizeof(Socket_mod)==sizeof_Socket_mod,
                "sizeof_Socket_mod does not match the array" );


/* "le0" builtin module, defined as constant array */
const byte Le0_mod[] MODALIGN = {
    0x4a,0xfc,0x00,0x01,0x00,0x00,0x00,0xbc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xab,  //  J|.....<.......+
    0x05,0x55,0x0f,0x01,0x80,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  //  .U.....H........
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0x89,  //  ..............?.
    0xfe,0xc6,0x80,0x00,0x47,0x03,0x05,0x03,0x00,0xa5,0x00,0x96,0x00,0x00,0x00,0x00,  //  ~F..G....%......
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x09,0x00,0x00,0x00,0x00,0xab,0x05,0xdc,  //  .............+..
    0x00,0x22,0x00,0x86,0xff,0xff,0xff,0x00,0x00,0x76,0x00,0x9f,0x00,0x00,0x00,0x00,  //  .".......v......
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  //  ................
    0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x98,0x58,0x6e,0xff,0x00,0x00,  //  ...........Xn...
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x98,0x58,0x6e,0x73,0x00,0x00,  //  ...........Xns..
    0x00,0x00,0x00,0x00,0x00,0x00,0x69,0x66,0x5f,0x69,0x6c,0x61,0x63,0x63,0x00,0x00,  //  ......if_ilacc..
    0x00,0x00,0x00,0x00,0x00,0x69,0x66,0x6d,0x61,0x6e,0x00,0x6c,0x65,0x30,0x00,0x00,  //  .....ifman.le0..
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf6,0x57,0x1c                       //  .........vW.
};
mod_exec* Le0_ptr= (mod_exec*)Le0_mod;   /* ptr to le0 module */
#define sizeof_Le0_mod 188
/* The size constant is hand-written and the array beside it is 100+ lines of
   hex; nothing tied the two together. A constant left behind by an edit
   would hand F$Link a length that does not match the module. */
_Static_assert( sizeof(Le0_mod)==sizeof_Le0_mod,
                "sizeof_Le0_mod does not match the array" );



/* offset definitions for module "inetdb" */
#define OFFS_HOSTS 0x34 /* "hosts" location */
#define OFFS_DNS   0x4c /*  DNS    location */
         



// NOTE: The handles are no longer supported (bfo) !!!
// /* get modulehandle/modulebase, indepentently of system */
// #ifdef macintosh
//   static Handle os9h( int k )
//   {   
//  	 if (k>=MAXMODULES) return NULL;
//       return os9modules[k].modulehandle;
//   } /* os9h */
//    
// #else
//   static mod_exec* os9h( int k )
//   {   
//       if (k>=MAXMODULES) return NULL;
//       return os9modules[k].modulebase;
//   } /* os9h */   

// #endif



// NOTE: The handles are no longer supported (bfo) !!!
// /* get module pointer, indepentently of system */
mod_exec* os9mod( int k )
{
//  if (os9h(k)==NULL) return NULL;
//  
//  #ifdef macintosh
//      return *(os9modules[k].modulehandle);
//  #else

    if (k<0 || k>=MAXMODULES) return NULL;
    return os9modules[k].modulebase;
    
//  #endif
} /* os9mod */     



char* Mod_Name( mod_exec* mod )
/* get the module's name */
{   return (char*)mod + os9_long(mod->_mh._mname);
} /* Mod_Name */



void Update_MDir( void )
/* Update the image of the module directory structure */
{
    ulong       b;
    int         k;
    mod_exec*   mod;
    module_typ* modK;
    Boolean     ok;
    mdir_entry* en;
    
    for (k=0; k<MAXMODULES; k++) {
        en= &mdirField[k];

                 mod= os9mod(k);
                modK= &os9modules[k];
            ok= (mod!=NULL) && !modK->isBuiltIn; /* built-ins are not real 68k modules */
        if (ok) {
            b = 0;
            set_hiword( b, (ushort)modK->linkcount );

            en->m1  = os9_long( TO68K(mod) );
            /* The group pointer. A "module group" is several modules sharing one
               allocation, so that unlinking frees the block once -- see F$VModul,
               which takes the group base in d0. os9exec gives every module its own
               get_mem, including each module of a multi-module file (the load loop
               below), so each one IS its own group and pointing m2 at itself is the
               truthful answer rather than a placeholder. */
            en->m2  = en->m1;
            en->size= mod->_mh._msize; /* big/little endian is already correct !!! */
            en->lnk = os9_long( b );
        }
        else {
            en->m1  = 0;
            en->m2  = 0;
            en->size= 0;
            en->lnk = 0;
        }
    } /* for */
}


Boolean RangeInAnyModule( void* p, ulong cnt )
/* True if [p, p+cnt) lies within any loaded 68k module's memory. F$CpyMem lets
 * a user-state caller WRITE here as well as into its own data: modules live in
 * RAM, os9exec cannot tell a write-protected module from a writable one (no ROM,
 * no immutable attribute), and shared DATA MODULES -- the sanctioned way to pass
 * data between processes -- must stay writable even for a process that did not
 * create them. Self-modifying a shared code module is a bad idea but not made
 * illegal here, deliberately (owner's call). Built-in modules are skipped: their
 * storage is os9exec's own C image, never the 68k arena, so an arena destination
 * cannot fall inside one. Overflow-safe (cnt<=size-off) like RANGE_IN_ARENA. */
{
    byte* b= (byte*)p;
    int   k;

    if (cnt==0) return true;

    for (k=0; k<MAXMODULES; k++) {
        mod_exec* mod= os9mod( k );
        byte*     base;
        ulong     size;
        if (mod==NULL || os9modules[k].isBuiltIn) continue; /* real 68k modules only */
        base= (byte*)mod;
        size= os9_long( mod->_mh._msize );
        if (b>=base && b<base+size && cnt<=(ulong)(base+size-b)) return true;
    } // for

    return false;
} /* RangeInAnyModule */


/*
void MoveBlk( byte* dst, byte* src, ulong size )
// copy the block with <size> form <src> to <dst
// forward and backward mode supported (in case of overlapping structures
{
    ulong n;
    byte *s;
    byte *d;
    
    if (src>=dst) { // condition for forward/backward copy
        s=   src;   // normal forward copy
        d=   dst;
        for ( n=0; n<size; n++ ) { *d= *s; s++; d++; }
    }
    else {          // reverse ordered copy
        s=  (char*)src + size - 1;
        d=  (char*)dst + size - 1;
        for ( n=0; n<size; n++ ) { *d= *s; s--; d--; }
    }
} // MoveBlk
*/

/*
void MoveBlk( void* dst, void* src, ulong size )
// copy the block with <size> form <src> to <dst
// forward and backward mode supported (in case of overlapping structures
{
  ulong n;
  byte* s;
  byte* d;
    
  if (src>=dst) {    // condition for forward/backward copy
    s= (byte*)src;   // normal forward copy
    d= (byte*)dst;
    for ( n=0; n<size; n++ ) { *d= *s; s++; d++; }
  }
  else {             // reverse ordered copy 
    s= (byte*)( (ulong)src + size-1 );
    d= (byte*)( (ulong)dst + size-1 );
    for ( n=0; n<size; n++ ) { *d= *s; s--; d--; }
  } // if
} // MoveBlk
*/


Boolean SameBlk( byte *a, byte *b, ulong size )
/* compares two blocks */
{
    ulong n;
    for ( n=0; n<size; n++ ) { 
        if (*a!=*b) return false;
        a++; b++;
    }
    return true;
} /* SameBlk */


/* get the required size of a data module */
uint32_t DatMod_Size( uint32_t namsize, uint32_t datsize )
{
    uint32_t dsize= sizeof(struct modhcom) /* module header */
                  + sizeof(uint32_t)       /* data offset (4-byte OS-9 field) */
                  + datsize                /* the data segment */
                  + namsize                /* module name */
                  + sizeof(uint32_t);      /* CRC (4-byte OS-9 field) */

    dsize= (dsize+15) - ((dsize+15) % 16); // OS-9 data module sizes are divisible by 16
    return  dsize;
} /* DatMod_Size */


void FillTemplate( mod_exec *m, ushort access, ushort tylan, ushort attrev )
/* default values for data module. The three words are bit patterns and
   unsigned: an attribute word like $8001 arrived as a negative short, and
   os9_word shifted it -- undefined behaviour, found by UBSan through F$DatMod. */
{
    m->_mh._msync  = (short)os9_word(0x4AFC);  /* sync bytes ($4afc) */
    m->_mh._msysrev= os9_word(1);       /* system revision check value */
    m->_mh._msize  = 0;                 /* module size */
    m->_mh._mowner = 0;                 /* owner id */
    m->_mh._mname  = 0;                 /* offset to module name */
    m->_mh._maccess= os9_word(access);  /* access permission */
    m->_mh._mtylan = os9_word(tylan );  /* type/lang */
    m->_mh._mattrev= os9_word(attrev);  /* rev/attr */
    m->_mh._medit  = os9_word(0x0001);  /* edition */
    m->_mh._musage = 0;                 /* comment string offset */
    m->_mh._msymbol= 0;                 /* symbol table offset */
    m->_mh._mident = 0;                 /* ident code for ident program */
    m->_mh._mparity= 0;                 /* header parity */
}   


/* show modules */
void show_modules( char* cmp )
{
    mod_exec *mod;
    char*    nam;
    int      k;
    char     adrs[16];
    char     exeo[32];
    char     dats[32];
    char     stck[32];
    char*    mtyp;
    uint32_t modsize, nameoff;

    upo_printf("mID lnk T     68kAddr Type execoffs  datasiz stacksiz Name\n");
    upo_printf("--- --- - ----------- ---- -------- -------- -------- ----------------------------\n");

    for (k=0; k<MAXMODULES; k++) {
            mod= os9mod(k);
        if (mod!=NULL) {
            modsize= os9_long(mod->_mh._msize);
            nameoff= os9_long(mod->_mh._mname);
            if (nameoff == 0 || nameoff >= modsize)
                nam= "<bad-name>";
            else
                nam= Mod_Name( mod );

            if (cmp==NULL || ustrcmp( nam,cmp )==0) {
                debugprintf(dbgUtils,dbgNorm,("# imdir: %3d '%s'\n", k,nam ));

                snprintf( exeo,sizeof(exeo),"%8X",           os9_long(mod->_mexec )       );
                snprintf( dats,sizeof(dats),"%7.2fk", (float)os9_long(mod->_mdata )/KByte );
                snprintf( stck,sizeof(stck),"%7.2fk", (float)os9_long(mod->_mstack)/KByte );

                mtyp= Mod_TypeStr( mod );
                if (ustrcmp( mtyp,"Prog" )!=0 &&
                    ustrcmp( mtyp,"Trap" )!=0) {
                     strcpy( exeo,"-" );
                     strcpy( dats,"-" );
                     strcpy( stck,"-" );
                }

                if (os9modules[k].isBuiltIn)
                    strcpy(adrs, "  (builtin)");
                else
                    snprintf(adrs,sizeof(adrs), " $%08X", TO68K(mod));

                upo_printf("%3d %3d %c %11s %4s %8s %8s %8s %s\n",
                            k,
                            os9modules[k].linkcount,
                            os9modules[k].isBuiltIn ? 'I':'M',
                            adrs,
                            mtyp,
                            exeo,
                            dats,
                            stck,
                            nam
                          );
            } /* if (cmp==NULL || .. */
        } /* if (mod!=NULL) */
    } /* for */
} /* show_modules */
    

/* initialize internal "module directory" */
void init_modules()
{
    int k;
   
    for(k=0; k<MAXMODULES; k++) {
//      #ifdef macintosh
//        os9modules[k].modulehandle= NULL; /* no modules yet */
//      #else
          os9modules[k].modulebase  = NULL; /* no modules yet */
//      #endif
    
        os9modules[k].isBuiltIn= false;
        os9modules[k].linkcount= 0;
        os9modules[k].group    = (ushort)k; /* a group of one until a load says otherwise */
    }
   
    /* special treatement for init module */
    init_module= NULL;
} /* init_modules */


/* release a module by ID */
void release_module(ushort mid, Boolean modOK)
{
    char*            pNam;
    ushort           k, t;
    process_typ*     cp;
    traphandler_typ* tp;
    mod_trap*        tMod;
    
    mod_exec* mod= os9mod(mid); if (mod==NULL) return;

    if (modOK) {
        pNam= Mod_Name( mod );

        for (k=0; k<MAXPROCESSES; k++) {
                cp= &procs[k];
            if (cp->state!=pUnused &&
                cp->state!=pDead) {
            
                if (mid==cp->mid) return;
                
                for (t=0; t<NUMTRAPHANDLERS; t++) {
                    tp  = &cp->TrapHandlers[t];
                    tMod=  tp->trapmodule;
                    
                    if (tMod!=NULL) {
                        if (mid==tp->mid) return;
                    }
                } /* for all traphandlers do */
            }
        } /* for all processes do */
    
     /* special treatment for 'init' module: disconnect at the globals */
        if (ustrcmp(pNam,"init")==0) init_module= NULL;
    }


// The commented-out block below is MacOS-classic resource-fork handling
// (ReleaseResource/DisposeHandle), dead since the handle indirection went
// away. It is NOT an outstanding leak: the live path a few lines down does
// release_mem(mod) for every module that is not built in.
//  #ifdef macintosh
//    if (os9modules[mid].isBuiltIn) {
//        UnlockMemRange(mod,(unsigned long) GetHandleSize(os9modules[mid].modulehandle));
//        ReleaseResource(os9modules[mid].modulehandle); /* forget the block */
//    }
//  #endif


// NOTE: The handle part is now invisible (bfo) !!!
//  #ifdef macintosh
//    UnlockMemRange(mod,(unsigned long) GetHandleSize(os9modules[mid].modulehandle));
//    if (os9modules[mid].isBuiltIn)                 /* release the resource */
//        ReleaseResource(os9modules[mid].modulehandle); /* forget the block */
//    else      /* module was not loaded from a resource, just return memory */
//        DisposeHandle  (os9modules[mid].modulehandle);
//             
//    os9modules[mid].modulehandle= NULL;
//
//  #else

    if (!os9modules[mid].isBuiltIn) release_mem( mod ); /* free only if not built-in */
    os9modules[mid].modulebase  = NULL;

//  #endif
     
    os9modules[mid].isBuiltIn= false;      
    os9modules[mid].linkcount= 0;
    os9modules[mid].group    = mid;
} /* release_module */
    


mod_exec* get_module_ptr( int mid )
/* get pointer to module by mid */
{
    if (mid>=MAXMODULES) return NULL;
    return os9mod(mid);
} /* get_module_ptr */


int get_mid( void *modptr )
/* find mid of module given by ID */
{
    int k;
    for(k=0; k<MAXMODULES; k++) {
        if ((void*)get_module_ptr(k)==modptr) return k;
    }
    
    return MAXMODULES;
} /* get_mid */


ushort Mod_Revision( const mod_exec* mod )
/* The module's revision level: the LOW byte of M$RevsAttr, whose high byte is
   the attribute flags. */
{   return (ushort)( os9_word( mod->_mh._mattrev ) & 0xFF );
} /* Mod_Revision */


ushort Mod_Type( const mod_exec* mod )
/* The module's type: the HIGH byte of M$TyLan, whose low byte is the language. */
{   return (ushort)( os9_word( mod->_mh._mtylan ) >> BpB );
} /* Mod_Type */


int find_mod_id( const char* name )
/* find module by name, return mid or MAXMODULES if not found.
 *
 * Where several modules share a name, this returns the one with the HIGHEST
 * REVISION -- the "memory search" half of the rule M$Revs states: "If two
 * modules with the same name and type are found in the memory search or
 * loaded into memory, only the module with the highest revision level is
 * kept. This enables easy substitution of modules for update or correction."
 *
 * It used to return the first entry it came across, which is the same answer
 * only while no two modules share a name -- and load_module_local made sure
 * of that by throwing away every module it loaded whose name was already
 * taken, newer or not. Both halves had to change together; see the load path.
 */
{
    mod_exec *mod;
    int       best= MAXMODULES;
    ushort    bestRev= 0;
    int       k;

    for (k=0; k<MAXMODULES; k++) {
            mod= get_module_ptr(k);
        if (mod==NULL) continue; /* no module here, check next */

        if (ustrcmp( Mod_Name(mod),name )!=0) continue;   /* a different module */

        if (best==MAXMODULES || Mod_Revision(mod)>bestRev) {
            best   = k;
            bestRev= Mod_Revision( mod );
        } /* if */
    } /* for */

    return best;
} /* find_mod_id */     
    


int link_mod_id( char* name )
/* link module by name, return mid or MAXMODULES if not found */
{
    int    k= find_mod_id( name );
    if    (k<MAXMODULES) os9modules[k].linkcount++;
    return k;
} /* link_mod_it */     



int NextFreeModuleId( char* name )
/* Get next free module id: Start at mid=1, exception: <OS9exec> => mid=0 */
{
    int k;

    /* special handling for "OS9exec" module */
    if (name==NULL || ustrcmp( name, OS9exec_name )==0) {
        name= "<OS9exec>";
        k= 0;
    }
    else {                             /* this one is still empty => ok */
        for (k=1; k<MAXMODULES; k++) { if (os9mod( k )==NULL) break; }
    } // if
    
    debugprintf(dbgModules,dbgNorm,( "# NextFreeModuleId: %3d '%s'\n", k,name ));
    return k;
} /* NextFreeModule */             



static void init_setname( mod_exec* mh, uint32_t off, const char* s,
                          uint32_t slotmax, const char* what )
/* Copy one namestring into the "init" module, but only where it provably fits.
 *
 * This used to be a bare strcpy() per name. Two of the three destination
 * offsets are read out of the module image (0x50, 0x5A), so for an "init"
 * module loaded from a FILE they are file-controlled, and the copy wrote up to
 * 18 bytes at mh + <any 16-bit value>. Demonstrated: a 366-byte module whose
 * 0x50 word said 65280 -- blessed with a good CRC and good parity by OS-9's own
 * fixmod, so nothing upstream rejected it -- put hw_name 64914 bytes past the
 * end of its allocation. (Parity covers only the first 24 words, so patching
 * 0x50 does not disturb it, and load_module() verifies sync/parity/size/CRC but
 * never these two offsets.) load_module() already bounds the module's own name
 * offset against M$Size this same way; this applies that rule to the rest.
 *
 * `slotmax` is the structurally-known extent of the slot when there is one --
 * hw_site's is fixed at 0x4C and only reaches 0x50, where the hw_name offset
 * word itself lives, so an over-long hw_site would rewrite the very offset used
 * for the next copy. Pass 0 when only the module bound applies.
 */
{
    uint32_t modsize= os9_long( mh->_mh._msize );
    uint32_t n      = (uint32_t)strlen(s)+1; /* including the NUL */

    if (off==0 || off>=modsize || n>modsize-off || (slotmax!=0 && n>slotmax)) {
        debugprintf(dbgModules,dbgNorm,
          ("# adapt_init: %s slot at $%X + %u does not fit module size $%X"
           " (slotmax=%u) -- not copied\n",
             what, (unsigned)off,(unsigned)n, (unsigned)modsize,(unsigned)slotmax));
        return;
    }

    memcpy( (char*)mh+off, s, n );
} /* init_setname */


static void adapt_init( mod_exec* mh )
/* special treatment for the "init" module: set version+revision */
/* and connect it to the globals */
{
    byte*   bp;
    ushort* sp;

    bp= (byte*)  mh + 0x57; *bp= exec_version;
    bp= (byte*)  mh + 0x58; *bp= exec_revision;
    bp= (byte*)  mh + 0x59; *bp= 0;

    /* The three namestrings go through init_setname(), never a bare strcpy:
     * two of the destinations are 16-bit offsets read OUT OF the module image
     * itself (0x50, 0x5A). For the built-in Init_mod template those are the
     * trusted constants 0x84/0x95, but adapt_init() also runs on an "init"
     * module loaded from a FILE, where they are whatever the file says. */
    init_setname( mh, 0x4c,                                       hw_site, 0x50-0x4c, "hw_site" );
    bp= (byte*)  mh + 0x50;     sp= (ushort*)bp;
    init_setname( mh, os9_word(*sp),                              hw_name, 0,         "hw_name" );
    bp= (byte*)  mh + 0x5A;     sp= (ushort*)bp;
    init_setname( mh, os9_word(*sp),                              sw_name, 0,         "sw_name" );
    init_module= mh;
    mod_crc    ( mh );
} /* adapt_init */



/* Does the byte range [off, off+len) lie inside this loaded module?
 *
 * The adapt_* routines below reach into a module at offsets they know by
 * heart -- and, for inetdb, at offsets the MODULE ITSELF supplies. Neither
 * was checked, and guest module data driving an unchecked pointer is the
 * worst shape a bug can have here. It is not hypothetical: loading this
 * disk's own /dd/CMDS/BOOTOBJS/SPF/inetdb (SPF, edition 9) reads a "hosts"
 * offset of 16777229 out of a 2008-byte module, makes `size` the negative
 * difference from an end offset of 116, and hands that to memcpy as an
 * unsigned ~4.2 billion -- a bus error every time, on master as well.
 *
 * Written to be overflow-safe: `off+len` could wrap, so the length is
 * compared against the room remaining instead of being added to the offset.
 * os9_long names its argument four times (it is a macro), hence the temp. */
static Boolean mod_range_ok( const mod_exec* mh, uint32_t off, uint32_t len )
{
    uint32_t raw    = mh->_mh._msize;
    uint32_t modSize= (uint32_t)os9_long( raw );

    return (Boolean)( off<=modSize && len<=modSize-off );
} /* mod_range_ok */


static void adapt_le0( mod_exec* mh, uint32_t inetAddr )
{
    byte*  bp;

    /* These are WRITES at hardcoded offsets, so a module shorter than 0x8e
       bytes was corrupting whatever followed it in the arena. Check before
       touching anything, and leave a module that is not the expected shape
       alone -- adapting it is a convenience, not a precondition for loading
       it. */
    if (!mod_range_ok( mh, 0x7a, 4 ) ||
        !mod_range_ok( mh, 0x8a, 4 )) {
        debugprintf( dbgModules,dbgNorm,
          ("# adapt_le0: module too short for the address fields, left alone\n") );
        return;
    }

    bp= (byte*) mh;
    SET_OS9L(bp, 0x7a, inetAddr);  /* broadcast address position */
    *(byte*)(bp + 0x7d)= 0xff;     /* specific for broadcast */

    SET_OS9L(bp, 0x8a, inetAddr);  /* my internet address position */

    mod_crc( mh );
} /* adapt_le0 */



static void fill_s( char** b, const char* end, const char* s )
/* copy <s> and its NUL to *b, then advance *b past it -- never writing at or
 * beyond <end>.
 *
 * <end> is not decoration: every caller writes into a field INSIDE a guest
 * module (inetdb's hosts and resolv.conf areas), whose length the module
 * itself declares, while the strings come from the host (/etc/resolv.conf,
 * gethostbyname). A long host domain name therefore used to run straight off
 * the end of the module's field and corrupt whatever followed it in the arena.
 * The size was known at both call sites all along -- it is the same length the
 * memset above each of them already uses to clear the field. */
{
    size_t room, n;

    if (*b>=end) return;                 /* nothing left */
    room= (size_t)(end - *b);
    n   = strlen( s );
    if (n>=room) n= room-1;              /* truncate, keeping space for the NUL */

    memcpy( *b, s, n );
    (*b)[n]= NUL;
    *b += n+1;
} /* fill_s */



static const char* list_name_end( const char* v, const char* vEnd )
/* One past the NUL that ends the name at <v>, or NULL when the name runs to
 * <vEnd> without one. */
{
    const char* nul= v<vEnd ? memchr( v, NUL, (size_t)(vEnd-v) ) : NULL;
    return nul==NULL ? NULL : nul+1;
} /* list_name_end */



static void go_thru_list( const char* v0, const char* vEnd, char* b0, const char* bEnd, uint32_t inetAddr )
/* adapt "localhost" at the "inetdb" module.
 * <v0>..<vEnd> is a copy of the module's hosts field and <b0>..<bEnd> the field
 * itself, which is rewritten from the copy only when "localhost" is found with
 * another address. Otherwise the field is left exactly as loaded.
 * Every count, jump and name in the copy comes out of the module, so none of
 * them is trusted: a walk that would leave <vEnd> stops, and nothing is
 * written at or past <bEnd> -- the host-supplied names included, see fill_s. */
{
    const char *v, *blk, *next;
    char       *b, *bBlk;
    /* byte*, not uint32_t*: the 4-byte inetaddr lives at <blk>+2 (right after the
     * 2-byte jump field), so it is only ever 2-aligned -- dereferencing it as a
     * uint32_t* was a misaligned access, the same undefined behaviour fixed in
     * os9_ll.h's GET_OS9L/SET_OS9L. Read it out with memcpy into <ipaVal>
     * instead; the byte pointer is still what the memcpy below wants as source. */
    const byte *ipa;
    uint32_t   ipaVal;
    /* Counts and byte lengths, so UNSIGNED 16-bit. These were plain `short`:
       a block length of 0x8000 or more read back negative and `blk+=jump`
       then walked backwards. The field is small in practice, which is why it
       never showed, but there is no reading under which a length is signed. */
    uint16_t   i, n, jump;

    /* The module stores addresses in 68k (big-endian) order; <inetAddr>
       arrives in HOST order -- adapt_le0 hands the same value to SET_OS9L,
       which swaps it on the way in. Convert ONCE, here, and compare against
       the converted copy everywhere below.
       This is a fix, not a tidy-up: the first loop compared the raw module
       bytes against the unconverted host value while the second compared
       against os9_long() of it, so the two disagreed on a little-endian host
       and agreed only on a big-endian one. The first test therefore never
       matched on x86/ARM, and its "everything is perfect already" early-out
       never fired -- the field was rewritten every time instead. Same result,
       reached the long way, and wrong for the reason that is hardest to
       notice. os9_long is a MACRO that names its argument four times, so it
       gets a temporary rather than being used inline. */
    const uint32_t wantAddr= (uint32_t)os9_long( inetAddr );

    const size_t head= sizeof(uint16_t)+sizeof(uint32_t); /* jump + inetaddr */
    Boolean lFound= false;

    if (vEnd-v0 < (ptrdiff_t)sizeof(uint16_t)) return;
    n= GET_OS9W( v0, 0 );                    /* number of entries */
    v0+= sizeof(uint16_t); /* skip the number of entries entry */

    v= v0;
    for (i=0; i<n; i++) {
        if (vEnd-v < (ptrdiff_t)head) return;
        blk= v;             v+= sizeof(uint16_t); jump= GET_OS9W( blk, 0 );
        ipa= (const byte*)v; v+= sizeof(uint32_t); /* get the 4-byte inetaddr */
        memcpy( &ipaVal, ipa, sizeof(ipaVal) );

        while (true) {
            next= list_name_end( v, vEnd );
            if (next==NULL) return;          /* a name with no end: not a layout we know */
            if (ustrcmp( v,"localhost" )==0) {
                lFound= true;
                if (ipaVal==wantAddr) return; /* everything is perfect already */
            }

            v= next;
            if (v>=vEnd || *v==NUL) break;
        } /* while */
        if (jump<head || jump>vEnd-blk) return;
        v= blk+jump;
    } /* for */
    if (!lFound) return; /* probably not enough room to put "localhost" in */

    /* Only now is the field rewritten, so the returns above leave it intact. */
    memset( b0, 0, (size_t)(bEnd-b0) );
    memcpy( b0, v0-sizeof(uint16_t), sizeof(uint16_t) ); /* copied RAW: still 68k order */
    b= b0+sizeof(uint16_t);

    v= v0;
    for (i=0; i<n; i++) {
        /* The walk above proved every entry of the copy; the field can still
           run out when "localhost" is added, and then the rest is dropped. */
        if (bEnd-b < (ptrdiff_t)head) return;
        blk =             v;  v+= sizeof(uint16_t); jump= GET_OS9W( blk, 0 );
        ipa = (const byte*)v; v+= sizeof(uint32_t); /* get the 4-byte inetaddr */
        memcpy( &ipaVal, ipa, sizeof(ipaVal) );

        bBlk=         b;       b+= sizeof(uint16_t);
        memcpy(b, ipa, sizeof(uint32_t)); b+= sizeof(uint32_t); /* copy 4-byte inetaddr */

    //  printf( "%3d %3d %08X '%s'\n", i, jump, os9_long( ipaVal ), v );

        fill_s( &b,bEnd, v );
        if (ipaVal==wantAddr) fill_s( &b,bEnd, "localhost" );
        fill_s( &b,bEnd, ""         ); /* one additional NUL char */
        
        if ((uintptr_t)b%2==1 && b<bEnd) b++; /* make address even */
        SET_OS9W( bBlk, 0, (uint16_t)(b-bBlk) );

        v= blk+jump;
    } /* for */
} /* go_thru_list */



static void adapt_inetdb( mod_exec* mh, uint32_t inetAddr, uint32_t dns1, uint32_t dns2, char* domainName )
/* the module "inetdb" (part of Internet Support Package ISP) will be adapted according */
/* to the OS9exec's host machine settings: <inetAddr> <dns1> <dns2> and <domainName>    */
{
    uint16_t hLen;   /* declared length of the resolv.conf area, module order */
    uint32_t hostsOff, endOff, dnsOff;
    char    *bp, *b0, *bL, *v0;
    char    *bpEnd; /* one past the last writable byte of the resolv.conf field */
    uint32_t d, size;
    byte    *h;
    char    sv[ OS9NAMELEN ];

    
 /* ------- hosts field adaption --------- */
    /* Each of these was two statements: point a char* at the offset word, then
       dereference it AS a uint32_t* and swap. One GET_OS9L does the whole job
       -- byte-wise load plus swap, no pointer of the wrong type formed at all,
       and no intermediate value that is a host pointer on one line and a
       module-relative offset on the next. */
    /* Both offsets come OUT OF THE MODULE, so nothing about them is trusted:
       the pair has to be inside the module before it can be read, the field
       they describe has to be inside it too, and the end has to follow the
       start. Measured on this disk's own SPF inetdb, which fails all three.
       Skipping leaves the module exactly as loaded, which is the right
       outcome -- it simply is not the layout this adaption knows. */
    if (!mod_range_ok( mh, OFFS_HOSTS, 2*sizeof(uint32_t) )) {
        debugprintf( dbgModules,dbgNorm,
          ("# adapt_inetdb: no room for the hosts offsets, field left alone\n") );
        return;
    }
    hostsOff= GET_OS9L( (char*)mh, OFFS_HOSTS );
    endOff  = GET_OS9L( (char*)mh, OFFS_HOSTS + sizeof(uint32_t) );

    if (endOff<=hostsOff ||
        !mod_range_ok( mh, hostsOff, endOff-hostsOff )) {
        debugprintf( dbgModules,dbgNorm,
          ("# adapt_inetdb: hosts field $%X..$%X not inside the module, left alone\n",
              hostsOff, endOff) );
        return;
    }

    b0= (char* ) mh + hostsOff;   /* start of "hosts" */
    bL= (char* ) mh + endOff;     /* end   of "hosts" */
  
                      size= bL-b0;
        v0=  get_mem( size );
    if (v0==NULL) {
        debugprintf( dbgModules,dbgNorm,
          ("# adapt_inetdb: no memory for a %u-byte copy of the hosts field, left alone\n", size) );
        return;
    }
    memcpy( v0,b0, size );

    go_thru_list( v0,v0+size, b0,bL, inetAddr );  /* rearrange the field */

    release_mem( v0 );

    
        
 /* ------- DNS field adaption --------- */    
    if (!mod_range_ok( mh, OFFS_DNS, sizeof(uint32_t) )) {
        debugprintf( dbgModules,dbgNorm,
          ("# adapt_inetdb: no room for the DNS offset, field left alone\n") );
        return;
    }
    dnsOff= GET_OS9L( (char*)mh, OFFS_DNS );

    /* 4 bytes: the 2 skipped below plus the 2-byte declared length after them */
    if (!mod_range_ok( mh, dnsOff, 4 )) {
        debugprintf( dbgModules,dbgNorm,
          ("# adapt_inetdb: resolv.conf field at $%X not inside the module, left alone\n",
              dnsOff) );
        return;
    }

    bp = (char* ) mh + dnsOff;   /* start of "resolv.conf" */
    bp+= 2;
    /* The area's own declared length, read once instead of aliasing a short*
       at it and dereferencing that twice. */
    hLen= GET_OS9W( bp, 0 );
    bp+= 2;

    /* The writable area is what the memset below clears: hLen-2 bytes starting
       at bp. Capture its end BEFORE anything advances bp -- every write past
       this point is bounded by it.
       hLen comes out of a GUEST module, so a declared length under 2 is
       possible and used to be catastrophic rather than merely wrong: hLen-2
       promotes to int, goes negative, and the memset below converts it to a
       huge size_t. Treat anything under 2 as an empty area. */
    if (hLen<2) hLen= 2;
    bpEnd= bp + (hLen-2);

    /* hLen is the module's OWN claim about its field, so clamp it to what
       the module actually holds: a declared length longer than the module
       would let the memset and every fill_s below write past the end. */
    {   uint32_t raw= mh->_mh._msize;
        char*    modEnd= (char*)mh + (uint32_t)os9_long( raw );
        if (bpEnd>modEnd) bpEnd= modEnd;
        if (bpEnd<bp)     bpEnd= bp;
    }

    /* Bounded, because <bp> is the module's own field and its existing domain
       name is whatever the module file happened to contain: a strcpy of it into
       sv[OS9NAMELEN] (29 bytes) was a straight stack overflow for any module
       carrying a longer name. */
    strncpy( sv, bp, sizeof(sv)-1 );    /* make a copy of the existing domain name */
    sv[sizeof(sv)-1]= NUL;
    memset( bp, 0, (size_t)(bpEnd-bp) );                 /* clear the original area */

    if  (strcmp( domainName,"" )==0) domainName= sv;
    fill_s( &bp,bpEnd, domainName );                 /* fill in the domain name */

                                  d= os9_long( dns1 );
                                  h= (byte*) &d;
    if (bp<bpEnd) {
        snprintf( bp,(size_t)(bpEnd-bp), "%d.%d.%d.%d", h[0],h[1],h[2],h[3] ); /* fill in DNS IP address */
        bp=  bp + strlen( bp )+1;
    }

    if (dns2!=0 && bp<bpEnd) {    d= os9_long( dns2);
                                  h= (byte*) &d;
        snprintf( bp,(size_t)(bpEnd-bp), "%d.%d.%d.%d", h[0],h[1],h[2],h[3] ); /* fill in DNS IP address */
        bp=  bp + strlen( bp )+1;
    }

    fill_s( &bp,bpEnd, ""         ); /* one additional NUL char */
    fill_s( &bp,bpEnd, domainName ); /* fill in the domain name */
    
    
 /* --- update module CRC --- */          
    mod_crc( mh );
} /* adapt_inetdb */



static void adapt_L2( mod_exec* mh )
/* special treatment for the "L2" module: set port address */
{
    /* M$Port lives at $030, so a module shorter than that plus its own four
       bytes cannot hold it -- writing anyway would land past the module. */
    if (!mod_range_ok( mh, 0x30, 4 )) {
        debugprintf( dbgModules,dbgNorm,
          ("# adapt_L2: module too short for M$Port, left alone\n") );
        return;
    }

    /* The write goes through the byte accessor, like adapt_le0's just above:
       the offset this function already bounds-checked is the one it then
       writes at, rather than a mod_dev overlay laid over a mod_exec* whose
       field happens to sit there. SET_OS9L does the byte order itself. */
    SET_OS9L( (byte*)mh, 0x30, TO68K(&l2.hw_location) );

    mod_crc( mh );
} /* adapt_L2 */


static os9err load_module_local( ushort pid, char* name, ushort* midP, Boolean exedir, 
                                                                       Boolean linkstyle,
                                                                       void*   modBase,
                                                                       ushort  path,
                                                                       ulong   bootPos,
                                                                       ulong   bootSiz )
/* load a module by name or path
 * Note: if a null pointer is passed for name, load defaults
 *       to 'OS9C' id 0 (only on Mac)
 */
{
    ushort  mid, mid0, oldmid;
    ushort  linkmid;
    Boolean isBuiltIn;
    /* "the file really did contain an OS-9 module": set once the MODSYNC check
     * below has passed. It decides which error a failed load reports -- see the
     * end of this function. */
    Boolean wasModule= false;
    uint32_t dns1= 0, dns2= 0;

    #ifdef MACOS9
      Handle hh;
    #endif

    mod_exec* theRemainP;    
    mod_exec* theModuleP;
    
    char    datapath  [OS9PATHLEN];
    char    domainName[OS9PATHLEN];
    Boolean isPath;
    char*   pn;
    void*   pp;
    
    #ifdef MACFILES
      OSErr  oserr;
      FSSpec modSpec;
      short  refNum;
    #else
      FILE   *stream;
    #endif
    
    uint32_t dsize, loadbytes;
    ushort   fileOwner= 0; /* FD_OWN of the file read; a host file counts as the super user's */
    os9err err;
    ushort par;
    uint32_t crc;

    #define MODNLEN 33
    char realmodname[MODNLEN];

    uint32_t  modSize;    /* OS-9 module size */
    ushort    mode, sync;
    ptype_typ type;       /* distinguish which file manager to be used */
    
    /* first, find an empty module dir entry */
    isPath=false;
    debugprintf(dbgModules,dbgNorm,("# load_module: searching module '%s' (%s, %s)\n",
                name==NULL ? "<default: OS9C ID=1>" : name, 
                exedir    ? "exec"    : "data", 
                linkstyle ? "linking" : "loading"));
    
    mid= NextFreeModuleId( name );
   
    /* check for default loading of 'OS9C' 0 */
    #ifdef MACOS9
      if (name==NULL) {
          if (mid>=MAXMODULES) return os9error(E_DIRFUL); /* module directory is full */

              hh = GetResource('OS9C', 0);
          if (hh!=NULL) {
              isBuiltIn= true;
              theModuleP= *hh;
              goto modulefound;
          }
          return os9error( linkstyle ? E_MNF:E_PNNF );
      }
    #endif
               
    /* check for pathlist and default to file-load if one is found */
    mode= exedir ? 0x05 : 0x01;
    type= IO_Type( pid,name, mode );    
    
    if (type==fRBF) {
        isPath= false; /* make life easy */
        strcpy( datapath,name );
    }
    else {
        pn = name;
        err= parsepathext(pid,&pn,datapath,exedir,&isPath); if (err) return err;
    }

    if (linkstyle && !isPath) {
        /* if it is not a path, try to link it */
            linkmid= link_mod_id( name );
        if (linkmid<MAXMODULES) {
            debugprintf(dbgModules,dbgNorm,("# load_module: module '%s' found in module dir mid=%d, link=%d\n",
                                               name,linkmid,os9modules[linkmid].linkcount));
            *midP= linkmid;
            return 0;
        }
    }
    
    
    /* then, try to load it */
    if (mid>=MAXMODULES) return os9error(E_DIRFUL); /* module directory is full */
    
	debugprintf(dbgModules,dbgNorm,("# load_module: mid=%d isPath=%d exedir=%d\n", 
	                                   mid, isPath,exedir ));
    
    /* now actually try to locate the module */
    do {
        if (!isPath && exedir) {
            /* no path, and not relative to data directory, so try extra loading tricks */
            isBuiltIn= false;
              
            /* 1a: Check if it is the built-in "OS9exec" module */
            if (ustrcmp(name,OS9exec_name)==0) {
            	OS9exec_ptr= (mod_exec*)OS9exec_mod;
                dsize      =     sizeof_OS9exec_mod;
                theModuleP =            OS9exec_ptr;
                isBuiltIn  = true;
                break; /* found */
            } /* if built-in OS9exec module */
                        
            /* 1b: Check if it is the built-in "init" module */
            if (ustrcmp(name,"init")==0) {
            	Init_ptr= (mod_exec*)Init_mod;
                dsize   =     sizeof_Init_mod;
                
                /* it can't be const def, because it wil be changed afterwards */
                pp= get_mem( dsize );
           	
                        theModuleP= pp;
                memcpy( theModuleP, Init_ptr, dsize );
             
                isBuiltIn= true;
                break; /* found */
            } /* if built-in Init module */

            /* 1c: Check if it is the built-in "socket" module */
            if (ustrcmp(name,"socket")==0) {
                Socket_ptr = (mod_exec*)Socket_mod;
                dsize      =     sizeof_Socket_mod;
                theModuleP =            Socket_ptr;
                isBuiltIn  = true;
                break; /* found */
            } /* if built-in socket module */

            /* 1d: Check if it is the built-in "le0" module */
            if (ustrcmp(name,"le0")==0) {
                Le0_ptr= (mod_exec*)Le0_mod;
                dsize  =     sizeof_Le0_mod;
                
                /* it can't be const def, because it wil be changed afterwards */
                pp= get_mem( dsize );
           	
                        theModuleP= pp;
                memcpy( theModuleP, Le0_ptr, dsize );
             
                isBuiltIn= true;
                break; /* found */
            } /* if built-in Le0 module */

            
            /* 2: Assume loading from the {OS9MDIR} directory */
            /* %%% this part is not yet adapted for access through filestuff interface */
            #ifdef MACOS9
        //    upe_printf( "mdir.volID %d %s\n", mdir.volID, name );
              if (mdir.volID!=0) {
                  debugprintf(dbgModules,dbgNorm,("# load_module: trying to load from {OS9MDIR}\n"));

                  #ifdef MACFILES
                         err= getFSSpec(pid,datapath, _mdir, &modSpec);
                    if (!err) goto streamload;
                    
                    if (ustrcmp(datapath,applName)==0) {
                             err= getFSSpec(pid,datapath, _appl, &modSpec);
                        if (!err) goto streamload;
                    }
                  
                  #else
                        oserr= HSetVol(NULL,mdir.volID,
                                            mdir.dirID);
                    if (oserr)  return host2os9err(oserr,E_BPNAM);
                
                        stream= fopen(datapath,"rb"); /* open for read, binary mode (bfo) */
                    if (stream!=NULL) goto streamload;
                
                    /* switch back to exe dir */
                        oserr= HSetVol(NULL,procs[pid].x.volID,
                                            procs[pid].x.dirID);
                    if (oserr) return host2os9err(oserr,E_BPNAM);
                  #endif
              }

            #else
            if (*mdirPath!=0) {
                char pathbuf[OS9_MAXPATH]; /* temp buffer for path */
                debugprintf(dbgModules,dbgNorm,("# load_module: trying to load from OS9MDIR: %s\n",mdirPath));
                /* Built with a bound. mdirPath is itself OS9_MAXPATH, so
                   mdirPath + separator + module name could exceed pathbuf even
                   when each part fits on its own -- three unchecked copies into
                   one buffer. A name that will not fit cannot name a real file
                   either, so the attempt is simply skipped. */
                if (snprintf( pathbuf,sizeof(pathbuf), "%s%s%s",
                              mdirPath, PATHDELIM_STR, name ) >= (int)sizeof(pathbuf)) {
                    debugprintf(dbgModules,dbgNorm,
                       ("# load_module: OS9MDIR path too long for '%s', skipped\n", name));
                }
                else {
                        stream= fopen(pathbuf,"rb"); /* open for read, binary mode (bfo) */
                    if (stream!=NULL) goto streamload;
                }
            }
            #endif
            
                        
            #ifdef macintosh
            /* 3: Look for an MPW tool with that name and look if it contains an appropriate OS9C */
            /* %%% to be implemented */
            #endif
        }

        if (linkstyle && modBase!=NULL) {
          if (modBase==No_Module) modBase= OS9exec_ptr;
          
          theModuleP= modBase;
          dsize     = os9_long( theModuleP->_mh._msize );
          isBuiltIn = true;
          break; /* found */
        } // if

        /* no more trials for linking */
        if (linkstyle) return E_MNF;
        

        /* 4: Assume it to be a path relative to the execution or data directory (dep. on exedir) */
        debugprintf(dbgModules,dbgNorm,( "# load_module: load path (%s) = %s\n",
                                            exedir ? "exec":"data", datapath ));

        /* try to open a file */
// openit:
//      mode= exedir ? 0x05 : 0x01;
//      type= IO_Type( pid,name, mode );
        if (type==fFile || type==fDir) name= datapath;

        err= 0;
        if (bootPos==0) {     // for <bootPos> > 0, it's already opened for boot reading
              err= usrpath_open( pid, &path,type, name,mode );
          if (err)
            return os9error( linkstyle ? E_MNF:err ); /* as the real OS-9 */
        } // if

        if (bootSiz==0) {
              err= usrpath_getstat( pid,path,SS_Size, NULL,NULL,NULL,&dsize,NULL );
          if (err) return err;
        }
        else {
          dsize= bootSiz;
        } // if

            pp= get_mem( dsize );
        if (pp==NULL) {
          if (bootPos==0) err= usrpath_close( pid, path );

          return os9error(E_NORAM); /* not enough memory */
        } // if

        loadbytes = dsize;
        theModuleP= pp;

            err= usrpath_read ( pid, path, &loadbytes, theModuleP, false );
        if (err || loadbytes<dsize) {
          if (bootPos==0) err= usrpath_close( pid, path );
          return E_READ;
        } /* if */         
              
        isBuiltIn= false; /* is no resource-based module */
                        
        debugprintf(dbgModules,dbgNorm,
          ("# load_module: loaded %u bytes from module's file\n", loadbytes));
          
        #ifdef RBF_SUPPORT
          if (bootPos==0) {
              syspath_typ* fsp= get_syspath( pid, procs[ pid ].usrpaths[ path ] );
              if (fsp!=NULL && fsp->type==fRBF) fileOwner= PathFDOwner( fsp );
          }
        #endif

        if (bootPos==0) err= usrpath_close( pid, path );
        break; /* module data loaded */
                  

// this section is only used because of "goto streamload"
        #ifdef MACFILES
             err= getFSSpec( pid,datapath, exedir ?_exe:_data, &modSpec);
        if (!err) { /* ok, file is there */
        #else
            stream=fopen(datapath,"rb"); /* open for read, binary mode (bfo) */
        if (stream!=NULL) {
        #endif
        
    streamload:
            #ifdef MACFILES
              /* open file for read */
                  oserr= FSpOpenDF(&modSpec, fsRdPerm, &refNum);
              if (oserr) return host2os9err(oserr,E_PNNF);
              /* get  file size */
                  oserr= GetEOF(refNum, (long *)&dsize); 
              if (oserr) return host2os9err(oserr,E_SEEK);
            #else
              fseek(stream,0,SEEK_END);     /* go to EOF */
              dsize= (ulong) ftell(stream); /* get position now = file size */
              fseek(stream,0,SEEK_SET);     /* back to beginning */
            #endif

            
            /* allocate memory for the module */
            debugprintf(dbgModules,dbgDetail,
              ("# load_module: module file size = %u\n",dsize));

                pp= get_mem( dsize );
            if (pp==NULL) {
                #ifdef MACFILES
                  FSClose(refNum);
                #else
                  fclose (stream);
                #endif
                return os9error(E_NORAM); /* not enough memory */
            }
            
            theModuleP= pp;
            debugprintf(dbgModules,dbgNorm,
              ("# load_module: allocated memory %u @ %p\n", dsize, (void*)theModuleP));
                
            #ifdef MACFILES
              loadbytes= dsize; /* now read module */
                  oserr= FSRead( refNum, &loadbytes, theModuleP );
              if (oserr) {
                  FSClose(refNum);
                  return host2os9err(oserr,E_READ);
              }
            #else
                  loadbytes= fread( theModuleP, 1,dsize, stream );
              if (loadbytes==0) {
                  fclose(stream);
                  return c2os9err(errno,E_READ);
              }
            #endif

            
            debugprintf(dbgModules,dbgNorm,
              ("# load_module: loaded %u bytes from file\n", loadbytes));
              
            #ifdef MACFILES
              FSClose(refNum);
            #else
              fclose (stream);
            #endif
            
            break; /* module data loaded */
        }
        /* no success */
        
        debugprintf(dbgModules,dbgNorm,
          ("# load_module: OS9 module could not be installed\n"));
          
        return os9error( linkstyle ? E_MNF:E_PNNF ); /* could not link / load */    
    } while(true);
    /* "If any of the modules loaded belong to the super-user, the file must
       also be owned by the super-user. This prevents normal users from
       executing privileged service requests" (F$Load, page 1-41), and "If not,
       the modules contained within the file are not loaded" (file security).
       It is what keeps F$SUser's "change to the module's owner" from letting
       any user stamp 0.0 into a module and become the super user. The super
       user is group 0: FD_OWN's high byte, M$Owner's high word. None of the
       file's modules is entered unless all of them pass, so look at every
       header first. E$Permit: "The process or module must be owned by the
       super-user to perform the requested function". */
    if (!isBuiltIn && (fileOwner>>8)!=0) {
        ulong off= 0;
        while (off+sizeof(modhcom)<=dsize) {
            mod_exec* m= (mod_exec*)( (byte*)theModuleP + off );
            ulong     msize;

            if (os9_word(m->_mh._msync)!=MODSYNC) break; /* the loop below judges the rest */
            if ((os9_long(m->_mh._mowner)>>16)==0) {
                debugprintf(dbgModules,dbgNorm,("# load_module: super-user module in a file owned by $%04X\n",
                                                  fileOwner));
                release_mem( theModuleP );
                return os9error(E_PERMIT);
            }
                msize= os9_long(m->_mh._msize);
            if (msize==0) break;
            off+= msize;
        }
    }

    /* module found, insert it into module directory */
    
    
    #ifdef MACOS9
      modulefound:
    #endif
    
    mid0= mid;     /* take the first one if using module groups */
    while (true) {
        os9modules[mid].modulebase= theModuleP; /* enter pointer in free table entry */   
        os9modules[mid].isBuiltIn = isBuiltIn;
        debugprintf(dbgModules,dbgNorm,
          ("# load_module: (found) mid=%d, theModuleP=%p, ^theModuleP=%08X\n",
              mid, (void*) theModuleP, GET_OS9L( (byte*)theModuleP, 0 )));
   
        /* "All modules that are loaded are added to the system module
           directory, and the first module read is linked" (F$Load, page 1-41).
           Every module of a file was linked, so the later ones could never be
           unlinked away. They come in unlinked now, as the first one's group
           (see unlink_module). */
        os9modules[mid].group    = mid0;
        os9modules[mid].linkcount= mid==mid0 ? 1 : 0;
        
        /* don't forget to flush the CodeRange !! */
        Flush68kCodeRange( theModuleP, dsize );

   
        /* make sure that module is ok */
        /* --- check module SYNC parity and CRC */
            sync= os9_word(theModuleP->_mh._msync);
        if (sync!=MODSYNC) {
            /* bad module sync */
            debugprintf(dbgModules,dbgNorm,
              ("# load_module: bad modsync: %04x, E_BMID\n", sync ));
            err= E_BMID;  break;
        } /* if */
        wasModule= true; /* past this point the file IS a module, however damaged */

            par= calc_parity( (ushort*)theModuleP, 24 );
        if (par!=0) {
            /* bad header parity */
            debugprintf(dbgModules,dbgNorm,
              ("# load_module: bad parity, check result=$%04X (should be 0)\n",par));
            err= E_BMHP;  break;
        } /* if */
        
            modSize= os9_long(theModuleP->_mh._msize);
        if (modSize>dsize) {
            debugprintf(dbgModules,dbgNorm,
              ("# load_module: bad size: %d>%d, E_BMID\n", modSize,dsize ));
            err= E_BMID;  break; /* as a native OS-9 system (bfo) */
        } /* if */

        	crc= calc_crc( (byte*)theModuleP, modSize, 0xFFFFFFFF );
        if (crc!=0xFF800FE3) {
            debugprintf(dbgModules,dbgNorm,
              ("# load_module: bad crc, crc result=$%08X (should be $FF800FE3)\n",crc));
            /* bad CRC */
            err= E_BMCRC; break;
        } /* if */
         
        /* --- module SYNC/parity/size/CRC are ok, but the NAME OFFSET is not yet
           trusted.  _mname is a 32-bit offset the module file chose; sync,
           parity and CRC can all be forged (see tools/fuzz-module.sh), so a
           corrupt or hostile module can point it anywhere.  Mod_Name() turns it
           into a raw HOST pointer that nullterm() then walks byte-by-byte until
           a terminator, so an out-of-range offset -- or an in-range name with no
           terminator before the module ends -- is an out-of-bounds host read.
           show_modules() already guards its display with the range half of this
           test; guard the loader too, so a bad-name module never enters the
           module directory and every later Mod_Name() caller stays safe. */
        {   uint32_t nameoff= os9_long(theModuleP->_mh._mname);
            uint32_t scan;
            Boolean  terminated= false;
            /* nullterm() stops on the first char <= ' ' (its s2 is signed, so
               high-bit OS-9 name terminators count as negative <= ' ' too);
               require such a byte within the loaded image. */
            for (scan= nameoff; nameoff!=0 && nameoff<modSize && scan<modSize; scan++) {
                if (((signed char*)theModuleP)[scan] <= ' ') { terminated= true; break; }
            }
            if (!terminated) {
                debugprintf(dbgModules,dbgNorm,
                  ("# load_module: bad name offset $%X (module size $%X), E_BMID\n",
                      nameoff, modSize));
                err= E_BMID; break;
            }
        }

        /* --- module loaded is ok */
        /* now check if we already have something like this in our module dir */
        nullterm(realmodname,Mod_Name( theModuleP ),MODNLEN);
        debugprintf(dbgModules,dbgNorm,
          ("# load_module: Name of module loaded='%s'\n",realmodname));

        /* A module of this name may already be resident. M$Revs decides which
         * one survives: "If two modules with the same name and type are found
         * in the memory search or loaded into memory, only the module with the
         * highest revision level is kept. This enables easy substitution of
         * modules for update or correction." (v2.4 Technical Manual.)
         *
         * F$VModul states the same rule as an algorithm, and is the sharper
         * citation of the two: "The module directory is first searched for
         * another module with the same name. If a module with the same name and
         * type exists, the one with the highest revision level is retained in
         * the module directory. Ties are broken in favor of the established
         * module." That last sentence is why an equal revision leaves the
         * resident one in place below.
         *
         * Until 2026-08-24 the resident module always won and the one just
         * loaded was discarded unread -- revision never consulted, type never
         * compared. That is why rebuilding a module and loading it appeared to
         * do nothing: the old one kept answering, and there was no way to
         * displace it short of unlinking it to zero.
         *
         * The newcomer does NOT evict the old entry. A process already running
         * holds a link to the old module and must keep working, so both stay in
         * the directory and the old one goes when its own link count reaches
         * zero, exactly as it would have without this. What changes is only
         * which of them a search finds -- find_mod_id returns the highest
         * revision, so the new module answers from here on. */
        os9modules[mid].modulebase=0; /* temporarily disable entry */
        
            oldmid= find_mod_id( realmodname );
        if (oldmid<MAXMODULES) {
            mod_exec* oldMod= get_module_ptr( oldmid );

            /* "same name AND type" -- a different type is a different module
               and does not participate in the comparison at all. */
            Boolean sameKind= Mod_Type( oldMod )==Mod_Type( theModuleP );
            Boolean isNewer = Mod_Revision( theModuleP )>Mod_Revision( oldMod );

            /* A later module of the file stays as it is, unlinked in its group:
               lookups still find the resident one, and releasing it here would
               free the rest of the file, which the loop reads next. */
            if (sameKind && !isNewer && mid==mid0) {
                /* the resident one is as good or better: keep it, as before */
                os9modules[oldmid].linkcount++; /* link the old one */
                os9modules[mid].modulebase=theModuleP; /* re-enable entry */
                release_module(mid, false); /* forget it again */
                *midP= oldmid;
                debugprintf( dbgModules,dbgNorm,
                  ("# load_module: '%s' rev %d not kept, rev %d is resident\n",
                     realmodname, Mod_Revision(theModuleP), Mod_Revision(oldMod) ));
                return 0; /* ... and throw away just loaded module */
            } /* if */

            debugprintf( dbgModules,dbgNorm,
              ("# load_module: '%s' rev %d supersedes the resident rev %d\n",
                 realmodname, Mod_Revision(theModuleP), Mod_Revision(oldMod) ));
        } /* if */
          
        os9modules[mid].modulebase= theModuleP; /* re-enable entry */

         
        /* "le0" and "inetdb" are adapted to my_inetaddr, which stays the
           loopback address (os9exec_nt.c) unless something sets it. The ISP
           stack asked the host for its real address here; it is retired, and
           nothing that uses SPF has wanted anything but loopback. */
              
        if (ustrcmp(realmodname,"init"  )==0) adapt_init  ( theModuleP );
        if (ustrcmp(realmodname,"le0"   )==0) adapt_le0   ( theModuleP, my_inetaddr );
        if (ustrcmp(realmodname,"inetdb")==0) adapt_inetdb( theModuleP, my_inetaddr,dns1,dns2, domainName );
        if (ustrcmp(realmodname,"L2"    )==0) adapt_L2    ( theModuleP );
       
        *midP= mid0;
        if    (dsize==modSize) return 0; /* its done now */ 
        dsize= dsize- modSize;
        theModuleP= (mod_exec *)( (uintptr_t)theModuleP + (uintptr_t)modSize );
                                                  /* get pointer to the next module */

        if (dsize>1 && os9_word(theModuleP->_mh._msync)!=MODSYNC) return 0;
                      /* if the remaining part is not a module -> skip it, no error */

            mid = NextFreeModuleId( "" ); /* name not yet available */
        if (mid>=MAXMODULES) return os9error(E_DIRFUL); /* module directory is full */
                
            pp= get_mem( dsize );                                 /* get the memory */
        if (pp==NULL)        return os9error(E_NORAM);         /* not enough memory */
        
        theRemainP= pp;
        MoveBlk( (byte*)theRemainP, (byte*)theModuleP, dsize );
        theModuleP=     theRemainP;
    } /* loop */
    
    /* --- bad module */
    release_module(mid,false); /* forget it again */

    /* Which error a failed LOAD reports depends on how far we got.
     *
     * A load is asked about a FILE NAME, and the name may be anything at all --
     * on this emulator a host directory routinely holds .c, .bas and .txt files
     * beside real modules, so "the sync word isn't MODSYNC" usually means "not a
     * module file", not "a damaged module". Answering E_BMCRC there would be
     * actively misleading and would make any command search spray module errors
     * over ordinary files; E_FNA ("without the correct access permissions --
     * check the file's attributes") is what OS-9 gives for a file you cannot
     * execute, and the load path does open with mode 0x05, read+exec. That is
     * why this collapsed to E_FNA, and for the not-a-module case it stays.
     *
     * But once MODSYNC has matched, that ambiguity is GONE: the file IS a module
     * and the parity/size/CRC/name check that just failed says exactly how it is
     * broken. Reporting E_FNA there sends the reader to the file's attributes and
     * owner for a fault that has nothing to do with either -- live-verified: one
     * flipped bit in a hand-built module gave 214 for a bad CRC and for bad header
     * parity alike, with a byte-identical copy loading clean as the control. The
     * v2.4 TRM gives E$BMCRC/E$BMHP for F$VModul, which performs precisely this
     * check, and says F$Load errors on "a module with a bad parity or CRC"; its
     * own definition of E$FNA rules 214 out. So pass the real error through.
     *
     * <linkstyle> was always exempt: a link names a module that is supposed to
     * already be one, so its caller's intent is not in doubt. */
    return os9error( (linkstyle || wasModule) ? err:E_FNA );
} // load_module_local



os9err load_module( ushort pid, char* name, ushort* midP, Boolean exedir )
{
  void* modBase = NULL;
  #ifdef INT_CMD
  Boolean isNative= false;
    isintcommand( name, &isNative, &modBase );
  #endif

  return load_module_local( pid, name, midP, exedir, false, modBase, 0,0,0 );
} // load_module



os9err link_module( ushort pid, const char* name, ushort* midP )
{
  os9err  err;
  Boolean isInt   = false;
  void*   modBase = NULL;

  char    lName[OS9PATHLEN];

  /* a name longer than any pathlist is no module's (the host command line
     reached this unchecked and overran lName, found by ASan) */
  if (strlen( name )>=sizeof(lName)) return E_BNAM;
  strcpy( lName, name );

  #ifdef INT_CMD
  Boolean isNative= false;
        isInt= isintcommand( lName, &isNative, &modBase )>=0 && !isNative;
    if (isInt && find_mod_id( lName )<MAXMODULES) {
      /* A genuine resident module must win over a same-named internal
         command: on a real OS-9, F$Link searches only the module directory.
         Without this check, a packed BASIC09 module group containing a
         procedure named after an internal command (e.g. "move") can never
         RUN it -- F$Link gets hijacked to 'OS9exec' and BASIC09 reports
         error 043 (unknown procedure). Internal-command substitution stays
         strictly as a fallback for names with no real module behind them. */
      isInt   = false;
      modBase = NULL;
    } // if
    if (isInt) {
      debugprintf(dbgModules,dbgNorm,
                 ( "# link_module: internal cmd '%s' => try 'OS9exec' instead\n", lName ));
      strcpy( lName,OS9exec_name );
    } // if
  #endif
    
      err= load_module_local( pid, lName, midP, true,true, modBase, 0,0,0 );
  if (err &&  isInt) {
//if (err && (isInt || (isNative && ( strcmp( lName,"hello_world" )==0) ||
//                                    strcmp( lName,"show"        )==0))) {
    *midP= 0; /* simulate load by using main module as result (it can't be unlinked!!!) */
    err  = 0;
    debugprintf(dbgModules,dbgNorm,
               ( "# link_module: 'OS9exec' not found, returned ptr to main module\n" ));
  } // if
  
//if (err && isNative) {
//  *midP= NextFreeModuleId( lName );
//} // if

//if    (err==E_MNF && isNative && strcmp( lName,"hello_world" )==0)
//       err= 0; // native programs can work without an os9 primary module 
  return err;
} /* link_module */


os9err link_load( ushort pid, char *name, ushort* midP )
/* try to link first, then try to load */
{
    os9err   err= link_module( pid, name, midP );
    if (err) err= load_module( pid, name, midP, true );
    return   err;
} /* link_load */


os9err load_OS9Boot( ushort pid )
{
  os9err err, cErr;
  ushort path;
  char   name[OS9PATHLEN];
  
  byte                  sect0[ 256 ];
  uint32_t size= sizeof( sect0 );
  ulong   pos, siz, scs;
  ushort  mid;

  process_typ* cp= &procs[ pid ];
  syspath_typ* spP;
  rbf_typ*     rbf;
  
  strcpy( name, cp->d.path );
  strcat( name, "@" );
  
  err=   usrpath_open( pid, &path, cp->d.type, name, 0x01 ); if (err) return err;
  
  do {
    err= usrpath_read( pid,  path, &size, &sect0,   false ); if (err) break;
    
    /* All three via the GET_OS9* accessors: sect0 is a byte[] local (alignment
     * 1), so the (ushort*) casts were potentially misaligned reads. The line
     * above already used GET_OS9L for the odd offset 0x15; these two were the
     * neighbours it missed. */
    pos= GET_OS9L(sect0, 0x15)>>BpB;
    siz= GET_OS9W(sect0, 0x18);
    scs= GET_OS9W(sect0, 0x68);
    
    if (pos==0) { err= E_PNNF; break; } // no sector 0 reference
    if (scs==0) scs= 256; // default
  //printf( "%08X %d: (%d)\n", pos, siz, scs );

          spP= get_syspathd( pid, cp->usrpaths[ path ] );
    rbf= &spP->u.rbf;
      
    if (siz==0) {          // large OS9Boot files => use as file
      spP->rawMode= false; // no longer raw device
      rbf->currPos= 0;     // initialize position to 0 */
      rbf->fd_nr= pos;
      err= ReadFD( spP ); if (err) break; // change structure to boot file
    }
    else {
      err= usrpath_seek( pid, path, pos*scs ); if (err) break;
    } // if
      
    err= load_module_local( pid, name, &mid, false,false,NULL, path, pos,siz );
  } while (false); // if
  
  cErr= usrpath_close( pid,  path ); if (!err) err= cErr;
  return err;
} // load_OS9Boot


#define ATTR_STICKY 0x4000   /* M$Attr bit 6, as the high byte of the attr/rev word */

void unlink_module( ushort mid )
/* unlink a module by ID. "When several modules are loaded together as a group,
   modules are only removed when the link count of all modules in the group have
   zero link counts" (F$UnLink, page 1-69): one that reaches zero while another
   member is still linked stays in the directory, and goes with the last of them. */
{
    ushort k, group;

    if (mid>=MAXMODULES)   return;
    if (os9mod(mid)==NULL) return;
   
    if (os9modules[mid].linkcount>1) { /* still used by other processes */
        os9modules[mid].linkcount--; return;
    }

    /* "A sticky module is retained in memory when its link count becomes
       zero. The module is removed from memory when its link count becomes -1
       or memory is required for another use" (M$Attr bit 6, Technical Manual,
       Module Header). So the last unlink leaves it in the directory at 0, and
       one more removes it. The init module's M$Compat bit 2 ("ignore sticky
       bit") is not read: os9exec does not take its compat flags from init. */
    if (os9modules[mid].linkcount==1 &&
        (os9_word( os9mod(mid)->_mh._mattrev ) & ATTR_STICKY)) {
        os9modules[mid].linkcount= 0;
        debugprintf(dbgModules,dbgNorm,("# unlink_module: mid=%d is sticky, kept at link count 0\n", mid));
        return;
    }
    
    if (mid==0) debugprintf(dbgModules,dbgNorm,
                           ("# unlink_module: attempt to unlink mid=0, **PREVENTED**\n"));

    group= os9modules[mid].group;
    for (k=0; k<MAXMODULES; k++) {
        if (k!=mid && os9mod(k)!=NULL && os9modules[k].group==group &&
            os9modules[k].linkcount>0) { os9modules[mid].linkcount= 0; return; }
    }
    for (k=0; k<MAXMODULES; k++) {
        if (k!=mid && os9mod(k)!=NULL && os9modules[k].group==group) release_module( k,true );
    }
    release_module( mid,true );
} /* unlink_module */


int release_sticky_modules( void )
/* Release every sticky module that no one has linked (link count 0) and
   answer how many went. Called when memory is short, which is the manual's
   other reason for one to go. */
{
    int k, n= 0;

    for (k=1; k<MAXMODULES; k++) {
        if (os9mod(k)!=NULL && os9modules[k].linkcount==0 &&
            (os9_word( os9mod(k)->_mh._mattrev ) & ATTR_STICKY)) {
            unlink_module( (ushort)k );
            if (os9mod(k)==NULL) n++;  /* a group member still linked keeps it */
        }
    }
    return n;
} /* release_sticky_modules */


void free_modules()
/* cleanup internal "module directory" */
{
   int k;
   for(k=0; k<MAXMODULES; k++) release_module(k,false);
} /* free_modules */



void init_exceptions(ushort pid)
/* initialize process' exceptions */
{
   /* no handler installed */
   int  k;
   for (k=0; k<NUMEXCEPTIONS; k++) procs[pid].ErrorTraps[k].handleraddr=0; 
} /* init_exceptions */


void init_traphandlers(ushort pid)
/* initialize process' exceptions */
{
   int  k;
   for (k=0; k<NUMTRAPHANDLERS; k++) {
        procs[pid].TrapHandlers[k].trapmodule= NULL; /* no traphandler module installed */
        procs[pid].TrapHandlers[k].trapentry = 0; /*IMPORTANT for new os9_llm implementation!! */
    }
} /* init_traphandlers */



ushort calc_parity(ushort *p,ushort numwords)
/* calculate header parity (if over 23 words) or check (over 24 words, must be 0) */
{
    ushort parity;
    
    parity=0;
    while (numwords>0) {
        parity ^= *p;   
        p++;
        numwords--;
    }
    parity = ~parity; /* invert */
    return parity;
} /* calc_parity */



uint32_t calc_crc(byte *p, uint32_t size, uint32_t accum)
/* calculate CRC over <size> bytes at <p>, starting with <accum> */
{
   uint32_t count,b,b1;
   int i,j;

   /* p can be FROM68K(a guest register); a guest passing address 0 (e.g. a bad
      F$CRC call) yields NULL -- return the CRC unchanged rather than dereference
      NULL and crash the host. */
   if (p==NULL) return accum;

   for (count=0;count<size;count++) {
      accum&=0x00FFFFFF;
      b=*(p++)<<16;
      b^=accum;
      accum<<=8;
      b1=(b>>=16);
      accum=accum^(b<<1)^(b<<6);
      j=0;
      for (i=0;i<8;i++) {
        if (b1&0x01)
          j++;
        b1>>=1;
      }
      if (j&0x01)
        accum^=0x00800021;
   } 
    return accum|0xFF000000; /* CRC value */
} /* calc_crc */



void mod_crc( mod_exec* m )
{
    uint32_t modsize= os9_long(m->_mh._msize);
    uint32_t crc;

    crc= calc_crc( (byte*)m,    modsize-4,0xFFFFFFFF );
    crc= calc_crc( (byte*)"\0", 1,        crc ); /* update with one additional 0 byte */
    crc=     ~crc; /* 1's complement */

    /* os9_set_l (memcpy-based), not a uint32_t* store: a module's base address
     * carries no 4-byte alignment guarantee, so `m+modsize-4` is routinely
     * misaligned and the old cast-and-store was undefined behaviour. UBSan on
     * s390x flagged it ("store to misaligned address ... requires 4 byte
     * alignment"). os9_set_l applies os9_long() itself, so pass crc raw. */
    os9_set_l( (byte*)m + modsize-4, crc ); /* assign now */

    debugprintf(dbgModules,dbgNorm,("# mod_crc: '%s' (size=%u): new CRC=$%08X\n",
                                       Mod_Name(m), modsize, crc ));
} /* mod_crc */



/* prepare data for execution of OS9 executable module/trap handler
 * returns msiz(actual size) and mp(actual base pointer, unbiased)
 */
os9err prepData(ushort pid, mod_exec *theModule, uint32_t memplus, uint32_t *msiz, byte **mp)
{
   uint32_t memsz, offs, cnt;
   uint32_t modSize, idOff, irOff, dOff;
   byte    *p, *p2, *bp, *modEnd, *bpEnd;
   os9err  why;
   int k;
   
   /* -- allocate memory for data */
   memsz=os9_long(theModule->_mdata); /* basic data size */
   debugprintf(dbgModules+dbgProcess,dbgDetail,("# prepData: Basic data size = %u\n",memsz));
   memsz+=os9_long(theModule->_mstack); /* stack data size */
   memsz+=EXTRAEMUSTACK; /* add some extra stack space, because emulation requires more than real OS-9 */
   memsz+=memplusall; /* additional memory for all processes */

   debugprintf(dbgModules+dbgProcess,dbgDetail,("# prepData: Basic + Stack data size = %u\n",memsz));
   memsz+=memplus; /* additional memory space */
   debugprintf(dbgModules+dbgProcess,dbgDetail,("# prepData: Basic + Stack + additional data size = %u\n",memsz));
   memsz=(memsz+15) & 0xFFFFFFF0; /* round up to next 16-boundary */
   debugprintf(dbgModules+dbgProcess,dbgNorm,("# prepData: Adjusted total data size = %u\n",memsz));

       bp=os9malloc( pid,memsz, &why ); /* allocate OS-9 memory block */
   if (bp==NULL) return why; /* E$NoRAM, or E$MemFul at the block limit */
   
   /* _midata/_midref are module-chosen offsets that os9exec turns into raw HOST
      pointers, exactly like _mname (guarded in load_module_local): an
      out-of-range one is an out-of-bounds host read, and the copy/relocation
      TARGETS computed from the tables are host writes.  A module reaches here
      only after F$Load, but nothing has yet checked these two offsets or the
      table contents against the module or the freshly allocated data block, so
      bound every host access below.  On any violation the module is malformed
      (E_BMID); free the data block first, as the E_NORAM path above does. */
   modSize= os9_long(theModule->_mh._msize);
   modEnd = (byte*)theModule + modSize;
   bpEnd  = bp + memsz;

   /* -- prepare initialized data */
   idOff= os9_long(theModule->_midata); /* idata */
   if ((uint64_t)idOff + 8 > modSize) { os9free(pid,bp,memsz); return os9error(E_BMID); }
   p2 = (byte *)theModule + idOff;
   dOff= GET_OS9L(p2, 0); /* offset into data space */
   p2+= 4; cnt= GET_OS9L(p2, 0); /* number of bytes to copy */
   p2+= 4;
   if ((uint64_t)idOff + 8 + cnt > modSize ||   /* source runs past module end */
       (uint64_t)dOff  + cnt     > memsz) {      /* dest runs past data block   */
      os9free(pid,bp,memsz); return os9error(E_BMID);
   }
   p= bp + dOff;

   debugprintf(dbgModules+dbgProcess,dbgDetail,("# prepData: idata at %p, data offset start=%p, bytecount=$%X\n",(void*)p2,(void*)p,cnt));
   /* `while (cnt-- >0)` copied the right number of bytes, but underflowed cnt to
      0xFFFFFFFF on the final test. Harmless (cnt is reassigned before its next
      use) yet it tripped -fsanitize=unsigned-integer-overflow on every startup,
      which made that sanitizer unusable as a gate. Decrement inside the body. */
   while (cnt>0) { *p++ = *p2++; cnt--; } /* copy initialized data */
   /* -- adjust initialized data and object pointers */
   irOff= os9_long(theModule->_midref); /* initalized data references */
   if (irOff > modSize) { os9free(pid,bp,memsz); return os9error(E_BMID); }
   p2  = (byte*)theModule + irOff;
   offs= TO68K(theModule); /* for first table, use code start address as offset (68k) */

   for (k=0;k<2;k++) {
      debugprintf(dbgModules+dbgProcess,dbgDetail,("# prepData: irefs correction to base address $%08X\n",offs));
      while (true) {
         if (p2+4>modEnd) { os9free(pid,bp,memsz); return os9error(E_BMID); } /* unterminated table */
         if (GET_OS9L(p2, 0)==0) break;
         p=bp + ((ulong)os9_word(*((ushort *)p2))<<16); /* calc group's base address */
         p2+=2; /* step over base address word */
         if (p2+2>modEnd) { os9free(pid,bp,memsz); return os9error(E_BMID); }
         debugprintf(dbgModules+dbgProcess,dbgDetail,("# prepData: irefs group at %p, count=%d\n",
            (void*) p,os9_word(*((ushort *)p2))));

         for (cnt= os9_word(*((ushort *)p2));cnt>0;cnt--) {
            p2+= 2; /* step to next offset word */
            if (p2+2>modEnd) { os9free(pid,bp,memsz); return os9error(E_BMID); }
            {  byte *fp= p+os9_word(*((ushort *)p2));
               if (fp<bp || fp+4>bpEnd) { os9free(pid,bp,memsz); return os9error(E_BMID); } /* wild write target */
               debugprintf(dbgModules+dbgProcess,dbgDetail,("# prepData: original value at %p = $%08X; offset=$%08X\n",
                   (void*)fp, GET_OS9L(fp, 0), offs));
               /* now correct: read 4-byte big-endian field, add offset, write back */
               SET_OS9L(fp, 0, GET_OS9L(fp, 0) + offs);
            }
         }
         p2+=2;
      }
      p2 += 4; /* skip 0 terminator */
      offs= TO68K(bp); /* for second table, use data base pointer as offset (68k) */
   }

   debugprintf(dbgModules+dbgProcess,dbgNorm,("# prepData: Finally allocated static for pid=%d:  %u Bytes at %p\n",pid,memsz,(void*)bp));
   *mp=bp;
   *msiz=memsz;
   return 0;
} /* prepData */



os9err install_traphandler( ushort pid, ushort trapidx,
                            char *mpath, uint32_t addmem, traphandler_typ **traphandler )
/* install a traphandler */
{
    mod_exec *theModule; /* OS-9 trap handler module header */
    traphandler_typ *tp;
    os9err err;
    ushort mid;

    debugprintf(dbgTrapHandler,dbgNorm,("# Installing Traphandler for pid=%d, Trap #%d, mpath='%s'\n",pid,trapidx+1,mpath));
    if (trapidx>=NUMTRAPHANDLERS) return os9error(E_ITRAP); /* invalid trap code */
    tp=&(procs[pid].TrapHandlers[trapidx]);
    if (tp->trapmodule!=NULL) return os9error(E_ITRAP); /* a handler is already installed for this trap */

    
    /* load from exe dir */
    err= link_load( pid,mpath,&mid );
    debugprintf(dbgTrapHandler,dbgNorm,("# install_traphandler: link_load('%s') for pid=%d returned err=$%02X\n",mpath,pid,err));
    if (err) return err;
    
    /* now prepare the trap handler data */
    theModule= get_module_ptr(mid);
    tp->mid=mid; /* save mid */
    /* mod_trap reads the same bytes as mod_exec plus the two trap-handler
       entries that follow them; link_load has already established this module
       is one. Same reading-not-conversion note as IsDesc in utilstuff.c. */
    tp->trapmodule=(mod_trap *) theModule; /* host pointer, used by high-level code */
    tp->trapentry= TO68K(theModule)+os9_long(theModule->_mexec); /* 68k entry address */

    /* --- module found, prepare as trap handler */
      { byte* trapdata;
        err= prepData(pid,theModule,addmem,&tp->trapmemsz,&trapdata);
        if (err) {
          /* tp->mid/tp->trapmodule were already set above so trapentry could
             be computed; on failure they must be rolled back to NULL/0, or
             this slot looks "installed" forever (OS9_F_TLink's already-
             installed check above would wrongly reject any future install),
             and release_traphandler will later call os9free() on this slot's
             still-zeroed trapmem/trapmemsz -- a spurious free of address 0 --
             while never unlinking the module this function itself linked. */
          unlink_module( mid );
          tp->trapmodule= NULL;
          tp->trapentry = 0;
          tp->mid       = 0;
          return err;
        }
        tp->trapmem= TO68K(trapdata); /* 68k offset of trap handler's static storage */
      }
    
    *traphandler= tp;
    return 0;
} /* install_traphandler */



os9err release_traphandler( ushort pid, ushort trapidx )
/* release a traphandler */
{
	traphandler_typ* tp;

	if (trapidx>=NUMTRAPHANDLERS) return os9error(E_ITRAP); /* invalid trap code */
	
	    tp=&procs[pid].TrapHandlers[trapidx];
	if (tp->trapmodule!=NULL) {
		/* release trap handler's static storage */
        os9free( pid, FROM68K(tp->trapmem), tp->trapmemsz );
       
        /* unlink trap handler's module */
		unlink_module( tp->mid );
		
		tp->trapmodule=NULL; /* no traphandler installed any more */
		tp->trapentry=0;     /* no traphandler entry available any more */
    } /* if */
     
    return 0;   
} /* release_traphandler */


void unlink_traphandlers( ushort pid )
/* unlink process' traphandlers */
{
    int  k;
    for (k=0; k<NUMTRAPHANDLERS; k++) release_traphandler(pid,k);
} /* unlink_traphandlers */



/* eof */
