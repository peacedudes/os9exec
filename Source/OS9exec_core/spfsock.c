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
 * SCOPE. Client connections (TCP, and UDP as far as connect/read/write take
 * it). Listening servers need the accept operation, which must hand back a
 * second path; what the caller expects there has not been established, so it
 * answers E$UnkSvc -- the same "not implemented" the library already checks
 * for -- rather than guessing. Raw sockets (ping) need host privileges and
 * are not attempted.
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
#define SPFOP_ACCEPT  0x01060080   /* take a connection (not implemented) */
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

    spP->u.spf.fdPlus1  = 0;
    spP->u.spf.proto    = 0;
    spP->u.spf.connected= false;

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
  #endif
    spP->u.spf.fdPlus1= 0;
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


static os9err pSnam( _pid_, syspath_typ* spP, char* volname )
{   strcpy( volname, spP->name ); return 0;
} /* pSnam */


static os9err pSspf( ushort pid, syspath_typ* spP, byte* blk )
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
    gs->_SS_Ready= pSready;
    gs->_SS_DevNm= pSnam;
    gs->_SS_SPF  = pSspf;

    /* setstat */
    ss->_SS_SPF  = pSspf;
} /* init_SPF */

/* eof */
