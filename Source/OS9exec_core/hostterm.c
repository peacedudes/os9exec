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

os9err hostterm_open( int term_id, syspath_typ* spP )
{
    #ifndef __GNUC__
    #pragma unused( term_id,spP )
    #endif
    (void)spP;
    hostterm_init();
    return os9error(E_UNIT); /* filled in by Task 2 */
} /* hostterm_open */

void hostterm_close( int term_id )
{
    #ifndef __GNUC__
    #pragma unused( term_id )
    #endif
    (void)term_id;
} /* hostterm_close */

int hostterm_put( int term_id, const char* buffer, int n )
{
    #ifndef __GNUC__
    #pragma unused( term_id,buffer,n )
    #endif
    (void)buffer; (void)n; (void)term_id;
    return -1; /* filled in by Task 3 */
} /* hostterm_put */

int hostterm_get( int term_id, char* c )
{
    #ifndef __GNUC__
    #pragma unused( term_id,c )
    #endif
    (void)c; (void)term_id;
    return 0; /* filled in by Task 4 */
} /* hostterm_get */

Boolean hostterm_ready( int term_id, long* cnt )
{
    #ifndef __GNUC__
    #pragma unused( term_id )
    #endif
    (void)term_id;
    *cnt= 0;
    return false; /* filled in by Task 4 */
} /* hostterm_ready */

void hostterm_poll( void )
{
    /* filled in by Task 4 */
} /* hostterm_poll */

/* eof */
