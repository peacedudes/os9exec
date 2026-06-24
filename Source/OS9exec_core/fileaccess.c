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
 *    Revision 1.51  2007/04/17 23:06:10  bfo
 *    FD_ID calling args changed and adapted
 *
 *    Revision 1.50  2007/04/13 10:08:43  bfo
 *    File copy correctly done for ftruncate workaround (MacOSX CW)
 *
 *    Revision 1.49  2007/04/10 22:13:19  bfo
 *    No longer dependent on "Volumes", same treatment MacOSX/Linux
 *
 *    Revision 1.48  2007/04/01 15:04:28  bfo
 *    workaround for MACH ftruncate: copy the file
 *
 *    Revision 1.47  2007/03/31 12:21:07  bfo
 *    - LINKED_HASH support (by using <cond>)
 *    - Set_FileDate supported now for MacOSX
 *
 *    Revision 1.46  2007/03/24 12:47:35  bfo
 *    - <readFlag> introduced (seek when change read -> write)
 *    - ftruncate adaptions (still not working for CW MacOSX)
 *    - check_vod moved to "macfiles"
 *
 *    Revision 1.45  2007/03/10 14:04:29  bfo
 *    MPW adaptions
 *
 *    Revision 1.44  2007/03/10 13:54:52  bfo
 *    MPW adaptions
 *
 *    Revision 1.43  2007/03/10 13:52:30  bfo
 *    MPW adaptions
 *
 *    Revision 1.42  2007/03/10 13:38:29  bfo
 *    Pending things not for 68k
 *
 *    Revision 1.41  2007/03/10 12:46:15  bfo
 *    - No longer GetFPos/SetFPos for every read/readln call
 *    - Use new <readFlag> instead
 *    - ".log" files are TEXT (for MACOS9)
 *    - DeletePath enhanced for MACOS9: Rename, move, delete (later) !
 *
 *    Revision 1.40  2007/02/24 14:28:30  bfo
 *    - All volume/file IDs will be stored as <hashV> now
 *    - General use of "assign_fdsect"
 *    - "/Volumes" adaption for MacOSX w/o Inode use
 *    - MAXTRIES_DEL debugging for directory delete (delay)
 *    - "check_vod" reads back <hashV>, <dirid> and <fName>
 *
 *    Revision 1.39  2007/02/14 20:57:07  bfo
 *    win32 remove will be done now with /A
 *
 *    Revision 1.38  2007/02/11 14:49:01  bfo
 *    "rmdir" adaption
 *
 *    Revision 1.37  2007/01/07 13:57:46  bfo
 *    Using MACOSX instead of __MACH__
 *
 *    Revision 1.36  2007/01/04 20:56:31  bfo
 *    'touchfile' only used for MACFILES
 *
 *    Revision 1.35  2006/10/12 20:00:54  bfo
 *    Several adaptions for Windows directory reading:
 *    - handling EOF correctly
 *    - pDsetatt with additional E_EOF support
 *    - rmdir with /S /Q parameters
 *    - dir flag adaption
 *
 *    Revision 1.34  2006/06/18 14:36:05  bfo
 *    Use 'SITD' instead of 'SIT!' for Mac file type now
 *
 *    Revision 1.33  2006/06/08 08:15:04  bfo
 *    Eliminate causes of signedness warnings with gcc 4.0
 *
 *    Revision 1.32  2006/05/27 00:13:22  bfo
 *    makefile => 'MPS ' / 'TEXT'
 *
 *    Revision 1.31  2006/05/26 01:30:19  bfo
 *    '.pch', '.r', '.x' are CWIE TEXT files now
 *
 *    Revision 1.30  2006/05/16 13:09:57  bfo
 *    Linux full path name adaption / Named pipe adaption
 *
 *    Revision 1.29  2006/02/19 16:17:53  bfo
 *    f->del (instead of f->delete) / type casting
 *
 *    Revision 1.28  2005/07/15 22:13:56  bfo
 *    "fflush" no longer active
 *
 *    Revision 1.27  2005/07/15 10:23:14  bfo
 *    Empty file problem fixed
 *
 *    Revision 1.26  2005/07/10 19:24:07  bfo
 *    Ignore truncate error
 *
 *    Revision 1.25  2005/07/06 21:01:13  bfo
 *    defined UNIX / fSetsz adaptions
 *
 *    Revision 1.24  2005/07/02 14:15:50  bfo
 *    Adapted for Mach-O / Special handling for /Volumes/XXX
 *
 *    Revision 1.23  2005/06/30 11:37:43  bfo
 *    .tmp.X eliminated / Mach-O support
 *
 *    Revision 1.22  2005/05/13 17:23:26  bfo
 *    SetSize access improved for Windows
 *
 *    Revision 1.21  2005/01/22 16:22:30  bfo
 *    SS_Opt: fdPos info for foreign file system (problem with "diff" fixed).
 *
 *    Revision 1.20  2004/12/03 23:56:04  bfo
 *    MacOSX MACH adaptions
 *
 *    Revision 1.19  2004/11/27 12:05:34  bfo
 *    _XXX_ introduced
 *
 *    Revision 1.18  2004/11/20 11:44:07  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.17  2004/10/22 22:51:11  bfo
 *    Most of the "pragma unused" eliminated
 *
 *    Revision 1.16  2004/09/15 19:55:46  bfo
 *    C++ extensions cp/cpp for Mac rsc included
 *
 *    Revision 1.15  2003/05/05 17:53:12  bfo
 *    Activate inactive "netatalk" nodes automatically in Mac file system
 *
 *    Revision 1.14  2003/04/25 19:35:47  bfo
 *    "Netatalk" support
 *
 *    Revision 1.13  2002/10/27 23:14:59  bfo
 *    get_mem/release_mem no longer with param <mac_asHandle>
 *
 *    Revision 1.12  2002/10/16 20:06:29  bfo
 *    Make it compatible for gcc 3.2
 *
 *    Revision 1.11  2002/09/22 20:58:47  bfo
 *    Some bugs (at least 5 !!) at getFD fixed: The access to the top directory (Windows32) was seriously wrong. Now it seems to work fine with "mdt".
 *
 *
 */


#include "os9exec_incl.h"

#ifdef win_unix
  #include <utime.h>
#endif


/* Macintosh, PC and Linux File System access */
/* ========================================== */

/* Plain file I/O operations */
/* ------------------------- */

/* --- local procedure definitions for object definition ------------------- */
void   init_File ( fmgr_typ* f );
os9err pFopen    ( ushort pid, syspath_typ*, ushort *modeP,  const char* pathname );
os9err pFclose   ( ushort pid, syspath_typ* );
os9err pFread    ( ushort pid, syspath_typ*, uint32_t *n,      char* buffer );
os9err pFreadln  ( ushort pid, syspath_typ*, uint32_t *n,      char* buffer );
os9err pFwrite   ( ushort pid, syspath_typ*, uint32_t *n,      char* buffer );
os9err pFwriteln ( ushort pid, syspath_typ*, uint32_t *n,      char* buffer );
os9err pFseek    ( ushort pid, syspath_typ*, uint32_t  *posP );
os9err pFdelete  ( ushort pid, syspath_typ*, ushort   *modeP,  char* pathname );

os9err pFsize    ( ushort pid, syspath_typ*, uint32_t *sizeP );
os9err pFopt     ( ushort pid, syspath_typ*,                   byte* buffer );
os9err pHvolnam  ( ushort pid, syspath_typ*,                   char* volname );
os9err pFpos     ( ushort pid, syspath_typ*, uint32_t  *posP );
os9err pFeof     ( ushort pid, syspath_typ* );
os9err pFready   ( ushort pid, syspath_typ*, uint32_t *n );
os9err pHgetFD   ( ushort pid, syspath_typ*, uint32_t *maxbytP, byte* buffer );
os9err pHgetFDInf( ushort pid, syspath_typ*, uint32_t *maxbytP,
                                             uint32_t *fdinf,   byte* buffer );

os9err pFsetsz   ( ushort pid, syspath_typ*, uint32_t *sizeP );
os9err pHsetFD   ( ushort pid, syspath_typ*,                   byte* buffer );



void   init_Dir  ( fmgr_typ* f );
os9err pDopen    ( ushort pid, syspath_typ*, ushort  *modeP,  const char* pathname );
os9err pDclose   ( ushort pid, syspath_typ* );
os9err pDread    ( ushort pid, syspath_typ*, uint32_t *n,      char* buffer );
os9err pDseek    ( ushort pid, syspath_typ*, uint32_t  *posP );
os9err pDchd     ( ushort pid, syspath_typ*, ushort  *modeP,  char* pathname );
os9err pDmakdir  ( ushort pid, syspath_typ*, ushort  *modeP,  char* pathname );

os9err pDsize    ( ushort pid, syspath_typ*, uint32_t *sizeP );
os9err pDpos     ( ushort pid, syspath_typ*, uint32_t  *posP );
os9err pDeof     ( ushort pid, syspath_typ* );
os9err pDsetatt  ( ushort pid, syspath_typ*, ulong   *attr );
/* ------------------------------------------------------------------------- */

void init_File( fmgr_typ* f )
/* install all procedures of the transparent file manager for files */
{
    gs_typ* gs= &f->gs;
    ss_typ* ss= &f->ss;
    
    /* main procedures */
    f->open       = (pathopfunc_typ)pFopen;
    f->close      = (pathopfunc_typ)pFclose;
    f->read       = (pathopfunc_typ)pFread;
    f->readln     = (pathopfunc_typ)pFreadln;
    f->write      = (pathopfunc_typ)pFwrite;
    f->writeln    = (pathopfunc_typ)pFwriteln;
    f->seek       = (pathopfunc_typ)pFseek;
    f->del        = (pathopfunc_typ)pFdelete;
    f->makdir     = (pathopfunc_typ)pDmakdir; /* access to directory will be done via fFile */
    
    /* getstat */
    gs->_SS_Size  = (pathopfunc_typ)pFsize;
    gs->_SS_Opt   = (pathopfunc_typ)pFopt;
    gs->_SS_DevNm = (pathopfunc_typ)pHvolnam;
    gs->_SS_Pos   = (pathopfunc_typ)pFpos;
    gs->_SS_EOF   = (pathopfunc_typ)pFeof;
    gs->_SS_Ready = (pathopfunc_typ)pFready;
    gs->_SS_FD    = (pathopfunc_typ)pHgetFD;
    gs->_SS_FDInf = (pathopfunc_typ)pHgetFDInf;

    /* setstat */
    ss->_SS_Size  = (pathopfunc_typ)pFsetsz;
    ss->_SS_Opt   = (pathopfunc_typ)pNop;    /* ignored */
    ss->_SS_Attr  = (pathopfunc_typ)pNop;
    ss->_SS_FD    = (pathopfunc_typ)pHsetFD;
    ss->_SS_WTrk  = (pathopfunc_typ)pUnimp; /* not used */
} /* init_File */


void init_Dir( fmgr_typ* f )
/* install all procedures of the transparent file manager for directories */
{
    gs_typ* gs= &f->gs;
    ss_typ* ss= &f->ss;
    
    /* main procedures */
    f->open      = (pathopfunc_typ)pDopen;
    f->close     = (pathopfunc_typ)pDclose;
    f->read      = (pathopfunc_typ)pDread;
    f->readln    = (pathopfunc_typ)pDread;   /* the same as read */
    f->write     = (pathopfunc_typ)pBadMode; /* not allowed */
    f->writeln   = (pathopfunc_typ)pBadMode; /* not allowed */
    f->seek      = (pathopfunc_typ)pDseek;
    f->chd       = (pathopfunc_typ)pDchd;
    f->makdir    = (pathopfunc_typ)pDmakdir;
    
    /* getstat */
    gs->_SS_Size = (pathopfunc_typ)pDsize;
    gs->_SS_Opt  = (pathopfunc_typ)pRBFopt;
    gs->_SS_DevNm= (pathopfunc_typ)pHvolnam;
    gs->_SS_Pos  = (pathopfunc_typ)pDpos;
    gs->_SS_EOF  = (pathopfunc_typ)pDeof;
    gs->_SS_Ready= (pathopfunc_typ)pUnimp;      /* not used */
    gs->_SS_FD   = (pathopfunc_typ)pHgetFD;
    gs->_SS_FDInf= (pathopfunc_typ)pHgetFDInf;

    /* setstat */
    ss->_SS_Size = (pathopfunc_typ)pBadMode; /* not allowed */
    ss->_SS_Opt  = (pathopfunc_typ)pNop;         /* ignored */
    ss->_SS_Attr = (pathopfunc_typ)pDsetatt;
    ss->_SS_FD   = (pathopfunc_typ)pHsetFD;
    ss->_SS_Lock = (pathopfunc_typ)pNop;         /* ignored */
    ss->_SS_WTrk = (pathopfunc_typ)pUnimp;      /* not used */
} /* init_Dir */

/* -------------------------------------------------------- */



#ifndef USE_UAEMU
  static void assert( Boolean b )
  {   if (!b) printf( "Assert Error\n" ); 
  }
#endif



