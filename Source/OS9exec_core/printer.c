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
/*         for Apple Macintosh, PC, Linux     */
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
 *    Revision 1.9  2004/11/27 12:09:30  bfo
 *    _XXX_ introduced
 *
 *    Revision 1.8  2004/11/20 11:44:08  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.7  2004/10/22 22:51:12  bfo
 *    Most of the "pragma unused" eliminated
 *
 *    Revision 1.6  2002/09/11 17:03:59  bfo
 *    Adapted for the Linux version
 *
 *    Revision 1.5  2002/09/04 12:23:29  luz
 *    Implemented auto-linefeed for /lp (allowing direct printing of text files)
 *
 *    Revision 1.4  2002/07/30 17:16:34  bfo
 *    E-Mail adress beat.forster@ggaweb.ch is updated everywhere
 *
 *    Revision 1.3  2002/06/29 21:51:21  bfo
 *    GetDefaultPrinter not available for CW5 -> skip whole printer part for CW5
 *
 *    Revision 1.2  2002/06/26 21:24:39  bfo
 *    Mac version can be compiled with "printer.c"
 *
 *    Revision 1.1  2002/06/25 20:44:33  luz
 *    Added /lp printer support under windows. Not tested or even compiled for Mac
 *
 */

#include "os9exec_incl.h"



#if defined(windows32) && __MWERKS__ >= CW7_MWERKS


/* Printer I/O routines */
/* ==================== */

// local utility routines
BOOL DefaultPrinterName(char *aName, int aMaxNameLen);
BOOL OpenPrinterRaw(LPSTR szPrinterName, LPSTR szDocName, HANDLE *hPrinter);
BOOL WritePrinterRaw(HANDLE hPrinter, LPBYTE lpData, DWORD dwCount);
BOOL ClosePrinterRaw(HANDLE hPrinter);


/* --- local procedure definitions for object definition ------------------- */
os9err pPrOpen   ( ushort pid, syspath_typ*, ushort  *modeP, char* pathname );
os9err pPrClose  ( ushort pid, syspath_typ* );
os9err pPrOut    ( ushort pid, syspath_typ*, uint32_t *maxlenP, char* buffer );
os9err pPrOutLn  ( ushort pid, syspath_typ*, uint32_t *maxlenP, char* buffer );

//os9err pPrOpt    ( ushort pid, syspath_typ*,                 byte* buffer );
os9err pPrReady  ( ushort pid, syspath_typ*, uint32_t *n );
//os9err pPrSetopt ( ushort pid, syspath_typ*,                 byte* buffer );

// used from consio.c
os9err pEOF      ( ushort pid, syspath_typ*, uint32_t *maxlenP, char* buffer );
os9err pCopt(ushort pid, syspath_typ* spP, byte* buffer);
os9err pCsetopt(ushort pid, syspath_typ* spP, byte* buffer);

/* ------------------------------------------------------------------------- */

void init_Printer( fmgr_typ* f )
/* install all procedures of the printer file manager */
{
    gs_typ* gs= &f->gs;
    ss_typ* ss= &f->ss;
    
    /* main procedures */
    f->open      = (pathopfunc_typ)pPrOpen;
    f->close     = (pathopfunc_typ)pPrClose;
    f->read      = (pathopfunc_typ)pEOF;
    f->readln    = (pathopfunc_typ)pEOF;
    f->write     = (pathopfunc_typ)pPrOut;
    f->writeln   = (pathopfunc_typ)pPrOutLn;
    f->seek      = (pathopfunc_typ)pBadMode; /* not allowed */
    
    /* getstat */
    gs->_SS_Size = (pathopfunc_typ)pUnimp;   /* -- not used */
    gs->_SS_Opt  = (pathopfunc_typ)pCopt;
    gs->_SS_DevNm= (pathopfunc_typ)pSCFnam;
    gs->_SS_Pos  = (pathopfunc_typ)pBadMode;
    gs->_SS_EOF  = (pathopfunc_typ)pNop;         /* ignored */
    gs->_SS_Ready= (pathopfunc_typ)pPrReady;

    /* setstat */
    ss->_SS_Size = (pathopfunc_typ)pNop;         /* ignored */
    ss->_SS_Opt  = (pathopfunc_typ)pCsetopt;
    ss->_SS_Attr = (pathopfunc_typ)pNop;         /* ignored */
} /* init_Cons */



/* --------------------------------------------------------- */

// Utility routines

