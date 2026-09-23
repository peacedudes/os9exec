/* spfsock.c -- socket paths ("/ip0#1/tcp0" and its siblings, and "/socket")
 * over host sockets
 *
 * The OS-9 networking programs on a licensed system disk (ping, telnet, ftp,
 * tcpsend ...) reach the network through a library that opens one of these
 * paths and then drives it with a single setstat code, carrying a parameter
 * block whose first longword selects the operation. os9exec had no such
 * device, so every one of those programs stopped at its socket call.
 *
 * WHAT THIS IS BUILT FROM. Everything here was learned by running those
 * programs under os9exec with a logging stand-in for the device and watching
 * what they ask for: the order of the calls, the block they pass, and the
 * values inside it. Nothing is transcribed from Microware's headers or
 * sources. The small operation numbers are the socket get/setstat codes that
 * OS-9 C headers in freely distributed archives have carried for decades, and
 * that os9exec's own os9funcs.h has shipped since v4.0.0.
 *
 * SCOPE. Client connections work end to end (TCP, and UDP as far as
 * connect/read/write take it), and so do servers: a program that binds,
 * listens and accepts receives its data, and telnetd and ftpd serve host
 * clients. Datagrams sent to an address given per call (ping's raw socket)
 * are not done yet. A few operations are answered E$UnkSvc on purpose,
 * because the callers check for exactly that and carry on.
 *
 * The connection a server accepts is held on the listening path until the
 * caller claims it; see the accept and attach cases below.
 *
 * THE OLDER LIBRARY. Programs built with the earlier socket library (ttcp,
 * the BIND tools, WN's inetd) open "/socket" instead, and make each socket
 * call a setstat of its own, with the arguments in registers. os9exec's
 * original authors served those calls in network.c until 2026, when it was
 * retired for this file; the same host sockets serve them now, through
 * pSisp at the end. The two front ends share every helper below, so a fix to
 * one is a fix to both.
 */

/* Taken before os9exec_incl.h, which undefines the POSIX socket error names
   so that os9errno.h can give them OS-9 values: past it, EINPROGRESS is not
   the host's number any more, and neither are the others classified here. */
#ifndef _WIN32
  #include <errno.h>
  static const int hostEINPROGRESS= EINPROGRESS;

  /* A host socket error, by what it means. The older library's callers are
     told what went wrong in OS-9's own socket error numbers (SpfIspErr), and
     this is the half of that translation that needs the host's names. */
  typedef enum { hsOther, hsConnRefused, hsAddrInUse, hsAddrNotAvail,
                 hsNetUnreach, hsTimedOut, hsConnReset, hsConnAborted,
                 hsNotConn, hsIsConn, hsAfNoSupport, hsProtoNoSupport,
                 hsNoProtoOpt, hsMsgSize, hsNoBufs, hsOpNotSupp,
                 hsDestAddrReq, hsAccess, hsInval, hsPipe } hostSockErr;

  static hostSockErr HostSockErr( int e )
  {
      switch (e) {
          case ECONNREFUSED:    return hsConnRefused;
          case EADDRINUSE:      return hsAddrInUse;
          case EADDRNOTAVAIL:   return hsAddrNotAvail;
          case ENETUNREACH:
          case EHOSTUNREACH:    return hsNetUnreach;
          case ETIMEDOUT:       return hsTimedOut;
          case ECONNRESET:      return hsConnReset;
          case ECONNABORTED:    return hsConnAborted;
          case ENOTCONN:        return hsNotConn;
          case EISCONN:         return hsIsConn;
          case EAFNOSUPPORT:    return hsAfNoSupport;
          case EPROTONOSUPPORT: return hsProtoNoSupport;
          case ENOPROTOOPT:     return hsNoProtoOpt;
          case EMSGSIZE:        return hsMsgSize;
          case ENOBUFS:         return hsNoBufs;
          case EOPNOTSUPP:      return hsOpNotSupp;
          case EDESTADDRREQ:    return hsDestAddrReq;
          case EACCES:
          case EPERM:           return hsAccess;
          case EINVAL:          return hsInval;
          case EPIPE:           return hsPipe;
          default:              return hsOther;
      }
  } /* HostSockErr */
#endif

#include "os9exec_incl.h"

#if defined UNIX && !defined MINGW
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>
  #include <sys/ioctl.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>
  #include <poll.h>
#endif

/* the operations seen in the first longword of the block */
#define SPFOP_SOCKET  0x01060002   /* make a socket: domain, type, protocol */
#define SPFOP_ACCEPT  0x01060080   /* take a connection: answers {domain,type,proto} */
#define SPFOP_BIND    0x6C
#define SPFOP_LISTEN  0x6D
#define SPFOP_CONNECT 0x6E
#define SPFOP_SOPT    0x74
#define SPFOP_NAME    0x922        /* observed: describe this socket */
#define SPFOP_NEWID   0x66         /* observed: the caller asks the connection's
                                      path to name itself ... */
#define SPFOP_ATTACH  0x70         /* ... and gives that name to the listening
                                      path, which is SS_Accept's own code */
#define SPFOP_EVENT   0x1004       /* observed: the event this path posts */
#define SPFOP_MYNAME  0x00FF0006   /* observed: this end's address ... */
#define SPFOP_PEER    0x00FF0007   /* ... and the far end's */
#define SPFOP_SENDTO  0x77         /* SS_SendTo: a datagram to an address given with it */
#define SPFOP_RECVFR  0x78         /* SS_RecvFr: a datagram, and whom it came from */

/* A socket address as a guest holds one: the BSD layout, a 16-bit family,
   then port and address in network order, then padding. */
#define GUEST_SOCKADDR 16
#define GUEST_AF_INET   2

/* BSD message flags, as a guest passes them to send and receive */
#define GUEST_MSG_OOB       0x1
#define GUEST_MSG_PEEK      0x2
#define GUEST_MSG_DONTROUTE 0x4


#if defined UNIX && !defined MINGW

static int SpfFd( syspath_typ* spP )
/* the host socket of this path, or -1 while it has none */
{   return spP->u.spf.fdPlus1-1;
} /* SpfFd */


/* A write to a connection the far end has dropped raises SIGPIPE, and its
   default action ends the process -- here the whole emulator, every OS-9
   process with it (measured: exit 141 from one tcpsend). The guest is owed a
   write error instead, so no socket of ours may raise it: macOS takes that
   per socket, Linux per send. */
#ifdef MSG_NOSIGNAL
  #define SPF_SENDFLAGS MSG_NOSIGNAL
#else
  #define SPF_SENDFLAGS 0
#endif

static void SpfNoSigpipe( int fd )
{
  #ifdef SO_NOSIGPIPE
    int on= 1;
    setsockopt( fd, SOL_SOCKET, SO_NOSIGPIPE, &on,sizeof(on) );
  #else
    (void)fd;
  #endif
} /* SpfNoSigpipe */


static void SpfNonBlocking( int fd )
/* reads, writes and accepts must never block the emulator */
{
    int flags= fcntl( fd, F_GETFL, 0 );
    if (flags>=0) fcntl( fd, F_SETFL, flags | O_NONBLOCK );
} /* SpfNonBlocking */


static os9err SpfPark( ushort pid )
/* Nothing to read yet: park the caller the way a console read does, and let
   the dispatcher run the same call again later (procstuff.c retries a
   pWaitRead process every NewAge arbitration rounds). The emulator itself
   must never block on a socket: other processes keep running. */
{
    process_typ* cp= &procs[pid];

    if (cp->state!=pWaitRead) cp->saved_state= cp->state;
    set_os9_state( pid, pWaitRead, "SPF read" );
    arbitrate= true;
    return 0;
} /* SpfPark */