/* input from file */
os9err pFread( _pid_, syspath_typ* spP, uint32_t *n, char* buffer )
{
    long cnt, k;
    file_typ* f= &spP->u.disk.u.file;
    
    #ifdef MACFILES
      long  effpos;
      OSErr oserr;
    #endif
    
    assert( buffer!=NULL );

    if     (spP->rawMode) {
        if (spP->rawPos    > STD_SECTSIZE) return E_EOF; /* finished */
        if (spP->rawPos+*n > STD_SECTSIZE) *n= STD_SECTSIZE-spP->rawPos;
        
        memcpy( buffer, spP->rw_sct+spP->rawPos, *n );
        spP->rawPos+= *n;
        return 0;
    } // if
  
    cnt= *n; if (cnt==0) return 0; /* null read returns w/o error */
    
    #ifdef MACFILES
      oserr= FSRead( f->refnum, &cnt, (void*)buffer );
      f->readFlag= true;

      debugprintf(dbgFiles,dbgDeep,("# pFread: FSRead requested=%ld, returned=%ld, oserr=%d\n",
                                       *n, cnt, oserr));
      if (oserr && oserr!=eofErr) return host2os9err(oserr,E_READ);
      if (cnt==0) return os9error(E_EOF); /* if nothing read, this is EOF */

      /* show read for debug */
      if (debugcheck(dbgFiles,dbgDeep)) {
          effpos=0x12345678; /* um Himmels willen Lukas, was soll denn das ? */
          GetFPos( f->refnum, &effpos);
          uphe_printf("pFread: read $%lX bytes, ending pos now=$%lX: ",cnt,effpos);
      }
    #else     
      cnt= fread( (void*)buffer, 1,cnt, spP->stream );
      f->readFlag= true;
      
      if (cnt==0 && feof(spP->stream)) return os9error(E_EOF);
    #endif
  
    /* show read for debug */
    if (debugcheck(dbgFiles,dbgDetail)) {
        uphe_printf("%08X: ", cnt );
        for  (k=0; k<16 && k<cnt; k=k+2) {
                         upe_printf( "%02X" , (byte)buffer[k  ] );
            if (k+1<cnt) upe_printf( "%02X ", (byte)buffer[k+1] );
        }
        upe_printf("...\n");
    }
  
    /* ok, return # of chars read to caller */
    *n= cnt; return 0;
} /* pFread */



/* input line from file */
os9err pFreadln( _pid_, syspath_typ* spP, uint32_t *n, char* buffer )
{
    long      cnt;
    file_typ* f= &spP->u.disk.u.file;

    #ifdef MACFILES
      long      bytes,chunkbytes, cht, k;
      OSErr     oserr;
      #define   READCHUNKSZ 200
    #else
      char* p;
      char  c;
    #endif
     
    assert( buffer!=NULL );

    #ifdef MACFILES
      /* input not more than one line from a FILE */
      bytes= *n; if (bytes==0) return 0; /* null read returns w/o error */

      /* read in chunks */
      cnt= 0;
      cht= 0; /* new variable */
      while  (bytes>0) {
          if (bytes>READCHUNKSZ)     chunkbytes= READCHUNKSZ; 
          else                       chunkbytes= bytes;
          oserr= FSRead( f->refnum, &chunkbytes, (void*)buffer);
          f->readFlag= true;

          if (debugcheck(dbgFiles,dbgDeep)) {
              int k;
              uphe_printf("pFreadLn: chunkread: FSRead returned=%ld, oserr=%d: '",chunkbytes,oserr);
              for (k=0; k<chunkbytes; k++) putc(buffer[k],stderr);
              debugprintf(dbgFiles,dbgDeep,("'\n"));            
          } // if
          if (oserr!=0 && oserr!=eofErr) return host2os9err(oserr,E_READ);
        
          /* now check what we've got */
          if (chunkbytes==0) break; /* nothing more to read */
        
          /* search for a CR */
          cht+=chunkbytes;              /* total bytes (bfo) */
          for (k=0; ++k<=chunkbytes;) { /* don't forget to read the whole buffer <= (bfo) */
              if (*buffer++==CR) {
                  cnt+= k; /* update total number of bytes actually read */
                  /* now pull back file pointer (if required) */
                  if (chunkbytes>k) {
                      /* seek back */
                          oserr= SetFPos( f->refnum, fsFromMark, k-chunkbytes );
                      if (oserr) return host2os9err(oserr,E_SEEK);
                  }
                  goto readlnok; /* exit both loops, cnt is updated */
              } // if
          } // for
        
          bytes-=k;
          cnt  +=k;
          if (oserr==eofErr) goto readlnok; /* don't try to read more, because this is EOF, even if there's no CR */
      } // while
    
      if (cnt==0) return os9error(E_EOF);
      /* on exit from while: cnt=# of chars actually read */
      
    readlnok:
      /* cnt is always >0 here, so no EOF check is needed */
      if (cnt>cht) cnt=cht;  /* this must be adapted also (bfo) */
 
    #else
      /* input not more than one line from a FILE */
      #ifdef MACOS9
        if     (*n <2) {
            if (*n!=0) {
                /* single char only */
                cnt= fread( (void*)buffer, 1,1, f->stream );
            }
            else return 0; /* nothing to read at all */
        }
        /* more than 1 char */
        if (fgets((void *)buffer, *n-1, f->stream)==NULL) { /* read all but one char */
            if (feof( f->stream)) return os9error(E_EOF);
            return c2os9err(errno,E_READ);
        } // if
        
            cnt=strlen(buffer); /* find size of string read */
        if (cnt==*n-1) {
            /* all read */
            if (buffer[cnt-1]!=CR) {
                /* not stopped by a CR, read one more, if possible */
                cnt+= fread( (void*)&buffer[cnt], 1,1, f->stream );
            }
        }
        if ((cnt==0) && feof( f->stream )) return os9error(E_EOF);
      
      #else
        /* we cannot rely on fgets() to stop on 0x0D on non-Mac */
        /* single char for now */
        p= buffer;

               cnt=0;
        while( cnt<*n ) {
              c= fgetc( spP->stream );
          if (c==EOF) {
            *n= cnt;
            return cnt==0 ? os9error(E_EOF) : 0; /* return EOF only if on first char */
          } // if
            
          *p++= c; /* save in the buffer */        
          cnt++;
          if (c==CR) break; /* abort on CR */
        } // while
    
        f->readFlag= true;
      #endif
    #endif
    
    debugprintf( dbgFiles,dbgDetail,("# pFreadLn: requested=%ld, returned=%ld\n", *n, cnt ));
    
    *n= cnt;
    return 0;
} /* pFreadln */



/* output to file */
os9err pFwrite( _pid_, syspath_typ* spP, uint32_t *n, char* buffer )
{
  file_typ*  f= &spP->u.disk.u.file;
  
  #ifdef MACFILES
    OSErr      oserr;
    long  cnt, effpos, k;

    assert( buffer!=NULL );
    
    /* output to a file */
    if ((cnt=*n)==0) return 0; /* null read returns w/o error */
    
    /* show what we write for debug */
    if (debugcheck(dbgFiles,dbgDeep)) {
                           effpos= 0x12345678;
      GetFPos( f->refnum, &effpos );
      uphe_printf("pFwrite: writing $%lX bytes at pos=$%lX: ",cnt,effpos);
      for (k=0; k<8 && k<cnt; k++) {
        upe_printf("$%02X ",(byte) buffer[k]);
      } // for
        
      upe_printf("...\n");
    } // if

    /* actual write */
    if (f->readFlag) {
      GetFPos( f->refnum,             &effpos );
      SetFPos( f->refnum, fsFromStart, effpos );
      f->readFlag= false;
    } // if
    
        oserr= FSWrite( f->refnum, &cnt, (void*)buffer );
    if (oserr) return host2os9err(oserr,E_WRITE);
    
    debugprintf(dbgFiles,dbgDeep,("# pFwrite: requested=%ld, written=%ld, oserr=%d\n",*n,cnt,oserr));   

  #else
    long   cnt;
    fpos_t tmp_pos;

    assert( buffer!=NULL );
          
    if (f->readFlag) {
      fgetpos( spP->stream, &tmp_pos );   /* save current position */
      fsetpos( spP->stream, &tmp_pos );   /* restore position */
      f->readFlag= false;
    } // if
    
    /* output to a FILE */
    cnt= fwrite( (void*)buffer, 1,*n, spP->stream );
    fflush( spP->stream ); /* don't forget this */

    debugprintf(dbgFiles,dbgDeep,("# pFwrite: requested=%ld, written=%ld, ferror=%d, errno=%d\n",*n,cnt,ferror(spP->stream),errno));
    if (cnt<0) return c2os9err(errno,E_WRITE); /* default: general write error */
  #endif
  
  *n= cnt;
  return 0;
} /* pFwrite */



/* output to file */
os9err pFwriteln( _pid_, syspath_typ* spP, uint32_t *n, char* buffer )
{
  file_typ* f= &spP->u.disk.u.file;
  
  #ifdef MACFILES
    long      cnt, effpos, k;
    OSErr     oserr;

    assert( buffer!=NULL );

    /* output line to a file */
    cnt= *n; if (cnt==0) return 0; /* null read returns w/o error */

    /* limit number of bytes to one line, max */
    for (cnt=0; cnt<*n; cnt++) if (buffer[cnt]==CR) { ++cnt; break; }

    /* show what we write for debug */
    if (debugcheck(dbgFiles,dbgDeep)) {
                           effpos=0x12345678;
      GetFPos( f->refnum, &effpos );
      uphe_printf("pFwriteln: writing $%lX bytes at pos=$%lX: ",cnt,effpos);
      for (k=0; k<8 && k<cnt; k++) {
        upe_printf("$%02X ",(byte) buffer[k]);
      } // for
      
      upe_printf("...\n");
    } // if

    /* actual write */
    if (f->readFlag) {
      GetFPos( f->refnum,             &effpos );
      SetFPos( f->refnum, fsFromStart, effpos );
      f->readFlag= false;
    } // if
    
    /* actual write */
        oserr= FSWrite( f->refnum, &cnt, (void*)buffer );
    if (oserr) return host2os9err(oserr,E_WRITE);
    
    debugprintf(dbgFiles,dbgDeep,("# pFwrite: requested=%ld, written=%ld, oserr=%d\n",*n,cnt,oserr));   

  #else
    long   cnt, ii;
    fpos_t tmp_pos;

    assert( buffer!=NULL );

    if (f->readFlag) {
      fgetpos( spP->stream, &tmp_pos );   /* save current position */
      fsetpos( spP->stream, &tmp_pos );   /* restore position */
      f->readFlag= false;
    } // if
    
    /* output line to a FILE */
    for (ii=0; ii<*n; ii++) if (buffer[ii]==CR) { ++ii; break; }
    cnt=fwrite((void *)buffer, 1,ii, spP->stream);
    fflush( spP->stream ); /* don't forget this */
    
    if (cnt<0) return c2os9err(errno,E_WRITE); /* default: general write error */
  #endif
  
  *n= cnt; 
  return 0;
} /* pFwriteln */



#ifdef MACOS9
  // Get deP->fdSect from <volid>,<objid>,<dirid>
  static void assign_fdsect( os9direntry_typ *deP, short volid, long objid, long dirid )
  {
    char            hashV[ 80 ];
    char            fName[ OS9NAMELEN ];
    Boolean         cond;
    dirtable_entry       m; // exists "physically" here
    dirtable_entry* mP= &m; // the reference to it
    
    #ifdef LINKED_HASH
      cond= true;
    #else
      cond= volid>=       0 || 
            volid< VOLIDMIN ||
            objid>=  IDSIGN || 
            objid<  -IDSIGN;
    #endif        
    
    if (cond) {      
      strcpy( fName, deP->name );
              fName[ strlen(fName)-1 ] &= 0x7f; // back to "normal"
      
      mP->fName= fName;
      mP->dirid= dirid;
       
      sprintf( hashV, "%d %d", -volid, objid ); 
      FD_ID  ( hashV, NULL,           &objid,  &mP );
      debugprintf( dbgAnomaly,dbgDetail,( "get_dir_entry: ID=%ld is out of range %ld..%ld: '%s'\n",
                                           objid, -IDSIGN,IDSIGN-1, deP->name ));
      debugprintf( dbgAnomaly,dbgDetail,( "get_dir_entry: hashV='%s'\n", hashV ));
                      
      deP->fdsect= objid; /* assign the new value */
    }
    else {
      // combine them again
      deP->fdsect = (volid & VOLIDMASK) << IDSHIFT;
      deP->fdsect|=  objid &    IDMASK;
    } // if
  } /* assign_fdsect */
#endif



os9err pFopt( ushort pid, syspath_typ* spP, byte *buffer )
{
  os9err err= pRBFopt( pid,spP, buffer );
  uint32_t fdID;

  #ifdef MACOS9
    FSSpec*         spc= &spP->u.disk.spec;
    CInfoPBRec      cipb;
    os9direntry_typ dEnt;
    
    short volid;
    long  objid;
      
    err= getCipb( &cipb, spc );  
    
    volid= spc->vRefNum;           // get volume ID
    objid= cipb.hFileInfo.ioDirID; // get object ID
 
    strcpy        ( dEnt.name, spP->name );
    assign_fdsect( &dEnt, volid, objid, 0 );
    fdID=           dEnt.fdsect;
    
    /*
          volid= spc->vRefNum; // get volume ID
    fd=  (volid & VOLIDMASK) << IDSHIFT;
          objid= cipb.hFileInfo.ioDirID;
    fd |= objid & IDMASK;
    */
    
  #elif defined win_unix
    dirtable_entry* mP= NULL;
    dirent_typ      dEnt;
    strcpy        ( dEnt.d_name, spP->name );
    FD_ID                      ( spP->fullName, &dEnt, &fdID, &mP );
  //upe_printf( "FD_ID '%s' '%s' fdPos=%08X\n", spP->fullName, dEnt.d_name, fdpos );                  
  #endif
    
  SET_OS9L(buffer, PD_FD, fdID << BpB); /* LSN of file */
  return err;
} /* pFopt */



