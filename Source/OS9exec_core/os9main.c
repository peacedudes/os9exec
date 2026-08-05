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
 *    Revision 1.30  2007/03/24 12:30:24  bfo
 *    <isRBF> dependency introduced
 *
 *    Revision 1.29  2007/01/28 21:27:21  bfo
 *    -k: use '*_dbg' plugin dlls / closing the last path correctly
 *
 *    Revision 1.28  2007/01/07 14:02:58  bfo
 *    Using MACOSX instead of __MACH__
 *
 *    Revision 1.27  2007/01/02 11:31:31  bfo
 *    2007 text adaption
 *
 *    Revision 1.26  2006/12/01 20:04:34  bfo
 *    Enhanced (and shorter) "systime" display
 *
 *    Revision 1.25  2006/11/12 21:08:18  bfo
 *    "-v" description added
 *
 *    Revision 1.24  2006/11/12 13:32:59  bfo
 *    "-?" text updated / <userOpt> adaption for MacOSX/Linux
 *
 *    Revision 1.23  2006/10/25 20:36:27  bfo
 *    <dbgAnomaly> flag will be switched on by default
 *
 *    Revision 1.22  2006/08/29 22:04:13  bfo
 *    an error of tcgetattr() will not terminate the program
 *    (this error happens e.g. with CW in debugger mode)
 *
 *    Revision 1.21  2006/08/20 18:34:59  MG
 *    restore_term(void) / setup_term(void) added
 *    (Changes done by Martin Gregorie)
 *
 *    Revision 1.20  2006/07/29 08:49:54  bfo
 *    "adaptor" => "adapter"
 *
 *    Revision 1.19  2006/06/17 14:19:44  bfo
 *    <CR> adaption
 *
 *    Revision 1.18  2006/06/17 11:09:47  bfo
 *    Global var <withTitle> introduced
 *
 *    Revision 1.17  2006/06/07 16:08:15  bfo
 *    XCode / IntelMacs supported now
 *
 *    Revision 1.16  2006/06/02 18:59:09  bfo
 *    Option -g activated
 *
 *    Revision 1.15  2006/06/01 21:04:39  bfo
 *    g_ipAddr things added
 *
 *    Revision 1.14  2006/02/19 15:47:30  bfo
 *    Header changed to 2006
 *
 *    Revision 1.13  2005/07/06 21:07:57  bfo
 *    defined UNIX
 *
 *    Revision 1.12  2005/07/02 14:23:42  bfo
 *    Adapted for Mach-O
 *
 *    Revision 1.11  2005/06/30 11:54:36  bfo
 *    Mach-O support
 *
 *    Revision 1.10  2005/01/22 16:19:37  bfo
 *    Windows path search adaption
 *
 *    Revision 1.9  2004/12/04 00:10:56  bfo
 *    MacOSX MACH adaptions
 *
 *    Revision 1.8  2004/11/20 11:44:08  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.7  2003/05/17 10:32:15  bfo
 *    UserOpt activated for Carbon (gs_ready problem, same as Linux version)
 *
 *    Revision 1.6  2002/10/09 20:14:03  bfo
 *    "show_timing" call with additional parameter <ticksLim>
 *
 *
 */

/* includes */
/* ======== */

#include "os9exec_incl.h"
#include <ctype.h>

#if defined UNIX && !defined MINGW
#include <termios.h>
#include <ctype.h>
#endif

#ifdef MINGW
#include <windows.h>  /* GetConsoleMode/SetConsoleMode -- mingw-w64 has no termios.h */
#include <io.h>       /* isatty */
#endif

/* statics */
/* ======= */

int   commentoutput;        /* commenting stdout */
int   disablefilters;       /* filtering stdout */
ulong memplus;              /* additional memory for first process */
ulong memplusall;           /* additional memory for all processes */
ulong iniprior;             /* priority for first process */

extern ulong emul_arena_size; /* 68k arena size (set via -M option) */

extern int   os9_tick_request;  /* tick interval in microseconds, 0 = no clock */

#if defined UNIX && !defined MINGW
struct termios savedmodes;  /* saved terminal attributes     */
#endif

#ifdef MINGW
static DWORD savedConsoleMode;  /* saved console input mode */
#endif

/* locally defined procedures */
Boolean F_Avail( const char* pathname );
void    restore_term(void);
Boolean setup_term(void);
/* -------------------------- */

/* default standard output filter */
/* ============================== */

/* This routine is called by OS9exec/NT to output to stdout/stderr whenever
 * no dedicated filter procedure could be found for the executing process.
 * This standard writeline() implementation preceedes each non-empty output
 * line with a MPW comment character "#", unless RAWOUTPUT is defined.
 * Outputting informational and progress messages in comment format is standard
 * for MPW utilities.
 */
void writeline(char *linebuf, FILE *stream)
{   
    /* --- output line first */
    if (*linebuf!=0) {
        if (commentoutput) {
            fputs("# ",stream);
        }
        fputs(linebuf, stream);
    }
    
    putc('\n',stream);
    fflush   (stream);
} /* writeline */

