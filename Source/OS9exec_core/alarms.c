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
 *    Revision 1.8  2004/11/27 11:51:06  bfo
 *    _XXX_ introduced
 *
 *    Revision 1.7  2004/11/20 11:44:08  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.6  2004/10/22 22:51:11  bfo
 *    Most of the "pragma unused" eliminated
 *
 *    Revision 1.5  2003/05/17 11:03:20  bfo
 *    (CVS header included)
 *
 *
 */


/* includes */
/* ======== */
#include "os9exec_incl.h"



/* OS-9 alarm routines */
/* =================== */

void init_alarms(void)
{
    int  k;
    for (k=0; k<MAXALARMS; k++) {
    	alarms     [k].pid= 0;     /* invalidate alarms */
    	alarm_queue[k]= NULL;
    }
} /* init_alarms */




void A_Insert( alarm_typ* aa )
{
	alarm_typ* q;
	int        k;

    debugprintf(dbgProcess,dbgNorm,("# A_Insert: aa=%p\n", (void*)aa ));
	for (k=MAXALARMS-1; k>0; k--) {
		    q= alarm_queue[k-1];
		if (q!=NULL) {
		    if (aa->due >= q->due ) {
		    	alarm_queue[k]= aa;
		    	return;
		    } /* if */
		
			alarm_queue[k]= q;
		} /* if */
	} /* for */
	
	alarm_queue[0]= aa; /* if list is still empty */
} /* A_Insert */



static void A_Dequeue( alarm_typ* aa )
/* take <aa> out of the due-time queue, leaving its slot allocated */
{
	Boolean fnd= false;
	int     k;   /* be careful: index k+1 !! */

	for (k=0; k<MAXALARMS-1; k++) {
		if (alarm_queue[k]==aa) fnd= true;
		if (fnd) alarm_queue[k]= alarm_queue[k+1];
	} /* for */

	/* the last position is free after a shift, or it was <aa> itself */
	if (fnd || alarm_queue[MAXALARMS-1]==aa) alarm_queue[MAXALARMS-1]= NULL;
} /* A_Dequeue */



void A_Remove( alarm_typ* aa )
/* take <aa> out of the queue and free its slot */
{
    debugprintf(dbgProcess,dbgNorm,("# A_Remove: aa=%p\n", (void*)aa ));
	A_Dequeue( aa );
	aa->pid= 0;
} /* A_Remove */



static alarm_typ* A_GetNew( ushort pid )
{
	alarm_typ* aa;
	int        k;
	
    for (k=0; k<MAXALARMS; k++) {
    	    aa= &alarms[k];
		if (aa->pid==0) {
			aa->pid= pid; /* activate it */
			return aa;
		} /* if */
	} /* for */	
		
	/* no empty entry available */
	return NULL;
} /* A_GetNew */



os9err A_Make( ushort pid, uint32_t *aId, ushort aCode, uint32_t aTicks, Boolean cyclic )
/* General routine to make alarms */
{
	alarm_typ* aa;
						 if (*aId!=0)  return E_BPADDR;
	aa= A_GetNew( pid ); if (aa==NULL) return E_NORAM;
	aa->signal= aCode;
	aa->ticks = aTicks;
	aa->due   = aTicks + GetSystemTick();
	aa->cyclic= cyclic;

	A_Insert   ( aa );
	/* slot + 1: an alarm ID is never 0, because A$Delete with ID 0 means
	   "all pending alarms" (Technical Manual, F$Alarm A$Delete). Slot 0 used
	   to hand out ID 0, which then could not be deleted by itself. */
	*aId= (uint32_t)(aa - alarms) + 1;
	return 0;
} /* A_Make */



void A_Kill( ushort pid )
/* Remove all alarms of this process */
{
	alarm_typ* q;
//	Boolean    fnd= false;
	int        k;
	
    debugprintf(dbgProcess,dbgNorm,("# A_Kill: pid=%d\n", pid ));
	/* A_Remove shifts the queue down over the removed entry, so the next
	   candidate is now at <k> again: advance only past an entry that stays.
	   Stepping on regardless skipped every second alarm of a dying process,
	   which then fired at whatever process got its ID next ("The system
	   automatically deletes a process's pending alarms when the process
	   dies", Technical Manual, F$Alarm). */
	for (k=0; k<MAXALARMS; ) {
		    q= alarm_queue[k];
		if (q!=NULL && q->pid==pid) A_Remove( q );
		else                        k++;
	} /* for */
} /* A_Kill */





/* ------------------------------------------------------------------ */