/* check ready */
os9err pFready( _pid_, _spP_, uint32_t *n )
{   *n= 1; return 0;
} /* pFready */


/* get device name from HFS object */
os9err pHvolnam( _pid_, syspath_typ* spP, char* volname )
{
    #ifdef MACOS9
      long  free= 0;
      short volid= spP->u.disk.spec.vRefNum;
      
      #ifdef USE_CARBON
        strcpy( volname,"" );
      #else
        OSErr oserr= GetVInfo(0, volname, &volid, &free);
        if   (oserr) return host2os9err(oserr,E_BPNUM);
        p2cstr( volname );
      #endif
      
      debugprintf(dbgFiles,dbgDetail,("# pFvolnam: name='%s', free bytes=%ld\n",volname,free));

    #elif defined windows32
      volname[ 0 ]= toupper( spP->fullName[ 0 ] ); /* first char only */       
      volname[ 1 ]= NUL;     
    
    #elif defined UNIX
      int  ii; // get the current top path as name
      for (ii= 0; ii<strlen( spP->fullName ); ii++) {
        volname[ ii ]= spP->fullName[ ii+1 ];
        if ( ii>0 && volname[ ii ]=='/' ) { volname[ ii ]= NUL; break; }
      } // for
      
    //strcpy( volname,spP->fullName ); /* none for Linux, top directory structure is different */
      
//  #elif defined MACOSX
      /* MacOSX has a very special structure */
//    strcpy( volname, "Volumes" );
      
      /*
      #define VVP "Volumes"
      #define VV "/Volumes/"
      
      char* w= &spP->fullName;
      char* v= w;

      if (ustrncmp( w,VV, strlen(VV) )==0) {
        strcpy( volname, VVP );
        
      //if  (*v==PATHDELIM) v++; // cut slash at the beginning
      //strcpy( volname, v );
      //
      //w= volname + strlen(VV)-1;
      //v= strstr( w, PATHDELIM_STR );
      //if (v!=NULL) *v= NUL;   // Keep "/Volumes/XXX"
      } // if 
      else
        strcpy( volname, "" );
      */
        
     // v= &volname[ strlen(volname)-1 ];
     // if (*v==PATHDELIM) *v= NUL; // cut slash at the end
      
      /*  
      char* v;
      strcpy( volname, "" );
      v=     &volname[ strlen(volname)-1 ];
      if (*v==PATHDELIM) *v= NUL; // cut slash at the end
      */
      
    #else
      return E_UNKSVC;
    #endif
    
    return 0;
} /* pHvolnam*/



#ifdef win_unix
  static void Set_FileDate( syspath_typ* spP, time_t t )
  /* Set (Windows/Linux) file date */
  {
    #ifdef windows32
      Boolean    isFolder= false;
      int        hh;
      HANDLE     hFile;
      LPWIN32_FIND_DATA data;
      FILETIME   cr, lastA, lastW, lastL;
      SYSTEMTIME sy;
      struct tm  tim;

    //printf( "Set_FileDate '%s'\n", spP->fullName );
      /* open as file or as current directory */
          hh= _lopen( spP->fullName, OF_READWRITE );
      if (hh!=-1) hFile= (HANDLE) hh;
      else {
        hFile= NULL;
        if (PathFound(spP->fullName)) {
          return; // avoid date/time adaption of directories
          
          isFolder= true;
              hFile=       FindFirstFile( spP->fullName, &data );
          if (hFile!=NULL) FindNextFile ( hFile,         &data );
        }
      }
    //printf( "Set_FileDate %d %08X'\n", hh, hFile );
      if (hFile==NULL) return;
        
      GetFileTime( hFile, &cr, &lastA, &lastW );
      FileTimeToLocalFileTime( &lastW, &lastL );
      FileTimeToSystemTime   ( &lastL, &sy );
        
      if (t==0) GetTim  ( &tim );
      else      TConv( t, &tim );
        
      sy.wYear  = tim.tm_year+1900;
      sy.wMonth = tim.tm_mon+1;
      sy.wDay   = tim.tm_mday;
      sy.wHour  = tim.tm_hour;
      sy.wMinute= tim.tm_min;
      sy.wSecond= tim.tm_sec;
        
      SystemTimeToFileTime   ( &sy,    &lastL );
      LocalFileTimeToFileTime( &lastL, &lastW );
      SetFileTime( hFile, &cr, &lastA, &lastW );
    //printf( "Set_FileDate DONE\n" );

      if (isFolder) FindClose( hFile );
      else _lclose( hh );
    //printf( "Set_FileDate raus\n" );

    #elif defined UNIX
      struct utimbuf buf;
          
      buf.actime = t;
      buf.modtime= t;
      utime( spP->fullName, &buf );
    #endif
  } /* Set_FileDate */
#endif


#ifdef MACFILES
  static os9err touchfile( ushort pid, syspath_typ* spP )
  {
    struct tm tim;       // Important Note: internal use of <tm> as done in OS-9
    byte      fdbeg[16]; // buffer for preparing FD
    int       k;
    
    for (k=0; k<16; k++) fdbeg[ k]= 0; // initialize it
    
    GetTim   ( &tim );
    fdbeg[ 3 ]= tim.tm_year;
    fdbeg[ 4 ]= tim.tm_mon+1; // somewhat different month notation
    fdbeg[ 5 ]= tim.tm_mday;
    fdbeg[ 6 ]= tim.tm_hour;
    fdbeg[ 7 ]= tim.tm_min;
    
    return pHsetFD( pid,spP, fdbeg );
  } /* touchfile */


  typedef struct {
      char*   p;
      OSType* creator;
      OSType* type;
  } VType;
  

  static Boolean TCSuff( VType* v, char* suffix, OSType creator, OSType type )
  {
	  if (ustrcmp( suffix,v->p )==0 ||
	  	  ustrcmp( suffix,""   )==0) {
	  	  *v->creator= creator;
	      *v->type   = type; return true;
	  }
	            
  	  return false;
  } /* TCSuff */


  static Boolean TCCWIE( VType* v, char* suffix )
  {   return TCSuff( v,suffix, 'CWIE','TEXT' ); /* CodeWarrior text */
  } /* TCCWIE */
  
  
  static Boolean TCRBFi( VType* v, char* suffix )
  {   return TCSuff( v,suffix, 'os9a','RBFi' ); /* CodeWarrior text */
  } /* TCRBFi */


  static void Get_Creator_And_Type( FSSpec* spc, OSType *creator, OSType *type )
  {

	  VType   v;
 	  char*   q;
	  char    fName[32];
      memcpy( fName, spc->name,32 );
	  p2cstr( fName );

	  v.creator= creator; /* basic assignment */
	  v.type   = type;
	  
	  /* get the suffix */
	  v.p= &fName;
	  while (true) {
	  	  q  = strstr( v.p,"." ); if (q==NULL) break;
	  	  v.p= q+1;
	  } /* loop */

	  if (v.p==&fName) { /* no suffix */
	      if (TCSuff( &v, "Read_Me",  'MPS ','TEXT' )) return;
	      if (TCSuff( &v, "makefile", 'MPS ','TEXT' )) return;
	  }
	  else {
	  	      v.p--; /* making e.g. ".c" */
	  	  if (v.p==&fName) { /* starting with "." */
	          TCSuff( &v, "",         'os9a','PROG' ); return; // ".*"
	      } // if
	                           
	      if (TCCWIE( &v, ".h"   ))                    return; // include files
	      if (TCCWIE( &v, ".c"   ) ||
	          TCCWIE( &v, ".cp"  ) ||
	          TCCWIE( &v, ".cpp" ) ||
	          TCCWIE( &v, ".r"   ) ||
	          TCCWIE( &v, ".pch" ))                    return; // C/C++ source
	      if (TCCWIE( &v, ".p"   ) ||
	          TCCWIE( &v, ".m"   ) ||
	          TCCWIE( &v, ".x"   ))                    return; // Pascal source
	      if (TCCWIE( &v, ".f"   ))                    return; // Pascal include files
	      if (TCCWIE( &v, ".a"   ))                    return; // Assembler
	      if (TCCWIE( &v, ".d"   ))                    return; // Assembler include files
	      if (TCSuff( &v, ".tm",      'CWIE','MPLF' )) return;

								  /* creator type */
	      if (TCSuff( &v, ".exe",     'CWIE','DEXE' )) return; // executables

	      if (TCSuff( &v, ".pic",     '8BIM','PICT' )) return; // PICT
	      if (TCSuff( &v, ".psd",     '8BIM','8BPS' )) return; // Photoshop
	      if (TCSuff( &v, ".jpg",     '8BIM','JPEG' )) return; // JPEG
	      if (TCSuff( &v, ".tif",     '8BIM','TIFF' )) return; // TIFF
	      if (TCSuff( &v, ".gif",     '8BIM','GIFf' )) return; // GIF
	      if (TCSuff( &v, ".pdf",     'CARO','PDF ' )) return; // PDF
	      if (TCSuff( &v, ".rm",      'PNst','PNRM' )) return; // RealMovie

	      if (TCSuff( &v, ".lzh",     'LARC','LHA ' )) return; // lzh
	      if (TCSuff( &v, ".sit",     'SIT!','SITD' )) return; // StuffIt
	      if (TCSuff( &v, ".zip",     'ZIP ','ZIP ' )) return; // zip

	      if (TCSuff( &v, ".doc",     'MSWD','W8BN' )) return; // MS Word
	      if (TCSuff( &v, ".xls",     'XCEL','XLS8' )) return; // MS Excel
	      if (TCSuff( &v, ".ppt",     'PPT3','SLD8' )) return; // MS Powerpoint

	      if (TCSuff( &v, ".Me",      'MPS ','TEXT' )) return; // text file
	      if (TCSuff( &v, ".txt",     'ttxt','TEXT' ) ||
	          TCSuff( &v, ".log",     'ttxt','TEXT' )) return; // text file
      }

      if   ( RBF_Rsc( spc ) && TCRBFi( &v, "" ) )      return; // RBF images
   			  TCSuff( &v, "",         'os9a','PROG' );         // default
  } /* Get_Creator_And_Type */
#endif 


