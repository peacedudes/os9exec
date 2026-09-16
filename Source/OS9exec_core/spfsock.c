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
 * WHAT WAS OBSERVED, and is all this relies on:
 *  - the block begins with three longwords: an operation, a byte count, and a
 *    pointer to that many bytes of arguments;
 *  - creating a socket passes three longwords, which are a BSD domain, type
 *    and protocol (2, 1, 0 for a TCP client);
 *  - connecting passes a BSD socket address: a 16-bit family, then the port
 *    and the address, both already in network order;
 *  - option calls pass a level and an option, and the clients run fine when
 *    they are accepted and not applied;
 *  - once connected, data moves with ordinary I$Read and I$Write on the path.
 *
 * SCOPE. Client connections work end to end (TCP, and UDP as far as
 * connect/read/write take it). The server calls -- bind, listen and accept --
 * are here and each does what it says, but NO SERVER WORKS YET: after accept
 * the caller asks for more of the protocol than is decoded (see the roadmap).
 * Raw sockets (ping) need host privileges and are not attempted.
 *
 * What accept returns is worth stating, because it is not what it looks like.
 * It does NOT hand back a path. It answers three longwords {domain, type,
 * protocol}, which the caller feeds to its own socket() routine -- that opens
 * a second "/ip0#1/tcp0" itself and makes a fresh socket on it. So the
 * connection accepted here is held on the listening path (acceptPlus1) until
 * the call that transfers it to that path is understood; today it is closed
 * when the listening path closes. This was read out of the client's own code,
 * not guessed: four register and three memory layouts all behaved identically
 * beforehand, which is what "the value is never read" looks like.
 */

#include "os9exec_incl.h"

#if defined UNIX && !defined MINGW
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <sys/ioctl.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>
#endif

/* the operations seen in the first longword of the block */
#define SPFOP_SOCKET  0x01060002   /* make a socket: domain, type, protocol */
#define SPFOP_ACCEPT  0x01060080   /* take a connection: answers {domain,type,proto} */
#define SPFOP_BIND    0x6C
#define SPFOP_LISTEN  0x6D
#define SPFOP_CONNECT 0x6E
#define SPFOP_SOPT    0x74


#if defined UNIX && !defined MINGW

static int SpfFd( syspath_typ* spP )
/* the host socket of this path, or -1 while it has none */
{   return spP->u.spf.fdPlus1-1;
} /* SpfFd */


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
        ssize_t n= send( fd, buffer+done, *lenP-done, 0 );
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
   with "can't accept" while it answered E$UnkSvc. These are the same values
   the ISP manager has always returned for a socket (network.c, netstdopts):
   PD_DTP=7 is the socket device type. */
static const byte spfstdopts[OPTSECTSIZE]=
                { 7,        /* PD_DTP: 7 = SOCKET */
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
    len = os9_get_l( blk+4 );
    ptr = os9_get_l( blk+8 );
    args= (byte*)FROM68K( ptr );
    if (ptr!=0 && !RANGE_IN_ARENA( args,len>64 ? 64:len )) return os9error(E_BPADDR);

    switch (op) {
        case SPFOP_SOCKET: {
            uint32_t type;
            int      fd, flags;

            if (ptr==0 || len<12) return os9error(E_PARAM);
            type= os9_get_l( args+4 );          /* the BSD socket type */
            if (SpfFd( spP )>=0) close( SpfFd( spP ) );

                fd= socket( AF_INET, type==2 ? SOCK_DGRAM : SOCK_STREAM, 0 );
            if (fd<0) return os9error(E_NOTRDY);

            spP->u.spf.fdPlus1  = fd+1;
            spP->u.spf.proto    = (ushort)type;
            spP->u.spf.connected= false;
            flags= fcntl( fd, F_GETFL, 0 );     /* reads must never block the emulator */
            if (flags>=0) fcntl( fd, F_SETFL, flags | O_NONBLOCK );
            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: socket type=%u -> host fd %d\n",
                                                 (uint32_t)type, fd ));
            return 0;
        }

        case SPFOP_CONNECT: {
            struct sockaddr_in sa;
            int  fd= SpfFd( spP ), r, flags;

            if (fd<0) return os9error(E_NOTRDY);
            if (ptr==0 || len<8) return os9error(E_PARAM);

            memset( &sa,0,sizeof(sa) );
            #ifdef __APPLE__
              sa.sin_len= sizeof(sa);
            #endif
            sa.sin_family= AF_INET;
            memcpy( &sa.sin_port,        args+2, 2 ); /* both already in network order */
            memcpy( &sa.sin_addr.s_addr, args+4, 4 );

            flags= fcntl( fd, F_GETFL, 0 );           /* connect blocking, read non-blocking */
            if (flags>=0) fcntl( fd, F_SETFL, flags & ~O_NONBLOCK );
                r= connect( fd, (struct sockaddr*)&sa, sizeof(sa) );
            if (flags>=0) fcntl( fd, F_SETFL, flags |  O_NONBLOCK );

            debugprintf( dbgSpecialIO,dbgNorm,("# SPF: connect -> %d\n", r ));
            if (r!=0) return os9error(E_NOTRDY);
            spP->u.spf.connected= true;
            return 0;
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
            if (bind( fd, (struct sockaddr*)&sa, sizeof(sa) )!=0) return os9error(E_SHARE);
            return 0;
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

        case SPFOP_SOPT:                        /* accepted, not applied: the clients run */
            return 0;

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
    ss->_SS_SPF  = pSspf;
} /* init_SPF */

/* eof */
