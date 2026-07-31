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
    for (k=0; k<MAXEVENTS; k++) events[k].id= 0; /* invalidate event */
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
    *evId    = ev->id;
    
    return 0;
} /* evCreat */



os9err evDelet( char* evName )
{
    event_typ* ev;
    int        k;

    if (strlen(evName)>=OS9EVNAMELEN) return E_BNAM; /* name too long */

    for (k=0;  k<MAXEVENTS; k++) {
            ev= &events[k];
        if (ev->id!=0 && strcmp(ev->name,evName)==0 ) {
            ev->id= 0; return 0;
        }
    } /* for */
    
    return E_EVNF;
} /* evDelet */


os9err evWait( uint32_t evId, int minV, int maxV, int *evValue )
{
    os9err       err;
    syspath_typ* spP;
    uint32_t     cnt;
    int          k, prev;
    
    event_typ*   ev= getEv( evId );
    if          (ev==NULL) return E_EVNTID;

    /* search for events from peripheral devices */ 
    for (k=0; k<MAXSYSPATHS; k++) {
            spP= get_syspath( currentpid,k );
        if (spP!=NULL && 
            spP->set_evId==evId) {
                 err= syspath_gs_ready( currentpid,k, &cnt );
            if (!err) { err= evSet( evId, 1, &prev ); break; }
        } /* if */
    }
    
    if (ev->value>=minV &&
        ev->value<=maxV) {
        ev->value+= ev->wInc;
        *evValue  = ev->value;
        return 0;
    } /* if */

    return EV_NOTYET; /* valid event, just not in range yet -- caller should park & retry */
} /* evWait */



os9err evSignl( uint32_t evId )
{
    event_typ*  ev= getEv( evId );
    if         (ev==NULL) return E_EVNTID;
    ev->value+= ev->sInc;
    return 0;
} /* evSignl */


os9err evSet( uint32_t evId, int evValue, int *prvValue )
{
    event_typ* ev= getEv( evId );
    if        (ev==NULL) return E_EVNTID;

    *prvValue= ev->value;
    ev->value= evValue;
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



os9err evSetR( uint32_t evId, int evIncr, int *prvValue )
/* Ev$SetR: like evSet(), but the caller supplies an increment to apply to the
   current value rather than the value itself, and the signal auto-increment is
   NOT used. Returns the previous value. */
{
    event_typ* ev= getEv( evId );
    if        (ev==NULL) return E_EVNTID;

    *prvValue= ev->value;
    ev->value= evSatAdd( ev->value, evIncr );
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
        /* 24 and 28 are the queue links. os9exec has no event queue -- waiters
           park and re-test the range (see evWait) instead of being threaded
           onto one -- so both stay 0 rather than carrying a host pointer that
           would mean nothing in the guest's address space. */

        *foundP= (ushort)k;
        return 0;
    } /* for */

    return E_EVNTID; /* "The index is above all active events." */
} /* evInfo */



/* eof */

