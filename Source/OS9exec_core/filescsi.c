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
 *    Revision 1.17  2006/02/19 15:42:23  bfo
 *    Header changed to 2006
 *
 *    Revision 1.16  2005/06/30 11:38:36  bfo
 *    SCSI not yet supported for Mach-O
 *
 *    Revision 1.15  2005/04/15 11:16:09  bfo
 *    SCSI params are combined in one struct now
 *
 *    Revision 1.14  2005/01/22 15:50:31  bfo
 *    Renamed to ifdef MACOS9
 *
 *    Revision 1.13  2004/12/03 23:56:46  bfo
 *    MacOSX MACH adaptions
 *
 *    Revision 1.12  2004/11/20 11:44:07  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.11  2003/05/08 23:12:14  bfo
 *    MacOSX: RBF activated for Carbon
 *
 *    Revision 1.10  2002/10/27 23:17:46  bfo
 *    get_mem/release_mem no longer with param <mac_asHandle>
 *
 *    Revision 1.9  2002/09/11 17:15:13  bfo
 *    Definition changed from "ulong* dat_buf" ==> "byte* dat_buf"
 *
 *
 */


/* This file contains the RBF Emulator */
#include "os9exec_incl.h"
#include "filescsi.h"

#ifdef MACOS9
  #include <Scsi.h>
#elif defined(windows32)
  // NT DDK includes (extracted from NT4DDK)
  #include "devioctl.h"
  //#include <ntdddisk.h> // seems to be included in W2K windows.h
  #include "ntddscsi.h"
  #include "stddef.h"
  // spti struct with sense buffer included (as we need to specify offset...)
  typedef struct {
    SCSI_PASS_THROUGH_DIRECT sptd;
    ULONG             Filler;      // realign buffer to double word boundary
    UCHAR             ucSenseBuf[32];
  } SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;
#endif


#define CmdRead     0x08
#define CmdWrite    0x0a
#define CmdSel      0x15
#define CmdSense    0x1a
#define CmdCapacity 0x25

#define CB_Size        6
#define CB_SizeExt    10
#define SizeCmd       12

#define NTries        30



#ifdef windows32

#define MAXNUMSCSIADAPTORS 10

void scsiadaptor_help(void)
{
    int   n;
    char  w[30];
    char  v[5];
    short adaptno;
    char  adaptername[20] = "\\\\.\\scsix:";
    UCHAR buffer[2048];
    HANDLE fileHandle;
    ULONG returned,i;
    PSCSI_ADAPTER_BUS_INFO  adapterInfo;
    PSCSI_INQUIRY_DATA inquiryData;
        
    for(adaptno=0; adaptno<MAXNUMSCSIADAPTORS; adaptno++) {
      // - insert adaptor no in name
      adaptername[8]=0x30+adaptno;
      // - create file handle to use SCSI Pass Through with
      fileHandle = CreateFile((const char *)adaptername,
        GENERIC_READ,       // read only here
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
      );
      // - check error
      if (fileHandle == INVALID_HANDLE_VALUE) continue; // try next
      // - get inquiry data  
      if (!DeviceIoControl(
        fileHandle,
        IOCTL_SCSI_GET_INQUIRY_DATA,
        NULL,
        0,
        buffer,
        sizeof(buffer),
        &returned,
        FALSE
      )) {
        // failed
        CloseHandle(fileHandle);
        continue;
      }
      // - show inquiry data
      adapterInfo = (PSCSI_ADAPTER_BUS_INFO) buffer;
      n= adapterInfo->NumberOfBuses;
      upho_printf("Adapter SCSI%d has %d %s:\n",adaptno,n, n==1 ? "bus":"buses");
      for (i=0; i<adapterInfo->NumberOfBuses; i++) {
        inquiryData = (PSCSI_INQUIRY_DATA) (buffer +
          adapterInfo->BusData[i].InquiryDataOffset);
        upho_printf("- Bus #%d, Initiator SCSI-ID = %d:\n",i, adapterInfo->BusData[i].InitiatorBusId);
        while (adapterInfo->BusData[i].InquiryDataOffset) {
            memcpy( w, &inquiryData->InquiryData[ 8], 24 );
                    w[24]= NUL; /* terminate it */
            memcpy( v, &inquiryData->InquiryData[32],  4 );
                    v[ 4]= NUL; /* terminate it */
           
            upho_printf("  - SCSI ID=%d, LUN=%d: %s %s%s\n",
               inquiryData->TargetId,
               inquiryData->Lun,
               w,v,
              (inquiryData->DeviceClaimed) ? " (NOT AVAILABLE)" : "" );
              
            if (inquiryData->NextInquiryDataOffset == 0) break;
            
            inquiryData= (PSCSI_INQUIRY_DATA) (buffer +
            inquiryData->NextInquiryDataOffset);
        } // while
      } // for each bus
    } // for each adaptor
} // scsiadaptor_help


