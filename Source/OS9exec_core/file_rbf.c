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
 *    Revision 1.56  2007/04/17 23:12:36  bfo
 *    FD Cache things added (not yet active)
 *
 *    Revision 1.55  2007/01/07 13:53:23  bfo
 *    Up to date
 *
 *    Revision 1.54  2007/01/04 20:37:22  bfo
 *    Some unused vars eliminated
 *    Boolean return type for 'IsSCSI'
 *    FD_Segment: break loop 'if (scs==0) break;'
 *    DoAccess: Loop until enough sectors allocated ('scsi.r' problem)
 *
 *    Revision 1.53  2006/12/17 00:47:23  bfo
 *    devCopy raw device opening / copy length is OS9PATHLEN
 *
 *    Revision 1.52  2006/12/01 20:03:19  bfo
 *    "MountDev" with <devCopy> support ( -d )
 *
 *    Revision 1.51  2006/11/18 10:07:12  bfo
 *    PrepareRAM adapted:
 *    - Offset to sector 1 is now dev->sctSize, no longer A_Base 0x100
 *    - MaxKB introduced and checked
 *    - <cluSize> must be a power of 2 now
 *    - dev->sas= DD_MINALLOC
 *    - <sctSize>, <cluSize> + root sector calculations correcty
 *    - Params -n and -c added for "mount" command
 *    - bb/coff calculation corrected (for wMode || mlt)
 *
 *    Revision 1.50  2006/11/12 13:30:29  bfo
 *    "ReadFD" visible from outside
 *
 *    Revision 1.49  2006/07/29 08:49:54  bfo
 *    "adaptor" => "adapter"
 *
 *    Revision 1.48  2006/06/13 22:18:47  bfo
 *    SS_EOF implementation added for RBF file manager
 *
 *    Revision 1.47  2006/06/08 08:15:04  bfo
 *    Eliminate causes of signedness warnings with gcc 4.0
 *
 *    Revision 1.46  2006/05/16 13:07:34  bfo
 *    scs>1 bug fixed
 *
 *    Revision 1.45  2006/02/19 16:25:26  bfo
 *    printf commented out / reformatted
 *
 *    Revision 1.44  2005/06/30 11:35:41  bfo
 *    Mount reduced AND full / Mach-O support
 *
 *    Revision 1.43  2005/05/13 17:21:48  bfo
 *    mount -I is supported now
 *
 *    Revision 1.42  2005/04/15 11:13:04  bfo
 *    Reduced size of RBF images is supported now
 *
 *    Revision 1.41  2004/12/03 23:54:51  bfo
 *    MacOSX MACH adaptions
 *
 *    Revision 1.40  2004/11/27 12:04:28  bfo
 *    _XXX_ introduced
 *
 *    Revision 1.39  2004/11/20 11:44:07  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.38  2004/10/22 22:51:11  bfo
 *    Most of the "pragma unused" eliminated
 *
 *    Revision 1.37  2004/01/04 02:11:00  bfo
 *    Assign rbf->lastPos & rbf->att also in case of root dir
 *    ("rename" directory cut bug)
 *
 *    Revision 1.36  2003/08/01 11:16:28  bfo
 *    do not create file w/o existing subdir
 *
 *    Revision 1.35  2003/07/31 14:38:56  bfo
 *    Change calculation of PD_FD/PD_DFD (sector size instead of 256)
 *
 *    Revision 1.34  2003/05/05 17:55:07  bfo
 *    Activate most of the ramDisk things even without RAM_SUPPORT
 *
 *    Revision 1.33  2002/10/27 23:31:02  bfo
 *    ReleaseBuffers at pRclose done in every case
 *    get_mem/release_mem without param <mac_asHandle>
 *
 *    Revision 1.32  2002/10/15 18:38:23  bfo
 *    Consider only lobyte at OS9_I_Delete
 *
 *    Revision 1.31  2002/10/09 20:41:16  bfo
 *    uphe_printf => upe_printf
 *
 *    Revision 1.30  2002/10/02 19:21:37  bfo
 *    Correct handling for "mount" without parameters
 *
 *    Revision 1.29  2002/09/21 20:02:52  bfo
 *    E_DIDC problem fixed.
 *
 *    Revision 1.28  2002/09/19 22:00:18  bfo
 *    Disabling "r0" more specific now
 *
 *    Revision 1.27  2002/09/17 00:24:09  bfo
 *    /r0 is disabled as name, if RAM_SUPPORT is disabled.
 *
 *    Revision 1.26  2002/09/14 23:13:38  bfo
 *    Relative paths can be mounted (again).
 *
 *    Revision 1.25  2002/09/11 17:05:55  bfo
 *    Bug at incomplete sector of multisector write fixed
 *
 *
 */

// #define RBF_CACHE

/* This file contains the RBF Emulator */
#include "os9exec_incl.h"
#include <ctype.h>
#include <sys/stat.h>
#ifdef MINGW
  #include <windows.h>      /* GetDiskFreeSpaceExA, for HostDirLSN0 */
#else
  #include <sys/statvfs.h>  /* statvfs, for HostDirLSN0 */
#endif
#include "filescsi.h"
#include <ctype.h>

#define  FD_Header_Size 16 // size of FD header
#define  SegSize         5 // number of bytes per segment

/* the RBF device entry itself */			
typedef struct {
  /* common for all types */
  char    name [OS9NAMELEN];	 /* device         name */
  char    name2[OS9NAMELEN];	 /* device 2nd     name */
  char    name3[OS9NAMELEN];	 /* device 3rd     name %%% dirty solution */
  char    alias[OS9PATHLEN];	 /* device's alias name */

  int      nr;					 /* own reference number (array index) */
  uint32_t sctSize;			     /* sector size for this device */
  ushort   mapSize;              /* size of allocation map */
  byte     pdtyp;                /* device type: hard disk, floppy */
  ushort   sas;				     /* sector allocation size */
  uint32_t root_fd_nr;           /* sector nr of root fd */
  uint32_t clusterSize;          /* cluster size (allocation) */
  uint32_t totScts;              /* total   number of sectors */
  uint32_t imgScts;              /* current number of sectors at this image */
  ushort   last_diskID;          /* last disk ID, inherited by new paths */
  uint32_t last_alloc;           /* the last allocation was here */
  uint32_t currPos;			     /* current position at image */
  ulong    rMiss, rTot,		     /* device statistics */
           wMiss, wTot;
  ushort   sp_img;			     /* syspath number of image file */
  char	   img_name[OS9PATHLEN]; /* full path name of image file */
	
  byte*    tmp_sct; 			 /* temporary buffer sector */
  Boolean  wProtected;			 /* true, if write  protected */
  Boolean  fProtected;			 /* true, if format protected */
  Boolean  imgMode;              /* true, if reduced img size is allowed/supported */
  Boolean  multiSct;             /* true, if multi sector support */
			
  Boolean  isRAM;                /* true, if RAM disk */
  byte*    ramBase;              /* start address of RAM disk */
      
  scsi_dev scsi;                 /* SCSI device variables */
  Boolean  installed;            /* true, if device is already installed */
} rbfdev_typ;

/* the RBF devices */
rbfdev_typ  rbfdev[MAXRBFDEV];		

/* OS9exec builtin module, defined as constant array */
const byte RAM_zero[] = {
    0x00,0x20,0x00,0x00,0x04,0x00,0x00,0x01,0x00,0x00,0x05,0x00,0x00,0xbf,0x00,0x00,  // . ...........?..
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x07,0x03,0x14,0x2d,0x52,  // ..............-R
    0x61,0x6d,0x20,0x44,0x69,0x73,0x6b,0x20,0x28,0x43,0x61,0x75,0x74,0x69,0x6f,0x6e,  // am Disk (Caution
    0x3a,0x20,0x56,0x6f,0x6c,0x61,0x74,0x69,0x6c,0x65,0xa9,0x00,0x00,0x00,0x00,0x00,  // : Volatile).....
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // ................
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // ................
    0x43,0x72,0x75,0x7a,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x01,0x00,0x00,0x00,0x00,  // Cruz............
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // ................
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // ................
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // ................
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // ................
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // ................
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // ................
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // ................
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // ................
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00   // ................
};    

/* --- local procedure definitions for object definition ------------------- */
void   init_RBF  ( fmgr_typ* f );

os9err pRopen    ( ushort pid, syspath_typ*, ushort *modeP,   const char* pathname );
os9err pRclose   ( ushort pid, syspath_typ* );
os9err pRread    ( ushort pid, syspath_typ*, uint32_t *lenP,    char* buffer );
os9err pRreadln  ( ushort pid, syspath_typ*, uint32_t *lenP,    char* buffer );
os9err pRwrite   ( ushort pid, syspath_typ*, uint32_t *lenP,    char* buffer );
os9err pRwriteln ( ushort pid, syspath_typ*, uint32_t *lenP,    char* buffer );
os9err pRseek    ( ushort pid, syspath_typ*, uint32_t *posP );
os9err pRchd     ( ushort pid, syspath_typ*, ushort *modeP,   const char* pathname );
os9err pRdelete  ( ushort pid, syspath_typ*, ushort *modeP,   const char* pathname );
os9err pRmakdir  ( ushort pid, syspath_typ*, ushort *modeP,   const char* pathname );
os9err pRsize    ( ushort pid, syspath_typ*, uint32_t *sizeP );
os9err pRopt     ( ushort pid, syspath_typ*,                  byte* buffer );
os9err pRnam     ( ushort pid, syspath_typ*,                  char* volname );
os9err pRpos     ( ushort pid, syspath_typ*, uint32_t *posP  );
os9err pReof     ( ushort pid, syspath_typ* );
os9err pRlock    ( ushort pid, syspath_typ*, uint32_t *d0, uint32_t *d1, uint32_t *d2 );
os9err pRsetopt  ( ushort pid, syspath_typ*,                  byte* buffer );
os9err pRticks   ( ushort pid, syspath_typ*, uint32_t *d2 );
os9err pRready   ( ushort pid, syspath_typ*, uint32_t *n     );
os9err pRgetFD   ( ushort pid, syspath_typ*, uint32_t *maxbytP, byte* buffer );
os9err pRgetFDInf( ushort pid, syspath_typ*, uint32_t *maxbytP,
                                             uint32_t *fdinf,   byte* buffer );
os9err pRdsize   ( ushort pid, syspath_typ*, uint32_t *size,    uint32_t *dtype );

os9err pRsetsz   ( ushort pid, syspath_typ*, uint32_t *size );
os9err pRsetatt  ( ushort pid, syspath_typ*, uint32_t *attr  );
os9err pRsetFD   ( ushort pid, syspath_typ*,                  byte* buffer );
os9err pRWTrk    ( ushort pid, syspath_typ*, uint32_t *trackNr, byte* buffer );

void init_RBF_devs();
/* ------------------------------------------------------------------------- */

void init_RBF( fmgr_typ* f )
/* install all procedures of the RBF file manager */
{
    gs_typ* gs= &f->gs;
    ss_typ* ss= &f->ss;
    
    /* main procedures */
    f->open      = pRopen;
    f->close     = pRclose;
    f->read      = pRread;
    f->readln    = pRreadln;
    f->write     = pRwrite;
    f->writeln   = pRwriteln;
    f->seek      = pRseek;
    f->chd       = pRchd;
    f->del       = pRdelete;
    f->makdir    = pRmakdir;
    
    /* getstat */
    gs->_SS_Size = pRsize;
    gs->_SS_Opt  = pRopt;
    gs->_SS_DevNm= pRnam;
    gs->_SS_Pos  = pRpos;
    gs->_SS_EOF  = pReof;
    gs->_SS_Ready= pRready;
    gs->_SS_FD   = pRgetFD;
    gs->_SS_FDInf= pRgetFDInf;
    gs->_SS_DSize= pRdsize;   /* get drive size in sectors */

    /* setstat */
    ss->_SS_Size = pRsetsz;
    ss->_SS_Opt  = pRsetopt;      /* PD_SAS is real; the other eight are drive geometry */
    ss->_SS_Attr = pRsetatt;
    ss->_SS_FD   = pRsetFD;
    ss->_SS_Lock = pRlock;
    ss->_SS_Ticks= pRticks;
    ss->_SS_WTrk = pRWTrk;
    
     init_RBF_devs(); /* init RBF devices */
} /* init_RBF */

/* --------------------------------------------------------- */

void init_RBF_devs()
/* initialize them all to not installed */
{
  rbfdev_typ *dev;
  int  ii;
  for (ii=1; ii<MAXRBFDEV; ii++) {
    dev= &rbfdev[ ii ];
    dev->installed= false;
  } // for

  for (ii=0; ii<MAXSCSI; ii++) {
    strcpy( scsi[ ii ].name,"" );
  } // for
} /* init_RBF_devs */

// -----------------------------------------------------------------------

static Boolean IsSCSI( rbfdev_typ* dev )
{ return dev->scsi.ID!=NO_SCSI;
} /* IsSCSI */

static os9err ReadSector( rbfdev_typ* dev, ulong sectorNr, 
                                           ulong nSectors, byte* buffer )
/* Read sectors, either from SCSI or from an RBF image file */
{
    os9err  err= 0;
    Boolean pos_already;
    uint32_t sect     = dev->sctSize;
    ulong    pos      =     sectorNr*sect; // get position and length to read
    ulong    len      =     nSectors*sect;
    ulong    img      = dev->imgScts*sect;
    ulong    sectorLim= sectorNr + nSectors;   // upper limit
    ulong    blindNr, nBlinds;                 // not accessible sectors
    uint32_t cnt = 0;

//  if (sectorNr==0) {
//      debugprintf(dbgFiles,dbgDetail,("# RBF read  sector0\n"));
//  }
//  if (dev->imgScts<dev->totScts) upo_printf( "read  sectors %d..%d (%d)\n", sectorNr, sectorLim-1, dev->imgScts );

//  if (dev->imgScts>2097152) upo_printf( "read  sectors %d..%d (%d/%d)\n", 
//                                         sectorNr, sectorLim-1, dev->imgScts, dev->totScts );

    debugprintf(dbgFiles,dbgNorm,("# RBF read  sectorNr: $%06X (n=%d) @ $%08X\n",
                                     (uint32_t)sectorNr, (uint32_t)nSectors, (uint32_t)pos));
    if (sectorNr>0) {
      if (dev->totScts==0)        return E_NOTRDY;
      if (dev->totScts<sectorLim) return E_EOF; /* out of valid range */
      if (dev->imgScts<sectorLim) {
                              blindNr= dev->imgScts; 
        if (blindNr<sectorNr) blindNr= sectorNr;
    //  upo_printf( "read       sectors %d..%d\n", sectorNr, sectorLim-1 );
    //  upo_printf( "read blind sectors %d..%d\n", blindNr,  sectorLim-1 );
          
        nBlinds  = sectorLim-blindNr;
        nSectors-= nBlinds;
        len     -= nBlinds*sect;  
        memset( buffer + len, 0xEE, nBlinds*sect ); // fill rest with 0xEE
    //  upo_printf( "nBlinds=%d nSectors=%d len=%d\n", nBlinds,nSectors,len );
      } // if
    } // if
   
    do {
     // #ifdef RAM_SUPPORT    
          if (dev->isRAM) {
              memcpy( buffer, dev->ramBase+(sectorNr*sect), nSectors*sect );
              break;
          }
     // #endif
        
        if (IsSCSI(dev)) {
          err= ReadFromSCSI( &dev->scsi, sectorNr,nSectors, len,buffer ); if (!err) break;
          err= Set_SSize   ( &dev->scsi,                          sect ); if  (err) break; /* adjust sector size */
          err= ReadCapacity( &dev->scsi, &dev->totScts,          &sect ); dev->sctSize= sect;
          dev->imgScts= dev->totScts; if (err) break;                                   /* and get new info back */
                
          err= ReadFromSCSI( &dev->scsi, sectorNr,nSectors, len,buffer );
          

         //     err= ReadFromSCSI(  dev->scsiAdapt, dev->scsiBus, dev->scsiID, dev->scsiLUN, sectorNr,nSectors,  len,buffer ); 
         // if (err) {
         //     err= Set_SSize   (  dev->scsiAdapt, dev->scsiBus, dev->scsiID, dev->scsiLUN,                   dev->sctSize ); 
                                            
         //     err= ReadCapacity(  dev->scsiAdapt, dev->scsiBus, dev->scsiID, dev->scsiLUN,  &dev->totScts,  &dev->sctSize ); 
         //     dev->imgScts= dev->totScts; if (err) break; /* and get new info back */
                
         //     err= ReadFromSCSI(  dev->scsiAdapt, dev->scsiBus, dev->scsiID, dev->scsiLUN, sectorNr,nSectors, len,buffer );
         // } // if
        }
        else {
        //if (rawMode && sectorNr>=60 && sectorLim<=90) {
        //  if (sectorLim-1>sectorNr) upo_printf( "READ  RAW: %d..%d\n", sectorNr,sectorLim-1 );
        //  else                      upo_printf( "READ  RAW: %d\n",     sectorNr );
        //} // if
          
          pos_already = (pos==dev->currPos);
          dev->currPos= UNDEF_POS; /* invalidate for case of an error */
    
          if (len>0) {
            if (!pos_already) {       dev->rMiss++;
                err= syspath_seek( 0, dev->sp_img,  pos ); if (err) break;
            //  if (dev->imgScts>2097152) upo_printf( "seek err=%d\n", err );
            } // if
                                      dev->rTot++;  cnt= len;
                err= syspath_read( 0, dev->sp_img, &cnt, buffer, false ); 
            //  if (dev->imgScts>2097152) upo_printf( "read err=%d len=%d cnt=%d\n", err, len, cnt );
            if (err)                   break;
            if (cnt<len) { err= E_EOF; break; };  /* if not enough read, make EOF */
          } // if
          
          // don't set <currPos> behind <img> position 
               dev->currPos= pos+cnt; /* here is now our current read/write position */
          if  (dev->currPos>img) { /* upo_printf( "redu %d -> %d\n", dev->currPos,img ); */ dev->currPos= img; }
        }
    } while (false);

    return err;
} /* ReadSector */

static os9err WriteSector( rbfdev_typ* dev, ulong sectorNr, 
                                            ulong nSectors, byte* buffer )
/* Write sectors, either to SCSI or on an RBF image file */
{
    os9err  err= 0;
    Boolean pos_already, extendIt= false;
    uint32_t sect     = dev->sctSize;
    ulong    pos      =     sectorNr*sect;     // get position and length to write
    ulong    len      =     nSectors*sect;
    uint32_t img      = (uint32_t)(dev->imgScts*sect);
    ulong    sectorLim= sectorNr + nSectors;   // upper limit
    ulong    blindNr;                          // not accessible sectors
    uint32_t cnt;
    byte    ee;

    /* take care of write and format protection */
    if (dev->wProtected) return E_WP;
    if (sectorNr==0) {
        if (dev->fProtected) return E_FORMAT;
    }

//  if (dev->imgScts<dev->totScts) upo_printf( "write sectors %d..%d (%d)\n", sectorNr, sectorLim-1, dev->imgScts );
//  if (dev->imgScts>2097152) upo_printf( "write sectors %d..%d (%d/%d)\n", 
//                                         sectorNr, sectorLim-1, dev->imgScts, dev->totScts );
    
    debugprintf(dbgFiles,dbgNorm,("# RBF write sectorNr: $%06X (n=%d) @ $%08X\n",
                                     (uint32_t)sectorNr, (uint32_t)nSectors, (uint32_t)pos));
    if (sectorNr>0) {
      if (dev->totScts==0)        return E_NOTRDY;
      if (dev->totScts<sectorLim) return E_EOF; /* out of valid range */
      if (dev->imgScts<sectorLim) {
                              blindNr= dev->imgScts; 
        if (blindNr<sectorNr) blindNr= sectorNr;
     // upo_printf( "write       sectors %d..%d\n", sectorNr, sectorLim-1 );
     // upo_printf( "write blind sectors %d..%d\n", blindNr,  sectorLim-1 );
     // 
     // if (pos==dev->imgScts*dev->sctSize) upo_printf( "perfect fit" );
     // if (pos< dev->imgScts*dev->sctSize) upo_printf( "partly" );
        
        if (pos>img) {
          pos=  img;
          err= syspath_seek ( 0, dev->sp_img,  pos ); if (err) return err;
       // upo_printf( "fill blind sectors %d..%d\n", dev->imgScts, sectorNr-1 );
         
          // fill the unused part of the sector
          // this should not happen, as the RBF allocator will ask for the follow up sector
          while (pos<sectorNr*sect) {       cnt= sizeof(ee); ee= 0xFF; // slow because no buffer
            err= syspath_write( 0, dev->sp_img, &cnt,       &ee, false ); if (err) return err;
            pos++;
          } // while
        } // if
        
        dev->currPos= UNDEF_POS;
        dev->imgScts= sectorLim;
        img         = (uint32_t)(dev->imgScts*sect);
        extendIt    = true;
        err= syspath_setstat( 0, dev->sp_img, SS_Size, NULL,NULL, NULL,NULL,&img,NULL );
      } // if
    } // if
        
    do {
     // #ifdef RAM_SUPPORT
          if (dev->isRAM) {
              memcpy( dev->ramBase+(sectorNr*sect), buffer, nSectors*sect );
              break;
          }
     // #endif
        
        if (IsSCSI(dev))
          err= WriteToSCSI( &dev->scsi, sectorNr,nSectors, len,buffer ); 
        else {
        //if (rawMode && sectorNr>=60 && sectorLim<=90) {
        //  if (sectorLim-1>sectorNr) upo_printf( "WRITE RAW: %d..%d\n", sectorNr,sectorLim-1 );
        //  else                      upo_printf( "WRITE RAW: %d\n",     sectorNr );
        //} // if
          
          pos_already = (pos==dev->currPos);
          dev->currPos= UNDEF_POS; /* invalidate for case of an error */
    
          if (!pos_already) {        dev->wMiss++;
              err= syspath_seek ( 0, dev->sp_img,  pos ); if (err) break;
          } // if
                                     dev->wTot++;  cnt= len;
              err= syspath_write( 0, dev->sp_img, &cnt, buffer, false ); 
        //upo_printf( "write err=%d cnt=%d\n", err, cnt );
          if (err)                   break;
          if (cnt<len) { err= E_EOF; break; }; /* if not enough read, make EOF */

          dev->currPos= pos+cnt; /* here is now our current read/write position */
            
          if (extendIt) {
          //upo_printf( "extend cnt=%d %02X %02X %02X\n", cnt, buffer[ 0 ],buffer[ 1 ],buffer[ 2 ] );
            err= syspath_seek ( 0, dev->sp_img,  0            ); // flush the buffer
          //upo_printf( "seek0 err=%d\n", err );
            err= syspath_seek ( 0, dev->sp_img,  dev->currPos );
            
          //                                     cnt=         sect;
          //upo_printf( "seekN err=%d pos=%d\n", err, dev->currPos );
          //err= syspath_read ( 0, dev->sp_img, &cnt, dev->tmp_sct, false ); 
           
          //upo_printf( "size is now %d err=%d pos=%d\n", img, err, dev->currPos );
          //err= syspath_setstat( 0, dev->sp_img, SS_Size, NULL,NULL, NULL,NULL,&img,NULL );
          //upo_printf( "size is now %d err=%d pos=%d\n", img, err, dev->currPos );
          } // if
        }
    } while (false);
    
    return err;
} /* WriteSector */

// ----------------------------------------------------------------
// utility procs

static os9err CutOS9Path( char** p, char* cmp_entry )
/* gets the next subpath of <p> into <cmp_entry> */
{
    os9err  err= 0;
    int     n  = 1;
    char*   c  = cmp_entry;
    
    if (ustrcmp ( *p,"..."     )==0 || /* 3 possibilities */
        ustrncmp( *p,"....", 4 )==0 ||
        ustrncmp( *p,".../", 4 )==0) { /* support multiple up path: ... */
        strcpy( cmp_entry,"..");
        (*p)++; return 0; /* no error */
    }
    
    while   (**p!=NUL) {
        /* the separator first: counted first, a 28-character name -- the
           longest RBF allows -- was refused E$BPNam on the '/' after it, so
           it worked as a path's last element and nowhere else */
        if  (**p==PSEP) { (*p)++; break; };
        if  (n++>=OS9NAMELEN) { err= E_BPNAM; break; } /* avoid too long strings */
        *c=  **p; 
        c++; (*p)++;
    }
    
    *c= NUL;
    return err;
} /* CutOS9Path */

static os9err CheckSectorSize( uint32_t sctSize )
/* Is this a sector size an RBF device can have here?
 *
 * The manual's rule, from E$SectSize's own entry (v2.4 Technical Manual,
 * 000:176): "The sector size of a RBF device must be a binary multiple of 256
 * (256, 512, 1024, etc.). The maximum sector size is 32768."
 *
 * os9exec stops lower, at MIN_TMP_SCT_SIZE, because GetBuffers below sizes a
 * path's FD and data sector buffers to it. That is a real ceiling and not a
 * gap worth closing: DD_TOT is three bytes, so a device holds at most
 * 16777215 sectors, and 2048-byte sectors already address 32 GiB -- past any
 * media OS-9/68k ever ran on. See DECISIONS-68k.md.
 *
 * Either way the fault is the sector size, so E$SectSize is what says so.
 * Both callers used to answer something else: the RAM disk path let the value
 * through unchecked and it surfaced later as E$NotRdy ("device not ready",
 * which is not why it failed), and the SCSI path returned <err> at a point
 * where <err> is zero -- reporting SUCCESS for a sector size it had just
 * decided it could not read. */
{
    if (sctSize < STD_SECTSIZE)     return os9error(E_SECTSIZE); /* below 256 */
    if (sctSize & (sctSize-1))      return os9error(E_SECTSIZE); /* not 256<<n */
    if (sctSize > MIN_TMP_SCT_SIZE) return os9error(E_SECTSIZE); /* past our buffers */
    return 0;
} /* CheckSectorSize */


static void GetBuffers( _rbf_, syspath_typ* spP )
{
//Boolean pp= spP->fd_sct==NULL || spP->rw_sct==NULL;
    
//if (pp) upe_printf( "Getbuffers %d %d %08X %08X\n", 
//                     spP->nr, dev->sctSize, spP->fd_sct,spP->rw_sct );

  /* allocate them only once, assume sctSize will not change */
//if (spP->fd_sct==NULL) spP->fd_sct= get_mem( dev->sctSize, false );
//if (spP->rw_sct==NULL) spP->rw_sct= get_mem( dev->sctSize, false );

  /* %%% there is currently an allocation bug, use always 2048 bytes */
  if (spP->fd_sct==NULL) spP->fd_sct= get_mem( 2048 );
  if (spP->rw_sct==NULL) spP->rw_sct= get_mem( 2048 );

//if (pp) upe_printf( "Getbuffers %d %d %08X %08X\n", 
//                     spP->nr, dev->sctSize, spP->fd_sct,spP->rw_sct );
} /* GetBuffers */

static void Set_FDSize( syspath_typ* spP, ulong size ); /* defined with the other FD accessors */
static ulong FDSize   ( syspath_typ* spP );             /* likewise */
static void WakeOnFile( syspath_typ* spP );            /* defined with the other ring walkers */

/* Paths open on the same file are linked into a ring, so each one can reach
 * the others' buffers. Every path keeps its own <fd_sct>/<rw_sct> -- they are
 * working copies at that path's own position, and <fd_sct> is scratch that
 * CreateNewFile deliberately repurposes -- but a path on its own cannot see
 * what the others are doing, which is how a reader following a writer came to
 * sit at the size, segment list and sector contents the file had when it
 * opened. The ring turns those private buffers into a shared cache: read from
 * the ring before the device, and take the FD from whoever is writing it. */

