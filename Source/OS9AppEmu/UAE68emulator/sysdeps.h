 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Try to include the right system headers and get other system-specific
  * stuff right & other collected kludges.
  *
  * If you think about modifying this, think twice. Some systems rely on
  * the exact order of the #include statements. That's also the reason
  * why everything gets included unconditionally regardless of whether
  * it's actually needed by the .c file.
  *
  * Copyright 1996, 1997 Bernd Schmidt
  */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
  
#if defined __MACH__ && defined __GNUC__ && !defined __BIG_ENDIAN__
  #define __INTEL__
#endif

/* This UAE-derived corner of the tree is standalone -- it doesn't include
 * os9exec_incl.h, so os9main_incl_precomp.h's own __INTEL__ detection
 * (which covers mingw-w64) never reaches it. Without this, maccess.h's
 * `#ifdef __INTEL__` block -- where do_get_mem_long() et al. actually
 * live -- is skipped entirely on Windows. */
#if defined __MINGW32__ || defined __MINGW64__
  #define __INTEL__
#endif

#ifndef _WIN32
#ifndef __STDC__
#error "Your compiler is not ANSI. Get a real one."
#endif
#endif

#include <stdarg.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_VALUES_H
#include <values.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_UTIME_H
#include <utime.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if HAVE_DIRENT_H
# include <dirent.h>
#else
# define dirent direct
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

#ifdef HAVE_SYS_UTIME_H
# include <sys/utime.h>
#endif

#include <errno.h>
#include <assert.h>

#if EEXIST == ENOTEMPTY
#define BROKEN_OS_PROBABLY_AIX
#endif

#ifdef __NeXT__
#define S_IRUSR S_IREAD
#define S_IWUSR S_IWRITE
#define S_IXUSR S_IEXEC
#define S_ISDIR(val) (S_IFDIR & val)
struct utimbuf
{
    time_t actime;
    time_t modtime;
};
#endif

#if defined(__GNUC__) && defined(AMIGA)
/* gcc on the amiga need that __attribute((regparm)) must */
/* be defined in function prototypes as well as in        */
/* function definitions !                                 */
#define REGPARAM2 REGPARAM

#elif defined(__GNUC__)
/* linux also uses gcc */
#define REGPARAM
#define REGPARAM2

#else /* not(GCC & AMIGA) */
#define REGPARAM2
#endif

/* sam: some definitions so that SAS/C can compile UAE */
#if defined(__SASC) && defined(AMIGA)
#define REGPARAM2 
#define REGPARAM
#define S_IRUSR S_IREAD
#define S_IWUSR S_IWRITE
#define S_IXUSR S_IEXECUTE
#define S_ISDIR(val) (S_IFDIR & val)
#define mkdir(x,y) mkdir(x)
#define truncate(x,y) 0
#define creat(x,y) open("T:creat",O_CREAT|O_TEMP|O_RDWR) /* sam: for zfile.c */
#define strcasecmp stricmp
#define utime(file,time) 0
struct utimbuf
{
    time_t actime;
    time_t modtime;
};
#endif

#ifdef __DOS__
#include <pc.h>
#include <io.h>
#endif

/* Acorn specific stuff */
#ifdef ACORN

#define S_IRUSR S_IREAD
#define S_IWUSR S_IWRITE
#define S_IXUSR S_IEXEC

#define strcasecmp stricmp

#endif

#ifndef L_tmpnam
#define L_tmpnam 128 /* ought to be safe */
#endif

/* If char has more then 8 bits, good night. */
typedef unsigned char uae_u8;
typedef signed char uae_s8;

typedef struct { uae_u8 RGB[3]; } RGB;

#if SIZEOF_SHORT == 2
typedef unsigned short uae_u16;
typedef short uae_s16;
#elif SIZEOF_INT == 2
typedef unsigned int uae_u16;
typedef int uae_s16;
#else
#error No 2 byte type, you lose.
#endif

#if SIZEOF_INT == 4
typedef unsigned int uae_u32;
typedef int uae_s32;
#elif SIZEOF_LONG == 4
typedef unsigned long uae_u32;
typedef long uae_s32;
#else
#error No 4 byte type, you lose.
#endif

typedef uae_u32 uaecptr;

#undef uae_s64
#undef uae_u64

#if SIZEOF_LONG_LONG == 8
#define uae_s64 long long
#define uae_u64 long long
#define VAL64(a) (a ## LL)
#define UVAL64(a) (a ## uLL)
#elif SIZEOF___INT64 == 8
#define uae_s64 __int64
#define uae_u64 unsigned __int64
#define VAL64(a) (a)
#define UVAL64(a) (a)
#elif SIZEOF_LONG == 8
#define uae_s64 long;
#define uae_u64 unsigned long;
#define VAL64(a) (a ## l)
#define UVAL64(a) (a ## ul)
#endif

#ifdef HAVE_STRDUP
#define my_strdup strdup
#else
extern char *my_strdup (const char*s);
#endif

extern void *xmalloc(size_t);
extern int   in_m68k_go;

