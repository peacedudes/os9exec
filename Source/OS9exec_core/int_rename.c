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
 *    Revision 1.14  2007/01/04 20:39:19  bfo
 *    Rearranged (unused) vars
 *
 *    Revision 1.13  2006/02/19 15:43:15  bfo
 *    Header changed to 2006
 *
 *    Revision 1.12  2005/06/30 11:42:08  bfo
 *    Mach-O support
 *
 *    Revision 1.11  2005/01/22 16:10:07  bfo
 *    Renamed to ifdef MACOS9
 *
 *    Revision 1.10  2004/12/04 00:00:16  bfo
 *    MacOSX MACH adaptions
 *
 *    Revision 1.9  2004/11/27 12:07:42  bfo
 *    _XXX_ introduced
 *
 *    Revision 1.8  2004/11/20 11:44:07  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.7  2004/10/22 22:51:12  bfo
 *    Most of the "pragma unused" eliminated
 *
 *    Revision 1.6  2003/07/31 14:42:02  bfo
 *    "get_locations": Additional parameter for sectorsize
 *
 *    Revision 1.5  2003/05/17 21:36:31  bfo
 *    rename to the same file name on RBF is no longer possible now
 *
 *    Revision 1.4  2002/10/09 20:46:14  bfo
 *    uphe_printf => upe_printf
 *
 *
 */

/* OS9exec/nt internal rename */
/* ========================== */

/* global includes */
#include "os9exec_incl.h"
#include <ctype.h>

static void usage( char* name, _pid_ )
{
    upe_printf( "Syntax:   %s [<opts>] <path> <name> [<opts>]\n", name );
    upe_printf( "Function: rename a file or directory\n" );
    upe_printf( "Options:\n" );
    upe_printf( "     -x       path starts from execution dir\n" );
} /* usage */

/*
static os9err Flush_Dir( ushort cpid, ushort* pathP, const char* nmS )
{
  os9err          err;
  os9direntry_typ d;
  size_t          dir_size;
  syspath_typ*    spP;
  char            fullName[OS9PATHLEN];
  int             oLen;
  dirtable_entry* mP= NULL;
  ulong           fd_hash;
    
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
      
        if (mP->dirid!=0 &&
            mP->dirid!=d.fdsect)
          main_printf( "name='%s' fdsect=%06X %06X\n", fullName, d.fdsect, mP->dirid );

        mP->dirid= 0;
      } // if
    } // while
    
    if (err==E_EOF) err= 0;
  } while (false);

  return err;
} // Flush_Dir
*/