static void RingLeave( syspath_typ* spP )
/* unlink from the ring, leaving the others linked to each other */
{
    syspath_typ* spK;
    ushort       k;

    for (k=1; k<MAXSYSPATHS; k++) {
          spK= &syspaths[k];
      if (spK!=spP && spK->type==fRBF &&
          spK->u.rbf.sameFile==spP->nr) {
          spK->u.rbf.sameFile= spP->u.rbf.sameFile;
        break;
      } // if
    } // for

    spP->u.rbf.sameFile= spP->nr;
} /* RingLeave */

static void RingAdopt( syspath_typ* spP )
/* take the ring's current view of the file on joining it. A path that opens
 * a file others already have open reads the FD from the device, and a writer
 * that has grown the file holds the new size in its <lastPos> only (see
 * RingPublish for why its FD does not carry it yet) -- so the newcomer took
 * the file to end where it ended before those writes, and read EOF with the
 * bytes already there. Found by the skills session: a reader opened after a
 * producer appended "A" read the old 11 bytes and stopped. The view adopted
 * is the one that knows the file furthest along, exactly as RingPublish would
 * have handed it over had this path been open at the time. */
{
    rbfdev_typ*  dev = &rbfdev[spP->u.rbf.devnr];
    syspath_typ* best= NULL;
    syspath_typ* spK;
    ulong        size= spP->fd_sct!=NULL ? FDSize( spP ) : 0, sK;
    ushort       k   = spP->u.rbf.sameFile;

    if (spP->fd_sct==NULL) return;

    while (k!=spP->nr && k!=0) {
             spK= &syspaths[k];
      if (   spK->fd_sct!=NULL) {
             sK= FDSize( spK );
         if (spK->u.rbf.lastPos>sK) sK= spK->u.rbf.lastPos;
         if (sK>size) { size= sK; best= spK; }
      } // if
      k= spK->u.rbf.sameFile;
    } // while

    if (best==NULL) return;                  /* nobody knows more than the FD */
    memcpy   ( spP->fd_sct, best->fd_sct, dev->sctSize );
    Set_FDSize( spP, size );                 /* after the copy: it wins */
    if (spP->u.rbf.lastPos<size) spP->u.rbf.lastPos= size;
} /* RingAdopt */

static void RingJoin( syspath_typ* spP )
/* link into the ring of paths already open on this file (a ring of one when
 * this is the only path). Called wherever <fd_nr> is established, not only at
 * open: CreateNewFile moves a path off the directory onto the new file, and a
 * path left in the wrong ring would be reading strangers' buffers. */
{
    rbf_typ*     rbf= &spP->u.rbf;
    syspath_typ* spK;
    ushort       k;

    /* The END of the file it is leaving is not its to keep either. An
     * update-mode open walks the directory through this very path, reads the
     * directory to its end, and so takes the directory's EOF lock -- which used
     * to ride along onto the file the path opened. Two update paths on one file
     * in one process then each held the other's end, and the first read at EOF
     * on either was E$DEADLK, "the writer is us": dbz opens history.pag twice
     * for update and failed "store failed" on every RBF image. Dropped here,
     * while the path is still in the old ring, waking anyone asleep on that
     * end exactly as pRclose does. */
    if (rbf->eofLock) { rbf->eofLock= false; WakeOnFile( spP ); }

    RingLeave( spP );                          /* out of the previous one first */

    /* Any record it held belonged to the file it is leaving -- most often the
     * directory it just walked through to get here, since the walk reads
     * through the same path. Carrying those byte offsets over to a different
     * file would lock a stretch of it at random. */
    rbf->lockBeg= 0;
    rbf->lockEnd= 0;

    if (spP->rawMode || rbf->fd_nr==0) return; /* not a file: stays alone */

    for (k=1; k<MAXSYSPATHS; k++) {
          spK= &syspaths[k];
      if (spK!=spP && spK->type==fRBF && !spK->rawMode &&
          spK->u.rbf.devnr==rbf->devnr &&
          spK->u.rbf.fd_nr==rbf->fd_nr) {
          rbf->sameFile      = spK->u.rbf.sameFile; /* splice in behind it */
          spK->u.rbf.sameFile= spP->nr;
          RingAdopt( spP );
        return;
      } // if
    } // for
} /* RingJoin */

static void RingPublishFD( syspath_typ* spP )
/* hand this path's FD to every other path open on the same file. Anything
 * that changes the descriptor -- attributes, owner, size -- has to do this,
 * or the others go on using the copy they took when they opened, which is the
 * whole class of bug the ring exists to remove. */
{
    rbfdev_typ*  dev= &rbfdev[spP->u.rbf.devnr];
    syspath_typ* spK;
    ushort       k  =  spP->u.rbf.sameFile;

    if (spP->fd_sct==NULL || spP->rawMode) return;

    while (k!=spP->nr && k!=0) {
             spK= &syspaths[k];
      if (   spK->fd_sct!=NULL) memcpy( spK->fd_sct, spP->fd_sct, dev->sctSize );
      k= spK->u.rbf.sameFile;
    } // while
} /* RingPublishFD */

static void RingSetLastPos( syspath_typ* spP, ulong size )
/* force every other path's idea of the end of the file to <size>. Needed when
 * the file is TRUNCATED: publishing the FD alone leaves a reader whose
 * <lastPos> still sits past the new end. */
{
    syspath_typ* spK;
    ushort       k= spP->u.rbf.sameFile;

    while (k!=spP->nr && k!=0) {
             spK= &syspaths[k];
      if (   spK->u.rbf.lastPos>size) spK->u.rbf.lastPos= size;
      if (   spK->u.rbf.currPos>size) spK->u.rbf.currPos= size;
      k= spK->u.rbf.sameFile;
    } // while
} /* RingSetLastPos */

static void RingPublish( syspath_typ* spP, ulong size )
/* hand this path's view of the file to the others open on it: the segment
 * list, attributes and owner exactly as they stand in this path's FD, plus
 * <size>, which a growing file only has in <lastPos>.
 * The FD's own size field is deliberately left alone here. This path's FD
 * buffer is flushed to the device by the allocate/create machinery at points
 * of its choosing, and a size written into it ahead of those reaches the disk
 * too early -- which silently loses directory entries. The other paths never
 * flush an FD (a read-mode path writes none), so their copies are safe to
 * write. */
{
    rbfdev_typ*  dev= &rbfdev[spP->u.rbf.devnr];
    syspath_typ* spK;
    ushort       k  =  spP->u.rbf.sameFile;

    if (spP->fd_sct==NULL) return;

    while (k!=spP->nr && k!=0) {
             spK= &syspaths[k];
      if (   spK->fd_sct!=NULL) {
        memcpy   ( spK->fd_sct, spP->fd_sct, dev->sctSize );
        Set_FDSize( spK, size );                  /* after the copy: it wins */
        if (spK->u.rbf.lastPos<size)
            spK->u.rbf.lastPos= size;
      } // if

      k= spK->u.rbf.sameFile;
    } // while
} /* RingPublish */

static Boolean RingSector( syspath_typ* spP, ulong sect, byte* b, ulong len )
/* fetch a sector from another path's buffer, if one of them is holding it:
 * a sector written but not yet flushed exists only there, and the device
 * still has the bytes that were in it before */
{
    syspath_typ* spK;
    ushort       k= spP->u.rbf.sameFile;

    while (k!=spP->nr && k!=0) {
             spK= &syspaths[k];
      if (   spK->rw_sct!=NULL && spK->mustW!=0 && /* 0 is "nothing pending" */
             spK->mustW==sect) {
        memcpy( b, spK->rw_sct, len );
        return true;
      } // if

      k= spK->u.rbf.sameFile;
    } // while

    return false;
} /* RingSector */

static os9err RingFlushRange( syspath_typ* spP, ulong sect, ulong n )
/* write out any other path's unflushed copy of sectors <sect>..<sect+n-1>,
 * before a multi-sector read goes straight to the device for them -- RingSector
 * only serves one sector at a time, so without this the read would return the
 * bytes the device held before those writes. The copies stay valid: they now
 * match the device. */
{
    rbfdev_typ*  dev= &rbfdev[spP->u.rbf.devnr];
    syspath_typ* spK;
    ushort       k= spP->u.rbf.sameFile;
    os9err       err;

    while (k!=spP->nr && k!=0) {
             spK= &syspaths[k];
      if (   spK->rw_sct!=NULL && spK->mustW!=0 &&
             spK->mustW>=sect  && spK->mustW<sect+n) {
        err= WriteSector( dev, spK->mustW,1, spK->rw_sct ); if (err) return err;
        spK->mustW= 0;
      } // if

      k= spK->u.rbf.sameFile;
    } // while

    return 0;
} /* RingFlushRange */

static Boolean ShareConflict( syspath_typ* spP, Boolean wantSingle )
/* Would this open break the single-user rule? True if another PROCESS already
 * has this file open and either side wants it to itself.
 *
 * "If the single-user bit is set, only one process may open the file at a
 * time. If another process attempts to open the file, error (#253) is
 * returned" (Non-sharable Files, page 7-8). I$Open's access-mode table gives
 * bit 6 as "open file for non sharable use" -- the transient form the manual
 * recommends for the common case, as against setting the attribute on the file
 * itself. Both were accepted and neither was enforced: two opens both
 * succeeded.
 *
 * Qualified exactly as RingJoin qualifies a same-file path, because that is
 * what "the same file" means here -- device plus FD sector, not the pathlist
 * used to reach it.
 *
 * Another PROCESS, not another path: the manual counts processes ("only one
 * process may open the file at a time"), and one process opening its own file
 * twice is the multi-path idiom that page 7-8 recommends two paragraphs
 * earlier. Refusing that would re-break what CONF68K t29 exists to protect. */
{
    syspath_typ* spK;
    ushort       k;

    if (spP->rawMode || spP->u.rbf.fd_nr==0) return false; /* not a file */

    for (k=1; k<MAXSYSPATHS; k++) {
          spK= &syspaths[k];
      if (spK!=spP && spK->type==fRBF && !spK->rawMode &&
          spK->u.rbf.devnr==spP->u.rbf.devnr &&
          spK->u.rbf.fd_nr==spP->u.rbf.fd_nr &&
          spK->u.rbf.ownPid!=spP->u.rbf.ownPid &&
         (wantSingle || spK->u.rbf.single)) return true;
    } // for

    return false;
} /* ShareConflict */



static Boolean RingHasOther( syspath_typ* spP )
/* True if some path OTHER than <spP> is open on the same file, in any mode.
 * I$Delete: "The caller must have non-sharable write access to the file (the
 * file may not already be open) or an error results." (Technical Manual, page
 * 2-7). Only another WRITER used to be refused: a writer strands every cluster
 * it allocates after the delete (<DeallocateBlocks> runs once, at delete time),
 * but a READER was let through too, and kept reading sectors the delete had
 * already freed and the next allocation could hand to another file. */
{
    ushort k= spP->u.rbf.sameFile;

    return k!=spP->nr && k!=0;
} /* RingHasOther */

/* A reader that has caught up to a file another path still has open for
 * writing is not at the end of it, it is merely early: the writer may write
 * more, so end-of-file is not the answer yet. It sleeps instead, and the
 * writer starts it again on its next write, or when it closes and there
 * really is nothing more to come. Nothing is locked to do this -- the wait
 * sits past the last byte, where there is no data to lock.
 *
 * Sleeping means returning and being called again: a file manager here is a C
 * function called from the syscall dispatcher, so it cannot suspend part-way
 * through the way RBF's own Read could. The dispatcher has already saved the
 * registers, so the read simply runs again from the top when the process next
 * runs -- which is what <pWaitRead> arranges. */

/* Record locking. A read on a path open for update locks exactly the bytes it
 * handed back, and the next write on that path lets them go again -- so a
 * read-modify-write cycle is safe against another process doing the same
 * thing, with the application making no locking calls of its own. A path holds
 * at most one record, so a later read simply replaces the earlier lock.
 * Read-only and write-only paths take nothing: a read-only path cannot modify
 * what it read, and a write-only path never read anything to modify. */

static syspath_typ* LockHolder( syspath_typ* spP, ulong beg, ulong end )
/* Another path on this file whose locked record overlaps [beg,end) and whose
   owner is a DIFFERENT process.

   The owner test is the manual's rule, not an optimisation: "If an application
   requires more than one record to be locked out, multiple paths to the same
   file may be opened with each path having its own record locked out. RBF
   notices that the same process owns both paths and keeps them from locking
   each other out" (Technical Manual, Record Locking and Unlocking, page 7-8).
   Two paths held by one process do not conflict AT ALL, so the question "what
   should happen when they collide" never arises -- which is why the three
   callers no longer answer it with E$DeadLk. That answer broke the very idiom
   the manual recommends: a program locking two records the documented way got
   a deadlock error reported against itself.

   Owner against owner rather than owner against the running process, because
   that is what the manual says ("the same process owns both paths"), and
   because it makes a conflict a property of the two paths instead of something
   that varies with whoever happens to be executing. The two differ only for a
   path used by a process that did not open it -- an inherited path in a forked
   child -- where the manual's wording is the one to follow.

   A path still never conflicts with itself: the loop bounds skip spP, exactly
   as before. */
{
    syspath_typ* spK;
    ushort       k= spP->u.rbf.sameFile;

    while (k!=spP->nr && k!=0) {
             spK= &syspaths[k];
      if (   spK->u.rbf.lockBeg<spK->u.rbf.lockEnd && /* holds one at all */
             spK->u.rbf.lockBeg<end && beg<spK->u.rbf.lockEnd &&
             spK->u.rbf.ownPid!=spP->u.rbf.ownPid) return spK;

      k= spK->u.rbf.sameFile;
    } // while

    return NULL;
} /* LockHolder */

static void LockDrop( syspath_typ* spP )
/* let go of whatever this path holds */
{
    spP->u.rbf.lockBeg= 0;
    spP->u.rbf.lockEnd= 0;
} /* LockDrop */

/* The EOF lock. Quoting the v2.4 Technical Manual (ch.7, "End of File Lock")
 * because every word of it is load-bearing:
 *
 *   "An EOF lock occurs when the user reads or writes data at the end of file.
 *    The user keeps the end of file locked until a read or write is performed
 *    that is not at the end of the file. EOF lock is the only time that a
 *    write call automatically causes lock out of any part of the file. This
 *    avoids problems that could occur when two users try to simultaneously
 *    extend a file."
 *
 *   "An extremely useful side effect occurs when a program creates a file for
 *    sequential output. As soon as the file is created, EOF lock is gained,
 *    and no other process is able to pass the writer in processing the file.
 *    ... if you redirect an assembly listing to a disk file, a spooler utility
 *    can open and begin listing the file before the assembler has written even
 *    the first line of output. Record locking always keeps the spooler one
 *    step behind the assembler."
 *
 * Three consequences this code has to get right:
 *
 * 1. It is NOT gated on update mode. Update mode governs the RECORD lock a
 *    read takes (ch.7, "Record Locking"); the EOF lock is the one lock a
 *    plain write-only path acquires, which is exactly what makes the spooler
 *    example work -- an assembler redirecting its listing opened that file
 *    for sequential output, not for update.
 * 2. It is HELD, not re-taken per write: released by an access that is not at
 *    the end of the file, so a run of appends keeps it throughout.
 * 3. It is separate state from lockBeg/lockEnd. Letting a record go does not
 *    let the end of the file go.
 *
 * Only paths that could actually extend the file take it. The manual's "reads
 * or writes" is about the position, not a licence for a read-only follower to
 * lock the end -- if it did, the spooler would block the assembler, inverting
 * the very example the feature exists to serve. A reader still WAITS on it.
 *
 * End of file is a lock to acquire, not a condition to compute. This used to
 * be computed -- "is any update-mode path open for writing?" -- which both
 * made write-only appenders invisible to a follower and made a writer that
 * never went near the end block one forever. */

static ushort EofLockHolder( syspath_typ* spP )
/* the process holding this file's end through one of the other paths, 0 if
 * none. Which process it is matters: waiting for one's own process to write
 * more is waiting for oneself, since the wait is what stops it getting there. */
{
    syspath_typ* spK;
    ushort       k= spP->u.rbf.sameFile;

    while (k!=spP->nr && k!=0) {
             spK= &syspaths[k];
      if (   spK->u.rbf.eofLock) return spK->u.rbf.ownPid;

      k= spK->u.rbf.sameFile;
    } // while

    return 0;
} /* EofLockHolder */

static Boolean WaitExpired( syspath_typ* spP )
/* has this path waited as long as SS_Ticks said it was willing to? Only ever
 * true if a limit was actually set: with none, waiting is unbounded, which is
 * what a program that would rather be late than fail wants.
 *
 * NOTE this can only fire if the waiting process is actually re-run while it
 * waits, and without a system tick (-q) almost nothing re-runs it: measured
 * on a 2-second hold, a blocked reader got two chances to look and then none
 * until the holder released, so the limit was never noticed. With -q it is
 * checked regularly and expires when asked. A timeout is only as good as the
 * scheduling underneath it. */
{
    rbf_typ* rbf= &spP->u.rbf;

    if (rbf->lockTicks==0) return false;      /* no limit asked for */
    if (rbf->waitUntil ==0) {                 /* first time round: set the deadline */
        rbf->waitUntil= GetSystemTick() + rbf->lockTicks;
        return false;
    } // if

    return GetSystemTick() >= rbf->waitUntil;
} /* WaitExpired */

static void SleepOnFile( syspath_typ* spP, ushort pid )
/* wait for this file to grow, or for its writer to close */
{
    process_typ* cp= &procs[pid];

    spP->u.rbf.waitPid= pid;
                        cp->saved_state= cp->state; /* to come back to */
    set_os9_state( pid, pWaitRead, "RBF eof" );
} /* SleepOnFile */

static void WokeOnFile( ushort pid )
/* put the process back as it was once its read can go ahead. Without this it
 * stays <pWaitRead>, and the dispatcher keeps restoring the saved registers
 * and running the same read over and over -- it succeeds every time and is
 * re-entered every time, which looks exactly like a hang. */
{
    process_typ* cp= &procs[pid];

    if (cp->state==pWaitRead) set_os9_state( pid, cp->saved_state, "RBF eof resume" );
} /* WokeOnFile */

static void WaitDone( syspath_typ* spP )
/* no longer waiting: forget the deadline so the next wait starts a fresh one */
{
    spP->u.rbf.waitUntil= 0;
} /* WaitDone */

static void WakeOnFile( syspath_typ* spP )
/* start every path asleep on this file -- all of them, not one: each looks
 * again for itself, and what it finds is its own business */
{
    syspath_typ* spK;
    ushort       k= spP->u.rbf.sameFile;

    while (k!=spP->nr && k!=0) {
             spK= &syspaths[k];
      if (   spK->u.rbf.waitPid!=0) {
        /* eligible at the next arbitration, and NOT made active: the state has
         * to stay <pWaitRead>, because that is what makes the dispatcher put
         * the saved registers back and run the read again. Waking it to
         * pActive would resume it past a read that never happened. */
        procs[ spK->u.rbf.waitPid ].pW_age= 0;
             spK->u.rbf.waitPid= 0;
      } // if

      k= spK->u.rbf.sameFile;
    } // while
} /* WakeOnFile */

static void RingInvalidate( syspath_typ* spP, ulong sect )
/* drop this sector from the other paths' buffers -- unflushed copies included.
 *
 * A clean copy has to go or its path keeps serving itself the bytes it already
 * holds: DoAccess only re-reads a sector when <rw_nr> differs.
 *
 * An unflushed copy has to go too, and keeping it -- "its own unflushed work" --
 * lost writes. Only one path may hold a sector dirty. A path dirties a sector
 * only after loading it, from the device or, through RingSector, from the one
 * path holding it dirty; any copy loaded earlier was dropped here when that
 * other path dirtied it. So the path calling this already carries every
 * unflushed change, and an older copy left dirty would be written back over it
 * at the next flush. That is what happened: move links the new name through
 * one update path on the directory and clears the old one through a second,
 * both paths flushed the same sector at close, and the stale one landed last --
 * the old name survived beside the new, two names on one FD, so deleting
 * either would free sectors the other still used. Measured on a plain file
 * too: two update paths writing different bytes of one sector kept only one. */
{
    syspath_typ* spK;
    ushort       k= spP->u.rbf.sameFile;

    while (k!=spP->nr && k!=0) {
             spK= &syspaths[k];
      if (   spK->rw_nr==sect) spK->rw_nr= 0;
      if (   spK->mustW==sect) spK->mustW= 0; /* superseded by this path's copy */

      k= spK->u.rbf.sameFile;
    } // while
} /* RingInvalidate */

static void ReleaseBuffers( syspath_typ* spP )
{
//upe_printf( "Relbuffers %d %08X %08X\n", spP->nr, spP->fd_sct,spP->rw_sct);
  RingLeave( spP );


  /* the NULLing is unconditional -- on its own line, so it reads that way */
  if (spP->fd_sct!=NULL) release_mem( spP->fd_sct );
  spP->fd_sct= NULL;

  if (spP->rw_sct!=NULL) release_mem( spP->rw_sct );
  spP->rw_sct= NULL;
} /* ReleaseBuffers */

static os9err ReleaseIt( ushort pid, rbfdev_typ* dev )
{
  os9err err= 0;
  Boolean isRAMDisk= dev->isRAM;
  if     (isRAMDisk) release_mem( dev->ramBase );
    
  if (!isRAMDisk &&  
      !IsSCSI( dev )) err= syspath_close( pid, dev->sp_img );
    
  if (!err) {    dev->installed= false;
    release_mem( dev->tmp_sct );
                 dev->tmp_sct= NULL;
  } // if
    
  return err;
} /* ReleaseIt */

static os9err DevSize( rbfdev_typ* dev )
/* get the size of the device as numbers of sectors
   and adjust either device's sector size if possible, otherwise
   copy device's current sector size to dev->sctSize */
{
    os9err   err;
    uint32_t size;
    uint32_t ssize;

    /* should be defined already for the RAM disk */
 // #ifdef RAM_SUPPORT
      if (dev->isRAM) return 0;
 // #endif

    if (IsSCSI(dev)) {  /* try to switch to correct sector size first */
        err= Set_SSize( &dev->scsi, dev->sctSize );
        if (err) {
            // not possible to change sector size to what OS9 expects
            // - get sector size of SCSI device
            err= Get_SSize( &dev->scsi, &ssize );
            if (err) return err; 
            err= CheckSectorSize( ssize ); if (err) return err; // can't read sector 0 that big
            // - use sector size of SCSI device for now
            dev->sctSize=ssize;
        }
        err= ReadCapacity( &dev->scsi, &dev->totScts, &dev->sctSize );
        dev->imgScts=                   dev->totScts;
    }
    else { 
        err= syspath_gs_size( 0, dev->sp_img, &size );
    //  upo_printf( "DevSize v img=%d/tot=%d \n", dev->imgScts, dev->totScts );
        dev->imgScts= size/dev->sctSize;
    //  dev->imgScts= dev->totScts;
    //  upo_printf( "DevSize n img=%d/tot=%d \n", dev->imgScts, dev->totScts );
    }

    return err;
} /* DevSize */

static os9err ChkIntegrity( rbfdev_typ* dev, syspath_typ* spP, 
                            byte* mysct, Boolean ignore )
{
    ushort diskID= GET_OS9W(mysct, 14);

        dev->last_diskID = diskID;
    if (spP->u.rbf.diskID!=diskID) { /* doesn't matter big/little endian */
        spP->u.rbf.diskID= diskID;
        if (ignore) return 0;

        spP->u.rbf.fd_nr= 0; /* do not access any more this fd */
        spP->mustW      = 0; /* don't write this sector */
        DevSize( dev );      /* get new device size */
        return E_DIDC;
    }
    
    return 0;
} /* ChkIntegrity */

static os9err GetTop( ushort pid, rbfdev_typ* dev )
{
  os9err err = 0;
  short  sp  = dev->sp_img;
  ulong    sect= dev->sctSize;
  ulong    map = dev->mapSize;
  ulong    last= map % sect;
  uint32_t len;
  ulong    pos = ( (map-1)/sect + 1 )*sect;
  byte     b   = 0;
  ulong    offs= 0;
  ulong    sv= dev->imgScts;
  uint32_t img;
  
//upo_printf( "map=%d sect=%d pos=%d\n", map, sect, pos );
  
  if (last==0) last= sect;
  
  // search for the last allocated position at the allocation map
  while (pos>=sect) {
    err= syspath_seek( pid,  sp,  pos );                       if (err) return err;
                                  len= sect;
    err= syspath_read( pid,  sp, &len, dev->tmp_sct, false );  if (err) return err;
    
    while (last>0) {
          b= dev->tmp_sct[ --last ];
      if (b) break; 
    } // while
    
    if (b) break;
    pos-= sect;
    last= sect;
  } // while
  
  while (b) { b= b<<1; offs++; }

//upo_printf( "GetTop v img=%d tot=%d\n", dev->imgScts, dev->totScts );
  dev->imgScts= ( ( pos-sect+last )*BpB + offs )*dev->clusterSize;
//upo_printf( "GetTop n img=%d tot=%d\n", dev->imgScts, dev->totScts );
  
  if (sv>dev->imgScts) {
//  upo_printf( "REDUCE\n" );
    img= (uint32_t)(dev->imgScts*sect);
//  upo_printf( "REDUCE img=%d\n",  img );
    err= syspath_setstat( pid, sp, SS_Size, NULL,NULL, NULL,NULL,&img,NULL );
//  upo_printf( "REDUCED err=%d\n", err );
  } // if
  
//upo_printf( "map=%d sect=%d pos=%d last=%d img=%d rest=%d\n", map, sect, pos, last, img, dev->totScts-img );
  return err;
} /* GetTop */

static os9err GetFull( ushort pid, rbfdev_typ* dev )
{
  os9err   err = 0;
  short    sp  = dev->sp_img;
  ulong    sect= dev->sctSize;
  uint32_t img = (uint32_t)(dev->totScts*sect);

  err= syspath_setstat( pid, sp, SS_Size, NULL,NULL, NULL,NULL,&img,NULL );
  if (!err) dev->imgScts= dev->totScts;
  
//upo_printf( "FULL AGAIN err=%d\n", err );
  return err;
} /* GetFull */

