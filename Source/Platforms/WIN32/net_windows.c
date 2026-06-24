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
 *
 */

#include "os9exec_incl.h"
#include "net_platform.h"

/* specific network definitions */
#include <WinSock2.h>


/*
#define WSA_ASYNC WM_USER+1

// Newer socket libraries require these types
#if !defined __SOCKADDR_ARG
  #define    __SOCKADDR_ARG SOCKADDR_IN*
#endif

#if !defined __CONST_SOCKADDR_ARG
  #define    __CONST_SOCKADDR_ARG SOCKADDR_IN*
#endif
*/


os9err NetInstall(void)
{
  WORD    wVersionRequested= MAKEWORD(2,2);
  WSADATA wsaData;

  if (netInstalled) return 0; /* already done, ok */
    
  WSAStartup( wVersionRequested, &wsaData );
  
  netInstalled= true; /* it is ok now */
  return 0;
} /* NetInstall */



os9err MyInetAddr( uint32_t *inetAddr, uint32_t *dns1Addr,
                                    uint32_t *dns2Addr, char* domainName )
{
  #define  Unk 0x7f7f7f7f
  HOSTENT*  h;
  uint32_t *a;
//uint32_t *a0, *a1, *a2;
  int       i;
      
  OSStatus err= NetInstall(); if (err) return err;

  *inetAddr= 0; /* default values */
  *dns1Addr= 0;
  *dns2Addr= 0;
  strcpy( domainName, "" );

      h= gethostbyname( "" ); /* my own host */
  if (h==NULL) return OS9_ENETDOWN;
      
  a= NULL; 
  i= h->h_length/sizeof(uint32_t);
  while (i>0 && a==NULL) {
    a= h->h_addr_list[ i-1 ];
//  printf( "length: %d %X\n", i, a );
    i--;
  } /* while */
      
//  a0= h->h_addr_list[0]; 
//  a1= h->h_addr_list[1]; 
//  a2= h->h_addr_list[2]; 
//  a1= NULL;
//  a2= NULL;
//  printf( "length: %d\n", h->h_length );

/* Windows allows more than one host address */   
//                         a= a2; /* e.g. PPP under Win2000 */
// if (a==NULL || *a==Unk) a= a1; /* e.g. LAN */
// if (a==NULL || *a==Unk) a= a0; /* if no other one */

  if (a==NULL) return OS9_ENETDOWN;       

  *inetAddr= os9_long( *a );         /* take simply the last one  */
  *dns1Addr= 0xd5a02802;             /* %%% currently fixed value */
  *dns2Addr= 0xd5a02822;             /* %%% currently fixed value */
  strcpy( domainName, "ggaweb.ch" ); /* %%% currently fixed value */
      
//printf( "inet/dns %08X %08X\n", *inetAddr,*dns_Addr );
  return 0;
} /* MyInetAddr */



OSStatus netReadBlock( ushort pid, net_typ* net, uint32_t *nBytes )
{
  OSStatus err;
  uint32_t arg;
  int      flags= 0;
  WSANETWORKEVENTS ev;

       err= WSAEnumNetworkEvents( net->ep, net->hEventObj, &ev );
  if (!err && (ev.lNetworkEvents & FD_CLOSE)!=0)
       net->closeIt= true; /* close event: close after last read */
                
       err= ioctl( net->ep, FIONREAD, &arg );
  if  (err>=0 && arg>0) 
       err= recv ( net->ep, net->transferBuffer, *nBytes, flags );
  if  (err>=0) { *nBytes= err; return 0; }
  
  *nBytes= 0;
  switch (err) {
    case WSAECONNABORTED: net->closeIt= true; return E_EOF;
    case WSAEWOULDBLOCK : return 0; /* this is not an error */
   
    case WSAECONNRESET  :
    case WSAENOTCONN    : send_signal( pid, S_HangUp ); return E_READ;
  } /* switch */

  return err;
} /* netReadBlock */



OSStatus netWriteBlock( ushort pid, net_typ* net, uint32_t *nBytes )
{
  OSStatus err;
  int      loopit= 10;
  int      flags =  0;

  while (true) {
    /*  <err> is the number of bytes written, if >=0 */
         err= send( net->ep, net->transferBuffer, *nBytes, flags );
    if  (err>=0) { *nBytes= err; return 0; }
    
         err= WSAGetLastError();
    if (!err) break; /* no error, no data */
    
    if (loopit-- <= 0) break;
    Sleep( 1 );
  } /* while */
                
  *nBytes= 0;
  switch (err) {
    case WSAECONNABORTED: net->closeIt= true; return E_EOF;
    case WSAEWOULDBLOCK : return 0; /* this is not an error */
   
    case WSAECONNRESET  :
    case WSAENOTCONN    : send_signal( pid, S_HangUp ); return E_WRITE;
  } /* switch */

  return err;
} /* netWriteBlock */



/* eof */

