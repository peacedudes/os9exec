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
 *    Revision 1.10  2006/02/19 15:46:07  bfo
 *    Header changed to 2006
 *
 *    Revision 1.9  2004/11/27 12:07:01  bfo
 *    _XXX_ introduced
 *
 *    Revision 1.8  2004/11/20 11:44:07  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.7  2004/10/22 22:51:12  bfo
 *    Most of the "pragma unused" eliminated
 *
 *    Revision 1.6  2003/05/17 10:26:09  bfo
 *    Include the "non-debug" flag (bit7 of path) for OS9_I_WritLn
 *
 *    Revision 1.5  2002/10/15 18:36:39  bfo
 *    Consider only lobyte at OS9_I_Delete
 *
 *    Revision 1.4  2002/10/02 19:16:43  bfo
 *    OS9_I_OpenCreate Bugfix for poCreateMask problem: consider only byte (d0.b)
 *
 *
 */



#include "os9exec_incl.h"



/* I$xxxx calls */
/* ============ */


static os9err OS9_I_OpenCreate( regs_type *rp, ushort cpid, Boolean cre )
/* common routine for both I$Open and I$Create, for internal use only */
{
    os9err    err;
    ushort    mode    = loword(rp->d[0]) & 0x00ff; /* consider only byte => bugfix for the */
    char*     os9_name= (char*)FROM68K(rp->a[0]);  /* poCreateMask problem */
    char      os9_path[OS9PATHLEN]; 
    char*     pastpath;
    
    ushort    xmode =  mode | ( cre ? poCreateMask:0 ); /* add the create flag */
    ushort    path;
    uint32_t  size= 0; /* the default value, if size bit is not set */
    ptype_typ type;
    char      pipename[OS9NAMELEN];
    char*     co= cre ? "I$Create":"I$Open";
    

    pastpath= nullterm( os9_path,os9_name,OS9PATHLEN );
    
    if (xmode & 0x20) size= rp->d[2]; /* initial file size, if size bit is set */

    procs[cpid].fileAtt     = loword(rp->d[1]);
    procs[cpid].cre_initsize= size;

    type= IO_Type( cpid,os9_path, mode );
    debugprintf( dbgFiles,dbgNorm,("# %s: trying file '%s', accessmode=$%04X / %s\n",
                                      co, os9_path,mode, TypeStr(type) ));

    /* adapt pipe names */
    if (type==fPipe) {
        switch (os9_path[5]) {
            case NUL: /* unnamed pipe or directory of named pipes */
                      if (mode==0x00) return E_FNA; /* force 2nd open for pipe dir */
                    //if (mode & 0x80) return E_FNA; /* pipe directory / %%% to be implemented */
                      strcpy( os9_path,"" );
                      break;
                          
            case '/': /* named pipe */
                      nullterm( pipename,&os9_path[ 6 ],OS9NAMELEN );
                      strcpy( os9_path,pipename );
                      break;
                          
            default:  return E_BNAM;
        } /* switch */
    }

    err= usrpath_open( cpid,&path,type, os9_path,xmode ); if (err) return err;
    
    retword(rp->d[0])=        path;     /* return path number */
    rp->a[0]         = TO68K(pastpath); /* return updated pathname pointer */
    debugprintf( dbgFiles,dbgNorm,("# %s successful, path number= %d\n", co,path ));
    return 0;
} /* OS9_I_OpenCreate */



os9err OS9_I_Open( regs_type *rp, ushort cpid )
/* I$Open:
 * Input:   d0.b=access mode (D S - - - E W R)
 *          (a0)=pathname pointer
 * Output:  d0.w=path number
 * Error:   Carry set
 *          d1.w = error
 *             E$PthFul:   cannot open more files
 *             E$BPNam:    bad path name
 *             E$Bmode:    file is not accessible in this mode
 *             E$PNNF:     file not found
 * Restrictions:- path names are parsed MPW-like, except that / characters are
 *                also accepted as path element delimiters. Spaces cannot be
 *                part of names used by OS9exec, they are treated as path
 *                termination characters.
 *              - Directory open is not possible
 *              - Error reporting is rudimentary
 */
{   return OS9_I_OpenCreate( rp,cpid, false );
} /* OS9_I_Open */