/* We can only rely on GNU C getting enums right. Mickeysoft VSC++ is known
 * to have problems, and it's likely that other compilers choke too. */
#ifdef __GNUC__
#define ENUMDECL typedef enum
#define ENUMNAME(name) name
#else
#define ENUMDECL enum
#define ENUMNAME(name) ; typedef int name
#endif

/*
 * Porters to weird systems, look! This is the preferred way to get
 * filesys.c (and other stuff) running on your system. Define the
 * appropriate macros and implement wrappers in a machine-specific file.
 *
 * I guess the Mac port could use this (Ernesto?)
 */

#undef DONT_HAVE_POSIX
#undef DONT_HAVE_REAL_POSIX /* define if open+delete doesn't do what it should */
#undef DONT_HAVE_STDIO
#undef DONT_HAVE_MALLOC

#if defined _WIN32
#include <windows.h>
#define O_NDELAY 0

//#define DONT_HAVE_POSIX /* I want Mathias' posixemu_ functions! */

#ifdef _CONSOLE
#define write_log printf
#define gui_message printf
#else
extern void write_log( const char *, ... );
extern void gui_message( const char *, ...);
#endif

#if defined __WATCOMC__

#include <direct.h>
#define dirent direct
#define mkdir(a,b) mkdir(a)
#define strcasecmp stricmp

#elif defined __GNUC__
#define abort() do { write_log( "Internal error in module %s at line %d\n", __FILE__, __LINE__ );exit(1); } while (0)

/* mingw-w64 (unlike the ancient MinGW this block was written for) ships a
 * real dirent.h and a standards-conformant S_ISDIR, both already pulled in
 * via os9main_incl_precomp.h -- declaring our own conflicts with those. */
#if !defined __MINGW32__ && !defined __MINGW64__
#define DIR struct DIR
extern DIR* opendir(char *);
struct direct
{
    char d_name[1];
};
struct dirent *readdir (DIR * dir);
extern void closedir(DIR *);
#undef S_ISDIR
#define S_ISDIR(a) (a&0x100)
#endif

#define __int64 long long

#elif defined _MSC_VER
#include <io.h>
#include <direct.h>

#ifndef _WIN32
#define O_BINARY _O_BINARY
#define O_WRONLY _O_WRONLY
#define O_RDONLY _O_RDONLY
#define O_RDWR   _O_RDWR
#define O_CREAT  _O_CREAT
#define O_TRUNC  _O_TRUNC
#endif

#define strcasecmp _stricmp
#define REGPARAM
#define REGPARAM2
#define __inline__ __inline
#define __volatile__ volatile
#define __asm__(a) ;
#undef my_strdup
#define my_strdup _strdup
#define stricmp _stricmp
#define off_t  int
#define utimbuf _utimbuf
#define fdopen _fdopen
#define fileno _fileno
#define DIR struct DIR

#ifndef USE_DIRENT_PORT
  extern DIR* opendir(char *);
  struct direct
  {
    char d_name[1];
  };
  struct dirent *readdir (DIR * dir);
  extern void closedir(DIR *);
#endif

#define W_OK 0x2
#define R_OK 0x4

// %%% LuZ: definitions to make UAE work with CW 5 Win
#elif defined __MWERKS__

//#include <io.h>
//#include <direct.h>
//#define O_BINARY _O_BINARY
//#define O_WRONLY _O_WRONLY
//#define O_RDONLY _O_RDONLY
//#define O_RDWR   _O_RDWR
//#define O_CREAT  _O_CREAT
//#define O_TRUNC  _O_TRUNC
//#define strcasecmp _stricmp
#define REGPARAM
#define REGPARAM2
#define __inline__ __inline
#define __volatile__ volatile
//#define __asm__(a) ;
//#undef my_strdup
//#define my_strdup _strdup
//#define stricmp _stricmp
//#define off_t  int
//#define utimbuf _utimbuf
//#define fdopen _fdopen
//#define fileno _fileno
//#define DIR struct DIR
//extern DIR* opendir(char *);
//struct direct
//{
//    char d_name[1];
//};
//struct dirent *readdir (DIR * dir);
//extern void closedir(DIR *);
//#define W_OK 0x2
//#define R_OK 0x4

// %%% end MetroWerks
#endif

#define FILEFLAG_DIR     0x1
#define FILEFLAG_ARCHIVE 0x2
#define FILEFLAG_WRITE   0x4
#define FILEFLAG_READ    0x8
#define FILEFLAG_EXECUTE 0x10
#define FILEFLAG_SCRIPT  0x20
#define FILEFLAG_PURE    0x40
#undef S_ISDIR
#undef S_IWUSR
#undef S_IRUSR
#undef S_IXUSR
#define S_ISDIR(a) (a&FILEFLAG_DIR)
#define S_ISARC(a) (a&FILEFLAG_ARCHIVE)
#define S_IWUSR FILEFLAG_WRITE
#define S_IRUSR FILEFLAG_READ
#define S_IXUSR FILEFLAG_EXECUTE

#undef L_tmpname
#define L_tmpname MAX_PATH /* For posixemu_tmpnam() */

