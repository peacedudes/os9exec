// 
//    OS9exec,   OS-9 emulator for Mac OS, Windows and Linux 
//    Copyright (C) 2002 Lukas Zeller / Beat Forster
//    Available under http://www.synthesis.ch/os9exec
// 
//    This program is free software; you can redistribute it and/or 
//    modify it under the terms of the GNU General Public License as 
//    published by the Free Software Foundation; either version 2 of 
//    the License, or (at your option) any later version. 
// 
//    This program is distributed in the hope that it will be useful, 
//    but WITHOUT ANY WARRANTY; without even the implied warranty of 
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
//    See the GNU General Public License for more details. 
// 
//    You should have received a copy of the GNU General Public License 
//    along with this program; if not, write to the Free Software 
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 
//

/**********************************************/
/*             O S 9 E x e c / NT             */
/*  Cooperative-Multiprocess OS-9 emulation   */
/*         for Apple Macintosh and PC         */
/*                                            */
/* (c) 1993-2006 by Lukas Zeller, CH-Zuerich  */
/*                  Beat Forster, CH-Maur     */
/*                                            */
/* email: luz@synthesis.ch                    */
/*        bfo@synthesis.ch                    */
/**********************************************/

/*
 *  CVS:
 *    $Author$
 *    $Date$
 *    $Revision$
 *    $Source$
 *    $State$
 *    $Name$ (Tag)
 *    $Locker$ (who has reserved checkout)
 *  Log:
 *    $Log$
 *    Revision 1.5  2004/11/20 11:44:06  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.4  2003/05/17 11:04:17  bfo
 *    (CVS header included)
 *
 *
 */


/* includes */
/* ======== */
#include "os9exec_incl.h"



/* OS-9 event routines */
/* =================== */

void init_events(void)
{
    int  k;
    for (k=0; k<MAXEVENTS; k++) {
        events[k].id   = 0;            /* invalidate event */
        events[k].qHead= MAXPROCESSES; /* nobody waiting on it */
        events[k].qTail= MAXPROCESSES;
    }
} /* init_events */



static event_typ* getEv( uint32_t evId )
{
    event_typ* ev;
    int  k;
    for (k=0; k<MAXEVENTS; k++) {
            ev= &events[k];
        if (ev->id==evId ) return ev;
    }

    return NULL;
} /* getEv */



/* The wait queue */
/* ============== */

/* "If the value is not in range, the calling process is suspended in a FIFO
   event queue" (Ev$Wait). os9exec used to have no such queue: a waiter parked
   and re-tested the range for itself every time it was rescheduled. That
   cannot express Ev$Pulse at all, whose pulsed value exists only while the
   signal search runs and is restored immediately afterwards -- so every
   waiter woke up too late to see it, every time.

   With the queue, the search decides WHO wakes, in what order, and at what
   value, at the moment of the signal or pulse. A woken waiter is flagged and
   carries the value the search saw; it still has to be scheduled before it
   can return, but what it returns was settled back then. That split is why
   none of this needed a change to the scheduler or to the park/retry
   machinery in os9exec_nt.c. */

static Boolean evQueued( process_typ* wp, event_typ* ev )
/* Is this process really a waiter on this event? Asked of every entry the
   search walks, because a queue of process indices outlives the processes it
   names: a slot can be freed and handed to a new process while its index is
   still linked in. Every new process starts with ev_id 0 (procstuff.c), and
   only evWait sets it, so the two have to agree before an entry counts. */
{
    return (Boolean)( wp->ev_id==ev->id && ev->id!=0 );
} /* evQueued */



static void evUnlink( event_typ* ev, ushort prev, ushort pid )
/* Take pid out of ev's queue. <prev> is the entry before it, or MAXPROCESSES
   when it is the head. */
{
    process_typ* wp= &procs[pid];

    if (prev>=MAXPROCESSES) ev->qHead          = wp->ev_next;
    else                    procs[prev].ev_next= wp->ev_next;

    if (ev->qTail==pid)     ev->qTail          = prev;

    wp->ev_next= MAXPROCESSES;
} /* evUnlink */