static void SpfResume( ushort pid )
/* back to what it was before it parked, so a satisfied read returns */
{
    process_typ* cp= &procs[pid];

    if (cp->state==pWaitRead) set_os9_state( pid, cp->saved_state, "SPF read resume" );
} /* SpfResume */


static void SpfHostAddr( const byte* guest, struct sockaddr_in* sa )
/* <guest>, a socket address in the guest's layout, as the host's. The family
   is not read: every caller has decided it is AF_INET before this. */
{
    memset( sa,0,sizeof(*sa) );
    #ifdef __APPLE__
      sa->sin_len= sizeof(*sa);
    #endif
    sa->sin_family= AF_INET;
    memcpy( &sa->sin_port,        guest+2, 2 ); /* both already in network order */
    memcpy( &sa->sin_addr.s_addr, guest+4, 4 );
} /* SpfHostAddr */


static uint32_t SpfGuestAddr( const struct sockaddr_in* sa, byte* guest, uint32_t room )
/* <sa> written to <guest> in the guest's layout, no more than <room> bytes of
   it; answers how many were written. */
{
    byte ga[GUEST_SOCKADDR];

    if (room>GUEST_SOCKADDR) room= GUEST_SOCKADDR;
    memset   ( ga,0,sizeof(ga) );
    os9_set_w( ga, GUEST_AF_INET );
    memcpy   ( ga+2, &sa->sin_port, 2 );                    /* network order */
    memcpy   ( ga+4, &sa->sin_addr.s_addr, 4 );
    memcpy   ( guest, ga, room );
    return room;
} /* SpfGuestAddr */


static int SpfMake( syspath_typ* spP, uint32_t type )
/* Give <spP> a new host socket of BSD socket <type>, replacing any it had.
   Answers 0, or the host's errno. */
{
    int fd;

    if (SpfFd( spP )>=0) close( SpfFd( spP ) );
    spP->u.spf.fdPlus1= 0;

    /* A raw socket (type 3, what ping opens) becomes the host's ICMP
       datagram socket, which an ordinary user may open where a raw one
       needs the super user; it hands back replies IP header first, as
       a raw socket does. Failing that, a real raw socket. */
    spP->u.spf.bareIcmp= false;
    if (type==3) {
            fd= socket( AF_INET, SOCK_DGRAM, IPPROTO_ICMP );
        #ifdef linux
          spP->u.spf.bareIcmp= fd>=0; /* see SpfRecvFrom */
        #endif
        if (fd<0) fd= socket( AF_INET, SOCK_RAW, IPPROTO_ICMP );
    }
    else    fd= socket( AF_INET, type==2 ? SOCK_DGRAM : SOCK_STREAM, 0 );
    if (fd<0) return errno;
    SpfNoSigpipe  ( fd );
    SpfNonBlocking( fd );

    spP->u.spf.fdPlus1   = fd+1;
    spP->u.spf.proto     = (ushort)type;
    spP->u.spf.connected = false;
    spP->u.spf.connecting= false;
    debugprintf( dbgSpecialIO,dbgNorm,("# SPF: socket type=%u -> host fd %d\n",
                                         (uint32_t)type, fd ));
    return 0;
} /* SpfMake */


typedef enum { stepDone, stepWait, stepFailed } spfstep;

static spfstep SpfConnect( syspath_typ* spP, const byte* to, int* herr )
/* Connect <spP> to the guest address <to>, or, while a connect is already
   under way, ask whether it has finished (<to> is not read then).

   Never a blocking connect: it would hold every OS-9 process for as long as
   the handshake takes, and the system tick's signal cuts it short anyway
   (connect is not restarted, SA_RESTART or not), which failed a slow
   connection at once. So it is started and the caller parked, as a read with
   nothing to read is; the same call comes round again, and then asks whether
   it is done. */
{
    struct sockaddr_in sa;
    struct pollfd      pf;
    int                fd= SpfFd( spP ), r, soerr= 0;
    socklen_t          elen= sizeof(soerr);

    *herr= 0;
    if (spP->u.spf.connecting) {
        /* Only an event says the handshake is over. poll() can also
           return -1 when the tick's signal lands in it, and SO_ERROR
           reads 0 on a connection still under way -- taken together
           those declared a pending connect made (then its first write
           failed), about one time in four under the tick. */
        pf.fd= fd; pf.events= POLLOUT; pf.revents= 0;
        if (poll( &pf,1, 0 )<=0 || pf.revents==0) return stepWait; /* not yet */
        spP->u.spf.connecting= false;
        if (getsockopt( fd, SOL_SOCKET, SO_ERROR, &soerr,&elen )!=0) soerr= errno;
        if (soerr!=0) {
            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: connect failed, %d\n", soerr ));
            *herr= soerr;
            return stepFailed;
        }
        debugprintf( dbgSpecialIO,dbgNorm,("# SPF: connect -> done\n" ));
        spP->u.spf.connected= true;
        return stepDone;
    }

    SpfHostAddr( to, &sa );
        r= connect( fd, (struct sockaddr*)&sa, sizeof(sa) ); /* fd is non-blocking */
    debugprintf( dbgSpecialIO,dbgNorm,("# SPF: connect -> %d\n", r ));
    if (r==0) { spP->u.spf.connected= true; return stepDone; }
    if (errno!=hostEINPROGRESS && errno!=EINTR) { *herr= errno; return stepFailed; }
    spP->u.spf.connecting= true;
    return stepWait;
} /* SpfConnect */


static int SpfBind( syspath_typ* spP, const byte* at )
/* Bind <spP> to the guest address <at>. Answers 0, or the host's errno. */
{
    struct sockaddr_in sa;
    int                fd= SpfFd( spP ), on= 1, e;

    SpfHostAddr( at, &sa );

    /* A server that has just exited leaves its port in TIME_WAIT, and
       without this the next run cannot bind it for a minute or more.
       The guest asks for its own options through the option call,
       which we accept without applying -- so if this were left to the
       guest no server could be restarted. */
    setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &on,sizeof(on) );

    debugprintf( dbgSpecialIO,dbgNorm,("# SPF: bind port %u (host fd %d)\n",
                                         (uint32_t)ntohs( sa.sin_port ), fd ));
    if (bind( fd, (struct sockaddr*)&sa, sizeof(sa) )==0) return 0;
    e= errno;

    /* A port below 1024 on one address is the super user's on macOS,
       while the same port on every address is not. OS-9 grants it
       (ftpd binds its data connection to 127.0.0.1 port 20), so try
       the port the program asked for on every address before
       refusing. */
    if (e==EACCES && ntohs( sa.sin_port )<1024 && sa.sin_addr.s_addr!=htonl( INADDR_ANY )) {
        sa.sin_addr.s_addr= htonl( INADDR_ANY );
        if (bind( fd, (struct sockaddr*)&sa, sizeof(sa) )==0) return 0;
        e= errno;
    }
    debugprintf( dbgSpecialIO,dbgNorm,("# SPF: bind port %u refused, errno %d\n",
                                         (uint32_t)ntohs( sa.sin_port ), e ));
    return e;
} /* SpfBind */


static int SpfListen( syspath_typ* spP, int backlog )
/* Answers 0, or the host's errno. A backlog of 0 or less means the default. */
{
    if (backlog<=0) backlog= 5;
    debugprintf( dbgSpecialIO,dbgNorm,("# SPF: listen backlog %d\n", backlog ));
    return listen( SpfFd( spP ), backlog )==0 ? 0 : errno;
} /* SpfListen */