/* internal "rename" command for OS9exec/nt */
os9err int_rename( ushort cpid, int argc, char **argv )
{
    #ifdef MACOS9
      /* special mac includes */
      #include <Finder.h>
      OSErr  oserr= 0;
      FSSpec theFSSpec;
    #endif

    char *p, *sv;
    int nargc=0, h;
    #define     MAXARGS 2
    char *nargv[MAXARGS];
    
    os9err     err, cer;
    int        exedir= 0;
    ushort     path;
    ptype_typ  type;
    char       *nameP, *qq;
    char       nmS     [OS9PATHLEN];
    char       nmD     [OS9PATHLEN];
    char       newName [OS9PATHLEN];
    uint32_t   fd, dfd, dcp, dcpD, sSct, len;
    Boolean    asDir;
    char       oldPath [OS9PATHLEN];

  //syspath_typ*    spP;
  //dirtable_entry* mP= NULL;
  //ulong           fd_hash;

    #if defined win_unix
      char*    pp;
      char     newPath [OS9PATHLEN]; 
      char     adaptOld[OS9PATHLEN];
      char     adaptNew[OS9PATHLEN];
    #endif   
    
    debugprintf(dbgUtils,dbgNorm,( "# rename\n" ));
    
    /* get arguments and options */
    for (h=1; h<argc; h++) {
        p= argv[h];    
        if (*p=='-') { 
            p++;
            switch (tolower((unsigned char)*p)) {
                case '?' :  usage(argv[0],cpid); return 0;
                case 'x' :  exedir= 1; break;
                default  :  upe_printf("Error: unknown option '%c'!\n",*p); 
                            usage(argv[0],cpid); return 1;
            }   
        }
        else {
            if (nargc>=MAXARGS) { 
                upe_printf("Error: no more than %d arguments allowed\n",MAXARGS); return 1;
            }
            nargv[nargc++]= argv[h];
        }
    }   
        
    if (nargc!=2) {
        upe_printf("Error: two arguments required\n"); return 1;
    }

    /* first get file/dir to be renamed */
               nameP= nargv[0];
    strcpy(nmS,nameP);
    
    err= parsepath( cpid, &nameP,oldPath, exedir ); if (err) return err;
    
    /* ... and new name */
             strcpy( newName, nargv[1] );
        qq=  strstr( newName,PSEP_STR ); /* no path delimiters allowed */
    if (qq!=NULL) return _errmsg( E_BPNAM,"can't rename to \"%s\"",newName);

        len= strlen( newName );
    if (len>DIRNAMSZ)
                  return _errmsg( E_BPNAM,"name too long \"%s\"", newName );

        type= IO_Type( cpid,nmS, 0x00 ); /* get the device type: Mac/PC or RBF */
    if (type==fRBF) {
        /* do it the same way as the OS-9 rename */
            err= get_locations( cpid,type, nmS,false, &asDir, &fd,&dfd,&dcp,&sSct );
        if (err) return err;
        
        // cache flush of this file
        err= Flush_Entry( cpid, nmS );
        
        /*
                 err= usrpath_open( cpid, &path, type, nmS, 0x01 );
        if (err) err= Flush_Dir   ( cpid, &path,       nmS       );
        
        spP= get_syspath( cpid, procs[ cpid ].usrpaths[ path ] ); // get spP for fd sects
             err= FD_ID( spP->fullName, NULL, &fd_hash, &mP ); 
        if (!err) mP->dirid= 0;
        cer= usrpath_close( cpid, path ); if (!err) err= cer;
        */
        
        p = (char*)&nmS;
        sv= p; sv--;
        
        while  (*p!=NUL) {
            if (*p==PSEP) sv= p; /* search for the last one */
            p++;
        }
        
        if      (sv==nmS) return E_FNA; /* this is the root path */
        if      (sv >nmS) *sv= NUL;     /* cut the file name at the end */
        else strcpy( nmS, "." );        /* or take the current path */
        
        strcpy( nmD,nmS );
        strcat( nmD,PSEP_STR );
        strcat( nmD,newName );
        	 err= get_locations  ( cpid,type, nmD,false, &asDir, &fd,&dfd,&dcpD,&sSct );
        if (!err && dcp!=dcpD) return E_CEF;   /* file already exists with different name ? */
                                     /* case sensitive changes of the same name are allowed */
        
        err=   usrpath_open ( cpid,&path, type, nmS,     0x83);   if (err) return err;
        
        do {
          err= usrpath_seek ( cpid, path, dcp );                  if (err) break;
          LastCh_Bit7( newName, true );    len++; // write also NUL terminator
          err= usrpath_write( cpid, path, &len, newName, false ); if (err) break;
        } while (false);
        
        cer=   usrpath_close( cpid, path ); if (!err) err= cer;
    }
    else {
        #if defined MACOS9
          /* now do the real stuff */
          /* first make FSSpec for file/dir to be renamed */
              err= getFSSpec( cpid,oldPath, exedir ?_exe:_data, &theFSSpec ); 
          if (err) return err;
    
          /* now rename it */       c2pstr(newName);
              oserr= FSpRename( &theFSSpec,newName);
          if (oserr==dupFNErr) _errmsg(0,"can't rename to \"%#s\"",newName);
          err= host2os9err(oserr,E_PNNF);
    
        #elif defined win_unix
                           pp=  oldPath;
          err= AdjustPath( pp, adaptOld, false ); 
          pp=                  adaptOld;
          if (err) return _errmsg( err,"can't open \"%s\"",oldPath );
          
          strcpy   ( newPath,pp );
          qq=        newPath+strlen(newPath)-1;
          while  (qq>newPath && *qq!=PATHDELIM) qq--; /* cut the existing file name */
          *++qq= NUL;
          strcat  ( newPath,newName );
          
                           qq=  newPath;
          err= AdjustPath( qq, adaptNew, false ); 
          if (!err) return _errmsg( E_CEF,"can't rename to \"%s\"",newName );
        
          err= host2os9err( rename( pp,qq ),E_CEF );
          debugprintf(dbgUtils,dbgNorm,( "# rename from: '%s'\n",        pp      ));
          debugprintf(dbgUtils,dbgNorm,( "# rename to:   '%s' err=%d\n", qq, err ));
          if  (err) return _errmsg( err,"can't rename to \"%s\"",newName );
        #endif
    }
    
    return err;
} /* int_rename */

/* eof */