static os9err Alarm_Delete( ushort pid, uint32_t aId )
/* A$Delete call: 0. "If zero is passed as the alarm ID, all pending alarm
 * requests are removed" -- the caller's own, as the system-state page says
 * ("for the current process"). Any other ID must be one of the caller's
 * pending alarms: it used to delete whichever alarm held that slot, another
 * process's included. */
{
	alarm_typ* aa;

	if (aId==0) { A_Kill( pid ); return 0; }

	if (aId > MAXALARMS) return E_BPADDR;
	aa= &alarms[ aId-1 ];
	if (aa->pid==pid) {
		A_Remove( aa );
		return 0;
	}

	/* not a pending alarm of this process */
	return E_BPADDR;
} /* Alarm_Delete */



uint32_t A_Interval( uint32_t aTime )
/* An interval in ticks, from a value that is a tick count or, with its high bit
 * set, 256ths of a second: F$Alarm's A$Set/A$Cycle d3, and SS_Ticks' d2, which
 * the Technical Manual describes the same way. For F$Alarm the caller's d3 is a
 * tick count, or,
 * with its high bit set, "the low 31 bits are interpreted as 256ths of a
 * second" and "all times are rounded up to the nearest clock tick" (OS-9
 * v2.4 Technical Manual, F$Alarm). It was stored as raw ticks, so a C
 * program's alarm(2) -- unix.l passes the seconds shifted left 8 with bit 31
 * set, $80000200 -- came due about 2^31 ticks away and never fired.
 * 64-bit because 2^31-1 256ths times TICKS_PER_SEC overflows 32 bits. */
{
	uint64_t t;

	if ((aTime & 0x80000000)==0) return aTime;
	t= ((uint64_t)(aTime & 0x7fffffff)*TICKS_PER_SEC + 255)/256;
	return t>0x7fffffff ? 0x7fffffff : (uint32_t)t;
} /* A_Interval */



static os9err Alarm_Set( ushort pid, uint32_t *aId, ushort aCode, uint32_t aTicks )
/* A$Set call: 1 */
{	return A_Make( pid, aId,aCode,A_Interval( aTicks ), false );
} /* Alarm_Set */



static os9err Alarm_Cycle( ushort pid, uint32_t *aId, ushort aCode, uint32_t aTicks )
/* A$Cycle call: 2 */
{	return A_Make( pid, aId,aCode,A_Interval( aTicks ), true );
} /* Alarm_Cycle */



/* How many ticks from now an absolute alarm is due, given the current Julian
   time/date and the alarm's. A time ALREADY PAST is not an error: the manual
   says the signal "is sent anytime the system date/time becomes greater than
   or equal to the alarm time" (A$AtDate, page 1 - 3; A$AtJul, page 1 - 4),
   which for a past time is already so, and the Guru describes the system
   process firing any absolute alarm whose date and time "have been reached
   (or exceeded)" -- naming the F$STime case, where alarms that have expired
   "are immediately executed" (The OS-9 Guru, The Facts, 8.11, pages 176-177).
   So a past alarm is due now (zero ticks) and fires at the next check; only a
   date too far ahead to count in ticks is refused. os9exec returned E$Param
   for anything in the past until 2026-09-20 (CONF68K t91). */
static uint32_t A_Absolute( uint32_t iTime, uint32_t iDate,
                            uint32_t aTime, uint32_t aDate, uint32_t mx, os9err* errP )
{
	int32_t days, secs;

	*errP= 0;
	if (aDate>iDate && aDate-iDate>=mx) { *errP= E_PARAM; return 0; } /* beyond a tick count */

	days= (aDate>=iDate) ?  (int32_t)(aDate-iDate)
	                     : -(int32_t)(iDate-aDate);
	secs= days*(int32_t)SecsPerDay + (int32_t)aTime - (int32_t)iTime;
	if (secs<=0) return 0;                       /* already due */
	return (uint32_t)secs * TICKS_PER_SEC;
} /* A_Absolute */


static os9err Alarm_AtDate( ushort pid, uint32_t *aId, ushort aCode, uint32_t aTime, uint32_t aDate )
/* A$AtDate call: 3 */
{
	uint32_t iTime, iDate, aTicks;
	os9err   err;
	uint32_t gt_time, gt_date;
	int      dayOfWk, currentTick;
	uint32_t mx= (0xffffffff-GetSystemTick())/SecsPerDay/TICKS_PER_SEC;
    /* Declare the 32-bit object and take a BYTE view of it, rather than
     * declaring byte[4] and casting it up to uint32_t*: a byte array only has
     * alignment 1, so the old cast formed a possibly-misaligned uint32_t*
     * (undefined behaviour -- same family as the GET_OS9L/SET_OS9L fix in
     * os9_ll.h). Going this direction the alignment is guaranteed by the type,
     * and byte* aliasing of it is explicitly allowed. */
    uint32_t   tcv;
    byte*      tc= (byte*)&tcv;

	Get_Time( &gt_time,&gt_date, &dayOfWk,&currentTick, false,false );
	iTime= gt_time;  iDate= gt_date;

    tcv  = os9_long( aTime );         /* get time */
    aTime= tc[1]*3600+tc[2]*60+tc[3]; /* seconds since midnight */
    
    tcv  = os9_long( aDate );         /* get date */
    aDate= j_date(tc[3],tc[2], hiword( aDate ) );

	
	aTicks= A_Absolute( iTime,iDate, aTime,aDate, mx, &err );
	if (err) return err;
	return A_Make( pid, aId,aCode,aTicks, false );
} /* Alarm_AtDate */



