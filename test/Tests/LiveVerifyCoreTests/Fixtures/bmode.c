/* Access-mode (E$BMode) enforcement on disk files. On real OS-9 RBF, writing to
 * a file opened read-only, or reading a file opened write-only, is E$BMode (203).
 * os9exec used to ignore the open mode and let both through. Exits 0 (PASS) only
 * if BOTH directions are now rejected with 203. See syspath_read/write in
 * filestuff.c.
 */
#include <stdio.h>
#include <errno.h>

main()
{
    int  p, r, ok = 1;
    char b[4];

    p = creat("/h5/bmtest", 3);         /* create read+write, seed 3 bytes */
    write(p, "ABC", 3);
    close(p);

    p = open("/h5/bmtest", 1);          /* READ-ONLY (mode 1) */
    errno = 0;
    r = write(p, "Y", 1);               /* write must fail E$BMode */
    close(p);
    if (!(r < 0 && errno == 203)) { printf("FAIL write-to-read-only ret=%d errno=%d\n", r, errno); ok = 0; }

    p = open("/h5/bmtest", 2);          /* WRITE-ONLY (mode 2) */
    errno = 0;
    r = read(p, b, 3);                  /* read must fail E$BMode */
    close(p);
    if (!(r < 0 && errno == 203)) { printf("FAIL read-from-write-only ret=%d errno=%d\n", r, errno); ok = 0; }

    if (ok) { printf("PASS E$BMode enforced both directions\n"); exit(0); }
    exit(1);
}
