/*
 * Captured 2026-07-30 from /h0/wtest.c on the live 68k system disk, a
 * working master not under version control -- this was the only copy.
 * Interleaves buffered printf with a raw write(1,...) to show what
 * ordering the C library actually produces, with an explicit fflush
 * between them. Note the length 29 covers the 28-character string plus
 * its terminating NUL, so the NUL reaches the path too.
 */
#include <stdio.h>

main()
{
    printf("before write\n");
    fflush(stdout);
    write(1, "raw write with fflush first\n", 29);
    printf("after write\n");
}