static int SpfTake( syspath_typ* spP, struct sockaddr_in* peer, int* herr )
/* The next connection waiting on listening path <spP>, as a host socket set
   up the way every socket here is, or -1 with the host's errno in <*herr>
   (EAGAIN while there is none yet). */
{
    socklen_t plen= sizeof(*peer);
    int       nfd;

    memset( peer,0,sizeof(*peer) );
        nfd= accept( SpfFd( spP ), (struct sockaddr*)peer, &plen );
    if (nfd<0) { *herr= errno; return -1; }
    SpfNonBlocking( nfd );
    SpfNoSigpipe  ( nfd );
    *herr= 0;
    return nfd;
} /* SpfTake */


static os9err SpfSend( ushort pid, syspath_typ* spP, uint32_t* lenP, const byte* buffer,
                       int flags, int* herr )
/* Send <*lenP> bytes, all of them, parking whenever the host cannot take more
   yet; <*lenP> comes back as the count sent. <*herr> has the host's errno when
   the connection failed. */
{
    /* Resuming a write that parked: those bytes are already on the wire.
       The dispatcher re-runs the whole call, same buffer and same length, so
       picking up at the recorded offset is what stops them going twice. */
    uint32_t done= spP->u.spf.writeDone;
    int      fd  = SpfFd( spP );

    *herr= 0;
    /* A different write starts over. Only the parked call itself -- same
       process, same buffer -- resumes: a write abandoned by a signal, or one
       from another process sharing the path, left its offset behind, and the
       next write skipped that many bytes and reported them sent. */
    if (done>*lenP || pid!=spP->u.spf.writePid ||
        (const void*)buffer!=spP->u.spf.writeBuf) done= 0;

    while (done<*lenP) {
        ssize_t n= send( fd, buffer+done, *lenP-done, flags | SPF_SENDFLAGS );
        if (n>0) { done+= (uint32_t)n; continue; }

        /* EINTR put nothing on the wire, so an immediate retry duplicates
           nothing. The 100Hz tick lands here often enough to matter. */
        if (n<0 && errno==EINTR) continue;

        /* EAGAIN is the host's send buffer full with the peer not draining
           it. Every socket here is O_NONBLOCK (reads must never block the
           emulator), so this is reachable any time a reader is
           slower than a writer -- and retrying in place spun inside the
           syscall with the WHOLE emulator stopped: no other process ran, no
           alarm came due, and a reader that stopped for good hung everything.
           consio.c made this exact mistake for the console and fixed it this
           way. Park instead, remembering how far we got, and let the
           dispatcher bring the call back. The guest still sees one write that
           completes in full, which is what a socket on real OS-9 gives it. */
        if (n<0 && errno==EAGAIN) {
            spP->u.spf.writeDone= done;
            spP->u.spf.writePid = pid;
            spP->u.spf.writeBuf = (void*)buffer;
            *lenP= 0;
            return SpfPark( pid );
        }
        *herr= n<0 ? errno : EPIPE;
        break;                                  /* the connection is gone */
    }

    spP->u.spf.writeDone= 0;
    if (done==0 && *lenP>0) { *lenP= 0; return os9error(E_WRITE); }
    *lenP= done;
    return 0;
} /* SpfSend */


static ssize_t SpfSendTo( syspath_typ* spP, const byte* buf, uint32_t len, int flags,
                          const byte* to )
/* One datagram to the guest address <to>. Answers the count sent, or -1 with
   the host's errno. */
{
    struct sockaddr_in sa;

    SpfHostAddr( to, &sa );
    if (spP->u.spf.bareIcmp && len>=8) memcpy( spP->u.spf.echoId, buf+4, 2 );
    return sendto( SpfFd( spP ), buf,len, flags | SPF_SENDFLAGS, (struct sockaddr*)&sa, sizeof(sa) );
} /* SpfSendTo */


static ssize_t SpfRecvFrom( syspath_typ* spP, byte* buf, uint32_t size, int flags,
                            struct sockaddr_in* from )
/* One datagram into <buf>, and whom it came from. Answers its length, or -1
   with the host's errno (EAGAIN while none has come); EINVAL when <size>
   cannot hold even the header a raw socket's reader expects. */
{
    socklen_t flen= sizeof(*from);
    ssize_t   n;

    /* Linux's ICMP datagram socket gives the reply without the IP
       header a raw socket puts first, and with the socket's own echo
       identifier in place of the one the guest sent. The guest reads
       it as raw, so it gets a header and its identifier back. */
    uint32_t hdr= spP->u.spf.bareIcmp ? 20:0;
    if (size<=hdr) { errno= EINVAL; return -1; }

    memset( from,0,sizeof(*from) );
        n= recvfrom( SpfFd( spP ), buf+hdr,size-hdr, flags, (struct sockaddr*)from, &flen );
    if (n<0) return n;
    if (hdr>0) {
        memset   ( buf,0,hdr );
        buf[0]= 0x45;                         /* IPv4, five words */
        os9_set_w( buf+2, (uint16_t)(n+hdr) );
        buf[8]= 64;                           /* time to live */
        buf[9]= IPPROTO_ICMP;
        memcpy   ( buf+12, &from->sin_addr.s_addr, 4 );
        memcpy   ( buf+16, &from->sin_addr.s_addr, 4 );
        if (n>=8) memcpy( buf+hdr+4, spP->u.spf.echoId, 2 );
        n+= hdr;
    }
    return n;
} /* SpfRecvFrom */


static Boolean SpfOptName( uint32_t level, uint32_t name, int* hl, int* hn )
/* A socket option by its BSD numbers, which the host may number differently
   (Linux does), as the host's own. False for one this does not know. */
{
    *hl= -1; *hn= -1;
    if (level==0xFFFF) {                 /* SOL_SOCKET */
        *hl= SOL_SOCKET;
        switch (name) {
            case 0x0004: *hn= SO_REUSEADDR; break;
            #ifdef SO_REUSEPORT
            case 0x0200: *hn= SO_REUSEPORT; break;
            #endif
            case 0x0020: *hn= SO_BROADCAST; break;
            case 0x0008: *hn= SO_KEEPALIVE; break;
            case 0x1001: *hn= SO_SNDBUF;    break;
            case 0x1002: *hn= SO_RCVBUF;    break;
            case 0x1007: *hn= SO_ERROR;     break;
            case 0x1008: *hn= SO_TYPE;      break;
        }
    }
    else if (level==0) {                 /* IPPROTO_IP */
        *hl= IPPROTO_IP;
        switch (name) {
            case  9: *hn= IP_MULTICAST_IF;    break;
            case 10: *hn= IP_MULTICAST_TTL;   break;
            case 11: *hn= IP_MULTICAST_LOOP;  break;
            case 12: *hn= IP_ADD_MEMBERSHIP;  break;
            case 13: *hn= IP_DROP_MEMBERSHIP; break;
        }
    }
    else if (level==6) {                 /* IPPROTO_TCP */
        *hl= IPPROTO_TCP;
        if (name==1) *hn= TCP_NODELAY;
    }
    return *hn>=0;
} /* SpfOptName */


static int SpfSetOpt( syspath_typ* spP, uint32_t level, uint32_t name,
                      const byte* val, uint32_t olen, Boolean* known )
