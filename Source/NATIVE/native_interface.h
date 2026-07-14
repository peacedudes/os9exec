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
 *    Revision 1.1  2007/01/28 23:13:41  bfo
 *    NATIVE "hello_world" added
 *
 *
 */


// OS9exec native programs
#ifndef __native_interface_h
#define __native_interface_h


#ifndef  __GLOBDEF
#define  __GLOBDEF
  typedef unsigned char  byte;
  typedef unsigned short os9err;

  #if defined __INTEL__ || defined __MACH__ || defined __GNUC__
    typedef char Boolean;
    #define true  1
    #define false 0
  #endif

  /* Same LLP64 caveat as c_access.h: `ulong` is the pointer-width native word,
   * and this header's Call_Intercept() passes a host pointer in one. mingw is
   * __GNUC__, where plain `unsigned long` is only 32 bits, so it needs the
   * `unsigned long long` branch to agree with os9main_incl_precomp.h. Dormant
   * today (this file is built only under PTOC_SUPPORT/NATIVE_SUPPORT, neither
   * of which is enabled for mingw), but the trap is identical -- keep in step. */
  #if defined __MINGW32__ || defined __MINGW64__
    typedef unsigned long long ulong;
  #elif defined __MACH__ || defined __GNUC__
    typedef unsigned long  ulong;
  #endif

  #if !defined __MACH__ || defined __GNUC__
    typedef unsigned short ushort;
  #endif

  #define VHPCNT
#endif //__GLOBDEF

#ifndef VHPCNT
  typedef unsigned short ushort;
#endif 



// Visual Studio support
#ifdef _MSC_VER
  #define _ENTRY_ _declspec(dllexport)
#else
  #define _ENTRY_
#endif


#ifdef __cplusplus
  extern "C" {
#endif

// The plugin's version/revision
_ENTRY_ long Module_Version( void );


/* 
 * Go thru the list of native programs of this library.
 * <i> must be intialize to 0 before called the first time.
 * it will be incremented with each successful call.
 *
 * output params: <progName> native program's name
 *                <callMode> ''    normal
 *                           '-'   put it into exclude list
 *                           '+'   put it into include list
 *                           'I'   direct call (not using 'StartMain'
 *                           'I-'  direct call / exclude list 
 *                           'I+'  direct call / include list 
 *
 * (bool) result: true,  as long as there are valid elements
 *                false, if no more elements
 */
_ENTRY_ int Next_NativeProg( int *i, char* progName, char* callMode );


/*
 * Check, if <progName> is a native program; abs paths allowed as input
 *
 * output param : <modBase> (can be NULL, if not available
 * (bool) result: true, if native program with this name
 *
 */
_ENTRY_ int Is_NativeProg( const char* progName, void** modBase );


/*
 * Start native <progName>
 * <nativeInfo> contains <pid> reference, <modBase>, <os9_args> and callback structure
 */
_ENTRY_ os9err Start_NativeProg( const char* progName, void* nativeinfo );


/*
 * Intercept callback
 */
_ENTRY_ os9err Call_Intercept( void* routine, short code, ulong ptr );


#ifdef __cplusplus
  } // end extern "C"
#endif


#endif // __native_interface_h
/* eof */
