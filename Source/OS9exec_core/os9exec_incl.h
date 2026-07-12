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
 *    Revision 1.19  2006/06/07 16:11:55  bfo
 *    Partly beautified
 *
 *    Revision 1.18  2006/05/26 14:28:32  bfo
 *    Use always "procid_adapted.h"
 *
 *    Revision 1.17  2006/05/19 17:58:58  bfo
 *    Use only procid_adapted
 *
 *    Revision 1.16  2006/03/12 19:25:24  bfo
 *    string.h included, according to Martin Gregorie's proposal
 *
 *    Revision 1.15  2006/02/19 16:06:29  bfo
 *    os9defs adaptions
 *
 *    Revision 1.14  2005/07/15 22:22:06  bfo
 *    "module_special.h" no longer needed
 *
 *    Revision 1.13  2005/06/30 15:46:13  bfo
 *    Adapted for universal use
 *
 *    Revision 1.12  2005/06/30 15:32:33  bfo
 *    Adapted for universal use
 *
 *    Revision 1.11  2005/06/30 11:49:38  bfo
 *    Mach-O support
 *
 *    Revision 1.10  2005/01/22 16:12:19  bfo
 *    Renamed to ifdef MACOS9
 *
 *    Revision 1.9  2004/12/04 00:02:42  bfo
 *    MacOSX MACH adaptions
 *
 *    Revision 1.8  2004/11/20 11:44:08  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.7  2003/05/21 20:23:39  bfo
 *    Use "procid_special.h" with *except[10] instead of *_except[10]
 *
 *    Revision 1.6  2002/09/11 17:32:04  bfo
 *    Import all os9defs files from here / adapt it for "errno_adapted.h"
 *
 *    Revision 1.5  2002/09/03 18:39:38  bfo
 *    include "errno_adapted.h"
 *
 *    Revision 1.4  2002/07/30 16:46:14  bfo
 *    E-Mail adress bfo@synthesis.ch       is updated everywhere
 *
 *    Revision 1.3  2002/06/25 20:44:33  luz
 *    Added /lp printer support under windows. Not tested or even compiled for Mac
 *
 */

/* includes for all project modules */
/* ================================ */

#ifndef OS9EXEC_INCL_H
#define OS9EXEC_INCL_H

// include global options (external file to make
// things more clear to look at, specific for each target)
// #include "target_options.h"


// Note: Inclusion of all non-OS9exec includes has been moved
//       into separate _precomp file to make them precompilable
#include "os9main_incl_precomp.h"


// don't define this twice
#undef E_ABORT

/* OS-9 include files */
#ifdef MPW
/* search path for MPW is different */
  #include ":os9defs:module.h"         /* must be before "procid.h" */
  #include ":os9defs:procid_adapted.h" /* _except -> except */
  #include ":os9defs:sgstat.h"
  #include ":os9defs:errno_adapted.h"  /* errno_adapted.h: errno -> errno2 */
#else
  #include <string.h>  /* according to Martin Gregorie's proposal */
  #include "module_from_book.h"
  #include "procid_from_book.h"
  #include "sgstat_from_book.h"
  /* undef POSIX names that errno_from_book.h redefines with OS-9 values */
  #undef EWOULDBLOCK
  #undef EINPROGRESS
  #undef EALREADY
  #undef EDESTADDRREQ
  #undef EMSGSIZE
  #undef EPROTOTYPE
  #undef ENOPROTOOPT
  #undef EPROTONOSUPPORT
  #undef ESOCKNOSUPPORT
  #undef EOPNOTSUPP
  #undef EPFNOSUPPORT
  #undef EAFNOSUPPORT
  #undef EADDRINUSE
  #undef EADDRNOTAVAIL
  #undef ENETDOWN
  #undef ENETUNREACH
  #undef ENETRESET
  #undef ECONNABORTED
  #undef ECONNRESET
  #undef ENOBUFS
  #undef EISCONN
  #undef ENOTCONN
  #undef ESHUTDOWN
  #undef ETOOMANYREFS
  #undef ETIMEDOUT
  #undef ECONNREFUSED
  #undef EBUFTOOSMALL
  #undef ESMODEXISTS
  #undef ENOTSOCK
  #include "errno_from_book.h"