static os9err Alarm_AtJul( ushort pid, uint32_t *aId, ushort aCode, uint32_t aTime, uint32_t aDate )
/* A$AtJul call: 4 */
{
	uint32_t iTime, iDate, aTicks;
	os9err   err;
	uint32_t gt_time, gt_date;
	int      dayOfWk, currentTick;
	uint32_t mx= (0xffffffff-GetSystemTick())/SecsPerDay/TICKS_PER_SEC;

	Get_Time( &gt_time,&gt_date, &dayOfWk,&currentTick, false,false );
	iTime= gt_time;  iDate= gt_date;

	aTicks= A_Absolute( iTime,iDate, aTime,aDate, mx, &err );
	if (err) return err;
	return A_Make( pid, aId,aCode,aTicks, false );
} /* Alarm_AtJul */



void CheckAlarms( void )
/* Deliver the signal for the earliest-due alarm, if it's actually due.
 * alarm_queue is kept sorted by A_Insert, so index 0 is always the next one
 * to fire.
 *
 * Callers: previously only the syscall (TRAP0) dispatch branch of
 * os9exec_loop, which meant a due alarm sat unchecked for as long as every
 * process in the system was asleep/blocked -- confirmed live, a 1-second
 * alarm did not interrupt a 10-second F$Sleep, the signal only arriving once
 * the sleep expired naturally and the process made its own next syscall.
 * Root cause traced with a debug counter: while anything is asleep,
 * os9exec_loop's own dispatch loop is not what's iterating -- do_arbitrate()
 * (procstuff.c) blocks inside ITS OWN loop, calling DoWait() over and over
 * until something's wakeUpTick expires, and control doesn't return to
 * os9exec_loop until then. So the fix lives in DoWait() itself: the same
 * function already polls stdin there (see its own comment, added for the
 * identical "only DoWait() ever runs while idle" reason with tsmon), now
 * also checked once per os9exec_loop iteration for symmetry/redundancy
 * (harmless -- A_Remove() makes a duplicate check inert). */
{
	alarm_typ* aa= alarm_queue[ 0 ];
	ushort     pid;
	ushort     sig;

	if (aa!=NULL && GetSystemTick()>=aa->due) {
		pid= aa->pid;
		sig= aa->signal;

		if (aa->cyclic) {
			/* Re-arm IN PLACE: the same slot, so the ID A$Cycle returned stays
			   valid for A$Delete. It used to be re-made in a new slot on every
			   firing, and A$Delete of the original ID then missed it -- or hit
			   another process's alarm that had taken the old slot. The queue
			   entry is taken out and re-inserted at its next due time; the pid
			   is kept, so the slot stays allocated. At least a tick per period,
			   or a zero interval would be due forever. */
			A_Dequeue( aa );
			aa->due+= aa->ticks>0 ? aa->ticks : 1;
			A_Insert ( aa );
		}
		else A_Remove( aa );

		/* last: the signal may kill the process, and kill_process then
		   removes its alarms (A_Kill) from the queue as it now stands */
		send_signal( pid, sig );
	} /* if */
} /* CheckAlarms */



os9err Alarm( ushort pid, uint32_t *aId, short aFunc, ushort sig, uint32_t aTime, uint32_t aDate )
{
	#define A_Delete    0x00
	#define A_Set       0x01
	#define A_Cycle     0x02
	#define A_AtDate    0x03
	#define A_AtJul     0x04
	
	os9err err;
	
    switch (aFunc) {
      case A_Delete: err= Alarm_Delete( pid, *aId                   ); break;
    	case A_Set   : err= Alarm_Set   ( pid,  aId, sig, aTime       ); break;
     	case A_Cycle : err= Alarm_Cycle ( pid,  aId, sig, aTime       ); break;
    	case A_AtDate: err= Alarm_AtDate( pid,  aId, sig, aTime,aDate ); break;
    	case A_AtJul : err= Alarm_AtJul ( pid,  aId, sig, aTime,aDate ); break;
    	default      : err= E_UNKSVC;
    } /* switch */
    
    return err;
} /* Alarm */



/* eof */

