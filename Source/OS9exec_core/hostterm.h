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

/* Release one PATH on this device -- NOT the endpoint. Decrements the open
   count; the host fd (and a self-allocated pty's slave reference) deliberately
   stay open until the emulator exits, so the device a user attached to remains
   the same device between commands, and so closing cannot discard bytes the
   far end has not read yet. Matches OS-9's own model, where a device
   descriptor outlives any single path to it. Safe on an unbound device. */
void    hostterm_close    ( int term_id );

/* Write <n> bytes, without blocking. Returns the count written; 0 if the
   endpoint would block (its buffer is full and the far end has not drained);
   -1 on a genuine error or an unbound device.
   0 is NOT an error and NOT a licence to discard: the caller must park the
   writing process (ConsoleOut uses pWaitWrite) and retry the same bytes. This
   function deliberately does not retry internally -- the scheduler is
   cooperative, so sleeping here would stall every other OS-9 process. */
int     hostterm_put      ( int term_id, const char* buffer, int n );

/* Take one buffered byte. Returns 1 if <c> was filled, 0 if none ready. */
int     hostterm_get      ( int term_id, char* c );

/* How many bytes are buffered for this device? */
Boolean hostterm_ready    ( int term_id, long* cnt );

/* Drain every bound host fd into its input buffer. Called from
   CheckInputBuffers(), alongside the tty and main-console sweeps. */
void    hostterm_poll     ( void );

/* Apply <bps> (bits per second, already resolved from the path's PD_BAU
   option code) to the bound host fd. A no-op if the device is not bound,
   if <bps> has no POSIX B-constant (2000/3600/7200 -- OS-9's own table has
   no equivalent), or on any platform without termios. A pty stores the
   speed without honouring it: this is provable against a pty but not
   against a real serial line. */
void    hostterm_setspeed ( int term_id, ulong bps );

/* Record the process that just wrote to <term_id> as that terminal's
   last writer, so an abort character typed ON that terminal has a target.
   KeyToBuffer reads the target from the device's own syspath
   (`lastwritten_pid`), and only lw_pid() ever sets it -- so without this the
   abort char still flushed the terminal's paced backlog (losing queued output)
   while signalling nobody, and the process it was meant to interrupt ran to
   completion. Lives here because `hostterms[]`, and thus each device's
   ttydev_typ, is private to hostterm.c. No-op if the device is not bound, has
   no syspath, or on any platform where hostterm is a stub. */
void    hostterm_note_writer( int term_id );

/* Is output to <term_id> currently held by an XOFF typed on it? False for a
   device that is not bound -- that terminal's output falls back to the main
   console, whose own hold is the one that applies. Same reason as
   hostterm_note_writer: `hostterms[]` is private to hostterm.c. */
Boolean hostterm_held( int term_id );

#endif /* HOSTTERM_H */

/* eof */
