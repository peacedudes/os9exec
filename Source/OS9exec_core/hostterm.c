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

/* glibc gates posix_openpt/grantpt/unlockpt/ptsname behind __USE_XOPEN2K(8),
   which needs _XOPEN_SOURCE/_POSIX_C_SOURCE/_GNU_SOURCE -- none of which this
   tree's Linux build defines. Same fix utilstuff.c already uses for
   realpath() (a different gate, __USE_MISC, but the same "must be defined
   before the first system header" constraint): must come before
   os9exec_incl.h or the include guard locks the declarations out. Harmless
   on Darwin, which declares all four unconditionally. */
#define _GNU_SOURCE

#include "os9exec_incl.h"

#if defined UNIX && !defined MINGW
  #include <errno.h>
  #include <fcntl.h>
  #include <stdlib.h>
  #include <termios.h>
  #include <unistd.h>
#endif

typedef struct {
    Boolean     open;       /* host fd is live */
    int         fd;         /* -1 when not open */
    int         spareFd;    /* self-allocated pty only; -1 otherwise -- see
                                its own comment in hostterm_open */
    int         openCount;  /* live OS-9 paths on this device; the endpoint
                                itself is never closed once bound -- see
                                hostterm_close */
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
        hostterms[k].spareFd    =    -1;
        hostterms[k].openCount  =     0;
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

    /* Fall back to the bare OS9T wildcard: "any /tN not named individually".
       Deliberately opt-in. Auto-allocating by default would turn a typo
       (">/t5" for ">/t4") into a terminal nobody is attached to, silently --
       and unlike /hx's fallback, which DISCOVERS a directory that already
       exists, this CREATES a resource, so it can never fail to find anything
       and can never tell you that you were wrong. Real OS-9 has no device
       without a descriptor; E_UNIT stays the default because it is the
       faithful answer. */
    if (v==NULL) {
        v= getenv( "OS9T" );
        if (v!=NULL && *v==NUL) v= NULL;
    }

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
    if (h->open) { h->openCount++; h->dev.spP= spP; return 0; } /* another holder */

    spec= hostterm_spec( term_id );
    if (spec==NULL) return os9error(E_UNIT);

    if (ustrcmp( spec,"pty" )==0) {
        char* slave;
        int   spareFd;

        fd= posix_openpt( O_RDWR | O_NOCTTY );
        if (fd<0 || grantpt( fd )!=0 || unlockpt( fd )!=0) {
            uphe_printf( "OS9T%d: cannot allocate a pty\n", term_id );
            if (fd>=0) close( fd );
            return os9error(E_DEVBSY);
        }

        slave= ptsname( fd );
        if (slave==NULL) {
            uphe_printf( "OS9T%d: pty has no slave name\n", term_id );
            close( fd );
            return os9error(E_DEVBSY);
        }

        /* Open our own reference to the slave and hold it for the life of
           this binding. Verified necessary on macOS, not cosmetic:
             1. tcgetattr on the bare MASTER from posix_openpt fails ENOTTY
                ("Inappropriate ioctl for device") until the slave has been
                opened at least once -- so the hostterm_raw() call below,
                which runs on <fd> (the master) for both endpoint spellings,
                would otherwise fail every time for "pty" and this whole
                endpoint would silently never work. A pty's termios is one
                struct shared by both ends (confirmed live: setting raw mode
                via the master after this open is visible reading it back
                from the slave), so opening the slave here is enough to
                unlock it -- the raw-mode call itself still targets <fd>.
             2. Separately, once the LAST open reference to a pty's slave
                closes, any output not yet read by the far end is discarded
                on macOS, deterministically. Before whoever runs `screen`
                ever attaches, this spare reference is what keeps early
                output alive to be read once they do. */
        spareFd= open( slave, O_RDWR | O_NOCTTY );
        if (spareFd<0) {
            uphe_printf( "OS9T%d: cannot open pty slave '%s'\n", term_id, slave );
            close( fd );
            return host2os9err( spareFd, E_DEVBSY );
        }
        h->spareFd= spareFd;

        /* The name is the whole point -- without it there is no way to
           attach. stderr via uphe_printf, so it survives stdout redirection
           and matches the emulator's other "# ..." startup messages. */
        uphe_printf( "/t%d is %s   (attach with: screen %s)\n",
                     term_id, slave, slave );

        /* We hold the MASTER. O_NONBLOCK is set here rather than at
           posix_openpt because the grant/unlock dance wants the plain fd. */
        fcntl( fd, F_SETFL, fcntl( fd,F_GETFL,0 ) | O_NONBLOCK );
        strncpy( h->endpoint,slave, OS9PATHLEN-1 );
                 h->endpoint[       OS9PATHLEN-1 ]= NUL;
    }
    else if (*spec==PATHDELIM) {
        /* O_NONBLOCK at open, and kept: it skips the carrier-detect wait a
           real serial port would otherwise impose, and it is what keeps
           every later read non-blocking. O_NOCTTY: this must never become
           our controlling terminal, which would route the host's
           job-control signals here. */
        fd= open( spec, O_RDWR | O_NOCTTY | O_NONBLOCK );
        if (fd<0) {
            uphe_printf( "OS9T%d: cannot open '%s'\n", term_id, spec );
            /* <fd> is -1: host2os9err's UNIX arm wants the POSIX RETURN
               CODE, not an errno -- it reads errno itself, and returns
               SUCCESS if handed 0. E_DEVBSY stays as the fallback for an
               errno it does not map. */
            return host2os9err( fd, E_DEVBSY );
        }
        strncpy( h->endpoint,spec, OS9PATHLEN-1 );
                 h->endpoint[      OS9PATHLEN-1 ]= NUL;
    }
    else {
        /* Neither spelling: refuse loudly -- a mistyped OS9T1 must not look
           like it worked. */
        uphe_printf( "OS9T%d: unsupported endpoint '%s'\n", term_id, spec );
        return os9error(E_UNIT);
    }

    if (!hostterm_raw( fd )) {
        uphe_printf( "OS9T%d: '%s' is not a terminal\n", term_id, h->endpoint );
        close( fd );
        if (h->spareFd>=0) { close( h->spareFd ); h->spareFd= -1; }
        return os9error(E_DEVBSY);
    }

    h->fd       = fd;
    h->open     = true;
    h->openCount=    1;

    h->dev.installed = true;
    h->dev.inBufUsed =     0;
    h->dev.holdScreen= false;
    h->dev.pid       =     0;
    h->dev.spP       =   spP;

    debugprintf( dbgTerminal,dbgNorm,
                 ( "# hostterm: /t%d -> %s (fd %d)\n", term_id, h->endpoint, fd ) );
    return 0;
} /* hostterm_open */

