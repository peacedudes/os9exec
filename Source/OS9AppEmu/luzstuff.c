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

// stuff to make UAE compile

#include "sysconfig.h"
#include "sysdeps.h"

#include "config.h"
#include "options.h"
#include "memory.h"
#include "readcpu.h"
#include "newcpu.h"
#include "compiler.h"

#if defined __MINGW32__ || defined __MINGW64__
/* sysdeps.h's `_WIN32` block (always true under mingw-w64) redefines
 * abort() to call write_log() -- a symbol the CodeWarrior-era Windows port
 * expected some GUI/console shell file to supply. cpuemu.c and readcpu.c
 * hit this abort() but don't include luzstuff.h, so the write_log ->
 * upe_printf substitution below never reaches them; give MINGW a real,
 * literal write_log() here (before that substitution takes effect for
 * this file too). This file is part of the standalone UAE tree (doesn't
 * include os9exec_incl.h), so this checks the compiler's own __MINGW32__/
 * __MINGW64__ predefine rather than our project's MINGW macro, which is
 * only visible to files that route through os9main_incl_precomp.h. */
#include <stdarg.h>
void write_log( const char* format, ... )
{
    va_list args;
    va_start( args, format );
    vfprintf( stderr, format, args );
    va_end( args );
} /* write_log */
#endif

#define extern
#include "luzstuff.h"
#undef extern



// Functions used by CPU emu
void customreset(void) {
    write_log("Custom reset called\n");
}


void *xmalloc( size_t n )
{
    void *a= malloc( n );
    if   (a==NULL) {
        write_log( "Virtual memory exhausted!\n" );
        abort();
    }
    
    return a;
}


int in_m68k_go= 0;


/* eof */