os9err pFopen( ushort pid, syspath_typ* spP, ushort *modeP, const char* pathname )
{
/* open/create file */
    os9err    err;
    char*     pastpath;
    char      hostpath[OS9PATHLEN];
    char      ploc    [OS9PATHLEN];
    Boolean   isW   = IsWrite(*modeP);
    Boolean   exedir= IsExec (*modeP);
    Boolean   cre   = IsCrea (*modeP);
    file_typ* f= &spP->u.disk.u.file;
    char*     p;
    char*     pp;
    char*     vn;
    int       len;
    
    #ifdef MACFILES
      OSErr      oserr;
      CInfoPBRec cipb;
      defdir_typ defdir;
      FSSpec*    spc= &spP->u.disk.spec;
      OSType     creator, type;

    #else
      FILE* stream;
      
      #ifdef win_unix
        char adapted[OS9PATHLEN];
      #endif
    #endif

          strcpy( ploc,pathname );
    if          (*ploc==NUL) return E_BPNAM;
    while (IsRaw( ploc )) {
        spP->rawMode= true;
        len= strlen(ploc); 
        ploc[len-1]= NUL;
    } /* while */
        
    
    pastpath= ploc;
    err     = parsepath( pid, &pastpath,hostpath, exedir ); if (err) return err;
    pp      = hostpath;

    if (spP->rawMode) {        /* rawmode allows only reading of 1st sector */
        spP->rw_sct = get_mem( STD_SECTSIZE );       /* for some info procs */
        spP->rawPos = 0;
                     vn= (char*)&spP->rw_sct[31];
        VolInfo( pp, vn );    /* this is the correct position */      
        
        #ifdef MACFILES
          if (*vn==NUL) {             defdir= exedir ?_exe:_data;   
              err= getFSSpec( pid,pp, defdir, spc );
              pHvolnam      ( pid,spP, vn );
          }
        #endif

        return 0;
    } /* if IsRaw */

    f->readFlag= false;

    #ifdef MACFILES
    //if (strlen(loc)>DIRNAMSZ) return os9error(E_BPNAM);
    
      /* Get FSSpec for requested file */
                               defdir= exedir ? _exe:_data;
      err= getFSSpec( pid, pp, defdir, spc );
      debugprintf( dbgFiles,dbgNorm, ("# pFopen: getFSSpec returned err=%d\n", err ) );
      if (err==E_FNA) return err; /* seems to be a file instead of a sub-directory */

      /* now see if create or open */
      if (cre) {
          /* --- create it */
          debugprintf( dbgFiles,dbgNorm, ( "# pFopen: trying to create '%s' err=%d\n", pp, err ) );
          /* check if file exists */
          if (err==0)      return os9error( E_CEF ); /* no error on MakeFSSpec -> object already exists */
          if (err!=E_PNNF) return err;
        
          /* file does not exist yet, create it first */
          Get_Creator_And_Type ( spc, &creator, &type );
               oserr= FSpCreate( spc,  creator,  type, smSystemScript);
          if  (oserr) {
           debugprintf( dbgFiles,dbgNorm, ( "# pFopen: trying to create '%s' oserr=%d\n", pp, oserr ) );
           if (oserr==dupFNErr) return E_PNNF; // special case handling
           return  host2os9err( oserr, E_ILLARG );
          } // if
          
          /* now open it */     
              oserr= FSpOpenDF( spc, fsRdWrPerm, &f->refnum );
          if (oserr) {
            debugprintf( dbgFiles,dbgNorm, ( "# pFopen: trying to create '%s' oserr=%d\n", pp, oserr ) );
            return host2os9err( oserr, E_PNNF );
          } // if
      }
      else {
          /* --- open it */
          debugprintf( dbgFiles,dbgNorm, ( "# pFopen: trying to open '%s', mode=$%04hX err=%d\n", pp, *modeP, err ) );
          if (!err) {
               /* object exists, but make sure it is not a directory */
            getCipb( &cipb, spc ); /* it's a directory, return error */ 
            if      ( cipb.hFileInfo.ioFlAttrib & ioDirMask ) return os9error( E_FNA );
          } // if
          
          if (err) return err;
                 
              oserr= FSpOpenDF( spc, isW ? fsRdWrPerm:fsRdPerm, &f->refnum );
          if (oserr) {
            debugprintf( dbgFiles,dbgNorm, ( "# pFopen: trying to open '%s' oserr=%d\n", pp, oserr ) );
            return host2os9err( oserr,E_PNNF );
          } // if
           
          /* and adapt file date if open for writing */
          if (isW) { err= touchfile( pid, spP ); if (err) return err; }
      } // if
      
      debugprintf( dbgFiles,dbgNorm, ("# pFopen: open/create successful, refnum=%d\n", f->refnum ) );

    #else
      debugprintf(dbgFiles,dbgNorm,("# pFopen: trying to %s '%s', mode=$%04hX\n",
                                     cre ? "create":"open", pp,*modeP));

      err= AdjustPath( pp,adapted, cre ); if (err) return err;
      pp=                 adapted; 

      if (cre) {
          /* --- create */
          /* check if file exists */
          if (FileFound( pp )) return os9error(E_CEF); /* create existing file not allowed */
          #ifdef windows32
          if (GetLastError()==ERROR_NOT_READY) return os9error(E_NOTRDY);
          #endif

              stream= fopen( pp,"wb+" ); /* create for update, use binary mode (bfo) ! */
          if (stream==NULL) return c2os9err(errno,E_FNA); /* default: file not accessible in this mode */  
      }
      else {
          /* --- open */
          #ifdef win_unix
            /* object exists, but make sure it is not a directory */
            #ifdef windows32
              SetLastError(ERROR_SUCCESS);
            #endif
            
            if (PathFound( pp )) return os9error(E_FNA);

            #ifdef windows32
              if (GetLastError()==ERROR_NOT_READY) return os9error(E_NOTRDY);
            #endif
          #endif
        
              stream= fopen( pp,"rb" ); /* try to open for read, use binary mode */
          if (stream==NULL) return os9error(E_PNNF); /* file not found */

          if (isW) { /* open (also) for write */
              fclose(stream);  /* close the read-only path again */
                  stream= fopen( pp,"rb+" ); /* open for update, use binary mode */
              if (stream==NULL) {
                  return c2os9err(errno,E_FNA); /* default: file no access in this mode */
              }
          }
      }
      
    //spP->rw_sct= get_mem( BUFSIZ );
    //err= setvbuf( stream, spP->rw_sct, _IOFBF, BUFSIZ ); // make buffered I/O
      spP->stream=  stream;                                // assign stream
    
      #ifdef win_unix
                spP->dDsc= NULL; /* this is not a directory */
        strcpy( spP->fullName, pp );
      #endif
    #endif
    
    p= (char*)ploc+strlen(ploc)-1; /* only the filename itself, no path name */
    while (p>=ploc && *p!=PSEP) p--;
    p++;
    
    strcpy( spP->name,p );
    
    f->moddate_changed= false;
    #ifdef windows32
      if (isW) {
          f->moddate        = 0;
          f->moddate_changed= true;
      }
    #endif
    
    return 0;
} /* pFopen */



/* close a file */
os9err pFclose( _pid_, syspath_typ* spP )
{
    #ifdef MACFILES
      OSErr      oserr;
      FSSpec*    spc= &spP->u.disk.spec;
      CInfoPBRec cipb;
    #endif 
      
    file_typ* f= &spP->u.disk.u.file;
      
    if (spP->rawMode) {
        release_mem( spP->rw_sct ); 
        spP->rw_sct= NULL;
        spP->rawMode= false;
        return 0;
    }
    
    #ifdef MACFILES
            oserr=FSClose( f->refnum );
      if   (oserr) return host2os9err(oserr,E_WRITE);
      
      if (f->moddate_changed) {
              oserr= getCipb( &cipb, spc );
          if (oserr) return host2os9err(oserr,E_WRITE);
      
          cipb.hFileInfo.ioFlMdDat= (ulong)f->moddate-OFFS_1904;
      
              oserr= setCipb( &cipb, spc ); /* and write it back */
          if (oserr) return host2os9err(oserr,E_WRITE);
      } /* if */

            oserr=FlushVol( NULL, spc->vRefNum );
      if   (oserr) return host2os9err(oserr,E_UNIT);
      
    #else
      if (fclose(spP->stream)<0) return c2os9err( errno, E_WRITE );    
      if (f->moddate_changed) Set_FileDate( spP, f->moddate );
      
    //release_mem( spP->rw_sct ); // don't use I/O buffer anymore
    //             spP->rw_sct= NULL;
    #endif
    
    return 0;
} /* pFclose */


os9err pFseek( _pid_, syspath_typ* spP, uint32_t *posP )
/* seek within a file */
{
    #ifdef MACFILES
      OSErr oserr;
      ulong effpos;
      file_typ* f= &spP->u.disk.u.file;
    #else
//    int   fildes;  /* was for commented-out ioctl(fildes, FIOSETEOF, ...) */
    #endif

    if (spP->rawMode) {
        if (*posP>STD_SECTSIZE) return E_SEEK;
        spP->rawPos= *posP;
        return 0;
    } /* if */

    #ifdef MACFILES
      /* try to set position */
      oserr= SetFPos( f->refnum, fsFromStart, (long) *posP );
      debugprintf(dbgFiles,dbgNorm,("# pFseek: tried to seek to $%08lX, oserr=%d\n",*posP,oserr));
      if (oserr==eofErr) {
          /* tried to seek beyond currend EOF -> try to extend file */
          oserr= SetEOF( f->refnum, (long) *posP );
          debugprintf(dbgFiles,dbgNorm,("# pFseek: Tried to extend file to size=$%08lX, oserr=%d\n",*posP,oserr));
          if (oserr==wrPermErr) {
              debugprintf(dbgFiles,dbgNorm,("# pFseek: File not writable, oserr=wrPermErr\n"));
              oserr=0; /* file not open for write, can't extend, simply ignore */
          }
          if (!oserr) {
              /* now as file is extended, seek to new LEOF */
              oserr= SetFPos( f->refnum, fsFromLEOF, 0 );
              debugprintf(dbgFiles,dbgNorm,("# pFseek: Extended file to size $%08lX (setting pos -> oserr=%d\n",*posP,oserr));
          } // if
      } // if
      
      /* show result of seek */
      if (debugcheck(dbgFiles,dbgNorm)) {
          effpos=0x12345678;
          GetFPos( f->refnum, &effpos );
          uphe_printf("pFseek: Actual pos (GetFPos)=$%lX\n",effpos);
          if (effpos!=*posP) {
              uphe_printf("pFseek: FATAL: seek did not reach desired position ($%lX)\n",*posP);
              debugwait();
          }
          effpos=0x12345678;
          GetEOF( f->refnum, &effpos );
          uphe_printf("pFseek: Actual EOF (GetEOF)=$%lX\n",effpos);
      }
      /* now seek should be done or failed definitely */
      return host2os9err(oserr,E_SEEK);

    #else
      if (fseek( spP->stream, (long)*posP, SEEK_SET )==0) return 0;

      debugprintf(dbgFiles,dbgDetail,("# pFseek: tried to seek to $%08lX, got errno=%d\n",*posP,errno));

      #if !defined(__MWERKS__) && !defined(linux)
      if (errno==ESPIPE || errno==ENXIO)
      #else
      if (errno!=0) /* %%% rude version, if seek fails, assume that we must enlarge the file */
      #endif
      {
          /* try extending file */
          debugprintf(dbgFiles,dbgDetail,("# pFseek: Trying to extend file to size=$%08lX\n",*posP));
          fflush(spP->stream); /* unbuffer everything */
//        fildes= fileno( spP->stream );  /* was for FIOSETEOF ioctl */

          #ifdef macintosh
            return c2os9err(errno,E_SEEK); /* %%% FIOSETEOF is not available in MSL on Mac: use MACFILES-Version normally !! */
            /* if (ioctl(fildes, FIOSETEOF, (long *) (*sizeP))<0) return c2os9err(errno,E_SEEK); */
          #else
            return c2os9err(errno,E_SEEK); /* %%% FIOSETEOF is not available in MSL !! */
          #endif

          if (fseek(spP->stream, (long int) *posP, SEEK_SET)!=0) return c2os9err(errno,E_SEEK);
      } 
      else return c2os9err(errno,E_SEEK);
    
      fflush( spP->stream );  
      return 0;
    #endif
} /* pFseek */



os9err pFdelete( ushort pid, _spP_, ushort *modeP, char* pathname )
{
    os9err  err;    
    OSErr   oserr= 0;
    char*   pastpath;
    char    pp[OS9PATHLEN];
    Boolean exedir= IsExec(*modeP);
    
    #ifdef MACFILES
      FSSpec     delSpec;
      CInfoPBRec cipb;
      Boolean    isDir;
      int        kk;
    #endif
    
    #ifdef windows32
      char cmd[OS9PATHLEN];
    #endif
        
    #ifdef win_unix
      char adapted[OS9PATHLEN];
    #endif
    

    pastpath= pathname;
    err     = parsepath( pid, &pastpath,pp, exedir ); if (err) return err;  
    pathname= pp;

    debugprintf( dbgFiles,dbgNorm,( "# I$Delete: pathname=%s, err=%d\n", pathname,err ));
    if (err) return err;

    #ifdef MACFILES
          err= getFSSpec( pid,pathname, exedir ?_exe:_data, &delSpec );
      if (err) return err;
                  
      getCipb( &cipb,&delSpec);
      isDir=  ( cipb.hFileInfo.ioFlAttrib & ioDirMask ); if (isDir) return E_FNA;

      /* unfortunately the Mac system needs some time to be ready   */
      /* for deleting an empty directory => make some timeout loops */
      for (kk=0; kk<MAXTRIES_DEL; kk++) {
             oserr= FSpDelete( &delSpec );
        if (!oserr) break;
        HandleEvent();
      } // for
    
    #elif defined win_unix
      err     = AdjustPath( pathname,adapted, false ); if (err) return err;
      pathname= adapted;
      
      #ifdef windows32
//      if (!DeleteFile( adapted )) oserr= GetLastError();
        sprintf( cmd, "del %s /A", pathname );
        err= call_hostcmd( cmd, pid, 0,NULL ); if (err) return err;

      #else
        oserr= remove( pathname );
      #endif  

    #else
      #error I_Delete not yet implemented
    #endif
    
    return host2os9err( oserr,E_SHARE );  
} /* pFdelete */



/* get file position */
os9err pFpos( _pid_, syspath_typ* spP, uint32_t *posP )
{   
  #ifdef MACFILES
    file_typ* f= &spP->u.disk.u.file;
    return host2os9err( GetFPos( f->refnum, (long *)posP),E_SEEK );
  #else
    *posP= (ulong) ftell( spP->stream );
  //fgetpos( spP->stream,  posP );   /* save current position */
    return 0;
  #endif
} /* pFpos */



/* get file size */
os9err pFsize( _pid_, syspath_typ* spP, uint32_t* sizeP )
{
  os9err err= 0;
    
  #if defined MACFILES
    file_typ* f= &spP->u.disk.u.file;

    OSErr oserr= GetEOF( f->refnum, (long*)sizeP );
    err= host2os9err( oserr,E_SEEK );

  #elif defined win_linux
    int    fd= fileno( spP->stream );
    struct stat info;

        err= fstat( fd,&info );
    if (err) return E_SEEK;
    *sizeP= info.st_size; // for MACOSX <st_size> is unfortunately 0
      
  #else
    fpos_t tmp_pos;
     
    fgetpos( spP->stream,  &tmp_pos );   /* save current position */
    fseek  ( spP->stream,0,SEEK_END );   /* go to EOF */
    *sizeP= (ulong)ftell( spP->stream ); /* get position now = file size */
  //fgetpos( spP->stream,  sizeP    );   /* get position now = file size */
    fsetpos( spP->stream,  &tmp_pos );   /* restore position */
  #endif
      
  return err;
} /* pFsize */



