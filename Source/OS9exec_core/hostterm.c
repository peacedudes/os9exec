//
//    OS9exec, OS-9 emulator for Mac OS, Windows and Linux
//
//    Host-backed terminal devices: /t1../t49 bound to a real host endpoint.
//
//    Selected per device by an environment variable, mirroring OS9DISK and
//    OS9Hx: OS9T1, OS9T2, ... The VALUE picks the backend, exactly as
//    OS9DISK's value already picks host-directory vs RBF image:
//
//        OS9T1=pty              allocate a new pty, report the slave name
//        OS9T1=/dev/ttys004     open an existing tty device by name
//
//    Deliberately NOT the /hx discovery cascade (TwoCharDev, filestuff.c):
//    there is nothing to search for. Set = bound, unset = E_UNIT.
//

#include "os9exec_incl.h"

#if defined UNIX && !defined MINGW
  #include <errno.h>
  #include <fcntl.h>
  #include <termios.h>
  #include <unistd.h>
#endif

typedef struct {
    Boolean     open;       /* host fd is live */
    int         fd;         /* -1 when not open */
    ttydev_typ  dev;        /* per-device input buffer; KeyToBuffer target */
    char        endpoint[OS9PATHLEN]; /* what we actually opened, for messages */
} hostterm_typ;

/* Indexed by term_id directly; slot 0 is unused so the arithmetic stays
   obvious at every call site. */
static hostterm_typ hostterms[ HOSTTERM_MAX+1 ];
static Boolean      initialised= false;

static void hostterm_init( void )
{
    int k;
    if (initialised) return;

    for (k=0; k<=HOSTTERM_MAX; k++) {
        hostterms[k].open       = false;
        hostterms[k].fd         =    -1;
        hostterms[k].endpoint[0]=   NUL;
        hostterms[k].dev.installed = false;
        hostterms[k].dev.inBufUsed =     0;
        hostterms[k].dev.holdScreen= false;
        hostterms[k].dev.pid       =     0;
        hostterms[k].dev.spP       =  NULL;
    }

    initialised= true;
} /* hostterm_init */

Boolean hostterm_in_range( int term_id )
{
    return term_id>=HOSTTERM_MIN && term_id<=HOSTTERM_MAX;
} /* hostterm_in_range */

/* The OS9T<n> value for this device, or NULL.
   Plain getenv, not egetenv: egetenv resolves relative values against
   startPath, which is right for a DISK and wrong for an endpoint -- "pty" is
   not a path and must not become "<startPath>/pty". egetenv happens to pass
   non-OS9DISK names through verbatim today (os9main.c:302), but depending on
   that would silently couple us to a disk-path decision. */
static char* hostterm_spec( int term_id )
{
    char  name[16];
    char* v;

    if (!hostterm_in_range( term_id )) return NULL;

    snprintf( name,sizeof(name), "OS9T%d", term_id );
    v= getenv( name );

    if (v!=NULL && *v==NUL) v= NULL; /* set-but-empty means unset */
    return v;
} /* hostterm_spec */

Boolean hostterm_configured( int term_id )
{
    return hostterm_spec( term_id )!=NULL;
} /* hostterm_configured */

Boolean hostterm_bound( int term_id )
{
    hostterm_init();
    if (!hostterm_in_range( term_id )) return false;
    return hostterms[ term_id ].open;
} /* hostterm_bound */

#if defined UNIX && !defined MINGW

/* Put a tty into 8-bit-transparent raw mode.
   Verified necessary, not assumed: without this a round-trip through a pty
   mangles exactly the bytes a file transfer depends on -- XON/XOFF are
   swallowed, CR becomes CRLF, DEL becomes BS-space-BS.

   The flag manipulation below is the standard cfmakeraw() recipe (termios(3):
   disable input translation/flow control, disable output post-processing,
   disable canonical/echo/signal-generating input processing, 8-bit chars, no
   parity), applied by hand rather than calling the library function itself.
   os9_ll.h deliberately `#undef __USE_MISC` on Linux (module_from_book.h's
   `ulong` needs the slot glibc's own __USE_MISC-gated typedef would
   otherwise occupy -- a pre-existing, unrelated decision, not something to
   unpick here), which is also the guard glibc's termios.h puts around
   cfmakeraw()'s own declaration, so calling it directly on Linux is an
   implicit-function-declaration warning. macOS/BSD declare it unconditionally
   and mingw doesn't compile this arm at all, but the same code path having
   different warning behaviour per platform is exactly the kind of thing this
   project's "make it fail once" rule exists to catch -- so it is spelled out
   here once, portably, rather than patched per-platform. */