os9err OS9_I_Create( regs_type *rp, ushort cpid )
/* I$Create:
 * Input:   d0.b=access mode (- S I - - E W R)
 *          d1.b=file attributes
 *          d2.l=initial size (if I bit is set)
 *          (a0)=pathname pointer
 * Output:  d0.w=path number
 * Error:   Carry set
 *          d1.w = error
 *             E$PthFul:   cannot open more files
 *             E$BPNam:    bad path name
 *             E$FNA:      file is not accessible in this mode
 *             E$PNNF:     file not found
 * Restrictions:- path names are parsed MPW-like, except that / characters are
 *                also accepted as path element delimiters. Spaces cannot be
 *                part of names used by OS9exec, they are treated as path
 *                termination characters.
 *              - File attributes are not used
 *              - Error reporting is rudimentary
 */
{   return OS9_I_OpenCreate( rp,cpid, true );
} /* OS9_I_Create */




os9err OS9_I_Delete( regs_type *rp, ushort cpid )
/* I$Delete:
 * Input:   d0.b=access mode (E W R)
 *          (a0)=pathname pointer
 * Output:  (a0)=updated past pathlist
 * Error:   Carry set
 *          d1.w = error
 *             E$FNA:      cannot delete
 *             E$PNNF:     file not found
 *             E$SHARE:    directory file not empty
 * Restrictions:- path names are parsed MPW-like, except that / characters are
 *                also accepted as path element delimiters. Spaces cannot be
 *                part of names used by OS9exec, they are treated as path
 *                termination characters.
 */
{
    ushort    mode    = lobyte(rp->d[0]); /* take do.b, avoid poCreate bug problem */
    char*     os9_name= (char*)FROM68K(rp->a[0]);
    char      os9_path[OS9PATHLEN];
//  char*     pastpath;
    ptype_typ type;


    if (os9_name==NULL)                     os9_name= "";
    /*pastpath=*/ nullterm(         os9_path,os9_name, OS9PATHLEN );
    type    =  IO_Type( cpid,      os9_path,mode ); if (type==fNone) return E_BPNAM;
    return delete_file( cpid,type, os9_path,mode );
} /* OS9_I_Delete */



os9err OS9_I_MakDir( regs_type *rp, ushort cpid )
/* I$MakDir:
 * Input:   d0.b=access mode (D - I - - E W R)
 *          d1.b=file attributes
 *          d2.l=initial size (if I bit is set)
 *          (a0)=pathname pointer
 * Output:  (a0)=updated past pathlist
 * Error:   Carry set
 *          d1.w = error
 *          E$CEF: directory already exists
 *
 * Restrictions:- path names are parsed MPW-like, except that / characters are
 *                also accepted as path element delimiters. Spaces cannot be
 *                part of names used by OS9exec, they are treated as path
 *                termination characters.
 *              - Access mode is not used
 *              - File attributes are not used
 */
{
    ushort    mode    = loword(rp->d[0]) | poCreateMask; /* internal open used */
    char*     os9_name= (char*)FROM68K(rp->a[0]);
    char      os9_path[OS9PATHLEN];
//  char*     pastpath;
    ptype_typ type;


    /*pastpath=*/ nullterm(        os9_path,os9_name, OS9PATHLEN );
    type    =  IO_Type( cpid,      os9_path,mode ); if (type==fNone) return E_BPNAM;
    return    make_dir( cpid,type, os9_path,mode );
} /* OS9_I_MakDir */



os9err OS9_I_ChgDir( regs_type *rp, ushort cpid )
/* I$ChgDir:
 * Input:   d0.b=access mode (E W R)
 *          (a0)=pathname pointer
 * Output:  (a0)=updated past pathlist
 * Error:   Carry set
 *          d1.w = error
 *             E$PNNF:      directory not found
 *
 * Restrictions:- path names are parsed MPW-like, except that / characters are
 *                also accepted as path element delimiters. Spaces cannot be
 *                part of names used by OS9exec, they are treated as path
 *                termination characters.
 *              - Access mode is only used to ignore "chx"-requests
 */
{
    ushort    mode    = (loword(rp->d[0]) & 0x07) | 0x80; /* ignore some flags */
    char*     os9_name= (char*)FROM68K(rp->a[0]);
    char      os9_path[OS9PATHLEN];
//  char*     pastpath;
    ptype_typ type;


    if (os9_name==NULL)                     os9_name= ""; /* avoid problems without $HOME */
    /*pastpath=*/ nullterm(        os9_path,os9_name, OS9PATHLEN );
    type=      IO_Type( cpid,      os9_path,mode ); if (type==fNone) return E_BPNAM;
    return  change_dir( cpid,type, os9_path,mode );
} /* OS9_I_ChgDir */



