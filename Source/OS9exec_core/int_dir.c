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
/* (c) 1993-2006 by Lukas Zeller, CH-Zuerich  */
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
 *    Revision 1.9  2005/01/22 15:57:30  bfo
 *    Renamed to ifdef MACOS9
 *
 *    Revision 1.8  2004/12/03 23:58:08  bfo
 *    MacOSX MACH adaptions
 *
 *    Revision 1.7  2004/11/20 11:44:07  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.6  2004/10/22 22:51:12  bfo
 *    Most of the "pragma unused" eliminated
 *
 *    Revision 1.5  2002/10/09 20:45:06  bfo
 *    uphe_printf => upe_printf
 *
 *
 */

/* OS9exec/nt internal dir */
/* ======================= */

/* global includes */
#include "os9exec_incl.h"
#include <ctype.h>

#ifndef MACOS9
  os9err int_dir(ushort pid, int argc, char **argv)
  /* internal "dir" command for OS9exec/nt */
  {
      os9err err;

      #ifdef windows32
        /* using Window's command line shell to perform this task */
        err= call_hostcmd( "dir", pid,argc-1,&argv[1] );

      #elif defined linux
        err= call_hostcmd( "ls",  pid,argc-1,&argv[1] );

      #else
        upe_printf( "internal dir not yet implemented %%%\n" );
        err= 1; /* SIGWAKE */
      #endif
      
      return err;
  } /* int_dir */

#else
  /* special mac includes */
  #include <Finder.h>

static void usage( char* name, ushort /* pid */ )
{
    upe_printf("Usage:    %s [options] [dirname]\n",name);
    upe_printf("Function: Displays directory\n");
    upe_printf("Options:  -x       relative to execution dir\n");
    upe_printf("          -e       extended dir listing\n");
} /* usage */

static void Two( char* vv, int ii )
{
    if (ii<10) sprintf( vv, "0%d", ii );
    else       sprintf( vv,  "%d", ii );
} /* Two */

