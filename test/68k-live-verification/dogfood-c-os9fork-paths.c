/*
 * Captured 2026-07-30 from /h0/forktest.c on the live 68k system disk, a
 * working master not under version control -- this was the only copy.
 * os9fork + wait: the child writes to both path 1 and path 2 to show that
 * a forked child inherits the parent's open paths, and exits 7 so the
 * parent's wait() status can be checked against a known value.
 *
 * Hardcodes its own module path /dd/forktest -- os9fork takes a path, not
 * a module name, so the binary must sit there or the fork fails.
 */
#include <stdio.h>
#include <strings.h>

main(argc, argv)
int argc;
char *argv[];
{
    int pid, status;

    if (argc > 1 && strcmp(argv[1], "child") == 0) {
        fprintf(stdout, "child: on path 1 (stdout)\n");
        fprintf(stderr, "child: on path 2 (stderr)\n");
        exit(7);
    }

    pid = os9fork("/dd/forktest", 6, "child", 1, 1, 4096);
    if (pid == -1) {
        printf("os9fork failed\n");
        exit(1);
    }
    printf("parent: forked child pid=%d, waiting...\n", pid);
    wait(&status);
    printf("parent: child exit status=%d\n", status);
}
