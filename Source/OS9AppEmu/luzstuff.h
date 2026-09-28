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

/* required stuff to make UAE 68k emu work */


extern int savestate_wanted;
extern int quit_program;

void                upe_printf( const char* format, ... );
#define console_out upe_printf
#define write_log   upe_printf

extern struct uae_prefs currprefs;
extern void   custom_reset(void);
extern void   *xmalloc(size_t n);


/* The system tick on hosts with no SIGALRM (Windows, a browser). There the
   emulation loop reads the clock itself: every OS9_SOFT_TICK_INSTRS
   instructions it calls os9_soft_tick() (os9_tick.c), which ends the run
   exactly as the signal handler does elsewhere once a tick is due. One
   definition of which hosts those are, shared by both sides. */
#if defined _WIN32 || defined __EMSCRIPTEN__
  #define OS9_SOFT_TICK
  extern int os9_soft_budget; /* instructions left before the clock is read */
  void       os9_soft_tick( void );
#endif