os9err OS9_I_Close( regs_type *rp, ushort cpid )
/* I$Close:
 * Input:   d0.w=path number
 * Output:  none
 * Error:   Carry set
 *          d1.w = error
 *             E$BPNum:    bad path number
 * Restrictions:- Standard paths (0..2) cannot be closed
 *              - Error reporting is rudimentary
 */
{
    ushort path= loword(rp->d[0]);
    debugprintf(dbgFiles,dbgNorm,("# I$Close: close path number = %d\n",path));
    return usrpath_close( cpid,path );
} /* OS9_I_Close */  



os9err OS9_I_WritLn( regs_type *rp, ushort cpid )
/* I$WritLn:
 * Input:   d0.w=path number
 *          d1.l=maximum number of bytes to write
 *          (a0)=address of buffer
 * Output:  d1.l=number of bytes actually written
 * Error:   Carry set
 *          d1.w = error
 *             E$BPNUM: this path is not open
 *             E$BMODE: file not open for write
 * Restrictions:- path numbers 1-2 are used as output to the STDOUT and STDERR of MPW,
 *              - path numbers>2 are always disk files
 *              - Output to stderr/stdout may be written to the wrong path (stderr instead of
 *                stdout or vv.) if calls do not output an entire line at a time due to the
 *                (single) buffering in stdwrite. The entire line will always be output to the
 *                path to which the CR character is written.
 */
{
    ushort   path;
    uint32_t cnt;
    uint32_t ii;
    char     *buff;
    os9err   err;

    if (debugcheck(dbgWarnings,dbgDetail)) {
        regcheck(cpid,"I$WriteLn buffer start",rp->a[0],           RCHK_DRU+RCHK_ARU+RCHK_MEM+RCHK_MOD);
        regcheck(cpid,"I$WriteLn buffer end",  rp->a[0]+rp->d[1]-1,RCHK_DRU+RCHK_ARU+RCHK_MEM+RCHK_MOD);
    }

    path= loword(rp->d[0]);
    cnt =        rp->d[1];
    buff= (char*)FROM68K(rp->a[0]);

    path= path & 0x7f; /* mask the specific OS9exec non-debug flag */
   
    /* search if there is a CR char before end of the buffer */
    /* if yes, terminate the string a little bit earlier     */
           ii= 0;
    while (ii<cnt) {
        if (buff[ii]==CR) cnt= ii+1;
        ii++;
    }
    
    /* now write */
    err= usrpath_write(cpid,path, &cnt,buff, true);
    rp->d[ 1 ]= cnt;
    return err;
} /* OS9_I_WritLn */



os9err OS9_I_Write( regs_type *rp, ushort cpid )
/* I$Write:
 * Input:   d0.w=path number
 *          d1.l=maximum number of bytes to write
 *          (a0)=address of buffer
 * Output:  d1.l=number of bytes actually written
 * Error:   Carry set
 *          d1.w = error
 *             E$BPNUM: this path is not open
 *             E$BMODE: file not open for write
 * Restrictions:- path numbers 1-2 are used as output to the STDOUT/STDERR of MPW,
 *              - path numbers>2 are always disk files
 *              - Output to stderr/stdout may be written to the wrong path (stderr instead of
 *                stdout or vv.) if calls do not output an entire line at a time due to the
 *                (single) buffering in stdwrite. The entire line will always be output to the
 *                path to which the CR character is written.
 */
{
    ushort   path;
    uint32_t cnt;
    char     *buff;
    os9err err;

    if (debugcheck(dbgWarnings,dbgDetail)) {
        regcheck(cpid,"I$Write buffer start",rp->a[0],           RCHK_DRU+RCHK_ARU+RCHK_MEM+RCHK_MOD);
        regcheck(cpid,"I$Write buffer end",  rp->a[0]+rp->d[1]-1,RCHK_DRU+RCHK_ARU+RCHK_MEM+RCHK_MOD);
    }

    path= loword(rp->d[0]);
    cnt =        rp->d[1];
    buff= (char*)FROM68K(rp->a[0]);

    /* now write */
    err= usrpath_write(cpid,path, &cnt,buff, false);
    rp->d[ 1 ]= cnt;
    return err;
} /* OS9_I_Write */