/* Apply a socket option given by its BSD numbers. Answers 0, or the host's
   errno; <*known> is false for an option this does not know, which is left
   unapplied. */
{
    int hl, hn, r;

    *known= SpfOptName( level, name, &hl, &hn );
    if (!*known) return 0;

    if (olen==4 && hl==IPPROTO_IP && (hn==IP_MULTICAST_TTL || hn==IP_MULTICAST_LOOP)) {
        /* given as an int, big-endian in the guest; the host takes these one
           as a byte (macOS insists), so the value goes over, not the bytes */
        unsigned char cv= (unsigned char)os9_get_l( val );
        r= setsockopt( SpfFd( spP ), hl, hn, &cv, sizeof(cv) );
    }
    else if (olen==4 && hl!=IPPROTO_IP) {        /* an int, big-endian in the guest */
        int iv= (int)os9_get_l( val );
        r= setsockopt( SpfFd( spP ), hl, hn, &iv, sizeof(iv) );
    }
    else if (olen==1) {                          /* TTL, loop: one byte */
        unsigned char cv= val[0];
        r= setsockopt( SpfFd( spP ), hl, hn, &cv, sizeof(cv) );
    }
    else {                                       /* addresses: network order already */
        r= setsockopt( SpfFd( spP ), hl, hn, val, (socklen_t)olen );
    }
    return r==0 ? 0 : errno;
} /* SpfSetOpt */

#endif


static os9err pSopen( _pid_, syspath_typ* spP, _modeP_, const char* pathname )
/* open a socket path. The host socket is not made here: the caller asks for
   it with the socket operation, which carries the domain and type. */
{
    const char* p= pathname;

  #if defined __EMSCRIPTEN__
    /* A browser page cannot open a TCP connection at all, so there is no
       network behind these paths there. Refuse the open as a device that is
       not present, which is what the callers already handle, rather than let
       them ask a socket that can never answer. */
    (void)p;
    return os9error(E_UNIT);
  #endif

    spP->u.spf.fdPlus1    = 0;
    spP->u.spf.acceptPlus1= 0;
    spP->u.spf.proto      = 0;
    spP->u.spf.connected  = false;
    spP->u.spf.connecting = false;
    spP->u.spf.bareIcmp   = false;
    spP->u.spf.writeDone  = 0;
    spP->u.spf.writePid   = 0;
    spP->u.spf.writeBuf   = NULL;
    spP->u.spf.isp        = ustrcmp( pathname,"/socket" )==0;

    while (*p!=NUL) p++;                    /* the protocol is the last element */
    while (p>pathname && *(p-1)!='/') p--;
    strncpy( spP->name, p, OS9NAMELEN-1 );
    spP->name[ OS9NAMELEN-1 ]= NUL;
    return 0;
} /* pSopen */


static os9err pSclose( _pid_, syspath_typ* spP )
{
  #if defined UNIX && !defined MINGW
    if (SpfFd( spP )>=0) close( SpfFd( spP ) );
    if (spP->u.spf.acceptPlus1>0) close( spP->u.spf.acceptPlus1-1 );
  #endif
    spP->u.spf.fdPlus1    = 0;
    spP->u.spf.acceptPlus1= 0;
    spP->u.spf.writeDone  = 0;
    return 0;
} /* pSclose */


static os9err pSread( ushort pid, syspath_typ* spP, uint32_t* lenP, char* buffer )
{
  #if defined UNIX && !defined MINGW
    ssize_t n;
    int     fd= SpfFd( spP );

    SpfResume( pid );
    if (fd<0) return os9error(E_NOTRDY);
    if (*lenP==0) return 0;

        n= recv( fd, buffer,*lenP, 0 );
    if (n>0) { *lenP= (uint32_t)n; return 0; }
    if (n==0) { *lenP= 0; return os9error(E_EOF); } /* the other end closed */

    /* EAGAIN alone: os9exec_incl.h undefines the POSIX EWOULDBLOCK so that
       os9errno.h can give the name an OS-9 value, and on every host we build
       for the two are the same number anyway (hostterm.c tests EAGAIN too). */
    if (errno==EAGAIN || errno==EINTR) {
        *lenP= 0;
        return SpfPark( pid );
    }
    *lenP= 0;
    return os9error(E_READ);
  #else
    (void)pid; (void)spP; (void)buffer; *lenP= 0;
    return os9error(E_UNKSVC);
  #endif
} /* pSread */


static os9err pSwrite( ushort pid, syspath_typ* spP, uint32_t* lenP, char* buffer )
{
  #if defined UNIX && !defined MINGW
    int herr;

    SpfResume( pid );
    if (SpfFd( spP )<0) { spP->u.spf.writeDone= 0; return os9error(E_NOTRDY); }
    return SpfSend( pid, spP, lenP, (const byte*)buffer, 0, &herr );
  #else
    (void)pid; (void)spP; (void)buffer; *lenP= 0;
    return os9error(E_UNKSVC);
  #endif
} /* pSwrite */


static os9err pSready( _pid_, syspath_typ* spP, uint32_t* n )
/* how many bytes can be read right now */
{
  #if defined UNIX && !defined MINGW
    int cnt= 0, fd= SpfFd( spP );

    *n= 0;
    if (fd<0) return os9error(E_NOTRDY);
    if (ioctl( fd, FIONREAD, &cnt )!=0) return os9error(E_NOTRDY);
    *n= (uint32_t)cnt;
    if (cnt>0) return 0;

    /* Nothing pending, but readable all the same: the far end has closed.
       Say one byte is ready, as a broken pipe does (pPready), so the caller
       reads and meets the end of file. Answering "not ready" left a telnet
       client waiting forever on a connection the server had already closed. */
    {   struct pollfd pf;
        pf.fd= fd; pf.events= POLLIN; pf.revents= 0;
        if (spP->u.spf.connected && poll( &pf,1, 0 )>0 && (pf.revents & (POLLIN|POLLHUP))) {
            *n= 1; return 0;
        }
    }
    return os9error(E_NOTRDY);
  #else
    (void)spP; *n= 0;
    return os9error(E_UNKSVC);
  #endif
} /* pSready */


/* The option section of a socket path. The caller asks the LISTENING path for
   this straight after it has opened a path for the connection, and gave up
   with "can't accept" while it answered E$UnkSvc. The rest are the values the
   ISP manager has always returned for a socket (network.c, netstdopts), but
   not the device type: ftpdc refuses to run ("must be forked from 'ftpd'")
   unless the path it was handed says 15 there, where ISP's said 7. */
static const byte spfstdopts[OPTSECTSIZE]=
                { 15,       /* PD_DTP, as ftpdc checks it */
                  0,
                  0x01,
                  0,
                  0,
                  0x10 };   /* the rest is zero */

static os9err pSopt( _pid_, _spP_, byte* buffer )
{   memcpy( buffer, spfstdopts, OPTSECTSIZE ); return 0;
} /* pSopt */


static os9err pSnam( _pid_, syspath_typ* spP, char* volname )
{   strcpy( volname, spP->name ); return 0;
} /* pSnam */