/* set file size */
os9err pFsetsz( ushort pid, syspath_typ* spP, uint32_t *sizeP )
{
    os9err   err= 0;
    char     b= 0;
    uint32_t n;
    uint32_t p;
    uint32_t tmp_pos= 0;
    
    #if defined win_unix || defined MACFILES
      long curSize;
    #endif

    #ifdef MACFILES
      OSErr     oserr;
      file_typ* f= &spP->u.disk.u.file;
      
      oserr= GetFPos( f->refnum, (long*)&tmp_pos );
      err= host2os9err( oserr,E_SEEK ); if (err) return err;
     
             GetEOF ( f->refnum, &curSize     );
      oserr= SetEOF ( f->refnum, (long)*sizeP );
      err= host2os9err( oserr,E_SEEK ); if (err) return err;
      
    #else
      #ifdef windows32
        int    fno;
        HANDLE hFile;
        DWORD  pos;
 
        fgetpos( spP->stream, &tmp_pos );  // save current position
        fflush ( spP->stream );
        
                                       fno= _fileno( spP->stream ); 
        hFile= (HANDLE)_get_osfhandle( fno );
        pos  = SetFilePointer( hFile, *sizeP,NULL, FILE_BEGIN );
        SetEndOfFile         ( hFile );
        curSize= *sizeP;
        
      #elif defined UNIX
        int  fd, i, j, cnt;
//      OSErr oserr= 0;  /* holdover from MACFILES version, not used in UNIX path */
        char tmpName[ OS9PATHLEN ];
        FILE* tmp__stream;
        #define      BUFFSIZE 1024
        byte buffer[ BUFFSIZE ];
        
        tmp_pos= (uint32_t)ftell( spP->stream );       /* make it compatible for gcc >= 3.2 */
        err= fseek    ( spP->stream,0,SEEK_END ); if (err) return err;         /* go to EOF */

        curSize= (ulong)ftell( spP->stream ); /* get position now = file size */
      //upo_printf( "curSize=%d\n", curSize );
      //upe_printf( "pos=%d size=%d newSize=%d\n", tmp_pos, curSize, *sizeP );
        
        if (*sizeP<curSize) {
          err= fseek ( spP->stream,0,SEEK_SET ); if (err) return err; // go to beginning
          fd = fileno( spP->stream );                                 // <fd> used for "ftruncate"
          
          if (fd!=0) err= ftruncate( fd, *sizeP  );                   // cut the file at <*sizeP>
          else {
            // problems with Mach ...
          //upe_printf( "problems_with_mach\n" );
            strcpy          ( tmpName, spP->fullName );
                   i= strlen( tmpName );
            while (i>=0  &&   tmpName[ i ]!='/') i--;
            tmpName[ i+1 ]= NUL;
            sprintf( tmpName, "%s.tmpfile.%d", tmpName, pid );
            
            err= fclose( spP->stream );
            err= remove               ( tmpName );
          //upe_printf( "remove '%s' err=%d\n", tmpName, err );
            err= rename( spP->fullName, tmpName );
          //upe_printf( "rename err=%d\n", err );
           
            spP->stream= fopen( spP->fullName, "wb+" );
            tmp__stream= fopen(       tmpName, "rb+" );
           
            err= E_FNA;
            if  (spP->stream && tmp__stream) {
              err= 0;
              
                     i= *sizeP;
              while (i>0) {
                j=   i;
                if  (j>BUFFSIZE) j= BUFFSIZE;
                cnt= fread ( (void*)buffer, 1,  j, tmp__stream );
                cnt= fwrite( (void*)buffer, 1,cnt, spP->stream );
                i -= cnt;
              } // while
              
              err= fclose( tmp__stream );
              err= remove( tmpName );
            } // if
          } // if
        }  // if
        
        /*  
        //fflush     ( spP->stream );                             // unbuffer everything
          fd = fileno( spP->stream );                       // <fd> used for "ftruncate"
          upe_printf( "fd=%d errno=%d fdGOOD=%d\n", fd, errno, spP->stream->_file );
 
          err= fclose( spP->stream );
          upe_printf( "close err=%d\n", err );
 
        //upe_printf( "name='%s' fd=%d\n", spP->fullName, fd );
          upe_printf( "name='%s' size=%d\n", spP->fullName, *sizeP );
          err= truncate( spP->fullName, *sizeP  );              // cut the file at <*sizeP>
        //err= SetEOF( spP->stream->handle );
          upe_printf( "err=%d errno=%d\n", err, errno );
          
              spP->stream= fopen( spP->fullName,"rb+" ); // open for update, use binary mode
          if (spP->stream) err= 0;
          else             err= E_FNA;
          upe_printf( "stream=%08X err=%d\n", spP->stream, err );
          
        //if (fd!=0) err= ftruncate( fd, *sizeP  );             // cut the file at <*sizeP>
          // problems with Mach ...
          
        //upe_printf( "TRUncate=%d fd=%d err=%d\n", *sizeP, fd, err );
        //fflush        ( spP->stream );
        //fseek         ( spP->stream, *sizeP,SEEK_SET );
        //fflush        ( spP->stream );
        //upe_printf( "NOW=%d\n", ftell( spP->stream ) ); 
        
        //             fclose( spP->stream );
        //spP->stream= fopen ( spP->fullName,"rb+" ); // create for update, use binary mode (bfo) !
        //fseek              ( spP->stream, *sizeP,SEEK_SET );
        }  // if
          
      //            curSize= *sizeP;
      //if (tmp_pos>curSize)            tmp_pos= curSize;
      //fsetpos         ( spP->stream, &tmp_pos );   // restore position
        */
      #endif
    #endif

    /* just read and write back one char at the end to get the correct size */
    do {           // do not change the content if smaller -> read it first
      #ifdef UNIX
        if (*sizeP==0) {
          fclose( spP->stream );
                          remove( spP->fullName );
              spP->stream= fopen( spP->fullName,"wb+" );       /* create for update, use binary mode (bfo) ! */
          if (spP->stream==NULL) return c2os9err(errno,E_FNA); /* default: file not accessible in this mode */  
        }
      #endif
      
      /*
      if (*sizeP>0) {
        #if defined win_unix || defined MACFILES
          if (*sizeP<=curSize) {              n= sizeof(b);
                                    p= *sizeP-n;
            err= pFseek( pid, spP, &p );               if (err) break;
            err= pFread( pid, spP,           &n, &b ); if (err) break;
          } // if
        #endif
                                           n= sizeof(b);
                                 p= *sizeP-n;
        err= pFseek ( pid, spP, &p );               if (err) break;
        err= pFwrite( pid, spP,           &n, &b ); if (err) break;
      } // if
      */

      if (*sizeP>curSize) {                n= sizeof( b );
                                 p= *sizeP-n;
        err= pFseek ( pid, spP, &p );               if (err) break;
        err= pFwrite( pid, spP,           &n, &b ); if (err) break;
      } // if
      
      if (tmp_pos>*sizeP)        tmp_pos= *sizeP; // adapt current pos, if now smaller
      err=   pFseek ( pid, spP, &tmp_pos );

    //upe_printf( "'%s' new  pos=%d err=%d\n", spP->name, tmp_pos, err );
    //fflush ( spP->stream );
    } while (false);
    
    if    (err==E_EOF) err= E_FULL;
    return err;
} /* pFsetsz */
 


os9err pFeof( _pid_, syspath_typ* spP )
/* check for EOF */
{
  #ifdef MACFILES
    long  curpos, curend;
    OSErr oserr;
    file_typ* f= &spP->u.disk.u.file;
    
        oserr= GetEOF ( f->refnum, &curend );
    if (oserr) return host2os9err(oserr,E_SEEK);

        oserr= GetFPos( f->refnum, &curpos );
    if (oserr) return host2os9err(oserr,E_SEEK);
    
    if (curpos>=curend)    return os9error(E_EOF);

  #else   
    if (feof(spP->stream)) return os9error(E_EOF);
  #endif

  return 0;
} /* pFeof */


// attempt to improve things a little
#define NEW_LUZ_FD_IMPL 1

/* prepare a FD from a cipb */
static void getFD( void* fdl, ushort maxbyt, byte *buffer )
{
    #define   FDS 16
    byte      fdbeg[FDS];                  /* buffer for preparing FD */
    byte*     att     = (byte*) &fdbeg[0]; /* the position of the attr field */
    ulong*    sizeP   = (ulong*)&fdbeg[9]; /* the position of the size field */
    Boolean   isFolder= false;
    ulong     u       = 0;
    struct tm tim;
    
    #if defined MACOS9
      CInfoPBRec* cipbP= (CInfoPBRec*)fdl;

    #elif defined win_unix
      char*       pathname;
      struct stat info;
      syspath_typ spRec;
      Boolean     ok;
      
      #ifdef windows32
        HANDLE hFile;
        SYSTEMTIME sy;
        // This was the bad thing: declared pointer instead of data structure !
        // LPWIN32_FIND_DATA data;
        WIN32_FIND_DATA data;
        FILETIME cr, lastA, lastW, lastL;
        Boolean uDir= false;
              
          mode_t v;
      #elif defined MACOSX
          mode_t v;
      #else
        __mode_t v;
      #endif
      
    #else
      #pragma unused(fdl)
    #endif

    /* fill up with 0 as far as needed, used as default */
    memset( buffer,0, maxbyt );
    memset( fdbeg, 0, FDS    );
    
    #if defined NEW_LUZ_FD_IMPL && defined windows32
      #pragma unused(v,lastA,tim)
      // use FindFirstFile for everything
      pathname= (char*)fdl;
      
          ok= (pathname!=NULL && ustrcmp( pathname,"" )!=0);
      if (ok) {
          hFile= FindFirstFile( pathname, &data );
          ok= ( (int)hFile>0 ); /* is -1, if error -> compare on >= !! */
      }
        
      if (ok) { 
          FindClose( hFile );
          // now extract all the data from WIN32_FIND_DATA structure
          // - is it a folder?
        //isFolder= (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0;
          isFolder= PathFound( pathname ); /* don't call stat_ for directories under Windows */
        //printf( "%d '%s'\n", isFolder, pathname );
          
          // - basic r or rw
          *att=poRead | (poRead<<3); // always readable
          if ((data.dwFileAttributes & FILE_ATTRIBUTE_READONLY)==0)
              *att |= poWrite | poWrite<<3; // if not write protected, also writeable
          // - always executable
          *att |= poExec | (poExec<<3);
          
          // - dirs are special
          if (isFolder) {
              *att |= 0x3F; // all permissions
              *att |= 0x80; // and dir bit
          } // if
          
          // - owner
          fdbeg[1] =0;
          fdbeg[2] =0; /* owner = superuser */
          
          // - get moification dates
          lastW = data.ftLastWriteTime;  
          // - convert to systemtime
          FileTimeToLocalFileTime ( &lastW, &lastL );
          FileTimeToSystemTime    ( &lastL, &sy );
          fdbeg[3]= sy.wYear-1900;
          fdbeg[4]= sy.wMonth;
          fdbeg[5]= sy.wDay;
          fdbeg[6]= sy.wHour;
          fdbeg[7]= sy.wMinute;

          // link count
          fdbeg[8]= 1; /* link count = 1 */
          
          // creation date
          cr = data.ftCreationTime;
          FileTimeToLocalFileTime ( &cr, &lastL );
          FileTimeToSystemTime    ( &lastL, &sy );
          fdbeg[13]= sy.wYear-1900;
          fdbeg[14]= sy.wMonth;
          fdbeg[15]= sy.wDay;
          
          // size of file
          // NOTE: only lower 32 bits are used. NT returns 64bits here...
          // NOTE: this is still a little hacky, as we use the info record only for size
          //       by now.
          info.st_size = data.nFileSizeLow; 
      }
      else {
          isFolder= true; /* assuming it is the top directory */
          *att    = 0xBF; /* dir bit and all permissions */

          /*
          if (isFolder) {
              *att |= 0x3F; // all permissions
              *att |= 0x80; // and dir bit
          } // if
          */

//     // file does not exist (any more)
//        *att= 0; // not accessible
      }
    
//    printf( "fd: " );
//    for (ii=0; ii<FDS; ii++) {
//        printf( "%02X ", fdbeg[ii] );
//    }
//    printf( "- %d '%s'\n", hFile, pathname );
    
    #else
    // conventional ifdef haystack :-)

    /* fill in constants */
    *att= 0x3F; /* default: peprpwerw (exe always set for now, %%% later: check file type!) */         

    #if defined MACOS9
      /* now get info from CInfoPBRec */
      isFolder= cipbP->hFileInfo.ioFlAttrib & ioDirMask; /* is it a folder */
      if (cipbP->hFileInfo.ioFlAttrib & 0x01) *att &= 0xED; /* clear write attrs */
      
    #elif defined win_unix
      pathname= (char*)fdl;
            
      #ifdef windows32
        isFolder= PathFound( pathname ); /* don't call stat_ for directories under Windows */
      #endif
      
    //upe_printf( "'%s' %s\n", pathname, isFolder ? "folder":"file" );
      *att= 0x00; 
      if (!isFolder) {
          stat_( pathname, &info );
          v= info.st_mode;
      
          if (v & S_IRUSR)    *att|= poRead;
          if (v & S_IWUSR)    *att|= poWrite;
          *att|= poExec; /* always — if (v & S_IXUSR) was here */

          if (v & S_IROTH)    *att|= 0x08;
          if (v & S_IWOTH)    *att|= 0x10;
          *att|= 0x20;       /* always — if (v & S_IXOTH) was here */
      
          #ifdef windows32
            *att|= 0x03; /* workaround because currently not visible */
          #endif
      
          isFolder= IsTrDir(v);
      }
      
      if (isFolder) *att|= 0x3F; /* all attr for directory */
    #endif
    
    if (isFolder) *att|= 0x80; /* set if it is a directory */
    

    fdbeg[1] =0;
    fdbeg[2] =0; /* owner = superuser */

    /* file dates */
    #ifdef MACOS9
      u= cipbP->hFileInfo.ioFlMdDat + OFFS_1904; /* get modification time */
    
    #elif defined win_unix
      u= info.st_mtime;
      
      #ifdef windows32
        if (isFolder) {
            // used to crash here because data was wrong type
            hFile= FindFirstFile( pathname, &data);
            if (hFile!=NULL) {
                debugprintf(dbgFiles,dbgNorm,("# getFD: '%s'\n",pathname));
                FindNextFile( hFile, &data);
                GetFileTime ( hFile, &cr, &lastA, &lastW );
                FileTimeToLocalFileTime ( &lastW, &lastL );
                FileTimeToSystemTime    ( &lastL, &sy );
                        
                uDir= true;
                u   = 0;
                
                FindClose( hFile );
            }
        }
      #endif
    #endif

    TConv( u, &tim );
//  tp = localtime( (time_t*)&u );
//  tim= *tp; /* copy it, as it might be overwritten */

    #ifdef windows32
      if (uDir) {
          tim.tm_year= sy.wYear-1900;
          tim.tm_mon = sy.wMonth-1; /* somewhat different month notation */
          tim.tm_mday= sy.wDay;
          tim.tm_hour= sy.wHour;
          tim.tm_min = sy.wMinute;
      }
    #endif
    
    fdbeg[3]= tim.tm_year;
    fdbeg[4]= tim.tm_mon+1; /* somewhat different month notation */
    fdbeg[5]= tim.tm_mday;
    fdbeg[6]= tim.tm_hour;
    fdbeg[7]= tim.tm_min;
    
    fdbeg[8]= 1; /* link count = 1 */
    debugprintf(dbgFiles,dbgNorm,("# getFD: %02d/%02d/%02d %02d:%02d\n", 
                tim.tm_year % 100,tim.tm_mon+1,tim.tm_mday, tim.tm_hour,tim.tm_min));
    
    #if defined MACOS9
      u= cipbP->hFileInfo.ioFlCrDat + OFFS_1904; /* get creation time */
    #elif defined win_unix
      #ifdef windows32
        if (isFolder) info.st_ctime= 0;
      #endif
      
      u= info.st_ctime;
    #endif

    TConv( u, &tim );
//  tp = localtime( (time_t*)&u );
//  tim= *tp; /* copy it, as it might be overwritten */

    fdbeg[13]= tim.tm_year;
    fdbeg[14]= tim.tm_mon+1;
    fdbeg[15]= tim.tm_mday;

    /* file length */
//  *sizeP= 0; /* by default */
    #endif // NEW_LUZ_FD_IMPL

        
    #if defined MACOS9
      if (isFolder) { /* virtual size is number of items plus 2 for . and .. */
          *sizeP= (ulong)(cipbP->dirInfo.ioDrNmFls+2)*DIRENTRYSZ;
      }
      else {          /* file size is the data fork size */
          *sizeP= (ulong) cipbP->hFileInfo.ioFlLgLen;
      }

    #elif defined win_unix
      if (isFolder) {
          *sizeP= 0; /* by default */
          
              ok= (pathname!=NULL && ustrcmp( pathname,"" )!=0);
          if (ok) {
                        strcpy( spRec.fullName, (char*)fdl );
                  ok= OpenTDir( spRec.fullName, &spRec.dDsc );
              if (ok) {
                  *att  = 0x80 | 0x3F;
                  *sizeP= os9_long( 2*DIRENTRYSZ ); /* if no entries */   
              
                  if (spRec.dDsc!=NULL) {
                      *sizeP= os9_long( DirSize(&spRec) );
                      closedir( spRec.dDsc );
                  }
              }
    	  } /* if (ok) */
      }
      else *sizeP= os9_long(info.st_size);
        
//    printf( "%d %10d '%s'\n", isFolder, os9_long(*sizeP), pathname );
    #endif
    
    
    /* copy FD beginning to caller's buffer */
    memcpy(buffer,fdbeg,maxbyt>FDS ? FDS : maxbyt);

//  /* fill up with 0 as far as needed */
//  if (maxbyt>16) {
//      memset( &buffer[16],0, maxbyt-16 );
//  }
} /* getFD */