void evDequeue( ushort pid )
/* Take a process off whatever event queue it is on, and forget it was
   waiting. Called when a process is killed, and whenever a wait ends. Doing
   nothing for a process that is not queued is what lets kill_process call it
   without first asking whether it needs to. */
{
    process_typ* wp;
    event_typ*   ev;
    ushort       k, prev;

    if (pid>=MAXPROCESSES) return;
        wp= &procs[pid];
    if (wp->ev_id==0) { wp->ev_woken= false; return; }

        ev= getEv( wp->ev_id );
    if (ev!=NULL) {
        prev= MAXPROCESSES;
        for (k= ev->qHead; k<MAXPROCESSES; k= procs[k].ev_next) {
            if (k==pid) { evUnlink( ev, prev, pid ); break; }
            prev= k;
        } /* for */
    } /* if */

    wp->ev_id   = 0;
    wp->ev_woken= false;
    wp->ev_next = MAXPROCESSES;
} /* evDequeue */



static void evEnqueue( event_typ* ev, ushort pid, int minV, int maxV )
/* Add a waiter at the TAIL: the manual calls it a FIFO queue, and the signal
   search below relies on the order to decide which single process wakes. */
{
    process_typ* wp= &procs[pid];

    evDequeue( pid ); /* it can only ever wait on one event at a time */

    wp->ev_id       = ev->id;
    wp->ev_minV     = minV;
    wp->ev_maxV     = maxV;
    wp->ev_next     = MAXPROCESSES;
    wp->ev_woken    = false;
    wp->ev_wakeValue= 0;
    wp->ev_wakeErr  = 0;

    if (ev->qTail<MAXPROCESSES) procs[ev->qTail].ev_next= pid;
    else                        ev->qHead                = pid;
    ev->qTail= pid;
} /* evEnqueue */



static void evWake( process_typ* wp, int evValue, os9err err )
/* Record what a waiter is to be told. It cannot be told now -- it is not the
   running process -- so this is the whole of the wake, and the value has to
   be captured here rather than re-read later: after an Ev$Pulse the event
   does not hold it any more. */
{
    wp->ev_wakeValue= evValue;
    wp->ev_wakeErr  = err;
    wp->ev_woken    = true;
} /* evWake */



static void evSearch( event_typ* ev, Boolean allProcs )
/* The Ev$Signl search routine, which Ev$Signl, Ev$Set, Ev$SetR and Ev$Pulse
   all run once they have set the event value. The manual's own sequence, in
   its own order:

       "The signal auto-increment is added to the event variable" -- done by
        the caller, since each of the four reaches this point differently;
       "The first process in range is awakened";
       "The event variable is updated with the wait auto-increment";
       "The search continues with the updated value."

   and the sequence repeats "for each event in the queue until the queue is
   exhausted" only when the MS bit of d1 asked for it: "If the MS bit of d1
   (the function code) is set, all processes in the event queue that have a
   value in range are activated." Without it the search stops at the first,
   which is the difference this queue exists to be able to express -- polling
   waiters each re-tested the range for themselves, so every signal woke
   everybody. */
{
    process_typ* wp;
    ushort       pid, next, prev;

    prev= MAXPROCESSES;
    pid = ev->qHead;

    while (pid<MAXPROCESSES) {
        wp  = &procs[pid];
        next= wp->ev_next;

        if (!evQueued( wp,ev )) { /* stale entry -- drop it and keep going */
            evUnlink( ev, prev, pid );
            pid= next;
            continue;
        } /* if */

        if (ev->value>=wp->ev_minV &&
            ev->value<=wp->ev_maxV) {
            evWake  ( wp, ev->value, 0 );
            evUnlink( ev, prev, pid );

            /* Saturating, where the old polling code added plainly. The
               values are the guest's and the event value is a full signed
               32-bit quantity, so a plain add is undefined behaviour at the
               extremes -- and this is the one place OS-9 applies the wait
               increment without the caller having asked for a specific sum. */
            ev->value= evSatAdd( ev->value, ev->wInc );

            if (!allProcs) return;
        }
        else prev= pid;

        pid= next;
    } /* while */
} /* evSearch */




os9err evLink( char* evName, uint32_t *evId )
{
    event_typ* ev;
    int        k;
    if (strlen(evName)>=OS9EVNAMELEN) return E_BNAM; /* name too long */

    for (k=0;  k<MAXEVENTS; k++) {
            ev= &events[k];
        if (ev->id!=0 && strcmp(ev->name,evName)==0 ) { 
            ev->e_linkcount++;
            *evId= ev->id;
            return 0; 
        }
    } /* for */
    
    return E_EVNF;
} /* evLink */