// get default printer name
BOOL DefaultPrinterName(char *aName, int aMaxNameLen)
{
  // HOWTO: Retrieve and Set the Default Printer in Windows
  // http://support.microsoft.com/support/kb/articles/q246/7/72.asp
  // HOWTO: Get and Set the Default Printer in Windows
  // http://support.microsoft.com/support/kb/articles/q135/3/87.asp
  OSVERSIONINFO os;
  DWORD siz;
  const DWORD bufsiz=300;
  char buffer[bufsiz];
  const char *p;

  memset((void *)&os,0,sizeof(os));
  os.dwOSVersionInfoSize=sizeof(os);
  // get OS version
  if (!GetVersionEx(&os)) return FALSE;
  
  // check platform
  if (os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
    // not supported by now
    return FALSE;
    /* VB code to do it
    ' Determine how big the buffer needs to be
    Call EnumPrinters(PRINTER_ENUM_DEFAULT, vbNullString, 2, ByVal 0&, 0, BufSize, Returned)
    If BufSize > 0 Then
      ' Size buffer accordingly
      ReDim Buffer(0 To BufSize - 1) As Byte
      ' Call again to retrieve needed info
      Call EnumPrinters(PRINTER_ENUM_DEFAULT, vbNullString, 2, Buffer(0), BufSize, BufSize, Returned)
      ' A pointer to the default printer name is
      ' returned at the 5th byte in the buffer.
      Call CopyMemory(pPrinterName, Buffer(4), 4)
      Result = PointerToStringA(pPrinterName)
    End If
    */
  }
  else if (os.dwPlatformId == VER_PLATFORM_WIN32_NT) {
    // 
    if (os.dwMajorVersion >= 5) {
      siz = aMaxNameLen;
      if (!GetDefaultPrinter(aName,&siz))
        return FALSE;
    }
    else {
      // The old WIN.INI [Windows] section is mapped to
      // HKCU\Software\Microsoft\Windows NT\CurrentVersion\Windows
      // and we can just use GetProfileString to extract! :-)
      // Returns: "printer name,driver name,port"
      if (!GetProfileString("Windows", "device", "", buffer, bufsiz))
        return FALSE;
      // copy only name
      p=strchr(buffer,',');
      if (!p || p-buffer>aMaxNameLen) strncpy(aName,buffer,aMaxNameLen);
      else strncpy(aName,buffer,p-buffer);
    }
  }
  return TRUE;
} // DefaultPrinterName



// OpenPrinterRaw - open printer for direct writing
// 
// szPrinterName: NULL-terminated string specifying printer name
// szDocName:     NULL-terminated string specifying document name
// hPrinter:      returns handle to printer
// 
// Returns: TRUE for success, FALSE for failure.
// 
BOOL OpenPrinterRaw(LPSTR szPrinterName, LPSTR szDocName, HANDLE *hPrinter)
{
  DOC_INFO_1 DocInfo;
  DWORD      dwJob;

  // Need a handle to the printer.
  if( ! OpenPrinter( szPrinterName, hPrinter, NULL ) )
    return FALSE;

  // Fill in the structure with info about this "document."
  DocInfo.pDocName = szDocName;
  DocInfo.pOutputFile = NULL;
  DocInfo.pDatatype = "RAW";
  // Inform the spooler the document is beginning.
  if( (dwJob = StartDocPrinter( *hPrinter, 1, (LPBYTE)&DocInfo )) == 0 )
  {
    ClosePrinter( *hPrinter );
    return FALSE;
  }
  // Start a page.
  if( ! StartPagePrinter( *hPrinter ) )
  {
    EndDocPrinter( *hPrinter );
    ClosePrinter( *hPrinter );
    return FALSE;
  }
  return TRUE;
} // OpenPrinterRaw


// WritePrinterRaw - write data to printer
// 
// hPrinter:      handle to printer
// lpData:        Pointer to raw data bytes
// dwCount        Length of lpData in bytes
// 
// Returns: TRUE for success, FALSE for failure.
// 
BOOL WritePrinterRaw(HANDLE hPrinter, LPBYTE lpData, DWORD dwCount)
{
  DWORD byteswritten;
  // Send the data to the printer.
  if( !WritePrinter( hPrinter, lpData, dwCount, &byteswritten ) ) {
    EndPagePrinter( hPrinter );
    EndDocPrinter( hPrinter );
    ClosePrinter( hPrinter );
    return FALSE;
  }
  if (byteswritten!=dwCount) {
    EndPagePrinter( hPrinter );
    EndDocPrinter( hPrinter );
    ClosePrinter( hPrinter );
    return FALSE;
  }
  return TRUE;
} // WritePrinterRaw



