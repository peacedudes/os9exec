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

#ifdef NET_SUPPORT
  #include "net_platform.h"
#endif

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
 
/* OS9exec builtin module, defined as constant array */
const byte OS9exec_mod[] = {
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
    


/* "init" builtin module, defined as constant array */
const byte Init_mod[] = {
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


/* "socket" builtin module, defined as constant array */
const byte Socket_mod[] = {
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


/* "le0" builtin module, defined as constant array */
const byte Le0_mod[] = {
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
            hiword( b ) = (ushort)modK->linkcount;

            en->m1  = os9_long( TO68K(mod) );
            en->m2  = en->m1;               /* %%% module groups not yet supported */
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


void FillTemplate( mod_exec *m, short access, short tylan, short attrev )
/* default values for data module */
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

                sprintf( exeo,"%8X",           os9_long(mod->_mexec )       );
                sprintf( dats,"%7.2fk", (float)os9_long(mod->_mdata )/KByte );
                sprintf( stck,"%7.2fk", (float)os9_long(mod->_mstack)/KByte );

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
                    sprintf(adrs, " $%08X", TO68K(mod));

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


// %%% simply forget the memory of the resources !! 
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


int find_mod_id( const char* name )
/* find module by name, return mid or MAXMODULES if not found */
{
    char     *p;
    mod_exec *mod;
    
    int  k;
    for (k=0; k<MAXMODULES; k++) {
        /* compare name */
            mod= get_module_ptr(k);
        if (mod==NULL) continue; /* no module here, check next */
        
        p= Mod_Name( mod );
        if (ustrcmp(p,name)==0) return k; /* module found */
    } /* for */
    
    return MAXMODULES;
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



static void adapt_init( mod_exec* mh )
/* special treatment for the "init" module: set version+revision */
/* and connect it to the globals */
{
    byte*   bp;
    char*   nm;
    ushort* sp;
    
    bp= (byte*)  mh + 0x57; *bp= exec_version;
    bp= (byte*)  mh + 0x58; *bp= exec_revision;
    bp= (byte*)  mh + 0x59; *bp= 0;

    nm= (char*)  mh + 0x4c;          strcpy( nm,hw_site );
    bp= (byte*)  mh + 0x50;     sp= (ushort*)bp;
    nm= (char*)  mh + os9_word(*sp); strcpy( nm,hw_name );
    bp= (byte*)  mh + 0x5A;     sp= (ushort*)bp;
    nm= (char*)  mh + os9_word(*sp); strcpy( nm,sw_name );
    init_module= mh;
    mod_crc    ( mh );
} /* adapt_init */



static void adapt_le0( mod_exec* mh, uint32_t inetAddr )
{
    byte*  bp;

    bp= (byte*) mh;
    SET_OS9L(bp, 0x7a, inetAddr);  /* broadcast address position */
    *(byte*)(bp + 0x7d)= 0xff;     /* specific for broadcast */

    SET_OS9L(bp, 0x8a, inetAddr);  /* my internet address position */

    mod_crc( mh );
} /* adapt_le0 */



static void fill_s( char** b, char* s )
{
    sprintf( *b, "%s", s );
    *b=      *b + strlen( *b )+1;
} /* fill_s */



static void go_thru_list( char* v0, char* b0, uint32_t inetAddr )
/* adapt "localhost" at the "inetdb" module */
{
    char      *v, *b, *blk, *bBlk;
    uint32_t  *ipa;
    short      i, jump;
    
    short   n     = os9_word( *(short*)v0 );
    Boolean lFound= false;
    
    *(short*)b0= *(short*)v0;
    
    v0+= sizeof(short); /* skip the number of entries entry */
    b0+= sizeof(short);
    
    v= v0;
    for (i=0; i<n; i++) {
        blk= v;             v+= sizeof(short);    jump= os9_word( *(short*)blk );
        ipa= (uint32_t*)v;  v+= sizeof(uint32_t); /* get the 4-byte inetaddr */

        while (true) {
            if (ustrcmp( v,"localhost" )==0) {
                lFound= true;
                if (*ipa==inetAddr) return; /* everything is perfect already */
            }

            v= v+strlen(v)+1;
            if (*v==NUL) break;
        } /* while */
        if (lFound) break;

        blk+= jump;
        v   = blk;
    } /* for */
    if (!lFound) return; /* probably not enough room to put "localhost" in */

    v= v0;
    b= b0;
    for (i=0; i<n; i++) {
        blk =             v;  v+= sizeof(short);    jump= os9_word( *(short*)blk );
        ipa = (uint32_t*)v;   v+= sizeof(uint32_t); /* get the 4-byte inetaddr */

        bBlk=         b;       b+= sizeof(short);
        memcpy(b, (byte*)ipa, sizeof(uint32_t)); b+= sizeof(uint32_t); /* copy 4-byte inetaddr */

    //  printf( "%3d %3d %08X '%s'\n", i, jump, os9_long( *ipa ), v );

        fill_s( &b, v );
        if (*ipa==os9_long( inetAddr )) fill_s( &b, "localhost" );
        fill_s( &b, ""         ); /* one additional NUL char */
        
        if ((ulong)b%2==1) b++; /* make address even */
        *(short*)bBlk= os9_word( (short)(b-bBlk) );

        blk+= jump;
        v   = blk; 
    } /* for */
} /* go_thru_list */



static void adapt_inetdb( mod_exec* mh, uint32_t inetAddr, uint32_t dns1, uint32_t dns2, char* domainName )
/* the module "inetdb" (part of Internet Support Package ISP) will be adapted according */
/* to the OS9exec's host machine settings: <inetAddr> <dns1> <dns2> and <domainName>    */
{
    short   *hp;
    char    *bp, *b0, *bL, *v0;
    uint32_t d, size;
    byte    *h;
    char    sv[ OS9NAMELEN ];

    
 /* ------- hosts field adaption --------- */    
    b0= (char* ) mh + OFFS_HOSTS;
    b0= (char* ) mh + os9_long( *(uint32_t*)b0 ); /* get start position of "hosts" field */

    bL= (char* ) mh + OFFS_HOSTS + sizeof(uint32_t);
    bL= (char* ) mh + os9_long( *(uint32_t*)bL ); /* get end   position of "hosts" field */
  
                      size= bL-b0;
    v0=      get_mem( size );
    memcpy( v0,b0,    size );
    memset(    b0, 0, size ); /* clear the whole original field */

    go_thru_list( v0,b0, inetAddr );     /* rearrange the field */

    release_mem( v0 );

    
        
 /* ------- DNS field adaption --------- */    
    bp = (char* ) mh + OFFS_DNS;
    bp = (char* ) mh + os9_long( *(uint32_t*)bp ); /* get start position of "resolv.conf" field */
    bp+= 2;       hp= (short*)bp;
    bp+= 2;

    strcpy( sv, bp );                   /* make a copy of the existing domain name */
    memset( bp, 0, os9_word( *hp )-2 );                 /* clear the original area */
    
    if  (strcmp( domainName,"" )==0) domainName= sv;
    fill_s( &bp, domainName );                       /* fill in the domain name */
    
                                  d= os9_long( dns1 );
                                  h= (byte*) &d;
    sprintf( bp, "%d.%d.%d.%d",   h[0],h[1],h[2],h[3] ); /* fill in DNS IP address */
    bp=  bp + strlen( bp )+1;

    if (dns2!=0) {                d= os9_long( dns2);
                                  h= (byte*) &d;
        sprintf( bp, "%d.%d.%d.%d",   h[0],h[1],h[2],h[3] ); /* fill in DNS IP address */
        bp=  bp + strlen( bp )+1;
    }
                        
    fill_s( &bp, ""         ); /* one additional NUL char */
    fill_s( &bp, domainName ); /* fill in the domain name */
    
    
 /* --- update module CRC --- */          
    mod_crc( mh );
} /* adapt_inetdb */



static void adapt_L2( mod_exec* mh )
/* special treatment for the "L2" module: set port address */
{
    mod_dev* dsc;
    
    dsc= (mod_dev*)mh;
    dsc->_mport= os9_long( TO68K(&l2.hw_location) );
    
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
                strcpy(pathbuf,mdirPath);
                strcat(pathbuf,PATHDELIM_STR ); /* at least Linux requires this separator */
                strcat(pathbuf,name);

                    stream= fopen(pathbuf,"rb"); /* open for read, binary mode (bfo) */
                if (stream!=NULL) goto streamload;
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
              mid, (void*) theModuleP, (uint32_t)os9_long( *(uint32_t*)theModuleP )));
   
        os9modules[mid].linkcount= 1; /* module is loaded and linked */
        
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
         
        /* --- module loaded is ok */
        /* now check if we already have something like this in our module dir */
        nullterm(realmodname,Mod_Name( theModuleP ),MODNLEN);
        debugprintf(dbgModules,dbgNorm,
          ("# load_module: Name of module loaded='%s'\n",realmodname));

        /* %%% replacement with higher revision number is not yet implemented */
        os9modules[mid].modulebase=0; /* temporarily disable entry */
        
            oldmid= find_mod_id( realmodname );
        if (oldmid<MAXMODULES) {
            /* there is already another module with the same name */
            os9modules[oldmid].linkcount++; /* link the old one */
            os9modules[mid].modulebase=theModuleP; /* re-enable entry */
            release_module(mid, false); /* forget it again */
            *midP= oldmid;
            return 0; /* ... and throw away just loaded module */
        }
          
        os9modules[mid].modulebase= theModuleP; /* re-enable entry */

         
        /* special treatment for the "le0" and "inetdb" module: set internet address */
        if (ustrcmp(realmodname,"le0"   )==0 ||
            ustrcmp(realmodname,"inetdb")==0) {
        	#ifdef NET_SUPPORT
            err= MyInetAddr( &my_inetaddr, &dns1,&dns2, domainName ); /* assign my internet address */
          #endif
        } /* if */
              
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
    return os9error( linkstyle ? err:E_FNA );
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
  strcpy( lName, name );

  #ifdef INT_CMD
  Boolean isNative= false;
        isInt= isintcommand( lName, &isNative, &modBase )>=0 && !isNative;
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
  ushort* sp;
  ushort* sc;
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
    
    pos= GET_OS9L(sect0, 0x15)>>BpB;
    sp= (ushort*)&sect0[ 0x18 ]; siz= os9_word(*sp);
    sc= (ushort*)&sect0[ 0x68 ]; scs= os9_word(*sc);
    
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


void unlink_module( ushort mid )
/* unlink a module by ID */
{
    if (mid>=MAXMODULES)   return;
    if (os9mod(mid)==NULL) return;
   
    if (os9modules[mid].linkcount>1) { /* still used by other processes */
        os9modules[mid].linkcount--; return;
    }
    
    if (mid==0) debugprintf(dbgModules,dbgNorm,
                           ("# unlink_module: attempt to unlink mid=0, **PREVENTED**\n"));
                           
	release_module( mid,true ); }
/* unlink_module */


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

    *((uint32_t*)((uintptr_t)m+modsize-4))= os9_long(crc); /* assign now */

    debugprintf(dbgModules,dbgNorm,("# mod_crc: '%s' (size=%u): new CRC=$%08X\n",
                                       Mod_Name(m), modsize, crc ));
} /* mod_crc */



/* prepare data for execution of OS9 executable module/trap handler
 * returns msiz(actual size) and mp(actual base pointer, unbiased)
 */
os9err prepData(ushort pid, mod_exec *theModule, uint32_t memplus, uint32_t *msiz, byte **mp)
{
   uint32_t memsz, offs, cnt;
   byte    *p, *p2, *bp;
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

       bp=os9malloc( pid,memsz ); /* allocate OS-9 memory block */
   if (bp==NULL) return os9error(E_NORAM);    /* not enough RAM */
   
   /* -- prepare initialized data */
   p2 = (byte *)theModule+os9_long(theModule->_midata); /* idata */
   p  =    bp + GET_OS9L(p2, 0); /* offset into data space */
   p2+= 4; cnt= GET_OS9L(p2, 0); /* number of bytes to copy */
   p2+= 4;

   debugprintf(dbgModules+dbgProcess,dbgDetail,("# prepData: idata at %p, data offset start=%p, bytecount=$%X\n",(void*)p2,(void*)p,cnt));
   while (cnt-- >0) *p++ = *p2++; /* copy initialized data */
   /* -- adjust initialized data and object pointers */
   p2  = (byte*)theModule+os9_long(theModule->_midref); /* initalized data references */
   offs= TO68K(theModule); /* for first table, use code start address as offset (68k) */

   for (k=0;k<2;k++) {
      debugprintf(dbgModules+dbgProcess,dbgDetail,("# prepData: irefs correction to base address $%08X\n",offs));
      while (GET_OS9L(p2, 0) != 0) {
         p=bp + ((ulong)os9_word(*((ushort *)p2))<<16); /* calc group's base address */
         p2+=2; /* step over base address word */
         debugprintf(dbgModules+dbgProcess,dbgDetail,("# prepData: irefs group at %p, count=%d\n",
            (void*) p,os9_word(*((ushort *)p2))));

         for (cnt= os9_word(*((ushort *)p2));cnt>0;cnt--) {
            p2+= 2; /* step to next offset word */
            {  byte *fp= p+os9_word(*((ushort *)p2));
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

    debugprintf(dbgTrapHandler,dbgNorm,("# Installing Traphandler for pid=%d, Trap #%d\n",pid,trapidx+1));
    if (trapidx>=NUMTRAPHANDLERS) return os9error(E_ITRAP); /* invalid trap code */
    tp=&(procs[pid].TrapHandlers[trapidx]);
    if (tp->trapmodule!=NULL) return os9error(E_ITRAP); /* a handler is already installed for this trap */

    
    /* load from exe dir */ 
    err= link_load( pid,mpath,&mid ); if (err) return err;
    
    /* now prepare the trap handler data */
    theModule=(mod_exec *) get_module_ptr(mid);
    tp->mid=mid; /* save mid */
    tp->trapmodule=(mod_trap *) theModule; /* host pointer, used by high-level code */
    tp->trapentry= TO68K(theModule)+os9_long(theModule->_mexec); /* 68k entry address */

    /* --- module found, prepare as trap handler */
      { byte* trapdata;
        err= prepData(pid,theModule,addmem,&tp->trapmemsz,&trapdata);
        if (err) return err;
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
