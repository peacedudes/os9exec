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
/* (c) 1993-2004 by Lukas Zeller, CH-Zuerich  */
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
 *    Revision 1.1  2004/11/27 12:19:56  bfo
 *    "net_platform" introduced
 *
 *
 */



#include "os9exec_incl.h"
#include "net_platform.h"


/* specific network definitions */
// #ifdef USE_CARBON
//  OTNotifyUPP gYieldingNotifierUPP= NULL;
// #else
//  #include <OTDebug.h>
//  #include <Threads.h>
// #endif


/*
// Newer socket libraries require these types
#if !defined __SOCKADDR_ARG
  #define    __SOCKADDR_ARG SOCKADDR_IN*
#endif

#if !defined __CONST_SOCKADDR_ARG
  #define    __CONST_SOCKADDR_ARG SOCKADDR_IN*
#endif
*/



// Common function for MacOS9
// Can't be XXXInContext for classic, because already defined in "OpenTransport.h"
// but not linkable

EndpointRef OTOpenEndpoint_( OTConfigurationRef config,
                             OTOpenFlags        oflag,
                             TEndpointInfo*     info, /* can be NULL */
                             OSStatus*          err )
{
  #ifdef USE_CARBON
    return OTOpenEndpointInContext( config, oflag, info, err, nil );
  #else
    return OTOpenEndpoint         ( config, oflag, info, err );
  #endif
} // OTOpenEndpoint_



void* OTAllocMem_( OTByteCount size )
{
  #ifdef USE_CARBON
    return OTAllocMemInContext( size, nil );
  #else
    return OTAllocMem         ( size );
  #endif
} // OTAllocMem_    



static OSStatus InitOpenTransport_()
{
  #ifdef USE_CARBON
    return InitOpenTransportInContext( kInitOTForApplicationMask, nil );
  #else
    return InitOpenTransport();
  #endif
} // InitOpenTransport_



static InetSvcRef OTOpenInternetServices_( OTConfigurationRef   cfig,
                                           OTOpenFlags          oflag,
                                           OSStatus *           err )
{
  #ifdef USE_CARBON
    return OTOpenInternetServicesInContext( cfig, oflag, err, nil );
  #else
    return OTOpenInternetServices         ( cfig, oflag, err );
  #endif
} // OTOpenInternetServices_



static void CloseOpenTransport_()
{
  #ifdef USE_CARBON
    CloseOpenTransportInContext( nil );
  #else
    CloseOpenTransport();
  #endif
} // CloseOpenTransport_




// --------------------------------------------------------------------------------------------------------
os9err NetInstall(void)
{
  OSStatus   err;
  InetSvcRef inet_services= nil;
    
  if (netInstalled) return 0; /* already done, ok */

      err= InitOpenTransport_();
  if (err)                          return E_UNIT;
  inet_services= OTOpenInternetServices_( kDefaultInternetServicesPath, 0, &err );
  if (err) { CloseOpenTransport_(); return E_UNIT; }
  
  netInstalled= true; /* it is ok now */
  return 0;
} /* NetInstall */



os9err MyInetAddr( uint32_t *inetAddr, uint32_t *dns1Addr,
                                    uint32_t *dns2Addr, char* domainName )
{
  InetInterfaceInfo iinfo;

  OSStatus err= NetInstall(); if (err) return err;

  *inetAddr= 0; /* default values */
  *dns1Addr= 0;
  *dns2Addr= 0;
  strcpy( domainName, "" );
  err= OTInetGetInterfaceInfo( &iinfo,0 ); if (err) return OS9_ENETDOWN;

  *inetAddr= iinfo.fAddress;
  *dns1Addr= iinfo.fDNSAddr;
//*dns2Addr= 0;
  strcpy( domainName, &iinfo.fDomainName );
  return 0;
} /* MyInetAddr */




OSStatus netReadBlock( _pid_, net_typ* net, uint32_t *nBytes  )
{
  OSStatus err;
  OTFlags  junkFlags;
  OTResult lookResult;

  /* keep interface fast for at least one minute */
  gNetActive= GetSystemTick()+60*TICKS_PER_SEC;
        
  /* <err> is the number of bytes read, if >=0 */
  /*       and error status, if negative */ 
      err= OTRcv( net->ep, (void*)net->transferBuffer, *nBytes, &junkFlags );
  if (err>=0) { *nBytes= err; return 0; }
  
  *nBytes= 0; 
  switch (err) {
    case kOTNoDataErr: return 0;
    case kOTLookErr  :     lookResult= OTLook(net->ep);
                       if (lookResult==T_ORDREL) { net->closeIt= true; return 0; }
                       else return E_READ;
  } /* switch */
      
  return err;
} /* netReadBlock */



OSStatus netWriteBlock( _pid_, net_typ* net, uint32_t *nBytes )
{
  OSStatus err;
  OTResult lookResult;

  /* keep interface fast for at least one minute */
  gNetActive= GetSystemTick()+60*TICKS_PER_SEC;

  /* <err> is the number of bytes written, if >=0 */
  /*       and error status, if negative */ 
      err= OTSnd( net->ep, (void*)net->transferBuffer, *nBytes, 0 );
  if (err>=0) { *nBytes= err; return 0; }

  *nBytes= 0;
  switch (err) {
    case kOTNoDataErr: 
    case kOTFlowErr  : return 0;
    case kOTLookErr  :     lookResult= OTLook(net->ep);
                       if (lookResult==T_ORDREL) { net->closeIt= true; return 0; }
                       else return E_WRITE;
  } /* switch */

  return err;
} /* netWriteBlock */



/* eof */

