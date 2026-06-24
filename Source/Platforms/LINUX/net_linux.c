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
 *    Revision 1.3  2005/06/30 10:35:21  bfo
 *    Extended for UNIX (Mach-O Support), unused vars eliminated,
 *    hostPtr==NULL support => "unknown"
 *
 *    Revision 1.2  2004/12/04 00:13:50  bfo
 *    <nBytes> adaption
 *
 *    Revision 1.1  2004/11/27 12:19:49  bfo
 *    "net_platform" introduced
 *
 *
 */

#include "os9exec_incl.h"
#include "net_platform.h"

/* specific network definitions */
#include <sys/utsname.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
  
#ifdef linux
  #include <asm/ioctls.h>
#endif
  
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
  
typedef struct sockaddr_in SOCKADDR_IN;



os9err NetInstall(void)
{
  if (netInstalled) return 0; /* already done, ok */
    
  // do nothing here
  netInstalled= true; /* it is ok now */
  return 0;
} /* NetInstall */



os9err MyInetAddr( uint32_t *inetAddr, uint32_t *dns1Addr,
                                    uint32_t *dns2Addr, char* domainName )
{
  struct utsname     sysname= { 0 };
  struct hostent     *hostPtr;  
  struct sockaddr_in serverName= { 0 };
      
  FILE*   stream;
  char*   p;
  char    c;
  int     cnt;
  Boolean isEOF;
  #define LINELEN 132
  char    buffer[ LINELEN ];
  char*   v;
  int     n= 1;
  char*   dom;
  char    hostName[ OS9PATHLEN ];

        
  OSStatus err= NetInstall(); if (err) return err;

  *inetAddr= 0; /* default values */
  *dns1Addr= 0;
  *dns2Addr= 0;
  strcpy( domainName, "" );

  err=                  uname( &sysname);
      hostPtr = gethostbyname(  sysname.nodename );
  if (hostPtr==NULL) return OS9_ENETDOWN;

  memset( &serverName, 0, sizeof(serverName));
  memcpy( &serverName.sin_addr, hostPtr->h_addr,
                                hostPtr->h_length );

  *inetAddr= os9_long( serverName.sin_addr.s_addr );

      stream= fopen( "/etc/resolv.conf","r" ); /* open "resolv.conv" file */
  if (stream!=NULL) {
            isEOF= false;
    while (!isEOF) {
      p  = buffer;
      cnt= 0;
          
      while( cnt<LINELEN ) {
        if ((c=fgetc(stream))==EOF) { isEOF= true; break; }
        if ( c=='\n' ) break; /* abort on CR */
                  
        *p++= c; /* save in the buffer */        
        cnt++;
      } /* while */

      *p++= '\0'; 
  //  if (*buffer!='#') printf( "'%s'\n", buffer );
              
      v= "search ";
      if (strstr( buffer,v )!=NULL) {
        v= buffer + strlen( v );
        strcpy( domainName, v );
      } /* if */
              
      v= "nameserver ";
      if (strstr( buffer,v )!=NULL) {
        v= buffer + strlen( v );
        if (n==1) *dns1Addr= os9_long( inet_addr( v ) );
        else      *dns2Addr= os9_long( inet_addr( v ) );
        n++;
      }
    } /* loop */
          
    fclose( stream );
  } /* if */

      hostPtr= gethostbyaddr( (void*)&serverName.sin_addr.s_addr,sizeof(uint32_t), AF_INET );
  if (hostPtr==NULL) strcpy( hostName, "unknown" );
  else               strcpy( hostName, hostPtr->h_name );
  
  dom= strstr( hostName,domainName );
  if (dom!=NULL && strlen(dom)==strlen(domainName)) {
      dom--; if (*dom=='.') *dom= '\0';
  } /* if */
  
// printf( "'%s'\n", hostName );
// printf( "i12d %08X %08X %08X '%s'\n", *inetAddr, *dns1Addr, *dns2Addr, domainName );
// printf( "inet/dns %08X %08X\n", *inetAddr,*dns_Addr );

  return 0;
} /* MyInetAddr */




OSStatus netReadBlock( _pid_, net_typ* net, uint32_t *nBytes )
// platform specific reading
{
  OSStatus err;
  int      flags= 0;

      err= recv( net->ep, net->transferBuffer, *nBytes, flags );
  if (err==0)    net->closeIt= true;
  if (err< 0) { *nBytes= 0; return err; }

  *nBytes= (uint32_t)err;
  return 0;
} /* netReadBlock */



OSStatus netWriteBlock( _pid_, net_typ* net, uint32_t *nBytes )
// platform specific reading
{
  OSStatus err;
  int flags= 0;

      err= send( net->ep, net->transferBuffer, *nBytes, flags );
  if (err==0)    net->closeIt= true;
  if (err< 0) { *nBytes= 0; return err; }

  *nBytes= (uint32_t)err;
  return 0;
} /* netWriteBlock */



/* eof */

