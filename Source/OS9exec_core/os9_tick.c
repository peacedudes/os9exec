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
 * This puts the clock back, and since 2026-07-20 it is ON BY DEFAULT: a machine
 * with a clock is what OS-9 is, so that is what OS9exec should be out of the
 * box. "-q0" restores the old cooperative behaviour, "-q<ms>" retunes the rate.
 *
 * A one-shot host timer re-arms itself on each expiry, so exactly one is ever
 * outstanding -- never none, never two, which an interval timer cannot promise
 * if a handler is ever delayed. The handler does nothing except record what
 * happened and clear <os9_running>; the emulation loop's own
 * "while (os9_running)" is what stops, so nothing is added to the innermost
 * loop of the emulator to pay for this.
 *
 * A SYSTEM CALL IS NEVER CUT IN HALF -- but not for the reason the code below
 * appears to give, and the difference matters to anyone changing this.
 *
 * The rule being honoured is OS-9's: a tick cannot pre-empt a process inside a
 * system request (file manager, driver); it may wake a sleeping system process
 * and pre-empt a USER-state process to run it. User state is guaranteed
 * nothing and may be switched anywhere.
 *
 * OS9exec gets the first half of that for free, structurally. A system request
 * here is not emulated 68k code at all: it traps OUT of the emulator and runs
 * as host C, while the emulation loop is not executing. The tick handler only
 * sets a flag, and the switch is decided at the loop boundary -- which is only
 * ever reached with the guest in user state. So the un-interruptible window is
 * enforced by the architecture, not by a test.
 *
 * m68k_os9go()'s "if (regs.s) defer" therefore NEVER FIRES: measured over a
 * full suite run, 167 of 167 ticks had s=0, and nothing in OS9exec ever sets
 * the supervisor bit for guest execution. It is kept as an honest guard in
 * case emulated supervisor code is ever run -- but do not read it as the thing
 * that protects system calls, and do not "fix" a bug by tightening it.
 *
 * Nothing gates starting it except the requested rate being non-zero. An
 * earlier version also waited for the guest to set the time, on the grounds
 * that a real clock starts at <setime> -- but that made -q silently do nothing
 * on its own, which cost a whole run of the test suite reporting that it
 * "passed with pre-emption" while the tick had never once started.
 * Faithfulness that can be mistaken for a working feature is worse than a
 * switch that means what it says.
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

/* Microseconds per tick; 0 = no clock. ON BY DEFAULT since 2026-07-20 -- a
 * machine with a clock is what OS-9 actually is, and without one a dead loop
 * hogs the emulator and a cyclic alarm can never reach a computing process.
 * "-q0" turns it off for anyone who needs the old cooperative behaviour. */
int  os9_tick_request= TICK_US_DEFAULT;

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
    struct sigaction sa;

    /* SA_RESTART asked for explicitly rather than inherited: a signal 100
     * times a second interrupts blocking host calls, and nothing in OS9exec
     * checks for EINTR anywhere. Today that is harmless -- the only blocking
     * calls on the hot paths are idle nanosleeps, which simply return early --
     * but relying on signal()'s implementation-defined behaviour to keep it
     * that way is luck, not design. Let the kernel restart them. */
    sa.sa_handler= os9_tick_handler;
    sigemptyset( &sa.sa_mask );
    sa.sa_flags  = SA_RESTART;
    sigaction   ( SIGALRM, &sa, NULL );

    it.it_interval.tv_sec = 0; /* deliberately NOT an interval timer */
    it.it_interval.tv_usec= 0;
    it.it_value.tv_sec    = 0;
    it.it_value.tv_usec   = os9_tick_us;
    setitimer   ( ITIMER_REAL, &it, NULL );
} /* os9_tick_arm */

void os9_tick_start( void )
/* Start the clock, once. Calling it again is harmless. */
{
    if (os9_tick_request==0) return; /* clock switched off with -q0 */
    if (os9_tick_us       !=0) return; /* already running */

    os9_tick_us= os9_tick_request;
    os9_tick_arm();
} /* os9_tick_start */

void os9_tick_stop( void )
/* Take the clock away again. Nothing calls this, deliberately.
 *
 * A host debugger does not need it: because the timer is a one-shot re-armed
 * inside its own handler, stopping the process stops the clock by itself --
 * nothing re-arms while nothing runs, and standard signals do not queue, so
 * however long you sit at a breakpoint there is at most ONE pending SIGALRM
 * waiting. Resuming costs a single spurious task switch and the cadence picks
 * up again. Nor is there anything guest-visible to go stale meanwhile: no
 * interrupt is faked, nothing is stacked, no vector taken -- the tick only
 * tells OS9exec's own dispatcher to arbitrate.
 *
 * So this exists for something that genuinely must not be interrupted at an
 * arbitrary instruction, and is left uncalled because a stop that is never
 * paired with a start is a real bug, while not calling it cannot be. */
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
