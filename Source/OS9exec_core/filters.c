// 
//    OS9exec,   OS-9 emulator for Mac OS, Windows and Linux 
//    Copyright (C) 2002 Lukas Zeller / Beat Forster
//	  Available under http://www.synthesis.ch/os9exec
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
 *    Revision 1.7  2004/11/27 12:06:02  bfo
 *    _XXX_ introduced
 *
 *    Revision 1.6  2004/11/20 11:44:07  bfo
 *    Changed to version V3.25 (titles adapted)
 *
 *    Revision 1.5  2004/10/22 22:51:12  bfo
 *    Most of the "pragma unused" eliminated
 *
 *    Revision 1.4  2002/10/27 23:22:34  bfo
 *    use get_mem/release_mem
 *
 *
 */


/* global includes */
#include "os9exec_incl.h"


void releasefilter( void **memoryPP )
/* release filter and its memory */
{
    /* return the memory */
    if (*memoryPP!=NULL) { release_mem( *memoryPP );
        *memoryPP= NULL; /* no memory any more */
    }
} /* releasefilter */


#ifndef MPW_FILTERS
/* no filters available */
/* get pointer to filter function for module name returns NULL if no filter found
 */
filterfunc initfilterfunc( _txt_, _txt2_, _memPP_ )
{   return NULL; /* no filters */
} /* getfilterfunc */



/* show available filters from outside command line */
void printfilters( void )
{   uphe_printf("No compiler output filters available\n");
} /* printfilters */




#else

/* output filter for Omegasoft pascal */
/* ================================== */


typedef struct {
    char filepath[255];
} omega_record;


static void* omega_init( _txt_ )
{
    omega_record *fm;
    char *p,*p2;
    int srchspac;
    
    (void*)fm= get_mem(sizeof(omega_record),false);

    strcpy(fm->filepath,"<unknown>"); /* no file name found yet */
    /* try to obtain filename from command line */
    srchspac=false;
    for (p=argline; *p>=0x20; p++) {
        if (srchspac) {
            if (*p==' ') srchspac=false; /* next space found */
        }
        else {
            if (*p!=' ') {
                /* could be start of argument */
                if (*p=='-') {
                    /* is an option, forget it */
                    srchspac=true; /* skip until next space is found */
                }
                else {
                    /* is first argument, copy as filename */
                    p2=fm->filepath;
                    while (*p>0x20) *p2++ = *p++; /* copy */
                    *p2=0; /* terminate */
                    break; /* done */
                }
            }
        }
    }

    /* now we should have a copy of the filename in fm->filepath */
    return (void*)fm;
}


static void omega_filter(char *linebuf, FILE *stream, void *fmv)
{
    ulong thisline;
    omega_record *fm;
    
    
    fm=(omega_record *)fmv;
    /* write line anyway */
    fprintf(stream,"# %s\n",linebuf);
    /* check if this could possibly be an error line display */
    if (linebuf[4]=='*' || linebuf[4]==':') {
        /* could be */
        if (sscanf(&linebuf[0],"%d", &thisline)==1) {
            /* line number seems to be ok */
            if (isdigit((unsigned char)linebuf[5])) {
                /* we take this as a Omegasoft error number */
                fputs("#--------------------------------------------------------------------------------------------------------------------------------\n",stream);
                fprintf(stream,"    File \"%s\"; Line %d\n",fm->filepath,thisline);
                fputs("#--------------------------------------------------------------------------------------------------------------------------------\n",stream);
            }
        }
    }
} /* omega_filter */



/* output filter for icc6811 */
/* ========================= */

typedef struct {
    char lastline[200];
} icc_record;


static void* icc_init( char* /* argline */ )
{
    icc_record *fm;
    
    (void*)fm= get_mem(sizeof(icc_record),false);
//  #ifdef MACMEM
//  (void*)fm=NewPtrClear(sizeof(icc_record));
//  #else
//  (void*)fm=    cmalloc(sizeof(icc_record));
//  #endif

    fm->lastline[0]=0; /* ensure empty lastline */
    return (void*)fm;
}


