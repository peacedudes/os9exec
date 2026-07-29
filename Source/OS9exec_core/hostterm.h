//
//    OS9exec, OS-9 emulator for Mac OS, Windows and Linux
//
//    Host-backed terminal devices: /t1../t49 bound to a real host endpoint.
//

#ifndef HOSTTERM_H
#define HOSTTERM_H

/* /t0 is the main console (term_id 0); /t50 and up are VMod territory
   (VModBase). So the host-backed range is 1..VModBase-1. */
#define HOSTTERM_MIN  1
#define HOSTTERM_MAX  (VModBase-1)

/* Is <term_id> inside the host-backed range at all? */
Boolean hostterm_in_range ( int term_id );

/* Is an OS9T<n> environment variable set for this device?
   Says nothing about whether it is currently open. */
Boolean hostterm_configured( int term_id );

/* Is this device currently open on a host fd? */
Boolean hostterm_bound    ( int term_id );

/* Open the configured endpoint and bind it to <spP>.
   Returns 0, or an os9err (E_UNIT if unconfigured or the spelling is
   unsupported, E_DEVBSY if the host refuses). */
os9err  hostterm_open     ( int term_id, syspath_typ* spP );

/* Close and release the host fd. Safe on an unbound device. */
void    hostterm_close    ( int term_id );

/* Write <n> bytes. Returns bytes written, or -1 on a genuine error. */
int     hostterm_put      ( int term_id, const char* buffer, int n );

/* Take one buffered byte. Returns 1 if <c> was filled, 0 if none ready. */
int     hostterm_get      ( int term_id, char* c );

/* How many bytes are buffered for this device? */
Boolean hostterm_ready    ( int term_id, long* cnt );

/* Drain every bound host fd into its input buffer. Called from
   CheckInputBuffers(), alongside the tty and main-console sweeps. */
void    hostterm_poll     ( void );

#endif /* HOSTTERM_H */

/* eof */
