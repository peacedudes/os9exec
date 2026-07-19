/**********************************************/
/*             O S 9 E x e c / NT             */
/*  Cooperative-Multiprocess OS-9 emulation   */
/*         for Apple Macintosh and PC         */
/**********************************************/

/* The system tick.
 *
 * OS9exec has always run a process until it traps or faults: nothing takes the
 * CPU away from one between two of its own instructions. That is what the
 * "Cooperative" in the banner above means, and it has consequences a program
 * written for a real machine would not expect -- a loop that makes no system
 * calls owns the machine until it finishes, a cyclic alarm cannot reach a
 * process that is computing, and two processes can never interleave inside a
 * read-modify-write, so a lost update is not merely unlikely here, it is
 * impossible. Real OS-9 has a clock interrupt and none of that holds.
 *
 * Enabling this (-q) puts the clock back. A one-shot host timer re-arms itself
 * on each expiry, so exactly one is ever outstanding -- never none, never two,
 * which an interval timer cannot promise if a handler is ever delayed. The
 * handler does nothing except record what happened and clear <os9_running>;
 * the emulation loop's own "while (os9_running)" is what stops, so nothing is
 * added to the innermost loop of the emulator to pay for this.
 *
 * Following what the clock does on a real machine: the timed-out flag is set
 * whether or not it can be acted on, and it is acted on only in user state.
 * A tick that lands in system state leaves the flag pending, and the switch
 * happens on the way back out to user state -- so a system call is never cut
 * in half, and the switch is deferred rather than lost.
 *
 * The clock does not run until the guest sets the time, matching a real
 * system where the clock starts at <setime>: see os9_tick_start().
 */

#include "os9exec_incl.h"
#include <signal.h>          /* sig_atomic_t is needed on every platform */

#if defined UNIX && !defined MINGW
  #include <sys/time.h>      /* setitimer: the tick itself is UNIX-only */
#endif

/* Lives in the emulation core (newcpu.c), which this file does not include:
 * clearing <os9_running> is what stops the emulation loop. */
extern volatile sig_atomic_t os9_running;
extern int os9_tick_us;   /* live interval; also "is the clock running" */
extern int os9_timed_out; /* set by the tick, cleared once acted upon */

#define TICK_US_DEFAULT 10000 /* 100Hz, the OS-9/68k default tick rate */

int  os9_tick_request= 0; /* microseconds asked for on the command line, 0=off */

#if defined UNIX && !defined MINGW

static void os9_tick_arm( void );

static void os9_tick_handler( int sig )
/* Nothing may be done here that is not safe from a signal: record it and go.
 * Clearing <os9_running> is what actually stops the emulation, at whatever
 * instruction boundary it reaches next. */
{
    (void)sig;
    os9_timed_out= 1;
    os9_running  = 0;
    os9_tick_arm();  /* one-shot, re-armed: exactly one outstanding */
} /* os9_tick_handler */

static void os9_tick_arm( void )
{
    struct itimerval it;

    it.it_interval.tv_sec = 0; /* deliberately NOT an interval timer */
    it.it_interval.tv_usec= 0;
    it.it_value.tv_sec    = 0;
    it.it_value.tv_usec   = os9_tick_us;

    signal    ( SIGALRM, os9_tick_handler );
    setitimer ( ITIMER_REAL, &it, NULL );
} /* os9_tick_arm */

void os9_tick_start( void )
/* Start the clock, once. Called when the guest first sets the time, so a
 * system that never does simply never has one -- and calling it again is
 * harmless, which matters because <setime> can be run more than once. */
{
    if (os9_tick_request==0) return; /* not asked for */
    if (os9_tick_us       !=0) return; /* already running */

    os9_tick_us= os9_tick_request;
    os9_tick_arm();
} /* os9_tick_start */

void os9_tick_stop( void )
/* Take the clock away again -- for anything that must not be interrupted at an
 * arbitrary instruction, and so a host debugger session is not walked on. */
{
    struct itimerval it;

    if (os9_tick_us==0) return;

    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec= 0;
    it.it_value.tv_sec    = 0;
    it.it_value.tv_usec   = 0;
    setitimer( ITIMER_REAL, &it, NULL );

    os9_tick_us  = 0;
    os9_timed_out= 0;
} /* os9_tick_stop */

#else

/* No tick on platforms without setitimer: OS9exec behaves as it always has. */
void os9_tick_start( void ) {}
void os9_tick_stop ( void ) {}

#endif

int os9_tick_default_us( void ) { return TICK_US_DEFAULT; }
