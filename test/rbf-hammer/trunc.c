/* trunc <file> <newsize>: set a file's size to <newsize> bytes via SS.Size.
   No stdio, so no cio dependency. os9exec cannot fork this from a host
   directory -- copy it onto the RBF device and run it there. */
extern int open(), close(), _ss_size(), atoi();
extern void exit();
main(argc, argv)
int argc; char **argv;
{
    int path;
    if (argc < 3) exit(1);
    path = open(argv[1], 3);        /* update: read+write */
    if (path < 0) exit(1);
    _ss_size(path, atoi(argv[2]));
    close(path);
    exit(0);
}
