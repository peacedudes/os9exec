# Dogfood report: BASIC09 file/line/char counter on real NitrOS-9 (6809)

Task: write, enter, and live-run a BASIC09 program that creates a file
from typed input, reads it back, and reports line/char counts. Program:
`dogfood-file-line-counter.bas` in this directory. Used the `os9-dev`
skill the way an ordinary user would -- read it, then wrote code from
it, without falling back on outside knowledge of BASIC09 first.

## Did the skill get it right on (near) the first try?

**At the BASIC09 language level: yes, completely.** The 28-statement
procedure was typed into the real interactive editor in one pass and
compiled and ran with **zero** `Error #NNN` messages at any point --
not during entry, not at `RUN`. Every non-obvious syntax choice in the
program came straight from the skill and was correct as documented:

- `WHILE LEN(line$) > 0 DO` -- the skill's "WHILE requires DO" note
  (`basic09-language.md`) stopped me from writing the bare-`WHILE`
  form that's a live syntax error.
- `ON ERROR GOTO 900` targeting a literal numbered line
  (`900 CLOSE #rp`) mixed into otherwise unnumbered structured code --
  the skill's note that "line numbers are just optional prefixes to
  any statement" is what told me this mixing is legal at all.
- The whole read-loop design (`ON ERROR GOTO` around a `LOOP`/
  `READ`/`ENDLOOP` with no `EOF()` pre-check) came directly from the
  skill's "`EOF()` behaves like C's `feof()`" gotcha -- see below,
  this is the single biggest thing the skill saved me from getting
  wrong.
- `CREATE #wp, fname: WRITE` / `OPEN #rp, fname: READ` mode syntax,
  `WRITE`/`READ` semantics (CR-delimited, CR stripped on read) --
  straight from the I/O section, no guessing needed.

**At the REPL-harness level: real friction, but all recoverable from
what's documented.** Two `send` calls timed out:

```
$ ./tools/nitros9repl.sh send "e linecnt"
[TIMEOUT waiting for prompt]
```

and later

```
$ ./tools/nitros9repl.sh send "list linecnt"
[TIMEOUT waiting for prompt]
```

Both are exactly the documented "`send`'s prompt-gate doesn't
recognize a sub-program's own prompt" class of issue from
`6809/using-nitros9-repl.md` -- but that section names only two
confirmed cases (`debug <modulename>`'s `DB:` prompt and `help`'s
`Topic:` prompt). BASIC09's own `E:` edit-mode prompt is a third case
that isn't named there, even though it's arguably the most likely one
an agent doing "ordinary" work will hit. I had to generalize the
documented *pattern* ("switch to `key` for further interaction")
rather than match a listed case verbatim. It worked on the first
retry once I made that generalization, so this cost one wasted call,
not a real debugging detour.

## Was anything wrong, missing, or confusing?

- **Gap, not an error:** `6809/using-nitros9-repl.md`'s "Loading a
  BASIC09 program from a host-authored file" section documents only
  the `tee >progname` + `LOAD` route for getting source onto the 6809
  disk, and frames it as "the equivalent that works here" to the 68k
  REPL's host-file-editing trick. It never mentions or cross-references
  driving BASIC09's own interactive `e` editor directly via `key`
  (which the task asked for, and which the 68k-side
  `common/using-os9exec-repl.md` *does* document generically for its
  own REPL: `B:e test → edit mode, * / E: prompt`, `leading space
  means insert`, `q → back to B:`). I had to infer this 68k-documented
  mechanic would carry over unchanged to 6809 BASIC09 (same product,
  same editor) and confirm it live myself -- it did work exactly as
  described (leading space per inserted line, `q` to exit), but the
  6809-specific reference file gives no indication this path is even
  viable, which could easily send a less-informed agent straight to
  the heavier `tee`/`LOAD` route for a task that didn't need it.