static os9err pSspf( ushort pid, syspath_typ* spP, uint32_t* d1, byte* blk )
/* The one call the socket library makes: an operation, a byte count, and a
   pointer to the arguments (all observed, see the top of this file). */
{
  #if defined UNIX && !defined MINGW
    uint32_t op, len, ptr;
    byte*    args;
    int      herr;

    (void)d1;
    if (blk==NULL || !RANGE_IN_ARENA( blk,12 )) return os9error(E_BPADDR);
    op  = os9_get_l( blk   );

    /* This operation's block is laid out apart from the rest -- the words
       the others read below are left unset -- so it is taken first. The event
       id goes where an event waiter already looks for one (set_evId, as
       SS_SEvent keeps it on a tty), and the waiter polls this path's SS_Ready;
       zero clears it. */
    if (op==SPFOP_EVENT) {
        byte* ev;

        if (!RANGE_IN_ARENA( blk,24 )) return os9error(E_BPADDR);
            ev= (byte*)FROM68K( os9_get_l( blk+20 ) );
        if (!RANGE_IN_ARENA( ev,32 ))  return os9error(E_BPADDR);
        spP->set_evId= os9_get_l( ev+24 );
        debugprintf( dbgSpecialIO,dbgNorm,("# SPF: data-ready event -> %X\n", spP->set_evId ));
        return 0;
    }

    len = os9_get_l( blk+4 );
    ptr = os9_get_l( blk+8 );
    args= (byte*)FROM68K( ptr );

    /* ATTACH's operand is a value, not an address: it must not be range
       checked as one. */
    if (op!=SPFOP_ATTACH &&
        ptr!=0 && !RANGE_IN_ARENA( args,len>64 ? 64:len )) return os9error(E_BPADDR);

    switch (op) {
        case SPFOP_SOCKET:
            if (ptr==0 || len<12) return os9error(E_PARAM);
            /* the BSD socket type is the middle one of the three */
            return SpfMake( spP, os9_get_l( args+4 ) )==0 ? 0 : os9error(E_NOTRDY);

        case SPFOP_CONNECT:
            SpfResume( pid );   /* this call may be a retry of a parked connect */
            if (SpfFd( spP )<0) return os9error(E_NOTRDY);
            if (!spP->u.spf.connecting && (ptr==0 || len<8)) return os9error(E_PARAM);
            switch (SpfConnect( spP, args, &herr )) {
                case stepDone: return 0;
                case stepWait: return SpfPark( pid );
                default:       return os9error(E_NOTRDY);
            }

        case SPFOP_BIND:
            if (SpfFd( spP )<0) return os9error(E_NOTRDY);
            if (ptr==0 || len<8) return os9error(E_PARAM);
            /* the same BSD socket address connect is given, observed */
            return SpfBind( spP, args )==0 ? 0 : os9error(E_SHARE);

        case SPFOP_LISTEN: {
            int backlog= 5;

            if (SpfFd( spP )<0) return os9error(E_NOTRDY);
            if (ptr!=0 && len>=4) backlog= (int)os9_get_l( args );
            return SpfListen( spP, backlog )==0 ? 0 : os9error(E_NOTRDY);
        }

        case SPFOP_ACCEPT: {
            struct sockaddr_in peer;
            int                nfd;

            SpfResume( pid );   /* this call may be a retry of a parked accept */
            if (SpfFd( spP )<0) return os9error(E_NOTRDY);
            if (ptr==0 || len<12) return os9error(E_PARAM);

                nfd= SpfTake( spP, &peer, &herr );
            if (nfd<0) {
                if (herr==EAGAIN || herr==EINTR) return SpfPark( pid );
                return os9error(E_NOTRDY);
            }

            /* The caller does NOT want a path number here. It wants the socket
               to make one with: it feeds these three longwords straight to its
               own socket() (domain, type, protocol), which opens a fresh
               "/ip0#1/tcp0" -- and then tells us, through NEWID and ATTACH
               below, which path the connection should end up on. Anything it
               does not recognise here becomes its own error $070C. */
            os9_set_l( args+0, 2 );                       /* AF_INET */
            os9_set_l( args+4, spP->u.spf.proto==0 ? 1 : spP->u.spf.proto );
            os9_set_l( args+8, 0 );

            /* Hold the connection on this path. Dropping it here would close
               it on the client the instant it connects; the call that moves it
               onto the caller's own path is not decoded yet, so it lives here
               until this path closes. */
            if (spP->u.spf.acceptPlus1>0) close( spP->u.spf.acceptPlus1-1 );
            spP->u.spf.acceptPlus1= nfd+1;

            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: accept from port %u -> host fd %d, pending\n",
                                                 (uint32_t)ntohs( peer.sin_port ), nfd ));
            return 0;
        }

        case SPFOP_NEWID:
            /* The value is only ever handed straight back to us, so the
               syspath number serves: unique, stable, resolves in one step.
               Nothing outside this file depends on the choice. */
            if (ptr==0 || len<4) return os9error(E_PARAM);
            os9_set_l( args, spP->nr );
            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: newid -> sp=%d\n", spP->nr ));
            return 0;

        case SPFOP_ATTACH: {
            /* Moves the accepted connection onto the path the caller opened
               for it. The operand arrives by value, not as an address, which
               is why nothing is dereferenced here. */
            syspath_typ* nsp;
            uint32_t     id= ptr;

            if (spP->u.spf.acceptPlus1<=0) return os9error(E_NOTRDY);
            if (id==0 || id>=MAXSYSPATHS)  return os9error(E_BPNUM);
            if (id==spP->nr)               return os9error(E_BPNUM); /* would close
                                                        its own listening socket */
            nsp= &syspaths[ id ];
            if (nsp->type!=fSPF)           return os9error(E_BPNUM);

            if (SpfFd( nsp )>=0) close( SpfFd( nsp ) ); /* its own socket is unused */
            nsp->u.spf.fdPlus1    = spP->u.spf.acceptPlus1;
            nsp->u.spf.proto      = spP->u.spf.proto;
            nsp->u.spf.connected  = true;
            nsp->u.spf.connecting = false;              /* a fresh connection */
            nsp->u.spf.writeDone  = 0;
            spP->u.spf.acceptPlus1= 0;                  /* handed over */

            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: attach connection -> sp=%d\n", id ));
            return 0;
        }

        case SPFOP_NAME: {
            /* The caller inspects what we hand back and refuses the
               connection unless it suits. These are the values it accepts,
               arrived at by running it and watching which ones it rejected;
               nothing here is transcribed from a Microware header. */
            struct sockaddr_in who;
            socklen_t          wlen= sizeof(who);
            int  fd= SpfFd( spP );

            if (fd<0) return os9error(E_NOTRDY);
            if (ptr==0 || !RANGE_IN_ARENA( args,56 )) return os9error(E_BPADDR);

            /* the far end if there is one, this end otherwise: the caller
               prints this as "connected to <address> port <n>" */
            memset( &who,0,sizeof(who) );
            if (getpeername( fd, (struct sockaddr*)&who, &wlen )!=0) {
                wlen= sizeof(who);
                memset( &who,0,sizeof(who) );
                getsockname( fd, (struct sockaddr*)&who, &wlen );
            }

            memset( args,0,56 );
            os9_set_w( args+4,  8 );          /* the family it accepts */
            memcpy   ( args+6,  &who.sin_port, 2 );        /* network order */
            memcpy   ( args+8,  &who.sin_addr.s_addr, 4 );
            args[48]= 3;                      /* the type it accepts */
            args[51]= 4;                      /* four bytes of address follow */
            memcpy   ( args+52, &who.sin_addr.s_addr, 4 );

            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: name -> port %u\n",
                                                 (uint32_t)ntohs( who.sin_port ) ));
            return 0;
        }

        case SPFOP_MYNAME:
        case SPFOP_PEER: {
            /* A socket address comes back the way connect and bind are given
               one, and its length through the pointer the caller passed. A
               host that cannot answer gets E$UnkSvc, as before this existed:
               the callers seen then ask the NAME operation instead. */
            struct sockaddr_in who;
            socklen_t          wlen= sizeof(who);
            byte*              lenP= (byte*)FROM68K( len );
            uint32_t           room;
            int  fd= SpfFd( spP ), r;

            if (fd<0 || ptr==0 || len==0)         return os9error(E_UNKSVC);
            if (!RANGE_IN_ARENA( lenP,4 ))        return os9error(E_BPADDR);
            memset( &who,0,sizeof(who) );
            r= op==SPFOP_PEER ? getpeername( fd, (struct sockaddr*)&who, &wlen )
                              : getsockname( fd, (struct sockaddr*)&who, &wlen );
            if (r!=0 || who.sin_family!=AF_INET) return os9error(E_UNKSVC);

            room= os9_get_l( lenP ); if (room>GUEST_SOCKADDR) room= GUEST_SOCKADDR;
            if (!RANGE_IN_ARENA( args,room ))    return os9error(E_BPADDR);
            os9_set_l( lenP, SpfGuestAddr( &who, args, room ) );
            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: %s name -> port %u\n",
                         op==SPFOP_PEER ? "peer":"own", (uint32_t)ntohs( who.sin_port ) ));
            return 0;
        }

        case SPFOP_SENDTO: {
            /* The data is where the other operations keep their arguments;
               the address it goes to is in a record the block points to --
               its length in the fourth byte, then the address laid out the
               way connect is given one. The count sent goes back where the
               caller gave its own. */
            byte*    rec;
            ssize_t  n;

            if (SpfFd( spP )<0) return os9error(E_NOTRDY);
            if (!RANGE_IN_ARENA( blk,32 )) return os9error(E_BPADDR);
                rec= (byte*)FROM68K( os9_get_l( blk+28 ) );
            if (!RANGE_IN_ARENA( rec,20 ) || rec[3]<8) return os9error(E_PARAM);
            if (ptr==0 || !RANGE_IN_ARENA( args,len )) return os9error(E_BPADDR);

                n= SpfSendTo( spP, args,len, 0, rec+4 );
            if (n<0) return os9error(E_WRITE);
            os9_set_l( blk+4, (uint32_t)n );
            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: sendto %d bytes\n", (int)n ));
            return 0;
        }

        case SPFOP_RECVFR: {
            /* The arguments describe the read: the buffer at +16, its size at
               +20 (the count comes back there), and at +28/+32 a length and a
               buffer for the sender's address, which comes back the way the
               name operations give one. Nothing yet: park, as a read does. */
            struct sockaddr_in from;
            byte              *buf, *fromP, *flenP;
            uint32_t           size, room;
            ssize_t            n;

            SpfResume( pid );
            if (SpfFd( spP )<0) return os9error(E_NOTRDY);
            if (ptr==0 || !RANGE_IN_ARENA( args,40 )) return os9error(E_BPADDR);
            buf  = (byte*)FROM68K( os9_get_l( args+16 ) );
            size =                 os9_get_l( args+20 );
            flenP= (byte*)FROM68K( os9_get_l( args+28 ) );
            fromP= (byte*)FROM68K( os9_get_l( args+32 ) );
            if (!RANGE_IN_ARENA( buf,size )) return os9error(E_BPADDR);

                n= SpfRecvFrom( spP, buf,size, 0, &from );
            if (n<0) {
                if (errno==EAGAIN || errno==EINTR) return SpfPark( pid );
                if (errno==EINVAL) return os9error(E_PARAM);
                return os9error(E_READ);
            }
            os9_set_l( args+20, (uint32_t)n );

            if (os9_get_l( args+28 )!=0 && RANGE_IN_ARENA( flenP,4 )) {
                room= os9_get_l( flenP ); if (room>GUEST_SOCKADDR) room= GUEST_SOCKADDR;
                if (os9_get_l( args+32 )!=0 && RANGE_IN_ARENA( fromP,room ))
                    os9_set_l( flenP, SpfGuestAddr( &from, fromP, room ) );
            }
            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: recvfrom %d bytes\n", (int)n ));
            return 0;
        }

        case SPFOP_SOPT: {
            /* Anything this does not know -- and any the host refuses -- is
               still answered as done, as it always was: the clients measured
               check for nothing more. */
            uint32_t level, name, olen;
            byte*    val;
            Boolean  known;
            int      e;

            if (ptr==0 || len<20 || !RANGE_IN_ARENA( args,20 ) || SpfFd( spP )<0) return 0;
            level= os9_get_l( args+4 );
            name = os9_get_l( args+8 );
            val  = (byte*)FROM68K( os9_get_l( args+12 ) );
            olen = os9_get_l( args+16 );
            if (olen>16 || !RANGE_IN_ARENA( val,olen )) return 0;

            e= SpfSetOpt( spP, level,name, val,olen, &known );
            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: option level $%X name $%X len %u -> %s\n",
                         level, name, olen, !known ? "not known" : e==0 ? "applied" : "host refused" ));
            return 0;
        }

        default:
            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: operation $%08X not implemented\n",
                                                 (uint32_t)op ));
            return os9error(E_UNKSVC);          /* what the library checks for */
    } // switch
  #else
    (void)pid; (void)spP; (void)d1; (void)blk;
    return os9error(E_UNKSVC);
  #endif
} /* pSspf */


