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
 *    Revision 1.12  2007/03/24 13:05:45  bfo
 *    "DoWait" is visible from outside
 *
 *    Revision 1.11  2006/02/19 15:43:15  bfo
 *    Header changed to 2006
 *
 *    Revision 1.10  2005/06/30 11:41:21  bfo
 *    Mach-O support
 *
 *    Revision 1.9  2005/01/22 16:09:18  bfo
 *    Renamed to ifdef MACOS9
 *
 *    Revision 1.8  2004/12/03 23:59:19  bfo
 *    MacOSX MACH adaptions
 *
 *    Revision 1.7  2004/11/20 11:44:07  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.6  2003/07/31 14:42:50  bfo
 *    Use sector size instead of 256 at "int_move"
 *
 *    Revision 1.5  2002/11/09 11:30:37  bfo
 *    RBF move in same directly running now correctly
 *
 *    Revision 1.4  2002/10/09 20:47:41  bfo
 *    uphe_printf => upe_printf, move paths for RBF under Windows corrected
 *
 *
 */

/* OS9exec/nt internal move */
/* ======================== */
/* based on LuZ's PD move for OS9 V2.1 */

/* global includes */
#include "os9exec_incl.h"
#include <ctype.h>

/* special mac includes */
#ifdef MACOS9
#include <Finder.h>
#include <ctype.h>
#endif

/* Version/Revision (of LuZ's PD move for OS-9)
   2.0 : 88-08-15 : first version in C
   2.1 : 90-01-14 : -r option added
*/
#define VERSION 2
#define REVISION 1

/* Edition History
   ---------------
   001 : 88-08-15 : created
   002 : 90-01-14 : -r option added
   003 : 94-10-24 : ANSIfication (stp)
   004 : 96-10-11 : Made an internal utility of OS9exec/NT 2.0
*/

#define EDITION 4

/* global variables */
/* ================ */

static int nargc;  /* number of non-option (and non argv[0]) arguments  */

/* option flags */
/* ------------ */

static int   wild, quiet, dodir, rewrite;
static char *wilddir;
static int   exe_dir;

/* program code */
/* ============ */

/**** display program usage */
static void usage (char *pname)
{
  upe_printf("Syntax:   %s [<options>] <from> [<to>] [<options>]\n",pname);
  upe_printf("Function: Moves files from one directory to another without\n");
  upe_printf("          actually copying the file contents\n");
  upe_printf("Options:  -d : move subdirectories too (required with -w only)\n");
  upe_printf("          -p : don't print file names moved (with -w only)\n");
  upe_printf("          -w=<dir name> : wildcard move to specified directory\n");
  upe_printf("          -x : look in execution directory for source\n");
  upe_printf("          -r : rewrite destination\n");
} /* usage */

/**** analyze command line options */
static os9err options (int argc,char *argv[])
{
  int count, nextarg;
  char *sc;

  for (count=nargc=0;++count<argc;) {
    if (*(sc=argv[count])=='-') {
      /* this argument is an option (begins with a hyphen) */
      nextarg=0;
      while ((*(++sc) != 0)&&!nextarg) {
        switch (tolower(*sc)) {
          case '?' : usage(argv[0]); return 2;
          case 'x' : exe_dir= true;  break;
          case 'p' : quiet  = true;  break;
          case 'd' : dodir  = true;  break;
          case 'r' : rewrite= true;  break;
          case 'w' :
            wild=1;
            if (*(++sc)!='=') return _errmsg( 1,"missing '=' for -w option\n");
            wilddir=sc+1;
            nextarg=1;
            break;

          default  :
            usage(argv[0]);
            return _errmsg( 1,"unknown option '%c'\n",*sc );
            break;
        } /* switch */
      } /* while */
    } /* if hyphen */
    else nargc++;
  } /* for */
  return 0;
} /* options */

static os9err separate(char **dnp,char **fnp)
/**** separate file and directory name
 Input  : dnp = ptr to pathlist string (pointer)
          fnp = ptr to filename string (pointer, uninitialized)
 Output : dnp = ptr to directory pathlist string (pointer)
          fnp = ptr to filename string (pointer) */
{
  char *p;

  p=*dnp+strlen(*dnp);
  while (--p>*dnp) {
    if (*p=='/') {
      *(p++)=NUL;
      *fnp=p;
      return 0;
    }
  }
  if (*p=='/') /* if pathlist has only ONE slash at the beginning */
     return _errmsg( 1,"file nath expected, but not device name: \"%s\"\n",*dnp );
  *fnp=p;
  *dnp="."; /* default directory name */
  return 0;
} /* separate */

static os9err move_file( ushort cpid, char *fromdir,char *fromname,
                                      char   *todir,char   *toname )