os9err OS9_I_ReadLn( regs_type *rp, ushort cpid )
/* I$ReadLn:
 * Input:   d0.w=path number
 *          d1.l=maximum number of bytes to read
 *          (a0)=address of buffer
 * Output:  d1.l=number of bytes actually read
 * Error:   Carry set
 *          d1.w = error
 *             E$BPNUM: this path is not open
 * Restrictions:- path number 0 is used to input from MPW. I$ReadLn on this path causes
 *                an extra CR to be written to MPW stdout to ensure the input is on an empty
 *                line.
 *              - path #1 and #2 cannot be read
 *              - path numbers>2 are always disk files
 *              - NULL chars cannot be read from stdin using I$ReadLn 
 */
{
    char     *p;
    uint32_t  cnt;
    ushort    path;
    os9err    err;

    if (debugcheck(dbgWarnings,dbgDetail)) {
        regcheck(cpid,"I$ReadLn buffer start",rp->a[0],           RCHK_DRU+RCHK_ARU+RCHK_MEM);
        regcheck(cpid,"I$ReadLn buffer end",  rp->a[0]+rp->d[1]-1,RCHK_DRU+RCHK_ARU+RCHK_MEM);
    }

    p=(char *)  FROM68K(rp->a[0]);
    path=loword(rp->d[0]);
    cnt =       rp->d[1];

    err= usrpath_read(cpid,path,&cnt,p,true); if (err) return err;
    rp->d[1]= cnt; /* return # of chars actually read */
    return 0;
} /* OS9_I_ReadLn */



os9err OS9_I_Read( regs_type *rp, ushort cpid )
/* I$Read:
 * Input:   d0.w=path number
 *          d1.l=maximum number of bytes to read
 *          (a0)=address of buffer
 * Output:  d1.l=number of bytes actually read
 * Error:   Carry set
 *          d1.w = error
 *             E$BPNUM: this path is not open
 *             E$READ: attempt to read from path 0..2
 */
{
   char     *p;
   ushort    path;
   uint32_t  cnt;
   os9err    err;
   
   if (debugcheck(dbgWarnings,dbgDetail)) {
       regcheck(cpid,"I$Read buffer start",rp->a[0],           RCHK_DRU+RCHK_ARU+RCHK_MEM);
       regcheck(cpid,"I$Read buffer end",  rp->a[0]+rp->d[1]-1,RCHK_DRU+RCHK_ARU+RCHK_MEM);
   }

   p=(char *)  FROM68K(rp->a[0]);
   path=loword(rp->d[0]);
   cnt =       rp->d[1];

   err=usrpath_read(cpid,path,&cnt,p,false); if (err) return err;
   rp->d[1]= cnt; /* return # of chars actually read */
   return 0;
} /* OS9_I_Read */



os9err OS9_I_Seek( regs_type *rp, ushort cpid )
/* I$Seek:
 * Input:   d0.w=path number
 *          d1.l=new file position
 * Output:  none
 * Error:   Carry set
 *          d1.w = error
 *             E$BPNUM: this path is not open 
 */
{
    os9err   err;
    ushort   path= loword(rp->d[0]);
    uint32_t pos =        rp->d[1];

    err= usrpath_seek( cpid,path, pos );
    debugprintf(dbgFiles,dbgDetail,("# I$Seek: path=%d: New position=$%lX, err=%d\n",
                                       path,pos, err ));
    return err;
} /* OS9_I_Seek */



os9err OS9_I_SetStt( regs_type *rp, ushort cpid )
/* I$SetStt:
 * Input:   d0.w=path number
 *          d1.w=function code
 *          Others=function code dependent
 * Output:  function code dependent
 * Error:   Carry set
 *          d1.w = error
 *             E$UNKSVC: this function code does not exist
 *             E$BPNUM: this path is not open 
 * Restrictions:- 
 */
{
    ulong    a0= (ulong)FROM68K(rp->a[0]); /* register addresses -> host pointers */
    ulong    a1= (ulong)FROM68K(rp->a[1]);
    uint32_t d0= rp->d[0], d1= rp->d[1], d2= rp->d[2], d3= rp->d[3];

    ushort path= loword(d0);
    ushort func= loword(d1);

    os9err err= usrpath_setstat( cpid,path,func, &a0,&a1, &d0,&d1,&d2,&d3 );
    rp->d[0]= d0; rp->d[1]= d1; rp->d[2]= d2; rp->d[3]= d3; /* copy back results */
    return err;
} /* OS9_I_SetStt */

                              

os9err OS9_I_GetStt( regs_type *rp, ushort cpid )
/* I$GetStt:
 * Input:   d0.w=path number
 *          d1.w=function code
 *          Others=function code dependent
 * Output:  function code dependent
 * Error:   Carry set
 *          d1.w = error
 *             E$UNKSVC: this function code does not exist
 *             E$BPNUM: this path is not open 
 */
{
    ulong    a0= (ulong)FROM68K(rp->a[0]); /* register address -> host pointer */
    uint32_t d0= rp->d[0], d1= rp->d[1], d2= rp->d[2], d3= rp->d[3];

    ushort path= loword(d0);
    ushort func= loword(d1);

    /* perform getstat */
    os9err err= usrpath_getstat( cpid,path,func, &a0, &d0,&d1,&d2,&d3 );
    rp->d[0]= d0; rp->d[1]= d1; rp->d[2]= d2; rp->d[3]= d3; /* copy back results */
    return err;
} /* OS9_I_GetStt */



