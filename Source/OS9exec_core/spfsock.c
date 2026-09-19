/* spfsock.c -- socket paths ("/ip0#1/tcp0" and its siblings) over host sockets
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
 */

/* Taken before os9exec_incl.h, which undefines the POSIX socket error names
   so that os9errno.h can give them OS-9 values: past it, EINPROGRESS is not
   the host's number any more. */
#ifndef _WIN32
  #include <errno.h>
  static const int hostEINPROGRESS= EINPROGRESS;
#endif

#include "os9exec_incl.h"

#if defined UNIX && !defined MINGW
  #include <sys/socket.h>
  #include <netinet/in.h>
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

#endif


static os9err pSopen( _pid_, syspath_typ* spP, _modeP_, const char* pathname )
/* open a socket path. The host socket is not made here: the caller asks for
   it with the socket operation, which carries the domain and type. */
{
    const char* p= pathname;

    spP->u.spf.fdPlus1    = 0;
    spP->u.spf.acceptPlus1= 0;
    spP->u.spf.proto      = 0;
    spP->u.spf.connected  = false;
    spP->u.spf.connecting = false;
    spP->u.spf.bareIcmp   = false;

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
    uint32_t done= 0;
    int      fd  = SpfFd( spP );

    if (fd<0) return os9error(E_NOTRDY);

    while (done<*lenP) {
        ssize_t n= send( fd, buffer+done, *lenP-done, SPF_SENDFLAGS );
        if (n>0) { done+= (uint32_t)n; continue; }

        if (n<0 && (errno==EAGAIN || errno==EINTR)) continue; /* see the read path */
        break;                                  /* the connection is gone */
    }

    if (done==0 && *lenP>0) { *lenP= 0; return os9error(E_WRITE); }
    *lenP= done;
    return 0;
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
    return cnt>0 ? 0 : os9error(E_NOTRDY);
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

    (void)pid;
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
        case SPFOP_SOCKET: {
            uint32_t type;
            int      fd, flags;

            if (ptr==0 || len<12) return os9error(E_PARAM);
            type= os9_get_l( args+4 );          /* the BSD socket type */
            if (SpfFd( spP )>=0) close( SpfFd( spP ) );

            /* A raw socket (type 3, what ping opens) becomes the host's ICMP
               datagram socket, which an ordinary user may open where a raw one
               needs the super user; it hands back replies IP header first, as
               a raw socket does. Failing that, a real raw socket. */
            spP->u.spf.bareIcmp= false;
            if (type==3) {
                    fd= socket( AF_INET, SOCK_DGRAM, IPPROTO_ICMP );
                #ifdef linux
                  spP->u.spf.bareIcmp= fd>=0; /* see SPFOP_RECVFR */
                #endif
                if (fd<0) fd= socket( AF_INET, SOCK_RAW, IPPROTO_ICMP );
            }
            else    fd= socket( AF_INET, type==2 ? SOCK_DGRAM : SOCK_STREAM, 0 );
            if (fd<0) return os9error(E_NOTRDY);
            SpfNoSigpipe( fd );

            spP->u.spf.fdPlus1   = fd+1;
            spP->u.spf.proto     = (ushort)type;
            spP->u.spf.connected = false;
            spP->u.spf.connecting= false;
            flags= fcntl( fd, F_GETFL, 0 );     /* reads must never block the emulator */
            if (flags>=0) fcntl( fd, F_SETFL, flags | O_NONBLOCK );
            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: socket type=%u -> host fd %d\n",
                                                 (uint32_t)type, fd ));
            return 0;
        }

        case SPFOP_CONNECT: {
            /* Never a blocking connect: it would hold every OS-9 process for
               as long as the handshake takes, and the system tick's signal
               cuts it short anyway (connect is not restarted, SA_RESTART or
               not), which failed a slow connection at once. So it is started
               and the caller parked, as a read with nothing to read is; the
               same call comes round again, and then asks whether it is done. */
            struct sockaddr_in sa;
            struct pollfd      pf;
            int  fd= SpfFd( spP ), r, soerr= 0;
            socklen_t          elen= sizeof(soerr);

            SpfResume( pid );   /* this call may be a retry of a parked connect */
            if (fd<0) return os9error(E_NOTRDY);

            if (spP->u.spf.connecting) {
                /* Only an event says the handshake is over. poll() can also
                   return -1 when the tick's signal lands in it, and SO_ERROR
                   reads 0 on a connection still under way -- taken together
                   those declared a pending connect made (then its first write
                   failed), about one time in four under the tick. */
                pf.fd= fd; pf.events= POLLOUT; pf.revents= 0;
                if (poll( &pf,1, 0 )<=0 || pf.revents==0) return SpfPark( pid ); /* not yet */
                spP->u.spf.connecting= false;
                if (getsockopt( fd, SOL_SOCKET, SO_ERROR, &soerr,&elen )!=0 || soerr!=0) {
                    debugprintf( dbgSpecialIO,dbgNorm,("# SPF: connect failed, %d\n", soerr ));
                    return os9error(E_NOTRDY);
                }
                debugprintf( dbgSpecialIO,dbgNorm,("# SPF: connect -> done\n" ));
                spP->u.spf.connected= true;
                return 0;
            }

            if (ptr==0 || len<8) return os9error(E_PARAM);

            memset( &sa,0,sizeof(sa) );
            #ifdef __APPLE__
              sa.sin_len= sizeof(sa);
            #endif
            sa.sin_family= AF_INET;
            memcpy( &sa.sin_port,        args+2, 2 ); /* both already in network order */
            memcpy( &sa.sin_addr.s_addr, args+4, 4 );

                r= connect( fd, (struct sockaddr*)&sa, sizeof(sa) ); /* fd is non-blocking */
            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: connect -> %d\n", r ));
            if (r==0) { spP->u.spf.connected= true; return 0; }
            if (errno!=hostEINPROGRESS && errno!=EINTR) return os9error(E_NOTRDY);
            spP->u.spf.connecting= true;
            return SpfPark( pid );
        }

        case SPFOP_BIND: {
            struct sockaddr_in sa;
            int  fd= SpfFd( spP ), on= 1;

            if (fd<0) return os9error(E_NOTRDY);
            if (ptr==0 || len<8) return os9error(E_PARAM);

            /* the same BSD socket address connect is given, observed: a 16-bit
               family, then port and address already in network order */
            memset( &sa,0,sizeof(sa) );
            #ifdef __APPLE__
              sa.sin_len= sizeof(sa);
            #endif
            sa.sin_family= AF_INET;
            memcpy( &sa.sin_port,        args+2, 2 );
            memcpy( &sa.sin_addr.s_addr, args+4, 4 );

            /* A server that has just exited leaves its port in TIME_WAIT, and
               without this the next run cannot bind it for a minute or more.
               The guest asks for its own options through the option call,
               which we accept without applying -- so if this were left to the
               guest no server could be restarted. */
            setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &on,sizeof(on) );

            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: bind port %u\n",
                                                 (uint32_t)ntohs( sa.sin_port ) ));
            if (bind( fd, (struct sockaddr*)&sa, sizeof(sa) )==0) return 0;

            /* A port below 1024 on one address is the super user's on macOS,
               while the same port on every address is not. OS-9 grants it
               (ftpd binds its data connection to 127.0.0.1 port 20), so try
               the port the program asked for on every address before
               refusing. */
            if (errno==EACCES && ntohs( sa.sin_port )<1024 && sa.sin_addr.s_addr!=htonl( INADDR_ANY )) {
                sa.sin_addr.s_addr= htonl( INADDR_ANY );
                if (bind( fd, (struct sockaddr*)&sa, sizeof(sa) )==0) return 0;
            }
            return os9error(E_SHARE);
        }

        case SPFOP_LISTEN: {
            int fd= SpfFd( spP ), backlog= 5;

            if (fd<0) return os9error(E_NOTRDY);
            if (ptr!=0 && len>=4) backlog= (int)os9_get_l( args );
            if (backlog<=0) backlog= 5;

            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: listen backlog %d\n", backlog ));
            if (listen( fd, backlog )!=0) return os9error(E_NOTRDY);
            return 0;
        }

        case SPFOP_ACCEPT: {
            struct sockaddr_in peer;
            socklen_t          plen= sizeof(peer);
            int  fd= SpfFd( spP ), nfd, flags;

            SpfResume( pid );   /* this call may be a retry of a parked accept */
            if (fd<0) return os9error(E_NOTRDY);
            if (ptr==0 || len<12) return os9error(E_PARAM);

                nfd= accept( fd, (struct sockaddr*)&peer, &plen );
            if (nfd<0) {
                if (errno==EAGAIN || errno==EINTR) return SpfPark( pid );
                return os9error(E_NOTRDY);
            }

            flags= fcntl( nfd, F_GETFL, 0 );    /* reads must never block the emulator */
            if (flags>=0) fcntl( nfd, F_SETFL, flags | O_NONBLOCK );
            SpfNoSigpipe( nfd );

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
            nsp= &syspaths[ id ];
            if (nsp->type!=fSPF)           return os9error(E_BPNUM);

            if (SpfFd( nsp )>=0) close( SpfFd( nsp ) ); /* its own socket is unused */
            nsp->u.spf.fdPlus1    = spP->u.spf.acceptPlus1;
            nsp->u.spf.proto      = spP->u.spf.proto;
            nsp->u.spf.connected  = true;
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

            room= os9_get_l( lenP ); if (room>16) room= 16;
            if (!RANGE_IN_ARENA( args,room ))    return os9error(E_BPADDR);
            {   byte sa[16];
                memset   ( sa,0,sizeof(sa) );
                os9_set_w( sa, AF_INET );
                memcpy   ( sa+2, &who.sin_port, 2 );          /* network order */
                memcpy   ( sa+4, &who.sin_addr.s_addr, 4 );
                memcpy   ( args, sa, room );
            }
            os9_set_l( lenP, room );
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
            struct sockaddr_in to;
            byte*    rec;
            ssize_t  n;
            int      fd= SpfFd( spP );

            if (fd<0) return os9error(E_NOTRDY);
            if (!RANGE_IN_ARENA( blk,32 )) return os9error(E_BPADDR);
                rec= (byte*)FROM68K( os9_get_l( blk+28 ) );
            if (!RANGE_IN_ARENA( rec,20 ) || rec[3]<8) return os9error(E_PARAM);
            if (ptr==0 || !RANGE_IN_ARENA( args,len )) return os9error(E_BPADDR);

            memset( &to,0,sizeof(to) );
            #ifdef __APPLE__
              to.sin_len= sizeof(to);
            #endif
            to.sin_family= AF_INET;
            memcpy( &to.sin_port,        rec+6, 2 );
            memcpy( &to.sin_addr.s_addr, rec+8, 4 );

            if (spP->u.spf.bareIcmp && len>=8) memcpy( spP->u.spf.echoId, args+4, 2 );
                n= sendto( fd, args,len, SPF_SENDFLAGS, (struct sockaddr*)&to, sizeof(to) );
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
            socklen_t          flen= sizeof(from);
            byte              *buf, *fromP, *flenP;
            uint32_t           size, room;
            ssize_t            n;
            int                fd= SpfFd( spP );

            SpfResume( pid );
            if (fd<0) return os9error(E_NOTRDY);
            if (ptr==0 || !RANGE_IN_ARENA( args,40 )) return os9error(E_BPADDR);
            buf  = (byte*)FROM68K( os9_get_l( args+16 ) );
            size =                 os9_get_l( args+20 );
            flenP= (byte*)FROM68K( os9_get_l( args+28 ) );
            fromP= (byte*)FROM68K( os9_get_l( args+32 ) );
            if (!RANGE_IN_ARENA( buf,size )) return os9error(E_BPADDR);

            /* Linux's ICMP datagram socket gives the reply without the IP
               header a raw socket puts first, and with the socket's own echo
               identifier in place of the one the guest sent. The guest reads
               it as raw, so it gets a header and its identifier back. */
            {   uint32_t hdr= spP->u.spf.bareIcmp ? 20:0;
                if (size<=hdr) return os9error(E_PARAM);

                memset( &from,0,sizeof(from) );
                    n= recvfrom( fd, buf+hdr,size-hdr, 0, (struct sockaddr*)&from, &flen );
                if (n<0) {
                    if (errno==EAGAIN || errno==EINTR) return SpfPark( pid );
                    return os9error(E_READ);
                }
                if (hdr>0) {
                    memset   ( buf,0,hdr );
                    buf[0]= 0x45;                         /* IPv4, five words */
                    os9_set_w( buf+2, (uint16_t)(n+hdr) );
                    buf[8]= 64;                           /* time to live */
                    buf[9]= IPPROTO_ICMP;
                    memcpy   ( buf+12, &from.sin_addr.s_addr, 4 );
                    memcpy   ( buf+16, &from.sin_addr.s_addr, 4 );
                    if (n>=8) memcpy( buf+hdr+4, spP->u.spf.echoId, 2 );
                    n+= hdr;
                }
            }
            os9_set_l( args+20, (uint32_t)n );

            if (os9_get_l( args+28 )!=0 && RANGE_IN_ARENA( flenP,4 )) {
                room= os9_get_l( flenP ); if (room>16) room= 16;
                if (os9_get_l( args+32 )!=0 && RANGE_IN_ARENA( fromP,room )) {
                    byte sa[16];
                    memset   ( sa,0,sizeof(sa) );
                    os9_set_w( sa, AF_INET );
                    memcpy   ( sa+2, &from.sin_port, 2 );
                    memcpy   ( sa+4, &from.sin_addr.s_addr, 4 );
                    memcpy   ( fromP, sa, room );
                    os9_set_l( flenP, room );
                }
            }
            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: recvfrom %d bytes\n", (int)n ));
            return 0;
        }

        case SPFOP_SOPT: {
            /* The options arrive by their BSD numbers, which the host may
               number differently (Linux does), so each one this knows is
               translated and applied to the host socket. Anything else -- and
               any the host refuses -- is still answered as done, as it always
               was: the clients measured check for nothing more. */
            uint32_t level, name, olen;
            byte*    val;
            int      hl= -1, hn= -1, fd= SpfFd( spP ), r= -1;

            if (ptr==0 || len<20 || !RANGE_IN_ARENA( args,20 ) || fd<0) return 0;
            level= os9_get_l( args+4 );
            name = os9_get_l( args+8 );
            val  = (byte*)FROM68K( os9_get_l( args+12 ) );
            olen = os9_get_l( args+16 );
            if (olen>16 || !RANGE_IN_ARENA( val,olen )) return 0;

            if (level==0xFFFF) {                 /* SOL_SOCKET */
                hl= SOL_SOCKET;
                switch (name) {
                    case 0x0004: hn= SO_REUSEADDR; break;
                    #ifdef SO_REUSEPORT
                    case 0x0200: hn= SO_REUSEPORT; break;
                    #endif
                    case 0x0020: hn= SO_BROADCAST; break;
                    case 0x0008: hn= SO_KEEPALIVE; break;
                }
            }
            else if (level==0) {                 /* IPPROTO_IP */
                hl= IPPROTO_IP;
                switch (name) {
                    case  9: hn= IP_MULTICAST_IF;    break;
                    case 10: hn= IP_MULTICAST_TTL;   break;
                    case 11: hn= IP_MULTICAST_LOOP;  break;
                    case 12: hn= IP_ADD_MEMBERSHIP;  break;
                    case 13: hn= IP_DROP_MEMBERSHIP; break;
                }
            }

            if (hn>=0) {
                if (olen==4 && hl==SOL_SOCKET) {           /* an int, big-endian in the guest */
                    int iv= (int)os9_get_l( val );
                    r= setsockopt( fd, hl, hn, &iv, sizeof(iv) );
                }
                else if (olen==1) {                        /* TTL, loop: one byte */
                    unsigned char cv= val[0];
                    r= setsockopt( fd, hl, hn, &cv, sizeof(cv) );
                }
                else {                                     /* addresses: network order already */
                    r= setsockopt( fd, hl, hn, val, (socklen_t)olen );
                }
            }
            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: option level $%X name $%X len %u -> %s\n",
                         level, name, olen, hn<0 ? "not known" : r==0 ? "applied" : "host refused" ));
            return 0;
        }

        default:
            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: operation $%08X not implemented\n",
                                                 (uint32_t)op ));
            return os9error(E_UNKSVC);          /* what the library checks for */
    } // switch
  #else
    (void)pid; (void)spP; (void)blk;
    return os9error(E_UNKSVC);
  #endif
} /* pSspf */


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
