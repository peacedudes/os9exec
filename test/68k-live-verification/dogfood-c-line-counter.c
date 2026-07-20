/* Dogfood task 2026-07-18: an ordinary C program, not tied to a specific
 * skill claim. Opens a file named by argv[1], reads it and counts total
 * lines and total characters (excluding the CR line terminator), prints
 * a summary, and exits cleanly -- with a graceful error message/nonzero
 * exit if the file doesn't exist.
 *
 * Compiled with the toolchain os9-dev's os9-c-cheatsheet.md documents for
 * 68k (`cc`, forking `cpp`/`c68`/`o68`/`r68`/`l68`), run live on real
 * os9exec (OS-9/68000) via tools/os9repl.sh, from the `claude` account's
 * home dir /dd/USR/CLAUDE (== host os9/h0/USR/CLAUDE, a host-native
 * directory, so this source was authored on the host and converted with
 * `flip -m` per using-os9exec-repl.md rather than typed through `tee`).
 *
 * Test input (filename "testinput.txt", 4 lines, host-verified with
 * `awk`/`wc -c` before ever running this program):
 *   The quick brown fox jumps over the lazy dog.   (44 chars)
 *   Second line has fewer chars.                    (28 chars)
 *   Third.                                            (6 chars)
 *   A fourth and final line here for good measure.  (46 chars)
 *   -> 4 lines, 124 chars excl. terminators, 128 bytes incl. 4 CRs
 *      (matches `wc -c` on the host file: 128 bytes exactly)
 *
 * Literal verified program output (via
 * `/dd/USR/CLAUDE/linecnt testinput.txt`):
 *   File:       testinput.txt
 *   Lines:      4
 *   Characters: 124
 *
 * Error-case output verified live (nonexistent file):
 *   linecnt: can't open 'nonexistentfile.txt'
 * and the no-argument usage case:
 *   usage: /dd/USR/CLAUDE/linecnt filename
 *
 * Deviations from the skill's as-documented recipe, forced by real
 * compile errors (see dogfood-report-c-2026-07-18.md for the full
 * account):
 *   - Dropped `#include <stdlib.h>` -- os9-c-cheatsheet.md's example
 *     doesn't include it, but nothing rules it out either; it isn't on
 *     the Ultra C (`c68`) include search path at all (`cpp` fails with
 *     "can't open /h0/DEFS/stdlib.h"; the file only exists under
 *     DEFS/os9lib and DEFS/GCC2, both gcc2-toolchain-specific per
 *     os9-c-compiler-setup memory, not Ultra C's). `exit()` needs no
 *     prototype under K&R rules and links fine without it.
 *   - Dropped the `_pfltinit()` call os9-clib-reference.md's File I/O
 *     table says printing a `long` requires -- `l68` reports it as an
 *     unresolved symbol (not present in this SDK's `clib.l` at all), and
 *     `printf("%ld", ...)` prints correctly without it. See the report
 *     for the likely explanation (an artifact of `int`==`long` both
 *     being 32-bit on 68k, unlike the 6809 the claim may originate from).
 */
#include <stdio.h>

main(argc, argv)
int argc;
char *argv[];
{
    FILE *fp;
    int c;
    long linecount, charcount;

    if (argc != 2) {
        fprintf(stderr, "usage: %s filename\n", argv[0]);
        exit(1);
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "linecnt: can't open '%s'\n", argv[1]);
        exit(1);
    }

    linecount = 0;
    charcount = 0;

    while ((c = getc(fp)) != EOF) {
        if (c == '\n') {
            linecount++;
        } else {
            charcount++;
        }
    }

    fclose(fp);

    printf("File:       %s\n", argv[1]);
    printf("Lines:      %ld\n", linecount);
    printf("Characters: %ld\n", charcount);

    exit(0);
}