static os9err RootLSN( _pid_, rbfdev_typ* dev, syspath_typ* spP, Boolean ignore )
{
    os9err  err;
    ushort  sctSize;
    Boolean cruz;
    
 // upo_printf( "RootLSN\n" );
    while (true) { /* loop */
        debugprintf(dbgFiles,dbgNorm,("# RootLSN: sectorsize %d\n", dev->sctSize ));
        
        // get size of device 
        if (dev->totScts==0) { err= DevSize( dev ); if (err) return err; }
        err= ReadSector( dev, 0,1,  dev->tmp_sct ); if (err) return err;

        /* get the sector size of the device */
        cruz= (strcmp( (char*)&dev->tmp_sct[CRUZ_POS],Cruz_Str )==0);
        debugprintf(dbgFiles,dbgNorm,("# RootLSN: sectorsize %d %s\n", dev->sctSize, cruz?"(cruz)":"" ));
        
        if (cruz) {
            /* The image's own word, so as untrusted as the rest of it: every
               path's FD and data buffers are sized to MIN_TMP_SCT_SIZE, and a
               larger sector read into them ran past their ends. */
            sctSize= GET_OS9W(dev->tmp_sct, SECT_POS);
            /* 0 is what images from formats that predate the field carry, and
               has always meant the standard 256 (bfo 2002, below). Checked
               before that fallback, it refused images that had always opened. */
            if (sctSize==0) sctSize= STD_SECTSIZE;
            err= CheckSectorSize( sctSize ); if (err) return err;
        }
        else {
            if (dev->sctSize==0) sctSize= STD_SECTSIZE;
            else                 sctSize= dev->sctSize; // use existing (luz 2002-02-12)
        }   
        if          (sctSize==0) sctSize= STD_SECTSIZE; // but avoid 0  (bfo 2002-06-06)
           
        debugprintf(dbgFiles,dbgNorm,("# RootLSN: adapt sectorsize %d => %d\n",
                                         dev->sctSize, sctSize));
        if (dev->sctSize==sctSize) break; /* it is ok already ? */
        	dev->sctSize= sctSize;        /* this is the correct sector size */
    
        /* release buffers of old size */
        release_mem   ( dev->tmp_sct );
        ReleaseBuffers( spP );
        
        /* and get the new buffers with the new sector size */
        dev->tmp_sct= get_mem( dev->sctSize>MIN_TMP_SCT_SIZE ? dev->sctSize : MIN_TMP_SCT_SIZE );
        GetBuffers( dev,spP );

        dev->totScts   = 0; /* must be set to correct value */
        dev->last_alloc= 0; /* initialize allocater pointer */
    } /* loop */
    
    dev->totScts    = GET_OS9L(dev->tmp_sct, TOT_POS) >> BpB;
    if (dev->imgScts==0) dev->imgScts = dev->totScts;
    dev->mapSize    = GET_OS9W(dev->tmp_sct, MAP_POS);
    dev->clusterSize= GET_OS9W(dev->tmp_sct, BIT_POS);
    dev->root_fd_nr = GET_OS9L(dev->tmp_sct, DIR_POS) >> BpB;

    /* LSN 0 is data the IMAGE chose, and two of these fields are used as
       DIVISORS while a third sizes a scan -- so a corrupt or hostile image
       reaches host arithmetic before any OS-9 level check ever runs.
       clusterSize==0 divides by zero in AllocBit()/DevSize(): silently 0 on
       arm64, but SIGFPE on x86, which is what the Linux and Windows builds
       are. An oversized mapSize makes the allocator count clusters the media
       does not have -- measured with DD_MAP=$FFFF on a 504K image, `free`
       reported "128 Mb of 504 Kb free on media (26003%)" and the map scan read
       directory and file sectors as allocation bits.
       Refuse the structurally impossible here, once, rather than defending
       every divide downstream. The bound is deliberately slack -- rounded up
       to a whole sector, because a real formatter may pad the map, and
       refusing a legitimate disk would be a far worse bug than this one.
       Found by tools/fuzz-rbf-image.sh's targeted probes. */
    {   uint32_t totClu, needBytes, maxMap;
        if (dev->clusterSize==0 ||
            (dev->clusterSize & (dev->clusterSize-1))!=0) return E_BTYP;
        if (dev->mapSize==0)                              return E_BTYP;
        totClu   = (dev->totScts + dev->clusterSize-1)/dev->clusterSize;
        needBytes= (totClu       + BpB-1)             /BpB;
        maxMap   = ((needBytes + dev->sctSize-1)/dev->sctSize)*dev->sctSize;
        if (dev->mapSize>maxMap) {
            debugprintf(dbgFiles,dbgNorm,
              ("# RootLSN: map size %d exceeds the %d this media can need, E_BTYP\n",
                  dev->mapSize, maxMap ));
            return E_BTYP;
        }
    }

    spP->u.rbf.fd_nr=  dev->root_fd_nr;
    err= ChkIntegrity( dev,spP, dev->tmp_sct, ignore );
    return err;
} /* RootLSN */

static void CutPath( char* s )
/* Reduce a path to its last element, in place. */
{
  int  ii,len= strlen( s );
  for (ii=len-1; ii>=0; ii--) {
     /* memmove, not strcpy: source and destination are the SAME buffer, so the
        regions overlap whenever the separator is not the first character --
        undefined behaviour, and the second instance of it found in this sweep.
        Only gcc-on-Linux reported it (-Wrestrict, and only at -O2); clang and
        both mingw targets compiled it silently. */
     if (s[ii]==PSEP) { memmove( s,&s[ii+1], (size_t)(len-ii) ); break; }
  } /* for */
} /* CutPath */

static os9err Open_Image( ushort pid, rbfdev_typ* dev, ptype_typ type, char* pathName, 
                          ushort mode )
{
    #define R0 "/r0"
    os9err   err;
    ushort   sp, sctSize;
    uint32_t len, iSize;
    ulong    tSize;
//  ulong   imgScts;  /* only used in commented-out debug prints */
    ulong   totScts;
    byte    bb[STD_SECTSIZE]; /* one sector */
    
    do {
        #ifndef RAM_SUPPORT
          char* q;
          ulong lr0= strlen( R0 );
          if (ustrcmp( mnt_name, "r0" )==0) return E_UNIT;
         
          len= strlen( pathName );
          if (ustrcmp( pathName, "r0" )==0 || len<lr0) return E_UNIT;
          q=           pathName + len-lr0;
          if (ustrcmp( q, R0 )==0 ) return E_UNIT;
        #endif
       
        err= syspath_open   ( pid, &sp, type,pathName,mode ); if (err) return err;
        err= syspath_gs_size( pid,  sp, &iSize );             if (err) break;
        if (!RBF_ImgSize( iSize )) { err= E_FNA; break; }
      
                                         len= sizeof(bb);
        err= syspath_read   ( pid,  sp, &len, &bb, false );   if (err) break;
        err= syspath_seek   ( pid,  sp,  0 );                 if (err) break;

        /* the "Cruz" mark, or a sector 0 a disk this size can have */
        if (!RBF_IsImage( bb, iSize )) { err= E_FNA; break; }
        totScts= GET_OS9L(bb, TOT_POS) >> BpB;
        sctSize= GET_OS9W(bb, SECT_POS);
        if (sctSize==0)                     sctSize= STD_SECTSIZE;
       
//      imgScts= iSize/sctSize;
      //upo_printf( "name1='%s' size=%d %d\n", pathname, imgScts, totScts );

             tSize= totScts*sctSize;
        /* 512, not 2048: the RBF/Cruz-magic check above already confirms this
         * is genuinely an RBF image, and iSize % sctSize below already
         * enforces whole-sector alignment -- 2048 additionally assumed every
         * real image lands on a CD/hard-disk block boundary, which rejects
         * legitimate smaller-format media (e.g. real CD-i-era 5.25" floppies,
         * 653824 bytes = 2554 x 256-byte sectors, not a multiple of 2048). */
        if ((tSize %     512)!=0 ||
             tSize <    8192     ||
            (iSize % sctSize)!=0) { err= E_FNA; break; }
      
      //upo_printf( "name2='%s' size=%d %d\n", pathname, imgScts, totScts );
               
        dev->sp_img= sp; /* do this before calling DevSize */
        err= DevSize( dev );
    } while (false);
    
    if (err) { syspath_close( 0, sp ); return err; }
    strcpy( dev->img_name,pathName );
    
    /* if not the complete name at the image */
    if     (ustrcmp( &dev->img_name[1],dev->name )==0) {
            strcpy (  dev->img_name, pathName  );
    } // if

    if     (ustrcmp( &dev->img_name[1],dev->name )==0) {
        strcpy     (  dev->img_name, startPath );
        MakeOS9Path(  dev->img_name );
        strcat     (  dev->img_name, pathName  );
        
        if   (!FileFound( dev->img_name )) {
            strcpy     (  dev->img_name, strtUPath );
            MakeOS9Path(  dev->img_name );
            strcat     (  dev->img_name, pathName  );
        }
    } // if
    
    EatBack( dev->img_name );
    return 0;                                                         
} /* Open_Image */

Boolean InstalledDev( const char* os9path, const char* curpath,
                      Boolean fullsearch, ushort *cdv )
/* expect OS-9 notation */
{
    char        tmp[OS9PATHLEN];
    rbfdev_typ* dev;
    char        *p, *dvn;
    int         ii;

    /* A pathlist that does not fit cannot name an installed image. Internal
       commands (mount, idbg -o) hand their raw arguments down to here, and a
       long one overran tmp (found by ASan). */
    if ((AbsPath(os9path) ? 0 : strlen( curpath )+1) + strlen( os9path ) >= sizeof(tmp))
        return false;

    strcpy( tmp,curpath  ); /* create the full pathname */
    strcat( tmp,PSEP_STR );
    if (AbsPath(os9path)) strcpy( tmp,"" );
    strcat( tmp,os9path  );
    
    for (ii=1; ii<MAXRBFDEV; ii++) { /* search for identical image */
            dev= &rbfdev[ ii ];
        if (dev->installed &&     /* test for same path or with sub path */
            SamePathBegin( tmp,dev->img_name )) { *cdv= ii; return true; }
    } /* for */

         p= tmp;
    if (*p==PSEP) p++;
    dvn= p;             /* this is what we are looking for  */
    while  (*p!=NUL ) { /* cut raw device name or sub paths */
        if (*p=='@' ) { *p= NUL; break; }
        if (*p==PSEP) {
            if (fullsearch) { dvn= p+1;       }
            else            { *p= NUL; break; }
        }
        p++;
    } /* while */

    if (*dvn==NUL) return false; /* no device */
    if (fullsearch && *mnt_name!=NUL) 
                  dvn= mnt_name; /* in case of mount, compare to this */

    for (ii=1; ii<MAXRBFDEV; ii++) {     /* search a device */
            dev= &rbfdev[ ii ];          /* get RBF device  */
        if (dev->installed && 
           (ustrcmp( dvn,dev->name  )==0 
         || ustrcmp( dvn,dev->name2 )==0
         || ustrcmp( dvn,dev->name3 )==0) &&
           (SamePathBegin( &tmp[1],dev->name  ) 
         || SamePathBegin( &tmp[1],dev->name2 )
         || SamePathBegin( &tmp[1],dev->name3 )
         || SamePathBegin(  tmp,   dev->img_name ))) { *cdv= ii; return true; }
    } /* for */

    return false;
} /* InstalledDev */

static Boolean MWrong( int cdv )
/* check if already installed on a different device */
{
  rbfdev_typ*  dev;
  int          ii;

  for (ii=1; ii<MAXRBFDEV; ii++) {       
        dev= &rbfdev[ ii ];             /* get RBF device  */
    if (dev->installed && ii!=cdv && (ustrcmp( mnt_name,dev->name  )==0 ||
                                      ustrcmp( mnt_name,dev->name2 )==0 ||
                                      ustrcmp( mnt_name,dev->name3 )==0)) return true;
  } /* for */
    
  return false;
} /* MWrong */

static Boolean ParseDiskSize( const char* s, uint32_t* sizeKBOut )
/* Parses a size string the same way os9main.c's -m/-mm option does
 * (os9main.c:754-791): a bare number is bytes; a trailing g/M/k
 * (case-insensitive) scales it up by 1024^3/1024^2/1024 first. Returns
 * the result in kBytes -- the same unit -r=<size> already uses. False on
 * malformed input (nothing parsed, or an unrecognized modifier letter). */
{
    unsigned long val;
    char          modifier= 0;

    if (sscanf( s,"%lu%c", &val,&modifier )<1) return false;

    switch (tolower((unsigned char)modifier)) {
        case 'g' : val*= 1024; /* fall into M */
        case 'm' : val*= 1024; /* fall into k */
        case 'k' : val*= 1024;
        case  0  : break;
        default  : return false;
    } // switch

    /* A bare number is BYTES, so anything under 1K divides down to 0 kBytes --
     * and 0 is the caller's sentinel for "make a host directory instead of an
     * image". Without this guard the two meanings collide silently: `mount
     * -k=32` asked for a 32-byte image and got a DIRECTORY named h9 (verified
     * before this guard existed). Only a literal 0 may reach that sentinel;
     * a nonzero size too small to round to even 1K is a malformed request. */
    if (val!=0 && val<KByte) return false;

    *sizeKBOut= (uint32_t)(val / KByte);
    return true;
} /* ParseDiskSize */

#define SectsPerTrack 0x20
#define DefaultScts   8192
#define MaxKB         0x001ffffe // 2097151 kB = 2047.999 MB -- shared cap for -r=<size> and -k=<size>

static Boolean RoundSectorCount( uint32_t ramSizeKB, uint32_t sctSize, int clu,
                                  uint32_t* totSctsOut, uint32_t* totBitsOut )
/* Converts a kBytes request into a valid sector count: rounds up to a whole
 * track, then to a whole allocation cluster. Falls back to DefaultScts if
 * the request rounds down to zero (e.g. ramSizeKB==0). Identical math to
 * PrepareRAM's original inline computation -- moved, not changed. Returns
 * false (after printing the reason) if clu isn't a power of 2 -- checked
 * HERE, before the division below that uses clu as a divisor, not in
 * BuildBlankImage (which runs after this and would divide by an invalid
 * or zero clu first if the check lived there instead). */
{
    uint32_t totScts, tracks, totBits;
    Boolean  ok= false;
    int      ii;

    for (ii=0; ii<31; ii++) { if (1<<ii==clu) { ok= true; break; } }
    if (!ok) {
      upe_printf( "mount: cluster size must be a power of 2\n" );
      return false;
    } // if

              totScts= ramSizeKB*KByte/sctSize; /* adapt to KBytes */
    tracks  = (totScts-1) / SectsPerTrack + 1;
              totScts=      SectsPerTrack * tracks;  /* granulate to tracks */
    totBits = (totScts-1) / clu + 1;
              totScts=      clu * totBits;           /* granulate to clusters */
    if        (totScts==0)  totScts= DefaultScts;

    *totSctsOut= totScts;
    *totBitsOut= totBits;
    return true;
} /* RoundSectorCount */

static void StampVolume( byte* base, uint32_t sctSize, const char* volName )
/* Overwrites the two identification-sector fields that RAM_zero cannot supply
 * honestly, because a template is a snapshot of one particular disk:
 *
 *   DD_DAT ($1A) -- the template's bytes say 02-07-03 20:45, which Microware's
 *     own `free` renders as "Jul 3, 1902". Every image ever built carried it.
 *   DD_NAM ($1F) -- the template reads "Ram Disk (Caution: Volatile)", which is
 *     true for `mount -r` and a lie for the persistent host file `mount -k`
 *     writes. <volName> NULL keeps it, which is what the RAM disk wants.
 *
 * Everything else in the template is real and stays: DD_SYNC ("Cruz", which
 * Open_Image requires), DD_MapLSN=1, DD_LSNSize, DD_VersID=1.
 *
 * Both writes are guarded on sctSize, for the same reason the RAM_zero memcpy
 * is: a -n= smaller than the field would otherwise write into sector 1. */
{
    struct tm tim;
    size_t    len;

    if (sctSize>=DAT_POS+5) {
        GetTim( &tim );
        base[ DAT_POS   ]= (byte)tim.tm_year;    /* already years since 1900 */
        base[ DAT_POS+1 ]= (byte)(tim.tm_mon+1); /* tm counts months from 0  */
        base[ DAT_POS+2 ]= (byte)tim.tm_mday;
        base[ DAT_POS+3 ]= (byte)tim.tm_hour;
        base[ DAT_POS+4 ]= (byte)tim.tm_min;
    } // if

    if (volName==NULL || *volName==NUL || sctSize<NAM_POS+NAM_LEN) return;

          len= strlen( volName );
    if   (len>NAM_LEN) len= NAM_LEN;
    memset( &base[NAM_POS], 0,       NAM_LEN );
    memcpy( &base[NAM_POS], volName, len );
    base[ NAM_POS+len-1 ] |= 0x80; /* OS-9 strings are high-bit terminated */
} /* StampVolume */

void HostDirLSN0( const char* hostpath, byte* sct )
/* The identification sector of a device that is a host DIRECTORY, which has
 * none.  Microware's C library stat() will not describe a file until it has
 * read one: it opens the file, asks SS_FD and SS_DevNm, then opens "/<dev>@"
 * and reads 256 bytes -- and bash's PATH search, `[ -f ]` and `[ -x ]` all go
 * through stat().  While that raw open answered E$Unit (063f8d1), no command
 * on a host-directory device could be found by name: `tmode` said "command
 * not found" with /h1/CMDS/tmode running fine.  Measured 2026-09-10 by
 * tracing both an RBF /h1 and a host-directory /h1 through the same test.
 *
 * The sector is the RAM-disk template (DD_SYNC "Cruz", DD_MapLSN, DD_LSNSize,
 * DD_VersID -- what Open_Image itself requires), with today's date, the host
 * filesystem's size in DD_TOT and the directory's own name in DD_NAM.  It is
 * the only sector such a device has: pFread answers E$EOF past it, so `free`
 * and `dcheck` stop at the allocation map instead of reading a directory as
 * a disk. */
{
    const char* nm;
    const char* p;
    char        volName[NAM_LEN+1];
    size_t      len;
    uint32_t    tot= 0;

    memcpy( sct, RAM_zero, STD_SECTSIZE<sizeof(RAM_zero) ? STD_SECTSIZE : sizeof(RAM_zero) );

    /* the last path component names the volume */
    nm= hostpath;
    for (p= hostpath; *p!=NUL; p++) if (*p==PSEP && p[1]!=NUL) nm= p+1;
          len= strlen( nm );
    if   (len>NAM_LEN) len= NAM_LEN;
    memcpy( volName, nm, len ); volName[len]= NUL;
    StampVolume( sct, STD_SECTSIZE, volName );

    /* DD_TOT: the host filesystem in 256-byte sectors, as pHdsize reports it,
       capped at the three bytes the field has */
    #ifdef MINGW
    {
      ULARGE_INTEGER totalBytes;
      if (GetDiskFreeSpaceExA( hostpath, NULL, &totalBytes, NULL ))
          tot= (uint32_t)(totalBytes.QuadPart / STD_SECTSIZE);
    }
    #else
    {
      struct statvfs st;
      if (statvfs( hostpath, &st )==0)
          tot= (uint32_t)((unsigned long long)st.f_blocks * st.f_frsize / STD_SECTSIZE);
    }
    #endif
    if (tot>0xFFFFFF) tot= 0xFFFFFF;
    sct[0]= (byte)(tot>>16); sct[1]= (byte)(tot>>8); sct[2]= (byte)tot;
} /* HostDirLSN0 */

static Boolean BuildBlankImage( uint32_t totScts, uint32_t totBits, uint32_t sctSize, int clu,
                                 byte** bufOut, uint32_t* headSctsOut, Boolean headOnly,
                                 const char* volName )
/* Builds the HEAD of a ready-to-use RBF filesystem image in a freshly
 * allocated buffer, and reports its length in sectors via <headSctsOut>.
 * With <headOnly>, allocates ONLY the non-zero prefix and reports its length
 * in <headSctsOut>; everything past it is zeros, so a caller writing to a FILE
 * streams that part instead of buffering it. This used to always allocate
 * sctSize*totScts -- the whole image -- in the 68k arena, which capped
 * `mount -k` at about 28 MB of a 32 MB arena. A RAM disk really does need the
 * whole buffer, so PrepareRAM passes headOnly=false and is unaffected.
 * Only sectors 0..(allocSize+2) are ever written: identification sector (Cruz-stamped, via RAM_zero,
 * then corrected by StampVolume), allocation bitmap, root directory FD sector, root directory entry.
 * Returns false (after printing the reason) if the allocation bitmap
 * doesn't fit in the available map size -- the caller owns *bufOut only on
 * true. Caller must already have validated clu (via RoundSectorCount) --
 * this function trusts it's a valid power of 2. */
{
    ulong   allocSize, allocN, mapSize, f, r, fN, rN, cluRest, ii;
    uint32_t headScts;
    byte*   b;
    int     v;
    byte    pt;
    byte*   base;

             mapSize= (totBits-1)/BpB + 1; // rounding up
    if      (mapSize>0xffff) {
      while (mapSize>0xffff) { mapSize= mapSize/2; clu= clu*2; }
      upe_printf( "mount: error - cluster size is too small for this device.\n" );
      upe_printf( "cluster size must be at least %d.\n", clu );
      return false;
    } // if

    allocSize= (totBits-1)/(sctSize*BpB) + 1; // nr of allocation sectors, rounded up
    allocN   =  allocSize * sctSize*BpB;      // nr of allocation bits

    /* head = identification sector + allocation bitmap + root FD + root dir.
     * r is the last sector this function touches, so r+1 sectors is exactly
     * the non-zero prefix of the image. */
    headScts= headOnly ? allocSize + 3 : totScts;
    if (headScts>totScts) headScts= totScts;
            base= get_mem( sctSize*headScts );
    if    ( base==NULL ) return false;
    /* memset(dst, VALUE, LENGTH) -- the value and length were transposed, making
     * this a zero-LENGTH memset that cleared nothing at all, despite the comment.
     * Harmless only because get_mem() happens to hand back zeroed arena pages;
     * the moment it didn't, a fresh disk would come up full of stale bytes. */
    memset( base, 0, sctSize*headScts ); // clear the head
    /* RAM_zero is 256 bytes. Copying sctSize of it read off the end of the
     * array for any larger sector: with -n=512, 119 bytes of whatever
     * followed it in the data segment (including module magic, 0x4AFC) were
     * copied into the identification sector's tail. Copy what exists; the
     * rest is already zero from the memset above. */
    memcpy( base,RAM_zero, sctSize<sizeof(RAM_zero) ? sctSize : sizeof(RAM_zero) );
    StampVolume( base, sctSize, volName );

    f= allocSize + 1; fN= f*sctSize; // root dir fd sector position
    r=         f + 1; rN= r*sctSize;

    cluRest=       r/clu + 1;
    cluRest= cluRest*clu - r;

    /* Reserve all these bits in the allocation map */
    pt= 0x80;
    for (ii=0; ii<allocN; ii++) {
      if  (ii<=r/clu || ii>=totBits) { // including fd + dir
        v= ii/BpB;
        b= &base[ sctSize + v ]; *b |= pt;
      } // if
      pt= pt/2; if (pt==0) pt= 0x80; /* prepare the next pattern */
    } // for

    SET_OS9L(base, TOT_POS,  totScts << BpB); /* 0x03 overwritten, is 0 anyway */
             base[ TRK_POS ]= SectsPerTrack;   /* number of sectors per track */
    SET_OS9W(base, MAP_POS,  mapSize);
    SET_OS9W(base, BIT_POS,  clu);
    SET_OS9L(base, DIR_POS,  f << BpB);       /* 0x0b overwritten, is 0 anyway */
    SET_OS9W(base, SECT_POS, sctSize);

                 base[ fN      ]= 0xbf; /* prepare the fd sector */
                 base[ fN+0x08 ]= 0x01;
                 base[ fN+0x0C ]= 0x40;
    SET_OS9L(base, fN+0x10,  r << BpB);       /* fN+0x14 overwritten, is 0 anyway */
                 base[ fN+0x14 ]= cluRest;

                 base[ rN      ]= 0x2e; /* prepare the directory entry */
                 base[ rN+0x01 ]= 0xae;
    SET_OS9W(base, rN+0x1e,  f);
                 base[ rN+0x20 ]= 0xae;
    SET_OS9W(base, rN+0x3e,  f);

    *bufOut     = base;
    *headSctsOut= headScts;
    return true;
} /* BuildBlankImage */

// #ifdef RAM_SUPPORT
static os9err PrepareRAM( ushort pid, rbfdev_typ* dev, char* cmp )
{
    uint32_t ramHead;   /* unused: a RAM disk buffers the whole image */
    os9err    err, cErr;
    uint32_t  iSize;
    uint32_t  totBits;
    int       clu= mnt_cluSize;
    mod_dev*  mod;
    char*     p;
    ptype_typ type;
    ushort    sp;

    if (strcmp( mnt_devCopy,""  )!=0) {
      strcat  ( mnt_devCopy,"@" );
      type= IO_Type        ( pid,            mnt_devCopy, poDir );
      err = syspath_open   ( pid, &sp, type, mnt_devCopy, poDir ); if (err) return err;
      err = syspath_gs_size( pid,  sp, &iSize );

      if (!err && iSize>0) {
             dev->ramBase= get_mem( iSize );
        if ( dev->ramBase==NULL ) return E_NORAM;
        err= syspath_read( pid, sp, &iSize, dev->ramBase, false );
      } // if

      cErr= syspath_close( pid, sp ); if (!err) err= cErr;
      if (err) return err;

      dev->totScts    = GET_OS9L(dev->ramBase, TOT_POS) >> BpB;
                        dev->imgScts    = dev->totScts;
      dev->clusterSize= GET_OS9W(dev->ramBase, BIT_POS);
      dev->sctSize    = GET_OS9W(dev->ramBase, SECT_POS);
                                            dev->sas        = DD__MINALLOC;
      /* copied from an image, whose sector size is its own claim: checked as
         RootLSN checks it, before any path's buffer is filled to that size */
      if (dev->sctSize!=0) { err= CheckSectorSize( dev->sctSize ); if (err) return err; }
      return 0;
    } // if

    /* MaxKB is the ceiling for a FILE image, where the only real limit is host
     * disk space. A RAM disk is carved from the 68k arena in one piece, so its
     * true ceiling is smaller by two orders of magnitude -- the exact check is
     * below, once the rounded sector count is known. This one only catches the
     * absurd early, before the arithmetic. */
    if (mnt_ramSize>MaxKB) {
      upe_printf( "mount: error - size is too large for this device.\n" );
      return 1;
    } // if

    if (mnt_sctSize>0) { err= CheckSectorSize( mnt_sctSize ); if (err) return err;
                         dev->sctSize    = mnt_sctSize; }
                         dev->clusterSize= clu;
                         dev->sas        = DD__MINALLOC;

    if (!RoundSectorCount( mnt_ramSize, dev->sctSize, clu, &dev->totScts, &totBits ))
      return E_NORAM; /* RoundSectorCount already printed the specific reason */

    if ( mnt_ramSize==0
      && IsDesc( cmp, &mod, &p )
      && ustrcmp( p,"RBF" )==0 ) {
        p= (char*)mod + os9_word(mod->_mpdev);
        if (ustrcmp( p,"ram" )==0) {
                /* &mod->_mdtype is a pointer TO THE 128-byte ARRAY, so
                 * "+ PD_SCT" was advancing by PD_SCT*128 bytes, not
                 * PD_SCT -- mod->_mdtype decays to a plain byte pointer
                 * and gives the intended byte offset, matching GET_OS9W's
                 * own (buf, off) idiom used throughout this function. */
                dev->totScts= GET_OS9W( mod->_mdtype, PD_SCT );
        } // if
    } // if

    dev->imgScts= dev->totScts;

    /* Refuse what the arena cannot hold, naming the constraint. Without this
     * the request is accepted, get_mem fails deep inside BuildBlankImage, and
     * the user saw a bare "No more memory !!!" -- the same line an exhausted
     * memtable printed, and the same line the -M option's own bug produced, so
     * three unrelated causes were indistinguishable. Now it would be just
     * E$NoRAM, which says even less; this names the constraint. */
    {
        uint64_t need = (uint64_t)dev->totScts * dev->sctSize;
        ulong    avail= emul_arena_free();

        if (need>avail) {
          upe_printf( "mount: error - a %u kB RAM disk needs %u kB of the 68k arena, "
                      "which has %u kB left.\n",
                      (uint32_t)mnt_ramSize,
                      (uint32_t)(need/KByte + (need%KByte!=0)), (uint32_t)(avail/KByte) );
          upe_printf( "A RAM disk lives in emulated RAM; use mount -k=<size> for a disk "
                      "image instead, which does not.\n" );
          return E_NORAM;
        } // if
    }

    /* NULL: a RAM disk really IS volatile, so the template's own name is the
     * one true thing about it. Only the date gets corrected. */
    if (!BuildBlankImage( dev->totScts, totBits, dev->sctSize, clu, &dev->ramBase, &ramHead, false, NULL ))
      return E_NORAM;

    strcpy( dev->img_name,cmp );
    return 0;
} /* PrepareRAM */
// #endif