static void setFD( syspath_typ* spP, void* fdl, byte *buffer )
/* adapt cipb with info of FD */
// void setFD(CInfoPBRec *cipbP, ushort maxbyt, byte *buffer)
{
    byte fdbeg[16]; /* buffer for preparing FD */
    struct tm tim;
    time_t u;
    
    #ifdef MACFILES  
      CInfoPBRec* cipbP= fdl;
    #else
      #ifndef linux
      #pragma unused(fdl)
      #endif
    #endif
    
    memcpy(fdbeg,buffer,16);  /* the size IS 16 */  
    tim.tm_year= fdbeg[ 3]; 
    tim.tm_mon = fdbeg[ 4]-1; /* the month correction */
    tim.tm_mday= fdbeg[ 5];
    tim.tm_hour= fdbeg[ 6];
    tim.tm_min = fdbeg[ 7];
    tim.tm_sec = 0;           /* no seconds supported */
    
    u= UConv( &tim );

    spP->u.disk.u.file.moddate        = u;
    spP->u.disk.u.file.moddate_changed= true;

    #ifdef MACFILES
      cipbP->hFileInfo.ioFlMdDat= (ulong)u-OFFS_1904; /* fill it into Mac's record */
    #elif defined win_unix
      Set_FileDate( spP, u );
    #endif

  //printf( "Set_FileDate was1\n" );

    if (fdbeg[13]==0) return; /* don't change it for invalid year */
    
    tim.tm_year= fdbeg[13];
    tim.tm_mon = fdbeg[14]-1; /* the month correction */
    tim.tm_mday= fdbeg[15];
    tim.tm_hour= 0;
    tim.tm_min = 0;
    tim.tm_sec = 0;           /* no seconds supported */

  //printf( "Set_FileDate was2\n" );
    u= UConv( &tim );
	
    #ifdef MACFILES
      cipbP->hFileInfo.ioFlCrDat= (ulong)u-OFFS_1904; /* fill it into Mac's record */
    #endif
} /* setFD */


/* get file descriptor for object */
os9err pHgetFD( _pid_, syspath_typ* spP, uint32_t *maxbytP, byte *buffer )
{
    void* fdl;

    #ifdef MACOS9
      os9err err;
      CInfoPBRec cipb;  

      /* get the cipb and prepare an FD from it */
      err= getCipb( &cipb, &spP->u.disk.spec ); if (err) return err;
      fdl=          &cipb;
    
    #elif defined win_unix
      fdl= &spP->fullName; /* use this for linux */
    
    #else
      #pragma unused(spP)
      /* no FD sector read possible w/o native OS access */
      return E_UNKSVC;
    #endif
    
    getFD( fdl, loword(*maxbytP),buffer );
    debugprintf(dbgFiles,dbgNorm,("# pHgetFD: no longer alive\n"));
    return 0;
} /* pHgetFD */


/* set file descriptor for object */
/* %%% currently only the file date will be set */
os9err pHsetFD( _pid_, syspath_typ* spP, byte *buffer )
{
    os9err err= 0;
    void*  fdl;
    
    #ifdef MACFILES
      OSErr       oserr;
      CInfoPBRec  cipb;  
      FSSpec*     spc= &spP->u.disk.spec;
      FInfo       f;

      /* get the cipb */
                    fdl= &cipb;
      err= getCipb( fdl, spc ); if (err) return err;

    #elif defined win_unix
      fdl= NULL;

    #else
      /* no FD read possible w/o native OS access */
      return E_UNKSVC;
    #endif


    setFD( spP, fdl, buffer);


    #ifdef MACFILES
      err= setCipb( fdl, spc ); /* and write it back */

           oserr= FSpGetFInfo  ( spc, &f );
      if (!oserr) {           
           Get_Creator_And_Type( spc, &f.fdCreator, &f.fdType );
           oserr= FSpSetFInfo  ( spc, &f );
      }
    #endif

    return err;
} /* pHsetFD */
/* adapt time/date of a file */



/* get file descriptor for file specified by "sector" */
os9err pHgetFDInf( _pid_, syspath_typ* spP, uint32_t *maxbytP,
                                            uint32_t *fdinf, byte *buffer)
{
    void*  fdl;
    os9err err= 0;
    
    #ifdef MACFILES
      #pragma unused(spP)
      
      union {
          CInfoPBRec    cipb;
          HParamBlockRec hpb;
      } pbu;
    
      Str255      fName, fN;
      short       volid;
      long        objid, dirid;
      byte        isdir;
      OSErr       oserr;
    //Boolean     conv;
    
  //#elif defined win_unix
  //  char        name  [OS9PATHLEN];
  //  char        result[OS9PATHLEN];
    #endif
    
    #ifdef MACFILES
      /* extract volid and file/dirid */    /* if (volid & VOLIDSIGN) volid |= VOLIDEXT; */
      volid= (*fdinf >> IDSHIFT) & VOLIDMASK; if (volid != 0    ) volid |= VOLIDEXT;
      objid=  *fdinf &  IDMASK;               if (objid & IDSIGN) objid |= IDEXT;

      err= check_vod( &volid, &objid, &dirid, &fN );
      
      /* try to locate the file/dir */
      /* --- first assume that it is a file and try to resolve ID */
      pbu.hpb.fidParam.ioVRefNum= volid;
      pbu.hpb.fidParam.ioFileID = objid;
      fName[0]=0; /* make sure PBResolveFileIDRef uses VRefNum, not a volume name */
      pbu.hpb.fidParam.ioNamePtr=fName;
      oserr = PBResolveFileIDRefSync(&pbu.hpb);
      debugprintf(dbgFiles,dbgDetail,("# pHgetFDsect: resolving volid=%d, fileid=$%lX returned oserr=%d\n",volid,objid,oserr));

      if (oserr==paramErr) {
              isdir= err!=0; /* netatalk handling */
          if (isdir) dirid= objid;
          else {
              strcpy( fName,fN );
              c2pstr( fName );
          }
      }
      else {
          if (oserr==notAFileErr) {
              /* it must be a directory */
              dirid= objid;
              isdir= true;
          }
          else {
              /* it is a file */
              if (oserr) return host2os9err(oserr,E_SEEK); /* probably because no file ID ref has been created */
              dirid= pbu.hpb.fidParam.ioSrcDirID; /* get dir ID */
              isdir= false;
          }
      } // if
      
      /* parent ID, vRefNum and (if not isdir, fName) are now set */
      /* --- now get cipb */
      pbu.cipb.hFileInfo.ioCompletion   = 0L;
      pbu.cipb.hFileInfo.ioNamePtr      = fName;
      pbu.cipb.hFileInfo.ioVRefNum      = volid;
      pbu.cipb.hFileInfo.ioFDirIndex    = isdir ? -1 : 0;   /* use the dir & vRefNum (and fName if file) */
      pbu.cipb.hFileInfo.ioDirID        = dirid;

      oserr = PBGetCatInfoSync(&pbu.cipb);
      debugprintf(dbgFiles,dbgDetail,("# pHgetFDsect: getting info for '%#s' returned oserr=%d\n",fName,oserr));
      if (oserr) return host2os9err(oserr,E_PNNF);

      /* prepare an FD from it */
      fdl= &pbu.cipb;
    
    #elif defined win_unix
      err= FD_Name( *fdinf, (char**)&fdl ); if (err) return err;
      
      /*
      if      (DirName( spP->fullName, *fdinf, (char*)&result, false )) {
          strcpy( name, spP->fullName );
          strcat( name, PATHDELIM_STR );
          strcat( name, result );
          fdl=   &name; // use this for windows and linux
      }
      */
          
    #else
      #pragma unused(spP,fdinf)
      /* no FD sector read possible w/o native OS access */
      return E_UNKSVC;
    #endif
    
    getFD( fdl, loword(*maxbytP), buffer );
    return 0;
} /* pHgetFDInf */



/* Directory file emulation */
/* ------------------------ */

/* open directory */
os9err pDopen( ushort pid, syspath_typ* spP, ushort *modeP, const char* pathname )
{
    os9err  err= 0;
    char*   pastpath;
    char    hostpath[OS9PATHLEN];
    char    ploc    [OS9PATHLEN];
    Boolean exedir= IsExec(*modeP);
    char*   p;
    char*   pp;
    
    #if defined MACFILES
      CInfoPBRec cipb;
      defdir_typ defdir;
      FSSpec*    spc= &spP->u.disk.spec;
      
    #elif defined win_unix
      DIR*    d;
      char    adapted[OS9PATHLEN];
      Boolean ok;
    #endif

 /* if (strlen(pathname)>DIRNAMSZ) return os9error(E_BPNAM); */

    strcpy  ( ploc,pathname );
    if      (*ploc==NUL) return E_BPNAM;
    pastpath= ploc;
    err     = parsepath( pid, &pastpath,hostpath, exedir );
    pp      = hostpath;
    debugprintf( dbgFiles,dbgNorm,( "# parsepath='%s' err=%d\n", pp, err ) );
    if (err) return err;
    
    #if defined MACOS9
      /* make an FSSpec for the directory */
                                   defdir= exedir ?_exe:_data;  
           err= getFSSpec( pid,pp, defdir, spc );
      if (!err) {
        /* object exists, but make sure it is a directory */
        getCipb( &cipb, spc ); /* it's a file, return error */
        if     (!(cipb.hFileInfo.ioFlAttrib & ioDirMask)) return os9error( E_FNA );  

        /* OS9TCP/ftp requires open dir for write !?! */
        // if (*modeP & 0x02) return os9error(E_FNA); /* can't open dir for write */
        
        strcpy( spP->fullName, pp );  
      } // if

      debugprintf( dbgFiles,dbgNorm,( "# pDopen: MakeFSSpec '%s' returned err=%d\n", pp, err ) );

    #elif defined win_unix
      debugprintf( dbgFiles,dbgNorm,( "# pp (v)='%s'\n", pp ) );
      err= AdjustPath( pp,adapted, false ); if (err) return err;
      pp = adapted;
      debugprintf( dbgFiles,dbgNorm,( "# pp (n)='%s'\n", pp ) );

           ok= OpenTDir( pp, &d );
      if (!ok) {
        #ifdef windows32
        if (GetLastError()==ERROR_NOT_READY) return os9error(E_NOTRDY);
        #endif
        return E_FNA;
      } // if
      
              spP->dDsc= d;
      strcpy( spP->fullName, adapted ); /* for later use with stat function */

    #else
      /* no directory read possible w/o native OS access */
      return E_UNKSVC;
    #endif

    spP->u.disk.u.dir.pos= 0; /* rewind it ! */
    
    #ifdef win_unix
      spP->svD_n= 0;
    #endif
    
    p= (char*)ploc+strlen(ploc)-1; /* only the filename itself, no path name */
    while (p>=ploc && *p!=PSEP) p--;
    p++;

    strcpy( spP->name,p );
    return err;
} /* pDopen */