Boolean F_Avail( const char* pathname )
{
    struct  stat info;
    Boolean fOK, isd, ok;
    
         fOK= stat( pathname,&info )==0; /* file available ? */
    if (!fOK) {
        debugprintf(dbgStartup,dbgNorm,("# F_Avail '%s' (not found)\n", pathname ));
        return false;
    }                                                                
    
    isd= IsTrDir( info.st_mode ); /* it is a directory ? */
    ok = isd ||   info.st_size>0; /* size is > 0 */
    debugprintf(dbgStartup,dbgNorm,("# F_Avail '%s' %s (%s) size=%lld %08X\n", pathname, ok ? "true":"false",
                                                                        isd ? "dir":"file",
                                                                        (long long)info.st_size, info.st_mode ));
    return ok;
} /* F_Avail */

#ifdef MINGW
  /* egetenv()'s "is this already an absolute path?" checks below only test
   * for a leading PATHDELIM ('/'), which is right for the Wine-oriented
   * convention used elsewhere in this codebase (the realpath() shim strips
   * the drive letter so every host path is driveless-from-root). Native
   * Windows paths keep their drive letter ("C:/Users/...") and are never
   * stripped on this route, so without this check a perfectly valid,
   * already-absolute OS9DISK value gets wrongly treated as relative and
   * prefixed with startPath -- confirmed live: "C:/Users/x/demo/freeware"
   * became "C:/Users/x/demo/C:/Users/x/demo/freeware", found not to exist,
   * and os9exec reported E_MNF for every module on the disk. */
  static Boolean IsAbsHostPath( const char* p )
  {
      return *p==PATHDELIM || (isalpha((unsigned char)p[0]) && p[1]==':');
  } /* IsAbsHostPath */
#else
  #define IsAbsHostPath(p) (*(p)==PATHDELIM)
#endif

/* Build "<a><sep><b>" into a fixed buffer, refusing rather than overflowing.
   Returns false and empties <dst> when the result will not fit.

   This exists because egetenv() assembled paths out of startPath and the raw
   value of OS9DISK / OS9CMDS / OS9MDIR with strcpy+strcat into a 255-byte
   static, and an over-long environment variable simply wrote past it. Proven
   with AddressSanitizer 2026-08-05: OS9DISK set to a 305-character path gives
   "global-buffer-overflow ... WRITE of size 306" inside egetenv, reached
   straight from os9_main at startup. It did not crash without a sanitizer --
   the overrun lands in the adjacent static, which is worse than a crash,
   because the emulator then runs on quietly corrupted state. */
static Boolean joinPath( char* dst, size_t dstsz,
                         const char* a, const char* sep, const char* b )
{
    size_t need= strlen(a) + (sep!=NULL ? strlen(sep) : 0) + strlen(b) + 1;

    if (need>dstsz) {
        uphe_printf( "# path is %lu characters, over the %lu limit -- ignored\n",
                     (unsigned long)(need-1), (unsigned long)(dstsz-1) );
        *dst= NUL;
        return false;
    }

    strcpy( dst,a );
    if (sep!=NULL) strcat( dst,sep );
    strcat( dst,b );
    return true;
} /* joinPath */