void hostterm_close( int term_id )
{
    hostterm_typ* h;

    hostterm_init();
    if (!hostterm_in_range( term_id )) return;

    h= &hostterms[ term_id ];
    if (!h->open) return;

    if (h->openCount>0) h->openCount--;
    if (h->openCount>0) return;          /* another path still holds it */

    /* Deliberately does NOT close h->fd or h->spareFd. A bound terminal is a
       DEVICE, and a device does not cease to exist because the last path to it
       closed -- real OS-9 keeps its descriptor: I$Attach maintains a use count
       per device-table entry, and I$Detach only tears TERM/storage down when
       that count reaches zero, exactly the shape restored here (os9-dev and
       os9-systems-dev skills, memory-and-io.md / device-drivers.md). Two
       concrete reasons this matters for hostterm specifically: a
       self-allocated pty would otherwise hand out a DIFFERENT name on the
       next open, killing whatever `screen` was attached to the old one; and
       closing the master discards bytes the peer has not read yet (the mirror
       of the slave-side discard already documented in this file's plan).
       The fd is released when the emulator exits, which is when the device
       genuinely goes away. */
    h->dev.spP= NULL; /* the syspath is going away; do not keep a stale pointer */
} /* hostterm_close */

int hostterm_put( int term_id, const char* buffer, int n )
{
    hostterm_typ* h;
    ssize_t       w;

    hostterm_init();
    if (!hostterm_bound( term_id )) return -1;
    h= &hostterms[ term_id ];

    do { w= write( h->fd, buffer, (size_t)n ); } while (w<0 && errno==EINTR);

    /* 0 means "would block": the far end has not drained. NOT an error and
       NOT a licence to discard -- the caller (ConsoleOut) parks the writing
       process instead, exactly as the baud FIFO does, and retries from here
       on resume. Retrying in a loop HERE would stall every other OS-9
       process, because the scheduler is cooperative -- that loop used to
       exist, capped at 100ms, and then gave up and returned -1, which the
       original call site ignored: the exact silent-discard bug this
       function's contract exists to prevent (a best-effort auto-LF push
       discarded whenever the output FIFO was full, leaving bare-CR lines,
       fixed in 75a8ea8). Do not reintroduce a retry loop here. */
    if (w<0 && errno==EAGAIN) return 0;
    if (w<0)                  return -1; /* genuine error: EIO on a hung-up pty, etc. */

    return (int)w;
} /* hostterm_put */

