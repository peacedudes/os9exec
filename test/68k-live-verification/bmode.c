/* Opens a disk file read-only (access mode 1 = R) and writes to it. On real
 * OS-9 RBF that is E$BMode (203); os9exec used to ignore the open mode and let
 * the write through. Exits 0 (PASS) only if the write is now rejected with 203.
 * See usrpath/syspath_write in filestuff.c.
 */
#include <stdio.h>
#include <errno.h>

main()
{
    int p, r;

    p = creat("/h5/bmtest", 3);     /* create read+write, seed a byte */
    write(p, "X", 1);
    close(p);

    p = open("/h5/bmtest", 1);      /* re-open READ-ONLY (mode 1) */
    errno = 0;
    r = write(p, "Y", 1);           /* writing must fail E$BMode */
    close(p);

    if (r < 0 && errno == 203) { printf("PASS E$BMode enforced on write-to-read-only\n"); exit(0); }
    printf("FAIL write to read-only path returned ret=%d errno=%d (expected -1 / 203)\n", r, errno);
    exit(1);
}
