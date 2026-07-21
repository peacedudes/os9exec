/* Blocks forever writing to a named pipe with no reader -- the process parks
 * in os9exec's pSysTask state (a blocked pipe write). Used by
 * pipe-abort-repro.sh to prove Ctrl-C/Ctrl-E can now interrupt such a process
 * (previously it wedged until a full emulator restart). See the shell-abort
 * flag in os9-dev references/common/using-os9exec-repl.md.
 */
#include <stdio.h>

main()
{
    int  path, i;
    char buf[256];

    for (i=0; i<256; i++) buf[i]= 0x58;      /* 'X' */
    path= create("/pipe/blk", 0x02);         /* named pipe, write */
    if (path < 0) { printf("CREATEFAIL\n"); exit(1); }
    for (;;) write(path, buf, 256);          /* fills the pipe, then blocks */
}