char* egetenv( const char* name )
{
    static char tmp[OS9PATHLEN];
    static char ocm[OS9PATHLEN]; /* Can't use <tmp> recursively */

    #ifdef MACTERMINAL     
      int   k,i;
      char* p;
      char  v[OS9PATHLEN];
    
      #define ENVSIZE 3
      char    *environment[ENVSIZE+1] = { "OS9DISK=dd:", 
                                          "OS9CMDS=dd:CMDS:",
                                          "OS9MDIR=OS9MDIR:",
                                           NULL };
    #else
      Boolean u_disk, u_cmds, u_mdir;
      char*   q;
      char*   sv;      
      char*   rslt;
      Boolean cm= false;
      Boolean isRBF;
      
      #ifdef windows32
        Boolean isWin= true;
      #else
        Boolean isWin= false;
      #endif
    #endif

    #ifdef MACTERMINAL
      for (k=0; k<ENVSIZE; k++) {
          p= environment[k];

          for (i=0; *p!=0; i++) {
              if (*p=='=' && name[i]==NUL) {
                  p++;
                
                  memcpy( &v,&gFS.name, sizeof(gFS.name) );
                  p2cstr(  v );

                  if (*v!=NUL && k<2) {
                      strcat( v, ":"); /* mac path specific */

                      if (k==1) strcat( v,"CMDS:" );
                      p= v;
                  }

                  if (!joinPath( tmp,sizeof(tmp), startPath,NULL,p )) return NULL;
                  return  tmp; /* found */
              }

              if (*p++!=name[i]) break;
          } /* for */
      } /* for */

      return NULL; /* not found */
      
    #else
      /* getenv for use under MPW and for PC/Linux version */
      rslt= getenv( name );

      #ifdef MACOS9
        strcpy( tmp,rslt ); /* make a local copy !! */
        rslt=   tmp;
      #endif

      #ifdef win_unix
        u_disk= (ustrcmp( name,"OS9DISK")==0);
        u_cmds= (ustrcmp( name,"OS9CMDS")==0);
        u_mdir= (ustrcmp( name,"OS9MDIR")==0);

        if (u_disk || u_cmds || u_mdir) {
            if (rslt==NULL) {
                if (u_disk) {
                    strcpy( ocm,PATHDELIM_STR );
                    strcat( ocm,"dd" );
                    rslt=   ocm;
                }

                if (u_cmds) {
                    /* When OS9DISK is unset, this recursive call fills and
                       returns `ocm` ITSELF (see the u_disk branch above -- ocm
                       is a single shared static buffer, as its own declaration
                       comment admits). Copying that onto ocm is a
                       self-overlapping strcpy: undefined behaviour, and
                       Valgrind flagged it on the startup path of every run.
                       When OS9DISK *is* set the call returns getenv's own
                       storage and the copy is genuinely needed -- so test for
                       the aliasing case rather than assuming either way. */
                    char* base= egetenv("OS9DISK");

                    /* base can be NULL: egetenv now REFUSES a value too long
                       for its own buffer instead of overflowing it, and that
                       refusal reaches here. The pre-existing strcpy below
                       would have dereferenced it just as happily -- it was
                       simply unreachable while the overflow was the behaviour.
                       Nothing sensible can be derived from a disk that has no
                       usable name, so this says so the same way. */
                    if (base==NULL ||
                        strlen(base)+strlen(PATHDELIM_STR)+strlen("CMDS")
                            >= sizeof(ocm)) {
                        uphe_printf( "# OS9CMDS cannot be derived from OS9DISK"
                                     " -- unset or too long\n" );
                        return NULL;
                    }

                    if   (base!=ocm) strcpy( ocm,base );
                    strcat( ocm,PATHDELIM_STR );
                    strcat( ocm,"CMDS" );
                    rslt=   ocm;
                    cm  =  true;
                }

                if (u_mdir) {
                    strcpy( ocm,PATHDELIM_STR );
                    strcat( ocm,"OS9MDIR" );
                    rslt=   ocm;
                }
            }
            sv= rslt;

            isRBF= isWin && cm && IO_Type( 1, startPath, 0 )==fRBF;
          //if (!F_Avail(rslt) || isWin) some problems, mix is best of both worlds ??
            /* Even when a relative value happens to also resolve against
             * this process' own (often coincidental) working directory --
             * e.g. "h0" when os9exec was launched from the repo root --
             * still resolve it against startPath and save the real
             * absolute path, rather than leaving the shortcut the user
             * typed in place. Downstream code (GetCurPaths' non-RBF
             * fallback, MakeOS9Path) treats an already-"/"-prefixed
             * result as final and uses it directly; a bare relative
             * string surviving to there produces an OS-9-notation value
             * ("/h0") that still needs re-substituting every time it's
             * used as a directory prefix, rather than the resolved host
             * path -- confirmed live to cause a runaway repeated
             * re-resolution on a bare `dir` right after boot. */
            if (!F_Avail(rslt) || isRBF || ( isWin && !cm ) || !IsAbsHostPath(rslt)) {
              //upe_printf( "ty=%d fRBf=%d\n", IO_Type( 1, startPath, 0 ), fRBF ); // get device type: Mac/PC or RBF

                if (cm) rslt= "/dd/CMDS"; /* make it suitable for RBF devices */
                else {
                    debugprintf(dbgStartup,dbgNorm,("# startPath: '%s'\n", startPath));
                    if (!joinPath( tmp,sizeof(tmp), startPath,
                                   IsAbsHostPath(rslt) ? NULL : PATHDELIM_STR,
                                   "" )) return NULL;
                    #ifdef windows32
                      if (*rslt==PATHDELIM && tmp[ strlen(tmp)-1 ]==PATHDELIM) rslt++;
                    #endif
                    if (strlen(tmp)+strlen(rslt)+1 > sizeof(tmp)) {
                        uphe_printf( "# %s: path is too long once resolved -- ignored\n", name );
                        return NULL;
                    }
                    strcat( tmp,rslt );
                    rslt=   tmp;
                    debugprintf(dbgStartup,dbgNorm,("# startPath: '%s'\n", rslt));
                }
            } /* if */

            if (!cm && !F_Avail(rslt)) {
                rslt= sv;
                strcpy( tmp,startPath );
                q=      tmp+strlen(tmp)-1;

                /* Guard q>tmp: startPath is normally an absolute path (has
                 * a leading PATHDELIM to stop at), but a value with no
                 * delimiter at all would otherwise walk q below the start
                 * of tmp[], reading unmapped memory while hunting for a
                 * PATHDELIM that isn't there -- same reasoning as the
                 * guarded walk in filestuff.c's device-root resolution.
                 * Confirmed live: an empty startPath (see the MINGW
                 * StartDir() fix) crashed here with 0xC0000005. */
                while (*q!=PATHDELIM && q>tmp) q--;
                *q= NUL; /* cut the string at delimiter */

                if (strlen(tmp) + (IsAbsHostPath(rslt) ? 0 : strlen(PATHDELIM_STR))
                                 + strlen(rslt) + 1 > sizeof(tmp)) {
                    uphe_printf( "# %s: path is too long once resolved -- ignored\n", name );
                    return NULL;
                }
                if (!IsAbsHostPath(rslt)) strcat( tmp,PATHDELIM_STR );
                strcat( tmp,rslt );
                rslt=   tmp;
            } /* if */
        } /* if u_* */
      #endif
      
      debugprintf(dbgStartup,dbgNorm,("# egetenv %s: '%s'\n", name,rslt));
      return rslt;
    #endif
} /* egetenv */

