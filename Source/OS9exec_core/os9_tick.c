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
 * KNOWN LIMITATION, 2026-07-19 -- root cause identified, not yet fixed.
 *
 * With the tick on, 127 of 129 suite tests pass; all 129 pass with it off,
 * which is the default. What fails is not what it first looks like: the
 * F$STrap tests fail because <r68>, the assembler they build their test
 * program with, dies of a bus error part-way through -- a wild address in a
 * register -- so the program under test is never produced. It is CPU state
 * corruption, not anything to do with exception handling.
 *
 * The cause is that llm_os9_go() was only ever entered at a syscall or
 * exception boundary. Pre-emption re-enters it in the MIDDLE of an
 * instruction stream, which the round trip through regs_type does not fully
 * survive. Bisected:
 *   - switching processes is NOT the trigger: making the tick leave the loop
 *     without arbitrating at all made it fail 6/6 rather than 3/6, so it is
 *     the leave-and-re-enter itself.
 *   - regs.prefetch is one identified component. Calling fill_prefetch_0()
 *     after MakeFromSR() in llm_os9_go() took one failing test from 0/6 to
 *     6/6 and another from 0/6 to 3/6 -- so it is real, and it is partial.
 *     Not left in: llm_os9_go() runs for every syscall, and a partial fix in
 *     the emulation core is not worth the risk while -q is off by default.
 *
 * Whoever picks this up: the remaining state is somewhere else in that same
 * round trip. Compare regs before and after a leave/re-enter with no
 * instructions executed in between -- anything that differs is a candidate.
 *
 * Starting it is tied to -q and nothing else. An earlier version also waited
 * for the guest to set the time, on the grounds that a real clock starts at
 * <setime> -- but that made -q silently do nothing on its own, which cost a
 * whole run of the test suite reporting that it "passed with pre-emption"
 * while the tick had never once started. Faithfulness that can be mistaken
 * for a working feature is worse than a switch that means what it says.
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
    if (os9_tick_request==0) return; /* not asked for */
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