static os9err DeviceInit( ushort pid, rbfdev_typ** my_dev, syspath_typ* spP, 
                          ushort cdv, char* pathname, char* curpath, ushort mode, Boolean *new_inst )
/* Make Connection to SCSI system or to an TBF image file */ 
{
    os9err       err= 0;
    char         cmp[OS9PATHLEN],
                 ali[OS9PATHLEN],
                 tmp[OS9PATHLEN],
                 imgpath[OS9PATHLEN],
                 /* "  #000:%03d" is 7 fixed chars + up to 5 digits (err is a
                  * ushort) + NUL = 13, which never fit in the old [12] */
                 ers[16], *q, *p, *v;
    rbfdev_typ*  dev;
    ptype_typ    type;
    int          ii, n;
    Boolean      abs, isSCSI, isRAMDisk= false, wProtect;
    Boolean      isFolder;
    
    process_typ* cp    = &procs[pid];
    
    int          scsiID    = NO_SCSI;
    short        scsiAdapt = -1;
    short        scsiBus   = 0;
    short        scsiLUN   = 0;
    ushort       scsiSsize = STD_SECTSIZE;
    ushort       scsiSas   = 0;
    byte         scsiPDTyp = 0;
    
    Boolean      imgIsHost= false; /* <imgpath> holds a resolved HOST path */
    Boolean      mock  = *mnt_name!=NUL;
    Boolean      fu    = spP->fullsearch;
    Boolean      fum   = fu && mock;
    Boolean      opened= false;
        
    #ifdef MACFILES
      FSSpec fs, afs;
    #elif defined win_unix
      char rbfname[OS9PATHLEN];
      char rbfhost[OS9PATHLEN];
    #endif

    do {
        *new_inst= false;
        if (fu) {
            strcpy( tmp,curpath  ); /* create the full pathname */
            strcat( tmp,PSEP_STR );
            if (AbsPath(pathname)) strcpy( tmp,"" );
            strcat( tmp,pathname );
            strcpy( pathname,tmp );
        }

        strcpy( ali,""       ); /* no alias defined by default */
        strcpy( cmp,pathname ); /* default in case of error */
        strcpy( imgpath,pathname ); /* host path for Open_Image; overridden on win_unix */

            abs=        AbsPath( pathname );
        if (abs && InstalledDev( pathname,curpath, fu, &cdv )) {
            dev= &rbfdev[cdv];
     	    debugprintf(dbgFiles,dbgNorm,("# DevInit: cdv=%d path='%s' img='%s'\n", 
     	                                     cdv, pathname,dev->img_name ));
            if                 (!mock 
              ||        ustrcmp( mnt_name,dev->name  )==0
              ||        ustrcmp( mnt_name,dev->name2 )==0
              ||        ustrcmp( mnt_name,dev->name3 )==0
              || !SamePathBegin( pathname,dev->img_name )) break;
            
            /* already installed with a different name */
            if (*dev->name2==NUL) { strcpy( dev->name2,mnt_name ); break; }
            if (*dev->name3==NUL) { strcpy( dev->name3,mnt_name ); break; }
            return E_DEVBSY;
        } // if
        
        /* existing relative path ? */
        if (!abs && cdv!=0 && mnt_scsiID==NO_SCSI) break;

        err      =     0;
        isRAMDisk= false; /* do it for all conditions */
        
        if (!fu) {
            #ifdef RAM_SUPPORT
              isRAMDisk= RAM_Device( pathname );
            #endif
                 
            if (!isRAMDisk) {
                #ifdef MACFILES
                  err= GetRBFName( pathname,mode, &isFolder, &fs,&afs );
                #elif defined win_unix
                  err= GetRBFName( pathname,mode, &isFolder, (char*)&rbfname, (char*)&rbfhost );
                #endif

                /* must open it in the right mode */
                if (err==E_FNA && !IsDir(mode)) return err;
            }
        }

        if (isRAMDisk) {
                 isSCSI= false; /* don't forget to set a default */
            GetOS9Dev( pathname, (char*)&cmp );
        }
        else {
                 isSCSI= (err && mnt_scsiID!=NO_SCSI);
            if  (isSCSI) {
                scsiID   = mnt_scsiID;
                scsiAdapt= mnt_scsiAdapt>=0 ? mnt_scsiAdapt : defSCSIAdaptNo;
                scsiBus  = mnt_scsiBus  >=0 ? mnt_scsiBus   : defSCSIBusNo;    
                scsiLUN  = mnt_scsiLUN  >=0 ? mnt_scsiLUN   : 0;    
            }
            else isSCSI= (err && SCSI_Device( pathname, &scsiAdapt, &scsiBus, &scsiID, &scsiLUN, 
                                                        &scsiSsize, &scsiSas, &scsiPDTyp, &type ));
            if  (isSCSI)           GetOS9Dev( pathname, (char*)&cmp );
            else {
                if (fu) CutPath( cmp );
                else {
                    #ifdef MACFILES
                      /* this is the correct way to get pascal strings back */
                      if (!err) { /* GetRBFName called earlier already */
                          memcpy( &ali, &afs.name, sizeof(ali) );
                          p2cstr ( ali );
                      }
                      else {
                          GetOS9Dev( pathname, (char*)&cmp );
                              err= getFSSpec( 0,cmp, _start, &fs );
                          if (err) return err;
                      }
                  
                      memcpy   ( &cmp, &fs.name, sizeof(cmp) );
                      p2cstr    ( cmp );
                      if (strcmp( cmp,ali )==0) strcpy( ali,"" );
            
                    #elif defined win_unix
                      if (err) return E_UNIT; /* GetRBFName called earlier */

                      /* The device is named after the OS-9 path that asked for it
                         ("/h9", "/h9@", "/h9/CMDS" -> "h9"), and its image is the
                         host file GetRBFName already resolved.  Both used to be
                         derived from that host file's BASENAME instead: the name
                         was the basename, and the image path was rebuilt by feeding
                         "/" + basename back through parsepath().  That round trip
                         only returns to the same file when the basename is itself a
                         device name, i.e. exactly when the image sits at
                         <startPath>/hN.  Point OS9Hx at /somewhere/img.rbf and the
                         device registered as "img.rbf" while the image path became
                         the meaningless OS-9 path "/img.rbf", so Open_Image answered
                         E$FNA for every access -- while host DIRECTORIES, which never
                         come through here, honoured OS9Hx from anywhere.  Renaming
                         the same file to "h9" made it work, which is what identified
                         the decision as lexical.  GetOS9Dev needs an absolute path to
                         read a device out of; a relative one keeps the old basename. */
                      GetOS9Dev( pathname, (char*)&cmp );
                      if (*cmp==NUL) strcpy( cmp,rbfname );
                      strcpy( imgpath,rbfhost );
                      imgIsHost= true; /* a host path now -- see the IO_Type call */

                    #else
                      /* %%% some fixed devices defined currently */
                      GetOS9Dev( pathname, (char*)&cmp );
                      if      (ustrcmp( cmp,"mt")==0) cdv= 1; 
                      else if (ustrcmp( cmp,"c1")==0) cdv= 2;
                      else if (ustrcmp( cmp,"c2")==0) cdv= 3;
                      else if (ustrcmp( cmp,"c3")==0) cdv= 4;
                      else if (ustrcmp( cmp,"dd")==0) cdv= 5;
                      else return E_UNIT;
            
                      break;
                    #endif
                }
            } /* if isSCSI */
        } /* if !isRAMDisk */

        if (!AbsPath(pathname)) {
            if (IsExec(mode)) strcpy( tmp, cp->x.path );
            else              strcpy( tmp, cp->d.path );
            if (*tmp!=NUL)    strcat( tmp, PSEP_STR ); /* create the full path name */

            strcat( tmp,pathname );
            strcpy( pathname,tmp );
        }

        cdv= 0;                              /* now search a device */
        for (ii=1; ii<MAXRBFDEV; ii++) {       
                 dev= &rbfdev[ ii ];             /* get RBF device  */
            if (!dev->installed) {
                if (cdv==0) cdv= ii;           /* reserve empty one */
            }
            else {
               /* is there already a file with the same file name ? */
     			      debugprintf(dbgFiles,dbgNorm,("# DevInit: path='%s' img='%s'\n", 
     			                                 pathname,dev->img_name ));
                if (SamePathBegin( pathname,dev->img_name)) {
                	if (strcmp( cmp,dev->name )!=0) {
                    	strcpy( cmp,dev->name ); /* take existing name */
                    	*new_inst= true;        /* make a new instance */
                    }
                    
                    cdv= ii; break;
                }
            
                /* compare with mnt_name, if available */
                             q= cmp;
                if (mock)    q= mnt_name;
                if (ustrcmp( q,dev->name  )==0
                 || ustrcmp( q,dev->name2 )==0
                 || ustrcmp( q,dev->name3 )==0) { cdv= ii; break; }

                if (isSCSI && IsSCSI( dev ) &&
                    scsiID == dev->scsi.ID) {
                    if (*dev->name2==NUL) { strcpy( dev->name2,q ); cdv= ii; break; }
                    if (*dev->name3==NUL) { strcpy( dev->name3,q ); cdv= ii; break; }
                    return E_DEVBSY;
                }
            }
        } /* for */

        if (cdv==0) return E_UNIT; /* no valid device found */
    } while (false);

             dev= &rbfdev[ cdv ]; 
    *my_dev= dev;                      /* activate RBF device */
    strcpy ( dev->alias,ali );         /* can change all the time */
    if     (!dev->installed) {
      v= strstr( cmp,"@" ); if (v!=NULL) *v= NUL;
      strcpy( dev->name,cmp );
    } // if
    
             q= dev->name; /* compare with mnt_name, if available */
    if (fum) q= mnt_name;
    debugprintf(dbgFiles,dbgNorm,("# DevInit: '%s' (%d%s%s)\n", 
                                     q, cdv, dev->installed ? "/installed":"",
                                             *new_inst      ? "/new_inst" :"" ));

  //upo_printf( "DEVICEINIT='%s' img=%d tot=%d %d\n", pathname, dev->imgScts, dev->totScts, dev->installed );
    if (dev->installed) {
    	if (!abs) return 0; /* ok, if not absolute */
    	if (mock && MWrong( cdv )) return E_DEVBSY;
    	
    	strcpy  ( tmp,pathname );
    	n= strlen(tmp)-1;
    	while   ( tmp[n]=='@' ) tmp[n--]= NUL;
      p=        tmp+1;
      if (SamePathBegin( tmp,dev->img_name ) ||
          SamePathBegin( p,  dev->name     ) ||
          SamePathBegin( p,  dev->name2    ) ||
          SamePathBegin( p,  dev->name3    )) return 0;
        
        /* already in use !! */    
  //  printf( "path='%s' img='%s'\n", pathname,dev->img_name );
  //  printf( "path='%s'  n1='%s'\n", pathname,dev->name  );
  //  printf( "path='%s'  n2='%s'\n", pathname,dev->name2 );
  //  printf( "path='%s'  n3='%s'\n", pathname,dev->name3 );
      return E_DEVBSY;
   	} /* if installed */
	
    dev->nr        = cdv;          /* already done ?? */
    dev->wProtected= mnt_wProtect; /* if not otherwise defined */
    dev->imgMode   = mnt_imgMode;
    dev->fProtected= false;
    dev->multiSct  = true;         /* is now supported */
    dev->currPos   = UNDEF_POS;    /* to make access faster: no seeks all the time */
    dev->last_alloc= 0;            /* initialize allocater pointer */
    dev->sp_img    = 0;            /* the image syspath will be connected here later */
    dev->imgScts   = 0;
    dev->totScts   = 0;            /* set default values */
   
 // #ifdef RAM_SUPPORT
    dev->isRAM   = isRAMDisk;      /* RAM disk flag */
 // #endif
   
    strcpy( dev->img_name,"" );    /* no  image name by default */
    strcpy( dev->name2,   "" );    /* no  2nd   name by default */
    strcpy( dev->name3,   "" );    /* no  3rd   name by default */

    dev->rMiss     = 0;            /* reset device statistics */
    dev->rTot      = 0;
    dev->wMiss     = 0;
    dev->wTot      = 0;

    dev->scsi.adapt= scsiAdapt;
    dev->scsi.bus  = scsiBus;
    dev->scsi.ID   = scsiID;
    dev->scsi.LUN  = scsiLUN;  
    dev->pdtyp     = scsiPDTyp;
    
    if (IsSCSI(dev)) {
        dev->sctSize= scsiSsize;
        dev->sas    = scsiSas;
    }
    else {
        dev->sctSize= STD_SECTSIZE;       /* as OS-9 <=V2.3: const sectsize 256 bytes */
        if (ustrcmp(dev->name,"dd")==0) dev->sas= DD__MINALLOC; /* use default value  */
        else                            dev->sas= RBF_MINALLOC; /* smaller for others */
    }

    // make sure we get a buffer that is big enough for reading any sector 0 (will be adjusted later)
    dev->tmp_sct= get_mem( dev->sctSize>MIN_TMP_SCT_SIZE ? dev->sctSize : MIN_TMP_SCT_SIZE );
    
    debugprintf(dbgFiles,dbgNorm,("# RBF open: trying to open %s \"%s\"\n",
                                     IsSCSI(dev) ? "SCSI":"RBF Image", q));

    /* %%% currently hardwired */
    if (ustrcmp( cmp,"c0")==0) dev->fProtected= false;
    if (ustrcmp( cmp,"c1")==0) dev->fProtected= false;

//  if (ustrcmp( cmp,"c0")==0 ||
//      ustrcmp( cmp,"hx")==0) dev->multiSct= true;

    do {
        err= 0;                 /* set it as default   */
        dev->installed= true;   /* now it is installed */
        #ifdef RBF_CACHE
          Flush_FDCache( dev->name );
        #endif

     // #ifdef RAM_SUPPORT
        if (isRAMDisk) { 
          err= PrepareRAM( pid, dev, cmp );
          break; /* no more actions for RAM disk */
        } // if
     // #endif
        
        if (IsSCSI(dev)) break; /* no more actions for SCSI */
            
        /* <imgpath> is the HOST path GetRBFName resolved, so it must not be
         * classified by IO_Type -- that parses its argument as an OS-9 path.
         * The two spellings only agree by luck: "/deep/img" cannot be a device
         * name so it survives the round trip, but a single-component "/zz" is
         * exactly the shape of one, and parsepathext duly resolved it as device
         * "zz" at <startPath>/zz -- a different file, usually absent. IO_Type
         * then answered fNone and Open_Image failed E$MNF, which is why an
         * image sitting DIRECTLY IN THE FILESYSTEM ROOT was unusable from any
         * working directory while the same image one level down worked.
         * GetRBFName has already established this is a readable file carrying a
         * valid OS-9 identification sector, so fFile is what it is. The `mount`
         * path (fu) leaves <imgpath> as the OS-9 path it was given, and still
         * needs the lookup. */
        type    = imgIsHost ? fFile : IO_Type( pid, imgpath, poRead );
        wProtect= mnt_wProtect;

        /* inherit write protection to sub device */
        if (!wProtect && type==fRBF && InstalledDev( pathname,curpath, false, &cdv ))
             wProtect= rbfdev[cdv].wProtected;

        /* try to open in read/write mode first (if not asking for wProtection */
        /* if not possible, open it readonly */
        img_hostPath= imgIsHost; /* lowered again immediately below */
        if (!wProtect) {
               err= Open_Image( pid,dev, type,imgpath, poUpdate );
          if (!err) { img_hostPath= false; opened= true; break; }
        }
               err= Open_Image( pid,dev, type,imgpath, poRead );
               img_hostPath= false;
        if   (!err) { opened= true; dev->wProtected= true; }
    } while (false);

    do {                                    if (err) break;
        err= RootLSN( pid,dev, spP, true ); if (err) break;
        dev->last_diskID= spP->u.rbf.diskID;
        *new_inst= true;
    } while (false);

    if (!err) {
      if ((dev->imgMode==Img_Reduced ||
          (dev->imgMode==Img_Unchanged && dev->imgScts<dev->totScts)) && 
          !dev->isRAM && !IsSCSI( dev )) {
        err= GetTop( pid, dev );
      //upo_printf( "TOPALLOC=%d of %d err=%d\n", dev->imgScts, dev->totScts, err );
      } // if
      
      if (dev->imgMode==Img_FullSize) {
        err= GetFull( pid, dev );
      } // if
    } // if

    if (err) {
      if (opened) ReleaseIt( pid, dev );
      
      dev->installed= false;
      release_mem( dev->tmp_sct );
                   dev->tmp_sct= NULL;
    } // if
        
    snprintf( ers,sizeof(ers),"  #000:%03d", err );
    debugprintf(dbgFiles,dbgNorm,("# RBF open: \"%s\" (%d%s%s)\n", 
                                     cmp, cdv, dev->installed ? "/installed":"", 
                                     err ? ers:""));
    return err;
} /* DeviceInit */

static void mount_usage( char* name, _pid_ )
{
    upe_printf( "Syntax:   %s <image_file> [<device>]\n", name );
    upe_printf( "Function: mount an RBF image file\n" );
    upe_printf( "Options:  \n" );
    upe_printf( "    -w           open with write protection on\n" );
    upe_printf( "    -i           adapt for reduced image size\n" );
    upe_printf( "    -f           adapt for full    image size\n" );
    
    #ifdef windows32
    upe_printf( "    -ah          show all SCSI devices on all adapters and buses\n" );
    upe_printf( "    -a=<adapter> specify  SCSI adapter\n" );
    upe_printf( "    -b=<scsibus> specify  SCSI bus\n" );
    #endif
    
    upe_printf( "    -s=<scsiID>  connect to SCSI ID\n" );
    upe_printf( "    -l=<lun>     connect to LUN (default=0)\n" );
    upe_printf( "    -r=<size>    create RAM disk with size (in kBytes)\n" );
    upe_printf( "    -n=<bytes>   sector  size in bytes     for RAM disk\n" );
    upe_printf( "    -c=<num>     cluster size (default: 1) for RAM disk\n" );
    upe_printf( "    -d=<device>  create RAM disk as a copy of <device>\n" );
    upe_printf( "    -k=<size>    create blank hX device (K/M/G suffix; 0 = host dir)\n" );
    upe_printf( "    -v=<name>    volume name for -k (default: the device name)\n" );
} /* mount_usage */

static os9err CreateBlankDevice( ushort pid, const char* name, uint32_t sizeKB,
                                             int sctSizeArg, int cluSizeArg,
                                             const char* volNameArg )
/* Creates a new hX device at <startPath>/hX -- either a fully-formatted
 * blank RBF image (sizeKB>0) or a plain host directory (sizeKB==0).
 * Refuses if a file/dir already exists at that path. <name> may be given
 * with or without a leading '/' ("h7" or "/h7"), and only h0..hz is valid
 * -- /dd is fixed at boot via OS9DISK and is never a valid target.
 * The volume name defaults to the device's own name ("h7"); -v=<name>
 * overrides it. It is NOT the RAM disk's name -- see StampVolume.
 * Reuses the file-scope MaxKB defined above RoundSectorCount (Task 2, Step 1). */
{
    char      hostpath[OS9PATHLEN];
    char      volName[NAM_LEN+1];
    const char* p= name;
    byte*     buf;
    uint32_t  totScts, totBits, headScts;
    Boolean   ok;
    uint32_t  sctSize= (sctSizeArg>0) ? (uint32_t)sctSizeArg : STD_SECTSIZE;
    int       clu    = (cluSizeArg>0) ? cluSizeArg           : 1;
    FILE*     fp;

    if (*p==PSEP) p++; /* allow a leading '/' */
    if (tolower((unsigned char)p[0])!='h' || p[1]==NUL || p[2]!=NUL || !isalnum((unsigned char)p[1]))
      return _errmsg( E_BPNAM, "mount -k: device must be h0..hz, got \"%s\".\n", name );

    strcpy( hostpath,startPath );
    if (hostpath[strlen(hostpath)-1]!=PATHDELIM) strcat( hostpath,PATHDELIM_STR );
    strncat( hostpath,p,2 );

    if (FileFound( hostpath ) || PathFound( hostpath ))
      return _errmsg( E_CEF, "mount -k: '%s' already exists -- remove it first.\n", hostpath );

    if (sizeKB==0) { /* -k=0 : plain host directory */
      #ifdef windows32
        if (!CreateDirectory( hostpath,NULL ))
          return _errmsg( E_BPNAM, "mount -k: can't create directory '%s'.\n", hostpath );
      #elif defined MINGW
        if (mkdir( hostpath )!=0) /* no POSIX mode bits on Windows */
          return _errmsg( E_BPNAM, "mount -k: can't create directory '%s'.\n", hostpath );
      #else
        if (mkdir( hostpath,0x01c0 )!=0)
          return _errmsg( E_BPNAM, "mount -k: can't create directory '%s'.\n", hostpath );
      #endif
      upo_printf( "mount: created host directory '%s'\n", hostpath );
      return 0;
    } // if

    if (sizeKB>MaxKB)
      return _errmsg( E_BPNAM, "mount -k: size is too large for this device.\n" );

    if (!RoundSectorCount( sizeKB, sctSize, clu, &totScts, &totBits ))
      return E_NORAM; /* RoundSectorCount already printed the specific reason */

    /* Default the volume name to the device's own ("h7"), which is at least
     * true, rather than leaving the template's "Ram Disk (Caution: Volatile)"
     * on a file that is neither RAM nor volatile. */
    memset( volName,0, sizeof(volName) ); /* strncpy below never NUL-terminates */
    if (volNameArg!=NULL && *volNameArg!=NUL) strncpy( volName,volNameArg, sizeof(volName)-1 );
    else                                      strncpy( volName,p,          2 );

    if (!BuildBlankImage( totScts, totBits, sctSize, clu, &buf, &headScts, true, volName ))
      return E_NORAM; /* BuildBlankImage already printed the specific reason */

    fp= fopen( hostpath,"wb" );
    if (fp==NULL) { release_mem( buf ); return _errmsg( E_BPNAM, "mount -k: can't create '%s'.\n", hostpath ); }

    /* Head first, then the all-zero tail in chunks. Buffering the whole image
     * is what limited this to ~28 MB; the tail carries no information, so
     * there is nothing to hold in memory for it. */
    ok= fwrite( buf, sctSize, headScts, fp )==headScts;
    release_mem( buf );

    if (ok) {
      byte     zeros[ 4096 ];
      uint32_t left= totScts - headScts;
      memset( zeros, 0, sizeof(zeros) );
      while (ok && left>0) {
        uint32_t chunk= sizeof(zeros)/sctSize;   /* whole sectors per write */
        if (chunk==0)   chunk= 1;                /* sctSize > 4096: one at a time */
        if (chunk>left) chunk= left;
        ok= fwrite( zeros, sctSize, chunk, fp )==chunk;
        left -= chunk;
      } // while
    } // if

    if (!ok) {
      fclose( fp ); remove( hostpath );
      return _errmsg( E_BPNAM, "mount -k: write failed for '%s' (disk full?).\n", hostpath );
    } // if
    fclose( fp );

    upo_printf( "mount: created '%s' (%u sectors, %u bytes/sector)\n", hostpath, totScts, sctSize );
    return 0;
} /* CreateBlankDevice */

Boolean RBF_ImageOpenUnder( const char* hostpath )
/* Is a mounted RBF image's file <hostpath> or somewhere below it? The device
   keeps the image open by its host path, so renaming anything above it would
   leave the mount naming nothing. RAM disks and SCSI devices have no file. */
{
    size_t len= strlen( hostpath );
    int    ii;

    for (ii=0; ii<MAXRBFDEV; ii++) {
        rbfdev_typ* dev= &rbfdev[ ii ];
        const char* img;

        if (!dev->installed || dev->isRAM || IsSCSI( dev ) ||
             dev->sp_img==0 || dev->sp_img>=MAXSYSPATHS) continue;
        img= syspaths[ dev->sp_img ].fullName;
        if (ustrncmp( img,hostpath,(ushort)len )==0 &&
            (img[ len ]==NUL || img[ len ]==PATHDELIM)) return true;
    }
    return false;
} /* RBF_ImageOpenUnder */

os9err MountDev( ushort pid, char* name, char* mnt_dev, char* devCopy, short adapt,
                             ushort scsibus, short scsiID, ushort scsiLUN, 
                             int ramSize, int sctSize, int cluSize, 
                             Boolean wProtect, int imgMode )
{
    os9err    err= 0;
    char      tmp[OS9NAMELEN];
    char      ch;
    int       ii;
    ptype_typ type;
    ushort    sp;

    /* it might come as Mac or DOS path name ... --> OS-9 notation */
    /* commented out: wil be done at "filestuff.c" */
//  MakeOS9Path( name );

    /* Is there a different name for the mount device ? */
    /* OS9exec can't switch the task in-between */
    /* NOTE: mnt_name is only valid within this context here */
    if (*mnt_dev!=NUL || scsiID!=NO_SCSI || ramSize>0 || strcmp( devCopy,"" )!=0) {
        mnt_name   = mnt_dev;
        mnt_ramSize= ramSize;
        mnt_sctSize= sctSize;
        mnt_cluSize= cluSize;
        mnt_devCopy= devCopy;
        
        if (scsiID!=NO_SCSI) {
            mnt_name     = name;
            mnt_scsiID   = scsiID;
            mnt_scsiAdapt= adapt;
            mnt_scsiBus  = scsibus;
            mnt_scsiLUN  = scsiLUN;
            
            if (!AbsPath(name)) { /* make a device path */
                strcpy ( tmp,PSEP_STR );
                strncat( tmp,name, OS9NAMELEN-1 );
                name=    tmp;
            } // if
        } // if
        
        if (AbsPath(mnt_name)) mnt_name++;
        for (ii=0; ii<OS9NAMELEN; ii++) {
                ch= mnt_name[ii];
            if (ch== NUL)                 break;   /* ok */
            if (ch==PSEP) { err= E_FNA;   break; } /* no more path separators allowed */
            if (ch < '0' &&
                ch!= '.') { err= E_BPNAM; break; } /* illegal tokens in path name */
        } /* for */
        
        if (err) name= mnt_name; /* for error output */
    } /* if */

    mnt_wProtect= wProtect;
    mnt_imgMode = imgMode;
    
    if (!err) {                  type= fRBF;
        if (mnt_scsiID==NO_SCSI) type= IO_Type( pid,name, poDir );
        
        debugprintf(dbgUtils,dbgNorm,("# mount: name='%s', mnt_name='%s' type='%s'\n", 
                                          name, mnt_name,TypeStr(type) ));
        err= syspath_open( pid, &sp,type, name, poDir );
        debugprintf(dbgUtils,dbgNorm,("# mount: name='%s', mnt_name='%s' err=%d\n",
                                          name, mnt_name,err ));
    } // if

    mnt_wProtect= false; /* switch them off again */
    mnt_imgMode = Img_Unchanged;
    mnt_name    = "";
    mnt_scsiID  = NO_SCSI;
    mnt_ramSize = 0;
    mnt_sctSize = 0;
    mnt_cluSize = 1;
    mnt_devCopy = "";
    
    if   (!err) err= syspath_close( pid, sp );
    return err;
} /* MountDev */