/* advance cursor and force time slicing */
void eAdvanceCursor(void)
{
    #ifdef MACTERMINAL
      fflush(stdout);
    
    #elif defined macintosh
      #ifndef __INTEL__
      //RotateCursor(3*32); /* will rotate now */
      #endif
    #elif defined win_linux
      // sleep(1);
      
    #else
      #error not implemented
    #endif
} /* eAdvanceCursor */

/* spin cursor (event processing) */
void eSpinCursor (short incr)
{
    #ifdef MACTERMINAL
      static short count;
    
      count+=incr;
      if ((count & 0x1F)==0) {
        fflush(stdout);
        count=0;
      }
    #else
      #if defined MACOSX
        // sleep(1);
         
      #elif defined macintosh
        SpinCursor(incr);
        
      #elif defined windows32
        #pragma unused(incr)
        // sleep(1);
        
      #elif defined linux || defined MINGW
        // sleep(1);

      #else
        #error not implemented
      #endif
    #endif
} /* eSpinCursor */

/* os9 utility program */
/* =================== */

/* routines to display special wish text */
static void show_wish(void)
{
    upho_printf("I do not allow using this program to apply any kind of violence\n");
    upho_printf("to anybody.\n");
    upho_printf("As there is no absolute criterium for violent applications of software,\n");
    upho_printf("I trust your subjective interpretation - if it's honest for you, it's ok\n");
    upho_printf("for me.\n");
    upho_printf("Disregarding this statement means not respecting my intention I had when\n");
    upho_printf("writing this software and therefore would void your right to use this\n");
    upho_printf("program.\n");
    upho_printf("Lukas Zeller, September 1996\n");
     upo_printf("\n");
} /* show_wish */

/* display os9 usage */
static void os9_usage(char *name)
{
    /* print help */    
    upho_printf("Usage:    %s [options] [<os9program> [<os9parameters>,...]]\n",name);
    upho_printf("Function: Executes OS-9 program (which may start other os9 processes\n");
    upho_printf("          itself).\n");
    upho_printf("          <os9program> must be present within the tool's 'OS9C' resources,\n");
    upho_printf("          in the {OS9MDIR} directory or in the {OS9CMDS} directory\n");

    #if defined(windows32)
    upho_printf("Note:     Options can also be entered using the /-char instead of the hyphen\n");
    #endif

    upho_printf("Options:\n");
    #if defined(windows32)
    upho_printf("   -a num      Use SCSI adapter <num>. (IDE interfaces also count as SCSI)\n");
    upho_printf("   -b num      Use Bus <num> on specified SCSI adapter.\n");
    upho_printf("   -ah         Show all devices on all SCSI (and IDE) adapters\n");
    upho_printf("               Note that devices in use by Windows cannot be used by OS9exec\n");
    #endif
    upho_printf("   -c          comment output to MPW window (preceede lines with #)\n");
    upho_printf("   -i          disable internal commands\n");
    upho_printf("   -k          use '*_dbg' plugin dlls\n");      
    upho_printf("   -ih         show internal command help\n");
    upho_printf("   -o          disable output filtering (error message conversion)\n");        
    upho_printf("   -oh         show available output filters\n");      
    upho_printf("   -t          enable timing measurements\n");     
    upho_printf("   -m  n[k|M]  Give 1st OS-9 process extra static storage (kilo/mega)\n");
    upho_printf("   -mm n[k|M]  Give all OS-9 process extra static storage (kilo/mega)\n");
    upho_printf("   -M  n[k|M]  Set 68k arena size (default=32M)\n");
    upho_printf("   -p prio     Run  1st OS-9 process with prio (default=%d, NOIRQ>=%d)\n",MYPRIORITY,IRQBLOCKPRIOR);        
    upho_printf("   -q[ms]      Switch the system tick OFF (\"-q\" or \"-q0\"), or retune it\n");
    upho_printf("               (\"-q<ms>\"). The tick is ON by default at 10ms (100Hz), so\n");
    upho_printf("               processes are pre-empted. With it off a process keeps the CPU\n");
    upho_printf("               until it traps, so a dead loop hogs the machine and a cyclic\n");
    upho_printf("               alarm cannot reach a process that is computing. Only user\n");
    upho_printf("               state is pre-empted, never a system call.\n");
    upho_printf("   -d[n] msk   set  debug info mask [of level n, default=0] (default=1)\n");
    upho_printf("   -s msk      set  debug stop mask (default=0)\n");
    upho_printf("   -dh         show debug/stop mask help\n");
    upho_printf("   -n [name]   set name to trigger (causes debug stop)\n");
    upho_printf("   -w n        set waitnextevent() interval in 1/60 sec (default=%d)\n",DEFAULTSPININTERVAL);
    upho_printf("   -fe         use Fetch FTP name translation: '.' at filename beginning\n");
    upho_printf("               will be converted to '/' on Mac HFS)\n");       
    upho_printf("   -f[o]       use os9exec 1.14 dummy F$Fork emulation (no fork, but\n");      
    upho_printf("               only writing command line to stdout; -fo causes\n");        
    upho_printf("               cmd line generation for execution with 'os9' MPW tool)\n");     
    upho_printf("   -x width    define MGR screen width\n" );
    upho_printf("   -y height   define MGR screen height\n" );
    upho_printf("   -z          define MGR fullscreen mode\n" );
    upho_printf("   -g ip_addr  open   MGR screen at IP address\n");
    upho_printf("   -u          user defined option\n" );
    upho_printf("   -v          ctrl-C will stop OS9exec immediately\n" );
    upho_printf("   -r          run terminal output full speed (no baud-rate pacing)\n" );
    upho_printf("   -h[h]       show this help [and conditions for using the software]\n");
    upho_printf("\n");

    upho_printf("%s\n", OS9exec_Name() );
    upho_printf("1993-2007 by luz/bfo ( luz@synthesis.ch / bfo@synthesis.ch )\n");
     upo_printf("\n");
} /* os9_usage */