// ClosePrinterRaw - end writing data to printer
// 
// hPrinter:      handle to printer
// 
// Returns: TRUE for success, FALSE for failure.
// 
BOOL ClosePrinterRaw(HANDLE hPrinter)
{
  // End the page.
  if( ! EndPagePrinter( hPrinter ) )
  {
    EndDocPrinter( hPrinter );
    ClosePrinter( hPrinter );
    return FALSE;
  }
  // Inform the spooler that the document is ending.
  if( ! EndDocPrinter( hPrinter ) )
  {
    ClosePrinter( hPrinter );
    return FALSE;
  }
  // Tidy up the printer handle.
  ClosePrinter( hPrinter );
  return TRUE;
} // ClosePrinterRaw


/* --------------------------------------------------------- */



os9err pPrOpen(ushort pid, syspath_typ* spP, _modeP_, char* name)
/* routine for opening printer devices */
{
    os9err err; 
    int    id = -1;
    const int maxnamelen=200;
    char printername[maxnamelen];
    

    while (true) {
        // default printer
        if (ustrcmp(name,MainPrinter)==0) { id=0; break; } // "/lp"
        // %%% maybe add support for named printers like /lpt/LaserWriter1 later
        break;
    } /* end exit part */

    if (id<0) return E_MNF; // does not exist
    
    spP->term_id= id; // use term-id for LPT port number, 0=default printer
    strcpy( spP->name,&name[1] ); // copy the device name

    /* get the initialised path option table */
    err= pSCFopt( pid,spP, (byte*)&spP->opt ); /* no err returned */
    
    /* now do the actual open */
    // %%% needs to be enhanced for named printers, now for default printer only
    if (!DefaultPrinterName(printername,maxnamelen))
      return E_PNNF; // default printer does not exist
    if (!OpenPrinterRaw(printername, "os9exec printer job", &(spP->printerHandle)))
      return E_PNNF; // default printer does not exist
    
    debugprintf( dbgSpecialIO,dbgDetail,("# pPrOpen (%s): successful, pid=%d\n",
                                           name, pid ));
    return 0;
} /* pPrOpen */


os9err pPrClose( ushort pid, syspath_typ* spP )
{
  if (!ClosePrinterRaw(spP->printerHandle))
    return E_FNA;
  return 0;
} /* pPrClose */


/* output to printer */
os9err pPrOut  ( ushort pid, syspath_typ* spP, uint32_t *maxlenP, char* buffer )
{
  struct _sgs* ot = (struct _sgs*)&spP->opt; /* path opt table */
  char *p,*q;

  // search for lineends
  p=buffer; // start here
  q=buffer+*maxlenP; // end here
  // write line-by-line
  if (ot->_sgs_eorch) {
    q=p; // start at beginning
    while (q<buffer+*maxlenP) {
      if (*q==ot->_sgs_eorch) {
        q++; // line end inclusive!
        // write all up to here
        if (!WritePrinterRaw(spP->printerHandle, p, q-p))
          return E_WRITE;
        // add a LF if enabled
        if (ot->_sgs_alf) {
          if (!WritePrinterRaw(spP->printerHandle, "\x0A", 1))
            return E_WRITE;            
        }
        // update to what we've already written
        p=q;
      }
      else
        q++;
    }
  }
  // write rest (between p and q)
  if (p<q) {
    if (!WritePrinterRaw(spP->printerHandle, p, q-p))
      return E_WRITE;            
  }
  // ok
  return 0;
} /* pPrOut */


/* output line to printer */
os9err pPrOutLn( ushort pid, syspath_typ* spP, uint32_t *maxlenP, char* buffer)
{   
  struct _sgs* ot = (struct _sgs*)&spP->opt; /* path opt table */
  char *p;

  // find next line end in buffer
  p=buffer;
  if (ot->_sgs_eorch) {
    while (p<buffer+*maxlenP) {
      if (*p==ot->_sgs_eorch) {
        p++; // line end inclusive!
        break;
      }
      p++;
    }    
    // p is now beyond end of record or end of data
    *maxlenP=p-buffer; // number of chars to write now
  }
  // normal output
  return pPrOut(pid,spP,maxlenP,buffer);
} /* pPrOutLn */



os9err pPrReady( _pid_, syspath_typ*, uint32_t *n )
/* check ready */
/* NOTE: is valid for outputs also, when using "dup" */
{   return 0;
} /* pPrReady */

#elif defined(windows32) || defined macintosh || defined linux || defined MINGW
  void init_Printer( _fmgr_ ) { } /* do nothing */
#else
  #error "not yet implemented for other platforms than win32"
#endif

/* eof */
