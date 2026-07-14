/* prdbt.c -- guest-side verification for F$SetSys D_PrcDBT / F$GPrDBT.
 *
 * Walks the process descriptor block table and dereferences every entry, which
 * is only possible if the table AND the descriptor images it points at live in
 * the 68k arena. Before that fix the table held truncated HOST addresses, so a
 * guest walking it silently read unrelated emulator memory.
 *
 * Build and run inside the emulator (needs the SDK toolchain on /h0):
 *     chd /h0/USR/CLAUDE
 *     gcc2 -I/h0/DEFS/os9lib prdbt.c -o prdbt
 *     prdbt
 *
 * Expected: every live pid prints a descriptor address 2048 bytes apart, the
 * P$ID read back out of each descriptor equals its pid, and "mismatched=0".
 * NOTE: OS-9 sources need CR line endings -- convert before copying onto a disk.
 */
#include <stdio.h>
#include <setsys.h>

main()
{
    unsigned long   tbl;
    unsigned short *hdr;
    unsigned long  *ent;
    unsigned short  count, dsize;
    int             k, live, bad;

    tbl = (unsigned long)_getsys(D_PrcDBT, 4);
    printf("prDBT   = $%08lX\n", tbl);
    if (tbl == 0) { printf("FAIL: table address is 0\n"); exit(1); }

    hdr   = (unsigned short *)tbl;
    count = hdr[0];
    dsize = hdr[1];
    printf("count   = %u\n", count);
    printf("dsize   = %u\n", dsize);

    ent  = (unsigned long *)tbl;
    live = 0;
    bad  = 0;
    for (k = 1; k <= count; k++) {
        unsigned long   a = ent[k];
        unsigned short *pd;
        if (a == 0) continue;
        pd = (unsigned short *)a;
        printf("pid %3d -> desc $%08lX  P$ID=%u  P$PID=%u\n", k, a, pd[0], pd[1]);
        live++;
        if (pd[0] != (unsigned short)k) bad++;
    }
    printf("live=%d mismatched=%d\n", live, bad);
    return 0;
}