static void icc_filter(char *linebuf, FILE *stream, void *fmv)
{
    char *p;
    char *thisfile;
    int thisline,thiscol;
    int dispit; /* set if error message can be displayed */
    icc_record *fm;
    
    
    fm=(icc_record *)fmv;
    /* --- output line first */
    if (*linebuf!=0) {
        fputs("# ",stream);
        fputs(linebuf,stream);
    }
    putc('\n',stream);
    fflush(stream);
    dispit=false;
    /* --- analyze line */
    if (*linebuf=='"') {
        /* --- beginning of a C error message */
        if ((p=strstr (linebuf+1, "\","))!=NULL) {
            /* --- extract file name */
            *p=NUL; /* replace " by NUL terminator */
            thisfile=linebuf+1;
            /* --- extract line number */
            p+=2;
            thisline=atoi(p);
            /* --- find out column number */
            p=strchr(fm->lastline,'^');
            thiscol=(p==NULL ? 0 : p-fm->lastline);
            dispit=true; /* all info found */
        }
    }
    strncpy(fm->lastline,linebuf,200);
    if (dispit) {
        /* --- error message is complete */
        fputs("#--------------------------------------------------------------------------------------------------------------------------------\n",stream);
/*      if (thiscol<50) {
            fprintf(stream,"    File \"%s\"; Line Æ%d!%d:%dÆ\n",thisfile,thisline,thiscol,thisline);
        }
        else {
            fprintf(stream,"    File \"%s\"; Line Æ%d:¤!%d\n",thisfile,thisline,thiscol,thisline);
        }
*/
        fprintf(stream,"    File \"%s\"; Line %d\n",thisfile,thisline);
        fputs("#--------------------------------------------------------------------------------------------------------------------------------\n",stream);
    }
} /* icc_filter */


/* output filter for cfe (C-compiler) */
/* ================================== */

typedef struct {
    char fname[OS9PATHLEN];
    int fline;
    int fcol;
    int state;
} c_record;


static void* c_init( char* /* argline */ )
{
    c_record *fm;
    
    (void*)fm= get_mem(sizeof(c_record),false);
//  #ifdef MACMEM
//  (void*)fm=NewPtrClear(sizeof(c_record));
//  #else
//  (void*)fm=    cmalloc(sizeof(c_record));
//  #endif

    fm->state=0; /* begin without state */
    return (void*)fm;
} /* c_init */


static void c_filter( char *linebuf, FILE *stream, void *fmv )
{
    char *p,*p2;
    int dispit; /* set if error message can be displayed */
    c_record *fm;
    
    
    fm=(c_record *)fmv;
    /* --- output line first */
    if (*linebuf!=0) {
        fputs("# ",stream);
        fputs(linebuf,stream);
    }
    putc('\n',stream);
    fflush(stream);
    dispit=false;
    /* --- analyze line */
    debugprintf(dbgUtils,dbgNorm,("# cfe filter: state=%d\n",fm->state));
    switch (fm->state) {
        case 0 :
            if (*linebuf=='"') { /* beginning of an error message */
                debugprintf(dbgUtils,dbgNorm,("# cfe filter: found beginning of error message\n"));
                if ((p=strstr(linebuf+1,"\", line "))!=NULL) {
                    /* --- extract file name */
                    strncpy(fm->fname,linebuf+1,p-linebuf-1);
                    debugprintf(dbgUtils,dbgNorm,("# cfe filter: extracted filename=%s\n",fm->fname));
                    /* --- extract line number */
                    p+=7; /* skip the ", line " string */
                    if ((p2=strstr(p,": "))!=NULL) *(p2)=0; else break;
                    fm->fline=atoi(p);
                    debugprintf(dbgUtils,dbgNorm,("# cfe filter: extracted line no=%s\n",fm->fline));
                    /* first line processed */
                    fm->state=1; 
                } /* if */
            } /* if */
            break; /* case 0 */
        case 1 :
            if (*linebuf!='"') fm->state=2; /* source (and addtl message) line(s) skipped now */
            break; /* case 1 */
        case 2 :
            if ((p=strchr(linebuf,'^'))!=NULL) fm->fcol=p+1-linebuf; else fm->fcol=0;
            dispit=true; /* all info found */
            fm->state=0; /* wait for next error */
            break;
    } /* switch */
    if (dispit) {
        /* --- error message is complete */
        fputs("#--------------------------------------------------------------------------------------------------------------------------------\n",stream);
        fprintf(stream,"    File \"%s\"; Line %d\n",fm->fname,fm->fline);
        fputs("#--------------------------------------------------------------------------------------------------------------------------------\n",stream);
    }
} /* c_filter */


/* output filter for r68 Assembler */
/* =============================== */

typedef struct {
    char fname[OS9PATHLEN];
    int outline;
    int fline;
    int fcol;
    int state;
} r68_record;


