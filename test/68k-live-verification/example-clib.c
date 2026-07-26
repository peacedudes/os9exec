#include <stdio.h>
#include <errno.h>

#define READ_MODE  1
#define ATTRS      0x03

main()
{
    int path, child, status;

    /* --- happy path: create, write, close --- */
    path = creat("cexfile.txt", ATTRS);
    if (path < 0) {
        printf("FAIL creat, errno %d\n", errno);
        exit(1);
    }
    write(path, "written by cexample\015", 20);
    close(path);
    printf("PASS create/write/close\n");

    /* --- error path: open a file that is not there --- */
    path = open("no.such.file", READ_MODE);
    if (path < 0)
        printf("PASS open of a missing file: errno %d\n", errno);
    else {
        printf("FAIL open of a missing file succeeded\n");
        close(path);
    }

    /* --- error path: fork a module that is not there --- */
    child = os9fork("nosuchmod", 0, "", 1, 1, 0);
    if (child < 0)
        printf("PASS os9fork of a missing module: errno %d\n", errno);
    else
        printf("FAIL os9fork of a missing module returned %d\n", child);

    /* --- happy path: fork a real child and collect its status --- */
    child = os9fork("exfio1", 0, "", 1, 1, 0);
    if (child < 0) {
        printf("FAIL os9fork, errno %d\n", errno);
        exit(1);
    }
    child = wait(&status);
    printf("PASS child pid %d exited, status %d\n", child, status);
    exit(0);
}