os9err OS9_I_SGetSt( regs_type *rp, ushort cpid )
/* I$SGetSt:
 * Input:   d0.w=path number
 *          d1.w=function code
 *          Others=function code dependent
 * Output:  function code dependent
 * Error:   Carry set
 *          d1.w = error
 *             E$UNKSVC: this function code does not exist
 *             E$BPNUM: this path is not open 
 */
{
    ulong    a0= (ulong)FROM68K(rp->a[0]); /* register address -> host pointer */
    uint32_t d0= rp->d[0], d1= rp->d[1], d2= rp->d[2], d3= rp->d[3];

    ushort path= loword(d0);
    ushort func= loword(d1);

    /* perform getstat */
    os9err err= syspath_getstat( cpid,path,func, &a0, &d0,&d1,&d2,&d3 );
    rp->d[0]= d0; rp->d[1]= d1; rp->d[2]= d2; rp->d[3]= d3; /* copy back results */
    return err;
} /* OS9_I_SGetSt */



os9err OS9_I_Dup( regs_type *rp, ushort cpid )
/* I$Dup:
 * Input:   d0.w=path to be duplicated
 * Output:  d0.w=new path number for same path
 * Error:   Carry set
 *          d1.w = error (E_PTHFUL, E_BPNUM)
 */
{
    ushort  k, up,sp;
    ushort* spN;
    
        up= loword(rp->d[0]);
    if (up>=MAXUSRPATHS)          return os9error(E_BPNUM);
    
        sp= procs[cpid].usrpaths[up];
    if (sp>=MAXSYSPATHS || sp==0) return os9error(E_BPNUM);

    
    /* --- find another free entry */
    for (k=0; k<MAXUSRPATHS; k++) {
             spN= &procs[cpid].usrpaths[k];
        if (*spN==0 || *spN>=MAXSYSPATHS) {  /* --- free entry, try to open a syspath */
            *spN= sp;                        /* copy syspath number */
            
            usrpath_link( cpid,k,"dup  " );               /* increase link */
            retword(rp->d[0])= k;  /* return new path number for same path */

            debugprintf(dbgFiles,dbgDetail,("# I$Dup: created alias=%d for original path=%d\n",
                                               k,up));
            return 0;
        }
    }
    
    return os9error(E_PTHFUL); /* no more user paths */
} /* OS9_I_Dup */



os9err OS9_I_Attach( regs_type *rp, _pid_ )
/* I$Attach:
 * Input:   d0.b=Access mode
 *          (a0)=device name
 * Output:  (a2)=pointer to device table entry
 * Error:   Carry set
 *          d1.w = error
 * Restrictions:- Is only a dummy. Always returns 0xFF00FF00 for the device table entry
 */
{
	char* name= (char*) FROM68K(rp->a[0]);

	if (ustrcmp( name,"/L2" )==0) {
		rp->a[2]= 0x22002200; /* %%% /L2 identifier */
		return 0;
	} /* if */

    debugprintf((dbgPartial+dbgFiles),dbgNorm,("# I$Attach, simply returned dummy pointer\n"));
    rp->a[2]= 0xFF00FF00; /* dummy return pointer */
    return 0;
} /* OS9_I_Attach */



os9err OS9_I_Detach( regs_type *rp, _pid_ )
/* I$Detach:
 * Input:   (a2)=pointer to device table entry
 * Output:  None
 * Error:   Carry set
 *          d1.w = error
 *           E$Damage : bad device table pointer
 * Restrictions:- As I$Attach alwayst returns 0xFF00FF00, this value is expected as
 *                input to any I$Detach call.
 */
{
    /* special handling for "/L2" */
    if (rp->a[2]==0x22002200) {
    	init_L2(); /* switch off /L2 completely */
	    return 0;
    } /* if */

    debugprintf((dbgPartial+dbgFiles),dbgNorm,("# I$Detach, expects dummy pointer\n"));
    if (rp->a[2]!=0xFF00FF00) return os9error(E_DAMAGE); /* bad dummy pointer value */
    
    return 0;
} /* OS9_I_Detach */


/* eof */