void scsi_finddefaults(void)
{
    short adaptno;
    char adaptername[20] = "\\\\.\\scsix:";
    UCHAR buffer[2048];
    HANDLE fileHandle;
    ULONG returned,i;
    PSCSI_ADAPTER_BUS_INFO  adapterInfo;
    PSCSI_INQUIRY_DATA inquiryData;
        
    for(adaptno=0; adaptno<MAXNUMSCSIADAPTORS; adaptno++) {
      // - insert adaptor no in name
      adaptername[8]=0x30+adaptno;
      // - create file handle to use SCSI Pass Through with
      fileHandle = CreateFile((const char *)adaptername,
        GENERIC_READ,       // read only here
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
      );
      // - check error
      if (fileHandle == INVALID_HANDLE_VALUE) continue; // try next
      // - get inquiry data  
      if (!DeviceIoControl(
        fileHandle,
        IOCTL_SCSI_GET_INQUIRY_DATA,
        NULL,
        0,
        buffer,
        sizeof(buffer),
        &returned,
        FALSE
      )) {
        // failed
        CloseHandle(fileHandle);
        continue;
      }
      // - analyze inquiry data
      adapterInfo = (PSCSI_ADAPTER_BUS_INFO) buffer;
      for (i=0; i<adapterInfo->NumberOfBuses; i++) {
        inquiryData = (PSCSI_INQUIRY_DATA) (buffer +
          adapterInfo->BusData[i].InquiryDataOffset);
        if ( adapterInfo->BusData[i].InitiatorBusId <=31) {
            // assume this is a SCSI bus (initiator of IDE is 255)
            defSCSIAdaptNo = adaptno;
            defSCSIBusNo = i;
            break; // done
        } // while
      } // for each bus
    } // for each adaptor
} // scsi_finddefaults
#endif