void hostterm_poll( void )
{
    int id;

    hostterm_init();

    for (id=HOSTTERM_MIN; id<=HOSTTERM_MAX; id++) {
        hostterm_typ* h= &hostterms[ id ];
        int           room;

        /* dev.spP is NULL between the last path close and any reopen (the
           device stays open per hostterm_close above, but no syspath is
           bound to it) -- KeyToBuffer dereferences mco->spP unconditionally
           (utilstuff.c:1034, no NULL check there), so skip a device with no
           current path rather than crash on its next byte. */
        if (!h->open || h->dev.spP==NULL) continue;

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

/* OS-9 rates that have a POSIX B-constant. 2000, 3600 and 7200 appear in
   OS-9's own table (baud_bps, consio.c) and have no POSIX equivalent, so they
   land in the default arm: leave the port at whatever it was rather than
   silently substituting a different speed, which would be worse than not
   setting one at all. */
static speed_t hostterm_speed( ulong bps )
{
    switch (bps) {
        case     50: return B50;
        case     75: return B75;
        case    110: return B110;
        case    134: return B134;
        case    150: return B150;
        case    300: return B300;
        case    600: return B600;
        case   1200: return B1200;
        case   1800: return B1800;
        case   2400: return B2400;
        case   4800: return B4800;
        case   9600: return B9600;
        case  19200: return B19200;
        case  38400: return B38400;
        case  57600: return B57600;
        case 115200: return B115200;
        default    : return 0;
    }
} /* hostterm_speed */

void hostterm_setspeed( int term_id, ulong bps )
{
    hostterm_typ*  h;
    struct termios t;
    speed_t        sp;

    hostterm_init();
    if (!hostterm_bound( term_id )) return;

    sp= hostterm_speed( bps );
    if (sp==0) return; /* no POSIX equivalent: leave the port alone */

    h= &hostterms[ term_id ];
    if (tcgetattr  ( h->fd,&t   )!=0) return;
    if (cfsetispeed( &t,   sp   )!=0) return;
    if (cfsetospeed( &t,   sp   )!=0) return;

    tcsetattr( h->fd,TCSANOW, &t );

    debugprintf( dbgTerminal,dbgNorm,
                 ( "# hostterm: /t%d speed %lu\n", term_id, bps ) );
} /* hostterm_setspeed */

void hostterm_note_writer( int term_id )
{
    hostterm_typ* h;

    hostterm_init();
    if (!hostterm_bound( term_id )) return;

    h= &hostterms[ term_id ];
    if (h->dev.spP==NULL) return; /* lw_pid dereferences spP unconditionally */

    lw_pid( &h->dev );
} /* hostterm_note_writer */

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

void hostterm_setspeed( int term_id, ulong bps ) { (void)term_id; (void)bps; }

void hostterm_note_writer( int term_id ) { (void)term_id; }

#endif

/* eof */