#if defined UNIX && !defined MINGW

static os9err SpfIspErr( int herr, os9err otherwise )
/* The host's errno <herr> as the socket error the older library's callers
   print, or <otherwise> for one with no OS-9 counterpart. */
{
    os9err e;

    switch (HostSockErr( herr )) {
        case hsConnRefused:    e= OS9_ECONNREFUSED;    break;
        case hsAddrInUse:      e= OS9_EADDRINUSE;      break;
        case hsAddrNotAvail:   e= OS9_EADDRNOTAVAIL;   break;
        case hsNetUnreach:     e= OS9_ENETUNREACH;     break;
        case hsTimedOut:       e= OS9_ETIMEDOUT;       break;
        case hsConnReset:      e= OS9_ECONNRESET;      break;
        case hsConnAborted:    e= OS9_ECONNABORTED;    break;
        case hsNotConn:        e= OS9_ENOTCONN;        break;
        case hsIsConn:         e= OS9_EISCONN;         break;
        case hsAfNoSupport:    e= OS9_EAFNOSUPPORT;    break;
        case hsProtoNoSupport: e= OS9_EPROTONOSUPPORT; break;
        case hsNoProtoOpt:     e= OS9_ENOPROTOOPT;     break;
        case hsMsgSize:        e= OS9_EMSGSIZE;        break;
        case hsNoBufs:         e= OS9_ENOBUFS;         break;
        case hsOpNotSupp:      e= OS9_EOPNOTSUPP;      break;
        case hsDestAddrReq:    e= OS9_EDESTADDRREQ;    break;
        case hsAccess:         e= E_PERMIT;            break;
        case hsInval:          e= E_PARAM;             break;
        case hsPipe:           e= E_WRITE;             break;
        default:               e= otherwise;           break;
    }
    return os9error(e);
} /* SpfIspErr */


static byte* IspPtr( uint32_t guest, uint32_t len )
/* The guest's buffer <guest> of <len> bytes, or NULL if it is absent or not
   wholly in the guest's memory. */
{
    byte* p= (byte*)FROM68K( guest );
    return guest!=0 && RANGE_IN_ARENA( p,len ) ? p : NULL;
} /* IspPtr */


static const byte* IspAddr( uint32_t guest, uint32_t len, os9err* err )
/* The socket address a call was given, or NULL with <*err> set: an absent or
   short one is a bad parameter, and only the Internet family is served. */
{
    const byte* a= len>=8 ? IspPtr( guest,8 ) : NULL;

    if (a==NULL)                           { *err= os9error(E_PARAM);          return NULL; }
    if (os9_get_w( (byte*)a )!=GUEST_AF_INET) { *err= os9error(OS9_EAFNOSUPPORT); return NULL; }
    return a;
} /* IspAddr */