static Boolean IsHDevice( const char* name )
/* "h1" or "/h1": a letter /hX can name. */
{
    if (*name==PSEP) name++;
    return tolower( (unsigned char)name[ 0 ] )=='h' && isalnum( (unsigned char)name[ 1 ] ) &&
           name[ 2 ]==NUL;
} /* IsHDevice */

static os9err MountImageAs( ushort pid, char* image, char* devName, Boolean wProtect, int imgMode )
/* `mount <image> hX`: attach an RBF image file as /hX while running.
 *
 * This form never worked: MountDev took the device name from the image's own
 * path (`mount /h5/disk.dsk h1` became a device "h5" and then looked for
 * disk.dsk INSIDE it), and a relative image was classed by the directory it sat
 * in and opened as one (E$FNA). No test mounted an existing image, so nothing
 * noticed. Rather than untangle DevInit, the image is attached the way OS9Hx
 * attaches one at startup (TwoCharDev consults SetMountedImage first), and then
 * opened through the ordinary device path, so -w still applies.
 *
 * <image> is an OS-9 path to a file on a host directory, or else a host path. */
{
    char        host[OS9PATHLEN];
    char        devPath[4];
    char        tmp [OS9PATHLEN];
    char*       root= NULL;
    char*       p   = image;
    char        letter;
    os9err      err;

    if (*devName==PSEP) devName++;
    letter= (char)tolower( (unsigned char)devName[ 1 ] );

    if (parsepath( pid,&p,host,false )!=0 || !FileFound( host )) {
        if (!FileFound( image )) return _errmsg( E_PNNF, "can't find the image \"%s\".\n", image );
        snprintf( host,sizeof(host), "%s", image );
    } // if

    TwoCharDev( devName,&root,tmp );
    if (root!=NULL) return _errmsg( E_DEVBSY, "/%s is already a device.\n", devName );

    SetMountedImage( letter,host );
    snprintf( devPath,sizeof(devPath), "/h%c", letter );
    err= MountDev( pid, devPath, "", "", defSCSIAdaptNo, defSCSIBusNo, NO_SCSI, 0,
                   0, 0, 1, wProtect, imgMode );
    if (err) {
        SetMountedImage( letter,NULL );
        return _errmsg( err, "can't mount \"%s\" as /%s.\n", image, devName );
    } // if
    return 0;
} /* MountImageAs */

os9err int_mount( ushort pid, int argc, char** argv )
/* mount an RBF image partition file */
{
    os9err    err     = 0;
    int       nargc   = 0;
    short     adapt   = defSCSIAdaptNo;
    short     scsibus = defSCSIBusNo;
    short     scsiID  = NO_SCSI;
    short     scsiLUN = 0;
    int       ramSize = 0;
    int       sctSize = 0;
    int       cluSize = 1;
    char      devCopy[OS9PATHLEN];
    Boolean   wProtect= false;
    int       imgMode = Img_Unchanged;
    Boolean   blankImage = false;
    uint32_t  blankSizeKB= 0;
    char      volName[NAM_LEN+1];
    char      *p;
    int       k;

    #define     MAXARGS_ 2
    char* nargv[MAXARGS_];
    strcpy( devCopy,"" ); // default
    memset( volName,0, sizeof(volName) ); /* empty = "use the device name" */

    for (k=1; k<argc; k++) {
             p= argv[ k ];    
        if (*p=='-') { 
            p++;
            switch (tolower((unsigned char)*p)) {
                case '?' : mount_usage( argv[0], pid ); return 0;

                case 'w' : wProtect= true;         break;
                case 'i' : imgMode = Img_Reduced;  break;
                case 'f' : imgMode = Img_FullSize; break;

                case 'k' : if (*(p+1)=='=') p+=2;
                           else { k++; /* next arg */
                             if  (k>=argc) break;
                             p= argv[k];
                           } // if

                           if (!ParseDiskSize( p,&blankSizeKB )) {
                             upe_printf( "mount: error - invalid size '%s' -- a bare number is "
                                         "bytes (use 32k/8m/1g); 0 makes a host directory\n",p );
                             return 1;
                           } // if
                           blankImage= true;
                           break;

				#ifdef windows32
                case 'a' : if (*(p+1)=='h') {
                             upho_printf("Current defaults: adapter=SCSI%d, bus=%d\n\n",
                                            defSCSIAdaptNo,defSCSIBusNo);
                             scsiadaptor_help();
                             return 0;    
                           } // if
                             
                           if (*(p+1)=='=') p+=2;
                           else { k++; /* next arg */
                             if  (k>=argc) break;
                             p= argv[k];
                           } // if
                             
                           sscanf( p,"%hd", &adapt );
                           break;
                            
                case 'b' : if (*(p+1)=='=') p+=2;
                           else { k++; /* next arg */
                             if  (k>=argc) break;
                             p= argv[k];
                           } // if
                             
                           sscanf( p,"%hd", &scsibus );
                           break;
                #endif
                            
                case 's' : if (*(p+1)=='=') p+=2;
                           else { k++; /* next arg */
                             if  (k>=argc) break;
                             p= argv[k];
                           } // if
                           if (sscanf( p,"%hd", &scsiID )<1) scsiID= NO_SCSI;
                           break;

                case 'l' : if (*(p+1)=='=') p+=2;
                           else { k++; /* next arg */
                             if  (k>=argc) break;
                             p= argv[k];
                           } // if
                           
                           sscanf( p,"%hd", &scsiLUN );
                           break;

                case 'r' : if (*(p+1)=='=') p+=2;
                           else { k++; /* next arg */
                             if  (k>=argc) break;
                             p= argv[k];
                           } // if
                           
                           sscanf( p,"%d", &ramSize );
                           break;
  
                case 'n' : if (*(p+1)=='=') p+=2;
                           else { k++; /* next arg */
                             if  (k>=argc) break;
                             p= argv[k];
                           } // if
                           
                           sscanf( p,"%d", &sctSize );
                           break;
                           
                case 'c' : if (*(p+1)=='=') p+=2;
                           else { k++; /* next arg */
                             if  (k>=argc) break;
                             p= argv[k];
                           } // if
                           
                           sscanf( p,"%d", &cluSize );
                           break;

                case 'd' : if (*(p+1)=='=') p+=2;
                           else { k++; /* next arg */
                             if  (k>=argc) break;
                             p= argv[k];
                           } // if

                           /* -1 and terminate: <p> is an argv string. */
                           strncpy( devCopy, p, OS9PATHLEN-1 );
                                    devCopy[  OS9PATHLEN-1 ]= NUL;
                           break;

                case 'v' : if (*(p+1)=='=') p+=2;
                           else { k++; /* next arg */
                             if  (k>=argc) break;
                             p= argv[k];
                           } // if

                           strncpy( volName, p, sizeof(volName)-1 );
                           break;
                           
                default  : upe_printf("Error: unknown option '%c'!\n",*p); 
                           mount_usage( argv[0],pid ); return 1;
            }   
        }
        else {
            if (nargc>=MAXARGS_) { 
                upe_printf("Error: no more than %d arguments allowed\n",MAXARGS_); 
                return 1;
            }
            nargv[nargc++]= argv[k];
        }
    } /* for */

    if (blankImage) {
      /* A volume name with a space in it arrives here as extra positional
       * arguments, because argv is built by splitting on spaces and nothing
       * honours quotes -- see prepArgs, whose own comment says so. That is not
       * an os9exec shortcut: real OS-9 utilities behave the same way, measured
       * with a genuine Microware makdir, which turned `makdir "a b"` into a
       * directory literally named `"a`. Microware's own format takes the same
       * -v=<name> and has the same limit. So say what happened instead of
       * printing a usage line the caller has probably already read: the name
       * is the thing that is wrong, not the shape of the command.
       * Host-side, tools/mkblank.py's RBF_VOLNAME takes spaces fine. */
      if (nargc>1 && *volName!=NUL)
        return _errmsg( E_BPNAM, "-v: a volume name cannot contain spaces "
                                 "(got \"%s\", then \"%s\").\n", volName,nargv[0] );
      if (nargc!=1)
        return _errmsg( E_BPNAM, "usage: mount -k=<size> [-v=<name>] <h0..hz>\n" );
      err= CreateBlankDevice( pid, nargv[0], blankSizeKB, sctSize, cluSize, volName );
      if (err) return err; /* CreateBlankDevice already printed the reason */
      return 0;
    } // if

    /* -v only has a meaning while creating an image; silently accepting it on
     * an ordinary mount would read as "the volume was renamed", which it was not. */
    if (*volName!=NUL)
      return _errmsg( E_BPNAM, "-v=<name> only applies with -k=<size>.\n" );

    if (nargc==2 && scsiID==NO_SCSI && ramSize==0 && *devCopy==NUL && IsHDevice( nargv[ 1 ] ))
        return MountImageAs( pid, nargv[ 0 ], nargv[ 1 ], wProtect, imgMode );

    if (nargc==0) {      /* no param is not really allowed: exception is ramDisk with */
      if (ramSize>0 || /* size>0 or <devCopy> defined */
          strcmp( devCopy,"" )!=0) { nargv[0]= "/r0"; nargc= 1; }
      else return _errmsg( E_BPNAM, "can't mount device \"\".\n" );
    } // if

    /* nargv[0] is the name of the image to be mounted */
    /* nargv[1] is the name of the mounted device */
           err= MountDev( pid, nargv[ 0 ], nargc<2 ? "":nargv[ 1 ], devCopy,
                          adapt, scsibus, scsiID, scsiLUN,
                          ramSize, sctSize, cluSize, wProtect, imgMode );
    if    (err) return _errmsg( err, "can't mount device \"%s\".\n", nargv[0] );
    return err;
} /* int_mount */

static void unmount_usage( char* name, _pid_ )
{
    upe_printf( "Syntax:   %s <device>\n", name );
    upe_printf( "Function: unmount an RBF image file\n" );
    upe_printf( "Options:  None\n" );
} /* unmount_usage */

os9err int_unmount( ushort pid, int argc, char** argv )
/* unmount an RBF image partition file */
{
    os9err err= 0;
    int nargc=0, h;
    char *p, *name;
    #define     MAXARGS 1
    char *nargv[MAXARGS];

    int          ii;
    syspath_typ* spP;
    rbfdev_typ*  dev;
    char         devName[OS9NAMELEN];

    for (h=1; h<argc; h++) {
        p= argv[h];    
        if (*p=='-') { 
            p++;
            switch (tolower((unsigned char)*p)) {
                case '?' :  unmount_usage( argv[0], pid ); return 0;
                default  :  upe_printf("Error: unknown option '%c'!\n",*p); 
                            unmount_usage( argv[0], pid ); return 1;
            }   
        }
        else {
            if (nargc>=MAXARGS) { 
                upe_printf("Error: no more than %d arguments allowed\n",MAXARGS); return 1;
            }
            nargv[nargc++]= argv[h];
        }
    }   

    if (nargc==0) { unmount_usage( argv[0], pid ); return 1; } /* else nargv[0] is
                     an uninitialised pointer -- `unmount` with no device name would
                     dereference it in AbsPath below */

    name= nargv[0]; if (AbsPath(name)) name++;
    
    do {
        for (ii=0; ii<MAXSYSPATHS; ii++) {
                spP= get_syspathd( pid,ii );
            if (spP!=NULL) {
                err= syspath_gs_devnm( pid, spP->nr, devName ); if (err) break;
                if (ustrcmp(devName,name)==0) { err= E_DEVBSY; break; }
            } 
        } /* for */
        if (err) break;
        
        err = E_UNIT;
        for (ii=0; ii<MAXRBFDEV; ii++) {
                dev= &rbfdev[ii]; 
            if (dev->installed) { /* also name2 or name3 can be released now */
                if (ustrcmp( name,dev->name )==0) {
                    if (*dev->name2!=NUL ||
                        *dev->name3!=NUL)  { 
                          strcpy( dev->name, dev->name2 );
                          strcpy( dev->name2,dev->name3 ); 
                          strcpy( dev->name3,"" ); err= 0; break;
                    }
                    err= ReleaseIt( pid,dev );             break;
                }

                if (ustrcmp( name,dev->name2 )==0) {
                          strcpy( dev->name2,dev->name3 ); 
                          strcpy( dev->name3,"" ); err= 0; break;
                }

                if (ustrcmp( name,dev->name3 )==0) {
                          strcpy( dev->name3,"" ); err= 0; break;
                }
            }
        } /* for */
    } while (false);
        
    if    (err) return _errmsg( err, "can't unmount device \"%s\".\n", name );

    /* `mount <image> hX` also recorded the image under its letter, which is
       where a device name is looked up first: left there, the next access
       mounted the same image again, silently, and a new `mount ... hX`
       found /hX already a device (file-system review). */
    if (tolower((unsigned char)name[0])=='h' && name[1]!=NUL && name[2]==NUL)
        SetMountedImage( name[1], NULL );
    return err;
} /* int_unmount */

static Boolean Mega( long long size, float *r )
{
    #define MegaLim 10000.0;
    Boolean m;
                          *r= size/KByte;
    m= *r>MegaLim; if (m) *r= *r  /KByte;
    return m;
} // Mega

static char* Kb( char* v, size_t vSize, long long size )
/* <vSize> is the caller's buffer size: the formatted value is bounded by it
 * rather than by how large a disk anyone expected. The same assumption already
 * bit once here -- see the note on <v> at the call site, where a 20-byte buffer
 * could not hold "(123.456MB/123.456MB)" and smashed the stack. */
{
  float r;
  char* unit;
                       unit= "K";
  if (Mega( size,&r )) unit= "M";

  /* Whole units once there are at least ten of them: "274M" not "274.0MB".
     A disk size is read at a glance for its order of magnitude, and three
     decimals of a megabyte is spurious precision in a column. Small values
     keep a decimal, where the fraction is the whole of the information. */
  if (r>=  10) { snprintf( v,vSize, "%.0f%s", r,unit ); return v; }
                 snprintf( v,vSize, "%.1f%s", r,unit ); return v;
} // Kb

static void Disp_RBF_DevsLine( rbfdev_typ* rb, char* name, Boolean statistic )
{
    char  s [OS9NAMELEN];
    char  u [OS9PATHLEN];
    char  vI[20], vT[20];
    /* v holds "(<vI>/<vT>)" -- two 20-byte strings plus 3 punctuation chars, so
     * 20 could not possibly hold it: a mere "(123.456MB/123.456MB)" is 22 bytes
     * and smashed the stack on a large enough image. Size it to actually fit. */
    char  v [2*20+4], w [20];

    long long sizeI= (long long)rb->imgScts * rb->sctSize;
    long long sizeT= (long long)rb->totScts * rb->sctSize;

 // Boolean isRAMDisk= false;
 // #ifdef RAM_SUPPORT
 //   isRAMDisk= rb->isRAM;
 // #endif

    strcpy ( s,             name         );
//  sprintf( sc,     "%3d", rb->scsiID   );
    strcpy ( u,             rb->img_name ); 
    
    if     (rb->isRAM || *u==NUL) strcpy( u," -" );
    if     (rb->isRAM)  strcpy ( w, "ram" );
    else {
        if (IsSCSI(rb)) snprintf( w,sizeof(w), "SCSI: %d", rb->scsi.ID );
        else            strcpy ( w, "image" );
    }
    
    Kb   ( vT,sizeof(vT), sizeT );
    /* No parentheses: the column header says what it is, and the brackets cost
       two of the 79 characters the whole line has to fit in. */
    if (sizeI==sizeT) snprintf( v,sizeof(v),    "%s",                 vT );
    else              snprintf( v,sizeof(v), "%s/%s", Kb( vI,sizeof(vI), sizeI ),vT );
    
    upo_printf( "%-10s ", StrBlk_Pt( s,10 ) );
            
    if (statistic)
        upo_printf( "%10d /%10d   %10d /%10d\n",
                     (uint32_t)rb->rMiss, (uint32_t)rb->rTot,
                     (uint32_t)rb->wMiss, (uint32_t)rb->wTot );
    else
        /* Matches the header and devs_printf's SCF rows. Without the "Used by"
           column here the image path landed under it and every RBF row was a
           column out. The path is truncated from the LEFT: "/Users/rdoggett/
           Dev.." identifies nothing, while the tail carries the volume and
           file name that do. Size stays last -- it is the one field an image
           has that a host directory does not. */
        upo_printf( "%-8s %-5s %2d %-10s %-30s %s\n",
                     StrBlk_Pt( w,7 ),
                     "rbf",
                     rb->nr,
                     devs_holder( rb->nr ),
                     StrEnd_Pt( u,30 ),
                     v );
} /* Disp_RBF_DevsLine */

Boolean devs_rbf_listed( const char* name )
/* Did Disp_RBF_Devs already print a row for /<name>?
 *
 * `devs` lists configured host devices (OS9DISK, OS9Hx) so that a host
 * directory appears at all -- it has no struct anywhere, being resolved fresh
 * on every path lookup. An RBF image DOES get a struct, but only once
 * something mounts it, and that row carries the size and path number this one
 * cannot. So the rule is: if it is mounted, the rbfdev[] row wins and the
 * configured-device loop stays quiet.
 *
 * All THREE name slots: a device can be mounted under synonyms (name2/name3),
 * and matching only the first would print a second row for the same image
 * under its alternate name. Names are stored bare here ("h0", no slash). */
{
    rbfdev_typ* r;
    int         ii;

    for (ii=0; ii<MAXRBFDEV; ii++) {
        r= &rbfdev[ii];
        if (!r->installed) continue;

        if (ustrcmp( r->name, (char*)name )==0 ||
            ustrcmp( r->name2,(char*)name )==0 ||
            ustrcmp( r->name3,(char*)name )==0) return true;
    } /* for */

    return false;
} /* devs_rbf_listed */


void Disp_RBF_Devs( Boolean statistic )
{
    rbfdev_typ* r;
    
    int  ii;
    for (ii=0; ii<MAXRBFDEV; ii++) {
            r= &rbfdev[ii]; 
        if (r->installed) {     Disp_RBF_DevsLine( r, r->name,  statistic );
            if (*r->name2!=NUL) Disp_RBF_DevsLine( r, r->name2, statistic );
            if (*r->name3!=NUL) Disp_RBF_DevsLine( r, r->name3, statistic );
        }
    }
} /* Disp_RBF_Devs */

/* ------------------------------------------------------------------------------ */

static void AdaptPath( rbfdev_typ* dev, char** pathP )
{
    char  tmp[OS9PATHLEN];
    char* p   = *pathP;
    int   ilen= strlen(dev->img_name); 

    if (!AbsPath(p)) return;

    if           (*mnt_name!=NUL
      && (ustrcmp( mnt_name,dev->name  )==0 || /* search for all possible names */
      	  ustrcmp( mnt_name,dev->name2 )==0 ||
		  ustrcmp( mnt_name,dev->name3 )==0)
      &&  SamePathBegin ( p,dev->img_name )) {
        strcpy( tmp,p );
        strcpy( p,PSEP_STR   );
        strcat( p,dev->name  );
        strcat( p,&tmp[ilen] );
    }

    while  (*p!=NUL)  { /* search through the string */
        if (*p==PSEP) { /* for a slash */
            if (*(++p)==NUL) break;

            /* start with a path delimiter */
            if (SamePathBegin( p,dev->name  ) ||
                SamePathBegin( p,dev->name2 ) ||
                SamePathBegin( p,dev->name3 ) ||
                SamePathBegin( p,dev->alias )) { *pathP= p-1; break; }
        } /* if */

        p++;
    } /* while */
} /* AdaptPath */

/* ---------------------------------------------------------------- */
static ulong DirLSN( os9direntry_typ* dir_entry )
/* The entry's FD sector. Sector numbers are 24 bits (DD_TOT is three bytes),
   so only the low three bytes count: the high one is 0 in the 68000 format,
   and on a 6809 disk -- whose entries are a 29-byte name and a 3-byte sector
   -- it is the last character of a name that long. */
{   return os9_long( dir_entry->fdsect ) & 0x00FFFFFFUL;
} /* DirLSN */

os9err ReadFD( syspath_typ* spP )
/* read the current file description sector */
{   
    rbf_typ*    rbf= &spP->u.rbf;
    ulong       fd =         rbf->fd_nr;
    rbfdev_typ* dev= &rbfdev[rbf->devnr];
    
    if   (fd!=0) return  ReadSector( dev, fd,1, spP->fd_sct );
    return 0;
} /* ReadFD */

static os9err WriteFD( syspath_typ* spP )
/* write the current file description sector */
{   
    rbf_typ*    rbf= &spP->u.rbf;
    ulong       fd =         rbf->fd_nr;
    rbfdev_typ* dev= &rbfdev[rbf->devnr];

    if   (fd!=0) return WriteSector( dev, fd,1, spP->fd_sct );
    return 0;
} /* WriteFD */

static ulong FDSize( syspath_typ* spP )
/* get the file size  */
{   return GET_OS9L(spP->fd_sct, 9);
} /* FDSize */

static void Set_FDSize( syspath_typ* spP, ulong size )
/* set the file size  */
{   SET_OS9L(spP->fd_sct, 9, size);
} /* Set_FDSize */

static byte FDAtt( syspath_typ* spP )
/* get the file attributes  */
{   return spP->fd_sct[0];  
} /* FDAtt */

static void Set_FDAtt( syspath_typ* spP, byte att )
/* set the file attributes  */
/* but only if not root directory */
{
    rbf_typ*    rbf= &spP->u.rbf;
    rbfdev_typ* dev= &rbfdev[rbf->devnr];

    if (rbf->fd_nr!=dev->root_fd_nr) spP->fd_sct[0]= att;
} /* Set_FDAtt */

static ushort FDOwn( syspath_typ* spP )
/* get the file owner: packed group.user, group<<8|user */
{   return GET_OS9W(spP->fd_sct, 1);
} /* FDOwn */

ushort PathFDOwner( syspath_typ* spP )
/* the owner word (group byte, user byte) of an open RBF path's file */
{   return FDOwn( spP );
} /* PathFDOwner */

static void Set_FDOwn( syspath_typ* spP, ushort owner )
/* set the file owner: packed group.user, group<<8|user */
{   SET_OS9W(spP->fd_sct, 1, owner);
} /* Set_FDOwn */

typedef enum { permRead, permWrite, permExec } perm_typ;

static ushort CallerOwner( ushort pid )
/* the calling process's own group.user, packed the same way FD_OWN is */
{
    return (ushort)( (os9_word(procs[pid].pd._group)<<BpB) | os9_word(procs[pid].pd._user) );
} /* CallerOwner */

static Boolean IsOwner( ushort pid, ushort ownerWord )
/* true if the caller's group.user matches the file's owner word */
{
    return CallerOwner(pid)==ownerWord;
} /* IsOwner */

static Boolean has_perm( ushort pid, byte att, ushort ownerWord, perm_typ want )
/* single-bit permission test: super-user always passes; otherwise pick the
 * owner or public bit-triplet depending on ownership, and test one bit. */
{
    byte ownerBit, publicBit;

    if (is_super(pid)) return true;

    switch (want) {
      case permRead : ownerBit= 0x01; publicBit= 0x08; break;
      case permWrite: ownerBit= 0x02; publicBit= 0x10; break;
      default       : ownerBit= 0x04; publicBit= 0x20; break; /* permExec */
    } /* switch */

    return (att & (IsOwner(pid,ownerWord) ? ownerBit : publicBit)) != 0;
} /* has_perm */

static Boolean has_open_perm( ushort pid, byte att, ushort ownerWord, ushort mode )
/* every access type actually requested by <mode> (R/W/E) must be granted */
{
    if (IsRead (mode) && !has_perm(pid,att,ownerWord,permRead )) return false;
    if (IsWrite(mode) && !has_perm(pid,att,ownerWord,permWrite)) return false;
    if (IsExec (mode) && !has_perm(pid,att,ownerWord,permExec )) return false;
    return true;
} /* has_open_perm */

static byte FDLnk( syspath_typ* spP )
/* the link count: how many directory entries name this file */
{ return spP->fd_sct[8];
} /* FDLnk */

static void Set_FDLnk( syspath_typ* spP, byte lnk )
/* set the link count */
{ spP->fd_sct[8]= lnk;
} /* Set_FDLnk */

static os9err FD_Segment( syspath_typ* spP, byte *attr, ulong *size, ulong *totsize, 
                                           ulong *sect, ulong *slim, ulong *pref )
{
  rbfdev_typ* dev= &rbfdev[spP->u.rbf.devnr];
  ulong        v, scs, blk, add, pos;
  int         ii;
  Boolean     done= false;

  *attr= FDAtt ( spP );
  *size= FDSize( spP );
  *pref= 0; // preferred sector to allocate: none
  /* sect and slim initialised for the same reason pref already was: they are
   * only written inside the "found the segment holding currPos" branch below,
   * so a file whose segment list ends immediately (scs==0 on the first entry,
   * i.e. a zero-length file) or a currPos past every segment left them
   * untouched -- and the caller then used *sect as a SECTOR NUMBER. Returning
   * 0 is the same "nothing here" answer *pref gives.
   * (clang scan-build, "Uninitialized argument value".) */
  *sect= 0;
  *slim= 0;

  v= 0;  ii= FD_Header_Size;
  while (ii+SegSize <= dev->sctSize) {
    pos= GET_OS9L(spP->fd_sct, ii  ) >> BpB;
    scs= GET_OS9W(spP->fd_sct, ii+3);
    blk=                                scs * dev->sctSize;
        
    if (!done && v+blk > spP->u.rbf.currPos) {
      add  =           ( spP->u.rbf.currPos-v ) / dev->sctSize;
      *sect= pos + add;
      *slim= pos + scs;
      done = true;
    } // if
        
  //if (scs>0) *pref= pos + scs;
    if (scs==0) break;
    *pref= pos + scs;
           
    v += blk;
    ii+= SegSize; // goto next entry
  } // while
    
  *totsize= v;
  debugprintf(dbgFiles,dbgDetail,("# FD_Segment size/totsize/sect: %d %d $%x\n",
                                     (uint32_t)*size, (uint32_t)*totsize, (uint32_t)*sect ));
  return 0;
} /* FD_Segment */

static os9err GetThem( rbfdev_typ* dev, ulong pos, ulong scs, Boolean get_them )
/* Allocate the bits at the allocation map */
{
    os9err err;
    ulong  kk;
    
    ulong pdc = pos/dev->clusterSize;
    ulong ii  = pdc / BpB;
    ulong jj  = pdc % BpB;
    ulong asct= ii  / dev->sctSize + 1;
    ulong apos= ii  % dev->sctSize;
    
    byte           imask, rslt;
    byte*          ba;
    byte           mask= 0x80;
    while (jj>0) { mask= mask/2; jj--; } /* search first mask */
    
           kk= scs;
    while (kk>0) {     /* use the device's temporary sector */
        debugprintf(dbgFiles,dbgNorm,("# GetThem (%s) sectorNr: $%x, pos/scs: $%x %d\n",
                                         get_them ? "true":"false", (uint32_t)asct, (uint32_t)pos,(uint32_t)scs ));
        err= ReadSector ( dev, asct,1, dev->tmp_sct ); if (err) return err;
    
        while (kk>0) {
            if (mask==0) { 
                mask= 0x80; apos++;
                if         (apos==dev->sctSize) break; 
            }

            imask= 255-mask; /* invert it */
            ba= &dev->tmp_sct[apos]; /* current buffer field */
            if  (get_them) rslt= *ba |  mask;   /* set   bit */     
            else           rslt= *ba & imask;   /* clear bit */

            if  (rslt==*ba) return E_NES;
            *ba= rslt;

            mask= mask / 2;
            kk -= dev->clusterSize;
        } /* inner while */
    
        err= WriteSector( dev, asct,1, dev->tmp_sct ); if (err) return err;
        asct++;  /* prepare for the next sector */
        apos= 0; /* and don't forget to reset position in sector */
    } /* outer while */
        
    return 0;
} /* GetThem */