- **New fact this run surfaced, not currently documented anywhere I
  found (checked `basic09-language.md`, `basic09-cheatsheet.md`,
  `gotchas.md`):** a literal semicolon typed inside a `PRINT` string
  literal -- `"Enter lines of text; blank line ends input:"` -- comes
  back with a backslash escape both in `LIST`'s source display *and*
  in the actual `PRINT` output at runtime: `Enter lines of text\;
  blank line ends input:`. Since `;` is also BASIC09's statement
  separator, this is presumably the tokenizer disambiguating a literal
  `;` from a separator even inside a string constant, but the effect
  (an extra literal backslash byte appearing in real program output)
  is surprising and not mentioned in the PRINT/string sections. Purely
  cosmetic for this program (it's a static prompt string, not
  something written to the test file), but worth a line in
  `gotchas.md` since anyone building exact-output text (banners,
  formatted reports) could be bitten by it.
- **Possible refinement to an existing gotcha, not a contradiction:**
  the "after a long CPU-bound guest computation, the next key/send
  command can land corrupted or delayed" gotcha in
  `6809/using-nitros9-repl.md` frames the trigger as a *long
  computation* specifically. In this session, `LIST linecnt`'s output
  (and, separately, the editor's `q`-exit confirmation) arrived several
  commands late, bundled in with a subsequent unrelated command's
  response -- with no long computation involved anywhere, just normal
  editing and a `LIST` call on a ~30-line procedure. The delayed-output
  *symptom* is the same family the doc describes, but the trigger
  condition as currently written ("long CPU-bound computation") didn't
  hold in this case, suggesting the real cause may be closer to
  "several back-to-back `key` calls in quick succession" than to
  computation length specifically.

## What was surprisingly helpful?

- **The `EOF()`-as-`feof()` gotcha was the single most load-bearing
  fact in the whole task.** Coming from most other languages, the
  obvious (wrong) design is `WHILE NOT EOF(#path) DO READ ...
  ENDWHILE`. The skill's explicit warning that this always over-reads
  on the last iteration, plus the recommended fix (`ON ERROR GOTO`
  around the read loop, since `EOF()` only becomes true *after* a
  failed read raises `E$EOF`), is exactly the design used in the final
  program and is why it ran clean on the first try with no Debug Mode
  drop-in on the final read.
- **The `!`-comment-placement rules** ("first token on a line only,"
  and separately "never lead a *file* fed to `LOAD` with a comment on
  6809") gave me the context to deliberately put the header comment
  block in the *saved host file* (this repo's `.bas`, never fed
  through `LOAD`) rather than inside the actual on-disk OS-9 source,
  sidestepping a 6809-specific footgun entirely instead of hitting it.
- **The bash-quoting warning about a literal `$` in `key`/`send`
  arguments** (from `6809/using-nitros9-repl.md`) is why every `key`
  call touching `line$` in this session used single-quoted bash
  arguments. Without that note I'd very likely have double-quoted
  those calls, had bash silently eat the `$line`/`$wp`-adjacent text,
  and spent real time debugging a guest-side "bug" that was actually a
  host-side shell-quoting mistake with zero error on either end of the
  pipe.
- **The fabrication-guard notes** (no `ENDPROC`, no multi-line
  `TYPE`/`ENDTYPE`, no `#`-placeholder `PRINT USING`) didn't come up
  directly in this program, but reading them first meant I never
  second-guessed the plain `END`-and-fall-off-the-end procedure
  structure I did write.

## Overall: substitute for real OS-9/BASIC09 knowledge, or real gaps?

For the **language itself**, the skill functioned as a genuine
substitute here -- a moderately complex, stateful, multi-phase program
(create, write, close, reopen, read-with-EOF-handling, report) compiled
and ran correctly in one pass with no BASIC09-level errors at all. The
EOF/`feof()` gotcha in particular is exactly the kind of non-obvious,
easy-to-get-wrong fact that separates "looks like it should work" from
"actually works," and the skill had it right and prominent.

For **driving the actual live system**, the skill got me most of the
way but not all of the way: the two `send` timeouts required
recognizing an undocumented instance of a documented pattern, and the
6809 REPL reference's own recommended workflow (`tee`+`LOAD`) wasn't
the path that ended up being used, without an obvious pointer toward
the alternative that was. A user following the skill literally,
without a willingness to generalize past its exact wording, would
plausibly have gone down the `tee`+`LOAD` route instead (also a valid
approach, just not the one this task asked for) or burned more than
one timeout on the editor prompt before working out `key` was needed.
So: strong for "will the BASIC09 code I write be correct," weaker for
"will I drive the REPL harness smoothly on the first try" -- the gaps
are real but narrow and mechanical, not conceptual.