static void GetStartTick()
{
//  struct tm tim; /* Important Note: internal use of <tm> as done in OS-9 */
//  int    old_sec;
//    
//  GetTim( &tim );
//  old_sec= tim.tm_sec;
//    
//  do { /* this loop can take up to 1 second of time !! */
//      GetTim( &tim );
//  } while     (tim.tm_sec==old_sec);
//    
    startTick= 0; /* Initialize to 0, because subtracted in "GetSystemTick" */
    startTick= GetSystemTick();
    /* now the tick counter is synchronised to the second changing, as in OS-9 */
} /* GetStartTick */

/* Set the the terminal for unbuffered, no-echo operation.
   If errors are found they will be reported but the function
   exits normally so its caller can handle the cleanup. */
Boolean setup_term()
{
    int reply = 0;

    #if defined UNIX && !defined MINGW
      struct termios modes;

      if (!isatty(0)) return true; /* stdin is a pipe — skip terminal setup silently */

      reply = tcgetattr(0, &modes);           /* retrieve terminal attrs */
      if (reply == 0) {
          /*
              store the original modes so they can be restored
          */
          savedmodes = modes;

          /*
              change the attributes to set
              character at a time input and
              turn off echo.
          */

          modes.c_cc[VMIN] = 0;   /* non-blocking mode  */
          modes.c_cc[VTIME] = 1;  /* return after 100 mSec */
          modes.c_lflag &= ~ICANON;
          modes.c_lflag &= ~(ECHO | ECHOE | ECHOK /*| ECHOKE*/);
          modes.c_lflag &= ~ISIG; /* pass ^C/^Z as raw bytes; OS-9 handles signals */
          modes.c_iflag &= ~IXON; /* pass ^S/^Q as raw bytes too. We ARE the SCF driver
                                       here, and PD_XON/PD_XOFF are ours to consume
                                       (KeyToBuffer, utilstuff.c). With IXON left on, the
                                       HOST tty ate ^S before os9exec ever saw it and
                                       stopped accepting output, so the emulator blocked
                                       inside write(2) -- freezing every OS-9 process,
                                       the tick and pending alarms, not just the writer.
                                       Same reason hostterm_set_raw() clears it for /tN. */
          modes.c_oflag &= ~OPOST; /* SCF (consio.c) decides CR/LF on its own -- a host
                                       tty rewriting our LF bytes to CRLF (ONLCR, part of
                                       OPOST) silently breaks any termcap-driven program
                                       using a bare LF for pure cursor-down motion */

          /*
              set up the terminal for OS-9
          */
          reply = tcsetattr(0, TCSAFLUSH, &modes);
          if (reply) {
              upo_printf("Error %s: %s\n",
                         "setting up os9exec's terminal attributes",
                         strerror(errno));
          }
      }
      else {
          upo_printf("Error reading initial terminal settings: %s\n",
                     strerror(errno));
      }

    #elif defined MINGW
      /* mingw-w64 has no termios.h -- Windows Console API equivalent.
       * Clearing ENABLE_PROCESSED_INPUT (mirroring UNIX's ISIG-off above)
       * lets ^C arrive as a raw 0x03 byte through the normal read path
       * instead of firing the console control handler, matching how
       * consio.c already expects to see it on UNIX. */
      HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);

      if (!isatty(0)) return true; /* stdin is a pipe — skip terminal setup silently */

      if (!GetConsoleMode(hIn, &savedConsoleMode)) {
          reply = 1;
          upo_printf("Error reading initial terminal settings\n");
      }
      else {
          DWORD modes= savedConsoleMode
                       & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
          if (!SetConsoleMode(hIn, modes)) {
              reply = 1;
              upo_printf("Error setting up os9exec's terminal attributes\n");
          }
      }
    #endif

    return (reply == 0);
} // setup_term