static os9err BlkSearch( rbfdev_typ* dev, ulong uscs, ulong mpsct, ulong mploc,
                         Boolean *found,  ulong *pos, ulong *scs )
/* Search for a block with the given size       */
/* start searching in sector <mpsct> at <mploc> */
{
    os9err err;
    ulong  ii, jj;
    byte   vv, mask;
    int    taken;
    ulong  clu   = dev->clusterSize;
    ulong  sOffs = dev->sctSize*(mpsct-1)*BpB;
    ulong  posmax= *pos; /* max values are stored */
    ulong  scsmax= *scs;

    /* look for a new block now */
    *pos= clu*( sOffs+mploc*BpB ); 
    *scs= 0;

    *found= false;
    err= ReadSector( dev, mpsct,1, dev->tmp_sct ); if (err) return err;

    for (ii=mploc; ii<dev->sctSize; ii++ ) { /* go through the sector */
        vv  = dev->tmp_sct[ii];
        mask= 0x80;
        
        for (jj=0; jj<=7; jj++) {
                taken= vv & mask;       /* already taken ? */
            if (taken) { *scs= 0; *pos= clu*( sOffs+ii*BpB + jj+1 ); } 
            else        (*scs)+=        clu;
            
            if (*scs>=  uscs) { *found= true; return 0; }
            if (*scs> scsmax) { posmax= *pos; scsmax= *scs; }
            mask= mask/2;
        } /* for */
    } /* for */

    *pos= posmax;
    *scs= scsmax;
    return 0;
} /* BlkSearch */

static os9err AllocateBlocks( syspath_typ* spP, ulong uscs, ulong *posP, ulong *scsP,
                                                ulong prefpos )
/* allocate <uscs> sectors and. Result is <posP>,<scsP>. */
{
  os9err      err;
  rbfdev_typ* dev  = &rbfdev[spP->u.rbf.devnr];
  ulong       mxsct= (dev->mapSize-1)/dev->sctSize + 1;
  ulong       mpsct, s1;
  ulong       pdc  = prefpos        /dev->clusterSize/BpB;  
  ulong       lst  = dev->last_alloc/dev->clusterSize/BpB;  
  ulong       mploc;
  Boolean     first, found;

  *posP= 0;
  *scsP= 0;

  if (prefpos!=0) {
        mpsct= 1 + pdc / dev->sctSize;
        mploc=     pdc % dev->sctSize;
    if (mpsct>=mxsct) mpsct= 1;

        err= BlkSearch( dev, uscs, mpsct,mploc, &found, posP,scsP ); 
//  printf( "Ask=%5X get=%4X pos=%06X fnd=%d err=%d\n", uscs,*scsP,*posP, found, err );
    if (err) return err;
  }

  if (*scsP==0 || *posP!=prefpos) { /* no preferred allocation */
        mpsct= 1 + lst / dev->sctSize; 
        mploc=     lst % dev->sctSize;
    if (mpsct>mxsct) mpsct= 1;
    s1= mpsct;

    first= true;
    while (true) { /* run through for one whole round */
          err= BlkSearch( dev, uscs, mpsct,mploc, &found, posP,scsP );
//    printf( "ask=%5X get=%4X pos=%06X fnd=%d err=%d\n", uscs,*scsP,*posP, found, err );
      if (err)   return err;
      if (found) break;

      if (!first &&    mpsct==s1) break; /* all done */
      if (mpsct<mxsct) mpsct++;
      else             mpsct= 1;

      first= false;
      mploc= 0; /* from now on search from beginning */
    } /* while (true) */
  } // if

  if (*scsP==0) return E_FULL;

//printf( "FND=%5X get=%4X pos=%06X\n", uscs,*scsP,*posP );
  dev->last_alloc=   *posP + *scsP; /* next search position */
  err= GetThem( dev, *posP,  *scsP, true );
//printf( "GetThem err=%d\n", err );
  return err;
} /* AllocateBlocks */

static os9err DeallocateBlocks( syspath_typ* spP )
{
    rbfdev_typ* dev= &rbfdev[spP->u.rbf.devnr];
    ulong       fd =         spP->u.rbf.fd_nr;
    int         ii;
    for (ii=16; ii+SegSize <= dev->sctSize; ii+=SegSize) {
        ulong  pos= GET_OS9L(spP->fd_sct, ii  ) >> BpB;
        ushort scs= GET_OS9W(spP->fd_sct, ii+3);

        /* if the fd sector is allocated alone -> dealloc it alone */
        if (ii==16 && fd+1!=pos) GetThem( dev, fd, 1, false );

        if (scs>0) {
            debugprintf(dbgFiles,dbgNorm,("# Dealloate ii/pos/scs: %4d $%x %d\n",
                                             ii, (uint32_t)pos, scs ));
            if (ii==16 && fd+1==pos) { pos--; scs++; } /* add the fd sector */
            GetThem( dev, pos, scs, false ); /* release elements */

            if (dev->last_alloc>pos) dev->last_alloc= pos; /* next pos to search */
        } /* if */
    } /* for */
    
    return 0;
} /* DeallocateBlocks */

static os9err ReleaseBlocks( syspath_typ* spP, ulong lastPos )
{
  os9err      err;
  int         ii;
  rbfdev_typ* dev= &rbfdev[spP->u.rbf.devnr];
//ulong       fd =         spP->u.rbf.fd_nr;
  ulong       cmp= lastPos/dev->sctSize + 1; /* including fd sector for cluster allocation */
  ulong       tps= 0;
  ulong        diff;
  Boolean     broken= false;

  // go through the fs segment list
  for (ii=16; ii+SegSize <= dev->sctSize; ii+=SegSize) {
    ulong  pos= GET_OS9L(spP->fd_sct, ii  ) >> BpB;
    ushort scs= GET_OS9W(spP->fd_sct, ii+3);

    if (scs==0) broken= true;
    if (broken) {
      SET_OS9L(spP->fd_sct, ii,   0);
      SET_OS9W(spP->fd_sct, ii+3, 0);
      if (scs>0) GetThem( dev, pos, scs, false );
    }
    else {
          tps+= scs;
      if (tps>cmp) {
            diff= tps-cmp; /* keep track of granularity */
            diff= diff/dev->clusterSize*dev->clusterSize;
        if (diff>=scs) {
            diff= scs;
          SET_OS9L(spP->fd_sct, ii,   0);
          SET_OS9W(spP->fd_sct, ii+3, 0);
        }
        else {
          SET_OS9W(spP->fd_sct, ii+3, scs-diff);
        } // if

        broken= true;
        GetThem( dev, (pos+scs)-diff, diff, false );
      } // if
    } // if
  } // for
    
  err= WriteFD( spP );
  return err;
} /* ReleaseBlocks */

static os9err AdaptAlloc_FD( syspath_typ* spP, ulong pos, ulong scs )
{
  #define     LimScsPerSegment 0x8000 /* number of sectors per segment must be less than this */
  #define     First  16
  #define     Second First+SegSize
  os9err      err;
  rbfdev_typ* dev= &rbfdev[spP->u.rbf.devnr];
  int         ii, prev_ii= -1; /* -1 = no previous segment yet */
  ulong        prev_l= 0, blk, mx;
  ushort       psp;
  ulong       lpos= pos; /* treat them locally, because 'Get_Them' uses it also */
  ulong       lscs= scs; /* a ushort cut a count of 65536 or more: one call can
                            now be asked that many (SS_Size), and the bitmap bits
                            of the difference were set with no FD naming them */
  /* The loop writes the run into the FD sector as it goes. If the segment list
     runs out, the run's sectors are given back below, so the FD must be given
     back too: left as it was, it named freed sectors, and SS_Size's error path
     then published it to the file's other paths, the next of which to write
     its FD put that on the disk -- two files sharing sectors. */
  byte        fdsave[ MIN_TMP_SCT_SIZE ];
  memcpy( fdsave, spP->fd_sct, dev->sctSize );

  for (ii=First; ii+SegSize <= dev->sctSize; ii+=SegSize) {
    if (GET_OS9W(spP->fd_sct, ii+3)==0) { /* zero is zero for big/little endian */
          psp= (prev_ii>=0) ? GET_OS9W(spP->fd_sct, prev_ii+3) : 0;
      if (psp>0 && prev_l+psp==lpos) {  /* combine it */
        if (ii==Second) mx= LimScsPerSegment-1; /* assuming clusterSize is divideable */
        else            mx= LimScsPerSegment-dev->clusterSize;
        blk= Min( psp+lscs,mx ); /* but not larger than the max allowed size */
        SET_OS9W(spP->fd_sct, prev_ii+3, blk);

        lpos+= blk-psp;
        lscs-= blk-psp; /* adjust the number of blocks */
      } // if

      if (lscs>0) {
        if (ii==First) mx= LimScsPerSegment-1; /* assuming clusterSize is divideable */
        else           mx= LimScsPerSegment-dev->clusterSize;
        blk= Min( lscs,mx );

        SET_OS9L(spP->fd_sct, ii,   lpos<<BpB);
        SET_OS9W(spP->fd_sct, ii+3, blk); /* one byte overlapping */

        lpos+= blk;
        lscs-= blk;
      } // if

      if (lscs==0) {
        err= WriteFD( spP ); if (err) return err;
        return 0;
      } // if
    } /* if */

    prev_l  = GET_OS9L(spP->fd_sct, ii) >> BpB;
    prev_ii = ii;
  } /* for */

  memcpy( spP->fd_sct, fdsave, dev->sctSize );
  GetThem( dev, pos,scs, false );
  return E_SLF;
} /* AdaptAlloc_FD */

static os9err DoAccess( syspath_typ* spP, uint32_t *lenP, char* buffer,
                          Boolean lnmode, Boolean wMode )
/* this is the main read routine for the RBF emulator */
/* it is working for read and readln */
{
    os9err      err   =  0;
    rbf_typ*    rbf   = &spP->u.rbf;
    rbfdev_typ* dev   = &rbfdev[rbf->devnr];
    uint32_t    bstart=  rbf->currPos;
    ulong       boffs =  0;
    uint32_t    remain= *lenP;
    uint32_t    reqLen= *lenP;  /* what was ASKED for; *lenP becomes what was got */
    uint32_t*   mw    = &spP->mustW;
    /* PD_SAS is a path option, not a device setting: <rbf->sas> starts as the
       device descriptor's value and SS_Opt can change it for this path alone. */
    ulong       ma    = Max( rbf->sas,dev->clusterSize );
    ulong       sect, slim, offs, size, totsize, maxc, pos, scs, *rs, pref, coff, sv, req, lp0;
    byte*       bb;
    byte        attr;
    int         ii;
    Boolean     done= false;               // break condition for readln 
    Boolean     rOK = (remain==0);         // is true, if nothing to read
    Boolean     first= true;
    /* mltFirst=false: it IS set before the read at the bottom of this loop
       (`if (first)` a few lines above it, and `first` starts true), but that
       is a correlation across a long body with several early exits -- gcc on
       32-bit gives up on it, and so would anyone inserting a `continue`
       between the two. This is the RBF read/write path; an undefined value
       here would pick sector-cache behaviour at random. */
    Boolean     mlt, mltFirst= false;
    ulong       n, n0, d;
    byte*       b;
    
    debugprintf( dbgFiles,dbgDetail,("# >DoAccess (%s): n=%d\n", wMode ? "write":"read", *lenP ));
    sv = rbf->currPos;
    lp0= rbf->lastPos;

    /* Re-entered after parking -- restore the process whatever kind of access
     * parked it. This was `if (!wMode)` from when only a READ could park (at
     * EOF, waiting for a writer); parking a WRITE on a record-lock conflict was
     * added later and never reached here. The consequence was not that the
     * write stayed blocked: pWaitRead is exactly what makes the dispatcher put
     * the saved registers back and run the call again, so once the holder let
     * go the write SUCCEEDED and was then re-run, and succeeded, and was re-run
     * -- for ever, never returning to its caller. That is the "never came back"
     * this leaves behind. */
    WokeOnFile( currentpid );

    /* A write has to be stopped BEFORE it happens -- it changes the file, and
     * its length is exactly what was asked for, so the range is known here.
     * A read is checked afterwards instead: see below. */
    if (!spP->rawMode && !rbf->sysPath && wMode && *lenP>0) {
        syspath_typ* spH= LockHolder( spP, rbf->currPos, rbf->currPos+*lenP );

        /* The EOF lock stops a SECOND writer extending the file, and that is
         * the reason it exists: "EOF lock is the only time that a write call
         * automatically causes lock out of any part of the file. This avoids
         * problems that could occur when two users try to simultaneously extend
         * a file" (Technical Manual, page 7-9). It was consulted on the read
         * path only, so the documented spooler-behind-assembler case worked
         * while the case the manual gives as the lock's PURPOSE did not.
         *
         * Only a write that would actually extend the file collides with it --
         * one landing wholly inside existing data is not a second appender, and
         * the ghost lock covers no real bytes. Its holder's own record lock, if
         * any, is a separate matter and is what LockHolder above answers.
         *
         * A holder in our own process is not a conflict, for the same reason
         * two paths of one process never lock each other out (page 7-8). */
        Boolean blocked= (spH!=NULL);

        if (!blocked && rbf->currPos+*lenP > FDSize( spP )) {
            ushort wpid= EofLockHolder( spP );
            blocked= (wpid!=0 && wpid!=rbf->ownPid);
        } // if

        if (blocked) {
          /* LockHolder never returns one of our own process's paths now, so a
             holder here is genuinely somebody else and waiting for it is real. */
          if (WaitExpired( spP )) { WaitDone( spP ); return os9error( E_LOCK ); }
          SleepOnFile( spP, currentpid );
          *lenP= 0;
          return 0; /* the call runs again when the holder releases */
        } // if

        WaitDone( spP );
    } // if

    
    do {           // do this loop for every sector to be read into buffer
        if (spP->rawMode) {
          sect= rbf->currPos / dev->sctSize;          // get raw sector nr
          slim= -1;  
          size= -1; rs= &size;                           // no upper limit
        }
        else {                            // get sector nr of segment info
          FD_Segment( spP, &attr,&size,&totsize,&sect,&slim, &pref );
                                      rs= &totsize;   // the relevant size
          if (!wMode && size<totsize) rs= &size;
            
          if (totsize<=rbf->lastPos)      rbf->lastPos= totsize;
          if (*rs    <=rbf->lastPos) *rs= rbf->lastPos;

          if (!wMode) { // reduce to currect ammount
            if     (*rs < rbf->currPos+remain) 
            remain= *rs - rbf->currPos;
          } // if
            
          if (rbf->currPos >= *rs) {
            if (wMode) {
                  req= rbf->currPos - *rs;
              if (req>0) req= ( req-1 ) / dev->sctSize + 1; // at least requested
              
              while (true) {
                scs= Max( ma, req ); // alloc size might be larger
                err= AllocateBlocks( spP,scs, &pos,&scs, pref ); if (err) break;
                err= AdaptAlloc_FD ( spP,      pos, scs       ); if (err) break;
                
                if   (scs>=req) break;
                req-= scs; // still asking for some more sectors
              } // loop
              if (err) break;
                 
              FD_Segment( spP, &attr,&size,&totsize,&sect,&slim, &pref );
                                             rs= &totsize;
              if (rbf->currPos>=*rs) {
                debugprintf( dbgFiles,dbgDetail,("# DoAccess (%s) EOF: n=%d\n", 
                                                    wMode ? "write":"read", *lenP ));
                return E_EOF;
              } //if               
            }
            else {
              if (rOK)    break; /* reading is ok so far */

              /* nothing here yet -- but if another path holds the end of this
               * file, this is not the end of it, so wait for the next write
               * rather than reporting one */
              if (!spP->rawMode && !rbf->sysPath) {
                  ushort wpid= EofLockHolder( spP );

                  if (wpid!=0 && wpid==currentpid) {
                      err= E_DEADLK; break; /* the writer is us: we would be
                                             * waiting for ourselves */
                  } // if

                  if (wpid!=0) {
                      if (WaitExpired( spP )) { WaitDone( spP ); err= E_LOCK; break; }

                      SleepOnFile( spP, currentpid );
                      *lenP= 0;
                      return 0; /* the read runs again once we are woken */
                  } // if
              } // if

              err= E_EOF; break;
            } // if
          } // if
        } // if
     
        offs= rbf->currPos % dev->sctSize; /* get offset within this sector */

            mlt= (dev->multiSct && sect!=0 && offs==0 && remain>=2*dev->sctSize);
        if (first) { mltFirst= mlt; first= false; }
        
        if (mlt) {
                maxc= *rs;
            if (maxc>remain) maxc= remain;
            n=  maxc / dev->sctSize;
            d=  maxc % dev->sctSize;
            b=  (byte*)(buffer+boffs);
            
            if (sect+n>slim) n= slim-sect; /* not more than a segment   */
            if (n     > 255) n=       255; /* command for <=256 sectors */

            n0= n; if (d==0) n0--;
            maxc= dev->sctSize*n; /* don't take the last sector, if incomplete: could cause overflow */
        }
        else {
                maxc= dev->sctSize - offs; /* calculate the max number of bytes to be read */
            if (maxc>remain) maxc= remain;
            n= 1; n0= 0; d= 0;
            b= spP->rw_sct;
        } // if

      //if (spP->rawMode && sect>=60 && sect<=90) {
      //  upo_printf( "DoAccess rw_nr=%d sect=%d *mw=%d mlt=%d\n", spP->rw_nr, sect, *mw, mlt );
      //} // if
    
        if (spP->rw_nr==0    ||             /* read only if different one */
            spP->rw_nr!=sect || mltFirst) { /* it is important to decide about <mlt> of the first run */
          if (*mw!=0) {
          //  if (dev->multiSct) upe_printf( "Tsct slm n n0 offs d len %7d %7d %7d %7d %7d %7d %7d\n", sect,slim,n,n0,offs,d,*lenP );
              err= WriteSector( dev,  *mw,1, spP->rw_sct ); if (err) break;
              RingInvalidate ( spP,   *mw );
                *mw= 0; /* now it is written */
          } // if

          if (!mlt || !wMode) {
            if (n!=1 || !RingSector( spP, sect, b, dev->sctSize )) {
              if (n!=1) { err= RingFlushRange( spP, sect,n ); if (err) break; }
              err= ReadSector( dev, sect,n, b ); if (err) break;
            } // if
        //  if (dev->multiSct) upe_printf( "Rsct slm n n0 offs d len %7d %7d %7d %7d %7d %7d %7d\n", sect,slim,n,n0,offs,d,*lenP );
            spP->rw_nr= sect + n0;
                
            if (mlt) {  
              if (n>n0) { /* the full sector has been read */
                memcpy( spP->rw_sct, b + n0*dev->sctSize, dev->sctSize ); /* update rw_sct */
              }
              else      spP->rw_nr= 0; /* there must be a follow up */
            }
            else {
              if (sect==0) { err= ChkIntegrity( dev, spP, b,true ); if (err) break; }
            } // if
            
          } // if  
        } // if

        rOK = true;
        debugprintf(dbgFiles,dbgDeep,("# RBF %s: \"%s\" $%x bytes, sect: $%x, size: $%x\n",
                                         wMode ? "write":"read", dev->name, remain, (uint32_t)sect, (uint32_t)*rs));
      //if (sect==0) upe_printf( "Write 0 raw=%d\n", spP->rawMode );
                
        if (lnmode) {       /* depends on read or write */
            if (wMode || mlt) { bb= (byte*)buffer; coff= boffs; }
            else              { bb=   spP->rw_sct; coff=  offs; }

            for (ii=coff; ii<coff+maxc; ii++) {
                if (bb[ii]==CR) { done= true; maxc= ii-coff+1; break; }
            } // for
        } // if
    
        /* copy to/from the buffer */
        if (wMode) {
            if (!mlt) {
                memcpy(spP->rw_sct+offs, buffer+boffs, maxc);
                /* the moment it is dirtied, not when it is eventually flushed:
                 * a small file may never flush before close, and a reader
                 * holding this sector goes on serving itself the copy it took
                 * before this write unless it is dropped now */
                RingInvalidate( spP, sect );
            }
            
            if (*mw!=0 && *mw!=sect) { /* if sector nr has changed */
              err= WriteSector( dev,  *mw,1, spP->rw_sct ); if (err) break;
              spP->rw_nr= *mw;
            } // if

            /* if sector in raw mode */
            if (spP->rawMode) {
              if (sect==0) {
                ushort diskID= GET_OS9W(spP->rw_sct, 14); /* get the new disk ID */
                dev->last_diskID= diskID;     /* must be done */
                rbf->diskID=      diskID;
                
                /* write sector 0 always */
                err= WriteSector( dev,  sect,1, spP->rw_sct ); if (err) break;
              } // if

              if (sect==1) dev->last_alloc= 0; /* reset after "format" */
            } // if
                    
            *mw= sect;
            if (mlt) {
              err= WriteSector( dev, sect,n, b ); if (err) break;
              { ulong w; /* whole sectors replaced: nobody's copy of them stands */
                for (w=0; w<n; w++) RingInvalidate( spP, sect+w ); }
              /* As the read above does: <rw_nr> may only name a sector whose
                 bytes <rw_sct> really holds. It named the last one written
                 while the buffer still held an older sector, so the next call
                 landing there read old bytes, or patched them and flushed them
                 back over the write. */
              if (n>n0) {
                memcpy( spP->rw_sct, b + n0*dev->sctSize, dev->sctSize );
                spP->rw_nr= sect + n0;
              }
              else spP->rw_nr= 0;
              *mw= 0; /* already written */
            } // if
        }
        else {
          if (!mlt) memcpy( buffer+boffs, spP->rw_sct+offs, maxc );
        } // if
                    
        rbf->currPos+= maxc; /* calculate the new position */

        *lenP = rbf->currPos - bstart;

        if (done) {
            maxc = remain; /* loop is finished now because CR has been found */
        }
        else if (rbf->currPos > *rs) {         /* remaining byte calculation */
            *lenP = *lenP + *rs - rbf->currPos;
            rbf->currPos= *rs;
        }
        
        remain-= maxc; /* adapt them for the next loop */
        boffs += maxc;
    } while (remain>0);
    
    if (err==E_FULL || err==E_SLF) {
        /* The write did not happen: the pointer goes back to where it began,
           and the file keeps the size it had. Setting the size to the write's
           start shrank a file written past there earlier -- a write begun
           inside a file that ran out of disk cut the file at that point. Any
           segments allocated before the failure are in the FD already, so the
           file's other paths get it too (as SS_Size's error path does). */
        rbf->currPos= sv;
        rbf->lastPos= lp0;
        RingPublishFD( spP );
    }
    
    if (!spP->rawMode && *lenP==0 && sv==rbf->currPos) {
        LockDrop  ( spP ); /* a zero-byte read or write drops everything this
                            * path holds, whatever it was holding it for --
                            * the end of the file included */
        rbf->eofLock= false;
        WakeOnFile( spP );
    }
    else if (!spP->rawMode && !rbf->sysPath && !err) {
      /* The EOF lock, per ch.7: gained by an access AT the end of the file,
       * kept until an access that is NOT at the end. Only a path that could
       * extend the file takes it; a reader merely waits on it. Evaluated for
       * reads too, so that seeking back into the middle and reading drops it,
       * which is what "a read or write that is not at the end" says. */
      if (rbf->wMode) {
          /* The end as this path knows it: a growing file's new end is in
             lastPos until the FD is flushed, and against the FD's older size
             a write back at offset 10 of a 100-byte file still looked like a
             write at the end, so it kept the lock (CONF68K t106). */
          Boolean atEnd= (rbf->currPos >= Max( FDSize( spP ), rbf->lastPos ));

          if (atEnd!=rbf->eofLock) {
              rbf->eofLock= atEnd;
              if (!atEnd) WakeOnFile( spP ); /* the end is free again */
          } // if
      } // if

      if (wMode) {           /* the write releases what the read took */
          LockDrop  ( spP );
          WakeOnFile( spP ); /* whoever was waiting on it can go */
      }
      else if (rbf->currPos>sv) {
          /* Checked here, not before the read: how much a read really touches
           * is not known until it happens -- a line-oriented read stops at the
           * terminator, and a caller may offer a buffer far larger than the
           * record (BASIC09 offers 511 bytes for an 8-byte record). Judging the
           * conflict on the length ASKED FOR made a read of one record collide
           * with a lock held on a later one. Reading is not destructive, so the
           * honest thing is to see what was actually delivered and hand it back
           * only if nobody holds those bytes. */
          /* The range ASKED FOR, not the range delivered: the manual is
           * explicit that a ReadLn wanting 256 locks 256 wherever the CR
           * actually landed.
           * Clamped to the end of the file, though, because a reader has no
           * business holding bytes that do not exist yet -- that ground
           * belongs to whoever is appending, and is what the EOF lock covers.
           * Without the clamp a follower asking for more than is there locks
           * past the end, the producer's next append collides with it, and
           * the two wait on each other forever: demonstrated, a live deadlock
           * with an update-mode follower. */
          ulong        eofPos= FDSize( spP );
          ulong        lckEnd= sv+reqLen;
          syspath_typ* spH;

          if (lckEnd>eofPos) lckEnd= eofPos;
          spH= (lckEnd>sv) ? LockHolder( spP, sv, lckEnd ) : NULL;

          if (spH!=NULL) {
              rbf->currPos= sv; /* pretend it never happened */
              *lenP= 0;
              if (WaitExpired( spP )) { WaitDone( spP ); return os9error( E_LOCK ); }
              SleepOnFile( spP, currentpid );
              return 0; /* runs again once the holder lets go */
          } // if

          WaitDone( spP );
          if (rbf->updMode && lckEnd>sv) {
              rbf->lockBeg= sv;      /* requested extent, clamped to the end */
              rbf->lockEnd= lckEnd;
          }
          else LockDrop( spP ); /* nothing real was read: hold nothing */
      } // if
    } // if

    if (rbf->lastPos< rbf->currPos) { /* adapt lastpos */
        rbf->lastPos= rbf->currPos;

        /* the file just grew: hand the others the new size, and the segment
         * list that goes with it, so a reader following this writer can both
         * tell there is more and find the sectors it landed in */
        if (wMode && !spP->rawMode) {
            RingPublish( spP, rbf->lastPos );
            WakeOnFile ( spP );  /* there is more to read now */
        } // if
    } // if

    debugprintf( dbgFiles,dbgDetail,("# <DoAccess (%s): n=%d\n", wMode ? "write":"read", *lenP ));
    return err;
} /* DoAccess */

static os9err Create_FD( syspath_typ* spP, byte att, ushort owner, ulong size )
{
    rbfdev_typ* dev= &rbfdev[spP->u.rbf.devnr];

    int  ii;
    for (ii=0; ii<dev->sctSize; ii++) { /* clear sector */
          spP->fd_sct[ii]= NUL;
    }

    Set_FDAtt     ( spP,  att   ); /* attributes */
    Set_FDOwn     ( spP,  owner ); /* creator's group.user */
    Set_FDLnk     ( spP,    1   ); /* the link count */
    Set_FDSize    ( spP, size   ); /* file size  */
    return WriteFD( spP );         /* write FD sector */
} /* Create_FD */