/**** move file
 Input  : fromdir,fromname = source directory and filename
          todir, toname = destination directory and filename
 Note   : if toname is empty, it will be copied from the source name
*/
{
    os9err  err, cer;
    char    nmS[OS9PATHLEN];
    char    nmD[OS9PATHLEN];
    char    dvS[OS9PATHLEN];
    char    dvD[OS9PATHLEN];
    char    pS [OS9PATHLEN];
    char    pD [OS9PATHLEN];
    char    *destname;
    
    #if defined MACOS9
      OSErr      oserr;
      FSSpec     sourceSpec;
      FSSpec     destSpec;
      FSSpec     destDirSpec;
      CInfoPBRec cipb;
      
    #elif defined win_unix
      char   adD[OS9PATHLEN];
      char   adS[OS9PATHLEN];
      int    i;
    #endif
    
    ushort     pathS, pathD, pathX;
    ptype_typ  typeS, typeD;
    char       *nameS, *nameD;
    uint32_t   fdS, dfdS, dcpS, sctS, len,
               fdD, dfdD, dcpD, sctD, l;
    ulong      a0; /* host pointer container for SS_DevNm */
    Boolean    isRBF, asDirS, asDirD;
    
    
    strcpy(nmS,fromdir );
    strcat(nmS,PSEP_STR);
    strcat(nmS,fromname);
    nameS= nmS;
    
    typeS= IO_Type( cpid,nameS, 0x00 ); /* get the device type: Mac/PC or RBF */

    /* <nameS= pS> belongs INSIDE the guard: pS is only filled by parsepath(), so
     * on the RBF path (where parsepath is skipped) this used to point nameS at an
     * uninitialized stack buffer. Harmless only because the RBF branch below reads
     * nmS rather than nameS -- one edit away from being a real bug. */
    if (typeS!=fRBF) {
        parsepath( cpid, &nameS, pS, exe_dir );
        nameS= pS;
    }

    strcpy(nmD,todir);
    strcat(nmD,PSEP_STR);
    if      (strcmp( toname,"" )==0) /* use source's name if no new dest name specified */
         destname= fromname;
    else destname=   toname;
    strcat(nmD,destname);
    nameD= nmD;
    
    typeD= IO_Type( cpid,nameD, 0x00 );

    if (typeD!=fRBF) { /* same as the source side above */
        parsepath( cpid, &nameD, pD, exe_dir );
        nameD= pD;
    }
    
        len= strlen( destname );
    if (len>DIRNAMSZ)
        return _errmsg( E_BPNAM,"name too long \"%s\"\n", destname );

    asDirS= false;   
//  typeS= IO_Type( cpid,nameS, 0x00 ); /* get the device type: Mac/PC or RBF */
//  typeD= IO_Type( cpid,nameD, 0x00 );

   
        isRBF= (typeS==fRBF || typeD==fRBF);
    if (isRBF) {
        if     (typeS!=fRBF || typeD!=fRBF) /* both must be the same type */
            return _errmsg( 1, "can't move from one device to another" );
        
        /* try as file first, then as dir */
        err= get_locations( cpid,typeS, nmS,false, &asDirS, &fdS,&dfdS,&dcpS,&sctS );
        if (err) return _errmsg( err, "can't find \"%s\". ", nmS );
    }
    else {
        #ifdef MACOS9
          /* make FSSpec for source */
              err= getFSSpec( cpid,pS, exe_dir ?_exe:_data, &sourceSpec); /* find source */
          if (err) return _errmsg(err, "can't find \"%s\". ",pS);
          debugprintf( dbgUtils,dbgDeep,("# int_move: source vol=%d, dir=%d, name=%#s\n",
                       sourceSpec.vRefNum,sourceSpec.parID,sourceSpec.name));

          if (!dodir) {
              /* check if it is a directory */
              getCipb( &cipb,&sourceSpec );
              asDirS=  (cipb.hFileInfo.ioFlAttrib & ioDirMask);
          }

//      #elif defined(windows32)
//        /* using Window's command line shell to perform this task */
//        return call_hostcmd("move", pid, argc-1,&argv[1]);

        #elif defined win_unix
              err= AdjustPath( nameS, (char*)&adS, false );
          if (err) return _errmsg( err, "can't find \"%s\". ", adS );

          nameS= adS;
          if (!dodir) asDirS= PathFound( nameS );
        
        #else
          upe_printf("internal move not yet implemented %%%\n");
          return 1; /* SIGWAKE */
        #endif
        
    }

    if (!dodir && asDirS)
        /* it's a dir, return error => don't move, but it's ok */
        return _errmsg( 0, "directory \"%s\" NOT moved\n",nmS );

    if (isRBF) {        err= usrpath_open( cpid,&pathD, typeD,nmD,0x01 ); 
        if (err==E_FNA) err= usrpath_open( cpid,&pathD, typeD,nmD,0x81 );
        if (err==0) {
            err= usrpath_close( cpid, pathD );
            
            if (rewrite) {
                /* remove file at destination first */
                    err= delete_file( cpid,fRBF, nmD,0x01 );
                if (err) return _errmsg( err,"can't delete \"%s\". ",destname );
            }
            else {
                /* already exists, can't continue */
                return _errmsg( 1,"\"%s\" already exists\n",destname );
            }
        }
    }
    else {
        #ifdef MACOS9
          /* make FSSpec for destination */
          err= getFSSpec( cpid,pD, exe_dir ?_exe:_data, &destSpec ); /* find destination */
          debugprintf(dbgUtils,dbgNorm,("# int_move: destination err=%d vol=%d, dir=%d, name=%#s\n",
                    err,destSpec.vRefNum,destSpec.parID,destSpec.name));
                    
          /* check if destination exists */
          if (!err) {
              /* the destination object already exists */
              if (rewrite) {
                  getCipb( &cipb, &destSpec );
                      asDirD= (cipb.hFileInfo.ioFlAttrib & ioDirMask);
                  if (asDirD) err= E_FNA;
                  else    { oserr= FSpDelete(&destSpec);
                              err= host2os9err(oserr,E_FNA); }
                  if (err) return _errmsg( err,"can't delete \"%s\". ",destname );
              }
              else {
                  /* already exists, can't continue */
                  return _errmsg( 1,"\"%s\" already exists\n", destname );
              }
          }
          
        #elif defined win_unix
               err= AdjustPath( nameD, (char*)&adD, false );      
          if (!err) err= (!FileFound( adD ) && !PathFound( adD ));
          if (!err) {
              /* the destination object already exists */
              if (rewrite) {
                      asDirD= PathFound( adD );
                  if (asDirD) err= E_FNA;
                  else        err= remove( adD ); /* remove file at destination first */
                  if (err) return _errmsg( err,"can't delete \"%s\". ",adD );
              }
              else {
                  /* already exists, can't continue */
                  return _errmsg( 1,"\"%s\" already exists\n", adD );
              }
          }
        #endif
    }
    
    
    if (isRBF) {
        // cache flush of this file
        err= Flush_Entry( cpid, nmS ); if (err) return err;

                                                          // access to source directory
        err= usrpath_open   ( cpid,&pathS, typeS, fromdir, 0x83 ); if (err) return err;
        a0 = (ulong) dvS;
        err= usrpath_getstat( cpid, pathS, SS_DevNm, &a0, NULL,NULL,NULL,NULL ); 
                                                                   if (err) return err;
                                                          
                                                          // access to dest   directory
        err= usrpath_open   ( cpid,&pathD, typeD, todir,   0x83 ); if (err) return err;
        a0 = (ulong) dvD;
        err= usrpath_getstat( cpid, pathD, SS_DevNm, &a0, NULL,NULL,NULL,NULL ); 
                                                                   if (err) return err;

        /* files must be on the same device */
        if (strcmp( dvS,dvD )) {
            err= usrpath_close( cpid, pathS );
            err= usrpath_close( cpid, pathD );
            return _errmsg( 1,"can't move from one device (/%s) to another (/%s)", dvS,dvD );
        }

        do {
            /* create the new dest file */
            err= get_locations( cpid,typeD, nmD,true, &asDirD, &fdD,&dfdD,&dcpD,&sctD ); 
                                                                    if (err) return err; 
            /* reopen in the same directory */
            if (dfdS==dfdD) {
                err= usrpath_close( cpid, pathS );                       if (err) return err;
                err= usrpath_open ( cpid,&pathS, typeS, fromdir, 0x83 ); if (err) return err;
            }

            /* the backwards directory connection must be done also */
            if (asDirS) {
                err= usrpath_open ( cpid,&pathX, typeS, nmS,0x83 ); if (err) break;
                err= usrpath_seek ( cpid, pathX, DIRNAMSZ );        if (err) break;
                                    len= sizeof(uint32_t); l= os9_long(dfdD/sctD);
                err= usrpath_write( cpid, pathX, &len, &l, false ); if (err) break;
                err= usrpath_seek ( cpid, pathX, 0 ); /*hld alloc*/ if (err) break;
                err= usrpath_close( cpid, pathX );                  if (err) break;
            }

        
            /* and write the fd positions vice versa */
            err= usrpath_seek ( cpid, pathS, dcpS+DIRNAMSZ );       if (err) break;
                                len= sizeof(uint32_t); l= os9_long(fdD/sctD);
            err= usrpath_write( cpid, pathS, &len, &l, false );     if (err) break;
            err= usrpath_seek ( cpid, pathS, 0 ); /* hold alloc */  if (err) break;

            if (dfdS==dfdD)    pathX= pathS; /* must be done in the same directory */
            else               pathX= pathD; /* directories are different */
            
            err= usrpath_seek ( cpid, pathX, dcpD+DIRNAMSZ );       if (err) break;
                                len= sizeof(uint32_t); l= os9_long(fdS/sctS);
            err= usrpath_write( cpid, pathX, &len, &l, false );     if (err) break;
            err= usrpath_seek ( cpid, pathX, 0 ); /* hold alloc */  if (err) break;
        } while (false);
        
        cer= usrpath_close    ( cpid, pathS ); if (!err) err= cer;
        cer= usrpath_close    ( cpid, pathD ); if (!err) err= cer;  if (err) return err;

        err= Flush_Entry      ( cpid,       nmD );                  if (err) return err;
        err= delete_file      ( cpid, fRBF, nmS, 0x01 );            if (err) return err;
    }
    else {
        #ifdef MACOS9
          /* for FSpCatMove, we need the destination directory's FSSpec */
          FSMakeFSSpec(destSpec.vRefNum,destSpec.parID,"\p",&destDirSpec);
          debugprintf(dbgUtils,dbgNorm,("# int_move: dest vol=%d, dir=%d, name=%#s\n",
                      destDirSpec.vRefNum,destDirSpec.parID,destDirSpec.name));
                    
          /* now source and destination are ok, do the move now */
              oserr=FSpCatMove(&sourceSpec,&destDirSpec);
          if (oserr) return _errmsg(host2os9err(oserr,E_FNA),"can't move file/dir \"%s/%s\". ",
                                      fromdir,fromname);
          if (toname[0]!=0) {
              /* destination must be renamed, too */
              debugprintf(dbgUtils,dbgNorm,("# int_move: renaming required, newname=%s\n",destname));
              strcpy(pD,destname);
              c2pstr(pD); /* new name */
              BlockMoveData(sourceSpec.name, destSpec.name, 32); /* update destination spec to reflect actually moved item */

                  oserr = FSpRename( &destSpec,pD );        
              if (oserr) {
                  return _errmsg(host2os9err(oserr,E_FNA),"moved ok, but can't rename to \"%s\". \n",destname);
              }
          }
        
        #elif defined win_unix
               err= AdjustPath( nameD, (char*)&adD, true ); nameD= (char*)&adD;
          if (!err) {
            for (i= 0; i<10; i++) {
              err= rename( nameS,nameD ); if (!err) break;
              DoWait();
            } // for
          } // if
            
          if  (err) return _errmsg( err, "can't open dest directory \"%s\".", nameD );
        #endif
    } // if
    
    if (!quiet)
        return _errmsg( 0,"\"%s/%s\" moved to \"%s/%s\"\n", fromdir,fromname, todir,destname);
    
    return 0;
} /* move_file */