/* Restore the original terminal operation.
   It is invoked as part of the exit() function, so
   thje error report isn't followed by an exit() */
void restore_term()
{
    #if defined UNIX && !defined MINGW
      int reply;
      struct termios modes;

      if (!isatty(0)) return; /* stdin is a pipe — nothing to restore */

      modes = savedmodes;
      reply = tcsetattr(0, TCSAFLUSH, &modes);
      if (reply)
         upo_printf("Error restoring normal terminal operation: %s\n",
                    strerror(errno));
    #elif defined MINGW
      HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);

      if (!isatty(0)) return; /* stdin is a pipe — nothing to restore */

      if (!SetConsoleMode(hIn, savedConsoleMode))
         upo_printf("Error restoring normal terminal operation\n");
    #endif
} // restore_term

/* The smallest arena the first process can actually be loaded into, and the
 * largest that still addresses as a 32-bit offset. Both are refusals rather
 * than clamps: silently honouring a different size than the one asked for is
 * how -M came to look like it worked in the first place. */
#define ARENA_MIN (1u*1024u*1024u)
#define ARENA_MAX 0x80000000u

static void preparse_arena_size( int argc, char** argv )
/* Applies -M <size> BEFORE os9exec_globinit() allocates the arena from it.
 *
 * -M was inert for two compounding reasons, and the first hid the second:
 *
 *  1. The main option switch lowercases its character (`tolower(*++p)`), so
 *     `case 'M'` could never be reached -- -M fell into `case 'm'` and set
 *     memplus, the FIRST PROCESS's extra static storage. That is why "-M 8M"
 *     looked like it worked (an 8 MB process still fits in the arena) while
 *     "-M 32M" died with "No more memory !!!": 32 MB of extra storage does not
 *     fit in a 32 MB arena. The arena was never resized either way, which is
 *     also why -M could only ever appear to SHRINK it.
 *  2. The option loop runs after os9exec_globinit(), so even a reachable case
 *     would have assigned emul_arena_size after init_all_mem had already
 *     calloc'd the arena from its default.
 *
 * This scan fixes (2); the main loop's own -M case, now reachable via the raw
 * option letter, fixes (1) and consumes the argument so it cannot reach
 * memplus. Parse errors are left to that case, which reports them properly --
 * this pass only bails out, so nothing is reported twice. */
{
    int           k;
    unsigned long val;
    char          modifier;
    const char*   q;

    for (k=1; k<argc; k++) {
        #ifdef windows32
          if (argv[k][0]!='-' && argv[k][0]!='/') continue;
        #else
          if (argv[k][0]!='-') continue;
        #endif
        if (argv[k][1]!='M' || argv[k][2]!=NUL) continue;
        if (k+1>=argc) return; /* missing argument: the main loop says so */

        q= argv[k+1];
        modifier= 0;
        if (*q=='$') { if (sscanf( q+1,"%lx%c", &val,&modifier )<1) return; }
        else         { if (sscanf( q,  "%lu%c", &val,&modifier )<1) return; }

        switch (tolower((unsigned char)modifier)) {
            case 'm' : val*= 1024; /* fall into Kbytes */
            case 'k' : val*= 1024;
            case  0  : break;
            default  : return; /* bad modifier: the main loop says so */
        } // switch

        if (val<ARENA_MIN || val>ARENA_MAX) {
            printf( "# Error: -M %s is outside the usable range (%uM..%uM)\n",
                       q, ARENA_MIN/(1024u*1024u), (unsigned)(ARENA_MAX/(1024u*1024u)) );
            exit( 1 );
        } // if

        emul_arena_size= val;
        return;
    } // for
} /* preparse_arena_size */

