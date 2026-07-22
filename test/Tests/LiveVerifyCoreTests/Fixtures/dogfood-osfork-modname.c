/* Dogfood task 2026-07-20: resolves a known gap in os9-clib-reference.md
 * -- does os9fork()'s modname argument resolve a BARE module name (no
 * leading /) the way Shell does, or does it require a full path? A full
 * path was already known to work live; this test is specifically about
 * the bare-name case, which was untested. os9fork() is a thin C-library
 * wrapper around F$Fork (proprietary SDK source, never read -- this is
 * answered by observing behavior, not by reading how the wrapper is
 * implemented, matching this project's clean-room convention).
 *
 * Target: "childprg68k" (test/68k-live-verification/childprg68k.a) --
 * already proven live via a bare-name F$Fork in batch2-01.a, so it's
 * known-safe: exits immediately with F$Exit(77), no risk of attaching
 * to a terminal and hanging the way forking "shell" bare would.
 * type=1 (program) / lang=1 (native object code for this CPU) match
 * childprg68k.a's own psect declaration (Prgrm=1, Objct=1) exactly.
 */
#include <stdio.h>

main()
{
    int pid, status;

    pid = os9fork("childprg68k", 0, 0, 1, 1, 4096);
    if (pid == -1) {
        printf("bare name 'childprg68k' failed to fork\n");
        return 1;
    }
    printf("bare name 'childprg68k' forked ok, pid=%d\n", pid);
    wait(&status);
    printf("child exited, status=%d\n", status);
    return 0;
}