static void* r68_init( char* /* argline */ )
{
    r68_record *fm;
    
    (void*)fm= get_mem(sizeof(r68_record),false);
//  #ifdef MACMEM
//  (void*)fm=NewPtrClear(sizeof(r68_record));
//  #else
//  (void*)fm=    cmalloc(sizeof(r68_record));
//  #endif

    fm->state  =0; /* begin without state */
    fm->outline=0; /* no line written yet */
    return (void*)fm;
} /* r68_init */


static void r68_filter(char *linebuf, FILE *stream, void *fmv)
{
    char *p;
    int dispit; /* set if error message can be displayed */
    r68_record *fm;
    
    
    fm=(r68_record *)fmv;
    /* --- output line first */
    if (*linebuf!=0) {
        fputs("# ",stream);
        fputs(linebuf,stream);
    }
    putc('\n',stream);
    fflush(stream);
    dispit=false;
    fm->outline++;
    /* --- analyze line */
    if (fm->outline==2) {
        /* this is the filename */
        strncpy(fm->fname,linebuf+1,OS9PATHLEN);
        debugprintf(dbgUtils,dbgNorm,("# r68 filter: filename=%s\n",fm->fname));
    }
    switch (fm->state) {
        case 0 :
            if (strncmp(linebuf,"*** error - ",12)==0) {
                /* this is an error message line */
                fm->state=1;
            }
            break;
        case 1 :
            /* now extract line number */
            linebuf[5]=0;
            fm->fline=atoi(linebuf);
            fm->state=2;
            break;
        case 2 :
            /* now extract column number */
            if ((p=strchr(linebuf,'^'))!=NULL) fm->fcol=p-linebuf-5; else fm->fcol=0;
            dispit=true; /* all info found */
            fm->state=0; /* wait for next error */
            break;
    } /* switch */
    if (dispit) {
        /* --- error message is complete */
        fputs("#--------------------------------------------------------------------------------------------------------------------------------\n",stream);
        fprintf(stream,"    File \"%s\"; Line %d\n",fm->fname,fm->fline);
        fputs("#--------------------------------------------------------------------------------------------------------------------------------\n",stream);
    }
} /* r68_filter */





/* Filter-to-module name table */
/* --------------------------- */

typedef void* (*initfunc)(char *argline);

/* MWC: removed const %%% */
typedef struct {
    char *modname;
    filterfunc routine;
    initfunc initroutine;
    char *helptext;
} filtertable_typ;

filtertable_typ filtertable[] =
{
    { "r68",     r68_filter,   r68_init, "Microware Assembler for 68000"     },
    { "r68020",  r68_filter,   r68_init, "Microware Assembler for 68020"     },
    { "cfe",       c_filter,     c_init, "Microware C compiler (C-frontend)" },
    { "pc",     mega_filter, omega_init, "OmegaSoft Pascal Compiler"         },
    { "icc6811", icc_filter,   icc_init, "IAR HC 11 cross compiler"          },
    {  NULL,           NULL,       NULL                                      } /* terminator */
};


/* Routines */
/* -------- */


/* get pointer to filter function for module name
 * returns NULL if no filter found
 * initializes filter function (and allocates memory if
 * required and returns allocated pointer in *memoryPP.
 * Caller must call releasefilter(&filtermem)
 * later when process exits
 */
filterfunc initfilterfunc(char *modname, char *argline, void **memoryPP)
{
    filtertable_typ *fp;
    int index;
    
    if (disablefilters) return NULL; /* no filters */
    
    index=0;
    do {
        fp=&filtertable[index];
        if (fp->modname==NULL) break; /* none */
        if (ustrcmp(fp->modname,modname)==0) {
            /* entry found */
            if (fp->initroutine==NULL) *memoryPP=NULL; /* no memory required for filter */
            else {
                /* filter has an init routine, call it */
                *memoryPP=(fp->initroutine)(argline); /* may allocate memory for private filter storage */
            }
            return fp->routine;
        }
        index++;
    } while(1);
    return NULL;
} /* getfilterfunc */


/* show available filters from outside command line */
void printfilters(void)
{
    int k;
    
    uphe_printf("Filters for converting error output to MPW format is available for:\n");
    for (k=0; filtertable[k].modname!=NULL; k++) {
        uphe_printf("  %-20s : %s\n",filtertable[k].modname,filtertable[k].helptext);
    }
    upe_printf("\n");
} /* printfilters */

#endif

/* eof */