static Boolean hostterm_raw( int fd )
{
    struct termios t;

    if (tcgetattr( fd,&t )!=0) return false;

    t.c_iflag &= (tcflag_t)~( IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON );
    t.c_oflag &= (tcflag_t)~( OPOST );
    t.c_lflag &= (tcflag_t)~( ECHO|ECHONL|ICANON|ISIG|IEXTEN );
    t.c_cflag &= (tcflag_t)~( CSIZE|PARENB );
    t.c_cflag |=             ( CS8 );

    /* CLOCAL: do not wait on carrier detect. CREAD: enable the receiver.
       Both are no-ops for a pty and both matter for a real serial port, so
       they are set here rather than in a later serial-only change. */
    t.c_cflag |= CLOCAL | CREAD;

    /* Return immediately with whatever is there. The emulator polls; it must
       never block inside a read, because the scheduler is cooperative and a
       blocked read stalls every other OS-9 process. */
    t.c_cc[VMIN ]= 0;
    t.c_cc[VTIME]= 0;

    return tcsetattr( fd,TCSANOW, &t )==0;
} /* hostterm_raw */

os9err hostterm_open( int term_id, syspath_typ* spP )
{
    hostterm_typ* h;
    char*         spec;
    int           fd;

    hostterm_init();
    if (!hostterm_in_range( term_id )) return os9error(E_UNIT);

    h= &hostterms[ term_id ];
    if (h->open) { h->dev.spP= spP; return 0; } /* already open: share it */

    spec= hostterm_spec( term_id );
    if (spec==NULL) return os9error(E_UNIT);

    if (*spec!=PATHDELIM) {
        /* "pty" arrives in Task 5; anything else is simply not an endpoint.
           Refuse loudly -- a mistyped OS9T1 must not look like it worked. */
        uphe_printf( "OS9T%d: unsupported endpoint '%s'\n", term_id, spec );
        return os9error(E_UNIT);
    }

    /* O_NONBLOCK at open, and kept: it skips the carrier-detect wait a real
       serial port would otherwise impose, and it is what keeps every later
       read non-blocking. O_NOCTTY: this must never become our controlling
       terminal, which would route the host's job-control signals here. */
    fd= open( spec, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (fd<0) {
        uphe_printf( "OS9T%d: cannot open '%s'\n", term_id, spec );
        /* <fd> is -1: host2os9err's UNIX arm wants the POSIX RETURN CODE, not
           an errno -- it reads errno itself, and returns SUCCESS if handed 0.
           E_DEVBSY stays as the fallback for an errno it does not map. */
        return host2os9err( fd, E_DEVBSY );
    }

    if (!hostterm_raw( fd )) {
        uphe_printf( "OS9T%d: '%s' is not a terminal\n", term_id, spec );
        close( fd );
        return os9error(E_DEVBSY);
    }

    h->fd  = fd;
    h->open= true;
    strncpy( h->endpoint,spec, OS9PATHLEN-1 );
             h->endpoint[      OS9PATHLEN-1 ]= NUL;

    h->dev.installed = true;
    h->dev.inBufUsed =     0;
    h->dev.holdScreen= false;
    h->dev.pid       =     0;
    h->dev.spP       =   spP;

    debugprintf( dbgTerminal,dbgNorm,
                 ( "# hostterm: /t%d -> %s (fd %d)\n", term_id, spec, fd ) );
    return 0;
} /* hostterm_open */

void hostterm_close( int term_id )
{
    hostterm_typ* h;

    hostterm_init();
    if (!hostterm_in_range( term_id )) return;

    h= &hostterms[ term_id ];
    if (!h->open) return;

    close( h->fd );
    h->fd          =    -1;
    h->open        = false;
    h->dev.installed= false;
    h->dev.spP     =  NULL;
} /* hostterm_close */

int hostterm_put( int term_id, const char* buffer, int n )
{
    hostterm_typ* h;
    int           done= 0;
    int           tries= 0;

    hostterm_init();
    if (!hostterm_bound( term_id )) return -1;
    h= &hostterms[ term_id ];

    while (done<n) {
        ssize_t w= write( h->fd, buffer+done, (size_t)(n-done) );

        if (w>0) { done+= (int)w; tries= 0; continue; }

        if (w<0 && errno==EAGAIN) {
            /* Not also checking EWOULDBLOCK: os9exec_incl.h deliberately
               #undefs it (it becomes an OS-9 error constant instead), and
               POSIX guarantees the two are the same value wherever both
               exist, so EAGAIN alone already covers it.
               The far end is not draining. The fd is deliberately
               O_NONBLOCK -- blocking here would stall every OS-9 process,
               because the scheduler is cooperative -- so retry briefly and
               then report a write error rather than DROP the bytes.
               Silently dropping is the specific bug that shipped once
               already: a best-effort auto-LF push was discarded whenever the
               output FIFO was full, leaving bare-CR lines (fixed in 75a8ea8).
               Do not "improve" this into a discard.
               A proper fix parks the process in pWaitWrite exactly as the
               baud FIFO does (consio.c). Deferred: it needs the saved_cnt /
               saved_state resume dance, which is its own change. */
            if (++tries>10) break;
            usleep( 10000 ); /* 10ms x 10 = 100ms ceiling */
            continue;
        }

        if (w<0 && errno==EINTR) continue;
        return -1; /* genuine error: EIO on a hung-up pty, etc. */
    }

    return done<n ? -1 : done;
} /* hostterm_put */

void hostterm_poll( void )
{
    int id;

    hostterm_init();

    for (id=HOSTTERM_MIN; id<=HOSTTERM_MAX; id++) {
        hostterm_typ* h= &hostterms[ id ];
        int           room;

        if (!h->open) continue;

        /* Never read more than inBuf has guaranteed space for: KeyToBuffer
           silently drops once it is full, and a byte already taken off the fd
           cannot be pushed back. Same conservative floor HandleEvent() uses
           for stdin. Special chars do not consume inBuf space at all, so the
           worst case is deferring a few plain bytes to the next poll. */
        room= INBUFSIZE-1 - h->dev.inBufUsed;

        while (room-->0) {
            char c;
            if (read( h->fd,&c,1 )!=1) break; /* EAGAIN: nothing more today */
            KeyToBuffer( &h->dev, c );
        }
    }
} /* hostterm_poll */

int hostterm_get( int term_id, char* c )
{
    hostterm_typ* h;

    hostterm_init();
    if (!hostterm_bound( term_id )) return 0;
    h= &hostterms[ term_id ];

    hostterm_poll();
    if (h->dev.inBufUsed==0) return 0;

    *c= h->dev.inBuf[ 0 ];
    h->dev.inBufUsed--;
    if (h->dev.inBufUsed>0)
        MoveBlk( h->dev.inBuf, h->dev.inBuf+1, (ulong)h->dev.inBufUsed );

    return 1;
} /* hostterm_get */

Boolean hostterm_ready( int term_id, long* cnt )
{
    hostterm_typ* h;

    hostterm_init();
    *cnt= 0;
    if (!hostterm_bound( term_id )) return false;
    h= &hostterms[ term_id ];

    hostterm_poll();
    if (h->dev.inBufUsed==0) return false;

    *cnt= h->dev.inBufUsed;
    return true;
} /* hostterm_ready */

#else /* not UNIX, or MINGW: no termios, no pty */

os9err hostterm_open( int term_id, syspath_typ* spP )
{
    (void)spP;
    hostterm_init();
    (void)term_id;
    return os9error(E_UNIT);
} /* hostterm_open */

void hostterm_close( int term_id ) { (void)term_id; }

int hostterm_put( int term_id, const char* buffer, int n )
{
    (void)term_id; (void)buffer; (void)n;
    return -1; /* no host fd on this platform */
} /* hostterm_put */

int hostterm_get( int term_id, char* c )
{
    (void)term_id; (void)c;
    return 0; /* no host fd on this platform */
} /* hostterm_get */

Boolean hostterm_ready( int term_id, long* cnt )
{
    (void)term_id;
    *cnt= 0;
    return false; /* no host fd on this platform */
} /* hostterm_ready */

void hostterm_poll( void ) { } /* nothing to poll on this platform */

#endif

/* eof */
