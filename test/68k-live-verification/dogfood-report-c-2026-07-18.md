# Dogfood report: hand-written 68k C, ordinary file I/O (2026-07-18)

Task: write a C program for OS-9/68000 that opens a file named by
argv[1], counts total lines and total characters (excluding line
terminators), prints a summary, and exits cleanly -- handling a
missing-file argument gracefully -- using only the `os9-dev` skill plus
live testing via `tools/os9repl.sh`. Working source is
`test/68k-live-verification/dogfood-c-line-counter.c`, with its own
header comment giving the literal verified output.

This specifically stress-tests `c/os9-c-cheatsheet.md`'s toolchain
section (corrected earlier today -- 2026-07-17 -- after a redundancy
pass found it had been showing 6809 tool names in this 68k-labeled
file; never cold-tested against a live compile since) and
`c/os9-clib-reference.md`'s File I/O table.

## Did the skill get me there first-try, or did I have to guess/experiment?

Not first-try on the compile, but close: two real failed compiles before
a clean one, both with clear, specific error messages, and both were
gaps in the skill rather than dead ends requiring exploration. The
overall session was one login, one `chx` fix, three `cc` invocations,
one clean compile, one run that hit a genuine emulator quirk (worked
around immediately with a full path), and two more runs to exercise the
error paths -- no wasted cycles hunting for the toolchain itself.

**What worked immediately, no experimentation needed:**
- Which tools `cc` forks and in what order. The cheatsheet's `Setup` and
  `Pitfalls` sections say `cpp`→`c68`→`o68`→`r68`→`l68`, and the live
  compile's own progress output printed exactly that sequence, in that
  order, on the first successful build. **This is the toolchain-table
  fix working correctly** -- no 6809 tool names (`c.prep`/`c.comp`/etc.)
  appeared anywhere in the real interaction; those only exist in the
  file's clearly-labeled "6809 — do not blend" reference table, which I
  never needed to consult.
- `setenv CLIB /h0/LIB` / `setenv CDEF /h0/DEFS` and parking `chx` at the
  shared `/h0/CMDS` -- exactly as both `Setup` (cheatsheet) and the
  Accounts section of `using-os9exec-repl.md` describe. The `claude`
  account's own `.login` sets `chx /dd/CMDS/claude` (a personal
  subdirectory), which would have broken `cc`'s sub-tool forking exactly
  as the skill warns ("pointing it at the personal directory breaks
  every multi-stage tool") -- I fixed it with one `chx /h0/CMDS` before
  compiling and never hit the "cc: cannot execute the pre-processor"
  symptom the skill describes for this exact mistake.
- `\n` is CR, not LF, and source needs CR-only line endings before
  compiling -- I authored on the host (LF) and ran `flip -m` per
  `using-os9exec-repl.md`'s file-creation section; the compile never
  complained about a garbled single-line source, which is the documented
  symptom of skipping this step.

**What required a real failed compile, with literal errors hit:**

1. **`#include <stdlib.h>` doesn't exist on Ultra C's include path.**
   First compile attempt:
   ```
   /dd/USR/CLAUDE/linecnt.c : line 5 **** can't open /h0/DEFS/stdlib.h (err=216) ****
   #include <stdlib.h>
            ^
   ```
   `find`-ing the file on the host disk image showed it exists, just not
   where `c68`'s `cpp` looks: `DEFS/os9lib/stdlib.h` and
   `DEFS/GCC2/stdlib.h`, both associated with the `gcc2` toolchain per
   this project's own `os9-c-compiler-setup` memory (`-I/h0/DEFS/os9lib`
   for gcc2), not Ultra C's default `CDEF`. Neither
   `os9-c-cheatsheet.md` nor `os9-clib-reference.md` mentions `stdlib.h`
   at all -- not as available, not as unavailable -- so this isn't a
   case of the skill being wrong, just silent on a header a K&R-era
   programmer would reasonably reach for (`exit()` is declared there in
   ANSI C). Fix: drop the include; K&R doesn't require a prototype for
   `exit()` and the program links fine without it. **Worth a line in
   `os9-clib-reference.md`'s Standard Headers table** noting `stdlib.h`
   isn't on the default Ultra C search path.