/* the SCSI lowlevel call */
static os9err SCSIcall( short scsiAdapt, ushort scsiBus, ushort scsiID, ushort scsiLUN,
                        byte* cb, ulong  cb_size, byte* dat_buf, ulong len, Boolean doWrite )
{
    os9err err;
    char*  s;

    #if defined USE_CLASSIC
      #pragma unused(scsiAdapt,scsiBus,scsiLUN)
      
      OSErr  oserr, ocerr= 0;
      ushort tib[10];
      ulong  *l;
      short  stat, message= 0;
      int    wait= 200;

          tib[0]=     os9_word( scInc  ); /* fill the tibPtr structure */
      l= &tib[1]; *l= os9_long( (ulong)dat_buf );
      l= &tib[3]; *l= os9_long( len            );
          tib[5]=     os9_word( scStop );
      l= &tib[6]; *l= os9_long( NULL   );
      l= &tib[8]; *l= os9_long( NULL   );

      do {            oserr= SCSIGet(); /* go through the SCSI commands */
          if (!oserr) oserr= SCSISelect( scsiID );
          if  (oserr) { err= host2os9err(oserr,E_UNIT); break; }

               oserr= SCSICmd( cb,cb_size );
          if (!oserr) {
              if (doWrite) oserr= SCSIWrite( tib );
              else         oserr= SCSIRead ( tib );
          }

          if (oserr==5 && *cb==CmdCapacity && dat_buf[1]!=0) oserr= 0; /* workaround */

                      /* it's done */
                      ocerr= SCSIComplete( &stat,&message, wait );
          if (!oserr) oserr= ocerr;
          
	      err= host2os9err(oserr,E_NOTRDY);
	  } while (false);

    #elif defined windows32  
      HANDLE fileHandle;

      DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;  // default
      DWORD accessMode = GENERIC_WRITE | GENERIC_READ;       // default
      // number will be substituted
      char adaptername[12] = "\\\\.\\scsix:";
      BOOL repeated;
      
      // more vars
      byte *dataBuffer = NULL; // pointer to extra data buffer if passed buffer is ill-aligned
      byte *bufptr; // buffer pointer used for chunked transfers
      ULONG returned,length;
      ULONG align;
      IO_SCSI_CAPABILITIES capabilities;
      SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER spti_desc;
      ushort k;
      ushort sensekey;
    
      // test address first
      if (scsiAdapt<0) {
        debugprintf(dbgFiles,dbgNorm,(
          "# SCSI subsystem: bad adapter No (%d>) or bad Bus No (%d)\n",
          scsiAdapt,
          scsiBus
        ));
        err=E_UNIT;
        goto exitroutine;
      }
      // default error
      err=E_UNIT;
      // set correct adapter no
      adaptername[8]=0x30+scsiAdapt;
      // - create file handle to use SCSI Pass Through with
      fileHandle = CreateFile((const char *)adaptername,
        accessMode,
        shareMode,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
      );
      // - check error
      if (fileHandle == INVALID_HANDLE_VALUE) {
        debugprintf(dbgFiles,dbgNorm,(
          "# SCSI subsystem: failed to open adapter handle for '%s', winError=%d\n",
          adaptername,
          GetLastError()
        ));
        err=E_UNIT;
        goto exitroutine;
      }
      // - get capabilities
      if (!DeviceIoControl(
        fileHandle,
        IOCTL_SCSI_GET_CAPABILITIES,
        NULL,
        0,
        &capabilities,
        sizeof(IO_SCSI_CAPABILITIES),
        &returned,
        FALSE
      )) {
      // failed
        debugprintf(dbgFiles,dbgNorm,(
          "# SCSI subsystem: IOCTL_SCSI_GET_CAPABILITIES failed, winerr=%ld\n",
          GetLastError()
        ));
        err=E_UNIT;
        goto closeandexit;        
      }
      // check capabilities
      bufptr= dat_buf; // assume we can use the passed buffer directly
      // - check alignment
      align=capabilities.AlignmentMask;
      if ((align & (ULONG)dat_buf)!=0) {
          debugprintf(dbgFiles,dbgNorm,(
              "# SCSI subsystem: Passed buffer (@ $%0lX) does not meet alignment requirement of SCSI hardware: $%0lX\n",
              (ULONG)dat_buf,
              align
          ));
          // we need an aligned temp buffer
          // - get buffer that is long enough to contain aligned data
              dataBuffer= get_mem( len+align );
          if (dataBuffer==NULL) {
              debugprintf(dbgFiles,dbgNorm,("# SCSI subsystem: Could not allocate aligned buffer\n"));
              err=E_NORAM;
              goto closeandexit;
          }
          // - determine aligned starting point in new buffer
          bufptr = (byte *)(((ULONG)dataBuffer + align) & ~align);
          // - copy data to buffer if writing
          if (doWrite) CopyMemory(bufptr,dat_buf,len);
      }   
      // - check max transfer size
      if (len>capabilities.MaximumTransferLength) {
          debugprintf(dbgFiles,dbgNorm,(
              "# SCSI subsystem: Request exceeds ($%0lX) max transfer size of $%0lX bytes\n",
              len,
              capabilities.MaximumTransferLength
          ));
          err=E_PARAM;        
          goto closeandexit;
      }
      // Now issue SCSI command
      repeated=false;      
    doItAgain: // label for retry after UNIT ATTENTION
      // - create passthrough structure for request
      ZeroMemory(&spti_desc, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
      spti_desc.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
      // - addressing
      spti_desc.sptd.PathId = scsiBus;
      spti_desc.sptd.TargetId = scsiID;
      spti_desc.sptd.Lun = scsiLUN;
      // - set up data buffers
      spti_desc.sptd.DataIn = doWrite ? SCSI_IOCTL_DATA_OUT : SCSI_IOCTL_DATA_IN; // in or out
      spti_desc.sptd.SenseInfoLength = 24; // how much sense info we'd like to get max
      spti_desc.sptd.DataTransferLength = len; // how many bytes in(to) the buffer
      spti_desc.sptd.TimeOutValue = 2;
      spti_desc.sptd.DataBuffer = bufptr;
      spti_desc.sptd.SenseInfoOffset =
         offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
      // - set up command itself       
      spti_desc.sptd.CdbLength = cb_size; // size of command block
      // - copy command bytes
      for (k=0; k<cb_size; k++) spti_desc.sptd.Cdb[k]=cb[k];
      // - issue now
      length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
      if (!DeviceIoControl(
          fileHandle,
          IOCTL_SCSI_PASS_THROUGH_DIRECT,
          &spti_desc,
          length,
          &spti_desc,
          length,
          &returned,
          FALSE
      )) {
          // failed
          debugprintf(dbgFiles,dbgNorm,(
            "# SCSI subsystem: IOCTL_SCSI_PASS_THROUGH_DIRECT failed, winerr=%ld\n",
            GetLastError()
          ));
          err=E_UNIT;
          goto closeandexit;
      }
      // - call succeeded, but check for SCSI errors
      err=0; // assume ok
      if (spti_desc.sptd.ScsiStatus) {
          // non-zero SCSI status
          debugprintf(dbgFiles,dbgNorm,(
              "# SCSI subsystem: SCSI Status = 0x%02X\n",
              spti_desc.sptd.ScsiStatus
          ));
          // show sense info if available
          sensekey=0;
          if (spti_desc.sptd.SenseInfoLength>=3) {
              // at least 3 bytes, including sense key
              sensekey = spti_desc.ucSenseBuf[2]; // sense key 
              #ifndef NODEBUG    
              debugprintf(dbgFiles,dbgDetail,("# - Sense info : "));
              for (k=0; k<spti_desc.sptd.SenseInfoLength; k++) {
                debugprintf(dbgFiles,dbgDetail,("%02X ", spti_desc.ucSenseBuf[k]));
              }
              debugprintf(dbgFiles,dbgDetail,("\n"));
              #endif
          }
          // convert SCSI error to os9error
          switch (sensekey) {
            case 2: // not ready
              err=E_NOTRDY;
              break;          
            case 3: // medium error
              err=E_BTYP;
              break;
            case 4: // hardware failure
              err=E_SEEK;
              break;
            case 5: // illegal request
              err=E_SECT;
              break;
            case 6: // Unit attention
              if (!repeated) {
                // repeat once after unit attention
                repeated=true;
                goto doItAgain;
              }
              err=E_UNIT;
              break;
            case 7: // data protect
              err=E_WP;
              break;
            default:
              err=E_UNIT;
              break;          
          }  
      }    
      // if aligned buffer was allocated, copy back (on read) and deallocate it
      if (dataBuffer) {
          // copy back to original buffer on read
          if (!doWrite) CopyMemory(dat_buf,bufptr,len);
          // anyway, get rid of buffer
          release_mem( dataBuffer );
      }
      // - close file
    closeandexit:
      CloseHandle(fileHandle);
    exitroutine:
    #else
      // linux or others???
      err=E_UNIT; // just fail
    #endif

    if (debugcheck(dbgFiles,dbgDetail)) {
        switch (cb[0]) {
            case CmdSel     : s= "CmdSel";      break;
            case CmdSense   : s= "CmdSense";    break;
            case CmdRead    : s= "CmdRead";     break;
            case CmdWrite   : s= "CmdWrite";    break;
            case CmdCapacity: s= "CmdCapacity"; break;
            default         : s= "<none>";
        }

        upe_printf("# SCSIcall: %s (err=%d)\n", s, err );
    } /* if */

    return err;
} /* SCSIcall */



os9err Set_SSize( scsi_dev* scsi, uint32_t sctSize )
/* Set the SCSI sector size */
{
    byte     cb     [CB_Size];
    uint32_t dat_buf[3];

    cb[0]= CmdSel;
    cb[1]= (scsi->LUN & 0x07)<<5;
    cb[2]= 0;
    cb[3]= 0;
    cb[4]= SizeCmd;
    cb[5]= 0;

    dat_buf[0]= os9_long( 8 ); /* block descriptor */
    dat_buf[1]= os9_long( 0 );
    dat_buf[2]= os9_long( sctSize );

    return SCSIcall( scsi->adapt, scsi->bus, scsi->ID, scsi->LUN, 
                     (byte*)&cb,CB_Size, (byte*)&dat_buf,sizeof(dat_buf), true );
} /* Set_SSize */


os9err Get_SSize( scsi_dev* scsi, uint32_t *sctSize )
/* Get the SCSI sector size */
{
    os9err   err;
    byte     cb     [CB_Size];
    /* Zeroed: SCSIcall only fills dat_buf on SUCCESS, but the read below runs
     * unconditionally, so a failed call previously published whatever was on
     * the stack as the sector size. Zero at least makes the failure
     * deterministic and obviously-wrong rather than plausible garbage.
     * (GCC -fanalyzer, -Wanalyzer-use-of-uninitialized-value.) */
    uint32_t dat_buf[3]= { 0,0,0 };

    cb[0]= CmdSense;
    cb[1]= (scsi->LUN & 0x07)<<5;
    cb[2]= 0;
    cb[3]= 0;
    cb[4]= SizeCmd;
    cb[5]= 0;

    err= SCSIcall( scsi->adapt, scsi->bus, scsi->ID, scsi->LUN,
                   (byte*)&cb,CB_Size, (byte*)&dat_buf,sizeof(dat_buf), false );
    *sctSize = os9_long( dat_buf[2] );
    return err;
} /* Set_SSize */



os9err ReadCapacity( scsi_dev* scsi, uint32_t *totScts, uint32_t *sctSize )
/* Get the SCSI device size */
{
    os9err   err;
    byte     cb     [CB_SizeExt];
    uint32_t dat_buf[3];
    
    dat_buf[0]= 0; 
    dat_buf[1]= 0; 
    dat_buf[2]= 0; 

    cb[0]= CmdCapacity;
    cb[1]= (scsi->LUN & 0x07)<<5;
    cb[2]= 0;
    cb[3]= 0;
    cb[4]= 0;
    cb[5]= 0;
    cb[6]= 0;
    cb[7]= 0;
    cb[8]= 0;
    cb[9]= 0;

    err= SCSIcall( scsi->adapt, scsi->bus, scsi->ID, scsi->LUN,
                   (byte*)&cb,CB_SizeExt, (byte*)&dat_buf,sizeof(dat_buf), false );

    *totScts= os9_long( dat_buf[0] )+1; /* +1, because capacity returns latest sector */
    *sctSize= os9_long( dat_buf[1] );
    
    return err;
} /* ReadCapacity */



os9err Get_DSize( scsi_dev* scsi, uint32_t *totScts )
/* Get the SCSI device size */
{
    os9err   err;
    byte     cb     [CB_Size];
    /* Zeroed, and this one matters for behaviour, not just for UB: SCSIcall
     * fills dat_buf only on success, yet *totScts is read from it
     * unconditionally and then TESTED (`if (*totScts==0)`) to decide whether
     * to fall back to ReadCapacity -- the path that exists precisely for
     * drives which do not support mode-sense. With stack garbage that test
     * almost never sees 0, so a failed call skipped the fallback and returned
     * a bogus geometry. Zeroing makes a failure land on 0 and take the
     * recovery path the code already provides.
     * (GCC -fanalyzer, -Wanalyzer-use-of-uninitialized-value.) */
    uint32_t dat_buf[3]= { 0,0,0 };
    uint32_t sctSize;

    cb[0]= CmdSense;
    cb[1]= (scsi->LUN & 0x07)<<5;
    cb[2]= 0;
    cb[3]= 0;
    cb[4]= SizeCmd;
    cb[5]= 0;

    err= SCSIcall( scsi->adapt, scsi->bus, scsi->ID, scsi->LUN,
                   (byte*)&cb,CB_Size, (byte*)&dat_buf,sizeof(dat_buf), false );
    *totScts= os9_long( dat_buf[1] );
     sctSize= os9_long( dat_buf[2] );
    
    /* Iomega JAZ does not support the mode sense command */
    if (*totScts==0) {
        err= ReadCapacity( scsi, totScts,&sctSize );
    } // if
    
    return err;
} /* Get_DSize */



os9err ReadFromSCSI( scsi_dev* scsi, uint32_t sectorNr, uint32_t nSectors,
                                     uint32_t len,      byte* buffer )
{
    os9err   err;
    byte     cb[CB_Size];
    /* SET_OS9L below: cb is a byte[] local (alignment 1), so the old
     * (uint32_t*)&cb[0] store was potentially misaligned. */
    int      ii;

    for (ii=0; ii<NTries; ii++) {
        SET_OS9L( cb,0, sectorNr ); /* cb0,cb1,cb2,cb3 */
                    cb[0]= CmdRead;    /* and overwrite cb0 field */
                    cb[1]= (cb[1] & 0x1F) | ((scsi->LUN & 0x07)<<5); // upper 3 bits are LUN
                    cb[4]= nSectors;
                    cb[5]= 0;

            err= SCSIcall( scsi->adapt, scsi->bus, scsi->ID, scsi->LUN, 
                           (byte*)&cb,CB_Size, buffer,len, false ); 
//      if (err) printf( "Read  %008X %d %d\n", sectorNr,ii,err );
        if (err!=E_UNIT && err!=E_SEEK) break;     
        
        #ifdef windows32
          Sleep( 100 );
        #endif
    } /* for */

    if    (err==E_UNIT) err= E_READ;
    return err;
} /* ReadFromSCSI */



os9err WriteToSCSI( scsi_dev* scsi, uint32_t sectorNr, uint32_t nSectors,
                                    uint32_t len,      byte* buffer )
{
    os9err   err;
    byte     cb[CB_Size];
    /* SET_OS9L below: cb is a byte[] local (alignment 1), so the old
     * (uint32_t*)&cb[0] store was potentially misaligned. */
    int      ii;

    for (ii=0; ii<NTries; ii++) {
        SET_OS9L( cb,0, sectorNr ); /* cb0,cb1,cb2,cb3 */
                    cb[0]= CmdWrite;   /* and overwrite cb0 field */
                    cb[1]= (cb[1] & 0x1F) | ((scsi->LUN & 0x07)<<5); // upper 3 bits are LUN
                    cb[4]= nSectors;
                    cb[5]= 0;

            err= SCSIcall( scsi->adapt, scsi->bus, scsi->ID, scsi->LUN, (byte*)&cb,CB_Size, buffer,len, true ); 
//      if (err) printf( "Write %008X %d %d\n", sectorNr,ii,err );
        if (err!=E_UNIT && err!=E_SEEK) break;     
        
        #ifdef windows32
          Sleep( 100 );
        #endif
    } /* for */

    if    (err==E_UNIT) err= E_WRITE;
    return err;
} /* WriteToSCSI */


/* eof */