os9err evUnLnk( uint32_t evId )
{
    event_typ* ev;
    int        k;
    for (k=0;  k<MAXEVENTS; k++) {
                ev= &events[k];
        if     (ev->id!=0 && ev->id==evId ) { 
            if (ev->e_linkcount>0)
                ev->e_linkcount--;
            return 0; 
        }
    } /* for */
    
    return E_EVNTID;
} /* evUnLnk */



os9err evCreat( char* evName, int evValue, short wInc, short sInc, uint32_t *evId )
{
    event_typ* ev;
    int        k;

    if (strlen(evName)>=OS9EVNAMELEN) return E_BNAM; /* name too long */
        
    for (k=0; k<MAXEVENTS; k++) { /* already there ? */
            ev= &events[k];
        if (ev->id!=0 && strcmp(ev->name,evName)==0 ) return E_EVBUSY;
    } /* for */

        ev= getEv( 0 );   /* this is an event which is not yet in use */
    if (ev==NULL) return E_MEMFUL;
    
               newEventId+= EvOffs; /* get the next value */
    ev->id   = newEventId;
    strcpy(ev->name, evName );
    ev->value= evValue;
    ev->wInc = wInc;
    ev->sInc = sInc;
    ev->e_linkcount++;
    ev->qHead= MAXPROCESSES; /* a new event has nobody waiting on it, and the */
    ev->qTail= MAXPROCESSES; /* slot may have carried a deleted event's queue */
    *evId    = ev->id;
    
    return 0;
} /* evCreat */



os9err evDelet( char* evName )
{
    event_typ* ev;
    int        k;
    ushort     pid;

    if (strlen(evName)>=OS9EVNAMELEN) return E_BNAM; /* name too long */

    for (k=0;  k<MAXEVENTS; k++) {
            ev= &events[k];
        if (ev->id!=0 && strcmp(ev->name,evName)==0 ) {
            /* "An event may not be deleted unless its use count is zero"
               (Ev$Delet, page 1-21), which lists E$EvBusy as its answer when
               the count is not. Ev$Creat sets that count to one, so a creator
               has to Ev$UnLnk its own event before it can delete it -- this
               call used to free the slot regardless.

               THE MANUAL CONTRADICTS ITSELF HERE, so the reading is recorded
               rather than left implicit. Ev$UnLnk (page 1-20) says the count
               is decremented "and the event is deleted when the count reaches
               zero", which would make Ev$Delet unreachable: no count could
               ever still be zero by the time anyone called it. Three
               statements outweigh that one. The Ev$Delet page itself; the
               OS-9 Guru, which says that once the count reaches zero "the
               event can be deleted by a delete event call"; and OS-9
               Insights, whose evdel utility unlinks in a LOOP until the
               delete stops failing -- a loop that could not be written if the
               unlink reaching zero had already done the deleting. The last
               sentence of the Ev$UnLnk page, that OS-9 uses the count "only
               for error checking", is the one that fits all three.

               Neither book is Microware, so this stays a reading and not a
               fact: CONF68K t43/t44 pin both halves so that real OS-9/68k
               hardware can settle it. A system implementing the Ev$UnLnk
               sentence literally answers E$EvNF where t44 expects success. */
            if (ev->e_linkcount!=0) return E_EVBUSY;

            /* "The kernel will wake up any processes waiting on an event that
               is being deleted (returning them an invalid event ID error --
               E$EvntID)", and the Guru gives the reason: a process can unlink
               more than it linked, taking the count to zero while others are
               still waiting. So this is reachable even now that the count is
               enforced, and a waiter left behind would wait for an ID that no
               longer exists with nothing able to wake it. */
            while (ev->qHead<MAXPROCESSES) {
                       pid= ev->qHead;
                if    (evQueued( &procs[pid], ev ))
                       evWake  ( &procs[pid], 0, E_EVNTID );
                evUnlink( ev, MAXPROCESSES, pid );
                procs[pid].ev_id= 0;
            } /* while */

            ev->id   = 0;
            ev->qTail= MAXPROCESSES;
            return 0;
        }
    } /* for */

    return E_EVNF;
} /* evDelet */


static os9err evTakeWake( process_typ* wp, int *evValue )
/* Hand a woken waiter what the search recorded for it, and end its wait. */
{
    os9err err  = wp->ev_wakeErr;

    *evValue    = wp->ev_wakeValue;
    wp->ev_woken= false;
    wp->ev_id   = 0;
    wp->ev_next = MAXPROCESSES;
    return err;
} /* evTakeWake */