static int IspFlags( uint32_t guest )
/* BSD message flags as the host's; bits it does not know are dropped */
{
    int f= 0;
    if (guest & GUEST_MSG_OOB      ) f|= MSG_OOB;
    if (guest & GUEST_MSG_PEEK     ) f|= MSG_PEEK;
    if (guest & GUEST_MSG_DONTROUTE) f|= MSG_DONTROUTE;
    return f;
} /* IspFlags */


static os9err IspPutAddr( const struct sockaddr_in* sa, uint32_t to, uint32_t lenAt )
/* Hand a socket address back the BSD way: into the buffer at <to>, as much as
   the length at <lenAt> says it holds, and that length set to what was
   written. Either may be 0 when the caller does not want it. */
{
    byte*    lenP= IspPtr( lenAt,4 );
    byte*    buf;
    uint32_t room;

    if (to==0 || lenAt==0) return 0;
    if (lenP==NULL) return os9error(E_BPADDR);
    room= os9_get_l( lenP ); if (room>GUEST_SOCKADDR) room= GUEST_SOCKADDR;
    if ((buf= IspPtr( to,room ))==NULL && room>0) return os9error(E_BPADDR);
    os9_set_l( lenP, room>0 ? SpfGuestAddr( sa, buf, room ) : 0 );
    return 0;
} /* IspPutAddr */


static os9err IspAccept( ushort pid, syspath_typ* spP, regs_type* rp )
/* Accept on a listening "/socket" path: the connection comes back as a new
   path of the caller's own, its number in d1, and the peer's address where
   the caller asked for it. */
{
    struct sockaddr_in peer;
    syspath_typ*       nsp;
    ushort             up;
    int                nfd, herr;
    os9err             err;

    SpfResume( pid );   /* this call may be a retry of a parked accept */

    /* Where the peer's address is to go is checked before a connection is
       taken, so a bad pointer costs the caller an error, not the connection
       and a path it was never told about. */
    if (rp->a[0]!=0 && rp->a[1]!=0) {
        byte*    lenP= IspPtr( rp->a[1],4 );
        uint32_t room= lenP!=NULL ? os9_get_l( lenP ) : 0;

        if (lenP==NULL) return os9error(E_BPADDR);
        if (room>GUEST_SOCKADDR) room= GUEST_SOCKADDR;
        if (room>0 && IspPtr( rp->a[0],room )==NULL) return os9error(E_BPADDR);
    }

        nfd= SpfTake( spP, &peer, &herr );
    if (nfd<0) {
        if (herr==EAGAIN || herr==EINTR) return SpfPark( pid );
        return SpfIspErr( herr, E_NOTRDY );
    }

        err= usrpath_open( pid,&up, fSPF, "/socket", spP->mode );
    if (err) { close( nfd ); return err; }
    nsp= get_syspath( pid, procs[pid].usrpaths[up] );
    if (nsp==NULL) { close( nfd ); usrpath_close( pid, up ); return os9error(E_BPNUM); }

    nsp->u.spf.fdPlus1  = nfd+1;
    nsp->u.spf.proto    = spP->u.spf.proto;
    nsp->u.spf.connected= true;
    rp->d[1]= up;

    debugprintf( dbgSpecialIO,dbgNorm,("# SPF: accept from port %u -> host fd %d, path %u\n",
                                         (uint32_t)ntohs( peer.sin_port ), nfd, (uint32_t)up ));
    return IspPutAddr( &peer, rp->a[0], rp->a[1] );
} /* IspAccept */


static os9err IspName( syspath_typ* spP, regs_type* rp )
/* This end's address (d2=0), the far end's (1), or the host's name (2). */
{
    struct sockaddr_in who;
    socklen_t          wlen= sizeof(who);
    byte*              lenP= IspPtr( rp->a[1],4 );
    uint32_t           room;
    int                r;

    if (lenP==NULL) return os9error(E_BPADDR);
    room= os9_get_l( lenP );

    if (rp->d[2]==2) {
        byte* name= IspPtr( rp->a[0],room );
        if (name==NULL || room==0) return os9error(E_BPADDR);
        if (gethostname( (char*)name, room )!=0) return SpfIspErr( errno, E_PARAM );
        name[ room-1 ]= NUL;           /* a name that did not fit is cut short */
        return 0;
    }
    if (rp->d[2]>1) return os9error(E_PARAM);

    memset( &who,0,sizeof(who) );
    r= rp->d[2]==1 ? getpeername( SpfFd( spP ), (struct sockaddr*)&who, &wlen )
                   : getsockname( SpfFd( spP ), (struct sockaddr*)&who, &wlen );
    if (r!=0) return SpfIspErr( errno, E_NOTRDY );
    return IspPutAddr( &who, rp->a[0], rp->a[1] );
} /* IspName */


static os9err IspGetOpt( syspath_typ* spP, regs_type* rp )
/* A socket option's value, read the BSD way: an int, into the buffer at a0,
   with its length at a1. */
{
    byte*     lenP= IspPtr( rp->a[1],4 );
    byte*     val;
    int       hl, hn, iv= 0;
    socklen_t ilen= sizeof(iv);

    if (lenP==NULL)                                         return os9error(E_BPADDR);
    if (!SpfOptName( rp->d[3], rp->d[2], &hl, &hn ))        return os9error(OS9_ENOPROTOOPT);
    if (os9_get_l( lenP )<4 || (val= IspPtr( rp->a[0],4 ))==NULL) return os9error(E_PARAM);
    if (getsockopt( SpfFd( spP ), hl, hn, &iv, &ilen )!=0)  return SpfIspErr( errno, E_PARAM );

    /* a pending error is the host's number; the guest is owed OS-9's */
    if (hl==SOL_SOCKET && hn==SO_ERROR && iv!=0) iv= (int)SpfIspErr( iv, E_NOTRDY );
    os9_set_l( val,  (uint32_t)iv );
    os9_set_l( lenP, 4 );
    return 0;
} /* IspGetOpt */


