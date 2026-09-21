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
/*                                            */
/*  Main program for MPW version              */
/*  (running on Apple Macintosh)              */
/*                                            */
/*                                            */
/*             O S 9 E x e c / NT             */
/*  Cooperative-Multiprocess OS-9 emulation   */
/*         for Apple Macintosh und PC         */
/*                                            */
/* (c) 1993-2000 by Lukas Zeller, CH-Zuerich  */
/*                  Beat Forster, CH-Maur     */
/*                                            */
/* email: luz@synthesis.ch                    */
/*        beat.forster@ggaweb.ch              */
/**********************************************/

/* includes */
/* ======== */
#include "os9exec_incl.h"



#ifndef win_unix
int          gConsoleID        =    0;
syspath_typ* g_spP             = NULL;
int          gLastwritten_pid  =    0;
ulong        gNetActive        =    0;
#endif


#ifdef __EMSCRIPTEN__
  extern char** environ;
#endif

int main(int argc,char **argv,char **envp)
{
     /* Emscripten calls main with TWO arguments, so the third parameter is
        whatever happened to be in that slot. prepParams walks it as an
        environment and reads off the end of linear memory -- a hard wasm trap
        where a native host would merely read rubbish. The hosted C runtime's
        own `environ` is the portable answer, and it is what every other
        platform's entry point here already passes. */
  #ifdef __EMSCRIPTEN__
     (void)envp;
     envp= environ;
  #endif
     os9_main(argc,argv,envp);
}
/* eof */