os9err evWait( uint32_t evId, int minV, int maxV, ushort pid, int *evValue )
/* Ev$Wait. Called once when the process makes the call, and again on every
   retry while it is parked -- the two are told apart by whether this process
   is already on this event's queue, not by anything the caller passes.

   A queued waiter is satisfied ONLY by the signal search. It deliberately no
   longer re-tests the range for itself: that test is what used to make every
   signal wake every waiter regardless of the "first process in range"
   wording, and it can never see an Ev$Pulse, whose value is put back before
   any waiter runs again. */
{
    os9err       err;
    syspath_typ* spP;
    uint32_t     cnt;
    int          k, prev;
    process_typ* cp = &procs[pid];
    Boolean      queued;
    event_typ*   ev;

    queued= (Boolean)( cp->ev_id==evId && evId!=0 );

    /* Woken by a search that has already run -- including evDelet's, which is
       why this is asked before the event is looked up at all. What the search
       saw stands even if the event has gone since. */
    if (queued && cp->ev_woken) return evTakeWake( cp, evValue );

        ev= getEv( evId );
    if (ev==NULL) { evDequeue( pid ); return E_EVNTID; }

    /* Events set by peripheral devices. A waiter polls its OWN paths, on
       every retry, because nothing else in the system notices that a device
       has become ready. evSet runs the signal search, so a wake found this
       way is delivered through the queue like any other. */
    for (k=0; k<MAXSYSPATHS; k++) {
            spP= get_syspath( pid,k );
        if (spP!=NULL &&
            spP->set_evId==evId) {
                 err= syspath_gs_ready( pid,k, &cnt );
            if (!err) { evSet( evId, 1, false, &prev ); break; }
        } /* if */
    }

    if (queued) {
        /* That search may have been the one that satisfied us. */
        if (cp->ev_woken) return evTakeWake( cp, evValue );
        return EV_NOTYET;
    } /* if */

    /* First time in: "the event variable is compared to the range specified
       in d2 and d3. If the value is not in range, the calling process is
       suspended in a FIFO event queue." */
    if (ev->value>=minV &&
        ev->value<=maxV) {
        ev->value= evSatAdd( ev->value, ev->wInc );
        *evValue = ev->value;
        return 0;
    } /* if */

    evEnqueue( ev, pid, minV,maxV );
    return EV_NOTYET; /* valid event, caller should park & retry */
} /* evWait */



os9err evSignl( uint32_t evId, Boolean allProcs )
/* Ev$Signl: "the current event variable is updated with the signal
   auto-increment specified when the event was created. Then, the event queue
   is searched for the first process waiting for that event value." */
{
    event_typ*  ev= getEv( evId );
    if         (ev==NULL) return E_EVNTID;

    ev->value= evSatAdd( ev->value, ev->sInc );
    evSearch( ev, allProcs );
    return 0;
} /* evSignl */



os9err evPulse( uint32_t evId, int pulseValue, Boolean allProcs )
/* Ev$Pulse: "the event variable is set to the value passed in d2, and the
   signal auto-increment is not applied. Then, the Ev$Signl search routine is
   executed and the original event value is restored."

   Restoring afterwards discards whatever wait auto-increments the search
   applied along the way, which is what "the original event value" says and
   what makes a pulse leave no trace: its value exists only for processes that
   were ALREADY waiting, and for nobody else, ever. Nothing about that can be
   observed by a waiter that re-tests the range after the fact, which is why
   this call needed a real queue before it could be implemented at all. */
{
    int         saved;
    event_typ*  ev= getEv( evId );
    if         (ev==NULL) return E_EVNTID;

    saved    = ev->value;
    ev->value= pulseValue;
    evSearch( ev, allProcs );
    ev->value= saved;
    return 0;
} /* evPulse */



os9err evSet( uint32_t evId, int evValue, Boolean allProcs, int *prvValue )
/* Ev$Set: set the event variable AND signal an event occurrence -- the search
   is half of what the call is named for. */
{
    event_typ* ev= getEv( evId );
    if        (ev==NULL) return E_EVNTID;

    *prvValue= ev->value;
    ev->value= evValue;
    evSearch( ev, allProcs );
    return 0;
} /* evSet */



int evSatAdd( int a, int b )
/* Signed add, saturating instead of overflowing: "If an underflow or overflow
   occurs on the addition, the values $80000000 (minimum integer), and
   $7fffffff (maximum integer) are used, respectively" (Ev$WaitR; Ev$SetR says
   the same of its increment).
   Summed as UNSIGNED because signed overflow is undefined behaviour -- it is
   the thing being detected, so it must not be allowed to happen first. */
{
    int32_t sum= (int32_t)( (uint32_t)a + (uint32_t)b );

    if (b>0 && sum<a) return  2147483647;       /* overflowed  -> max integer */
    if (b<0 && sum>a) return -2147483647 - 1;   /* underflowed -> min integer */
    return sum;
} /* evSatAdd */