/* not used for Mac */
os9err pDclose( _pid_, syspath_typ* spP )
{
    os9err err= 0;

    #ifdef win_unix
      DIR* d= spP->dDsc;
      if  (d!=NULL) { err= closedir( d ); spP->dDsc= NULL; }
      debugprintf( dbgFiles,dbgNorm,("# pDclose: '%s' err=%d\n", spP->fullName, err ));
    #else
      debugprintf( dbgFiles,dbgNorm,("# pDclose: '%s' err=%d\n", spP->name,     err ));
    #endif
    
    return err;
} /* pDclose */


#ifdef MACOS9
  static os9err get_dir_entry( ushort index, os9direntry_typ *deP, syspath_typ *spP )
  /* get directory entry referred to by index
   *  index=0 returns parent     ("..")
   *  index=1 returns dir itself (".")
   */
  {
    os9err  err;
    OSErr   oserr;
    FSSpec* spc= &spP->u.disk.spec;
    short   volid;
    long    dirid, objid;
    char*   q;

    union {
        CInfoPBRec    cipb;
        HParamBlockRec hpb;
    } pbu;

    Str255 fName;
    ushort n;

    /* set the vrefnum of the file or dir */
    volid= spc->vRefNum; /* get volume ID */
                                       /* (volid>VOLIDSIGN-1 || volid<-VOLIDSIGN)) */
  //if (debugcheck(dbgAnomaly,dbgNorm) && (volid>=0          || volid< VOLIDMIN)) {
  //    uphe_printf("get_dir_entry: vRefnum=%hd out of 3-bit range\n",volid); 
  //} // if

    //      deP->fdsect= (volid & VOLIDMASK) << IDSHIFT;
    memset( deP->name, 0, DIRNAMSZ ); /* clear before using */

    /* now get the entry */
    if (index==0) {
        /* parent directory ist the FIRST entry (even if Dibble said it's the second!) */
        strcpy(deP->name,".\xAE");
        
            objid= spc->parID; /* save to check range later */
        if (objid==fsRtParID) {
            /* this points to the (nonexistant) "root of root" */
            objid= fsRtDirID; /* let parent entry point again to the root */
        } // if
                                        dirid= objid;
        assign_fdsect( deP, volid,objid,dirid );
        debugprintf(dbgFiles,dbgDetail,("# get_dir_entry: index=%d: name='..', volid=%d, dirid=$%lX, fdsect=$%08lX\n",
                                           index, volid, dirid, deP->fdsect ));
    }
    else {
		    err= getCipb( &pbu.cipb, spc );
        if (err) return err;
        
        dirid= pbu.cipb.dirInfo.ioDrDirID; /* copy the sucker */

        /* now check for "." */
        if (index==1) {
            /* current directory is the SECOND entry */
            strcpy(deP->name,"\xAE");
                                      objid= dirid;
            assign_fdsect( deP, volid,objid, dirid );
            debugprintf(dbgFiles,dbgDetail,("# get_dir_entry: index=%d: name='.', volid=%d, dirid=$%lX, fdsect=$%08lX\n",
                                               index, volid, objid, deP->fdsect ));
        }
        else {
            /* some entry within this dir */
            fName[0]= 0;
            pbu.cipb.hFileInfo.ioCompletion = 0;
            pbu.cipb.hFileInfo.ioNamePtr    = fName;
            pbu.cipb.hFileInfo.ioVRefNum    = volid;
            pbu.cipb.hFileInfo.ioDirID      = dirid;
            pbu.cipb.hFileInfo.ioFDirIndex  = index-1;  /* use index, range 1..n */

                oserr = PBGetCatInfoSync(&pbu.cipb);
            if (oserr==fnfErr) return          os9error(E_EOF); /* this is no error, but only no more items in the folder */
            if (oserr)         return host2os9err(oserr,E_READ);

            /* there is an entry for this index */
            n=fName[0]>DIRNAMSZ ? DIRNAMSZ : fName[0];
            strncpy(deP->name,&fName[1], n); /* copy the name */
                
            q= deP->name;
            while (true) { /* convert the spaces in the file names to underlines */
  	  	        q= strstr( q," " ); if (q==NULL) break;
                q[ 0 ]= '_';
            } /* loop */

                
            if (fetchnames) {
                /* show slashes at filename beginnings as periods (that's what Fetch FTP does on OS9->mac */
                if (deP->name[0]=='/') deP->name[0]='.';
            }
            
            deP->name[n-1] |= 0x80; /* set old-style terminator */
            if (pbu.cipb.hFileInfo.ioFlAttrib & ioDirMask) {
                                          objid= pbu.cipb.dirInfo.ioDrDirID;
                assign_fdsect( deP, volid,objid,dirid );
                debugprintf(dbgFiles,dbgDetail,("# get_dir_entry: index=%d: dirname='%#s', volid=%d, dirid=$%lX, fdsect=$%08lX\n",
                                                   index, fName, volid, objid, deP->fdsect ));
            }
            else {
                objid= pbu.cipb.hFileInfo.ioDirID; /* get file ID, which returned in ioDirID by PBGetCatInfo */
                /* make sure that a file ID reference exists to find the file by pHgetFDsect later */
                pbu.hpb.fidParam.ioNamePtr  = fName;
                pbu.hpb.fidParam.ioVRefNum  = volid;
                pbu.hpb.fidParam.ioSrcDirID = dirid;
                oserr= PBCreateFileIDRefSync(&pbu.hpb);
                
                debugprintf(dbgFiles,dbgDetail,("# get_dir_entry: PBCreateFileIDRef returns oserr=%d\n",oserr));
                /* now assign file id (for which a reference now should exist) */
                assign_fdsect( deP, volid,objid,dirid );
                debugprintf(dbgFiles,dbgDetail,("# get_dir_entry: index=%d: filename='%#s', volid=%d, fileid=$%lX, filerefid=$%lX, fdsect=$%08lX\n",
                                                   index, fName, volid, objid, pbu.hpb.fidParam.ioFileID, deP->fdsect));
            }
        }
    } /* if (index==0) */

    return 0;
  } /* get_dir_entry */
#endif



/* read from (simulated) directory file */
os9err pDread( _pid_, syspath_typ *spP, uint32_t *n, char* buffer )
{
  os9err err;
    
  uint32_t* pos  = &spP->u.disk.u.dir.pos; /* current file position */
  uint32_t  offs = *pos & 0x1F;            /* offset    to start */
  ushort    index= *pos >> 5;              /* dir index to start */
  char*  myb= buffer;
    
  ulong  cnt, nbytes;
  os9direntry_typ os9dirent;
  
  #ifdef win_unix
    dirent_typ*     dEnt;
    dirtable_entry* mP= NULL;
    int             len;
    uint32_t        fdpos;
    Boolean         topFlag= false;
      
    #ifdef windows32
      dirent_typ dField;
    #endif
  #endif
    
//printf( "pos=%8d n=%4d '%s'\n", *pos, *n, spP->name );
  debugprintf(dbgFiles,dbgDetail,("# pDread: requests $%lX bytes\n",*n)); 
  cnt= *n;
  if  (*n==0) return 0;
        
  /* now copy remaining entries (if any) */
  /* now do it in one single step (bfo)  */
  while (cnt>0) {
    #ifdef MACFILES
      err= get_dir_entry( index, &os9dirent, spP );
          
    #elif defined win_unix
      memset( &os9dirent, 0,DIRENTRYSZ ); /* clear before using */
          
      err= DirNthEntry( spP,index, &dEnt );
      #ifdef windows32
        if (dEnt==NULL && err==E_EOF && index<=1) {
            dEnt= &dField; /* virtual field */
              
          if (index==0) strcpy( dEnt->d_name,".." );
          if (index==1) strcpy( dEnt->d_name,"."  );
          err= 0;
        } // if
      #endif
          
      if      (err) {
        if    (err==E_EOF && cnt<*n) break; // EOF can be handled !!
        return err;
      } // if
          
      if (*pos-offs==0) { /* if 1st entry, ".." expected */
        #ifdef windows32
          if (dEnt==NULL) {
              dEnt= &dField;
              strcpy( dEnt->d_name,"" ); /* virtual field */
          } // if
                    
          if (dEnt!=NULL &&
            ustrcmp( (char*)dEnt->d_name, "." )!=0 ) {
            strcpy        ( dEnt->d_name, "." );
            topFlag= true;
         // printf( "top '%s' '%s'\n", spP->fullName,dEnt->d_name );
          }
        #endif
                  
        if (topFlag) strcpy( dEnt->d_name,".." );
      } // if
            
      if (dEnt!=NULL) {
        GetEntry( dEnt, os9dirent.name, true );
        FD_ID          ( spP->fullName, dEnt, &fdpos, &mP );                      
        if (topFlag) { seekD0( spP ); topFlag= false; }
      }
      else err= E_EOF;
          
      if (!err) {
        len= strlen( os9dirent.name );
        os9dirent.name[ len-1 ] |= 0x80; /* set old-style terminator */
        os9dirent.fdsect= os9_long( fdpos );
      } // if
      
    #else
      /* %%% no directory read possible w/o native OS access */
      return E_UNKSVC;
    #endif
        
    if (!err) {
      nbytes = (offs+cnt)>DIRENTRYSZ ? DIRENTRYSZ-offs:cnt;
      memcpy(myb,(byte*)&os9dirent+offs, nbytes);
            
      cnt -= nbytes;
      myb += nbytes;
      offs = (offs+nbytes) & 0x1F;
      *pos+= nbytes;
      if (offs==0) index++;
    }
    else {
      if (err==E_EOF) {
        if (cnt==*n) return err;  /* nothing to be read */
        /* ok, no more to read, but something was there */
        break;
      }
      else return err;
    } // if
  } /* while */
    
  /* sucessful, set number of bytes actually read */
  *n-= cnt; /* adjust to show actually read # of bytes */
  debugprintf(dbgFiles,dbgDetail,("# pDread: returned $%lX bytes\n",*n)); 

  /*
  printf( "buf=%8d n=%4d '%s'\n", buffer, *n, spP->name );
  for (ii=0; ii<*n; ii++) {
    printf( "%02X ", (byte)buffer[ii] );
    if (ii%16==15) printf( "\n" );
  } // for
  */
  
  return 0;
} /* pDread */


/* get pointer position */
os9err pDpos( _pid_, syspath_typ* spP, uint32_t *posP )
{
    *posP= spP->u.disk.u.dir.pos;
    return 0;
} /* pDpos */



/* get size of directory file */
os9err pDsize( _pid_, syspath_typ* spP, uint32_t *sizeP )
{
    os9err err= 0;
    
    #ifdef MACFILES
      CInfoPBRec cipb;

      err= getCipb(&cipb, &spP->u.disk.spec); if (err) return err;
      *sizeP=(cipb.dirInfo.ioDrNmFls + 2) * DIRENTRYSZ; /* calc size (don't forget two more for . and .. */
      debugprintf(dbgFiles,dbgDetail,("# pDsize: number of files=%ld\n",cipb.dirInfo.ioDrNmFls));

    #elif defined win_unix
      *sizeP= DirSize( spP );
      
    #else
      #pragma unused(spP,sizeP)
      return E_UNKSVC;
    #endif
    
    return err;
} /* pDsize */



/* set read position */
os9err pDseek( ushort pid, syspath_typ* spP, uint32_t *posP )
{
    #ifdef MACOS9
      ulong       size;
      os9err      err;
    #elif defined win_unix
      int         cnt, n;
      dirent_typ* dEnt;
    #else
      #pragma unused(pid)
    #endif
    
    
    #ifdef MACOS9
      err= pDsize( pid,spP, &size ); if (err) return err;
      debugprintf(dbgFiles,dbgNorm,("# pDseek: dir size is = $%lX, seek request to $%lX\n",size,*posP));
      if (*posP>size) return os9error(E_SEEK);

    #elif defined win_unix
      seekD0( spP ); /* start at the beginning */

      n= 0;    cnt= *posP/DIRENTRYSZ;
      while (n<cnt) { /* search for the nth entry */
        dEnt= ReadTDir( spP->dDsc );
        spP->svD_n= 0; /* catch again */
        
        /* special handling for the root directory */
        #ifdef windows32
          if (n==0 && (dEnt==NULL || ustrcmp( dEnt->d_name,"." )!=0)) {
              seekD0( spP ); break;
          }
        #endif
         
        if (dEnt==NULL) return os9error(E_SEEK);
        if (ustrcmp( dEnt->d_name,AppDo )!=0 ) cnt--; /* ignore ".AppleDouble" */
        n++;
      } /* while */

    #else
      return E_UNKSVC;
    #endif  

    spP->u.disk.u.dir.pos= *posP;
    return 0;
} /* pDseek */



