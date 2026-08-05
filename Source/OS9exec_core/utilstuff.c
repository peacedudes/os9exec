// 
//    OS9exec,   OS-9 emulator for Mac OS, Windows and Linux 
//    Copyright (C) 2002 Lukas Zeller / Beat Forster
//    Available under http://www.synthesis.ch/os9exec
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
 *    Revision 1.61  2007/04/17 23:10:57  bfo
 *    - My_Ino => My_FD
 *    - Flush_Dir, Flush_Entry, Flush_FDCache added (not yet active)
 *    - Several bug fixes for "CutUp"
 *    - FD Cache adaptions for RBF (not yet active)
 *
 *    Revision 1.60  2007/04/10 22:15:16  bfo
 *     Linux file names are treated case sensitively again
 *    (only Hash itself can't distinguish)
 *
 *    Revision 1.59  2007/04/07 09:05:14  bfo
 *    pStart will be displayed as process state "b"
 *
 *    Revision 1.58  2007/03/31 12:18:34  bfo
 *    - LINKED_HASH support and bug fixes (NULL initialisation)
 *    - extended <liCnt> range
 *
 *    Revision 1.57  2007/03/24 13:02:45  bfo
 *    - "HashF" hash function (using CRC) added
 *    - Hash function prepared for LINKED_HASH
 *    - Speedup for TFS directory reading
 *    - "StrReplace" function for Windows "con" problem ( => ".con" )
 *
 *    Revision 1.56  2007/03/10 12:41:13  bfo
 *    Use "malloc" instead "get_mem" here (avoid memory scattering)
 *
 *    Revision 1.55  2007/02/24 14:06:57  bfo
 *    - FD_ID: Additional params <dirid> and <fName>
 *    - MacOSX no longer based on Inodes (same as Linux)
 *    - Consistent m->ident key handling for MacOS9
 *
 *    Revision 1.54  2007/01/28 21:33:40  bfo
 *    'ustrcmp' made invisible / DirEntry dbgNorm -> dbgDetail
 *
 *    Revision 1.53  2007/01/07 13:17:25  bfo
 *    Use MACOS9 define
 *
 *    Revision 1.52  2006/12/02 12:13:33  bfo
 *    global <lastsyscall> eliminated
 *
 *    Revision 1.51  2006/12/01 20:05:44  bfo
 *    "MountDev" with <devCopy> support ( -d )
 *
 *    Revision 1.50  2006/10/12 19:50:23  bfo
 *    Extended for ".DS_Store" support
 *
 *    Revision 1.49  2006/08/20 18:35:29  bfo
 *    "noecho" mode is default again
 *    (Changes done by Martin Gregorie)
 *
 *    Revision 1.48  2006/07/23 14:21:43  bfo
 *    Quick return if no state change
 *
 *    Revision 1.47  2006/07/21 07:12:54  bfo
 *    task switch dbg display is now dbgDetail only
 *
 *    Revision 1.46  2006/06/13 19:17:29  bfo
 *    /p /p1 are recognized as printer devices
 *
 *    Revision 1.45  2006/06/11 22:02:32  bfo
 *    set_os9_state with 3rd param <callingProc>
 *
 *    Revision 1.44  2006/06/10 10:21:21  bfo
 *    t1..t49 no longer visible for Linux
 *
 *    Revision 1.43  2006/05/16 13:11:20  bfo
 *    Linux full path name adaption
 *
 *    Revision 1.42  2006/02/20 21:40:31  bfo
 *    no echo for Linux /term ; cTime/cDate assignment implented in a clean way
 *
 *    Revision 1.41  2006/02/19 16:36:23  bfo
 *    use <isIntUtil> / echo mode off by default
 *
 *    Revision 1.40  2005/07/06 21:05:19  bfo
 *    defined UNIX
 *
 *    Revision 1.39  2005/07/02 14:20:36  bfo
 *    Adapted for Mach-O / time correction for files / Volume name catch extension
 *
 *    Revision 1.38  2005/06/30 11:45:17  bfo
 *    Mach-O support
 *
 *    Revision 1.37  2005/04/15 11:40:44  bfo
 *    RBF_ImgSize implemented here now
 *
 *    Revision 1.36  2005/01/22 16:13:38  bfo
 *    Renamed to ifdef MACOS9
 *
 *    Revision 1.35  2004/12/04 00:07:26  bfo
 *    MacOSX MACH adaptions / small transferBuffer for Linux
 *
 *    Revision 1.34  2004/11/27 12:08:58  bfo
 *    _XXX_ introduced
 *
 *    Revision 1.33  2004/11/20 11:44:08  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.32  2004/10/22 23:00:25  bfo
 *    Most of the "pragma unused" eliminated
 *
 *    Revision 1.31  2003/05/05 17:57:02  bfo
 *    Activate most of the ramDisk things even without RAM_SUPPORT
 *
 *    Revision 1.30  2003/04/25 19:31:30  bfo
 *    Handle Netatalk IDs out of range correctly now (using FD_ID)
 *
 *    Revision 1.29  2002/11/06 20:08:23  bfo
 *    zero is zero for os9_word/os9_long
 *
 *    Revision 1.28  2002/10/27 23:16:39  bfo
 *    get_mem/release_mem no longer with param <mac_asHandle>
 *
 *    Revision 1.27  2002/10/15 17:58:21  bfo
 *    'CutUp' visible again (for linuxfiles.c)
 *
 *    Revision 1.26  2002/10/02 18:52:54  bfo
 *    GetScreen function is now defined at "utilstuff"
 *
 *    Revision 1.25  2002/09/22 20:54:34  bfo
 *    CutUp handling corrected /
 *    Min nr of dir entries is 2 ( ".." and "." )
 *
 *    Revision 1.24  2002/09/19 21:59:39  bfo
 *    "CutUp" invisible, "CutUp"/"Eatback" adapted for Win/Mac + OS9
 *
 *    Revision 1.23  2002/09/11 17:22:51  bfo
 *    os9_long_inc with unsigned int* param
 *
 *    Revision 1.22  2002/08/13 21:24:17  bfo
 *    Some more variables defined at the real procid struct now.
 *
 *    Revision 1.21  2002/08/13 15:15:23  bfo
 *    The state <dead> will be handled correctly now (state=0x9100)
 *
 *    Revision 1.20  2002/08/09 22:39:21  bfo
 *    New procedure set_os9_state introduced and adapted everywhere
 *
 *    Revision 1.19  2002/08/06 07:59:45  luz
 *    Fixed "Retry/Abort" pop-up dialog
 *
 *    Revision 1.18  2002/07/30 16:45:37  bfo
 *    E-Mail address beat.forster@ggaweb.ch is updated everywhere
 *
 *    Revision 1.17  2002/07/24 22:33:53  bfo
 *    Timer synchronisation enhanced
 *
 *    Revision 1.16  2002/07/23 19:40:33  bfo
 *    go back to the 1/60th seconds system, synchronise with <sycorr>
 *
 *    Revision 1.15  2002/07/21 15:11:13  bfo
 *    Up to date again
 *
 *    Revision 1.14  2002/07/21 14:37:16  bfo
 *    Extended 'Get_Time' function
 *
 *    Revision 1.13  2002/07/07 22:08:21  bfo
 *    take the RAM disk size from the descriptor (if available)
 *
 *    Revision 1.12  2002/07/06 15:56:42  bfo
 *    ram disks (also other names than /r0) can be mounted/unmounted with "mount"
 *    the size -r=<size> is prepared, but not yet active
 *
 *    Revision 1.11  2002/07/05 22:45:54  uid82848
 *    E_FULL problem fixed
 *
 *    Revision 1.10  2002/07/03 21:41:47  bfo
 *    RAM disk is now operational ( currently fixed to 2MB )
 *
 *    Revision 1.9  2002/07/02 18:55:39  bfo
 *    Preparations done for RAM disk /r0 implementation
 *
 *    Revision 1.8  2002/07/01 21:28:11  bfo
 *    p2cstr and c2pstr implemented correctly for Carbon version
 *
 *    Revision 1.7  2002/06/30 10:41:56  bfo
 *    check, if KeyToBuffer might cause an overflow -> seems not to be the case
 *
 *    Revision 1.6  2002/06/25 20:44:33  luz
 *    Added /lp printer support under windows. Not tested or even compiled for Mac
 *
 */

/* glibc gates realpath() behind __USE_MISC, which _GNU_SOURCE enables.
   It must be defined before the first system header -- reached via
   os9exec_incl.h below -- or the include guard locks realpath() out.
   Harmless on Darwin, which declares realpath() unconditionally. */
#define _GNU_SOURCE

#include "os9exec_incl.h"
#include <ctype.h>

/* General Utility routines */
/* ======================== */

#include <utime.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>    /* the UNIX errno -> OS-9 error mapping in host2os9err */
#include <stdlib.h>   /* realpath() (see _GNU_SOURCE note above); target_options.h only pulls this in for __MACH__ */

#ifdef MINGW
#include <io.h>       /* _access, _fileno, _get_osfhandle */
#include <windows.h>  /* HANDLE, GetFinalPathNameByHandleA -- see HostStreamWithinConfiguredDevice below */

/* mingw-w64 has no realpath() (POSIX-only). _fullpath() canonicalizes a
 * path but, unlike realpath(), doesn't require it to exist -- callers here
 * (HostPathDeviceName et al.) rely on realpath() failing so they can walk
 * up to the nearest existing ancestor, so the existence check below is
 * load-bearing, not decorative. */
static char* realpath( const char* path, char* resolved )
{
    char* p;

    if (_access(path,0)!=0) return NULL;
    if (_fullpath(resolved,path,PATH_MAX)==NULL) return NULL;

    /* _fullpath() returns Windows-native form ("Z:\Users\...\dd") --
     * this codebase represents every host path in Unix style (PATHDELIM
     * is '/' throughout, even under MINGW; see winfiles.c). Left as-is,
     * HostPathDeviceName's `real[rl]==PATHDELIM` check after a matched
     * prefix always fails against a literal '\\', silently rejecting
     * every subpath of a configured device root and falling back to the
     * root itself -- confirmed live: `dir /dd/CMDS/shell` resolved to
     * bare "/dd" instead of the file, tripping pFopen's is-a-directory
     * check (E_FNA) on an ordinary file open. Drop the "X:" drive
     * prefix (Wine always maps the host root to Z:) and flip \ to /
     * so the result matches every other path in this codebase. */
    p= resolved;
    if (p[0] && p[1]==':') p+= 2;
    if (p!=resolved) memmove(resolved,p,strlen(p)+1);
    for (p= resolved; *p; p++) if (*p=='\\') *p= '/';

    return resolved;
} /* realpath */
#endif



char* nullterm( char* s1, const char* s2, ushort max )
/* create null terminated version of s2. All chars<=SPACE will terminate s2
 * returns updated pointer (first char after end of string)
 */
{
    char *res= s1;
    ushort  n=  0;

    /* s2 is FROM68K(a guest register); a guest passing address 0 yields NULL
       here.  Treat it as an empty string instead of dereferencing NULL and
       crashing the host -- the caller then gets an empty name and fails with a
       normal error (E_MNF / E_BPNAM) downstream.  This one guard covers every
       F$ syscall that reads a path/name argument through nullterm(). */
    if (s2==NULL) { *s1= NUL; return (char*)s2; }

    /* TO68K, not a raw cast: regcheck() takes a 68k ARENA OFFSET (every other
     * caller passes a guest register or TO68K(...)), while s2 is a HOST pointer
     * (FROM68K of a guest register, per the comment above). Casting it straight
     * to regcheck's uint32_t truncated a 64-bit host pointer to its low 32 bits
     * under LLP64, so the RCHK_ARU/RCHK_MEM range checks were validating a
     * garbage value against the arena bounds -- the guard meant to catch
     * out-of-arena guest name pointers was effectively inert. Same "host pointer
     * stashed in something 32-bit" family as the earlier ulong/LLP64 fix;
     * surfaced here by clang's -Wshorten-64-to-32 on the ARM64 build. */
    regcheck( currentpid,"nullterm inptr",TO68K(s2),RCHK_ARU+RCHK_MEM );
    /* Copy at most max-1 chars so the terminating NUL at *s1 below still lands
       inside a destination buffer of size <max>. Every caller passes buf[SIZE]
       with max==SIZE, so the old "copy up to max chars, THEN write the NUL"
       wrote max+1 bytes and ran one past the buffer -- live: a path longer than
       OS9PATHLEN fed to `dir` overflowed icalls.c's 255-byte os9_path by that
       one NUL (ASan stack-buffer-overflow). `n+1<max` keeps the compare in int
       (no ushort underflow when max is 0). The source is still fully consumed,
       so the returned pointer is unchanged. */
    while(*s2>' ') {
        if (n+1<max) { *s1++= *s2++; n++; } /* reserve one byte for the NUL */
        else           s2++;                /* don't copy more, simply consume  */
    } /* while */

    *s1= NUL; /* terminate */
    trigcheck( "nullterm",res ); /* check for name trigger */
    return (char*)s2;
} /* nullterm */

/*
int ustrcmp( const char *s1,const char *s2 )
// case insensitive version of strcmp
//  Input  : *s1,*s2 = strings to be compared
//  Result : -1:s1<s2, 0:s1=s2, 1:s1>s2
//
{
    int  diff;
    char c;

    do {
        if ((diff=toupper((unsigned char)(c=*(s1++)))
                 -toupper((unsigned char)  *(s2++)))!=0) return diff>0 ?1:-1;
    } while (c!=NUL);

    return 0; // equal
} // ustrcmp
*/

int pustrcmp( const char *s1,const char *s2 )
/* case insensitive version of strcmp
 *  using PASCAL notation
 *  Input  : *s1,*s2 = strings to be compared
 *  Result : -1:s1<s2, 0:s1=s2, 1:s1>s2
 */
{
   int  len, diff, n;
   int  len1= s1[0];
   int  len2= s2[0];
   char c;

   if (len1<len2) { len= len1; }
                   len= len2;

   s1++; /* start with 1st char */
   s2++;
   
   for (n=1; n<=len; n++) {
      if ((diff=toupper((unsigned char)(c=*(s1++)))
               -toupper((unsigned char)  *(s2++)))!=0) return diff>0 ? 1:-1;
   }

   if (len1<len2) return -1;
   if (len1>len2) return  1;

   return 0; /* equal */
} /* pustrcmp */

int ustrncmp( const char *s1, const char *s2, ushort n )
/* case insensitive version of strcmp
 *  Input  : *s1,*s2 = strings to be compared
 *  Result : -1:s1<s2, 0:s1=s2, 1:s1>s2
 */
{
    int diff;
    char c;

    while (n-- > 0) {
        if ((diff=toupper((unsigned char)(c=*(s1++)))
                 -toupper((unsigned char)  *(s2++)))!=0) return diff>0 ? 1:-1;
        if (c==NUL) break;
    }
   
    return 0; /* equal */
} /* ustrncmp */

void os9_long_inc( unsigned int* a, uint32_t increment )
/* increment ulong field <a> dependent on big/little endian */
{   *a= os9_long( os9_long( *a ) + increment );
} /* os9_long_inc */

void set_os9_state( ushort cpid, pstate_typ state, const char* callingProc )
/* convert the state into OS-9 notation */
{
    process_typ* cp= &procs[cpid];
    procid*      pd= &cp->pd;
    if   (state==cp->state) return; // no change, ignore
    
    debugprintf(dbgTaskSwitch,dbgDetail,("#    state %d -> %d '%s'\n", 
                                           cp->state, state, callingProc ));
                                           
            cp->state= state;
    switch (cp->state) {
        case pStart    : pd->_state= os9_word(0x8800); pd->_queueid= 'b'; break;
        case pUnused   : pd->_state= 0;                pd->_queueid= '-'; break;
        case pActive   : pd->_state= os9_word(0x8800); pd->_queueid= 'a'; break;
        case pDead     : pd->_state= os9_word(0x9100); pd->_queueid= '-'; break;
        case pSleeping : pd->_state= os9_word(0xA000); pd->_queueid= 's'; break;
        case pWaiting  : pd->_state= os9_word(0x8000); pd->_queueid= 'w'; break;
        case pSysTask  : pd->_state= 0;                pd->_queueid= 't'; break;
        case pWaitRead : pd->_state= os9_word(0xA000); pd->_queueid= 'r'; break;
        case pWaitWrite: pd->_state= os9_word(0xA000); pd->_queueid= 'W'; break;
        default        : pd->_state= 0;                pd->_queueid= '?';
    } // switch
    
    if (cp->isIntUtil) { pd->_state = 0;               pd->_queueid = 'i'; }
} /* set_os9_state */

/* generates OS9 error (and saves traceback values) */
os9err os9error(os9err err)
{
    int dbg;
    process_typ* cp;
    char* name;

    errpid= currentpid; /* remember */
    
    if (debugcheck(dbgErrors,dbgNorm)) {
            cp=&procs[errpid];  dbg= dbgNorm;
        if (cp->state==pWaitRead) dbg= dbgDetail;
        
        get_error_strings(err, &name,NULL);
        debugprintf(dbgErrors,dbg,("# ** Error #%03d:%03d (%s), lastsyscall=%s, currentpid=%d\n",
                                          err>>8, err & 0xFF,name, 
                                          get_syscall_name(cp->lastsyscall), errpid ));
        if (lastpathparsed!=NULL) {
            debugprintf(dbgErrors,dbgDetail,("#    last path parsed=%s\n",lastpathparsed));
        } // if
    } // if

    return err;
} /* os9error */

/* translates mac OS error into OS-9 error codes */
os9err host2os9err(OSErr hosterr,ushort suggestion)
{
    os9err  err;
    Boolean known= true;

    #ifdef MACOS9
      if (hosterr==noErr) return 0;
    
      switch (hosterr) {
        case opWrErr            : /* same */
        case fBsyErr            : err=E_SHARE;  break;
        case fLckdErr           : err=E_FNA;    break;
        case dirNFErr           : /* same */
        case fnfErr             : err=E_PNNF;   break;
        case dskFulErr          : err=E_FULL;   break;
        case eofErr             : err=E_EOF;    break;
        case dirFulErr          : err=E_SLF;    break;
        case tmfoErr            : err=E_PTHFUL; break;
        case afpAccessDenied    : /* same */
        case permErr            : /* same */
        case wrPermErr          : err=E_FNA;    break;
        case vLckdErr           : /* same */
        case wPrErr             : err=E_WP;     break;
        case nsvErr             : err=E_UNIT;   break;
        case ioErr              : err=E_HARDWARE; break;
        case rfNumErr           : /* same */
        case fnOpnErr           : err=E_BPNUM;  break;
        case posErr             : err=E_NES;    break;
        case bdNamErr           : err=E_BPNAM;  break;
        case mFulErr            : err=E_NORAM;  break;
        case afpObjectTypeErr   : /* same */
        case dupFNErr           : err=E_CEF;    break;
        case gfpErr             : err=E_SEEK;   break;
        case readErr            : err=E_READ;   break;
        case abortErr           : err=E_PRCABT; break;
        default                 : err=suggestion; known=false;break;
      }
      debugprintf(dbgErrors,dbgNorm,("# ** Mac OSerr=%d%s\n",
                                      hosterr,known ? "" : "(not known, using suggestion)"));

    #elif defined(windows32)
      if     (hosterr==NOERROR) return 0;
      switch (hosterr) {
        case ERROR_PATH_NOT_FOUND: /* same */

        case ERROR_FILE_NOT_FOUND:         err=E_PNNF; break;

        case ERROR_NO_MORE_FILES:
        case ERROR_TOO_MANY_OPEN_FILES:    err=E_PTHFUL; break;

        case ERROR_OPEN_FAILED:
        case ERROR_NETWORK_ACCESS_DENIED:
        case ERROR_INVALID_ACCESS:
        case ERROR_SHARING_VIOLATION:
        case ERROR_NOACCESS:
        case ERROR_CANTOPEN:
        case ERROR_FILE_INVALID:
        case ERROR_ACCESS_DENIED:          err=E_FNA; break;

        case ERROR_INVALID_HANDLE:         err=E_BPNUM; break;

        case ERROR_INSUFFICIENT_BUFFER:
        case ERROR_BUFFER_OVERFLOW:
        case ERROR_STACK_OVERFLOW:
        case ERROR_OUTOFMEMORY:
        case ERROR_NOT_ENOUGH_MEMORY:      err=E_NORAM; break;

        case ERROR_PATH_BUSY:
        case ERROR_LOCK_VIOLATION:         err=E_SHARE; break;

        case ERROR_WRITE_PROTECT:          err=E_WP; break;

        case ERROR_DUP_NAME:
        case ERROR_FILE_EXISTS:            err=E_CEF; break;

        case ERROR_BAD_NETPATH:            err=E_BPNAM; break;

        case ERROR_NOT_READY:              err=E_NOTRDY; break;

        case ERROR_DRIVE_LOCKED:
        case ERROR_BUSY_DRIVE:             err=E_DEVBSY; break;

        case ERROR_SEEK:
        case ERROR_NEGATIVE_SEEK:
        case ERROR_SEEK_ON_DEVICE:
        case ERROR_INVALID_DRIVE:
        case ERROR_INVALID_BLOCK:
        case ERROR_SECTOR_NOT_FOUND:
        case ERROR_BAD_FORMAT:             err=E_SEEK; break;

        case ERROR_CRC:                    err=E_CRC; break;

        case ERROR_CANTWRITE:
        case ERROR_WRITE_FAULT:            err=E_WRITE; break;

        case ERROR_CANTREAD:
        case ERROR_READ_FAULT:             err=E_READ; break;

        case ERROR_HANDLE_EOF:             err=E_EOF; break;

        case ERROR_DISK_FULL:
        case ERROR_HANDLE_DISK_FULL:       err=E_FULL; break;

        case ERROR_DISK_CHANGE:            err=E_DIDC; break;

        case ERROR_CALL_NOT_IMPLEMENTED:   err=E_UNKSVC; break;

        case ERROR_UNRECOGNIZED_VOLUME:
        case ERROR_NO_VOLUME_LABEL:
        case ERROR_INVALID_NAME:           err=E_BNAM; break;

        case ERROR_DIR_NOT_EMPTY:          err=E_DNE; break;

        default: err=suggestion; known=false; break;
      }
      debugprintf(dbgErrors,dbgNorm,("# ** Win32 error=%d%s\n",
                                      hosterr,known ? "" : "(not known, using suggestion)"));

    #elif defined UNIX
      if     (hosterr==0) return 0;

      /* On UNIX <hosterr> is a POSIX RETURN CODE, not an error code: every
       * caller here passes the result of remove()/rename()/etc, which is
       * -1 on failure and carries no cause at all. (The OSErr values the
       * MACOS9 and windows32 branches switch on come from FSRead/FSpCreate
       * and GetLastError, which really are error codes -- only this branch
       * is handed a status.) So `err= suggestion` was not a lazy default;
       * it was the ONLY thing available, and the consequence was that every
       * host failure came back as whatever the caller guessed: `pFdelete`
       * passes E_SHARE, so a delete refused for permissions, a read-only
       * filesystem or a non-empty directory all reported "non-sharable file
       * busy". The cause was there the whole time, in errno -- just never
       * read. Do that here rather than at ~50 call sites.
       *
       * errno is fresh: we only consult it when the call reported failure,
       * and a failing libc call always sets it. The exception to know about
       * is an intervening libc call between the failure and this one -- the
       * debugprintf in pDsetatt's delete loop, say -- which is a no-op unless
       * that debug mask is enabled, so it cannot mislead a normal run.
       *
       * Deliberately conservative: only errnos whose OS-9 meaning is
       * unambiguous in EVERY calling context are mapped (there are ~50 call
       * sites passing 15 different suggestions). Anything context-dependent
       * -- EINVAL, EISDIR, EAGAIN -- still falls back to the caller's
       * suggestion, which knows which operation was attempted. */
      switch (errno) {
        case ENOENT:
        case ENOTDIR:       err=E_PNNF;     break;
        case EACCES:
        case EPERM:         err=E_FNA;      break;
        case EROFS:         err=E_WP;       break;
        case ENOTEMPTY:     err=E_DNE;      break;
        case EEXIST:        err=E_CEF;      break;
        case ENOSPC:        err=E_FULL;     break;
        case ENAMETOOLONG:  err=E_BPNAM;    break;
        case EIO:           err=E_HARDWARE; break;
        case EBUSY:
        case ETXTBSY:       err=E_SHARE;    break;
        case EMFILE:
        case ENFILE:        err=E_PTHFUL;   break;
        case ENXIO:
        case ENODEV:        err=E_UNIT;     break;
        case ESPIPE:        err=E_SEEK;     break;
        default: err= suggestion; known= false; break;
      }
      debugprintf(dbgErrors,dbgNorm,("# ** UNIX errno=%d%s\n",
                                      errno,known ? "" : " (not known, using suggestion)"));

    #else
      #error Unknown Target OS, no error translation implemented
    #endif

    return os9error(err);
} /* host2os9err */

os9err c2os9err(int cliberr,ushort suggestion)
/* translates mac/C-library error into OS-9 error codes */
{
   os9err err;
   
   if (!cliberr) return 0;
   switch (cliberr) {
      /* EPERM/ENOENT/ENOSPC/EIO are standard POSIX errno values, present and
       * usable on every live platform (verified: Linux gives 1/2/28/5). This
       * used to be gated `!defined(linux)` with a comment claiming Linux had
       * "no usable error codes" -- wrong, and the classic OS-as-proxy bug: it
       * meant Linux alone fell through to the generic `suggestion` for these
       * four, so e.g. a disk-full write returned E_WRITE instead of E_FULL, and
       * a missing file returned the caller's fallback instead of E_PNNF.
       * __MWERKS__ (dead CodeWarrior legacy) stays excluded, untouched. */
      #ifndef __MWERKS__
        case EPERM  : err=E_FNA;      break;
        case ENOENT : err=E_PNNF;     break;
        case ENOSPC : err=E_FULL;     break;
        case EIO    : err=E_HARDWARE; break;
      #endif

      default       : err=suggestion; break;
   }
    debugprintf(dbgErrors,dbgNorm,("# ** c2os9err: C-lib-err=%d\n",cliberr));
   return os9error(err);
} /* c2os9err */

uint32_t j_date(int d, int m, int y)
/* this routine returns the number of days
 * since January 1, 4713 B.C.              
 * it is used by the F$Time call.
 * expected <d>: 1..31 / <m>: 1..12 / <y>: XXXX 
 */
{ 
    long fct= 365*y + 31*(m-1) + d;
    int  yb = y-1; /* the year before */;

    if (m==1 || m==2) fct+= (yb/4) - ( 3*((yb/100)+1) /4);
    else              fct+= (y /4) - ((3*((y /100)+1))/4) - (4*m+23)/10;
    
    return fct + DAYS_SINCE_0000; 
} /* j_date */

void g_date(uint32_t jdn, int *dp, int *mp, int *yp )
/* this routine returns the date from julian day number */
{ 
  long fct;
  int  d, m, y, yb, fb;
  int  marr[ 12 ];
  
  
  fct= jdn - DAYS_SINCE_0000;    /* see above */
  y  = 4*(fct+15) / ( 4*365+1 ); /* 100/400 year adaption: 15 days */
  yb = y-1;                      /* the year before */
  fct= fct - 365*y - yb/4 + yb/100 - yb/400; 
  
  fb=28; if ( (y % 4)==0 ) fb=29;
  
  marr[  1 ]= 31; /* not so sophisticated as the julian conversion */
  marr[  2 ]= fb;
  marr[  3 ]= 31;
  marr[  4 ]= 30;
  marr[  5 ]= 31;
  marr[  6 ]= 30;
  marr[  7 ]= 31;
  marr[  8 ]= 31;
  marr[  9 ]= 30;
  marr[ 10 ]= 31;
  marr[ 11 ]= 30;
/*marr[ 12 ]= 31*/;
  
  m= 1; d= fct;
  while ( ( m<12 ) & ( d>marr[ m ] ) ) {
                    d= d-marr[ m ]; m++; }
   
  *yp= y;
  *mp= m;
  *dp= d;
} /* g_date */

void TConv( time_t u, struct tm* tim )
/* time conversion, seems to be buggy under CW7 -> 70 year correction */
{
	struct tm *tp;

	#if __MWERKS__ >= CW7_MWERKS && !defined __MACH__
	  u -= (72*365+17)*SecsPerDay; /* corrected bug of CW7, leap years included */
	#endif
	
    tp= localtime( (time_t*)&u );
    memcpy( tim,tp, sizeof(struct tm) ); /* copy it, as it might be overwritten */
	
	#if __MWERKS__ >= CW7_MWERKS && !defined __MACH__
	  tim->tm_year += 2; 
	#endif
} /* TConv */

time_t UConv( struct tm* tim )
/* time conversion, seems to be buggy under CW7 -> 70 year correction */
{
	time_t u;
	
	#if __MWERKS__ >= CW7_MWERKS && !defined __MACH__
	  tim->tm_year -= 2; 
	#endif

    u= mktime( tim );              /* set modification time */

	#if __MWERKS__ >= CW7_MWERKS && !defined __MACH__
	  u += (72*365+17)*SecsPerDay; /* corrected bug of CW7, leap years included */
	#endif
	
	return u;
} /* UConv */

void GetTim( struct tm* tim )
/* time conversion, seems to be buggy under CW7 -> 70 year correction */
{
	time_t     u;
	struct tm* tp;
	
    time                   ( &u );       /* get the current time, it seems to be   */
    tp = localtime( (time_t*)&u );       /* based on 1900, not 1904, as file dates */
    memcpy( tim,tp, sizeof(struct tm) ); /* copy it, as it might be overwritten    */
} /* GetTim */

void Get_Time( uint32_t *cTime, uint32_t *cDate, int *dayOfWk, int *currentTick,
               Boolean asGregorian, Boolean withTicks )
{
    struct tm tim; /* Important Note: internal use of <tm> as done in OS-9 */
  //byte   tc[4];
    int    ct0;
    int    y, m, d, tsm, ssm, syTick;
    
  //process_typ* cp= &procs[ currentpid ];
    
    /* Get consistent time/date and ticks */
    if      (withTicks) { syTick= GetSystemTick(); 
                    *currentTick= (syTick+syCorr) % TICKS_PER_SEC; }
    else            *currentTick= 0;
    
    do {
        if  (withTicks) ct0= *currentTick;
        
        GetTim( &tim );
        y=       tim.tm_year+1900;
        m=       tim.tm_mon +   1;
        d=       tim.tm_mday;
        
        if  (withTicks) { syTick= GetSystemTick();
                    *currentTick= (syTick+syCorr) % TICKS_PER_SEC; }
    } while (withTicks && *currentTick<ct0); // must be within the same second

    ssm= tim.tm_hour*3600
        +tim.tm_min *  60
        +tim.tm_sec;        /* seconds since midnight */
        
  //if (withTicks && cp->state!=pIntUtil) {
    if (withTicks) {
            tsm= (syTick+syCorr)/TICKS_PER_SEC;
        if (tsm>ssm) { 
        //upe_printf( "> %10d %10d %10d %10d %10d\n", tsm,ssm, syTick,syCorr,*currentTick );
          syCorr= (ssm+1)*TICKS_PER_SEC - syTick-1;
          *currentTick= (syTick+syCorr) % TICKS_PER_SEC;
        //upe_printf( "  %10d %10d %10d %10d %10d\n", tsm,ssm, syTick,syCorr,*currentTick );
        } // if
          
        if (tsm<ssm) {   
        //upe_printf( "< %10d %10d %10d %10d %10d\n", tsm,ssm, syTick,syCorr,*currentTick );
          syCorr=  ssm*   TICKS_PER_SEC - syTick;
          *currentTick= (syTick+syCorr) % TICKS_PER_SEC;
        //upe_printf( "  %10d %10d %10d %10d %10d\n", tsm,ssm, syTick,syCorr,*currentTick );
        } // if
    } // if
   
    if (asGregorian) {
		/* gregorian format */
		    *cTime= 0x10000*tim.tm_hour + 
		              0x100*tim.tm_min  +
		                    tim.tm_sec;
		    /*
        tc[0]= 0;
      	tc[1]= tim.tm_hour;
      	tc[2]= tim.tm_min;
      	tc[3]= tim.tm_sec;
      	*cTime= os9_long( *tcp );
        */
        
		    *cDate= 0x10000*y + 
		              0x100*m +
		                    d;
        /*
      	*((ushort*) &tc[0])= os9_word( y );
      	tc[2]= m;
      	tc[3]= d;
      	*cDate= os9_long( *tcp );
      	*/
      //debugprintf(dbgSysCall,dbgNorm,
      //       ("here we are: hour=%d %08X\n", tim.tm_hour, *cTime));      
    }
    else {
      	/* julian format */
      	*cTime= ssm;             /* seconds since midnight */
      	*cDate= j_date( d,m,y ); /* julian date, intenral clock starts 1904 */
    } // if

    *dayOfWk= tim.tm_wday; /* day of week, 0=sunday, 1=monday... */
} /* Get_Time */

ulong GetScreen( char mode )
/* Get screen dimensions: 'w'=width, 'h'=height */
{
    int r= 0; // no information, return zero (full screen)
    
    #ifdef windows32
      HWND dwh = GetDesktopWindow();
      RECT screenrec;

      if (GetWindowRect(dwh,&screenrec)) {
    	  switch (mode) {
    		  case 'w': r= screenrec.right -screenrec.left; break; /* return width  */ 
    		  case 'h': r= screenrec.bottom-screenrec.top;  break; /* return height */
    	  } 
      }
    #else
	  switch (mode) {
		  case 'w': r= 1600; break; /* return width  */ 
		  case 'h': r= 1200; break; /* return height */
	  } 
    #endif
    
    return (ulong)r;
} /* GetScreen */

/* ------------------------------------------------------------------------ */
#ifdef USE_CARBON
  char* p2cstr( unsigned char* s )
  {
      int ii;
      int n= (int)s[0];
      
      for  (ii=0;  ii<n; ii++) {
          s[ii]= s[ii+1];
      }
      s[n]= 0;
      
      return s;
  } /* p2cstr */

  unsigned char* c2pstr( char* s )
  {
      int ii;
      int n= 0;
      
      while (s[n]!=0) n++;
      for  (ii=n;  ii>0; ii--) {
          s[ii]= s[ii-1];
      }
      s[0]= (char)n;
      
      return s;
  } /* c2pstr */
#endif

ulong Min( ulong a, ulong b )
/* returns the smaller of two ulongs */
{   if (a<b) return a;
    else     return b;
} /* Min */

ulong Max( ulong a, ulong b )
/* returns the larger of two ulongs */
{   if (a>b) return a;
    else     return b;
} /* Max */

char* StrBlk_Pt( char* s, int n )
{
    if (strlen( s ) > (size_t)n) { s[n-2]= NUL; strcat( s,".." ); }
    return s;
} /* StrBlk_Pt */

Boolean IsRead( ushort mode )
/* returns true, if <mode> has read bit set */
{   return (mode & poRead)!=0;
} /* IsRead */

Boolean IsWrite( ushort mode )
/* returns true, if <mode> has write bit set */
{   return (mode & poWrite)!=0;
} /* IsWrite */

Boolean IsRW( ushort mode )
/* returns true, if <mode> has read or write bit set */
{   return IsRead(mode) && IsWrite(mode);
} /* IsRW */

Boolean IsExec( ushort mode )
/* returns true, if <mode> has execution bit set */
{   return (mode & poExec)!=0;
} /* IsExec */

Boolean IsDir( ushort mode )
/* returns true, if <mode> has directory bit set */
{   return (mode & poDir)!=0;
} /* IsDir */

Boolean IsCrea( ushort mode )
/* returns true, if <mode> has creation bit set */
{   return (mode & poCreateMask)!=0;
} /* IsCrea */

Boolean IsTrDir( ushort umode )
/* returns true, if <mode> has dir bit set */
{
    #ifdef windows32
      return umode==0x4E00;
    #elif defined MINGW
      /* S_ISDIR() is shadowed on MINGW the same way S_IRUSR/S_IWUSR are
       * (see the matching fix in fileaccess.c's getFD()): sysdeps.h's
       * _WIN32-only block redefines it to `(a & FILEFLAG_DIR)` — the UAE
       * core's own Amiga-disk-image attribute check, not a real stat()
       * mode test. S_IFMT/S_IFDIR themselves are NOT shadowed (confirmed
       * via -dM preprocessor dump), so check against those directly. */
      return (umode & S_IFMT) == S_IFDIR;
    #else
      return S_ISDIR( umode ); /* it is a directory ? */
    #endif
} /* IsTrDir */

/* ------------------------------------------------------------------------ */
/* default options for Console/non-Console SCF I$GetStt */
static struct _sgs init_consoleopts = {
        0,      /* PD_DTP   file manager class code 0=SCF */
        0,      /* PD_UPC   0 = upper and lower cases, 1 = upper case only */
        1,      /* PD_BSO   0 = BSE, 1 = BSE-SP-BSE */
        0,      /* PD_DLO   delete sequence */
        1,      /* PD_EKO   0 = no echo */
        1,      /* PD_ALF   0 = no auto line feed */
        0,      /* PD_NUL   end of line null count */
        0,      /* PD_PAU   0 = no end of page pause */
        24,     /* PD_PAG   lines per page */
        0x08,   /* PD_BSP   backspace character */
        0x18,   /* PD_DEL   delete line character */
        CR,     /* PD_EOR   end of record character */
        0x1B,   /* PD_EOF   end of file character */
        0x04,   /* PD_RPR   reprint line character */
        CtrlA,  /* PD_DUP   duplicate last line character */
        0x17,   /* PD_PSC   pause character */
        CtrlC,  /* PD_INT   keyboard interrupt character */
        CtrlE,  /* PD_QUT   keyboard abort character */
        0x08,   /* PD_BSE   backspace echo character */
        0x07,   /* PD_OVF   line overflow character (bell) */
        0,      /* PD_PAR   device initialization (parity) */
        0x0F,   /* PD_BAU   baud rate */
        0x0000, /* PD_D2P   offset to output device name string */
        XOn,    /* PD_XON   x-on char */
        XOff,   /* PD_XOFF  x-off char */
        0x09,   /* PD_TAB   tab character */
        4,      /* PD_TABS  tab size */
    0x00000000, /* PD_TBL   Device table address (copy) */
        0x0001, /* PD_Col   Current column number */
        0,      /* PD_Err   most recent error status */
      { 0 }   /*          _sgs_spare[93]; spare bytes */
};

/* get options from SCF device */
os9err pSCFopt( _pid_, _spP_, byte* buffer )
{   memcpy( buffer, &init_consoleopts, OPTSECTSIZE ); return 0;
} /* pSCFopt */

/* ------------------------------------------------------------------------ */
/* constant option sections for RBF I$GetStt */
               /* dev dsc / path */
const byte rbfstdopts[OPTSECTSIZE]= 
              { 1, /* $48   $80  PD_DTP: 1=RBF */
                0, /* $49   $81  PD_DRV: drive number */
                0, /* $4A   $82  PD_STP */
                0, /* $4B   $83  PD_TYP: floppy type */
                2, /* $4C   $84  PD_DNS */
                0, /* $4D   $85  reserved */
            0,128, /* $4E   $86  PD_CYL */
                1, /* $50   $88  PD_SID */
                0, /* $51   $89  PD_VFY */
   0, DEFAULT_SCT, /* $52   $8A  PD_SCT: ignored, because PD_Cntl is used */
   0, DEFAULT_SCT, /* $54   $8C  PD_T0S: ignored, because PD_Cntl is used */
   0,RBF_MINALLOC, /* $56   $8E  PD_SAS: segment allocaton size */
                0, /* $58   $90  PD_ILV */
                0, /* $59   $91  PD_TFM */
                0, /* $5A   $92  PD_TOffs */
                0, /* $5B   $93  PD_SOffs */
              1,0, /* $5C   $94  PD_SSize default: set to 256 */
              0,9, /* $5E   $96  PD_Cntl: automatically */
                              
              0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
              0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
              0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0 };
              
os9err pRBFopt( _pid_, _spP_, byte* buffer )
/* get standard options for RBF file */
{   memcpy( buffer, rbfstdopts, OPTSECTSIZE ); return 0;
} /* pRBFopt */

/* ------------------------------------------------------------------------ */

/* fill <key> to buffer and make special key handling */
Boolean KeyToBuffer( ttydev_typ* mco, char key )
{
    char pd_int = mco->spP->opt[ PD_INT  ]; /* get special chars from opt sct */
    char pd_qut = mco->spP->opt[ PD_QUT  ];
    char pd_xon = mco->spP->opt[ PD_XON  ];
    char pd_xoff= mco->spP->opt[ PD_XOFF ];

    int lwp= mco->spP->lastwritten_pid;        /* where to send the signal ? */
    if     ( mco->inBufUsed >= INBUFSIZE-1 ) {
    /*  printf( "buffer is full %s\n", mco->spP->name ); */
        return false; /* buffer is full */
    }
    
    /* these characters will be eaten before they reach the input buffer */
    /* treatment for special chars */
    if     (key!=NUL) {
        if (key==pd_int)  { baud_flush_device( mco->spP->term_id ); mco->inBufUsed= 0; if (lwp) send_signal( lwp, S_Intrpt ); return 0; }
        if (key==pd_qut)  { baud_flush_device( mco->spP->term_id ); mco->inBufUsed= 0; if (lwp) send_signal( lwp, S_Abort  ); return 0; }
        /* XON/XOFF are consumed by the driver, never handed to SCF ("the driver
           consumes the PD_XON and PD_XOFF characters itself" -- Technical I/O
           Manual V2.4, PD_XOFF). console_hold_changed() tells the paced-output
           FIFO that this device left or rejoined the drain schedule; without it
           a held device's stale deadline turns the scheduler's idle nap into a
           spin, and a released one dumps its whole backlog in one unpaced burst. */
        if (key==pd_xon)  { mco->holdScreen= false;
                            console_hold_changed( mco->spP->term_id, false ); return 0; }
        if (key==pd_xoff) { mco->holdScreen=  true;
                            console_hold_changed( mco->spP->term_id,  true ); return 0; }
    }
            
    mco->inBuf[ mco->inBufUsed++ ]= key; /* update the buffer */

    /* Data is now ready on this terminal.  If a process armed "signal on data
       ready" (SS_SSig) on it, wake it now.  The arming path is that process's
       OWN open of the device -- a different syspath than the terminal's main
       one (mco->spP) -- so we can't just look at mco->spP; deliver to every
       syspath bound to this same terminal (matching term_id) that has a signal
       pending.  tsmon depends on this: it arms SS_SSig then F$Sleeps and never
       reads the device itself, so without delivering here the terminal is dead
       to keypresses.  (syspath_read delivers the same signal, but only when
       someone actually reads the path.) */
    { int k; ushort tid= mco->spP->term_id;
      for (k=0; k<MAXSYSPATHS; k++) { syspath_typ* s= &syspaths[k];
          if (s->signal_to_send && s->term_id==tid) {
              send_signal( s->signal_pid, s->signal_to_send );
                           s->signal_to_send= 0;
          }
      }
    }
    return true;
} /* KeyToBuffer */

void LastCh_Bit7( char* name, Boolean setIt )
/* adapt the dir entry's last char to normal/dir style depending on <setIt> */
{
    char*    c= name;
    if     (*c==NUL) return; /* do nothing for the empty string */
    while  (*c!=NUL) c++;    /* search for the last char in string */
    
                c--;
    if (setIt) *c= *c | 0x80;
    else       *c= *c & 0x7f;
} /* LastCh_Bit7 */

void Console_Name( int term_id, char* consname )
{
    char    *p;
    Boolean isTTY;
    
    switch(term_id) {         /* allowed terminal names */
        case    Main_ID: p="term"; break;
        case SerialA_ID: p="ts1";  break;
        case SerialB_ID: p="ts2";  break;
        case   VModBase: p="vmod"; break;

        default        :
              isTTY= (term_id>=TTY_Base);
          /* Built directly in consname and returned. The old code set
             p= consname here and then fell through to strcpy(consname,p) --
             a strcpy of a buffer onto ITSELF, which is undefined for
             overlapping ranges. Third instance of that class here, after
             GetEntry (42da1f7) and egetenv (66c75de).
             Bounded by OS9NAMELEN, the device-name contract every caller
             already satisfies -- the same bound VolInfo() above relies on. */
          if (isTTY) snprintf( consname,OS9NAMELEN, "%s%02d","tty",term_id ); /* tty     */
          else       snprintf( consname,OS9NAMELEN, "%s%d"  ,"t",  term_id ); /* console */
          return;
    }

    snprintf( consname,OS9NAMELEN, "%s", p );
} /* Console_Name */

char* OS9exec_Name( void )
{
    static char theName[24]; /* was [15] -- "OS9exec V" is already 9 chars */

    snprintf( theName,sizeof(theName), "OS9exec V%x.%02x", exec_version,exec_revision );
    return   theName;
} /* OS9exec_Name */

uint32_t Pipe_NReady( pipechan_typ* p )
{
  uint32_t n= (uint32_t)(p->pwp-p->prp);
  if        (p->pwp<p->prp) n+= p->size; /* wrapper */
  return n;
} /* Pipe_NReady */

/* --------------------------------------------------------------- */
Boolean AbsPath( const char* pathname )
/* returns true if <pathname> is an absolute path */
{   return *pathname==PSEP;
} /* AbsPath */

void GetOS9Dev( const char* pathname, char* cmp_entry )
/* gets the next subpath of <p> into <cmp_entry> */
{
    char*       c= cmp_entry;
    const char* p= pathname;
    
    if (AbsPath( p )) { /* only searching absolute paths */
        p++;
        while  (*p!=NUL && *p!=PSEP && *p!='@') {
            *c= *p; 
            c++; p++;
        } /* while */
    }
    
    *c= NUL;
} /* GetOS9Dev */

Boolean IsWhat( const char* pathname, Boolean isRoot )
/* Test if <pathname> is a root/raw path */
{   
    char* p= (char*)pathname;

    /* root/raw must be absolute path */
    if (!isRoot && strcmp( p,"@" )==0) return true; /* my own device raw */ 
    if (!AbsPath( p )) return false;
    
    p++;
    while  (*p!=NUL) { /* further slashes are not allowed */
        if (*p=='@') { p++; break; } /* will be decremented again */
        if (*p==PSEP) return false;
        p++;
    } /* while */

    p--;
    if (isRoot) return *p!='@';
    else        return *p=='@';
} /* IsWhat */

Boolean IsRoot( const char* pathname )
/* Test if <pathname> is a root path */
{   return IsWhat( pathname,true );
} /* IsRoot */

Boolean IsRaw( const char* pathname )
/* Test if <pathname> is a raw path */
{   return IsWhat( pathname,false );
} /* IsRaw */

/* --------------------------------------------------------------- */
Boolean VolInfo( const char* pathname, char* volname )
{
    Boolean ok= true;

    /* UNIX FIRST, before macintosh: every UNIX-ish host -- Linux, mingw, AND
     * modern macOS (which defines both `macintosh` and `UNIX`) -- has a single
     * "/" root and should report it uniformly. macOS used to fall into the
     * legacy `macintosh` branch below and return "" instead, a 1-platform-vs-2
     * divergence observable through a raw LSN0 read. Only genuine classic Mac OS
     * (MACOS9: `macintosh` defined, `UNIX` not) now reaches the empty-string
     * HFS-volume branch. */
    #ifdef UNIX
      (void)pathname;
      strcpy( volname,"/" );

    #elif defined macintosh
      #pragma unused(pathname)
      strcpy( volname,"" );

    #elif defined(windows32)
      char    sysname[OS9NAMELEN];
      DWORD   serno,maxComp,sysFlags;
      
      ok= GetVolumeInformation( pathname,  volname, OS9NAMELEN, 
               &serno,&maxComp,&sysFlags, &sysname, OS9NAMELEN );
      if (!ok ||  *volname==NUL)
          snprintf( volname,OS9NAMELEN, "%c:", toupper((unsigned char)pathname[0]) ); /* same bound GetVolumeInformation above was given */  
    #endif
    
    return ok;
} /* VolInfo */

Boolean OpenTDir( const char* pathname, DIR** d )
/* Open Directory with special treatment of empty root dir on windows */
{
    #ifdef MACOS9
      struct stat info;

      *d= NULL; /* not used */
      return stat_( pathname, &info )==0 && IsTrDir(info.st_mode);

    #else
      Boolean ok= false;
      
      #ifdef TFS_SUPPORT
        #ifdef windows32
          char volname[OS9NAMELEN];
        #endif
        
             *d= (DIR*)opendir( pathname ); /* try to open */
        ok= (*d!=NULL);

        #ifdef windows32
          /* special handling for empty root directory */
          if (!ok) ok= VolInfo( pathname, &volname );
        #endif
      #endif
      
      return ok;
    #endif
} /* OpenTDir */

dirent_typ* ReadTDir( DIR* d )
{
    #ifdef MACOS9
      #pragma unused(d)
      return NULL;
    
    #else
      if (d==NULL) return NULL;
      return readdir( d );
    #endif
} /* ReadTDir */

Boolean PathFound( const char* pathname )
/* Check if this entry is a directory */
{
    DIR* d;
    Boolean ok= OpenTDir( pathname, &d );
    
    #if defined win_unix && defined TFS_SUPPORT
      if (d!=NULL) closedir( d );
    #endif
    
    debugprintf( dbgFiles,dbgNorm,("# PathFound %s '%s'\n", 
                                      ok ?" (ok)":"(err)", pathname ));
    return ok;
} /* PathFound */

Boolean FileFound( const char* pathname )
/* Check if this entry is a file */
{
    #ifdef windows32
    DWORD winerr;
    #endif
    FILE*        f= fopen( pathname,"rb" ); /* try to open for read */
    Boolean ok= (f!=NULL);
    if     (ok) fclose( f ); /* don't use it any more */

    #ifdef windows32
      if (!ok) {
        winerr=GetLastError();
        if (winerr==ERROR_SHARING_VIOLATION) ok= true;
      }
    #endif
    
    debugprintf( dbgFiles,dbgNorm,("# FileFound %s '%s'\n", 
                                      ok ?" (ok)":"(err)", pathname ));
    return ok;
} /* FileFound */

#ifdef linux    
  void include_2e( char* filename, char* pos )
  {
      char  tmp[OS9PATHLEN];
    
      *(pos)= NUL;
      strcpy( tmp,&pos[1] );
      strcat( filename,L_P );
      strcat( filename,tmp );
  } /* include_2e */
#endif

void CutUp( char* pathname, const char* prev )
/* cut out /xxxx/../ sequences */
{
    char *v, *q, *qs;
    #ifdef linux
    Boolean inc;
    #endif
//  int i;  /* dot-counter — only used in commented-out //if (i>0) q++; */

    v= pathname;
    while (true) {
        q =   strstr( v,prev ); if (q==NULL) break; /* search the string */
        qs= q+strlen(   prev );

        #ifdef linux
        inc= false;
        #endif
        switch (*qs) {
            case NUL      : *q= NUL; break; /* cut "/."  at the end */
            
            /* avoid duplicate definition */
            #ifndef UNIX
            case PATHDELIM:
            #endif
            case PSEP     : memmove( q, qs, strlen(qs)+1 ); break; /* cut "/./" anywhere */

            case '.'      : v= qs;
                         // i= 0;
                            while  (*(++v)=='.') /* i++ */;
                            switch (*v) {
                                case NUL:

                                /* avoid duplicate definition */
                                #ifndef UNIX
                                case PATHDELIM:
                                #endif
                                case PSEP     : while  (q>pathname) {
                                                        q--;
                                                  if  (*q==PATHDELIM ||
                                                       *q==PSEP) {
                                                    if (q==pathname) { q= qs-1; break; }
                                                        q++;
                                                  //if (i>0) q++;
                                                    break;
                                                  } // if
                                                } // while

                                                /* A Windows drive-letter host path ("C:/...") has no
                                                 * delimiter AT position 0 -- unlike "/..." on Unix, where
                                                 * pathname[0] IS the root delimiter and the q==pathname
                                                 * check just above always catches "collapsed all the way
                                                 * back to root". Here the backward walk instead exhausts
                                                 * via the outer q>pathname condition, leaving q on the
                                                 * drive letter itself. Give it the identical "nothing
                                                 * left to collapse into, we're at the root" treatment, or
                                                 * the memmove below overwrites the drive letter with the
                                                 * remaining path and silently drops it -- confirmed live:
                                                 * "C:/../../../USR/..." collapsed to "./../../USR/...",
                                                 * losing "C:" entirely and corrupting every subsequent
                                                 * AdjustPath step for a path with more ".."-equivalents
                                                 * than there are real directory levels. */
                                                if (q==pathname && *q!=PATHDELIM && *q!=PSEP) q= qs-1;

                                                memmove( q, qs, strlen(qs)+1 ); /* concatenate at the new position */
                                                q--;
                                                break;

                                default:        q++;
                                                #ifdef linux
                                                inc= true;
                                                #endif
                            } /* switch */
                            break;

            default:        q++;
                            #ifdef linux
                            inc= true;
                            #endif
        } /* switch */
      
        #ifdef linux
          if (inc) include_2e( pathname, q );
        #endif
        
        debugprintf( dbgFiles,dbgNorm,("# AdjustPath REDU '%s'\n", pathname ));
        v= q;
    } /* while */
} /* CutUp */

void EatBack( char* pathname )
{
    #define Prev  "/."
    #define PrevW "\\." /* Windows32 version */
    char*   p;
    
    int     eat = 0;
    int     eat0= 0;
    Boolean searchP= true;
    
    p= pathname+strlen(pathname)-1; /* start at end of string */

    while (p>pathname) {
        switch (*p) {
            case '.'      : if (searchP)    eat++;                      break;
            
            /* avoid duplicate definition */
            #ifndef UNIX
            case PATHDELIM:
            #endif
            case PSEP     : *p= NUL; eat--; eat0= eat;  searchP=  true; break;
            
            #ifdef windows32
              case ':'    : p++;     eat=      0;                       break;
            #endif
            
            default       :          eat=   eat0;       searchP= false; break;
        } /* switch */

        p--;
        if (eat<=0) break;
    }
    
    #ifdef windows32
      if (*p==':') { *++p= PATHDELIM; *++p= NUL; }
    #endif
          
    CutUp( pathname, Prev ); /* support also for RBF OS-9 paths */
    
    #ifdef windows32
      CutUp( pathname, PrevW );
    #endif
} /* EatBack */

// Take the CRC algorithm as hash function
//static int HashF( char* name, char* fName )
static int HashF( char* name )
{
  const ulong AccStart= 0xffffffff;
  ulong rslt;
  int  i, len;  
  char ups[OS9_MAXPATH]; // let the original be case sensitive for Linux
  
               len= strlen( name );  
  for (i= 0; i<len+1; i++) {
    ups[ i ]= toupper((unsigned char) name[ i ] ); // make comparisons more "the same"
  } // for
  
  
  rslt = AccStart;
  rslt = calc_crc( (byte*)&ups, len,  rslt );
  rslt^= AccStart;
  rslt = rslt % MAXDIRS;
  
  if    (rslt==0) rslt++; // do not allow 0
  return rslt;
} // HashF

os9err FD_ID( const char* pathname, dirent_typ* dEnt,
              uint32_t   *fdID,     dirtable_entry** mH )
            //ulong *id, long dirid,  char* fName )
{
  #ifdef MACOS9
    #pragma unused(dEnt)
  #endif

  #define ATTR_DIR 0x0010
  char      tmp[OS9_MAXPATH];
  int       ii, hh, n;
//direntry* m;
  Boolean   doit;
  ulong     liCnt= 0;
  uint32_t  dirid;
  
  #define MAXLICNT ( 0x00800000 / MAXDIRS )

  #ifdef MACOS9
  char*    fName= "";
  if (*mH) fName= (*mH)->fName;
  #endif
           dirid= 0;
  if (*mH) dirid= (*mH)->dirid;

  /* 
//upe_printf( "REIN name='%s'\n", pathname );
  #if defined UNIX
    // .. when using Inodes
    if (useInodes) {
    //if (spP!=NULL) {
    //  while (dEnt!=NULL && isFirst && strcmp( dEnt->d_name,"." )!=0) {
    //         dEnt= ReadTDir( spP->dDsc ); 
    //  } // while
    //} // if
      
      if (dEnt==NULL) *id= 0;
      else            *id= dEnt->d_ino;
    
      #if defined __MACH__
        if (*id==1) *id= 2; // adapt for top dir searching
      #endif
          
      return 0;
    }
  #endif
  */
  
  strcpy( tmp, pathname );
      
  #if defined win_unix
    if (dEnt!=NULL) {
      if     ( tmp[ strlen( tmp )-1 ]!=PATHDELIM ) strcat( tmp,PATHDELIM_STR );
      strcat ( tmp, dEnt->d_name );
    } // if
  #endif
  
  EatBack( tmp );
//upe_printf( "REIN name='%s'\n", tmp );
                
//if (dEnt->data.dwFileAttributes!=ATTR_DIR) return id;
      
  *fdID= 0; /* undefined */
    //hh= HashF( tmp, fName );
      hh= HashF( tmp );
  ii= hh;
//upe_printf( "id=%08X <= name='%s' START\n", ii, tmp );
      
  n= 1; *mH= &dirtable[ ii ];
  while (true) {
  //upe_printf( "m->ident=%08X\n", m->ident );
    if ((*mH)->ident==NULL) {
                (*mH)->ident= malloc( strlen( tmp )+1 );
      if       ((*mH)->ident==NULL) return os9error(E_NORAM); /* strcpy would deref it */
      strcpy  ( (*mH)->ident,                 tmp );

      #ifdef MACOS9
                (*mH)->fName= malloc( strlen( fName )+1 );
        if     ((*mH)->fName==NULL) return os9error(E_NORAM);
        strcpy( (*mH)->fName,                 fName );
      #endif
          
      if (liCnt==0) { hittable[ 0 ]--; } // adapt statistics
                      hittable[ n ]++;
    } /* if */
          
    if (dirid!=0) (*mH)->dirid= dirid;
    if (ustrcmp(  (*mH)->ident, tmp )==0) {
      *fdID= (ulong)ii;
      break;
    } /* if */

    #ifdef LINKED_HASH
                liCnt++;
          doit= liCnt>=MAXLICNT;
      if (doit) liCnt= 0;
    #else
      doit= true;
    #endif
  //upe_printf( "liCnt=%d\n", liCnt );
    
    if (doit) {    
          ii++;
      if (ii==MAXDIRS) ii= 1;
      if (ii==hh) break;
      *mH= &dirtable[ ii ];
    } 
    else {
      #ifdef LINKED_HASH
            doit= (*mH)->next==NULL;
        if (doit) {
                  (*mH)->next= malloc( sizeof( dirtable_entry ) );
          if     ((*mH)->next==NULL) return os9error(E_NORAM); /* the fields below deref it */
        } // if
        *mH=      (*mH)->next;
        
        if (doit) {
          (*mH)->ident= NULL;

          #ifdef MACOS9
          (*mH)->fName= NULL;
          #endif

          (*mH)->next = NULL;
        } // if
      #endif
    } // if
    
    if (n<MAXDIRHIT-1) n++; 
  } /* loop */
  if (*fdID==0) return E_NORAM;
  
  *fdID+= liCnt*MAXDIRS;
    
//upe_printf( "id=%08X <= name='%s'\n", *id, tmp );
  return 0;
} /* FD_ID */

os9err FD_Name( uint32_t fdID, char* *pathnameP )
// get back the real <volID> and <objID> for Mac file system
{
  os9err          err= 0;
  dirtable_entry* m;
//ulong           id= fdID;  /* referenced only in commented-out debug prints */
  
  #ifdef LINKED_HASH
    ulong i;
    ulong liCnt= fdID / MAXDIRS;
          fdID = fdID % MAXDIRS;
  #endif

//upe_printf( "REIN id=%08X, liCnt=%d fdID=%08X\n", id, liCnt, fdID );
   
  *pathnameP= NULL;
  if   (fdID>0 && fdID<MAXDIRS) {       // if the range is correct
    m= &dirtable[ fdID ]; // get the entry directly
  
    #ifdef LINKED_HASH
      for (i= 0; i<liCnt; i++) {
        if (m) m= m->next;
      } // for
    #endif
    
    if      (m && m->ident!=NULL) {
      *pathnameP= m->ident;
    } // if
  } // if
  
  if (*pathnameP==NULL) err= E_PNNF;  
//upe_printf( "id=%08X => name='%s' err=%d\n", fdID, *pathnameP ? *pathnameP : "", err );
  return err;
} // FD_Name

os9err Flush_Dir( ushort cpid, ushort* pathP, const char* nmS )
{
  os9err          err;
  os9direntry_typ d;
  uint32_t        dir_size;
  syspath_typ*    spP;
  char            fullName[OS9PATHLEN];
  int             oLen;
  dirtable_entry* mP= NULL;
  uint32_t        fd_hash;

  err=   usrpath_open( cpid,  pathP, fRBF, nmS, 0x81 ); if (err) return err;
  spP=   get_syspath ( cpid, procs[ cpid ].usrpaths[ *pathP ] ); // get spP for fd sects

  do {                                dir_size= sizeof( d );
    err= usrpath_read( cpid, *pathP, &dir_size, &d, false ); if (err) break; // ".."
                                      dir_size= sizeof( d );
    err= usrpath_read( cpid, *pathP, &dir_size, &d, false ); if (err) break; // "."

    strcpy      ( fullName, spP->fullName );
    strcat      ( fullName, PSEP_STR      );
    oLen= strlen( fullName );
  
    while (true) {                      dir_size= sizeof( d );
      err= usrpath_read( cpid, *pathP, &dir_size, &d, false ); if (err) break;
      
      if           ( d.name[ 0 ]!=NUL ) {
        LastCh_Bit7( d.name, false );
        
                    fullName[ oLen ]= NUL; // restore
        strcat    ( fullName, d.name );        mP= NULL;
        err= FD_ID( fullName, NULL, &fd_hash, &mP ); if (err) break;
      
      //if (mP->dirid!=0 &&
      //    mP->dirid!=d.fdsect)
      //  main_printf( "name='%s' fdsect=%06X %06X\n", fullName, os9_long( d.fdsect ), mP->dirid );

        mP->dirid= 0;
      } // if
    } // while
    
    if (err==E_EOF) err= 0;
  } while (false);

  return err;
} // Flush_Dir

// cache flush of this file or directory
os9err Flush_Entry( ushort cpid, const char* name )
{
  os9err          err, cer;
  ushort          path;
  syspath_typ*    spP;
  char            tmp[ OS9PATHLEN ];
  uint32_t        fd_hash;
  dirtable_entry* mP= NULL;
  
  if (name[ 0 ]=='/'  || 
      strcmp( procs[ cpid ].d.path,"" )==0) {
    strcpy( tmp, name );
  }
  else {
    strcpy( tmp, procs[ cpid ].d.path );
    strcat( tmp, PSEP_STR );
    strcat( tmp, name );
  } // if
  
       err= FD_ID( tmp, NULL, &fd_hash, &mP ); 
  if (!err) mP->dirid= 0;
  
       err= Flush_Dir( cpid, &path, name );
  if (!err) {   
    spP= get_syspath( cpid, procs[ cpid ].usrpaths[ path ] ); // get spP for fd sects

         err= FD_ID( spP->fullName, NULL, &fd_hash, &mP ); 
    if (!err) mP->dirid= 0;
  
    cer= usrpath_close( cpid, path ); if (!err) err= cer;
  } // if
  
  return 0;
  
  
  /*
           err= usrpath_open( cpid, &path, fRBF, name, 0x01 );
  if (err) err= Flush_Dir   ( cpid, &path,       name       );
        
  spP= get_syspath( cpid, procs[ cpid ].usrpaths[ path ] ); // get spP for fd sects

       err= FD_ID( spP->fullName, NULL, &fd_hash, &mP ); 
  if (!err) mP->dirid= 0;
  
  cer= usrpath_close( cpid, path ); if (!err) err= cer;
  return err;
  */
} // Flush_Entry

void Flush_FDCache( const char* pathname )
{
//const char* p= pathname;
//main_printf( "flush for '%s'\n", pathname );
} // Flush_FD_Cache

  
  
#ifdef win_unix
os9err DirNthEntry( syspath_typ* spP, int n, dirent_typ** dEnt )
/* prepare directory to read the <n>th entry */
{   
//int m= n;
  int i= 0;
  
  /*  
  debugprintf(dbgFiles,dbgDetail,("# DirEntry: ---\n" )); 
  seekD0( spP );
  while (m>0) {
        *dEnt= ReadTDir( spP->dDsc ); 
    if (*dEnt==NULL) break;
    debugprintf(dbgFiles,dbgDetail,("# DirEntry: '%s'\n", (*dEnt)->d_name )); 
    m--;
  } // while
  debugprintf(dbgFiles,dbgDetail,("# DirEntry: ---\n" )); 
  */
  
  do {
    if (n>2) {
             i=            spP->svD_n;
      if (n==i  ) { *dEnt= spP->svD_dEnt; break; } // still the same
      if (n!=i+1) i= 0;                            // not the next one
    } // if
  
  //if (i==0) { seekD0( spP ); i= 1; }  // start at the beginning

    if (n<=2 || i==0) { 
      seekD0( spP );
      i= 1; // ignore ".." and "." entries for n>=2
    } // if
    
    if (n==0) {     // search for ".."
      do     *dEnt= ReadTDir( spP->dDsc ); 
      while (*dEnt!=NULL && strcmp( (*dEnt)->d_name,".." )!=0);

      break;
    } // if

    if (n==1) {     // search for "."
      do     *dEnt= ReadTDir( spP->dDsc ); 
      while (*dEnt!=NULL && strcmp( (*dEnt)->d_name,"."  )!=0);

      break;
    } // if
    
  //#ifdef windows32 // missing top dir entry
    /*
    if (n==2 || i==0) { 
      seekD0( spP );
      i= 1; // ignore ".." and "." entries
    } // if
    */
  //#endif
    
    // starting after ".." and "."
    do {  *dEnt= ReadTDir( spP->dDsc );
      if (*dEnt==NULL) break;
      
      if (strcmp( (*dEnt)->d_name,".."  )!=0 &&
          strcmp( (*dEnt)->d_name,"."   )!=0 &&
         ustrcmp( (*dEnt)->d_name,AppDo )!=0 &&
         ustrcmp( (*dEnt)->d_name,DsSto )!=0) i++;
    } while ( i<n );
  } while (false);
  
  spP->svD_n   =     n;
  spP->svD_dEnt= *dEnt;
  
  if (*dEnt==NULL) return E_EOF;
  else             return 0;
} /* DirNthEntry */

os9err RemoveAppledouble( syspath_typ* spP )
{
    os9err      err;
    DIR*        app_d;
    dirent_typ* dEnt;
    char        app [OS9PATHLEN];
    char        fnam[OS9PATHLEN];
    
    err= DirNthEntry( spP,2, &dEnt ); if (err) return err; // start after "." and ".."

    while (dEnt!=NULL) {
      if (ustrcmp( dEnt->d_name,AppDo )!=0 ) return 0; /* there are still more entries */
      dEnt= ReadTDir( spP->dDsc ); 
    } /* loop */

    strcpy( app,spP->fullName );
    strcat( app,PATHDELIM_STR );
    strcat( app,AppDo ); /* full .AppleDouble path */
    
    app_d= opendir( app ); if (app_d==NULL) return 0; /* already removed */

    while (!err) { /* delete all entries of the .AppleDouble directory */
            dEnt= ReadTDir( app_d );
        if (dEnt==NULL) break;
        
        if (strcmp( dEnt->d_name,"."  )!=0 && /* ignore "." and ".." */
            strcmp( dEnt->d_name,".." )!=0) {
        	strcpy( fnam,app );
        	strcat( fnam,PATHDELIM_STR );
        	strcat( fnam,dEnt->d_name );
        	err= host2os9err( remove(fnam),E_PNNF ); if (err) break;
        }
    } /* while */

    closedir( app_d );
    if (err) return err; /* from previous error */
    
    /* and remove .AppleDouble itself */
    err= host2os9err( remove(app),E_PNNF ); if (err) return err;
    return 0;
} /* RemoveAppledouble */
  
  
  
void seekD0( syspath_typ* spP )
{   if (spP->dDsc!=NULL) rewinddir( spP->dDsc ); /* start at the beginning */
} /* seekD0 */

uint32_t DirSize( syspath_typ* spP )
/* get the virtual OS-9 dir size in bytes */
{
    int         cnt= 0;
    dirent_typ* dEnt;

    seekD0( spP );       /* start at the beginning */
    while (true) {       /* search for the nth entry */
            dEnt= ReadTDir( spP->dDsc );
        if (dEnt==NULL) break;
        if (ustrcmp( dEnt->d_name,AppDo )!=0) cnt++; /* ignore ".AppleDouble" */
    } /* loop */

    /* This walk left the host stream at EOF on every platform (rewinddir()+
     * readdir()-to-NULL, no telldir()/seekdir() restore -- mingw's dirent
     * doesn't support those reliably anyway). DirNthEntry's sequential-read
     * cache (svD_n/svD_dEnt) doesn't know that, so invalidate it the same
     * way pDopen/pDseek already do: the next DirNthEntry call sees i==0 and
     * reseeks from scratch instead of trusting a stale "just keep reading"
     * position. */
    spP->svD_n= 0;

                                      /* avoid also 1 entry !!! */
    if    (cnt<2) cnt= 2; /* at least two entries are there !!! */
    return cnt*DIRENTRYSZ;
} /* DirSize */
#endif

int stat_( const char* pathname, struct stat *buf )
/* slightly adapted version for Windows */
{
    int err= -1 /* default */;
    
    #if defined macintosh || defined UNIX
      /* do it the "normal" way */
      err= stat( pathname, buf );

    #elif defined windows32
      /* 'stat' at windows has a bug, it does not close correctly */
      FILE* stream;
      int   fd, cer;
      
      /* is it a file ?? */
          stream= fopen( pathname,"rb" );
      if (stream==NULL) {
        if (GetLastError()==ERROR_SHARING_VIOLATION) 
            err= stat( pathname, buf ); /* if error, try normal way */
      }
      else {
          /* stat seems not to close correctly under Windows */
          /* use fstat instead of stat */
          fd = fileno( stream );
          err= fstat ( fd,buf ); 
          cer= fclose( stream ); if (err==0) err= cer; /* close always */
      
          /* is it a dir ? then try it the "normal" way */
          if (err && PathFound( pathname ))
              err=        stat( pathname, buf );
      }
    #endif 
    
    return err;
} /* stat_ */

Boolean DirName( const char* pathname, uint32_t fdsect, char* result, Boolean useInodes )
// Only Linux StartDir uses useInodes = true
{
  Boolean ok= false;

  #ifdef MACOS9
    #pragma unused(pathname,fdsect,result,useInodes)
  #endif
    
  #ifdef win_unix
    DIR*            d;
    dirent_typ*     dEnt;
    dirtable_entry* mP= NULL;
    uint32_t        fd;
    Boolean         okINO;

    #ifdef __MACH__
      char   p [OS9PATHLEN];
      char   q [OS9PATHLEN];
      char   sv[OS9PATHLEN];
      struct stat qInfo;
      struct stat pInfo;
      int    len, err;
      
      strcpy( sv, result ); 
    #endif
     
        d= (DIR*)opendir( pathname ); /* search for the current inode */
    if (d!=NULL) {
      while (true) {
            dEnt= ReadTDir( d );
        if (dEnt==NULL) break;
      
        okINO= false;
        #if defined UNIX
          // .. when using Inodes
          if (useInodes) {
            okINO= true;
            
          //if (dEnt==NULL) fd= 0;
          //else            
            fd= dEnt->d_ino;
    
            #if defined __MACH__
              if (fd==1) fd= 2; // adapt for top dir searching
            #endif
          } // if
        #endif

        if (!okINO) FD_ID( pathname, dEnt, &fd, &mP );
              
      //upo_printf( "fd/fdsect=%d/%d '%s'\n", fd, fdsect, dEnt->d_name );
        if (fd==fdsect && strcmp( dEnt->d_name,".." )!=0) { /* this is it */
          strcpy( result,         dEnt->d_name );
          ok= true; break;
        }   
      } // loop
          
      closedir( d );
      
      
      if (!ok) {
        strcpy( result, "?" ); // in case of not found
      
        #ifdef __MACH__
          do {
            // prepare comparison of one higher inode
            strcpy( q,  pathname );
            len= strlen(pathname); if (len<=6) break;
            q[ len-6 ]= 0;
          
          //upo_printf( "q='%s' sv='%s'\n", q, sv );
		    err= stat_ ( q, &qInfo ); if (err) break;
          //upo_printf( "q='%s' ino=%d err=%d\n", q, qInfo.st_ino, err );
              
                d= (DIR*)opendir( pathname ); /* search for the current inode */
            if (d!=NULL) {
              while (true) {
                    dEnt= ReadTDir( d );
                if (dEnt==NULL) break;
              //upo_printf( "name='%s'\n", dEnt->d_name );
		                        
                strcpy( p, pathname );
                strcat( p, PATHDELIM_STR );
                strcat( p, dEnt->d_name );
                strcat( p, PATHDELIM_STR );
                strcat( p, sv );
		          
                // get inode and compare it
                     err= stat_( p, &pInfo );
                if (!err) {
               // upo_printf( "name='%s' ino=%d\n", p, pInfo.st_ino );
                  if (pInfo.st_ino==qInfo.st_ino) { 
                    strcpy( result, dEnt->d_name );
                    ok= true;
                    break;
                  } // if
                } // if
              } // loop
        
              closedir( d );
            } // if
          } while (false);
        #endif
      } // if
      
    //upo_printf( "'%s' %s => '%s'\n\n", pathname, ok ? "OK":"NOT OK", result ); 
    } /* if */
  #endif
  
  return ok;
} /* DirName */

uint32_t My_FD( const char* pathname )
{
  uint32_t fd= 0;

  #ifdef MACOS9
    #pragma unused(pathname)
  #endif
    
  #ifdef win_linux
    char            p[OS9PATHLEN];
    char*           q;
    DIR*            d;
    dirent_typ*     dEnt;
    dirtable_entry* mP= NULL;

    strcpy( p,pathname );
    q= p+strlen(p)-1;
    while (q>p && *q!=PATHDELIM) q--;
    *q++= NUL;

        d= opendir( p ); /* search for the current inode */
    if (d!=NULL) {
      while (true) {
            dEnt= ReadTDir( d );
        if (dEnt==NULL) break;
        if (ustrcmp( dEnt->d_name, q )==0) {
           FD_ID( p, dEnt, &fd, &mP );
           break;
        } // if
      } // while
          
      closedir( d );
    } // if
  #endif
  
  return fd;
} /* My_FD */

void MakeOS9Path( char* pathname )
{
  char  tmp[OS9PATHLEN];
  char* q= pathname;
  int   i= 0;
    
  if (*q!=PSEP) {
    if (q[ 1 ]==':' &&
       (q[ 2 ]==PATHDELIM ||
        q[ 2 ]=='\0')) {   /* windows */
        q[ 1 ]= q[ 0 ];    /* adapt it OS-9-like, same length */
        q[ 0 ]= PSEP;
    }
    else {
      tmp[ 0 ]= PSEP; /* start with a slash */
      i= 1;
    } // if
  } // if

  strcpy( &tmp[ i ],q ); q= tmp;
            
  while  (*q!=NUL) { /* replace slashes */
    if (*q==PATHDELIM) *q= PSEP;
    q++;
  } /* while */
    
  q--; if (*q==PSEP) *q= NUL;
  strcpy( pathname,tmp );
} /* MakeOS9Path */

static void CutRaw( char** pP )
/* expect OS-9 notation */
{
    char*    p= *pP;
    while  (*p!=NUL) { /* cut raw device name or sub paths */
        if (*p=='@') { *p= NUL; break; }
        p++;
    }
} /* CutRaw */

Boolean SamePathBegin( const char* pathname, const char* cmp )
{
    int    len= strlen(cmp);
    return len>0 
       &&  ustrncmp( pathname,cmp, len )==0
       && (pathname[len]==NUL || 
           pathname[len]=='@' || 
           pathname[len]==PSEP);
} /* SamePathBegin */

Boolean IsDesc( const char* dvn, mod_dev** mod, char** p )
{
    int      mid= find_mod_id( dvn );
    ushort   mty;
    uint32_t modSize, fmgrOff, pdevOff;

    if (mid==MAXMODULES) return false; /* no such module found */

       *mod= (mod_dev*)os9mod( mid );
        mty= os9_word( (*mod)->_mh._mtylan )>>BpB;
    if (mty!=MT_DEVDESC) return false; /* not the right type */

    /* _mfmgr/_mpdev are module-chosen 16-bit offsets that this and the callers
       (RAM_Device, SCSI_Device, file_rbf.c) turn into raw HOST pointers and
       ustrcmp -- the same hazard as _mname/_midata bounded in modstuff.c. A
       descriptor whose file-manager or driver name offset falls outside the
       module is malformed: refuse it here (the single gate for _mfmgr, and every
       reader of _mpdev only reads it once IsDesc has returned true) rather than
       read host memory past the module. */
    modSize= os9_long( (*mod)->_mh._msize );
    fmgrOff= os9_word( (*mod)->_mfmgr );
    pdevOff= os9_word( (*mod)->_mpdev );
    if (fmgrOff>=modSize || pdevOff>=modSize) return false; /* bad descriptor */

    *p= (char*)*mod + fmgrOff;
    return true;
} /* IsDesc */

Boolean SCSI_Device( const char* os9path,
                     short *scsiAdapt, short *scsiBus, int *scsiID, short *scsiLUN,
                     ushort *scsiSsize, ushort *scsiSas, byte *scsiPDTyp,
                     ptype_typ *typeP )
/* expect OS-9 notation */
{
    char      tmp[OS9PATHLEN];
    char      *p, *dvn;
    int       ii;
    mod_dev*  mod;
    byte      lun;
    byte      id;
    ushort    ssize, sas;
    byte      pdtyp;
    
    *typeP= fNone;         /* the default value */
    strcpy( tmp,os9path ); /* make a local copy */
         p= tmp;
    if (*p==PSEP) p++;
    dvn= p;             /* this is what we are looking for */
    while  (*p!=NUL) { /* cut raw device name or sub paths */
        if (*p=='@' || *p==PSEP) { *p= NUL; break; }
        p++;
    }

    if (*dvn==NUL) return false; /* no device */

    /* this is the official table of the SCSI entries */
    for (ii=0; ii<MAXSCSI; ii++) { /* returns true, if SCSI device */
        if (ustrcmp(dvn,scsi[ ii ].name)==0) {
            // found, return SCSI address 
            *scsiAdapt= scsi[ ii ].adapt;
            *scsiBus  = scsi[ ii ].bus;
            *scsiID   = scsi[ ii ].id;
            *scsiLUN  = scsi[ ii ].lun;
            *scsiSsize= scsi[ ii ].ssize;
            *scsiSas  = scsi[ ii ].sas;
            *scsiPDTyp= scsi[ ii ].pdtyp;
            *typeP    = fRBF;
            return true;
        }
    } /* for */

    if (!IsDesc( dvn, &mod, &p )) return false;
    
//        mid= find_mod_id( dvn );           /* no such module found */
//    if (mid==MAXMODULES) return false;
//    
//        mod= (mod_dev*)os9mod( mid );      /* not the right type */
//        mty= os9_word( mod->_mh._mtylan )>>BpB;
//    if (mty!=MT_DEVDESC) return false;
//
//    p= (char *)mod + os9_word(mod->_mfmgr);         
    if (ustrcmp( p,"RBF" )==0) {
        /* &mod->_mdtype is a pointer TO THE 128-byte ARRAY, so "+ PD_xxx"
         * was advancing by PD_xxx*128 bytes, not PD_xxx -- mod->_mdtype
         * decays to a plain byte pointer and gives the intended byte
         * offset. ssize (PD_SSize, book-tagged "w") needs the
         * big-endian-on-disk -> host swap (GET_OS9W, matching this
         * file's own idiom); sas (PD_SAS, book-tagged "b") does NOT --
         * it's a single byte, and PD_SAS+1 is PD_ILV (sector interleave
         * factor), a real unrelated field, not padding, so a 2-byte
         * read there was splicing PD_ILV's byte into the SAS value
         * instead of adding harmless zero bits. Done once here so the
         * immediate output params and the scsi[] cache below
         * (previously inconsistent: raw here, os9_word()'d there) agree
         * on the same host-order value. */
        id   = mod->_mdtype[PD_CtrlID];
        lun  = mod->_mdtype[PD_LUN];

        // full SCSI address
        *scsiID   = id;
        *scsiLUN  = lun;
        *scsiAdapt= defSCSIAdaptNo; // bus and adaptor come from defaults
        *scsiBus  = defSCSIBusNo;
        ssize     = GET_OS9W( mod->_mdtype, PD_SSize ); *scsiSsize= ssize;
        sas       = mod->_mdtype[PD_SAS];                *scsiSas  = sas;
        pdtyp     = mod->_mdtype[PD_TYP];                *scsiPDTyp= pdtyp;
        
        // find empty scsi entry
        for (ii=0; ii<MAXSCSI; ii++) {
            if (scsi[ii].name[0]==NUL) {
                // empty entry, add stuff here
                // - name
                strcpy( scsi[ii].name, dvn );
                // - full SCSI address
                scsi[ ii ].id    = id;
                scsi[ ii ].lun   = lun;
                scsi[ ii ].adapt = defSCSIAdaptNo; // bus and adaptor come from defaults
                scsi[ ii ].bus   = defSCSIBusNo;
                // - params (ssize is already host-order -- swapped once
                // above via GET_OS9W; os9_word() here would swap it a
                // second time. sas is a single byte, no swap applies.)
                scsi[ ii ].ssize= ssize;
                scsi[ ii ].sas  = sas;
                scsi[ ii ].pdtyp= pdtyp;
                *typeP= fRBF;
                // done!
                return true;
            }
        }
        // no more room in SCSI table
        return false; 
    } // if
    
    // is false, but useful anyway
    if (ustrcmp( p,"SCF" )==0) {
        *typeP= fSCF;
        if (ustrcmp( dvn,"vmod" )==0) *typeP= fVMod;
        return false; /* unconditional -- it just didn't look it */
    }
    if     (ustrcmp( p,  "SOCKMAN" )==0 ||
            ustrcmp( p,  "SFM"     )==0) { *typeP= fNET;  return false; }
    if     (ustrcmp( p,  "PKMAN"   )==0) { *typeP= fPTY;  return false; }
    
    return false;
} /* SCSI_Device */

#ifdef windows32
  static void StrReplace( char* dst, const char* src, const char* search, 
                                                      const char* replace )
  {
    int  i= 0;
    int  j= 0;
    int  k;
    char ch, v, w;
    Boolean found, foundOK= false;
    
    while (true) {
      ch= src[ i ]; if (ch=='\0') break;
      
      found= false; // <search> sequence ?
      if (i==0 || src[ i-1 ]=='\\') { // start or separator
        k= 0;
        while (true) {
          v= search[   k ]; 
          w= src   [ i+k ];
          
          if (v=='\0') {
            found= w=='\0' || 
                   w=='.'  ||
                   w=='\\'; // end or separator
            break;
          } // if
          if (toupper((unsigned char) v )!=toupper((unsigned char) w )) break;
          
          k++;
        } // loop
      } // if
      
      if (found) {
        i+= k; // skip <search> sequence
 
        k= 0; // insert <replace> sequence
        while (true) {
          v= replace[ k ]; 
          
          if (v=='\0') break;
          dst[ j ]= v;          
          
          j++;
          k++;
        } // loop
        
        foundOK= true;
      }
      else {
        dst[ j ]= ch;
        
        i++;
        j++;
      }
    } // loop
    
    dst[ j ]= '\0';
  //if (foundOK) upe_printf( "con src='%s' dst='%s'\n", src, dst );
  } // StrReplace
  
  
  
  os9err AdjustPath( const char* pathname, char* adname, Boolean creFile )
  /* do more or less nothing */
  { 
  //char* p;
      
    if ( *pathname==NUL ) return E_BPNAM;
      
    /* "con" is a problem in windows => convert it to ".con" */
    StrReplace( adname, pathname, "CON", ".CON" );
      
    /*
    strcpy( adname,pathname );
    if (ustrcmp( adname,   "con" )==0) 
        strcpy ( adname,  ".con" );
      
    p= adname + strlen( adname )-strlen( "\\con" );
    if (ustrcmp( p,      "\\con" )==0) {
        *p= NUL;
        strcat ( adname,"\\.con" );
    }
    */
      
  //upe_printf( "%s\n", adname );
    EatBack( adname );
      
    if (!creFile) { /* can't be here, if file not yet exists */
      if (!FileFound( adname )) {
        #ifdef windows32
          if (GetLastError()==ERROR_NOT_READY) return os9error(E_NOTRDY);
        #endif
        
        if (!PathFound( adname )) {
          #ifdef windows32
            if (GetLastError()==ERROR_NOT_READY) return os9error(E_NOTRDY);
          #endif
          
          return E_PNNF;
        } // if
      } // if
    } // if
      
    return 0;
  } /* AdjustPath */
#endif

#ifdef win_unix
  void GetEntry( dirent_typ* dEnt, char* name, Boolean do_2e_conv )
  /* Get the <name> of dir entry <dEnt> */
  /* Convert 2e string, if <do_2e_conv> is true */
  {
      char* q;
      const int L_Plen= strlen(L_P);
      
      /* strncpy does NOT terminate when the source is at least as long as the
       * limit, and both callers pass exactly char[DIRNAMSZ]. A host filename of
       * >=28 characters therefore filled `name` with no NUL at all, and every
       * C-string operation below then ran off the end -- Valgrind reported 36
       * "conditional jump depends on uninitialised value(s)" hits inside the
       * strstr() below, reached from CaseSens/AdjustPath, i.e. on ordinary path
       * resolution. Copy one fewer byte and terminate explicitly. 27 chars is
       * the real ceiling here regardless: the caller turns this back into an
       * OS-9 high-bit-terminated name via strlen(), which needs the NUL. */
      strncpy( name, dEnt->d_name, DIRNAMSZ-1 );
      name[DIRNAMSZ-1]= NUL;

      if (ustrncmp( name,L_P, L_Plen )==0 && do_2e_conv) {
            /* memmove, not strcat: source and destination are the SAME buffer
             * (appending name+L_Plen onto name+1), which strcat does not allow
             * -- overlapping arguments are undefined behaviour. It happened to
             * work because the copy runs from a higher index to a lower one. */
            size_t restlen= strlen( &name[L_Plen] );
            memmove( &name[1], &name[L_Plen], restlen+1 );
            name[0]= '.';  /* convert ".xx" string */
      }
      
      while (true) {
           q= strstr( name," " ); if (q==NULL) break; /* replace " " by "_" */
          *q= '_';
      } /* loop */
  } /* GetEntry */
#endif

Boolean RBF_ImgSize( long size )
/* Returns true, if it is a valid RBF Image size */
{
 //   if (size<8192 || (size % 2048)!=0)         return E_PNNF;
 //   if (size<1024 || (size % STD_SECTSIZE)!=0) return E_PNNF;
  return size>=1024 && (size % STD_SECTSIZE)==0;
} /* RBF_ImgSize */

#if defined MACOS9
  os9err RBF_Rsc( FSSpec *fs )
  {
      os9err     err;
      OSErr      oserr;
      CInfoPBRec cipb;
      ulong      size;
      short      refnum;
      int        cnt;
      char       bb[256]; /* one sector */
      
      err= getCipb( &cipb, fs ); if (err) return err;
      size= (ulong)  cipb.hFileInfo.ioFlLgLen;
      
      if (!RBF_ImgSize( size )) return E_PNNF;
            
          oserr=FSpOpenDF( fs, fsRdPerm, &refnum );
      if (oserr) return host2os9err(oserr,E_PNNF);

      /* Zeroed for the same reason as GetRBFName's copy further down: this
         FSRead's result is overwritten by FSClose's on the very next line, so
         a failed read was indistinguishable from a good one and the Cruzli
         strcmp then ran on stack garbage. Zeroing makes a failed read compare
         as "not an OS-9 image", which is the answer it should always have
         given. Otherwise left as found -- Mac Classic, untestable here. */
      memset( bb, 0, sizeof(bb) );
                               cnt= sizeof(bb);
      oserr= FSRead ( refnum, &cnt,       &bb );
      oserr= FSClose( refnum );     /* is this really an OS-9 partition ? */
      if (strcmp( &bb[CRUZ_POS],Cruz_Str )!=0) return E_PNNF; /* Cruzli check */
      
      return 0;
  } /* RBF_Rsc */

  os9err GetRBFName( char* os9path, ushort mode,
                     Boolean *isFolder, FSSpec *fs, FSSpec *afs )
  {
      os9err     err;
      char*      pp;
      Str255     nMac;
      char       sv[OS9PATHLEN];

      *isFolder= false; /* every exit must leave this defined -- see win_unix variant */

      strcpy( sv, os9path );
      pp=     sv; CutRaw( &pp );
      err= parsepath( 0,  &pp, nMac, false ); if (err) return E_PNNF;

      #ifdef RBF_SUPPORT
        err= Resolved_FSSpec( startVolID,startDirID, nMac, isFolder, fs,afs );
        if (err)          return err;
        if (*isFolder)    return E_FNA;
        if (!IsDir(mode)) return E_FNA;

        err= RBF_Rsc( fs );
      
      #else
        #pragma unused(mode,isFolder,fs,afs)
        err= E_UNIT;
      #endif
      
      return err;
  } /* GetRBFName */

#elif defined win_unix

  /* Returns true if <hostpath> (an absolute host path, which may not yet
   * exist -- e.g. a file about to be created) falls within one of the
   * currently configured host-native device roots (/dd, or any /h0-/h9,
   * /ha-/hz that resolves to a real directory). Without this, any
   * absolute OS-9 path whose first component isn't a recognized 2-char
   * device name falls straight through parsepathext/AdjustPath unmodified
   * and gets treated as a literal host path -- e.g. "chd /usr" or
   * "chd /etc" lands in the real host /usr or /etc, and ".." above a
   * device root walks into the real host filesystem, since host-native
   * directories (unlike RBF images, which clamp at a real root inode)
   * have no root of their own to stop at. Confirmed live 2026-07-06 (see
   * os9exec-host-confinement project memory).
   *
   * Resolves each candidate device name (dd, h0-h9, ha-hz) via
   * TwoCharDev() -- the SAME function parsepathext itself uses -- rather
   * than re-deriving device roots independently (e.g. checking only the
   * OS9DISK/OS9Hx env vars directly was an earlier, incomplete version of
   * this fix: it missed the equally-valid "h0"/"h1"/etc. directory or
   * symlink auto-discovered next to the binary when no env var is set,
   * which is how this project's own dd/h0/h1 are actually configured --
   * confirmed live when it broke ordinary command lookup via /h1/CMDS).
   * realpath() canonicalizes both sides to close off residual ".." or
   * symlink tricks (important here: this is the check applied to the
   * FINAL, already-".."-collapsed path, so a symlink planted inside a
   * device that points back out is still caught); since realpath()
   * requires its target to exist, walk up to the nearest existing
   * ancestor first -- a not-yet-created file/dir is fine as long as its
   * parent chain is real and inside a configured root. */
  Boolean HostPathWithinConfiguredDevice( const char* hostpath )
  {
      char nameOut[3];
      return HostPathDeviceName( hostpath, nameOut );
  } /* HostPathWithinConfiguredDevice */

  #ifdef MINGW
  /* AdjustPath()'s confinement check (winfiles.c, via
   * HostPathWithinConfiguredDevice above) runs on the LEXICAL host path --
   * a host symlink placed inside a device root but pointing outside it
   * still lexically looks confined, and this file's realpath() shim above
   * doesn't follow symlinks to catch it either (built on _fullpath(),
   * which is purely lexical, unlike POSIX realpath()). A caller's actual
   * fopen() DOES follow the symlink for real, though -- confirmed live: a
   * symlink inside a device root pointing at a file outside it leaked
   * that file's content past confinement.
   *
   * Deliberately NOT fixed by making realpath() itself symlink-aware
   * (tried first, reverted): HostPathWithinConfiguredDevice runs on EVERY
   * AdjustPath() call -- i.e. on every path resolution in the whole
   * emulator -- so swapping realpath()'s underlying Win32 call for a
   * more expensive/different one had broad, hard-to-characterize side
   * effects elsewhere (module search for a logged-in user regressed).
   * This is the narrow alternative: re-check confinement ONLY at the
   * point a stream is actually opened (pFopen, fileaccess.c), against
   * the stream's OWN already-open handle -- no extra CreateFile call, no
   * interaction with concurrent access to the same file. If fopen()
   * followed a symlink out of bounds, the handle's real resolved path
   * won't match any configured device root, and the caller should refuse
   * to hand back data read through it. */
  /* Fully resolve <path> the way the OS itself does -- following junctions,
   * symlinks and mount points -- and normalize to this codebase's driveless,
   * '/'-separated convention. Unlike the _fullpath()-based realpath() shim
   * above (purely lexical), this opens the object and asks Windows where the
   * handle actually landed. FILE_FLAG_BACKUP_SEMANTICS is what allows a
   * DIRECTORY to be opened here; without it a device root fails outright.
   * Returns false if the object cannot be opened at all. */
  static Boolean FinalHostPath( const char* path, char* out, size_t outsz )
  {
      HANDLE h;
      DWORD  len;
      char*  p;

      h= CreateFileA( path, 0, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                      NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );
      if (h==INVALID_HANDLE_VALUE) return false;

      len= GetFinalPathNameByHandleA( h, out, (DWORD)outsz, FILE_NAME_NORMALIZED );
      CloseHandle( h );
      if (len==0 || len>=outsz) return false;

      p= out;
      if (strncmp( p,"\\\\?\\",4 )==0) p+= 4;
      if (p[0] && p[1]==':')           p+= 2;
      if (p!=out) memmove( out,p, strlen(p)+1 );
      for (p= out; *p; p++) if (*p=='\\') *p= '/';
      return true;
  } /* FinalHostPath */

  /* Like HostPathWithinConfiguredDevice, but resolves the DEVICE ROOTS through
   * the real filesystem too, so both sides of the comparison have been through
   * the same resolution.
   *
   * Needed because a device root may itself be a junction/symlink: this
   * project's own h0 is routinely an NTFS junction to the (proprietary,
   * out-of-tree) SDK test disk. GetFinalPathNameByHandle resolves the junction
   * for the open stream ("/Users/x/os9test/h0/CMDS/shell") while the lexical
   * realpath() shim leaves the root unresolved ("/Users/x/repo/h0"), so the
   * prefix test below could never match and EVERY ordinary file open inside the
   * device was rejected as an escape -- os9exec could not even load `shell` and
   * refused to boot. Confirmed live 2026-07-18 on a junctioned h0.
   *
   * Confinement is not weakened: a symlink planted inside a root that points
   * outside every configured device still resolves to a path matching no root,
   * which is exactly what this check exists to catch. */
  static Boolean FinalPathWithinConfiguredDevice( const char* finalPath )
  {
      char   rootreal[PATH_MAX];
      char   devbuf[3];
      char   tmp[OS9PATHLEN];
      char   ch;
      char*  root;
      size_t rl;

      #define FINAL_DEV_MATCHES( d0,d1 ) \
          ( devbuf[0]=(d0), devbuf[1]=(d1), devbuf[2]=NUL, \
            root=NULL, TwoCharDev( devbuf,&root,tmp ), \
            root!=NULL && *root!=NUL && \
            FinalHostPath( root,rootreal,sizeof(rootreal) ) && \
            ( rl= strlen(rootreal), ustrncmp( finalPath,rootreal,rl )==0 && \
              (finalPath[rl]==NUL || finalPath[rl]==PATHDELIM) ) )

      if (FINAL_DEV_MATCHES( 'd','d' )) return true;
      for (ch= '0'; ch<='9'; ch++) if (FINAL_DEV_MATCHES( 'h',ch )) return true;
      for (ch= 'a'; ch<='z'; ch++) if (FINAL_DEV_MATCHES( 'h',ch )) return true;

      #undef FINAL_DEV_MATCHES
      return false;
  } /* FinalPathWithinConfiguredDevice */

  Boolean HostStreamWithinConfiguredDevice( FILE* stream )
  {
      HANDLE h;
      char   finalPath[PATH_MAX];
      DWORD  len;
      char*  p;

      h= (HANDLE)_get_osfhandle( _fileno(stream) );
      if (h==INVALID_HANDLE_VALUE) return true; /* can't check -- don't block a normal open */

      len= GetFinalPathNameByHandleA( h, finalPath, PATH_MAX, FILE_NAME_NORMALIZED );
      if (len==0 || len>=PATH_MAX) return true; /* couldn't resolve -- don't block a normal open */

      /* Match realpath()'s own normalization above: strip the "\\?\"
       * extended-length marker GetFinalPathNameByHandle always adds, drop
       * the drive letter, flip \ to /. */
      p= finalPath;
      if (strncmp( p,"\\\\?\\",4 )==0) p+= 4;
      if (p[0] && p[1]==':')           p+= 2;
      if (p!=finalPath) memmove( finalPath,p, strlen(p)+1 );
      for (p= finalPath; *p; p++) if (*p=='\\') *p= '/';

      /* Lexical roots first (cheap, and the usual case when no device root is
       * itself a link); fall back to fully-resolved roots for a junctioned or
       * symlinked device root. */
      if (HostPathWithinConfiguredDevice( finalPath )) return true;
      return FinalPathWithinConfiguredDevice( finalPath );
  } /* HostStreamWithinConfiguredDevice */
  #endif

  /* Same resolution as HostPathWithinConfiguredDevice, but also reports
   * WHICH device (dd, h0-h9, ha-hz) <hostpath> falls within, via
   * <nameOut> (must be at least 3 bytes). Used by pHvolnam (fileaccess.c,
   * the SS_DevNm GetStat handler for host-native paths) to report the
   * real device name instead of its previous, always-wrong behavior of
   * extracting whatever the first slash-delimited component of the RAW
   * HOST PATH happened to be (e.g. "Users" from
   * "/Users/rdoggett/.../dd/CMDS" -- confirmed live 2026-07-06). RBF's
   * own equivalent (pRnam, file_rbf.c) already reports its clean device
   * name this way, which is what lets a real OS-9 "pd" skip straight to
   * printing "/h0" at an RBF root instead of walking "..": it recognizes
   * "no further path component" from the device-name GetStat result
   * alone. Host-native "pd" fell back to the ".."-walking algorithm
   * purely because this GetStat was returning noise -- see the
   * os9exec-host-confinement project memory for the full chain. */
  Boolean HostPathDeviceName( const char* hostpath, char* nameOut )
  {
      char  real[PATH_MAX];
      char  rootreal[PATH_MAX];
      char  probe[PATH_MAX];
      char  devbuf[3];
      char  tmp[OS9PATHLEN];
      char  ch;
      char* slash;
      char* root;
      size_t rl;

      if (hostpath==NULL || *hostpath==NUL) return false;

      strncpy( probe, hostpath, sizeof(probe)-1 );
      probe[sizeof(probe)-1]= NUL;
      while (realpath( probe, real )==NULL) {
          slash= strrchr( probe, PATHDELIM );
          if (slash==NULL || slash==probe) return false; /* nothing left to try */
          *slash= NUL;
      }

      #define DEV_MATCHES( d0,d1 ) \
          ( devbuf[0]=(d0), devbuf[1]=(d1), devbuf[2]=NUL, \
            root=NULL, TwoCharDev( devbuf,&root,tmp ), \
            root!=NULL && *root!=NUL && \
            realpath( root,rootreal )!=NULL && \
            ( rl= strlen(rootreal), ustrncmp( real,rootreal,rl )==0 && \
              (real[rl]==NUL || real[rl]==PATHDELIM) ) )

      if (DEV_MATCHES( 'd','d' )) { strcpy(nameOut,devbuf); return true; }
      for (ch= '0'; ch<='9'; ch++)
          if (DEV_MATCHES( 'h',ch )) { strcpy(nameOut,devbuf); return true; }
      for (ch= 'a'; ch<='z'; ch++)
          if (DEV_MATCHES( 'h',ch )) { strcpy(nameOut,devbuf); return true; }

      #undef DEV_MATCHES
      return false;
  } /* HostPathDeviceName */

  /* True iff <hostpath> resolves to EXACTLY a configured device root
   * (dd, h0-h9, ha-hz) -- not merely a path within one. Like
   * HostPathDeviceName but requires the canonicalized paths to be equal,
   * with nothing left over. Used by the directory reader (pDread,
   * fileaccess.c) to recognize when it is enumerating a device root's own
   * entries, so it can make the synthesized ".." entry self-referential
   * (RBF-root semantics) -- see the comment at that call site. */
  /* Compare two already-canonicalized host paths for equality, tolerating one
   * or more trailing path separators on either side.
   *
   * POSIX realpath() never returns a trailing separator (except for "/"
   * itself), so on Unix/macOS a plain string compare was enough and this is
   * a no-op. MinGW has no realpath(); the _fullpath()-based shim above
   * PRESERVES a trailing separator, so a device root reached as ".../h0/"
   * canonicalized to ".../h0/" and never matched the configured root's
   * ".../h0". IsHostDeviceRoot then reported false for the device root
   * itself, the synthesized ".." entry lost the self-reference that marks
   * top-of-device, and every directory walker (pd, dsave) escaped the root
   * and ran on until E_EOF.
   *
   * Fixed here at the comparison rather than in the realpath() shim: the shim
   * is on the AdjustPath() hot path for every path resolution in the
   * emulator, and a previous attempt to change its semantics caused
   * wide-blast-radius regressions. Both separators are accepted because the
   * Win32 canonicalizer can emit either. */
  static Boolean SamePathIgnoringTrailingDelim( const char* a, const char* b )
  {
      size_t la= strlen( a );
      size_t lb= strlen( b );

      while (la>1 && (a[la-1]==PATHDELIM || a[la-1]=='/')) la--;
      while (lb>1 && (b[lb-1]==PATHDELIM || b[lb-1]=='/')) lb--;

      return (Boolean)( la==lb && ustrncmp( a,b, la )==0 );
  } /* SamePathIgnoringTrailingDelim */

  Boolean IsHostDeviceRoot( const char* hostpath )
  {
      char  real[PATH_MAX];
      char  rootreal[PATH_MAX];
      char  devbuf[3];
      char  tmp[OS9PATHLEN];
      char  ch;
      char* root;

      if (hostpath==NULL || *hostpath==NUL)   return false;
      if (realpath( hostpath, real )==NULL)    return false;

      #define ROOT_IS( d0,d1 ) \
          ( devbuf[0]=(d0), devbuf[1]=(d1), devbuf[2]=NUL, \
            root=NULL, TwoCharDev( devbuf,&root,tmp ), \
            root!=NULL && *root!=NUL && \
            realpath( root,rootreal )!=NULL && \
            SamePathIgnoringTrailingDelim( real,rootreal ) )

      if (ROOT_IS( 'd','d' )) return true;
      for (ch= '0'; ch<='9'; ch++) if (ROOT_IS( 'h',ch )) return true;
      for (ch= 'a'; ch<='z'; ch++) if (ROOT_IS( 'h',ch )) return true;

      #undef ROOT_IS
      return false;
  } /* IsHostDeviceRoot */

  /* Finds which configured device root (dd, h0-h9, ha-hz) <pathname>
   * *literally starts with*, if any, and copies that root's own
   * canonical (realpath'd) host path into <rootOut> (must be at least
   * PATH_MAX bytes). Returns false, leaving <rootOut> untouched, if none
   * match. When roots nest, the INNERMOST (longest-matching) one wins --
   * see the scan below.
   *
   * Deliberately does NOT realpath() <pathname> itself, unlike
   * HostPathWithinConfiguredDevice above -- this exists specifically for
   * AdjustPath (linuxfiles.c) to check its input BEFORE CutUp() collapses
   * any "/../" sequences in it. At that point <pathname> can still
   * literally be ".../dd/../../..": realpath()-ing it directly would
   * eagerly resolve straight through the ".." and answer a completely
   * different question ("where does this end up", which is exactly what
   * HostPathWithinConfiguredDevice is for) instead of "did this start
   * inside a device root at all", which is what a literal string-prefix
   * match answers correctly. AdjustPath uses this to distinguish "'..'
   * walked above the root it started in" (clamp back to that root) from
   * "never referenced a configured device to begin with, e.g. a literal
   * /etc" (reject) -- see the confinement check at the end of
   * AdjustPath, and the os9exec-host-confinement project memory. */
  Boolean FindConfiguredDeviceRoot( const char* pathname, char* rootOut )
  {
      char  devbuf[3];
      char  tmp[OS9PATHLEN];
      char  ch;
      char* root;
      size_t rl;
      char   best[PATH_MAX];
      size_t bestLen= 0;

      if (pathname==NULL || *pathname==NUL) return false;

      /* Compare against <root> itself (the literal string TwoCharDev/
       * parsepathext already substituted into the path, e.g. ".../dd"),
       * NOT its realpath()'d form -- "dd" is a symlink to "freeware" in
       * this very repo, so realpath("dd") is ".../freeware", which would
       * never textually match a pathname that still literally says
       * ".../dd/../". Confirmed live: this exact mismatch made pd's own
       * ".." lookup at the /dd root fail to clamp (fell through to
       * outright rejection instead) until fixed. */
      #define DEV_MATCHES( d0,d1 ) \
          ( devbuf[0]=(d0), devbuf[1]=(d1), devbuf[2]=NUL, \
            root=NULL, TwoCharDev( devbuf,&root,tmp ), \
            root!=NULL && *root!=NUL && \
            ( rl= strlen(root), ustrncmp( pathname,root,rl )==0 && \
              (pathname[rl]==NUL || pathname[rl]==PATHDELIM) ) )

      /* Keep the LONGEST match, not the first one found. Device roots can
       * NEST -- `mount -k=0 hb` with the working directory inside h5's own
       * host root makes <h5root>/hb a device root inside a device root -- and
       * every enclosing root is equally a literal prefix of a path in the
       * inner one. Returning the first match meant the scan order (dd, h0-h9,
       * ha-hz) decided, so a path under /hb reported h5 as "the root it
       * started in", AdjustPath's clamp then compared against the wrong root,
       * and `..` from /hb walked straight out into h5 (live-verified before
       * this fix: `list ../parentfile` from /hb read h5's file, and `dir ..`
       * listed h5's root). The innermost containing root is the one the
       * operation is actually in, and the longest matching prefix IS the
       * innermost. <root> must be copied here and now: TwoCharDev may hand
       * back a pointer into <tmp>, which the next iteration overwrites. */
      #define KEEP_LONGEST \
          if (rl>bestLen) { bestLen= rl; \
                            strncpy( best,root,PATH_MAX-1 ); best[PATH_MAX-1]= NUL; }

      if (DEV_MATCHES( 'd','d' )) KEEP_LONGEST;
      for (ch= '0'; ch<='9'; ch++)
          if (DEV_MATCHES( 'h',ch )) KEEP_LONGEST;
      for (ch= 'a'; ch<='z'; ch++)
          if (DEV_MATCHES( 'h',ch )) KEEP_LONGEST;

      #undef KEEP_LONGEST
      #undef DEV_MATCHES

      if (bestLen==0) return false;
      strcpy( rootOut,best );
      return true;
  } /* FindConfiguredDeviceRoot */

  /* Resolves an OS-9-style path to its would-be host path (same
   * parsepath+AdjustPath resolution GetRBFName does) and checks it
   * against HostPathWithinConfiguredDevice. Used by IO_Type's own final
   * fallback (see below): when nothing else classifies an absolute path
   * (OS9_Device already said no), IO_Type used to unconditionally trust
   * the caller's requested mode and default to fDir/fFile anyway -- the
   * actual last permissive-by-default gap that let "/etc"/"/usr" through
   * even after GetRBFName correctly rejected them, since OS9_Device
   * returning false was never enough on its own. */
  static Boolean OS9PathEscapesDeviceRoot( const char* os9path )
  {
      char  sv[OS9PATHLEN];
      char  adjust[OS9PATHLEN];
      char* pp;

      strncpy( sv, os9path, sizeof(sv)-1 );
      sv[sizeof(sv)-1]= NUL;
      pp= sv;
      if (parsepath( 0, &pp, adjust, false )) return false; /* not our concern here */
      strncpy( sv, adjust, sizeof(sv)-1 );
      sv[sizeof(sv)-1]= NUL;
      pp= sv;
      AdjustPath( pp, adjust, false ); /* ignore err -- adjust is populated
                                          regardless, same as GetRBFName */
      return !HostPathWithinConfiguredDevice( adjust );
  } /* OS9PathEscapesDeviceRoot */

  os9err GetRBFName( char* os9path, ushort mode,
                     Boolean *isFolder, char* rbfname, char* hostpath )
  /* <rbfname> gets the LAST COMPONENT of the host path this resolved to;
     <hostpath> (optional, may be NULL) gets that host path whole.  The two
     are not interchangeable: a caller that has only the last component has
     to rebuild the path from it, and rebuilding only lands back on the same
     file when the host file happens to be named after the OS-9 device -- see
     DeviceInit, which used to do exactly that. */
  {
      os9err err= 0;
      char   sv     [OS9PATHLEN];
      char   adjust [OS9PATHLEN];
      char   whole  [OS9PATHLEN]; /* <adjust> as it stood before the stripping loop */
      char   *pp, *qq;
      struct stat info;
      FILE*  stream;
      char   bb[STD_SECTSIZE]; /* one sector */

      /* Callers branch on *isFolder without first checking the returned error
         (OS9_Device), so every exit from here must leave it defined -- including
         the two early E_PNNF returns below, which used to leave the caller's
         local holding stack garbage.  A path that does not resolve is not a
         folder, so false is the right answer for them. */
      *isFolder= false;
      if (hostpath!=NULL) *hostpath= NUL; /* same rule: defined on every exit */

      strcpy    ( sv, os9path );
      pp= (char*)&sv; CutRaw( &pp );
      if (*pp==NUL || *pp!=PSEP) return E_PNNF; /* converted to an OS-9 path !! */
      EatBack   ( sv );            /* normalize /dev/. → /dev before host path resolution */

      err= parsepath( 0,  &pp,adjust, false ); if (err) return E_PNNF;
      strcpy( sv, adjust );
      pp =    sv;

      err= AdjustPath    ( pp,adjust, false );
      /* Don't abort if adjust failed — path may be a subpath inside an RBF image (e.g.
         /h0/CMDS).  AdjustPath already populated adjust via EatBack; let the stripping
         loop below find the image by peeling off trailing components. */
      if (err) err= 0;
      pp =                    adjust;

      /* Keep the un-stripped path. The loop below walks <pp>, which points INTO
       * <adjust>, and terminates it in place with *qq=NUL -- so after even one
       * strip, <adjust> and <pp> are the same string and comparing them can no
       * longer tell "the caller named the image file itself" from "the caller
       * named something inside it". See the check further down that needs it. */
      strcpy( whole, adjust );

      /* cut the path piece by piece (no sub paths within RBF images) */
      /* If AdjustPath/parsepath already reduced <pp> to an empty string
       * (e.g. a top-level path like "/test" that matches no known device),
       * pp+strlen(pp)-1 underflows to pp-1 -- a pointer qq can never equal
       * again, since the "qq>pp" guard below stops it from ever reaching
       * pp. That made the loop's "pp==qq" exit condition unreachable: an
       * infinite loop repeatedly checking the same empty path and writing
       * one byte before the buffer on every pass. Guard qq at pp itself
       * when pp is already empty, so the first pass's "pp==qq" check ends
       * the loop immediately with the correct E_PNNF instead of hanging. */
      qq = (*pp) ? pp+strlen(pp)-1 : pp;
      while (true) {
          if (!HostPathWithinConfiguredDevice( pp )) {
              /* escaped every configured device root -- treat exactly
               * like a nonexistent path rather than falling through to
               * the real host filesystem (see HostPathWithinConfiguredDevice) */
              *isFolder= false;
              err= E_PNNF;
          }
          else {
              *isFolder=    PathFound( pp );
              if  (!*isFolder && !FileFound( pp )) err= E_PNNF;
              else if (!*isFolder)                 err= 0; /* file found at pp — stop stripping */
          }

          debugprintf( dbgFiles,dbgNorm,("# GetRBFName: '%s' mode=%d err=%d (%s)\n",
                                            pp, mode,err, *isFolder ? "dir":"file" ));
          if (!err || pp==qq) break;

          while (*qq!=PATHDELIM && qq>pp) qq--;
          *qq= NUL;
      } /* loop */

      do {
          /* the first conditions for RBF image are min size and granularity */
          if (*isFolder) { err= E_FNA; break; }

          /* allow to access the image as a normal file, but not for root/raw device paths
             (e.g. /h0, /h0@, /dd) where the caller wants directory or raw-device access.
             Compare against <whole>, NOT <adjust>: the stripping loop truncates <adjust>
             in place through <pp>, so "adjust==pp" was true whenever anything had been
             stripped and this fired for every ordinary file inside an image. The effect
             was that opening a FILE on a not-yet-installed RBF image returned E$FNA, so
             IO_Type fell back to the host managers and the open failed E$PNNF -- while
             opening a DIRECTORY on the same image installed it and then worked. Hence
             `list /h1/hello.c` failed as the first command of a session but succeeded
             once anything had touched /h1. */
          if (ustrcmp(whole,pp)==0 && !IsDir(mode) && !IsRoot(os9path) && !IsRaw(os9path)) { err= E_FNA; break; }

          err= stat_( pp,  &info );           if (err) { err= E_PNNF; break; }
          if (!RBF_ImgSize( info.st_size ))            { err= E_FNA;  break; }

          stream= fopen( pp,"rb" );  if (stream==NULL) { err= E_PNNF; break; }
          /* Zeroed first, and the count CHECKED. The `(void)` cast did not
             silence glibc's warn_unused_result and should not have: on a short
             or failed read <bb> kept whatever was on the stack, and the Cruzli
             strcmp below then classified an image by uninitialised memory --
             and could run past the buffer hunting a terminator that was never
             written. Zeroing makes the comparison safe; checking the count
             makes a truncated file answer "not an OS-9 image", which is true. */
          memset( bb, 0, sizeof(bb) );
          if (fread( &bb, 1,sizeof(bb), stream )!=sizeof(bb)) {
              fclose( stream );                        err= E_FNA;  break;
          }
          fclose( stream ); /* is this really an OS-9 partition ? => Cruzli check */
          if (strcmp( &bb[CRUZ_POS],Cruz_Str )!=0)     { err= E_FNA;  break; }
      } while (false);

      if (hostpath!=NULL) {
          strncpy( hostpath, pp, OS9PATHLEN-1 );
                   hostpath[  OS9PATHLEN-1 ]= NUL;
      }

      qq = pp+strlen(pp);
      while (*qq!=PATHDELIM && qq>pp) qq--;
      qq++;

      strcpy( rbfname, qq );
      debugprintf( dbgFiles,dbgNorm,("# GetRBFName: '%s' host='%s' err=%d\n", rbfname,pp,err ));
      return err;
  } /* GetRBFName */
#endif

// #ifdef RAM_SUPPORT
Boolean RAM_Device( const char* os9path )
/* check, if it is a RAM device */
{
    char     cmp[OS9PATHLEN];
    mod_dev* mod;
    char*    p;
    
    GetOS9Dev( os9path, (char*)&cmp );
    if (mnt_ramSize>0 || strcmp( mnt_devCopy,"" )!=0) return true;
    
    if (IsDesc( cmp, &mod, &p )  && ustrcmp( p,"RBF" )==0) {
        p= (char*)mod + os9_word(mod->_mpdev);
        return ustrcmp( p,"ram" )==0;
    }
    
    return false;
} /* RAM_Device */
// #endif

static Boolean OS9_Device( char* os9path, ushort mode, ptype_typ *typeP )
/* Returns true, if <os9path> is an RBF device */
{
    os9err err= 0;
    int    id;
    short  lun;
    short  bus;
    short  adapt;
    ushort sas,ssize;
    byte   pdtyp;
    
    #ifdef MACOS9
      Boolean isFolder= false;
      FSSpec  fs,afs;

    #elif defined win_unix
      Boolean isFolder= false;
      char    rbfname[OS9PATHLEN];
    #endif
    
    #ifdef RBF_SUPPORT
      ushort cdv;
    #endif
    

    *typeP= fRBF; /* default value */
    #ifdef RBF_SUPPORT
      if (InstalledDev( os9path,"",false, &cdv )) return true; /* already ? */
      
      #ifdef RAM_SUPPORT
        if (RAM_Device( os9path )) return true;
      #endif
    #endif
    
    #ifdef MACOS9
                        err= GetRBFName(  os9path,   mode, &isFolder, &fs,&afs );
      if  (err==E_UNIT) err= GetRBFName( &os9path[1],mode, &isFolder, &fs,&afs );

    #elif defined win_unix
                        err= GetRBFName(  os9path,   mode, &isFolder, (char*)&rbfname, NULL );
      if  (err==E_UNIT) err= GetRBFName( &os9path[1],mode, &isFolder, (char*)&rbfname, NULL );

    #else 
      /* %%% some fixed devices defined currently */
      isFolder= false;
      if (ustrncmp( os9path,"/mt",3 )!=0 &&
          ustrncmp( os9path,"/c1",3 )!=0 &&
          ustrncmp( os9path,"/c2",3 )!=0 &&
          ustrncmp( os9path,"/c3",3 )!=0 &&
          ustrncmp( os9path,"/dd",3 )!=0) err= E_MNF;
    #endif
    
    /* try it again, it might be installed now */
    #ifdef RBF_SUPPORT
      if (err && InstalledDev( os9path,"",false, &cdv )) return true; /* already ? */
    #endif

    /* A RAW open ("/dd@") of a device backed by a host DIRECTORY. It arrives
     * here as E$FNA with isFolder set -- "you asked to open a folder as a
     * file" -- and the E$FNA line below would call it fNone, i.e. no such
     * device module, so every caller got E$MNF for a device that plainly
     * exists. Hand it to the host file manager instead: pFopen strips the '@',
     * sees rawMode, and its "host directories have no disk sectors" guard
     * answers E$Unit. Raw access stays refused -- that decision (063f8d1)
     * is unchanged; only the error becomes an honest one. The guard could
     * never fire before, because classification rejected the path first.
     * Verified as the deciding branch by tracing err/isFolder live, not by
     * reading: an earlier attempt patched the E$PNNF probe below and changed
     * nothing. */
    if (IsRaw( os9path ) && isFolder) { *typeP= fFile; return true; }

    if (!IsDir(mode) && err==E_FNA) { *typeP= fNone; return false; }
    if (!err && !isFolder)          { *typeP= fRBF;  return true;  }

    /* host path is a directory — honour that before checking SCSI/device descriptors;
       a device-descriptor module with the same name must not override a real host dir */
    if (isFolder && IsDir(mode))    { *typeP= fDir;  return true;  }

    /* If the path doesn't resolve on the host (E_PNNF — new file being created, or subpath
       inside a host-directory device), distinguish host-dir from RBF by resolving just the
       two-char device prefix.  If the prefix maps to a host directory (e.g., OS9H2 or the
       auto-discovered /h2 folder), the path belongs to the host-filesystem managers.
       RBF image files (e.g., OS9DISK → /dd file) are NOT directories, so parsepath on
       "/dd" returns a file path, PathFound() returns false, and we fall through to SCSI. */
    if (err==E_PNNF) {
        const char *op = os9path;
        if (*op==PSEP) op++;
        if (op[0]!=NUL && op[1]!=NUL && (op[2]==PSEP || op[2]==NUL)) {
            char devpath[4] = { PSEP, op[0], op[1], NUL };
            char hostdev[OS9PATHLEN];
            char *dp        = devpath;
            if (!parsepath(0, &dp, hostdev, false) && PathFound(hostdev)) {
                if (IsDir(mode)) { *typeP= fDir;  return true;  }
                else             { *typeP= fNone; return false; }
            }
        }
        /* Neither the full path nor its 2-char device prefix resolves on
         * the host — this isn't a host-filesystem-backed path at all (and,
         * per HostPathWithinConfiguredDevice in GetRBFName, may have been
         * deliberately kept out of bounds — e.g. "/etc", "/usr"). *typeP
         * still holds its unconditional fRBF default from the top of this
         * function; without resetting it here, the final "*typeP!=fNone"
         * fallback below would trust that leftover default and return
         * true regardless of what was actually determined above. Only a
         * real SCSI device descriptor should be considered past this
         * point. */
        *typeP= fNone;
    }

    /* searching for SCSI after searching file image !! */
    if (SCSI_Device( os9path, &adapt, &bus, &id, &lun, &ssize, &sas,&pdtyp, typeP ) ||
                                  *typeP!=fNone ) return true;
    return false;
} /* OS9_Device */

char* Mod_TypeStr( mod_exec* mod )
{
    char* nam;
    int   ty= os9_word(mod->_mh._mtylan)>>BpB;

    switch (ty) {
        case MT_ANY     : nam="    "; break;
        case MT_PROGRAM : nam="Prog"; break;
        case MT_SUBROUT : nam="Subr"; break;
        case MT_MULTI   : nam="Mult"; break;
        case MT_DATA    : nam="Data"; break;
        case MT_CSDDATA : nam="CSDD"; break;
        case MT_TRAPLIB : nam="Trap"; break;
        case MT_SYSTEM  : nam="Sys" ; break;
        case MT_FILEMAN : nam="Fman"; break;
        case MT_DEVDRVR : nam="Driv"; break;
        case MT_DEVDESC : nam="Desc"; break;
        default         : nam="????";
    }
    
    return nam;
} /* Mod_TypeStr */

char* PStateStr( process_typ* cp )
{
    char* nam;
    switch (cp->state) {
        case pUnused  : nam="pUnused";   break;
        case pActive  : nam="pActive";   break;
        case pDead    : nam="pDead";     break;
        case pSleeping: nam="pSleeping"; break;
        case pWaiting : nam="pWaiting";  break;
        case pSysTask : nam="pSysTask";  break;
        case pWaitRead: nam="pWaitRead"; break;
        default       : nam="unknown";
    } // switch
    
    if (cp->isIntUtil)  nam="pIntUtil";
    
    return nam;
} /* PStateStr */

char* TypeStr( ptype_typ type )
{
    char* nam;
    switch (type) {
        case fNone: nam="fNone"; break;
        case fCons: nam="fCons"; break;
        case fTTY : nam="fTTY";  break;
        case fNIL : nam="fNIL";  break;
        case fVMod: nam="fVMod"; break;
        case fSCF : nam="fSCF";  break;
        case fFile: nam="fFile"; break;
        case fDir : nam="fDir";  break;
        case fPipe: nam="fPipe"; break;
        case fPTY : nam="fPTY";  break;
        case fRBF : nam="fRBF";  break;
        case fNET : nam="fNET";  break;
        default   : nam="unknown";
    }
    
    return nam;
} /* TypeStr */

char* spP_TypeStr( syspath_typ* spP )
{
    char*          nam= "---";
    if (spP!=NULL) nam= TypeStr(spP->type);
    return         nam;
} /* spP_TypeStr */

ptype_typ IO_Type(ushort pid, char* os9path, ushort mode)
/* get the I/O type of <os9path> */
{
    ptype_typ    type;
    process_typ* cp= &procs[pid];
    
    debugprintf( dbgFMgrType,dbgNorm,("# IO_Type  (in): '%s'\n", os9path) );

    do {
        if (ustrncmp(os9path,">", 1)==0 ||        /* "sectorfiles" compatibility */
            ustrncmp(os9path,"<", 1)==0) { type= fNone; break; }
        if (!AbsPath(os9path) ) {
            if (IsExec(mode)) type= cp->x.type;   /* the types of exec/cur dir */
            else              type= cp->d.type;
        
            if (type==fDir && !(mode & 0x80)) type= fFile;
            break;
        } /* if */
    
        if  (ustrcmp (os9path,"/nil"   )==0) { type= fNIL;     break; }
        if  (ustrncmp(os9path,"/pipe",5)==0) { type= fPipe;    break; }
        if  (ustrncmp(os9path,"/lp",  3)==0 ||
             ustrcmp (os9path,"/p"     )==0 ||
             ustrcmp (os9path,"/p1"    )==0) { type= fPrinter; break; }

        #ifdef TERMINAL_CONSOLE
          /* there is more than one serial device possible now */
          if (ustrcmp(os9path,MainConsole)==0  /* for OS9serial, only this name is known */
    
          #ifndef SERIAL_INTERFACE
           || ustrcmp(os9path,SerialLineA)==0 /* ts1 */
           || ustrcmp(os9path,SerialLineB)==0 /* ts2 */
          
           /* No longer #ifndef linux. /t1../t49 classified as fCons on macOS
              and as a FILE on Linux, so the same path was a console on one
              host and not on another -- an accident of the old Mac-only
              serial code, not a deliberate platform difference. Nothing in
              the tree depended on the Linux behaviour (checked). */
           || (ustrcmp (os9path,"/t0") == 0)
           || (os9path[0]==PSEP &&
               os9path[1]=='t'  && atoi(&os9path[2])>= 1 && /* /t1 ../t49 */
                                   atoi(&os9path[2])<VModBase)
          #endif
          ) { type= fCons; break; }

          if  (ustrcmp (os9path,"/tty00")==0  /* /tty0 */
           || (ustrncmp(os9path,"/tty",4)==0 
               && atoi(&os9path[4])>= 1       /* /tty1 .. /tty99 */
               && atoi(&os9path[4])<=99)) { type= fTTY; break; }

          if  (ustrcmp (os9path,"/pty00")==0  /* /pty0 */
           || (ustrncmp(os9path,"/pty",4)==0
               && atoi(&os9path[4])>= 1       /* /pty1 .. /pty99 */
               && atoi(&os9path[4])<=99)) { type= fPTY; break; }
        #endif
 
        /* "/vmod" is built-in */ 
        if    (ustrcmp( os9path,"/vmod" )==0) { type= fVMod; break; }
        if (OS9_Device( os9path,mode,          &type ))      break;

        /* OS9_Device said no -- don't unconditionally trust the caller's
         * requested mode and default to fDir/fFile regardless (the last
         * permissive-by-default gap: this used to let "/etc"/"/usr"
         * through even after every earlier classification correctly
         * rejected them). Only default to file/dir if the path still
         * resolves within a configured device root -- e.g. a new file
         * being created inside an existing device, which is legitimate
         * and already reaches here with a genuine host path underneath. */
        if (OS9PathEscapesDeviceRoot( os9path )) { type= fNone; break; }

        if (IsDir(mode)) type= fDir;
        else             type= fFile;
    } while (false);

    debugprintf( dbgFMgrType,dbgNorm,("# IO_Type (out): '%s' %s\n",
                                         os9path, TypeStr(type)) );
    return type;
} /* IO_Type */

/* eof */