os9err evRead( uint32_t evId, int *evValue )
/* Read the value "without waiting or modifying the event variable" (Ev$Read).
   Deliberately does NOT apply the wait increment, which is what separates this
   from evWait(). */
{
    event_typ* ev= getEv( evId );
    if        (ev==NULL) return E_EVNTID;

    *evValue= ev->value;
    return 0;
} /* evRead */



os9err evSetR( uint32_t evId, int evIncr, Boolean allProcs, int *prvValue )
/* Ev$SetR: like evSet(), but the caller supplies an increment to apply to the
   current value rather than the value itself, and the signal auto-increment is
   NOT used. Returns the previous value, and runs the same search. */
{
    event_typ* ev= getEv( evId );
    if        (ev==NULL) return E_EVNTID;

    *prvValue= ev->value;
    ev->value= evSatAdd( ev->value, evIncr );
    evSearch( ev, allProcs );
    return 0;
} /* evSetR */



/* Big-endian stores into the guest's buffer, one byte at a time -- never a cast
   to a wider pointer type. The block's fields are not naturally aligned (its
   4-byte value sits at an even offset that is not a multiple of 4), and this
   code is built for hosts where an unaligned access through a wider type is
   undefined, as well as for both endiannesses. Same shape fcalls.c already uses
   to lay out a register image. */
static void putEvWord( byte* p, ushort v )
{
    p[0]= (byte)((v>> 8) & 0xFF);  p[1]= (byte)( v      & 0xFF);
} /* putEvWord */

static void putEvLong( byte* p, uint32_t v )
{
    p[0]= (byte)((v>>24) & 0xFF);  p[1]= (byte)((v>>16) & 0xFF);
    p[2]= (byte)((v>> 8) & 0xFF);  p[3]= (byte)( v      & 0xFF);
} /* putEvLong */



os9err evInfo( ushort index, byte* buffer, ushort* foundP )
/* Ev$Info: copy the information block of the first active event whose INDEX is
   >= <index>. The index is the event table position -- "the system event
   number, ranging from zero to the maximum number of system events minus one"
   -- not an event ID, which is why the call takes only the low word of d0.

   Field order and the 32-byte size are the manual's (event ID, name, value,
   wait increment, signal increment, link count, next, previous). The widths
   follow from what it already states -- a name of at most 12 characters, a
   "four-byte integer" value, and two queue pointers -- which leaves exactly one
   word each for the four remaining fields, and the ID being a word is why
   Ev$Info indexes on d0's low word at all. */
{
    event_typ* ev;
    int        k;

    for (k= index; k<MAXEVENTS; k++) {
            ev= &events[k];
        if (ev->id==0) continue; /* free slot -- keep looking */

        memset( buffer, 0, Ev_BlockSize );

        /* The low word of our 32-bit internal ID. The block has room for a word
           and no more, and the guest cannot pass this back as an event ID
           anyway -- Ev$Wait and friends take the full ID handed out by
           Ev$Creat/Ev$Link. It is here to be displayed. */
        putEvWord( buffer +  0, (ushort)( ev->id & 0xFFFF ) );

        /* Not strncpy: the field is a fixed 12 bytes, the block is already
           zeroed, and a name of exactly 12 characters legitimately fills it
           with no terminator. evCreat rejects anything longer. */
        memcpy   ( buffer +  2, ev->name, strlen( ev->name ) );

        putEvLong( buffer + 14, (uint32_t)ev->value );
        putEvWord( buffer + 18, (ushort)  ev->wInc  );
        putEvWord( buffer + 20, (ushort)  ev->sInc  );
        putEvWord( buffer + 22,           ev->e_linkcount );
        /* 24 and 28 are the queue links, and stay 0. There IS a queue now
           (see evSearch), but it is threaded through host process indices,
           and neither those nor the host addresses of the process descriptors
           they stand for mean anything in the guest's address space. A number
           that looks like a pointer and is not one would be worse than a
           zero, which at least says "not available here". */

        *foundP= (ushort)k;
        return 0;
    } /* for */

    return E_EVNTID; /* "The index is above all active events." */
} /* evInfo */



/* eof */