#endif

#undef E_ABORT


// Note: These have been renamed to OS9_Exxx to avoid potential clashes
//       with host (MSL) codes.
#define OS9_EWOULDBLOCK       0x701	/* I/O operation would block */
#define OS9_EINPROGRESS       0x702	/* I/O operation now in progress */
#define OS9_EALREADY          0x703	/* operation already in progress */
#define OS9_EDESTADDRREQ      0x704	/* destination address required */
#define OS9_EMSGSIZE          0x705	/* message too long */
#define OS9_EPROTOTYPE	      0x706	/* protocol wrong type for socket */
#define OS9_ENOPROTOOPT	      0x707	/* bad protocol option */
#define OS9_EPROTONOSUPPORT   0x708	/* protocol not supported */
#define OS9_ESOCKNOSUPPORT	  0x709	/* socket type not supported */
#define OS9_EOPNOTSUPP	      0x70a	/* operation not supported on socket */
#define OS9_EPFNOSUPPORT      0x70b	/* protocol family not supported */
#define OS9_EAFNOSUPPORT      0x70c	/* address family not supported by protocol */
#define OS9_EADDRINUSE	      0x70d	/* address already in use */
#define OS9_EADDRNOTAVAIL     0x70e	/* can't assign requested address */
#define OS9_ENETDOWN	      0x70f	/* network is down */
#define OS9_ENETUNREACH	      0x710	/* network is unreachable */
#define OS9_ENETRESET	      0x711	/* network dropped connection on reset */
#define OS9_ECONNABORTED      0x712	/* software caused connection abort */
#define OS9_ECONNRESET	      0x713	/* connection reset by peer */
#define OS9_ENOBUFS		      0x714	/* no buffer space available */
#define OS9_EISCONN		      0x715	/* socket is already connected */
#define OS9_ENOTCONN	      0x716	/* socket is not connected */
#define OS9_ESHUTDOWN	      0x717	/* can't send after socket shutdown */
#define OS9_ETOOMANYREFS      0x718	/* too many references */
#define OS9_ETIMEDOUT	      0x719	/* connection timed out */
#define OS9_ECONNREFUSED      0x71a	/* connection refused by target */
#define OS9_EBUFTOOSMALL      0x71b	/* mbuf too small for mbuf operation */
#define OS9_ESMODEXISTS	      0x71c	/* socket module already attached */
#define OS9_ENOTSOCK	      0x71d	/* path is not a socket	*/

// this is a badie, but seemed to work for earlier CWs. For CW7 and
// up we don't want it here any more!
#if !defined errno
  extern int errno;
#endif


/* low_level magic */
/* one single lowlevel include file for all types */
/* use either UAE emulator */
/* or  built-in interface to Mac 68k emulator */
#ifndef __cplusplus
  #include "os9_ll.h"
#endif


/* project headers */
/* --------------- */

/* OS9 main loop */
#include "os9main.h"

/* OS9 specific definitions */
#include "os9funcs.h"

/* OS9 emulator core */
#include "c_access.h"
#include "os9exec_nt.h"
#include "alarms.h"
#include "consio.h"
#include "printer.h"
#include "debug.h"
#include "events.h"
#include "funcdispatch.h"
#include "modstuff.h"
#include "memstuff.h"
#include "procstuff.h"
#include "telnetaccess.h"
#include "vmod.h"
#include "utilstuff.h"

#ifdef MACOS9
  #ifdef MPW
    #include "::Platforms:MAC:macfiles.h"
  #else
    #include "macfiles.h"
  #endif

#elif defined linux || defined MACOSX
  #include "linuxfiles.h"

#elif defined MINGW
  #include "winfiles.h"
#endif

#include "filestuff.h"
#include "fcalls.h"
#include "icalls.h"
#include "filters.h"

/* external utilities */
#include "intcommand.h"

#endif
/* eof */