os9err pDchd( ushort pid, _spP_, ushort *modeP, char* pathname )
{
    os9err       err= 0;
    char*        pastpath;
    char         p[OS9PATHLEN]; 
    Boolean      exedir= IsExec(*modeP); /* check if it is chd or chx */
    process_typ* cp= &procs[pid];
    char*        defDir_s;
    
    #ifdef MACFILES
      short   vref;
      long    dirID;
      long*   xD;
      short*  xV;

    #elif defined win_unix
      char    adapted[OS9PATHLEN];
      
    #else
      #error I_Chgdir not yet implemented
    #endif
    
    /* now obtain volume and directory of new path */
    pastpath= pathname;
    err     = parsepath( pid, &pastpath,p, exedir);
    pathname= p;
     
    debugprintf( dbgFiles,dbgNorm,("# I$ChgDir: try to %s to '%s', err=%d\n",
                                      exedir ? "chx":"chd", pathname,err ));
    if (err) return err;
      
    /* get current default path */
    if (exedir) defDir_s= cp->x.path;
    else        defDir_s= cp->d.path;
        
    #ifdef MACFILES
      /* change based on default execution/data dir */
      if (exedir) { xV= &cp->x.volID; xD= &cp->x.dirID; }
      else        { xV= &cp->d.volID; xD= &cp->d.dirID; }
    
      vref= *xV;  dirID= *xD;
      err =get_dirid( &vref,&dirID,pathname ); if (err) return err;
      *xV = vref; *xD= dirID; /* now save these params as new default directory */

      debugprintf( dbgFiles,dbgNorm,("# I$ChgDir: ch%c: volID=%d, dirID=%ld (of new defdir)\n",
                   exedir ? 'x':'d', *xV,*xD ));
    
    #elif defined win_unix
      /* now do a OS changedir to "resolve" (and verify) location */
      err= AdjustPath( pathname,adapted, false ); if (err) return err;
      pathname=                 adapted;
            
      if  (!PathFound( pathname )) return E_FNA;    
      strcpy( defDir_s,pathname );
    #endif

    if (exedir) cp->x.type= fDir; /* adapt the type */
    else        cp->d.type= fDir;
    
    return 0; /* ok */
} /* pDchd */



os9err pDmakdir( ushort pid, _spP_, ushort *modeP, char* pathname )
{
  os9err       err;
  OSErr        oserr= 0;
  char*        pastpath;
  char         p[ OS9PATHLEN ]; 
  Boolean      exedir= IsExec(*modeP); /* check if it is chd or chx */
//process_typ* cp= &procs[pid];

  #if defined MACOS9
    FSSpec     localFSSpec;
    defdir_typ defdir;
    long       newdirid;
    
  #elif defined win_unix
    char adapted[ OS9PATHLEN ];
  #endif

  pastpath= pathname;
  err     = parsepath( pid, &pastpath,p,exedir ); if (err) return err;
  pathname= p;
    
  #if defined MACOS9
    debugprintf(dbgFiles,dbgNorm,("# I$MakDir: Mac path=%s\n",pathname));

    // --- create FSSpec, use default volume/dir
                                   defdir= exedir ? _exe:_data;
    err= getFSSpec( pid, pathname, defdir, &localFSSpec );
    
    if (!err || err==E_PNNF) {
      debugprintf(dbgFiles,dbgDetail,("# I$MakDir: FSSpec for new dir: OSErr=%d, vRefNum=%d, dirID=%ld\n",
                                         err, localFSSpec.vRefNum,localFSSpec.parID));
      // --- create the directory
      oserr = FSpDirCreate(&localFSSpec, smSystemScript, &newdirid);
      debugprintf(dbgFiles,dbgNorm,  ("# I$MakDir: oserr=%d, Id of new dir is %ld\n",
                                         oserr,newdirid));
    } // if

  #elif defined windows32
    err= AdjustPath( pathname,adapted, true ); if (err) return err;
    pathname=                 adapted;

    debugprintf(dbgFiles,dbgNorm,("# I$MakDir: Win path=%s\n",pathname));
    if (!CreateDirectory( pathname,NULL )) oserr=GetLastError();

  #elif defined UNIX
    err= AdjustPath( pathname,adapted, true ); if (err) return err;
    
    debugprintf(dbgFiles,dbgNorm,("# I$MakDir: Linux path=%s\n",adapted));
    if (PathFound( adapted ) || 
        FileFound( adapted )) return E_CEF;
          
    if (mkdir( adapted,0x01c0 )==0) return 0; /* rwx------ */
    return E_BPNAM;

  #else
    /* other OS */
    #error I_Makdir not yet implemented
  #endif

  return host2os9err( oserr, E_WRITE );
} /* pDmakdir */



#if defined MACOS9 && defined powerc && !defined MPW
  static OSErr FSRename_Unique( FSRef* srcRef, FSRef* dstRef, FSRef* newRef )
  {
    #define DelMe ".delete_me"
    OSErr oserr;
    int   i, len, n= 1; 
    char  name    [ 80 ];
    char  unifield[ 80 ];
    FSRef renRef, savRef;
    
    /*
    const char*      p= pathName;
    if      (strstr( p, DelMe )==p) {
          p= strstr( p, "_" );
      if (p==NULL)   p= pathName;
      else           p++;
    } // if
    */
    
    while (true) {
      sprintf    ( name, "%s_%d", DelMe, n++ );
      len= strlen( name );
      
      for (i= 0; i<len; i++) {
        unifield[ 2*i   ]= '\0'; // Create unicode field
        unifield[ 2*i+1 ]= name[ i ];
      } // for
    
            oserr= FSRenameUnicode( srcRef, len, (UniChar*)unifield, kTextEncodingMacRoman, &renRef );
      if   (oserr!=dupFNErr) {
        if (oserr) break;
            
            oserr= FSMoveObject( &renRef, dstRef, newRef );
        if (oserr!=dupFNErr) break;
        
        memcpy( &savRef, &renRef, sizeof( FSRef ) ); // rename was possible, take new one already
        srcRef= &savRef;
      } // if
    } // loop
    
    return oserr;
  } // FSRename_Unique
#endif



os9err pDsetatt( ushort pid, syspath_typ* spP, ulong *attr )
{
    os9err err= 0;

    #if defined MACOS9 || defined linux
      OSErr  oserr= 0;
    #endif
      
    #ifdef MACOS9
      char*  pp= spP->name;
      char   pathname[OS9PATHLEN]; 
      FSSpec delSpec;
      FSSpec spc;
      OSType creator, type;

      #if defined powerc && !defined MPW
        FSSpec dstSpec;
        FSRef  delRef;
        FSRef  dstRef;
        pending_typ* dpp;
        int    pnd;
      #else
        int    k;
      #endif
        
    #elif defined win_unix
     ushort mode;
     char*  pp= spP->fullName;
      
      #if defined windows32 || defined MACOSX
        char cmd[OS9PATHLEN];
      #endif
    
    #else
      return E_UNKSVC;
    #endif
          

    if (*attr & 0x80 ) return 0; /* it is already a directory */
      
    #ifdef win_unix
          err= RemoveAppledouble( spP ); /* because this file is not visible */
      if (err==E_EOF) err= 0; // this is not an error !!
      if (err) return err;
    #endif
      
    #ifdef MACOS9
    //sv   = pp; /* 'parsepath' will change <pp> */
    //err  = parsepath( pid,&pp, pathname, false ); if (err) return err;
      strcpy                   ( pathname, spP->fullName );
      oserr= getFSSpec( pid,     pathname, _data, &delSpec );
    //pp   = sv;
    #endif
    
    /* can't change attributes in Mac/Linux/Windows */
    /* But remove and recreate as file is possible */
    err= pDclose      ( pid, spP     ); if (err) return err;
  //err= syspath_close( pid, spP->nr ); if (err) return err;
      
    #ifdef MACOS9
      #if defined powerc && !defined MPW
        // try to delete all of them
        for (pnd= 0; pnd<PENDING_MAX; pnd++) {
              dpp= &dPending[ pnd ];
          if (dpp->toBeDeleted) {
                 oserr= FSDeleteObject( &dpp->newRef );
            if (!oserr) dpp->toBeDeleted= false;
          } // if
        } // if

        for (pnd= 0; pnd<PENDING_MAX; pnd++) {
               dpp= &dPending[ pnd ];
          if (!dpp->toBeDeleted) break;
        } // if
      
             oserr= FSpMakeFSRef( &delSpec, &delRef );
        if (!oserr) oserr= getFSSpec( pid, "", _start, &dstSpec );
      //upe_printf( "oserr=%d -> pathname='%s'\n", oserr, "destDir" );

        if (!oserr) oserr= FSpMakeFSRef( &dstSpec, &dstRef );
      //upe_printf( "oserr=%d dstSpec\n", oserr );
 
      //if (!oserr) oserr= FSRename_Unique( &delRef, &dstRef, spP->name, &dpp->newRef );
        if (!oserr) oserr= FSRename_Unique( &delRef, &dstRef, &dpp->newRef );
      //upe_printf( "oserr=%d =>pathname='%s'\n", oserr, "renamed to DeleteMe_XXX" );
      
        if (!oserr) oserr= FSDeleteObject( &dpp->newRef );
      //upe_printf( "oserr=%d DELETED\n", oserr );

        /*
        if  (oserr) {
          for (k=0; k<MAXTRIES_DEL; k++) {
                 oserr= FSDeleteObject( &newRef );
            if (!oserr) break;
        
            HandleEvent(); 
          } // for
        } // if
        */
        
        if  (oserr==fBsyErr) { dpp->toBeDeleted= true; oserr= 0; }
    
      #else
        for (k=0; k<MAXTRIES_DEL; k++) {
               oserr= FSpDelete( &delSpec );
          if (!oserr) break;
        
          #ifdef MPW
            HandleEvent();
          #else
            if (k<20) HandleEvent();
            else      HandleOneEvent( nil, 100 );
          #endif
        } // for

      //upe_printf( "oserr=%d k=%d DELETED\n", oserr, k );
      #endif
            
      debugprintf( dbgFiles,dbgNorm,("# pDsetatt: '%s' remove oserr=%d\n", pp, oserr )); 
      if (oserr) err= host2os9err( oserr,E_SHARE );

    #elif defined windows32 // || defined MACOSX
//    spP->dDsc= opendir( spP->fullName );
//    while (true) {
//        dEnt= readdir( spP->dDsc ); if (dEnt==NULL) break;
//        printf( "'%s'\n", dEnt->d_name );
//    }
//    closedir( spP->dDsc );
    
      /* can't be removed at the moment */
//    if (!RemoveDirectory(pp)) err= E_DNE;
//    if (err) { 
//      spP->dDsc= opendir( spP->fullName );
//      printf( "%04X\n", GetFileAttributes( pp ) );
//      SetFileAttributes( pp, 0x0001 ); /* it's a little bit a bad joke */
//      return 0; 
//    }

      sprintf( cmd, "rmdir /S /Q %s", pp );
      err= call_hostcmd( cmd, pid, 0,NULL ); if (err) return err;

    #elif defined MACOSX
      sprintf( cmd, "rmdir %s", pp );
      err= call_hostcmd( cmd, pid, 0,NULL ); if (err) return err;

    #elif defined linux
  //#elif defined UNIX
      oserr= remove( pp ); if (oserr) err= host2os9err( oserr,E_DNE );
    #endif
      
    debugprintf( dbgFiles,dbgNorm,("# pDsetatt: '%s' remove err=%d\n", pp,err )); 
    if (err) return err;
      
    spP->type= fFile; /* change the file type now */
    
    #ifdef MACOS9
    //strcpy               ( pathname, "deleted_directory" );
      strcpy               ( pathname, spP->fullName );
      oserr= getFSSpec( pid, pathname, _data, &spc   );
      debugprintf( dbgFiles,dbgNorm, ("# pDsetatt: getFSSpec returned err=%d\n", err ) );
    
      /* file does not exist yet, create it first */
      Get_Creator_And_Type ( &spc, &creator, &type );
      oserr=      FSpCreate( &spc,  creator,  type, smSystemScript);
      debugprintf( dbgFiles,dbgNorm, ("# pDsetatt: FSpCreate returned err=%d\n", err ) );
        
          oserr= FSpOpenDF( &spc, fsRdWrPerm, &spP->u.disk.u.file.refnum );
      if (oserr) err= host2os9err( oserr,E_PNNF );
    //upe_printf( "oserr=%d err=%d OPEN_IT\n", oserr, err ); 
    #else
                               mode= 0x03 | poCreateMask;
       err= pFopen( pid, spP, &mode, pp ); // and open it as file again on the same system path
    #endif  
    
    /*
    #ifdef MACOS9
      for (k=0; k<MAXTRIES_DEL; k++) {
        if  (err!=E_CEF) break;
                                     mode= 0x03;
             err= pFopen( pid, spP, &mode, pp ); // try do open it
        if (!err) break;
        
                                     mode= 0x03 | poCreateMask;
        err=      pFopen( pid, spP, &mode, pp );
      } // for
    #endif
    */
    
    debugprintf( dbgFiles,dbgNorm,("# pDsetatt: '%s' end err=%d\n", pp, err )); 
    return err;
} /* pDsetatt */



/* check for EOF */
os9err pDeof( ushort pid, syspath_typ* spP )
{
    uint32_t n;
    os9err err= pDsize( pid,spP, &n ); if (err) return err;
    if    (n<=spP->u.disk.u.dir.pos) return os9error(E_EOF);
    
    return 0; 
} /* pDeof */


/* eof */