/* internal "dir" command for OS9exec/nt */
os9err int_dir(ushort pid, int argc, char **argv)
{
    /* options */
    Boolean extlisting;
    int     exedir;
    int     invis;

    /* command line scanning */
    char *p;
    int nargc=0,h;
    #define MAXARGS 2
    char *nargv[MAXARGS];

    /* internals */
    short volid;
    long dirid;
    OSErr oserr;
    os9err err;
    FSSpec theFSSpec;
    CInfoPBRec  cipb;
    Str255      fName;
    int isfolder;
    int k,index;
    Str255 dirName;
    char *inpathptr;
    #define MAXLINELEN 81
    int c,n;
    char typecreat[10];
    time_t modified;
    ulong ressize,datasize;
    #define MAXDATESIZE 30
    char outdate[MAXDATESIZE];
    struct tm tim;
    char   yy[3],mm[3],dd[3];

    /* %%% simple version for now */
    oserr=0;
    err=0;
    /* show args for debug */
    if (debugcheck(dbgProcess,dbgDeep)) {
        upho_printf("os9dir: argc=%d",argc);
        for (k=0; k<argc; k++) upo_printf(", argv[%d]='%s'",k,argv[k]);
        upo_printf("\n");
    }
    /* set default options */
    extlisting= false;
    exedir=0;
    invis=0;
    /* get arguments and options */
    for (h=1;h<argc;h++) {
        p=argv[h];    
        if (*p=='-') { 
            p++;
            switch (tolower(*p)) {
                case '?' :  usage(argv[0],pid); return 0;
                case 'e' :  extlisting= true; break;
                case 'a' :  invis=1; break;
                case 'x' :  exedir=1; break;
                default  :  upe_printf("Error: unknown option '%c'!\n",*p); 
                            usage(argv[0],pid); return 1;
            }   
        }
        else {
            if (nargc>=MAXARGS) { upe_printf("Error: no more than %d arguments allowed\n",MAXARGS); return 1; }
            nargv[nargc++]=argv[h];
        }
    }       
    if (nargc>1) {
        upe_printf("Error: no or one argument allowed\n"); return 1;
    }

    #ifdef MACOS9
      /* set right base dir */
      if (exedir) { volid= procs[pid].x.volID; dirid=procs[pid].x.dirID; }
      else        { volid= procs[pid].d.volID; dirid=procs[pid].d.dirID; }
    #endif
    
    debugprintf(dbgUtils,dbgNorm,("# int_dir: volid=%d, dirid=%ld\n",volid,dirid));
    /* now do the stuff */
    if (nargc==1) {
        inpathptr=nargv[0];
        parsepath(pid, &inpathptr, &dirName,exedir);

        /* make FSSpec out of first arg */
            err= getFSSpec( pid,dirName, exedir?_exe:_data, &theFSSpec); 
        if (err) return err;
        /*  check if this is a directory, if yes, get its dirID */
        err= getCipb( &cipb, &theFSSpec ); if (err) return err;
        
        if (!(cipb.hFileInfo.ioFlAttrib & ioDirMask)) {
            /* path specifies a file */
            dirid=theFSSpec.parID; /* just use parent dir's ID */
            upo_printf("Warning: specified path '%s' specifies a file. Listing parent dir instead\n",nargv[0]);
        }
        else {
            /* path specifies a directory */
            dirid=cipb.dirInfo.ioDrDirID; /* copy the sucker */
        }
        volid=theFSSpec.vRefNum;
    }
    index=1;
    c=0;
    /* display header */
    if (extlisting) {
        upo_printf("Filename                        Type Crea  W DataSize  ResSize  Modified\n");
        upo_printf("------------------------------  ---- ----  - -------- --------  --------\n");
    }
    /* now index trough the folder */
           oserr= 0;
    while (oserr == noErr) {
        //dirID = origDirID;
        //localIndex = index;
        fName [0] = 0;
        cipb.hFileInfo.ioCompletion = 0L;
        cipb.hFileInfo.ioNamePtr    = fName;
        cipb.hFileInfo.ioVRefNum    = volid;
        cipb.hFileInfo.ioFDirIndex  = index;    // use a real index
        cipb.hFileInfo.ioDirID      = dirid;
        oserr = PBGetCatInfoSync(&cipb);

        if (oserr==fnfErr) {
            /* this is no error, but only no more items in the folder */
            oserr=0;
            break; // exit while loop
        }
        else if (oserr==noErr && !(cipb.hFileInfo.ioFlFndrInfo.fdFlags & fInvisible)) {
            /* there is an entry for this index */
            isfolder= cipb.hFileInfo.ioFlAttrib & ioDirMask;
            /* --- append / if it is a directory */
            n=fName[0]; /* size of name */
            if (isfolder) {
                n++;
                fName[n]='/';
                fName[0]=n;
            }
            /* --- show trailing slashes as period */
            if (fetchnames) {
                if (fName[1]=='/') fName[1]='.';
            }
            /* -- now show it */
            if (extlisting) {   
                typecreat[4]=0x20;
                typecreat[9]=0;
                if (!isfolder) {
                    *((OSType *)typecreat)=cipb.hFileInfo.ioFlFndrInfo.fdType;
                    *((OSType *)&typecreat[5])=cipb.hFileInfo.ioFlFndrInfo.fdCreator;
                    modified=cipb.hFileInfo.ioFlMdDat + OFFS_1904;
                    datasize=cipb.hFileInfo.ioFlLgLen;
                    ressize=cipb.hFileInfo.ioFlRLgLen;
                    if (cipb.hFileInfo.ioFlFndrInfo.fdFlags & kIsAlias) {
                        strcpy(typecreat,"<Alias>");
                    }
                }
                else {
                    strcpy(typecreat,"<Folder>");
                    modified=cipb.dirInfo.ioDrMdDat + OFFS_1904;
                    ressize=0;
                    datasize=cipb.dirInfo.ioDrNmFls;
                }
                
                TConv( modified, &tim );
                Two( yy, tim.tm_year % 100 );
                Two( mm, tim.tm_mon+1      );
                Two( dd, tim.tm_mday       );
                sprintf( outdate, "%2s/%2s/%2s", yy,mm,dd );
                upo_printf("%#-32s%-9s  %c %8ld %8ld  %s\n",fName,typecreat,cipb.hFileInfo.ioFlAttrib & 0x01 ? '-' : 'w', datasize,ressize,outdate);
            }
            else {
                if (c+n > MAXLINELEN) {
                    upo_printf("\n");
                    c=0;
                }
                if (n<16) {
                    upo_printf("%#-16s",fName);
                    c+=16;
                }
                else {
                    upo_printf("%#-32s",fName);
                    c+=32;
                }
            }
        }
        index++; /* next file */
    }
    if (!extlisting) upo_printf("\n"); /* new line */
    return 0; /* ok */
} /* int_dir */

#endif