/* main program */
os9err int_move( ushort cpid, int argc, char **argv )
{
  char *fromdir, *fromname, *todir, *toname;
  char **argp;
  os9err err;

  /* set default options */
  wild   = false;
  quiet  = false;
  exe_dir= false;
  dodir  = false;
  rewrite= false; 
   
  /* process options */
  err= options(argc,argv); if (err) return err==2 ? 0:err;
  if (nargc==0)
    return _errmsg( 1,"must specify files to be moved\n" );
    
  argp=&argv[1];
  /* check if single or wildcard move */
  if (wild) {
     /* wildcard move */
     while (nargc-->0) {
        while (*(fromdir=*(argp++))=='-') {}; /* get next non-option argument */
        if       (separate(&fromdir,&fromname)) return 1;
        err=move_file(cpid, fromdir, fromname, wilddir,""); if (err) return err;
     } /* while */
  }
  else {
     /* single file move */
     quiet=1; /* single file moves are always quiet */
     if (nargc>2)
       return _errmsg( 1,"must use -w option to move more than 1 file\n" );

     while  (*(fromdir=*(argp++))=='-') {}; /* get next non-option argument */
     if    (separate(&fromdir,&fromname)) return 1;

     if (nargc==2) {
        while (*(todir=*(argp++))=='-') {}; /* get next non-option argument */
        if (separate(&todir,  &toname  )) return 1;
     }
     else {
        toname="";
        todir=".";
     }
     
     return move_file( cpid, fromdir,fromname, todir,toname );
  } /* single move */
  return 0;
} /* main */

/* eof */