static os9err OpenDir( rbfdev_typ* dev, ulong dfd, ushort *sp )
{
    os9err       err;
    syspath_typ* spP;
    rbf_typ*     rbf;

    err= syspath_new( sp,fRBF ); if (err) return err;

    spP         = &syspaths[*sp];
    spP->fd_sct = NULL;           /* prepare them locally */
    spP->rw_sct = NULL;
    spP->rw_nr  = 0;              /* undefined */
    GetBuffers  ( dev,spP );      /* dev must be assigned before */

    rbf         = &spP->u.rbf;
    rbf->sameFile= *sp;           /* a ring of its own: a reused slot would
                                   * otherwise carry a stale link, and the ring
                                   * scan could follow it into this path */
    rbf->currPos= 0;              /* initialize position to 0 */
    rbf->wMode  = true;           /* by default it can be written */
    rbf->devnr  = dev->nr;
    rbf->sas    = dev->sas;       /* PD_SAS starts as the device's (SS_Opt changes it) */
    rbf->fd_nr  = dfd;            /* the directory's sector */
    /* RBF's own path: it adds and removes entries on the caller's behalf, as
     * the file manager does on real OS-9, so it takes no record or EOF lock
     * and waits on none -- a caller holding its own directory open for update
     * would otherwise wait on itself. A reused slot's fields are cleared. */
    rbf->sysPath= true;
    rbf->updMode= false;
    rbf->ownPid = currentpid;
    rbf->lockBeg= 0;
    rbf->lockEnd= 0;
    rbf->eofLock= false;
    rbf->waitPid= 0;

        err=   ReadFD        ( spP );            /* IMPORTANT !! */
    if (err) { ReleaseBuffers( spP ); return err; }

    /* Into the directory's ring, once its FD is read, like any other path on
     * it: a user path may hold one of its sectors, clean or dirty, and this
     * path adds and removes entries. Alone, it neither saw that path's unflushed
     * copy nor dropped it, and the user path flushed its stale sector back over
     * the entry written here -- the new file's name was gone and its FD and
     * space owned by nothing. RingJoin also adopts a newer size from the ring.
     * ReleaseBuffers (via close) leaves the ring again. */
    RingJoin( spP );
    rbf->lastPos= FDSize( spP );           /* get the file size  */
    return 0;
} /* OpenDir */

static os9err CloseDir( ushort sp )
{
    os9err       err;
    syspath_typ* spP= &syspaths[sp];

    spP->u.rbf.currPos= 0; /* don't release blocks */
    err= syspath_close( 0,sp );
    return err;
} /* CloseDir */

static void Fill_DirEntry( os9direntry_typ* dir_entry, char* name, ulong fd )
{
    byte* b= (byte*)dir_entry;
    int   ii;

    if (*name==NUL) {          /* if entry must be cleared */
        *dir_entry->name= NUL; /* clear string: do it the same way as OS-9 */
    }
    else {
        for (ii=0; ii<DIRENTRYSZ; ii++) {
            *b= NUL; b++; /* Clear block */
        }

        /* write it only if new entry. A full-length name fills the field and
           has no NUL after it; strcpy put one in the FD sector number and
           tripped the host's overflow check (a 28-character makdir killed
           the emulator on macOS). */
        size_t len= strlen( name );
        if (len>DIRNAMSZ) len= DIRNAMSZ;
        memcpy( dir_entry->name, name, len );
        dir_entry->name[ len-1 ]|= 0x80;  /* the last character, OS-9 style */
                     dir_entry->fdsect= os9_long( fd );
    }
} /* Fill_DirEntry */

static os9err Access_DirEntry( rbfdev_typ* dev, ulong dfd,  ulong fd,
                                                char* name, uint32_t *deptr )
{
    os9err          err, cer;
    uint32_t        dir_len;
    os9direntry_typ dir_entry;
    char            entryName[DIRNAMSZ+1];
    ushort          sp;
    syspath_typ*    spP;
    Boolean         found;
    Boolean         deleteIt= (fd==0);
    char            *cmp, *fill;
         
    if (deleteIt)  { cmp= name; fill= "";   }
    else           { cmp= "";   fill= name; }
            
    err= OpenDir( dev,dfd, &sp ); if (err) return err;
    spP= &syspaths[sp];
    
    while (true) {               dir_len= DIRENTRYSZ; /* read 1 dir entry */
            err= DoAccess( spP, &dir_len, (char*)&dir_entry, false,false );
                                     DirEntry_Name( &dir_entry, entryName );
        if (err) {
            if (err!=E_EOF)              break;
            if (deleteIt) { err= E_PNNF; break; } /* E_EOF for del: not found */
        }
        
            found= (ustrcmp( cmp, entryName )==0);
        if (found)                       /* make something with the entry */
            spP->u.rbf.currPos= spP->u.rbf.currPos-dir_len;  /* seek back */
        
        if (err==E_EOF || found) {
            *deptr= spP->u.rbf.currPos;
            Fill_DirEntry               ( &dir_entry, fill, fd   );
                                 dir_len= DIRENTRYSZ;
            err= DoAccess( spP, &dir_len, (char*)&dir_entry, false,true ); 
            break;
        }
    } /* loop */
    
    cer= CloseDir( sp ); if (!err) err= cer;
    return err;
} /* Access_DirEntry */

static os9err Delete_DirEntry( rbfdev_typ* dev, ulong fd,  char* name )
{   /* file sector 0 deletes the file */
    uint32_t d; /* no interest in this value here */
    return Access_DirEntry( dev, fd, 0, name, &d );
} /* Delete_DirEntry */

static os9err touchfile_RBF( syspath_typ* spP, Boolean creDat )
{
    struct tm tim;  /* Important Note: internal use of <tm> as done in OS-9 */

    GetTim       ( &tim );
    spP->fd_sct[3]= tim.tm_year;
    spP->fd_sct[4]= tim.tm_mon+1; /* somewhat different month notation */
    spP->fd_sct[5]= tim.tm_mday;
    spP->fd_sct[6]= tim.tm_hour;
    spP->fd_sct[7]= tim.tm_min;
    
    if (creDat) {
        spP->fd_sct[13]= tim.tm_year;
        spP->fd_sct[14]= tim.tm_mon+1; /* somewhat different month notation */
        spP->fd_sct[15]= tim.tm_mday;
    }
    
    return WriteFD( spP );
} /* touchfile_RBF */

static os9err CreateNewFile( ushort pid, syspath_typ* spP, byte fileAtt, char* name, ulong csize )
{
    os9err      err;
    rbf_typ*    rbf= &spP->u.rbf;
    rbfdev_typ* dev= &rbfdev[rbf->devnr];
    ulong       sct=  dev->sctSize;
    ulong       clu=  dev->clusterSize;
    ulong       dfd=  rbf->fd_nr;
    uint32_t*   d  = &rbf->deptr;
    ulong       fd, scs, ascs, sTmp;
    ushort      owner= CallerOwner(pid);

    if (strlen(name)>DIRNAMSZ) return E_BPNAM;
    
         /* 1 fd + sectors according to cluster size */
    scs= (csize+sct-1)/sct+1;
    scs=       (scs-1)/clu+1; /* and adapt for cluster granularity */
    scs=        scs   *clu;
  //printf( "a) scs=%d\n", scs );
    err= AllocateBlocks ( spP, scs, &fd,  &ascs, 0 ); if (err) return err;
  //printf( "c) err=%d, ascs=%d\n", err, ascs );
         spP->u.rbf.fd_nr=          fd; /* access them correctly */
         RingJoin( spP );               /* now a path on the new file, not the dir */
         spP->u.rbf.fddir=     dfd;
    err= Create_FD      ( spP,         fileAtt, owner, 0 ); if (err) return err;
    err= Access_DirEntry( dev, dfd, fd,   name, d ); if (err) return err;

    if (scs>1) { // Adaption for scs>1 only
      sTmp= ascs; // get a copy before changing 
      fd++; ascs--; // get the remaining part
      err=   AdaptAlloc_FD ( spP,            fd,  ascs    ); if (err) return err;
    
      while (sTmp<scs) {
      //printf( "remania=%d\n", scs-sTmp );
        err= AllocateBlocks( spP, scs-sTmp, &fd, &ascs, 0 ); if (err) return err;
        err= AdaptAlloc_FD ( spP,            fd,  ascs    ); if (err) return err;
        sTmp+= ascs;
      } // while
    } // if
    
           err= touchfile_RBF( spP,true );
    if   (!err) syspath_setname( spP, name ); /* assign file name, if everything is ok */
    return err;
} /* CreateNewFile */

static os9err ConvertToDir( syspath_typ* spP )
/* Mark the freshly made file a directory. Only the directory bit is added: the
   permissions came from I$MakDir's d1 when the file was created, and a fixed
   $BF here used to replace them, whatever the caller asked for. */
{
  Set_FDAtt     ( spP, (byte)(FDAtt( spP ) | 0x80) );  /* as directory */
  return WriteFD( spP );
} /* ConvertToDir */

/* ------------------------------------------------------------ */
/* visible procs */

os9err pRopen( ushort pid, syspath_typ* spP, ushort *modeP, const char* name )
{
    char            tmp[OS9PATHLEN];
    char*           pathname;
    rbf_typ*        rbf= &spP->u.rbf;
    rbfdev_typ*     dev;
    os9err          err= 0;
    ulong           sect, slim, size, totsize, pref;
    uint32_t        dir_len;
    os9direntry_typ dir_entry;
    char            entryName[DIRNAMSZ+1];
    char            cmp_entry[OS9NAMELEN];
    int             root, isFileEntry;    
    char*           p;
    byte            attr;
    ushort          cdv;
    long            ls;
    process_typ*    cp    = &procs[pid];
    Boolean         isExec= IsExec(*modeP);
    Boolean         isFile=       (*modeP & 0x80)==0;
    Boolean         cre   = IsCrea(*modeP);
  //byte            mode  =       (*modeP & 0x47); /* attributes to be used */
    char*           co    = cre    ? "create":"open";
    char*           fo    = isFile ? "file"  :"dir";
    char*           curpath;
    Boolean         new_inst;
    Boolean         isAbs;
    
    #ifdef RBF_CACHE
  //char            vvv[OS9PATHLEN];
    uint32_t        fd_hash;
    dirtable_entry* mP= NULL;
    #endif

    /* -1 and terminate: EatBack() and the root comparison below both walk
       <tmp> as a C string, so an over-length pathname left it unterminated and
       they ran off the end of the frame. */
    strncpy ( tmp,name, OS9PATHLEN-1 );
              tmp[      OS9PATHLEN-1 ]= NUL;
    pathname= tmp;
    EatBackOS9( tmp );  /* normalize /dev/. and /dev/./ to /dev before root check */
    debugprintf(dbgFiles,dbgNorm,("# RBF %s: '%s' (%s)\n" , co, pathname, fo ));

    rbf->currPos= 0; /* initialize position to 0 */
    rbf->lastPos= 0;
    rbf->flushFDCache= false;
    rbf->sameFile= spP->nr; /* alone until RingJoin finds this file's others */
    rbf->waitPid = 0;
    rbf->lockTicks= 0;
    rbf->waitUntil= 0;
    rbf->ownPid  = currentpid;
    rbf->single  = (*modeP & poSingle)!=0;
    rbf->updMode = false;
    rbf->sysPath = false;
    rbf->lockBeg = 0;
    rbf->lockEnd = 0;
    rbf->eofLock = false;

        root= IsRoot( pathname ); /* root path must be a directory */
    if (root && isFile) return E_FNA;

    /* get pointers to execution/current path */
    if (isExec) { cdv= cp->x.dev; ls= cp->x.lsn; curpath= cp->x.path; }
    else        { cdv= cp->d.dev; ls= cp->d.lsn; curpath= cp->d.path; }

    err= DeviceInit( pid, &dev, spP, cdv, pathname,curpath, *modeP, &new_inst ); if (err) {
        return err;
    }
    debugprintf(dbgFiles,dbgNorm,("# RBF before adapt '%s' '%s'\n" , pathname,mnt_name ));
    AdaptPath            ( dev,          &pathname ); /* adapt to offical name */
    debugprintf(dbgFiles,dbgNorm,("# RBF after  adapt '%s' '%s'\n" , pathname,mnt_name ));

    /* now the path could be a new root */
        root= IsRoot(pathname); /* root path must be a directory */
    if (root && isFile) return E_FNA;
    if (root && new_inst && *mnt_name!=NUL) {
        strcpy( pathname,  PSEP_STR );
        strcat( pathname,  mnt_name ); /* the mount name can be different */
        strcpy( dev->name, mnt_name );           /* to the root path name */
    } // if
    
    debugprintf(dbgFiles,dbgNorm,("# RBF mount  adapt '%s' '%s' %d %d\n" , pathname,mnt_name, root,new_inst ));
    
    rbf->devnr = dev->nr;
    rbf->sas   = dev->sas;        /* PD_SAS starts as the device's (SS_Opt changes it) */
    rbf->diskID= dev->last_diskID;
    rbf->wMode = IsWrite(*modeP);
    rbf->updMode= IsRW(*modeP); /* read+write: a read here locks what it read */

    /* Nothing held yet. A creator's EOF lock is taken at the end of pRopen,
     * once the path is in its file's ring: set here, the RingJoin calls on
     * the way (CreateNewFile's and pRopen's own) dropped it again as the lock
     * of the directory the path had walked through. */
    rbf->eofLock= false;
//  printf( "GetBuffers %08X %08X\n", spP->fd_sct, spP->rw_sct );
    GetBuffers ( dev,spP ); /* get the internal buffer structures now */
    spP->rw_nr = 0;         /* undefined */
   
    do {
        /* take care of write protection */     
        if (cre && dev->wProtected) { err= E_WP; break; }

                   p= pathname; /* that's it for raw mode */
        if (IsRaw( p )) {
          if (rbf->wMode) {
              rbf->flushFDCache= true; // flush it in fact when closing the path
          // main_printf( "Flush RBF cache: pathname='%s' id=%d\n", pathname, pid );
          } // if

                spP->rawMode= true;
          syspath_setname( spP,pathname ); if (cre) { err= E_CEF; break; }
          return 0;
        } // if (IsRaw)

        isAbs= AbsPath( p ); /* if abs path -> search from the root */

        #ifdef RBF_CACHE
          if (isAbs || strcmp( curpath,"" )==0) {
            strcpy( spP->fullName, pathname );
          }
          else {
            strcpy( spP->fullName, curpath  );
            strcat( spP->fullName, PSEP_STR );
            strcat( spP->fullName, pathname );
          } // if
          
          err= FD_ID( spP->fullName, NULL, &fd_hash, &mP ); if (err) break;
        //main_printf( "'%s' hash=%d fd=%06X\n", vvv, fd_rbf, mP->dirid );
        #endif

        if (isAbs) {               
                err= RootLSN( pid, dev, spP, false );
            if (err==E_DIDC) err= 0;       /* changes recognized */
            if (err) break;       /* for all other errors: break */
            
            err= ReadFD( spP ); if (err) break;
            if (root) {
                syspath_setname( spP,pathname+1 );
                err= FD_Segment( spP, &attr,&size,&totsize,&sect,&slim, &pref ); if (err) break;
                rbf->lastPos= size;                   /* last pos is the filesize */
                rbf->att    = attr;                   /* save attributes */
                if (!has_perm( pid, attr, FDOwn(spP), permRead )) return E_FNA;
                return 0;
            } // if
            if (!has_perm( pid, FDAtt(spP), FDOwn(spP), permRead )) { err= E_FNA; break; }
                              p++; /* cut root path */
            err= CutOS9Path( &p, (char*)&cmp_entry ); if (err) break;
        }
        else {  /* if string is empty */
            if (*pathname==NUL) { err= E_FNA; break; }
            rbf->fd_nr= ls; /* take current path */
            err= ReadFD( spP );      if (err) break;
            if (!has_perm( pid, FDAtt(spP), FDOwn(spP), permRead )) { err= E_FNA; break; }
        }
    } while (false);
    if (err) return err;

    rbf->fddir= rbf->fd_nr;
    rbf->deptr= 0;
    err= CutOS9Path( &p, (char*)&cmp_entry ); if (err) return err;
    
    /* this is the recursion loop for directory entries */
    while (true) {               dir_len= DIRENTRYSZ; /* read 1 dir entry */
            err= DoAccess( spP, &dir_len, (char*)&dir_entry, false,false ); 
        DirEntry_Name( &dir_entry, entryName );    /* normalized, for compare */
        if (err) {
            if (err==E_EOF) {           /* do not create new sub paths !! */
                if (cre && strcmp( p,"" )==0) {            /* create it ? */
                    if (!has_perm( pid, FDAtt(spP), FDOwn(spP), permWrite )) {
                        err= E_FNA;
                    }
                    else {
                        err= CreateNewFile( pid, spP, procs[pid].fileAtt,
                              (char*)&cmp_entry, procs[pid].cre_initsize );
                        /* the new file's attributes, not those of the directory
                           searched to reach it: they decide single-user sharing
                           and are what SS_Opt reports as PD_ATT (pre-release
                           review) */
                        if (!err) rbf->att= spP->fd_sct[ 0 ];
                        rbf->currPos= 0;  /* initialize position to 0 */
                        rbf->lastPos= 0;
                    }
                }
                else err= E_PNNF; /* OS-9 expects E_PNNF, if entry not found */

            }
            break; /* leave loop also, if file has been correctly created */
        } /* if */

        debugprintf(dbgFiles,dbgDetail,("# RBF path : \"%s\" \"%s\"\n", 
                                           cmp_entry,entryName ));
        if (ustrcmp(entryName,cmp_entry)==0) { /* now the entry has been found */
            debugprintf(dbgFiles,dbgNorm,("# RBF path found : \"%s\"\n", entryName ));
            rbf->fddir= rbf->fd_nr;
            rbf->deptr= rbf->currPos-DIRENTRYSZ;
            rbf->currPos= 0;                      /* initialize position to 0 */
            rbf->lastPos= 0;                      /* last       position is 0 */

                             rbf->fd_nr= DirLSN( &dir_entry );
            err= ReadFD    ( spP );                                          if (err) break;
            err= FD_Segment( spP, &attr,&size,&totsize,&sect,&slim, &pref ); if (err) break;
            rbf->lastPos= size;                   /* last pos is the filesize */
            rbf->att    = attr;                   /* save attributes */
            isFileEntry= (attr & 0x80)==0x00;     /* recognized as file entry */

            /* every directory reached along the path -- intermediate or the
             * final target -- needs read permission to be searched/entered */
            if (!isFileEntry && !has_perm( pid, attr, FDOwn(spP), permRead )) {
                err= E_FNA; break;
            }

            err= CutOS9Path( &p, (char*)&cmp_entry ); if (err) break;

            if (*cmp_entry==NUL) {                /* no more sub directories */
                syspath_setname( spP, entryName );
                
                if   (isFileEntry) {              /* if it is a file entry */
                  if (isFile) {
                    if (cre)  err= E_CEF;         /* already there */
                    else if (!has_open_perm( pid, attr, FDOwn(spP), *modeP ))
                              err= E_FNA;          /* requested access not granted */
                    else      err= 0;             /* is there as file -> ok */
                  }
                  else      { err= E_FNA;         /* is path, should be file */
                    if (RBF_ImgSize( size ) &&
                       !spP->fullsearch) {        /* but try it again in special mode */
                        spP->fullsearch= true;
                      debugprintf(dbgFiles,dbgNorm,("# not a path >>fullsearch '%s'\n",         
                                                    pathname ));
                      err= pRopen( pid, spP, modeP, pathname );
                      debugprintf(dbgFiles,dbgNorm,("# not a path <<fullsearch '%s', err=%d\n",
                                                    pathname, err ));
                      spP->fullsearch= false;
                    } /* if */
                  }
                }
                else {                          /* if it is a dir  entry */
                  /* Asked for a file, found a directory. Creating one is still
                     "already there" (I$MakDir's errors are E$BPNam and E$CEF,
                     page 2 - 15): makdir creates through this open, and
                     answering E$FNA here told perl's mkdir and every script
                     testing for E$CEF something else. Only a plain open gets
                     "not a file". */
                  if (isFile) err= cre ? E_CEF : E_FNA;
                  /* A directory opened (or made current) must grant the access
                     the mode asks for, as a file does above: "the caller must
                     have access permission for the specified mode" (I$ChgDir,
                     page 2-3), and "the access mode must conform to the access
                     permissions" (I$Open, page 2-16). Only read was checked, on
                     the way through; update or execute on a d---rewr directory
                     was granted. */
                  else if (!has_open_perm( pid, attr, FDOwn(spP), *modeP )) err= E_FNA;
                  else        err= 0;           /* is there path -> ok */
                }
                
                if (!err &&  rbf->wMode
                         && !dev->wProtected) err= touchfile_RBF( spP, false );
                break;
            }
            else {                                      /* more sub directories ? */
                if (isFileEntry) { err= E_FNA; break; } /* must be a directory */
            }
        } /* if entry found */
    } /* while (true) */    
    
    #ifdef RBF_CACHE
    if (!err) {
      if ((mP->dirid!=0 && 
           mP->dirid!=rbf->fd_nr) || strcmp( spP->fullName,"" )==0) {
        main_printf( "wrong '%s' %06X <> %06X\n", spP->fullName, mP->dirid, rbf->fd_nr );
      } // if
      
      mP->dirid= rbf->fd_nr;
    } // if
    #endif
    
//  printf( "RelBuffers %08X %08X %d\n", spP->fd_sct, spP->rw_sct, err );
    /* Checked here, before RingJoin, so a refusal takes the ordinary error exit
     * and leaves nothing spliced into the file's ring to unpick. */
    /* single-user by the open's mode bit OR by the file's own attribute: "If
       the single-user bit is set, only one process may open the file at a time"
       (page 7-9) is about the S bit in the attribute byte, and only the mode
       bit was honoured. The other side needs no change: another path to the
       same file has the same attribute byte. */
    if (!err && ShareConflict( spP, rbf->single || (rbf->att & 0x40)!=0 )) err= os9error( E_SHARE );

    if    (err) ReleaseBuffers( spP );
    else {
        RingJoin( spP ); /* <fd_nr> is only final once open succeeds */

        /* "As soon as the file is created, EOF lock is gained" -- ch.7. A file
         * just created is empty, so its creator is standing at its end by
         * definition, and this is what lets a spooler open the listing and
         * wait before the assembler has written a single line. An ordinary
         * open of an existing file takes nothing here: it gains the lock when
         * an access actually lands at the end, which DoAccess decides. */
        rbf->eofLock= cre && rbf->wMode;
    }
    return err;
} /* pRopen */

os9err pRclose( ushort pid, syspath_typ* spP )
{
    os9err      err=  0;
    rbf_typ*    rbf= &spP->u.rbf;
    rbfdev_typ* dev= &rbfdev[rbf->devnr];
    ulong       crp=  rbf->currPos;
    ulong       lsp=  rbf->lastPos;
    ulong       v;

    debugprintf(dbgFiles,dbgNorm,("# RBF close (pid=%d) wMode/rawMode/mustW: %d %d $%x\n", 
                                     pid, rbf->wMode,spP->rawMode, spP->mustW ));

    do {
        if (rbf->wMode==0) break; /* no change in read mode */
        
        if (spP->mustW) { /* write the last cached sector */
            err= WriteSector( dev, spP->mustW,1, spP->rw_sct ); if (err) break;
        } // if

        if (spP->rawMode) break; // no more things to do

        /* set file size at close */
            v= FDSize( spP ); 
        if (v< lsp) { 
            v= lsp; Set_FDSize( spP,v ); /* new file size */
            err=       WriteFD( spP ); if (err) break;
        }

        /* release remaining part, if pointer is not at the end of file --
         * and only when no other path is open on the file. Another path took
         * this one's segment list (RingPublish/RingAdopt), preallocation
         * included; it writes into those sectors without allocating them and
         * writes its FD back at its own close. Freed here, the next allocation
         * handed them to a second file: two files on the same sectors. The
         * last path out trims, or the spare sectors simply stay with the file. */
        if (crp!=0 &&
            crp==lsp && !RingHasOther( spP )) {
            Set_FDSize        ( spP,crp ); /* new file size */        
            err= ReleaseBlocks( spP,crp );
        }
    } while (false);
    
    #ifdef RBF_CACHE
      if (rbf->flushFDCache) Flush_FDCache( dev->name );
    #endif
    
    /* before leaving the ring: anyone waiting on this file must look again --
     * if this was the writer they were waiting for, there is nothing more
     * coming and what they will now see is a genuine end of file.
     * Dropping the end of the file BEFORE the wake, not after: a follower
     * woken while this path still held it would look, still find it held, and
     * go straight back to sleep with nothing left alive to wake it again. */
    rbf->eofLock=  false;
    WakeOnFile    ( spP );

    ReleaseBuffers( spP );
    return err;
} /* pRclose */

os9err pRread( _pid_, syspath_typ* spP, uint32_t *lenP, char* buffer )
{   return DoAccess( spP, lenP,buffer, false,false );
} /* pRread */

os9err pRreadln ( _pid_, syspath_typ* spP, uint32_t *lenP, char* buffer )
{   return DoAccess( spP, lenP,buffer, true, false );
} /* pRreadln */

os9err pRwrite  ( _pid_, syspath_typ* spP, uint32_t *lenP, char* buffer )
{   return DoAccess( spP, lenP,buffer, false,true );
} /* pRwrite */

os9err pRwriteln( _pid_, syspath_typ* spP, uint32_t *lenP, char* buffer )
{   return DoAccess( spP, lenP,buffer, true, true );
} /* pRwriteln */

os9err pRseek( _pid_, syspath_typ* spP, uint32_t *posP )
/* seek to new file position <posP> */
{   
  rbfdev_typ* dev= &rbfdev[spP->u.rbf.devnr];
  debugprintf(dbgFiles,dbgNorm,("# RBF seek: dev='%s' file='%s' pos: %x\n", 
                                     dev->name, spP->name, *posP));
  spP->u.rbf.currPos= *posP;
    
//if (*posP==0x2a26) // debugging special alloc problem
//  upe_printf( "hallo\n" );
      
  return 0;
} /* pRseek */

os9err pRchd( ushort pid, syspath_typ* spP, ushort *modeP, const char* pathname )
{
    os9err       err;
    ushort       path;
    uint32_t*    xD;
    ushort*      xV;
    Boolean      exedir= IsExec(*modeP);
    process_typ* cp= &procs[pid];
    rbfdev_typ*  dev;
    int          n;
    char*        curpath;
    char         tmp[OS9PATHLEN];

      /* get current default path */
    if (exedir) { xV= &cp->x.dev; xD= &cp->x.lsn; curpath= cp->x.path; }
    else        { xV= &cp->d.dev; xD= &cp->d.lsn; curpath= cp->d.path; }
        
    err=     usrpath_open( pid,&path, fRBF, pathname,*modeP); if (err) return err;
        spP= get_syspathd( pid, cp->usrpaths[path] );
    if (spP==NULL) return os9error(E_BPNUM);
    
    /* Checked before anything is changed: the new current path is built in
       the process's own OS9PATHLEN field. A directory tree deep enough really
       exists (ten levels of 28-character names), and appending its path to
       the old one ran past the field. */
    if ((AbsPath(pathname) ? 0 : strlen( curpath )+1) + strlen( pathname ) >= OS9PATHLEN) {
        usrpath_close( pid, path );
        return os9error(E_BPNAM);
    }
    *xV= spP->u.rbf.devnr; dev= &rbfdev[ *xV ];
    *xD= spP->u.rbf.fd_nr;

    if (AbsPath(pathname)) strcpy( curpath,"" );
    else                   strcat( curpath,PSEP_STR );
    strcat( curpath,pathname );
      
    n= strlen(dev->img_name);
    if (SamePathBegin( curpath,dev->img_name ) &&
        1+strlen( dev->name )+strlen( &curpath[n] ) < OS9PATHLEN) {
        strcpy( tmp,   curpath );
        strcpy( curpath,PSEP_STR  ); /* get a str staring with dev->name instead of dev->img_name */
        strcat( curpath,dev->name );
        strcat( curpath,&tmp[n]   );
    } // if
    EatBackOS9( curpath );
    
    err= usrpath_close( pid, path ); if (err) return err;

    if (exedir) cp->x.type= fRBF; /* adapt the type */
    else        cp->d.type= fRBF;
    
    return 0;
} /* pRchd */