static os9err IspCall( ushort pid, syspath_typ* spP, ushort func, regs_type* rp )
/* One socket call of the older library, its arguments where that library puts
   them: a0 the buffer or address, a1 a second one, d2 a length or count, d3 and
   d4 what follows in the BSD call; a count or a new path comes back in d1. */
{
    int      herr;
    os9err   err= 0;
    uint32_t len= rp->d[2];

    if (func==SS_Resv) {
        /* socket(domain, type, protocol), as three longwords at a0 */
        const byte* dtp= IspPtr( rp->a[0],12 );

        if (dtp==NULL || len<12)                   return os9error(E_PARAM);
        if (os9_get_l( dtp )!=GUEST_AF_INET)       return os9error(OS9_EAFNOSUPPORT);
            herr= SpfMake( spP, os9_get_l( dtp+4 ) );
        return herr==0 ? 0 : SpfIspErr( herr, E_NOTRDY );
    }
    if (SpfFd( spP )<0) return os9error(OS9_ENOTSOCK);  /* no socket made yet */

    switch (func) {
        case SS_Bind: {
            const byte* at= IspAddr( rp->a[0], len, &err );
            if (at==NULL) return err;
            return (herr= SpfBind( spP, at ))==0 ? 0 : SpfIspErr( herr, E_NOTRDY );
        }

        case SS_Listen:
            return (herr= SpfListen( spP, (int)len ))==0 ? 0 : SpfIspErr( herr, E_NOTRDY );

        case SS_Connect: {
            const byte* to= NULL;

            SpfResume( pid );   /* this call may be a retry of a parked connect */
            if (!spP->u.spf.connecting && (to= IspAddr( rp->a[0], len, &err ))==NULL) return err;
            switch (SpfConnect( spP, to, &herr )) {
                case stepDone: return 0;
                case stepWait: return SpfPark( pid );
                default:       return SpfIspErr( herr, E_NOTRDY );
            }
        }

        case SS_Accept:
            return IspAccept( pid, spP, rp );

        case SS_Send:
        case SS_SendTo: {
            const byte* buf= len>0 ? IspPtr( rp->a[0],len ) : NULL;
            const byte* to = NULL;
            uint32_t    n  = len;
            ssize_t     sent;

            if (len>0 && buf==NULL) return os9error(E_BPADDR);
            if (func==SS_SendTo && rp->a[1]!=0 &&
                (to= IspAddr( rp->a[1], rp->d[4], &err ))==NULL) return err;

            if (to==NULL) {                             /* a connected socket */
                SpfResume( pid );   /* this call may be a retry of a parked send */
                    err= SpfSend( pid, spP, &n, buf, IspFlags( rp->d[3] ), &herr );
                if (err && herr!=0) err= SpfIspErr( herr, E_WRITE );
                if (!err) rp->d[1]= n;
                return err;
            }
                sent= SpfSendTo( spP, buf,len, IspFlags( rp->d[3] ), to );
            if (sent<0) return SpfIspErr( errno, E_WRITE );
            rp->d[1]= (uint32_t)sent;
            return 0;
        }

        case SS_Recv:
        case SS_RecvFr: {
            struct sockaddr_in from;
            byte*              buf= IspPtr( rp->a[0],len );
            ssize_t            n;

            SpfResume( pid );   /* this call may be a retry of a parked receive */
            if (len==0)     { rp->d[1]= 0; return 0; }
            if (buf==NULL)  return os9error(E_BPADDR);

                n= SpfRecvFrom( spP, buf,len, IspFlags( rp->d[3] ), &from );
            if (n<0) {
                if (errno==EAGAIN || errno==EINTR) return SpfPark( pid );
                return SpfIspErr( errno, E_READ );
            }
            rp->d[1]= (uint32_t)n;                      /* 0: the far end closed */
            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: receive %d bytes\n", (int)n ));
            return func==SS_RecvFr ? IspPutAddr( &from, rp->a[1], rp->d[4] ) : 0;
        }

        case SS_GNam:
            return IspName( spP, rp );

        case SS_SOpt: {
            /* level in d3, name in d2, the value at a0 and its length in d4.
               One this does not know is answered as done, as on the other
               front end: it cannot be applied, and refusing it would stop a
               program over an option that only tunes. */
            uint32_t    olen= rp->d[4];
            const byte* val = olen>0 ? IspPtr( rp->a[0],olen ) : NULL;
            Boolean     known;

            if (olen>GUEST_SOCKADDR || (olen>0 && val==NULL)) return os9error(E_PARAM);
            if (olen==0) return 0;          /* nothing to set: BSD's toggle of old */
                herr= SpfSetOpt( spP, rp->d[3], rp->d[2], val,olen, &known );
            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: option level $%X name $%X -> %s\n",
                         rp->d[3], rp->d[2], !known ? "not known" : herr==0 ? "applied" : "host refused" ));
            return herr==0 ? 0 : SpfIspErr( herr, E_PARAM );
        }

        case SS_GOpt:
            return IspGetOpt( spP, rp );

        case SS_Shut: {
            /* how, 0 to 2, as BSD numbers it; the host's names for the same */
            static const int how[3]= { SHUT_RD, SHUT_WR, SHUT_RDWR };
            if (len>2) return os9error(E_PARAM);
            return shutdown( SpfFd( spP ), how[len] )==0 ? 0 : SpfIspErr( errno, E_NOTRDY );
        }

        default:
            return os9error(E_UNKSVC);
    }
} /* IspCall */

#endif


static os9err pSisp( ushort pid, syspath_typ* spP, ushort func, regs_type* rp, Boolean* taken )
/* The older socket library's calls, on a path opened as "/socket" (see the top
   of this file). A path opened any other way leaves them to the usual
   dispatch, where they are not socket calls at all. */
{
    *taken= spP->u.spf.isp && func>=SS_Bind && func<=SS_RecvFr;
    if (!*taken) return 0;

  #if defined UNIX && !defined MINGW
    return IspCall( pid, spP, func, rp );
  #else
    (void)pid; (void)rp;
    return os9error(E_UNKSVC);
  #endif
} /* pSisp */


#if defined UNIX && !defined MINGW
int spf_add_wait_fds( fd_set* rfds, int maxfd )
/* Every socket armed with SS_SSig, for the same reason as the terminals: the
 * idle wait polls them (spf_poll_signals) and can stop polling once it is
 * watching them instead. A socket with no signal armed is not watched -- the
 * process waiting on it is parked in a read, which is a different wait. */
{
    int k;

    for (k=1; k<MAXSYSPATHS; k++) {
        syspath_typ* sp= &syspaths[k];
        int          fd;

        if (sp->type!=fSPF || sp->signal_to_send==0) continue;
        fd= SpfFd( sp );
        /* FD_SET past FD_SETSIZE writes off the end of the set -- host stack
           corruption, not a missed wakeup. A host that hands out an fd that
           high just goes back to being polled, which still works. */
        if (fd<0 || fd>=FD_SETSIZE) continue;
        FD_SET( fd, rfds );
        if (fd>maxfd) maxfd= fd;
    } /* for */

    return maxfd;
} /* spf_add_wait_fds */
#endif

void spf_poll_signals( void )
/* SS_SSig on a socket path: the signal goes out when data arrives. The arming
   call already sends it if data is there at that moment; after that, sockets
   have no one to notice, so the periodic input check asks here -- the way
   terminals are asked (KeyToBuffer). Readable covers data, an end of file and
   a connection waiting to be accepted. */
{
  #if defined UNIX && !defined MINGW
    int k;

    for (k=1; k<MAXSYSPATHS; k++) {
        syspath_typ*  sp= &syspaths[k];
        struct pollfd pf;

        if (sp->type!=fSPF || sp->signal_to_send==0 || SpfFd( sp )<0) continue;
        pf.fd= SpfFd( sp ); pf.events= POLLIN; pf.revents= 0;
        if (poll( &pf,1, 0 )>0 && pf.revents!=0) {
            send_signal( sp->signal_pid, sp->signal_to_send );
            sp->signal_to_send= 0;             /* one signal per arming, as OS-9 */
        }
    }
  #endif
} /* spf_poll_signals */


void init_SPF( fmgr_typ* f )
/* install all procedures of the SPF socket manager */
{
    gs_typ* gs= &f->gs;
    ss_typ* ss= &f->ss;

    f->open      = pSopen;
    f->close     = pSclose;
    f->read      = pSread;
    f->readln    = pSread;        /* a stream has no records */
    f->write     = pSwrite;
    f->writeln   = pSwrite;
    f->seek      = pNop_num;      /* no random access, and no error: see pipeman */
    f->regstat   = pSisp;         /* "/socket" paths: see the top of this file */

    /* getstat */
    gs->_SS_Opt  = pSopt;
    gs->_SS_Ready= pSready;
    gs->_SS_DevNm= pSnam;
    gs->_SS_SPF  = pSspf;

    /* setstat */
    ss->_SS_Opt  = pNop_opt;      /* the caller copies the listening path's
                                     option section onto the connection's path;
                                     accepted and ignored, as in the ISP manager */
    ss->_SS_SPF  = pSspf;
} /* init_SPF */

/* eof */
