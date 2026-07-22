/* truncsk <file> <newsize>: SS.Size then SEEK to the new EOF before close, so
   RBF's close-time truncation (which fires only with the pointer at EOF) runs
   and releases the tail. Companion to trunc, which leaves the pointer at 0 --
   the update-mode-not-at-EOF case RBF deliberately preserves for random-access
   files. No stdio, so no cio dependency; run it from the RBF device, not a host
   directory (os9exec cannot fork a host-directory binary). */
extern int open(), close(), _ss_size(), atoi();
extern long lseek();
extern void exit();
main(argc, argv)
int argc; char **argv;
{
    int path; long newsize;
    if (argc < 3) exit(1);
    newsize = (long)atoi(argv[2]);
    path = open(argv[1], 3);        /* update: read+write */
    if (path < 0) exit(1);
    _ss_size(path, newsize);
    lseek(path, newsize, 0);        /* pointer to new EOF -> close truncates */
    close(path);
    exit(0);
}