os9err pRdelete( ushort pid, syspath_typ* spP, ushort *modeP, const char* pathname )
{
    os9err      err, cer;
    rbfdev_typ* dev;
    ushort      path;
    ulong       dfd;
    
    #ifdef RBF_CACHE
    uint32_t        fd_hash;
    dirtable_entry* mP= NULL;
    #endif

        err= usrpath_open( pid,&path, fRBF, pathname,*modeP ); 
    if (err) return err;
        spP= get_syspath ( pid, procs[ pid ].usrpaths[ path ] ); /* get spP for fd sects */
    if (spP==NULL) return os9error(E_BPNUM);
    
    dev= &rbfdev[spP->u.rbf.devnr]; /* can't be assigned earlier */
    dfd=         spP->u.rbf.fddir;

    { /* deleting removes an entry from the parent directory: needs write there */
        ushort  dsp;
        os9err  dperr= OpenDir( dev, dfd, &dsp );
        if (dperr) { usrpath_close( pid, path ); return dperr; }
        Boolean okToDel= has_perm( pid, FDAtt(&syspaths[dsp]), FDOwn(&syspaths[dsp]), permWrite );
        os9err  dcerr = CloseDir( dsp );
        if (!okToDel) { usrpath_close( pid, path ); return E_FNA; }
        if (dcerr)    { usrpath_close( pid, path ); return dcerr; }
    }

    /* Deleting a file another path still has open for writing would strand
     * every cluster that path allocates after this point (see
     * RingHasOther). Real OS-9 refuses it; match that with E$Share -- and a
     * file open for reading as well, "the file may not already be open". */
    if (RingHasOther( spP )) { usrpath_close( pid, path ); return os9error( E_SHARE ); }

    do {
      err= Delete_DirEntry ( dev, dfd, (char*)&spP->name ); if (err) break;

      /* "Decrements link count in file descriptor. If the link count becomes
         zero, all disk space associated with the file is returned ... If the
         link count is non-zero, no disk space is returned" (Technical I/O
         Manual, RBF, I$Delete). It was returned whatever the count, so a file
         with a second name (clib link(), blarslib's ln) lost its FD and space
         under the name that survived, and the next allocation reused them --
         a LOCK file came back listing as a directory (found by osk-freeware). */
      if (FDLnk( spP )>1) {
          Set_FDLnk( spP, (byte)(FDLnk( spP )-1) );
          err= WriteFD( spP ); if (err) break;
          RingPublishFD( spP );
          spP->u.rbf.currPos= 0;   /* as below: nothing to trim at close */
          break;
      }
      err= DeallocateBlocks( spP );                         if (err) break;
      
      #ifdef RBF_CACHE
        err=   FD_ID( spP->fullName, NULL, &fd_hash, &mP ); if (err) break;
        mP->dirid= 0; // invalidate;
      #endif
      
      spP->u.rbf.currPos= 0;   /* seek back to zero, avoid release remaining part */
    } while (false);
    
    cer= usrpath_close( pid, path ); if (!err) err= cer; 
    return err;
} /* pRdelete */

os9err pRmakdir( ushort pid, syspath_typ* spP, ushort *modeP, const char* pathname )
{
    os9err   err;
    uint32_t size= 2*DIRENTRYSZ;
    os9direntry_typ dirblk[ 2 ];
    ushort path;
    
    /* "The new directory automatically has its directory bit set in the access
       permission attributes. The remaining attributes are specified by the
       bytes passed in register d1.w" (I$MakDir, page 2-15): the caller put d1
       in fileAtt. They were forced to $BF here, whatever was asked. And "If the
       execution bit is set, OS-9 begins searching for the file in the working
       execution directory": the create went through a mode of its own, which
       dropped that bit, so a relative name landed in the data directory. */
    procs[pid].fileAtt     |= 0x80;
    procs[pid].cre_initsize= 0;   /* don't use mode at the moment */
    err= usrpath_open( pid,&path, fRBF, pathname, (*modeP & poExec) | 0x03 | poCreateMask ); if (err) return err;

        spP= get_syspath( pid, procs[pid].usrpaths[path] ); /* get spP for fd sects */
    if (spP==NULL) return os9error(E_BPNUM);
    
    /* prepare the contents */
    Fill_DirEntry( &dirblk[0], "..", spP->u.rbf.fddir );    
    Fill_DirEntry( &dirblk[1], "." , spP->u.rbf.fd_nr );    
    
    err= usrpath_write( pid,path, &size, &dirblk, false ); if (err) return err;
    spP->u.rbf.currPos= 0;   /* seek back to zero, avoid release remaining part */
    err= ConvertToDir      ( spP );                        if (err) return err;
    err= usrpath_close(pid, path);                                  return err;
} /* pRmakdir */

os9err pRpos( _pid_, syspath_typ* spP, uint32_t *posP )
/* get current file position <posP> */
{
    rbf_typ*    rbf= &spP->u.rbf;
    rbfdev_typ* dev= &rbfdev[rbf->devnr];
    
    *posP= rbf->currPos;
    debugprintf(dbgFiles,dbgNorm,("# RBF pos: '%s' %d\n", dev->name, *posP ));
    return 0;
} /* pRpos */

os9err pRlock( ushort pid, syspath_typ* spP, uint32_t* d0, uint32_t* d1, uint32_t* d2 )
/* SS_Lock: take or release a record explicitly, for a program that would
 * rather say so than rely on the automatic lock a read in update mode takes.
 * <d2> is the size -- NOT d1, which carries the setstat code itself and is
 * always $11 here. Zero releases everything this path holds, -1 covers the
 * whole file, anything else covers that many bytes from the current position. */
{
    rbf_typ*     rbf= &spP->u.rbf;
    syspath_typ* spH;
    ulong        beg, end;

    (void)d0; (void)d1; /* path and setstat code; the size is in d2 */
    if (spP->rawMode) return 0;

    /* One rule, the same one the automatic lock follows: locking belongs to
     * update-mode opens. A path that cannot modify what it reads has nothing
     * to protect, and letting it take a lock anyway would give it a way to
     * hold up writers -- which is exactly the lockout this design avoids.
     * A release is always allowed: it can only ever let something go. */
    if (*d2!=0 && !rbf->updMode) return os9error( E_FNA );

    if (*d2==0) { /* release */
        LockDrop  ( spP );
        WakeOnFile( spP );
        return 0;
    } // if

    beg= rbf->currPos;
    /* Saturate instead of wrapping. The size is the guest's, so beg+size can
     * overflow 32 bits for any position past zero -- and an inverted range is
     * WORSE than a wrong one: LockHolder qualifies a holder with
     * lockBeg<lockEnd, so a wrapped range is stored, reported as success, and
     * then seen by nobody. That is exactly the "says it locked and did not"
     * failure this call was already fixed for once. A request that runs past
     * the end of the address space can only mean "from here on". */
    end= (*d2==0xFFFFFFFF || *d2 > 0xFFFFFFFF - beg)
             ? 0xFFFFFFFF
             : beg + *d2;

    /* The $FFFFFFFF sentinel is a WHOLE-FILE lock, and that is not the same as
     * "from here on": "If $FFFFFFFF bytes are requested, then the entire file
     * is locked out REGARDLESS OF WHERE THE FILE POINTER IS" (I$SetStt SS_Lock,
     * page 2 - 24). Anchoring it at currPos left every byte before the pointer
     * unprotected, so a holder that had read its way to position 90 was not
     * holding the file at all -- another process could still write at 0 and the
     * call had reported success. Only the explicit sentinel means the whole
     * file; a size that merely overflows still means "from here on", which is
     * why the saturation above stays as it is. */
    if (*d2==0xFFFFFFFF) beg= 0;

        spH= LockHolder( spP, beg,end );
    if (spH!=NULL) {
      return os9error( E_LOCK ); /* someone else's -- say so rather than block,
                                  * the caller asked for it deliberately */
    } // if

    rbf->lockBeg= beg;
    rbf->lockEnd= end;
    return 0;
} /* pRlock */

os9err pRsetopt( _pid_, syspath_typ* spP, byte* buffer )
/* SS_Opt SetStat on an RBF path.
 *
 * The manual names nine option fields a program may change through SetStat --
 * PD_STP, PD_TYP, PD_DNS, PD_CYL, PD_SID, PD_VFY, PD_SCT, PD_TOS and PD_SAS
 * -- and says the rest are protected by the file manager. Eight of the nine
 * describe a physical drive: step rate, cylinders, heads, sectors per track,
 * write verification. os9exec has no drive for those to be about, and pRopt
 * answers them from the emulated device rather than from stored path state,
 * so accepting a change would only mean storing a number nothing reads back.
 *
 * PD_SAS is the exception and the reason this routine exists: it is the
 * minimum number of sectors RBF allocates when a file grows, so a program
 * that sets it is asking for something the allocator really does -- "if your
 * system uses a small number of large files, this field should be set to a
 * relatively high value, and vice versa" (v2.4 Technical Manual, Segment
 * Allocation). It is stored per path, which is where the option section
 * lives, and DoAccess reads it from there.
 */
{
    ushort sas= GET_OS9W( buffer, PD_SAS );

    /* Zero is what an all-zero option buffer looks like, and a segment
       allocation of no sectors is not a thing to ask for: leave the path as
       it was rather than let a caller that filled in only one field silently
       clear this one. */
    if (sas!=0) spP->u.rbf.sas= sas;

    debugprintf( dbgFiles,dbgNorm,("# RBF SS_Opt: PD_SAS=%d (path '%s')\n",
                                      spP->u.rbf.sas, spP->name ));
    return 0;
} /* pRsetopt */

os9err pRticks( _pid_, syspath_typ* spP, uint32_t* d2 )
/* SS_Ticks: how long this path is willing to wait for a record somebody else
 * is holding, before giving up with E_LOCK instead of waiting on. Zero -- the
 * default -- waits for as long as it takes, which is right for a program that
 * would rather be late than fail, and wrong for one that must not hang behind
 * a peer that has stopped responding. */
{
    /* "d2.l = Delay interval ... If the high order bit is set, the lower 31
       bits are converted from 256th of a second into ticks" (I$SetStt SS_Ticks,
       page 2-28). Only the low word used to be kept, as raw ticks: $00010000
       became 0, wait for ever, and a sub-second 256ths value was a huge tick
       count. */
    spP->u.rbf.lockTicks= A_Interval( *d2 );
    return 0;
} /* pRticks */

os9err pReof( _pid_, syspath_typ* spP )
/* get current file position <posP> */
{
    rbf_typ*    rbf= &spP->u.rbf;
    rbfdev_typ* dev= &rbfdev[rbf->devnr];
    Boolean   isEOF= rbf->currPos >= rbf->lastPos;
    
    debugprintf(dbgFiles,dbgNorm,("# RBF eof: '%s' %s\n", dev->name, isEOF ? "true":"false" ));
    
    if (isEOF) return os9error(E_EOF);
    else       return 0;
} /* pReof */

/* get options for RBF file */
os9err pRopt(ushort pid, syspath_typ* spP, byte *buffer)
{
    os9err      err = pRBFopt( pid,spP, buffer );
    rbf_typ*    rbf = &spP->u.rbf;
    rbfdev_typ* dev = &rbfdev[rbf->devnr];
    ulong       sSct= dev->sctSize;
    byte*       b;
    char*       c;

    if (sSct==0) sSct= STD_SECTSIZE;     /* the std way how to handle unknow sector sizes */

    /* and fill some specific RBF path values */
    b= (byte*)&buffer[ PD_TYP    ]; *b= dev->pdtyp;
    b= (byte*)&buffer[ PD_CtrlrID]; *b= dev->scsi.ID;
    b= (byte*)&buffer[ PD_ATT    ]; *b= rbf->att;
    SET_OS9W(buffer, PD_SAS,    rbf->sas);       /* segment alloc size, this path's */
    SET_OS9W(buffer, PD_SSize,  sSct);           /* phys sect size    */
    SET_OS9L(buffer, PD_FD,     rbf->fd_nr*sSct); /* pos of file     */
    SET_OS9L(buffer, PD_DFD,    rbf->fddir*sSct); /* pos of its dir  */
    SET_OS9L(buffer, PD_DCP,    rbf->deptr);     /* dir entry pointer */
    SET_OS9L(buffer, PD_SctSiz, sSct);           /* logical sect size */
    c= (char*)&buffer[ PD_NAME ]; strcpy( c, spP->name ); /* name */

    return err;
} /* pRopt */

os9err pRready( _pid_, _spP_, uint32_t *n )
/* check ready */
{   *n= 1; return 0;
} /* pRready */

static uint32_t FDCopyCount( rbfdev_typ* dev, uint32_t d2 )
/* How many bytes SS_FD / SS_FDInf copy: "d2.w = Number of bytes to copy
 * (<=logical sector size of media)" (I$GetStt, pages 2-11 and 2-12), and for
 * SS_FDInf "(<=256)". Only the WORD counts, and never more than a sector. The
 * full 32-bit d2 used to reach memcpy: a stray upper word read past the
 * 2048-byte FD buffer on the host and wrote over guest memory. Capped at the
 * smaller host buffer too, whatever the device claims its sectors are. */
{
    uint32_t n  = loword( d2 );
    uint32_t max= dev->sctSize>0 ? dev->sctSize : STD_SECTSIZE;

    if (max>2048) max= 2048; /* fd_sct is 2048 bytes; tmp_sct is at least that */
    return n>max ? max : n;
} /* FDCopyCount */

os9err pRgetFD( _pid_, syspath_typ* spP, uint32_t *maxbytP, byte *buffer )
/* get the current FD sector of the opened path */
{
    rbfdev_typ* dev= &rbfdev[spP->u.rbf.devnr];
    uint32_t    n  = FDCopyCount( dev, *maxbytP );

    debugprintf(dbgFiles,dbgNorm,("# RBF getFD (fd/bytes): $%x %d\n", 
                                     spP->u.rbf.fd_nr, n ));

    if (!RANGE_IN_ARENA( buffer, n )) return os9error(E_BPADDR);
    memcpy( buffer, spP->fd_sct, n ); /* copy to the buffer */
    return 0;
} /* pRgetFD */

os9err pRgetFDInf( ushort pid, syspath_typ* spP, uint32_t *maxbytP,
                                                       uint32_t *fdinf, byte *buffer )
/* get any FD sector ( using variable <fdinf> ) */
{
    os9err      err;
    rbfdev_typ* dev= &rbfdev[spP->u.rbf.devnr];

    uint32_t    n  = FDCopyCount( dev, *maxbytP );

    debugprintf(dbgFiles,dbgNorm,("# RBF getFDInf (fd/bytes): $%x %d\n", 
                                     *fdinf, n ));

    /* "If SS_FDInf is called in user state, the caller must be a super-group
       user" (I$GetStt SS_FDInf, page 2-12): it reads any sector of the device
       by number. Every os9exec process is in user state. The page names no
       error; E$Permit is the super-user refusal. */
    if (!is_super( pid )) return os9error(E_PERMIT);
    if (!RANGE_IN_ARENA( buffer, n )) return os9error(E_BPADDR);
    err= ReadSector( dev,*fdinf,1, dev->tmp_sct ); if (err) return err;
    memcpy               ( buffer, dev->tmp_sct, n ); /* copy to the buffer */
    return 0;
} /* pRgetFDInf */

os9err pRsetFD( _pid_, syspath_typ* spP, byte *buffer )
/* set the current FD sector -- owner or super-user only */
{
    ulong maxbyt= 16;
    debugprintf(dbgFiles,dbgNorm,("# RBF setFD (fd/bytes): $%x %d\n",
                                     spP->u.rbf.fd_nr, (uint32_t)maxbyt ));

    /* Baseline gate, same as pRsetatt: writing the FD rewrites the owner word
     * and the attribute byte, so leaving it open let any process that could
     * merely open a file for WRITE take ownership of it (live-verified: a
     * plain user ran `chown` against a file owned 0.0 that only had
     * public-write set, and became its owner), and equally let that process
     * set attributes through SS_FD to side-step pRsetatt's check entirely.
     * Checked before the copy so a rejected call leaves spP->fd_sct
     * untouched. */
    if (!is_super(pid) && !IsOwner(pid, FDOwn(spP))) return E_FNA;

    /* "Only the super user can change the file's owner ID" (I$SetStt SS_FD,
     * page 2-24). The owner word is group.user, and an owner used to be let
     * change the group half -- a partial give-away, 1.3 handing its file to
     * 2.3, allowed on purpose and guarded only against group zero, which IS
     * the super user. rdoggett, 2026-09-15: follow the manual. Any change to
     * the owner word from a caller outside the super group is E$Permit. */
    if (!is_super(pid) && GET_OS9W( buffer,1 )!=FDOwn(spP)) return E_PERMIT;

    /* "The path must be open for write. NOTE: You can only change FD_OWN,
       FD_DAT, and FD_Creat. These are the only fields written back to disk."
       (I$SetStt SS_FD, page 2-24). All 16 bytes used to be copied, through any
       path: the attribute byte (directory bit included), the link count and
       the file size could all be rewritten from a read-only path. FD_OWN is
       $01-$02, FD_DAT $03-$07, FD_Creat $0D-$0F. */
    if ((spP->mode & 0x02)==0) return os9error(E_BMODE);
    (void)maxbyt;
    memcpy( &spP->fd_sct[1],  &buffer[1],  7 );  /* FD_OWN + FD_DAT */
    memcpy( &spP->fd_sct[13], &buffer[13], 3 );  /* FD_Creat */
    RingPublishFD( spP );  /* owner/dates just changed for every path, not one */
    return WriteFD( spP );
} /* pRsetFD */

os9err pRsize( ushort pid, syspath_typ* spP, uint32_t *sizeP )
/* get the size of a file */
{
    os9err      err;
    rbf_typ*    rbf= &spP->u.rbf;
    rbfdev_typ* dev= &rbfdev[rbf->devnr];
    byte        attr;
    ulong       sect, slim, totsize, sv, pref, fileSize;

    if (spP->rawMode) {
        *sizeP= (uint32_t)(dev->totScts*dev->sctSize);
        return 0;
	  } // if

    sv = rbf->currPos;
         rbf->currPos= 0;  /* initialize position to 0 */

    err= FD_Segment( spP, &attr,&fileSize,&totsize,&sect,&slim, &pref );
    *sizeP= (uint32_t)fileSize;

         rbf->currPos= sv; /* get saved position back */
    if  (rbf->lastPos>*sizeP) *sizeP= rbf->lastPos;

    debugprintf( dbgFiles,dbgNorm,("# RBF size (pid=%d): %d\n", pid, *sizeP ));
    return err;
} /* pRsize */

os9err pRdsize(ushort pid, syspath_typ* spP, uint32_t* size, uint32_t* dtype )
/* get the size of the device as numbers of sectors */
/* the <dtype> field will be returned as 0, to avoid problems with "castype" */
{
    os9err      err;
    rbfdev_typ* dev= &rbfdev[spP->u.rbf.devnr];
    
    err   = DevSize( dev ); if (err) return err;
    *size = dev->totScts;
    *dtype= 0;

    debugprintf( dbgFiles,dbgNorm,("# RBF dsize (pid=%d): %d\n", pid, *size ));
    return 0;
} /* pRdsize */

os9err pRsetsz( _pid_, syspath_typ* spP, uint32_t *size )
/* set the size of a file */
{
    rbf_typ*    rbf= &spP->u.rbf;
    rbfdev_typ* dev= &rbfdev[rbf->devnr];
    os9err      err;

    /* "The SETSTAT call (SS_Size) explicitly allocates file space" (I$Create,
       page 2-5). Only the size in the FD was set, so a read inside the new size
       found no sector there and answered E$EOF. Allocate what the new size
       needs the way a write does (DoAccess); a smaller size frees nothing, as
       nothing here ever has. */
    if (!spP->rawMode) {
        ulong fsize, totsize, sect, slim, pref, req, pos, scs;
        ulong ma= Max( rbf->sas,dev->clusterSize );
        byte  attr;

        FD_Segment( spP, &attr,&fsize,&totsize,&sect,&slim,&pref );
        if (*size>totsize) {
            req= ( *size-totsize-1 ) / dev->sctSize + 1;
            while (true) {
                scs= Max( ma, req ); /* alloc size might be larger */
                err= AllocateBlocks( spP,scs, &pos,&scs, pref ); if (err) break;
                err= AdaptAlloc_FD ( spP,      pos, scs       ); if (err) break;

                if (scs>=req) break;
                req-= scs; /* still asking for some more sectors */
            } // loop

            /* A disk that fills part way (E$Full, E$SLF) leaves the segments
               allocated so far in the FD, which AdaptAlloc_FD has written.
               The file's other paths must have them too: the next of those to
               write its own FD put its older copy back over them, and the
               bitmap then held sectors no FD named. The size stays as it was. */
            if (err) { RingPublishFD( spP ); return err; }
        } // if
    } // if

        rbf->lastPos= *size; /* set position      to new max */
    if (rbf->currPos> *size)
        rbf->currPos= *size; /* set position back to new max */

    Set_FDSize     ( spP,*size ); /* new file size */
    RingPublishFD  ( spP );
    RingSetLastPos ( spP,*size ); /* truncation must shrink the others too */
    return WriteFD ( spP );
} /* pRsetsz */

static os9err DirHasEntries( syspath_typ* spP, Boolean* hasP )
/* whether a directory holds anything besides ".." and ".", read sector by
   sector along its segment list -- not through DoAccess, which can park the
   caller on a lock. A deleted entry's name starts with a zero byte. */
{
    rbfdev_typ* dev = &rbfdev[spP->u.rbf.devnr];
    ulong       size= Max( FDSize( spP ), spP->u.rbf.lastPos ), seen= 0, pos, scs, k, off;
                /* the path's own end too: a directory it has just extended
                   is longer than the FD sector says until that is written */
    int         ii;
    byte*       buf;
    os9err      err= 0;

    *hasP= false;
    /* The device is not the whole truth: an entry can sit unwritten in this
       path's sector buffer or another path's. Written out first, so a
       directory with a live entry is never taken for an empty one (and turned
       into a plain file, orphaning that entry). */
    if (spP->mustW!=0 && spP->rw_sct!=NULL) {
        err= WriteSector( dev, spP->mustW,1, spP->rw_sct ); if (err) return err;
        spP->mustW= 0;
    }
    buf= malloc( dev->sctSize ); if (buf==NULL) return os9error(E_NORAM);

    for (ii=FD_Header_Size; ii+SegSize<=dev->sctSize && seen<size && !*hasP; ii+=SegSize) {
        pos= GET_OS9L(spP->fd_sct, ii) >> BpB;
        scs= GET_OS9W(spP->fd_sct, ii+3);
        if (scs==0) break;

        err= RingFlushRange( spP, pos, scs ); if (err) break;
        for (k=0; k<scs && seen<size && !*hasP && !err; k++) {
            err= ReadSector( dev, pos+k, 1, buf ); if (err) break;
            for (off=0; off+DIRENTRYSZ<=dev->sctSize && seen<size; off+=DIRENTRYSZ, seen+=DIRENTRYSZ) {
                if (seen>=2*DIRENTRYSZ && buf[ off ]!=NUL) { *hasP= true; break; }
            }
        }
    }

    free( buf );
    return err;
} /* DirHasEntries */

os9err pRsetatt( _pid_, syspath_typ* spP, uint32_t *attr )
/* set the attributes of a file -- owner or super-user only */
{
    if (!is_super(pid) && !IsOwner(pid, FDOwn(spP))) return E_FNA;

    /* "It is not permitted to set the dir bit of a non-directory file"
       (I$SetStt SS_Attr, page 2-22); it was stored as given, making a plain
       file look like a directory. The page names no error. */
    if ((*attr & 0x80) && (spP->fd_sct[0] & 0x80)==0) return os9error(E_FNA);

    /* "... or to clear the dir bit of a non-empty directory" (the same page),
       E$DNE being "an attempt ... to remove the directory attribute from a
       directory that is not empty". It turned the directory into a plain file
       and left what was in it with no entry leading to it. An emptied one --
       deldir's -- still has its bit cleared. */
    if ((*attr & 0x80)==0 && (spP->fd_sct[0] & 0x80)!=0) {
        Boolean has;
        os9err  err= DirHasEntries( spP, &has ); if (err) return err;
        if (has) return os9error(E_DNE);
    }
    Set_FDAtt      ( spP, (byte)*attr ); /* byte ordering is already correct */
    RingPublishFD  ( spP );  /* others must see the new attributes at once */
    return WriteFD ( spP );
} /* pRsetatt */

os9err pRnam( ushort pid, syspath_typ* spP, char* volname )
/* get device name of RBF device */
{
    process_typ* cp = &procs [pid];
    rbfdev_typ*  dev= &rbfdev[spP->u.rbf.devnr];
    
                     strcpy( volname, dev->name  );
    if (SamePathBegin( &cp->d.path[1],dev->name2 )) strcpy( volname, dev->name2 );
    if (SamePathBegin( &cp->d.path[1],dev->name3 )) strcpy( volname, dev->name3 );
    
    debugprintf( dbgFiles,dbgNorm,("# RBF name: %s\n", volname ));
    return 0;
} /* pRnam*/

os9err pRWTrk( ushort pid, syspath_typ* spP, uint32_t* trackNr, byte* buffer )
/* SS_WTrk: format one track of an RBF device.
   <buffer> is the caller's a0. The dispatcher has always passed it and this
   declaration has always omitted it, so every call went through a pointer of
   the wrong type; the argument survived in a register nobody read. It is named
   here to make the signature the truth, and stays unused because a track is
   formatted from the fill pattern below, not from anything the caller supplies. */
{
    os9err      err;
    rbfdev_typ* dev= &rbfdev[spP->u.rbf.devnr];
    int         ii;
    ulong       sctNr;
    uint32_t    scts, dtype;
    
    for (ii=0; ii<dev->sctSize; ii++)
        dev->tmp_sct[ii]= 0xE5; /* fill with formatting pattern */
    
    /* "Formats the entire media only when side 0 of the first accessable track
       is specified" (I$SetStt SS_WTrk, page 2-29). The loop below formats the
       whole device, so do it for track 0 only. It used to run for any track,
       from that track to the end of the device: SS_WTrk on track 1 of a
       mount -k image left $E5 from byte 16384 through its last byte. A later
       track has nothing left to format. */
    if (*trackNr!=0) return 0;

    err= pRdsize( pid,spP, &scts, &dtype );  if (err) return err; /* else scts is
                     uninitialised: pRdsize returns DevSize()'s error without setting
                     *size, and the format loop below would run a garbage sector count */

    for (ii=0; ii<scts; ii++) { sctNr= ii;
      err=    WriteSector( dev, sctNr,1, dev->tmp_sct ); if (err) return err;
    }
    
    return 0;
} /* pRWTrk*/

/* eof */
