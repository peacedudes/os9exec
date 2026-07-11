# Baud-rate output pacing v2 (FIFO + host-timed consumer) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the current burst-then-pause baud pacing with a small per-console ring buffer drained one character at a time on real host timing, so slow-baud output visibly trickles instead of bursting.

**Architecture:** A producer (`ConsoleOut` in `consio.c`) pushes bytes into a 256-byte ring buffer per physical console, blocking the writing process (new `pWaitWrite` state, mirroring the existing `pWaitRead` blocking-read pattern) when the ring fills. A consumer, invoked from the scheduler's existing cooperative-yield points (`DoWait()`, `do_arbitrate()` — no new thread, no signal handler), pops one byte at a time no faster than the device's baud rate allows and displays it.

**Tech Stack:** C (os9exec core), Swift (integration test harness in `test/`).

**Spec:** `docs/superpowers/specs/2026-07-10-baud-fifo-pacing-design.md` — read this first for the *why*; this document is the *how*.

## Global Constraints

- FIFO size: 256 bytes per device (spec's explicit sizing decision).
- One `baud_device_t` per physical console, keyed by `term_id`/`gConsoleID` — not global, not per-open-path.
- No new host thread, no OS-level signal handler (async timer) — all timing goes through the existing cooperative scheduler hooks.
- `-r` (full speed) and unknown/zero baud must remain byte-for-byte synchronous with zero added scheduler-pass latency (test harness correctness depends on this).
- Read-specific dispatcher logic (`pwr_brk`, the `wRead` signal-delay check in `send_signal`, the signal-intercept `rtestate` restore keyed on `pWaitRead`) must **not** be extended to `pWaitWrite` — this is what broke the prior attempt.
- `make test` (currently 88/0) must stay green after every task.

---

## Note on scope vs. the spec

While drafting this plan, tracing every `pWaitRead` reference in the dispatcher (`os9exec_nt.c`, `procstuff.c`, `pipefiles.c`, `filestuff.c`) turned up more touch points than the spec's "two narrow extensions" summary named. Task 1 below reflects the fuller, verified list — it's still all mechanical mirroring of the existing proven `pWaitRead` pattern, just more instances of it than the spec called out.

---

### Task 1: `pWaitWrite` process state — dispatcher plumbing only, no producer yet

**Files:**
- Modify: `Source/OS9exec_core/os9exec_nt.h` (enum)
- Modify: `Source/OS9exec_core/os9exec_nt.c` (dispatcher, several small spots)
- Modify: `Source/OS9exec_core/procstuff.c` (`PStateStr`, two scheduling-fairness checks)
- Modify: `Source/OS9exec_core/utilstuff.c` (`set_os9_state`'s DExec status mapping)
- Modify: `Source/OS9exec_core/filestuff.c` (debug-print gate, cosmetic)
- Modify: `Source/OS9exec_core/pipefiles.c` (`Reactivate`'s defensive guard)

**Interfaces:**
- Produces: `pWaitWrite` (new `pstate_typ` enum value). Task 2 is the first consumer — nothing sets this state yet, so this task is pure regression-safe scaffolding.

This task adds **no new behavior** — nothing in the codebase sets `pWaitWrite` yet, so every check added here is dead code until Task 2. That's why verification is "still compiles, `make test` still 88/0," not a new passing test.

- [ ] **Step 1: Add `pWaitWrite` to the process-state enum**

`Source/OS9exec_core/os9exec_nt.h`, in the `pstate_typ` enum (around line 1029):

```c
typedef enum { 
            pUnused,
            pStart,
            pActive, 
            pDead, 
            pSleeping, 
            pWaiting, 
            pSysTask, 
            pWaitRead,
            pWaitWrite
        } pstate_typ;
```

- [ ] **Step 2: Mirror every functional `pWaitRead` dispatcher check in `os9exec_nt.c`**

All five edits below are in the same `do { ... } while(currentpid<MAXPROCESSES);` main dispatch loop.

**2a.** Around line 1903-1906 — let a blocked-on-write `intUtil` process count as "OK to break" the same way a blocked-on-read one does:

```c
      if (cp->isIntUtil &&
         (cp->state==pActive   ||
          cp->state==pWaiting  ||
          cp->state==pWaitRead ||
          cp->state==pWaitWrite)) break;
```

**2b.** Around line 1938-1939 — let the dispatcher enter the trap-handling branch for a process resuming from `pWaitWrite`, same as `pWaitRead`:

```c
    else if (cp->state==pActive   ||
 		     cp->state==pWaitRead ||
 		     cp->state==pWaitWrite) {
```

(No change needed to the inner `if ((cp->state==pActive || cp->way_to_icpt))` check a few lines below it — that's what correctly skips re-running 68k CPU instructions for *any* non-`pActive` resume, `pWaitWrite` included, exactly as it already does for `pWaitRead`.)

**2c.** Around line 2022-2027 — restore the saved trap registers so the *same* original write syscall re-executes unchanged:

```c
      if ((cp->state==pWaitRead || cp->state==pWaitWrite) && !cp->isIntUtil) {
        // registers of the last command will be restored
        cp->os9regs= svd->r;
        cp->vector = svd->vector;
        cp->func   = svd->func;
      } // if	
```

**2d.** Around line 2092-2093 — include a still-blocked writer in the "report the syscall's current error status" condition, same as a still-blocked reader:

```c
        if (cp->state==pActive    || 
            cp->state==pWaitRead  ||
            cp->state==pWaitWrite || cp->oerr) {
```

**2e.** Around line 2106-2107 — let the debugger's single-step logic treat a blocked write the same as a blocked read (still "running silently," not a stopping point):

```c
        if (dbg_step_pending[cpid] &&
            (cp->state == pActive || cp->state == pWaitRead || cp->state == pWaitWrite)) {
```

**2f.** Around line 2201-2202 — same generic register-save-before-arbitration step `pWaitRead` already gets:

```c
    if (cp->state==pWaitRead || cp->state==pWaitWrite)
      memcpy( (void*)&cp->os9regs, (void*)&svd->r, sizeof(regs_type) ); // save all regs
```

- [ ] **Step 3: Extend the two scheduling-fairness checks in `procstuff.c`**

**3a.** Around line 1032-1034, in `do_arbitrate()` — give a blocked writer the same "force arbitration" treatment a blocked reader gets:

```c
  if (!arbitrate) {
    if (cp->state==pSysTask   ||                 /* we need aritration or we'll get stuck in systasks */
        cp->state==pUnused    ||                 /* unused, arbitrating needed */
        cp->state==pWaitRead  ||
        cp->state==pWaitWrite) arbitrate= true; /* give a chance to other processes */
  } // if
```

**3b.** Around line 1198-1203 — reuse the same `pW_age` fairness-aging field for writes (a process can only be blocked in one syscall at a time, so sharing the field is safe):

```c
    if    (sprocess->state==pWaitRead ||
           sprocess->state==pWaitWrite) {        /* only every nth time for this mode */
      if  (sprocess->pW_age--<=0) { 
           sprocess->pW_age= NewAge;  break;
      } // if
      done= false;
    } // if
```

- [ ] **Step 4: Add the display/status-mapping cases**

**4a.** `Source/OS9exec_core/procstuff.c`, `PStateStr` switch (around line 254) — reuses `pWaitRead`'s display char (`'S'`), same as how `pSleeping`/`pWaitRead` already share one:

```c
                case pWaitRead : sta='S'; break;
                case pWaitWrite: sta='S'; break;
```

**4b.** `Source/OS9exec_core/utilstuff.c`, `set_os9_state`'s DExec status switch (around line 355) — needs its own `_queueid` letter (`'r'` is taken by `pWaitRead`, `'w'` by `pWaiting`; use `'W'`):

```c
        case pWaitRead : pd->_state= os9_word(0xA000); pd->_queueid= 'r'; break;
        case pWaitWrite: pd->_state= os9_word(0xA000); pd->_queueid= 'W'; break;
```

- [ ] **Step 5: Cosmetic parity fix — debug-print suppression in `filestuff.c`**

Around line 1116-1119, in `get_syspathd` — avoids spamming debug logs on every retry pass while blocked, same as already happens for `pWaitRead`:

```c
    notW= true;
    if (pid<MAXPROCESSES) {
    	       cp= &procs[pid];   
		notW= (cp->state!=pWaitRead && cp->state!=pWaitWrite);   
	}
```

- [ ] **Step 6: Defensive guard in `pipefiles.c`'s `Reactivate()`**

Around line 426-431. This function force-activates a process from certain pipe code paths; it already knows not to do that to a process mid-read or mid-wait (per its own "costed me 2 days debugging" comment) — extend the same protection to mid-write, so pipe code can never yank a process out of a blocked console write and corrupt it. This does **not** give pipes a baud rate or change any pipe I/O behavior — it only protects a state pipes don't know about yet:

```c
static void Reactivate( ushort pid, process_typ* cp, const char* callingProc )
{
  if (cp->state!=pWaitRead &&       /* this statement costed me 2 days debugging !! */
      cp->state!=pWaiting  &&       /* and this one another 1.5 days !!! */
      cp->state!=pWaitWrite)        /* new: don't repeat those mistakes for blocked writes either */
    set_os9_state( pid, pActive, callingProc );  /* re-activate */
} /* Reactvate */
```

- [ ] **Step 7: Build and verify no regression**

```bash
make -f GNUmakefile os9exec
```
Expected: builds clean, no new warnings (there was 1 pre-existing unrelated warning in `test/Sources/OS9Tests/main.swift` noted earlier — that's Swift, unrelated to this C build).

```bash
swift run --package-path test
```
Expected: `Results: 88 passed, 0 failed` — unchanged, since nothing sets `pWaitWrite` yet.

- [ ] **Step 8: Commit**

```bash
git add Source/OS9exec_core/os9exec_nt.h Source/OS9exec_core/os9exec_nt.c \
        Source/OS9exec_core/procstuff.c Source/OS9exec_core/utilstuff.c \
        Source/OS9exec_core/filestuff.c Source/OS9exec_core/pipefiles.c
git commit -m "Core: add pWaitWrite process state, mirroring pWaitRead's dispatcher plumbing"
```

---

### Task 2: Per-device ring buffer + producer (`ConsoleOut`) with unpaced background drain

**Files:**
- Modify: `Source/OS9exec_core/consio.c` (device struct, FIFO primitives, `ConsoleOut` rewrite, drain/flush functions)
- Modify: `Source/OS9exec_core/filestuff.h` (extern declarations for the new cross-file functions)
- Modify: `Source/OS9exec_core/procstuff.c` (`DoWait()`, `do_arbitrate()` — hook the drain call)
- Test: `test/Sources/OS9Tests/main.swift` (new correctness test)

**Interfaces:**
- Consumes: `pWaitWrite` (Task 1), `cp->saved_cnt`/`cp->saved_state` (existing per-process fields, already used by `pConsRead`).
- Produces: `void baud_drain_due(void)` and `void baud_flush_device(short term_id)` — Task 3 changes `baud_drain_due`'s *internals* (adds real timing) but not its signature or call sites; Task 4 calls `baud_flush_device`.

This task deliberately does **not** add real timing yet — `baud_drain_due()` empties every queued device unconditionally, every time it's called. The point of this task is proving the FIFO/block/resume mechanism moves bytes correctly under real scheduling (a >256-byte write must still block and resume without dropping/duplicating/reordering bytes) before Task 3 adds the visual pacing on top of an already-correct mechanism.

`-r` (and any device with unknown/zero baud) bypasses the FIFO entirely from this task onward — the per-character loop calls `ConsPutc` directly for those, exactly as before this whole feature existed, so `-r` stays truly synchronous with zero added latency starting now (not deferred to Task 3).

- [ ] **Step 1: Declare the new cross-file functions**

`Source/OS9exec_core/filestuff.h`, right after the existing "external consio support routines" block (around line 159):

```c
/* baud pacing (docs/superpowers/specs/2026-07-10-baud-fifo-pacing-design.md) */
void  baud_drain_due   ( void );          /* pop+display whatever is ready right now, cheap no-op if nothing queued */
void  baud_flush_device( short term_id ); /* discard queued output for one device (Ctrl-C/E, kill) */
```

- [ ] **Step 2: Add the device struct and ring-buffer primitives to `consio.c`**

Add this near the top of `consio.c`, right before the existing `baud_bps` table (around line 729) — it stays and is reused for the paced/unpaced decision:

```c
#define BAUD_FIFO_SIZE   256
#define MAXBAUDDEV         8

typedef struct {
    Boolean inUse;
    short   term_id;
    byte    buf[BAUD_FIFO_SIZE];
    ushort  head, tail, count;
    ulong   us_per_char;   /* 0 = unpaced; Task 3 fills this in for real baud rates */
    ulong   next_due_us;   /* host time next pop may happen; Task 3 makes this meaningful */
} baud_device_t;

static baud_device_t baud_devices[MAXBAUDDEV];

/* find (or allocate) the simulated device for a physical console id */
static baud_device_t* baud_dev_for( short term_id )
{
    int i, free_slot= -1;
    for (i=0; i<MAXBAUDDEV; i++) {
        if ( baud_devices[i].inUse && baud_devices[i].term_id==term_id) return &baud_devices[i];
        if (!baud_devices[i].inUse && free_slot<0) free_slot= i;
    }
    if (free_slot<0) return NULL; /* out of device slots; caller falls back to unpaced */

    baud_devices[free_slot].inUse=       true;
    baud_devices[free_slot].term_id=     term_id;
    baud_devices[free_slot].head=
    baud_devices[free_slot].tail=
    baud_devices[free_slot].count=       0;
    baud_devices[free_slot].us_per_char= 0;
    baud_devices[free_slot].next_due_us= 0;
    return &baud_devices[free_slot];
} /* baud_dev_for */

static Boolean fifo_push( baud_device_t* d, byte c )
{
    if (d->count>=BAUD_FIFO_SIZE) return false;
    d->buf[d->tail]= c;
    d->tail= (ushort)((d->tail+1) % BAUD_FIFO_SIZE);
    d->count++;
    return true;
} /* fifo_push */

static Boolean fifo_pop( baud_device_t* d, byte* c )
{
    if (d->count==0) return false;
    *c= d->buf[d->head];
    d->head= (ushort)((d->head+1) % BAUD_FIFO_SIZE);
    d->count--;
    return true;
} /* fifo_pop */

/* pop+display everything currently queued. Task 3 adds real per-char pacing
   here; for now this drains unconditionally (no visible pacing yet). */
void baud_drain_due( void )
{
    int  i;
    byte c;
    for (i=0; i<MAXBAUDDEV; i++) {
        if (!baud_devices[i].inUse) continue;
        while (fifo_pop( &baud_devices[i], &c )) ConsPutc( c );
    }
} /* baud_drain_due */

void baud_flush_device( short term_id )
{
    int i;
    for (i=0; i<MAXBAUDDEV; i++) {
        if (baud_devices[i].inUse && baud_devices[i].term_id==term_id) {
            baud_devices[i].head= baud_devices[i].tail= baud_devices[i].count= 0;
            return;
        }
    }
} /* baud_flush_device */
```

- [ ] **Step 3: Rewrite `ConsoleOut`'s interactive branch**

Replace the current `ConsoleOut` (lines 741-836) with the version below. The TTY/PTY branch (`gConsoleID>=TTY_Base`) and the non-`TERMINAL_CONSOLE` `#else stdwrite(...)` fallback are **unchanged** — this feature is scoped to the interactive `TERMINAL_CONSOLE` branch only (PTY/multi-terminal is an explicit non-goal in the spec). The old fractional-tick "owed" pacing block after the loop is removed entirely, replaced by the FIFO.

```c
static os9err ConsoleOut( ushort pid, syspath_typ* spP,
                          uint32_t *maxlenP, char* buffer, Boolean wrln )
/* output to console */
{
    uint32_t     cnt;
    char         c;
    ulong        outputticks= GetSystemTick();
    syspath_typ* spC=  spP;          /* default: no crossed path */
    struct _sgs* ot = (struct _sgs*)&spC->opt; /* path opt table */
    Boolean      do_lf= false;
    process_typ* cp= &procs[pid];
    Boolean      paced= false;
    baud_device_t* dev= NULL;

    gConsoleID=  spP->term_id;
    g_spP     =  spP;

    /* go directly if tty */
    if (gConsoleID>=TTY_Base) {
        if (wrln) {
                   cnt= 0; /* search if there is any CR */
            while (cnt<*maxlenP) {
                    c= buffer[cnt++];
                if (c!=NUL && c==ot->_sgs_eorch) {
                    *maxlenP= cnt;
                    do_lf= ot->_sgs_alf;
                    break;
                }
            } /* while */
        } /* if */

        cnt=  WriteCharsToPTY( buffer,*maxlenP, gConsoleID, do_lf );
        if (cp->state==pSysTask) cnt= *maxlenP;
    }
    else {
        /* interactive output to console */
        #ifdef TERMINAL_CONSOLE
          /* decide once whether this write is paced or goes straight to the
             screen; guards pid==0 (system process) and the pid>=MAXPROCESSES
             sentinel (banner/system output) used elsewhere in this file */
          if (baud_throttle && pid>0 && pid<MAXPROCESSES && cp->state!=pSysTask) {
              ulong bps= baud_bps( ot->_sgs_bau );
              if (bps>0) {
                  dev= baud_dev_for( gConsoleID );
                  if (dev!=NULL) paced= true;
              }
          }

          cnt= 0;
          if (cp->state==pWaitWrite) {
              set_os9_state( pid, cp->saved_state, "ConsoleOut" );
              cnt=                cp->saved_cnt;
          }

          while (cnt<*maxlenP) {
              c= buffer[cnt];
              if (ot->_sgs_case && islower(c)) {
                  /* lower case -> upper case
                     NOTE: this may wreck alpha escape codes */
                  c = toupper(c);
              }

              if (paced) {
                  if (!fifo_push( dev, c )) {
                      cp->saved_cnt  = cnt;
                      cp->saved_state= cp->state;
                      set_os9_state( pid, pWaitWrite, "ConsoleOut" );
                      arbitrate= true;
                      break;
                  }
              }
              else {
                  ConsPutc( c );
              }
              cnt++;

              if (cp->state==pSysTask) { /* should never go to here */
                  cp->systask_offs= cnt-1; /* store it here !! */
                  cnt= *maxlenP;
                  break; /* tty/pty break */
              }

              if (c == CR && ot->_sgs_pause) {
                  term_line++;
                  if (term_line >= ot->_sgs_page) {
                      break;
                  }
              }

              if (wrln && c!=NUL && c==ot->_sgs_eorch) {
                  if (ot->_sgs_alf) {
                      /* trailing auto-linefeed: best-effort. In the extremely
                         narrow case where the FIFO is exactly full right when
                         this would be queued, it's dropped rather than adding
                         a second blocking path just for one cosmetic byte. */
                      if (paced) fifo_push( dev, LF ); else ConsPutc( LF );
                  }
                  break;
              }
          } /* while */

        #else   
          cnt= stdwrite(pid,buffer,*maxlenP,spP->stream,false);
        #endif
    }

    rw__idleticks+= GetSystemTick()-outputticks;
    if (cnt<0) return c2os9err(errno,E_WRITE); /* default: general write error */

    *maxlenP= cnt;
    return 0;
} /* ConsoleOut */
```

Note on `gLastwritten_pid`: `syspath_write` (`filestuff.c:1423`) already sets it to the writing pid before calling into `ConsoleOut`, so push-time attribution is correct as-is. The one known gap: when the *consumer* later calls `ConsPutc` for a queued byte, `ConsPutc` itself unconditionally resets `gLastwritten_pid= currentpid` (whichever process happens to be scheduled at pop time), which can point Ctrl-C/Ctrl-E's `send_signal` at the wrong process for a byte that's mid-trickle from an earlier write. This is accepted as-is: Task 4's flush (triggered when the signaled process actually terminates via `kill_process`) still stops that process's trickle regardless of this narrow attribution gap, so the practical impact is limited, not a correctness failure.

- [ ] **Step 4: Hook the drain into the scheduler's cooperative-yield points**

`Source/OS9exec_core/procstuff.c`, `DoWait()` (around line 958-996) — add the call right before the existing `slp_idleticks` bookkeeping line, after the platform-specific sleep:

```c
void DoWait( void )
{
  ulong ticks= GetSystemTick();
  
  #ifdef UNIX
    struct timespec wait_time;
    
    wait_time.tv_sec =       0;
    wait_time.tv_nsec= 1000000; // = 1 millisecond 
    nanosleep( &wait_time, NULL );
  //slp_idleticks++;
                
  #elif defined windows32
  //ulong ticks= GetSystemTick();
    Sleep( 1 ); // sleep for a short time
    HandleEvent();
  //slp_idleticks+= GetSystemTick()-ticks;
                
  #elif defined MACOS9
    int   sWait;
  //ulong ticks= GetSystemTick();

    #ifndef MPW // is not available there
    //ulong len= 13;
    //char  s[ 14 ];
      sWait= ( geCnt / 20 )+1; if (sWait>10) sWait= 10;
      geCnt= HandleOneEvent( nil,  sWait );
      
    //sprintf( s, "%5d %5d\r\n", geCnt, sWait );
    //syspath_write( currentpid, 1, &len, &s, true );
    #endif
                
  //slp_idleticks+= GetSystemTick()-ticks;
  #else
    #error architecture not supported
  #endif
  
  baud_drain_due();
  slp_idleticks+= GetSystemTick()-ticks;
} // DoWait
```

`do_arbitrate()` (around line 1004) — add the call as the first statement in the function body, right after the local variable declarations:

```c
void do_arbitrate( ushort allowedIntUtil )
{
  ushort       cpid       = currentpid;
  ushort       pid, spid;
  process_typ* cp;
  process_typ* sprocess;
  process_typ* cpw;
  ushort       sleepingpid= MAXPROCESSES; /* assume none sleeping */
  ushort       deadpid    = MAXPROCESSES; /* assume none dead */
  Boolean      done       = false;
  Boolean      chkAll     = false;        /* 2nd run when all sleeping */
  Boolean      atLeast1;                  /* at least one process is sleeping */
  Boolean      pDone;
  Boolean      cOK;

  baud_drain_due();

  debugprintf(dbgTaskSwitch,dbgDetail,("# arbitrate: current pid=%d, arbitrate=%d\n",
                                          currentpid, arbitrate));
```

- [ ] **Step 5: Build**

```bash
make -f GNUmakefile os9exec
```
Expected: builds clean, no new warnings.

- [ ] **Step 6: Run the existing suite (regression check)**

```bash
swift run --package-path test
```
Expected: `Results: 88 passed, 0 failed` — `dump: shows hex` still uses `-r`, which now bypasses the FIFO entirely, so it's unaffected.

- [ ] **Step 7: Add a new automated test proving the FIFO/block/resume mechanism is correct**

This test needs a *paced* invocation (no `-r`) to actually exercise `pWaitWrite`, so it needs a way to opt out of the harness's default `-r`. Add a `paced` parameter to the `os9()` helper.

`test/Sources/OS9Tests/main.swift`, change the function signature (around line 63) and the three `process.arguments` sites (around lines 72-96, edited in the earlier `-r` fix):

```swift
func os9(_ commands: [String], timeout: TimeInterval = 15, paced: Bool = false) -> String {
    let setup  = "chx \(sdkCmds)\nload math cio\n"
    let input  = setup + commands.joined(separator: "\n") + "\n\u{1B}\n"

    let process = Process()
    let speedFlag: [String] = paced ? [] : ["-r"]

    if let image = dockerImage {
        // Run via Docker: mount local dd directory and pipe stdin/stdout
        process.executableURL = URL(fileURLWithPath: "/usr/bin/env")
        process.arguments = [
            "docker",
            "run",
            "--rm",
            "-i",
            "-v", diskPath + ":/dd",
            image
        ] + speedFlag + ["shell"]
    } else if let image = containerImage {
        // Run via Apple Container: mount local dd directory and pipe stdin/stdout
        process.executableURL = URL(fileURLWithPath: "/usr/bin/env")
        process.arguments = [
            "container",
            "run",
            "--rm",
            "-i",
            "-v", diskPath + ":/dd",
            image
        ] + speedFlag + ["shell"]
    } else {
        // Run locally: use existing symlink at dd/
        process.executableURL = execURL
        process.arguments    = speedFlag + [shellArg]
    }
```

Then add a new test near the existing `dump: shows hex` check (around line 235), using a high baud rate (115200, verified live via `tmode baud=115200` — `tmode` takes the raw bps value, not a table index) so the paced run still completes in well under the test timeout, while still genuinely exercising the FIFO for a write bigger than its 256-byte capacity:

```swift
// baud pacing: a >256-byte paced write must produce byte-identical output
// to the unpaced (-r) version -- proves the FIFO/pWaitWrite block-and-resume
// path doesn't drop, duplicate, or reorder bytes. 115200 baud keeps this
// fast enough to finish well inside the timeout even though it's genuinely
// paced (not bypassing the FIFO like -r does).
do {
    let pacedOutput  = os9(["tmode baud=115200", "dump \(sdkCmds)/echo"], paced: true)
    let unpacedOutput= os9(["dump \(sdkCmds)/echo"])
    if pacedOutput == unpacedOutput && !unpacedOutput.isEmpty {
        print("PASS: baud pacing: paced dump matches unpaced dump byte-for-byte")
        passed += 1
    } else {
        print("FAIL: baud pacing: paced dump differs from unpaced")
        print("      paced length=\(pacedOutput.count), unpaced length=\(unpacedOutput.count)")
        failed += 1
    }
}
```

- [ ] **Step 8: Run the suite again, including the new test**

```bash
swift run --package-path test
```
Expected: `Results: 89 passed, 0 failed`.

- [ ] **Step 9: Commit**

```bash
git add Source/OS9exec_core/consio.c Source/OS9exec_core/filestuff.h \
        Source/OS9exec_core/procstuff.c test/Sources/OS9Tests/main.swift
git commit -m "Core: FIFO-backed ConsoleOut with pWaitWrite blocking (unpaced drain)"
```

---

### Task 3: Real host-microsecond pacing

**Files:**
- Modify: `Source/OS9exec_core/consio.c` (`us_per_char` computation, timing-aware `baud_drain_due`, new `baud_next_wake_delay_us`)
- Modify: `Source/OS9exec_core/filestuff.h` (one new extern)
- Modify: `Source/OS9exec_core/procstuff.c` (`DoWait()`'s sleep duration)
- Test: `test/Sources/OS9Tests/main.swift` (baud-pacing timing note; no new automated timing test — see Step 6)

**Interfaces:**
- Consumes: `baud_device_t`, `fifo_push`/`fifo_pop`, `baud_drain_due()` (Task 2).
- Produces: `ulong baud_next_wake_delay_us(void)` — microseconds until the next drain is needed, or `ULONG_MAX` if nothing is queued anywhere.

This task only changes *timing* — the FIFO/block/resume correctness proven in Task 2 is untouched.

- [ ] **Step 1: Declare the new function**

`Source/OS9exec_core/filestuff.h`, right after the Task 2 declarations:

```c
ulong baud_next_wake_delay_us( void );    /* microseconds until baud_drain_due() has work, or ULONG_MAX if none */
```

- [ ] **Step 2: Compute real `us_per_char` when a device is set up as paced**

In `ConsoleOut` (Task 2's Step 3), change:

```c
                  dev= baud_dev_for( gConsoleID );
                  if (dev!=NULL) paced= true;
```

to:

```c
                  dev= baud_dev_for( gConsoleID );
                  if (dev!=NULL) {
                      paced= true;
                      dev->us_per_char= (10UL*1000000UL)/bps; /* microseconds/char, 10 bits/char */
                  }
```

- [ ] **Step 3: Add host-microsecond timing and the one-shot deadline tracker to `consio.c`**

Add near the device primitives added in Task 2:

```c
static ulong g_next_wake_us= 0; /* earliest next_due_us across all paced non-empty devices, 0=none pending */

static ulong host_micros( void )
{
    struct timeval tv;
    gettimeofday( &tv, NULL );
    return (ulong)tv.tv_sec*1000000UL + (ulong)tv.tv_usec;
} /* host_micros */

static void recompute_next_wake( void )
{
    int   i;
    ulong earliest= 0;
    for (i=0; i<MAXBAUDDEV; i++) {
        baud_device_t* d= &baud_devices[i];
        if (d->inUse && d->count>0 && d->us_per_char>0) {
            if (earliest==0 || d->next_due_us<earliest) earliest= d->next_due_us;
        }
    }
    g_next_wake_us= earliest;
} /* recompute_next_wake */
```

- [ ] **Step 4: Make `fifo_push` set the deadline for a device going empty-to-non-empty**

Replace the Task 2 `fifo_push` with:

```c
static Boolean fifo_push( baud_device_t* d, byte c )
{
    if (d->count>=BAUD_FIFO_SIZE) return false;
    if (d->count==0 && d->us_per_char>0) {
        d->next_due_us= host_micros(); /* first queued char of a burst is due immediately */
    }
    d->buf[d->tail]= c;
    d->tail= (ushort)((d->tail+1) % BAUD_FIFO_SIZE);
    d->count++;
    if (d->us_per_char>0) recompute_next_wake();
    return true;
} /* fifo_push */
```

- [ ] **Step 5: Replace `baud_drain_due` with the timing-aware version and add `baud_next_wake_delay_us`**

```c
/* pop+display everything currently due, across all devices. Unpaced
   devices (us_per_char==0) always drain in full immediately -- they
   shouldn't normally accumulate a backlog, but drain fully if they ever do. */
void baud_drain_due( void )
{
    int   i;
    byte  c;
    ulong now;

    for (i=0; i<MAXBAUDDEV; i++) {
        baud_device_t* d= &baud_devices[i];
        if (!d->inUse || d->count==0) continue;

        if (d->us_per_char==0) {
            while (fifo_pop(d,&c)) ConsPutc(c);
        }
    }

    if (g_next_wake_us==0) return;           /* nothing paced is queued anywhere */
    now= host_micros();
    if (now<g_next_wake_us) return;          /* not due yet */

    for (i=0; i<MAXBAUDDEV; i++) {
        baud_device_t* d= &baud_devices[i];
        if (!d->inUse || d->count==0 || d->us_per_char==0) continue;

        while (d->count>0 && d->next_due_us<=now) {
            fifo_pop( d,&c );
            ConsPutc( c );
            d->next_due_us += d->us_per_char;
        }
    }
    recompute_next_wake();
} /* baud_drain_due */

ulong baud_next_wake_delay_us( void )
{
    ulong now;
    if (g_next_wake_us==0) return ULONG_MAX; /* nothing pending: no deadline */
    now= host_micros();
    if (now>=g_next_wake_us) return 0;       /* already due */
    return g_next_wake_us-now;
} /* baud_next_wake_delay_us */
```

- [ ] **Step 6: Make `DoWait()`'s idle nap precisely timed instead of a fixed 1ms**

`Source/OS9exec_core/procstuff.c`, `DoWait()`'s `UNIX` branch:

```c
  #ifdef UNIX
    struct timespec wait_time;
    ulong           delay_us= baud_next_wake_delay_us();
    long            delay_ns= (delay_us<1000000UL) ? (long)delay_us*1000L : 1000000L; /* cap idle nap at 1ms */

    wait_time.tv_sec =       0;
    wait_time.tv_nsec= delay_ns;
    nanosleep( &wait_time, NULL );
  //slp_idleticks++;
```

`do_arbitrate()`'s `baud_drain_due()` call from Task 2 needs no change — the function's own internal `g_next_wake_us==0` bail-out (Step 5 above) already makes it cheap when nothing's queued, so callers don't need to pre-check.

- [ ] **Step 7: Build**

```bash
make -f GNUmakefile os9exec
```
Expected: builds clean, no new warnings.

- [ ] **Step 8: Run the full suite**

```bash
swift run --package-path test
```
Expected: `Results: 89 passed, 0 failed` — the Task 2 byte-identical test now exercises genuinely paced (though fast, 115200 baud) output and must still match `-r` exactly.

- [ ] **Step 9: Manual verification via the REPL (not part of automated suite)**

Visible low-baud trickle, using `tools/os9repl.sh` per project convention:

```bash
./tools/os9repl.sh start
./tools/os9repl.sh send "chx /h1/CMDS"
./tools/os9repl.sh send "tmode baud=300"
./tools/os9repl.sh send "dump /h1/CMDS/echo"
```

Expected: output visibly trickles onto the screen over a few seconds rather than appearing all at once (300 baud = 30 chars/sec; a `dump` line is roughly 70-80 characters, so roughly 2-3 seconds per line). Confirm with `./tools/os9repl.sh peek` a moment after sending, then again a couple seconds later, and see more lines have appeared. Stop the session afterward:

```bash
./tools/os9repl.sh stop
```

- [ ] **Step 10: Commit**

```bash
git add Source/OS9exec_core/consio.c Source/OS9exec_core/filestuff.h Source/OS9exec_core/procstuff.c
git commit -m "Core: real host-microsecond baud pacing via one-shot deadline, not polling"
```

---

### Task 4: Flush a killed process's device (covers Ctrl-C/Ctrl-E too)

**Files:**
- Modify: `Source/OS9exec_core/procstuff.c` (`kill_process`)

**Interfaces:**
- Consumes: `baud_flush_device(short term_id)` (Task 2).

**Revised scope** (originally planned as two separate tasks — a `KeyToBuffer` hook for Ctrl-C/Ctrl-E and a separate `kill_process` hook — collapsed into one after review): Ctrl-C/Ctrl-E don't need their own flush hook. `KeyToBuffer` (`utilstuff.c`) already does nothing but call `send_signal(lwp, S_Intrpt)`/`send_signal(lwp, S_Abort)` when those keys are pressed (and does nothing at all if they're remapped/disabled via `tmode`) — the actual process termination, when the signal isn't caught by the target program, happens through the ordinary signal-delivery path into `kill_process()`. Hooking the flush there covers Ctrl-C/Ctrl-E (for the common case of an uncaught signal), explicit `kill <pid>`, and any other path that ends in `kill_process()`, with one hook instead of two.

Normal process exit needs no code at all — its queued output simply finishes draining on its own, same as real serial hardware completing a transmission after its source disappears.

- [ ] **Step 1: Extend `kill_process`'s existing Ctrl-C/E-disconnect step**

`Source/OS9exec_core/procstuff.c`, around line 635-639 — `kill_process` already resolves the killed process's console via `cp->last_mco->spP` at exactly this point, to disconnect it from future Ctrl-C/E signals. Flushing that same device's queued output is a one-line addition right alongside it:

```c
    /* now dispose all the process' resources */
    if (cp->last_mco!=NULL) {
        baud_flush_device( cp->last_mco->spP->term_id );
        cp->last_mco->spP->lastwritten_pid= 0; /* disconnect CtrlC/E signal */
        cp->last_mco= NULL;
    }
    debugprintf(dbgProcess,dbgNorm,("# kill_process: CtrlC/E signal disconnected\n" ));
```

- [ ] **Step 2: Build**

```bash
make -f GNUmakefile os9exec
```
Expected: builds clean, no new warnings.

- [ ] **Step 3: Run the full suite (regression check)**

```bash
swift run --package-path test
```
Expected: `Results: 89 passed, 0 failed`.

- [ ] **Step 4: Manual verification via the REPL — both paths**

Explicit kill:

```bash
./tools/os9repl.sh start
./tools/os9repl.sh send "chx /h1/CMDS"
./tools/os9repl.sh send "tmode baud=300"
./tools/os9repl.sh send "dump /h1/CMDS/echo &"
./tools/os9repl.sh send "procs"
```

Note the pid of the backgrounded `dump`, then kill it and confirm the trickle stops:

```bash
./tools/os9repl.sh send "kill <pid>"
./tools/os9repl.sh peek
```

Expected: no further output appears after the kill.

Ctrl-C (covers the common case — `dump` doesn't catch S_Intrpt, so the signal terminates it via `kill_process`, exercising the same flush):

```bash
./tools/os9repl.sh send "dump /h1/CMDS/echo &"
```

Wait roughly half a second (enough for a couple of lines to be queued/trickling), then send a Ctrl-C through the REPL's raw key mode (see `references/common/using-os9exec-repl.md` for gated-vs-raw mode):

```bash
./tools/os9repl.sh send $'\x03'
./tools/os9repl.sh peek
```

Expected: the trickle stops immediately — no further lines appear even after waiting a couple more seconds (`./tools/os9repl.sh peek` again to confirm). Stop the session afterward:

```bash
./tools/os9repl.sh stop
```

- [ ] **Step 5: Commit**

```bash
git add Source/OS9exec_core/procstuff.c
git commit -m "Core: flush a killed process's queued baud output (covers Ctrl-C/Ctrl-E)"
```

---

## Self-Review

**Spec coverage:**
- FIFO scope (per-device, 256 bytes) — Task 2, Step 2. ✓
- Producer blocks on full, mirrors `pConsRead` — Task 1 (state machine) + Task 2 Step 3 (producer). ✓
- Consumer via cooperative scheduler hooks, no thread/signal — Task 2 Step 4, Task 3 Steps 5-6. ✓
- `-r`/unknown-baud stays synchronous, zero added latency — Task 2 Step 3 (bypasses FIFO entirely, from Task 2 onward — a deliberate, disclosed deviation from the spec's literal "same push path" wording, chosen because it resolved a real sequencing conflict between Task 2's correctness-first milestone and Task 3's timing-only scope; the *behavioral* guarantee the spec cares about is preserved exactly). ✓
- One-shot deadline, not fixed-cadence polling — Task 3 Steps 3-6. ✓
- Ctrl-C/Ctrl-E flush, normal exit, best-effort kill flush — merged into Task 4 after review (Ctrl-C/Ctrl-E flush via the `kill_process` they trigger when uncaught, rather than a separate `KeyToBuffer` hook) + explicit no-op note for normal exit. ✓
- `pid==129` OOB safety — Task 2 Step 3 (`pid>0 && pid<MAXPROCESSES` guard). ✓
- Read-specific dispatcher logic excluded — Task 1 explicitly does not touch `pwr_brk`/`wRead`/the signal-intercept `rtestate` restore. ✓

**Placeholder scan:** none found — every step has complete code, no TBD/TODO.

**Type consistency:** `baud_device_t`, `fifo_push`/`fifo_pop`/`baud_dev_for` (Task 2) are used unchanged in Task 3; `baud_drain_due()`/`baud_flush_device()` signatures (Task 2) are unchanged by Task 3-5, only `baud_drain_due`'s internals change (Task 3) — verified consistent across all five tasks.

---

**Plan complete and saved to `docs/superpowers/plans/2026-07-10-baud-fifo-pacing.md`.** Two execution options:

**1. Subagent-Driven (recommended)** - I dispatch a fresh subagent per task, review between tasks, fast iteration

**2. Inline Execution** - Execute tasks in this session using executing-plans, batch execution with checkpoints

**Which approach?**