#define HAVE_GETTICKCOUNT

#endif /* _WIN32 */ 


/* %%% LuZ: definitions to make UAE work with CW 5 Mac */
#if defined macintosh  || defined __MACH__
#if defined __MWERKS__ || defined __GNUC__

//#include <io.h>
//#include <direct.h>
//#define O_BINARY _O_BINARY
//#define O_WRONLY _O_WRONLY
//#define O_RDONLY _O_RDONLY
//#define O_RDWR   _O_RDWR
//#define O_CREAT  _O_CREAT
//#define O_TRUNC  _O_TRUNC
//#define strcasecmp _stricmp
#define REGPARAM
#define REGPARAM2
#define __inline__ __inline
#define __volatile__ volatile
//#define __asm__(a) ;
//#undef my_strdup
//#define my_strdup _strdup
//#define stricmp _stricmp
//#define off_t  int
//#define utimbuf _utimbuf
//#define fdopen _fdopen
//#define fileno _fileno
//#define DIR struct DIR
//extern DIR* opendir(char *);
//struct direct
//{
//    char d_name[1];
//};
//struct dirent *readdir (DIR * dir);
//extern void closedir(DIR *);
//#define W_OK 0x2
//#define R_OK 0x4

// %%% end MetroWerks
#endif
#endif // macintosh




#ifdef DONT_HAVE_POSIX
#define access posixemu_access
extern int posixemu_access (const char *, int);
#define open posixemu_open
#define creat(a,b) posixemu_open(a,O_RDWR|O_CREAT,0)
extern int posixemu_open (const char *, int, int);
#define close posixemu_close
extern void posixemu_close (int);
#define read posixemu_read
extern int posixemu_read (int, char *, int);
#define write posixemu_write
extern int posixemu_write (int, const char *, int);
#undef lseek
#define lseek posixemu_seek
extern int posixemu_seek (int, int, int);
#define stat(a,b) posixemu_stat ((a), (b))
extern int posixemu_stat (const char *, struct stat *);
#define mkdir posixemu_mkdir
extern int posixemu_mkdir (const char *, int);
#define rmdir posixemu_rmdir
extern int posixemu_rmdir (const char *);
#define unlink posixemu_unlink
extern int posixemu_unlink (const char *);
#define truncate posixemu_truncate
extern int posixemu_truncate (const char *, long int);
#define rename posixemu_rename
extern int posixemu_rename (const char *, const char *);
#define chmod posixemu_chmod
extern int posixemu_chmod (const char *, int);
#define tmpnam posixemu_tmpnam
extern void posixemu_tmpnam (char *);
#define utime posixemu_utime
extern int posixemu_utime (const char *, struct utimbuf *);
#define opendir posixemu_opendir
extern DIR * posixemu_opendir (const char *);
#define readdir posixemu_readdir
extern struct dirent* readdir (DIR *);
#define closedir posixemu_closedir
extern void closedir (DIR *);

#if defined( SUPPORT_PENGUINS ) || defined( UAE_FILESYS_THREADS )
/* We need posixemu prototypes for the semaphore stuff */
typedef HANDLE  uae_sem_t;
typedef DWORD   penguin_id;
void sem_init( uae_sem_t*, int, int );
void sem_wait( uae_sem_t* );
void sem_post( uae_sem_t* );
int sem_trywait( uae_sem_t* );
int start_penguin( void *(*f)(void *), void *, penguin_id* );
#endif

/* This isn't the best place for this, but it fits reasonably well. The logic
 * is that you probably don't have POSIX errnos if you don't have the above
 * functions. */
extern long dos_errno (void);

#endif

#ifdef DONT_HAVE_STDIO

extern FILE *stdioemu_fopen (const char *, const char *);
#define fopen(a,b) stdioemu_fopen(a, b)
extern int stdioemu_fseek (FILE *, int, int);
#define fseek(a,b,c) stdioemu_fseek(a, b, c)
extern int stdioemu_fread (char *, int, int, FILE *);
#define fread(a,b,c,d) stdioemu_fread(a, b, c, d)
extern int stdioemu_fwrite (const char *, int, int, FILE *);
#define fwrite(a,b,c,d) stdioemu_fwrite(a, b, c, d)
extern int stdioemu_ftell (FILE *);
#define ftell(a) stdioemu_ftell(a)
extern int stdioemu_fclose (FILE *);
#define fclose(a) stdioemu_fclose(a)

#endif

#ifdef DONT_HAVE_MALLOC

#define malloc(a) mallocemu_malloc(a)
extern void *mallocemu_malloc (int size);
#define free(a) mallocemu_free(a)
extern void mallocemu_free (void *ptr);

#endif

#ifdef X86_ASSEMBLY
#define ASM_SYM_FOR_FUNC(a) __asm__(a)
#else
#define ASM_SYM_FOR_FUNC(a)
#endif

#if defined USE_COMPILER
#undef NO_PREFETCH_BUFFER
#undef NO_EXCEPTION_3
#define NO_EXCEPTION_3
#define NO_PREFETCH_BUFFER
#endif

#include "target.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