2. **`_pfltinit()` is an unresolved symbol, not a documented no-op.**
   Second compile attempt (source otherwise complete, still calling
   `_pfltinit()` before the `%ld` printf as
   `os9-clib-reference.md`'s File I/O table instructs -- "printing a
   `long` requires one call to `_pfltinit()` in the program first"):
   ```
   Symbol '_pfltinit' unresolved.
       Referenced by psect 'linecnt_c' in file 'ctmp.000006.r'
   l68: error - unresolved references
   ```
   Dropping the call and recompiling produced a clean link, and the
   resulting binary's `%ld` output was correct (`Lines: 4`,
   `Characters: 124`, matching the independent host count) with no
   `_pfltinit()` call anywhere. **This claim, as currently written for
   68k, is actively wrong** -- not missing, not ambiguous: calling the
   documented function fails to link on this SDK's `clib.l`, and the
   feature it's supposedly gating works fine without it. Plausible root
   cause (not independently verified further): `os9-c-cheatsheet.md`'s
   own data-types table notes `int` and `long` are both 32-bit on 68k,
   unlike the 6809 target that table's sole C-compiler-manual source
   describes (16-bit `int`) -- on 6809, `printf`'s default routines may
   only special-case `int`-width values, making a `long`-formatting
   marker function meaningful; on 68k, where `long` and `int` are the
   same width, there may be nothing left for such a marker to gate. This
   has the same shape as the toolchain-table issue this file's own
   `Pitfalls` section already flags twice ("cross-architecture noise
   from this file's sole C-compiler-manual source being the 1983 6809
   edition") -- `_pfltinit()`/`_prfloat()` look like a third instance of
   the same root problem, not yet caught. **Worth flagging in
   `os9-clib-reference.md`'s File I/O table** as unverified-on-68k at
   minimum, ideally re-tested for whether `_prfloat()` (float/double)
   has the same problem before trusting it either.

**The run itself hit a third, different-shaped problem** -- not a
compile-chain gap, a live emulator/shell oddity:
```
claude: linecnt testinput.txt
shell: can't execute "J"  - Error #000:216 (E_PNNF) Path Name Not Found
```
Expected, per `os9-c-cheatsheet.md`... actually per
`using-os9exec-repl.md`'s Compiling-and-running-C section ("A freshly
compiled program in your data directory can fail to run by bare name
... Run it by full path"), a bare-name run failing was fully
anticipated -- I went straight to the full-path form
(`/dd/USR/CLAUDE/linecnt testinput.txt`) without wasting a cycle
diagnosing it, and it worked correctly. But the *specific error text* is
surprising and doesn't match the skill's description of this failure
mode: it names `"J"` as the thing that couldn't execute, not `linecnt`.
I reproduced it three times (bare `linecnt testinput.txt`, bare
`linecnt` with no args, and a renamed copy `lc testinput.txt`) -- always
`"J"`, never the real command name, and a genuinely-nonexistent command
(`boguscmd123`) reported its own name correctly in the same error
format. This looks like a real, narrow os9exec/shell bug in how the
Path-Not-Found error message renders the failed name for *this specific
kind* of lookup failure (bare name not found via `chx`+`PATH`, as
opposed to a name that plain doesn't exist anywhere) -- distinct from,
and not documented by, the "run by full path" gotcha already in the
skill. Not chased further or fixed (out of scope for this pass, and the
skill's existing guidance already gave the correct workaround before I
needed to understand why); flagging here per this project's own
practice of treating a 68k anomaly as a probable real bug worth a
follow-up, not just a doc note.

## Was anything actually wrong (not just missing) in the skill?

Yes, one clear case: `os9-clib-reference.md`'s claim that printing a
`long` requires `_pfltinit()` first is wrong for this project's 68k
toolchain as stated -- the linker rejects the call outright, and the
feature works without it. The `stdlib.h`-not-on-the-include-path issue
is a gap (nothing asserted, just silent), not a wrong claim. The
toolchain-components table itself -- the thing this dogfood pass was
specifically checking -- held up perfectly: `cpp`/`c68`/`o68`/`r68`/`l68`
in that order, exactly as documented, on the first clean compile.

## Was anything surprisingly helpful?

- The Accounts section's warning about `chx` defaulting to a personal
  subdirectory (`/dd/CMDS/claude`) and breaking multi-stage tool forking
  was exactly right and exactly what I would have hit blind -- I fixed
  `chx` before ever running `cc`, so I never saw the
  "cannot execute the pre-processor" failure mode myself, only avoided
  it because the skill named the cause in advance.
- `using-os9exec-repl.md`'s host-side-editing route (`flip -m`) worked
  perfectly for the source file, and the same tool applied cleanly to
  the *test input* file too (not just source) -- the skill doesn't
  explicitly say `flip` also works for arbitrary data files, but nothing
  in its description was C-source-specific either, and reasoning that
  through saved a `tee`/heredoc detour for a 4-line file that would have
  been fiddly to get exactly right through raw keys.
- The `grep -a` warning wasn't needed this session (all commands were
  driven through `os9repl.sh`, not piped through host `grep`), but is a
  good example of the skill anticipating a specific silent-failure mode
  before it bites.

## Did I hit any REPL sub-prompt or harness friction?

No sub-prompt friction -- this whole session stayed at the ordinary
shell `$`-equivalent prompt (`claude:`), never entered BASIC09, `vi`, or
`debug`, so the gated-`send`-goes-silent-in-a-sub-prompt issue never
applied. The one piece of friction was the `"can't execute J"` anomaly
above, which the skill's existing "run by full path" advice sidestepped
without me needing to understand the error text at all -- a case where
following the documented workaround made root-causing the emulator
oddity optional rather than required to make progress.

## Overall: substitute for knowing OS-9/68k C, or real gaps?

Close to a substitute for the toolchain-mechanics layer, genuinely not
for the C-library-surface layer. Everything about *invoking the
compiler* -- which tools run, in what order, how `CLIB`/`CDEF` and `chx`
need to be set, the CR-only source requirement, how to get a file onto
the emulator in the first place -- was accurate and got me to a clean
compile in exactly the two error-driven iterations a `stdlib.h`-reaching
K&R habit would predict, not more. That's a real, verified win for the
toolchain-table correction this pass was designed to test.

But the *library reference* layer has at least one claim
(`_pfltinit()`) that is flatly wrong for this SDK's 68k `clib.l`, found
only by trying it and reading the linker's error -- a user trusting the
doc without live-testing would have shipped a program that fails to
link, with no way to guess the fix from the error message alone (nothing
in `l68: error - unresolved references` points back at a stale/wrong
doc claim; it looks exactly like any other missing-library problem). The
skill doesn't claim omniscience here and its own `os9-c-compiler-setup`
memory already flags the 6809-manual-as-sole-source problem generally --
but this specific instance hadn't been caught yet, which is exactly what
a cold dogfood pass like this one is for.