// main program
void os9_main( int argc, char **argv, char **envp )
{
  int     k, kX;
  char*   p;
  ulong*  ulp;
  /* scanf writes through the pointer you hand it, so its length modifier must
   * match the DESTINATION's real storage -- a cast cannot fix that. "%lu" writes
   * an `unsigned long`, which is 8 bytes on LP64 but only 4 on Windows (LLP64),
   * while `ulong` is 8 bytes on both. Scanning straight into a `ulong*` therefore
   * filled just half of it on Windows and left the top half uninitialized --
   * including for -M, the 68k arena size. Scan into a real `unsigned long`, then
   * assign. */
  unsigned long lnum;
  ushort* usp;
  char    modifier;
  char    optRaw;   /* the option letter as typed, before tolower */
  char*   toolname;
  ushort  level;
  ushort  err;
  Boolean no_app    = true;
  Boolean fullScreen= false;

  // set default options
  // - debug options
  debug[dbgNorm  ]= 1; // normal debug flags
  debug[dbgDetail]= 0;
  debug[dbgDeep  ]= 0;
  debughalt       = 0;
    
  // - other options
  commentoutput   = 0;
  disablefilters  = 0;
  memplus         = 0;
  dummyfork       = 0;
  fetchnames      = 0;
  iniprior        = MYPRIORITY;

  get_hw(); // make this very very early in program
  getversion( &exec_version, &exec_revision );
    
  #ifdef USE_UAEMU
    sw_name= "OS-9/68k Emu UAE";
  #else
    sw_name= "OS-9/68k Emulator";
  #endif
    
  #ifdef UNIX
    sec0= 0;
  #endif
    
  // -M sizes the arena that the next line allocates, so it cannot wait for
  // the option loop below. See preparse_arena_size.
  preparse_arena_size( argc,argv );

  // set up global init stuff, so user path output will work
  os9exec_globinit();
  GetStartTick();
    
  // start the logging system
  init_syscalltimers(); 

  // get arguments and options
  for (k=1; k<argc; k++) {
    p= argv[k];

    #ifdef windows32
      if (*p=='-' || *p=='/')
    #else       
      if (*p=='-')
    #endif
      {
        optRaw= *++p; /* -M vs -m: the only pair this switch must tell apart */
        switch (tolower((unsigned char)optRaw)) {
          case '?' :
          case 'h' :  if (*(p+1)=='h') { show_wish(); exit( 0 ); }
                      os9_usage( argv[0] );           exit( 0 );

          case 'f' :  if (*(p+1)=='e') { fetchnames=true; break; }
                      if (*(p+1)=='o') dummyfork= 2;
                      else             dummyfork= 1;
                      break;

          case 'i' :  if (*(p+1)=='h') { 
                        #ifdef INT_CMD
                          int_help( 0,0, NULL );
                        #endif 
                        exit( 0 ); 
                      } // if
                              
                      with_intcmds  = false; break;
          case 'k' :  with_dbgDLLs  = true;  break;
          case 'c' :  commentoutput = true;  break;
                
          case 'o' :  if (*(p+1)=='h') { printfilters(); exit( 0 ); }
                      disablefilters= true;  break;

          case 't' :  logtiming_disp= true;  break;

          case 'n' :  k++; /* next arg */
                      if (k>=argc) {triggername[0]=0; break; }
                      /* -1 and terminate: the name comes straight off the
                         command line and can be any length. */
                      strncpy(triggername,argv[k],TRIGNAMELEN-1);
                      triggername[TRIGNAMELEN-1]=0;
                      break;

          case 's' :  usp=&debughalt; goto getmask;

          #if defined RBF_SUPPORT && defined windows32
            case 'a' :  defSCSIAdaptNo = -1; // default to none
                        if (*(p+1)=='h') { scsiadaptor_help(); exit(0); }
                        k++; /* next arg */
                        if (k>=argc) { printf("# Error: missing SCSI Adapter Number for '%c' option!\n",*p); exit(1); }
                        p=argv[k];
                        if (sscanf(p,"%hd", &defSCSIAdaptNo)!=1) {
                          printf("# Error in SCSI Adapter number '%s'\n",p);
                          exit( 1 );
                        }
                        break;
                        
            case 'b' :  defSCSIBusNo = 0; // default to first bus
                        if (*(p+1)=='h') { scsiadaptor_help(); exit(0); }
                        k++; /* next arg */
                        if (k>=argc) { printf("# Error: missing SCSI Bus Number for '%c' option!\n",*p); exit(1); }
                        p=argv[k];
                        if (sscanf(p,"%hd", &defSCSIBusNo)!=1) {
                          printf("# Error in SCSI Bus number '%s'\n",p);
                          exit( 1 );
                        } // if
                        break;
          #endif
                
          case 'd' :  level=0; /* default to level 0 */
                      if (*(p+1)=='h') { debug_help( 0,0,NULL ); exit(0); }
                      if (isdigit((unsigned char)*(p+1))) {
                        level=*(p+1)-0x30;
                        if (level>DEBUGLEVELS) level=0; /* default to 0 if level invalid */
                      } // if
                      usp=&debug[level]; goto getmask;

                      getmask:
                      k++; /* next arg */
                      if (k>=argc) { printf("# Error: missing mask(hex) for '%c' option!\n",*p); exit(1); }
                      p=argv[k];
                      if (sscanf(p,"%hx", usp)!=1) {
                        printf("# Error in hex mask '%s'\n",p);
                        exit( 1 );
                      } // if
                      break;

          case 'u' :  userOpt    =  true; break; // set user option
          case 'v' :  catch_ctrlC= false; break; // don not install a ctrl C handler
          case 'r' :  baud_throttle= false; break; // run full speed (no baud pacing)

          case 'q' :  /* -q[ms]: the system tick, ON by default. Bare "-q"
                       * SWITCHES IT OFF -- the flag exists to turn the clock
                       * off, so that is what naming it alone does; "-q<ms>"
                       * retunes the rate instead ("-q0" is the same as "-q").
                       *
                       * The default is on because real OS-9 has a clock: it is
                       * what stops a dead loop hogging the machine and lets a
                       * cyclic alarm reach a process that is computing.
                       * Without it OS9exec runs a process until it traps or
                       * faults and nothing interrupts one between two of its
                       * own instructions. This flag is the fallback if
                       * pre-emption ever turns out to break a workload; a
                       * system call is never cut in half either way, see
                       * os9_tick.c. */
                      /* Only records the request. Starting the clock here
                       * would arm it during option parsing, long before the
                       * emulator is up -- and a tick landing in the middle of
                       * start-up made os9exec die before it produced a single
                       * line of output, intermittently. It is started once the
                       * emulation loop is actually running: see os9exec_loop. */
                      os9_tick_request= 0;             /* -q alone: clock off */
                      if (p[1]>='0' && p[1]<='9')      /* -q<ms> */
                          os9_tick_request= atoi( &p[1] )*1000;
                      break;
          case 'z' :  fullScreen =  true; break; // full screen mode
 
          case 'g' :  if (g_ipAddr==NULL) {
                            k++; /* next arg */
                        if (k>=argc) {
                          printf("# Error: missing argument for '%s' option!\n",p);
                          exit( 1 ); 
                        } // if
                              
                        p= argv[ k ];
                                g_ipAddr= malloc( strlen( p )+1 );
                        if (g_ipAddr==NULL) {
                          printf("# Error: out of memory for '%s' option!\n",p);
                          exit( 1 );
                        } // if
                        strcpy( g_ipAddr, p );
                      } // if
                      break;

          case 'x' :  ulp=&screenW;      goto getlnum;
          case 'y' :  ulp=&screenH;      goto getlnum;
          case 'w' :  ulp=&spininterval; goto getlnum;
          case 'p' :  ulp=&iniprior;       goto getlnum;
          case 'm' :  /* -M and -m are DIFFERENT options, and this switch sees a
                       * lowercased letter -- so the raw one decides. A plain
                       * `case 'M'` here was unreachable, which silently turned
                       * every -M into -m. preparse_arena_size() has already
                       * applied -M; assigning it again is harmless and keeps
                       * the argument from falling through to memplus. */
                      if (optRaw=='M')   { ulp=&emul_arena_size; goto getlnum; }
                      if (*(p+1)=='m') { ulp=&memplusall; goto getlnum; }
                      ulp=&memplus;      goto getlnum;
                            
                      getlnum:
                          k++; /* next arg */
                      if (k>=argc) {
                        printf("# Error: missing argument for '%s' option!\n",p);
                        exit( 1 ); 
                      } // if

                      p= argv[ k ];
                      modifier=0;
                      lnum= 0;
                      if (*p=='$') {
                        if (sscanf(++p,"%lx%c", &lnum, &modifier)<1) {
                          printf("# Error in hex number '$%s'\n",p);
                          exit( 1 );
                        } // if
                      }
                      else {
                        if (sscanf(p,"%lu%c", &lnum, &modifier)<1) {
                          printf("# Error in decimal number '%s'\n",p);
                          exit( 1 );
                        } // if
                      } // if
                      *ulp= lnum; /* now store it, at the destination's real width */

                      switch (tolower((unsigned char)modifier)) {
                        case 'm' : *ulp *=1024; /* fall into Kbytes */
                        case 'k' : *ulp *=1024;
                        case  0  : break;
                        default  : printf("# Error in modifier: '%c', must be 'k', 'M' or none\n", modifier);
                                   exit( 1 );
                      } // switch                                                
                      break;

          default  :  printf("# Error: unknown option '%c'!\n",*p); 
                    os9_usage( argv[0] ); exit( 1 );
        } // switch
      } 
      else {
        no_app= false;
        kX= k;
        break;
      } // if
  } // for
    
  // not enough arguments
  if (no_app) {
    os9_usage( argv[0] );
    exit( 1 );
  }
  else {
    toolname= argv[ kX ];
  } // if

  if (fullScreen) {
    screenW= 0; // full screen mode
    screenH= 0;
  } // if

  #ifdef USE_CARBON
    userOpt= true; // currently misused for Spectrapot software, as long as BusyRead is not working
  #else
    if (userOpt) catch_ctrlC= false;
  #endif
    
  #ifdef UNIX
    // Set the terminal modes and hook mode restoration to the exit function.
    // If setting then fails, the error will already have been reported
    // for suitably detailed fault analysis but the function exits so
    // os9_main() can handle the cleanup: at present this is just to exit.
    if (setup_term())
    atexit( restore_term );
  #endif

  // now here starts the os9 command line: go execute
  debug_prep(); // make sure debug info is adjusted
            
  // don't print before setup
  err= os9exec_nt( toolname,argc-kX-1,argv+kX+1,envp, memplus,iniprior );
  if (logtiming_disp) show_timing( STIM_NONE, 1,false );
  
  // always one new line              
  upo_printf( "\n" );
    
  // end message, if <withTitle> = calling shell/sh
  if (withTitle) upho_printf( "OS-9 emulation ends here.\n" );
  
  fflush( stdout );
  exit  ( err );
} // os9_main
/* eof */
