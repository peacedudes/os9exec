! Dogfood task 2026-07-18: RBF EOF-lock design intent, real NitrOS-9 (6809)
! side of the earlier OS-9/68000 os9exec pass (see
! test/68k-live-verification/dogfood-report-eoflock-2026-07-18.md and
! dogfood-eoflock-writer.bas in that directory for the original). This
! project's owner personally designed the *original* Microware EOF-lock
! mechanism at Microware; this run targets the real, original file
! manager (via XRoar + NitrOS-9, tools/nitros9repl.sh) rather than
! os9exec's reimplementation, to see whether the pipe-like design intent
! -- a slow writer (A) appending and a slow reader (B) consuming should
! coordinate "as if it were a pipe," B freely reading everything A has
! flushed and blocking right at the write edge -- holds on the real
! thing. Writer A appends a timestamped record once per real second, 13
! times (shortened from the 68k pass's 40 -- the 68k bug showed up
! immediately with zero successful reads from the first retry, so a
! short run answers the same question), to a shared file on a real RBF
! disk image (/DD, the DriveWire virtual disk's default working
! directory -- NOT /h1/CLAUDETEST, that's a 68k os9exec-specific mount).
! Entered via a `tee >eofwr` heredoc (not the interactive editor) per
! 6809/using-nitros9-repl.md -- note the guest-side file itself must NOT
! be led with this comment block (PROCEDURE must be line 1 or `LOAD`
! fails the whole file); this header is host-archive-only, added after
! the fact, matching this directory's existing convention (see
! dogfood-file-line-counter.bas).
!
! Real, harness-level bug found and fixed while typing this file in:
! tools/nitros9repl.sh's `tmux_escape()` helper replaced every ';' in a
! `key`/`send` argument with '\;', on the mistaken theory that tmux's
! send-keys needed it escaped. Live-verified this was wrong and actively
! harmful -- since the argument is already delivered as one properly
! quoted bash argv element, tmux never re-parses semicolons inside it,
! so the escaping just typed a literal backslash into the guest before
! every semicolon. Any BASIC09 PRINT statement using ';' as an argument
! separator (i.e. nearly all of them) landed on disk with a spurious
! `\` before each one. Fixed by making tmux_escape a no-op; re-verified
! clean with a `PRINT #2, "a="; 1; " b="; 2` probe before retyping this
! file. See the shared report (dogfood-report-eoflock-6809-2026-07-18.md)
! for the full write-up.
!
! Literal verified output (6809, real NitrOS-9, via
! tools/nitros9repl.sh, real-time-paced XRoar -- no -no-ratelimit, so
! DriveWire's OP_TIME keeps DATE$ in real wall-clock seconds --
! launched as `eofwr & eofrdro &`, one combined shell command line,
! reproduced identically across two independent runs):
!   writer: created path=4 err=0 time=08:04:43
!   writer: wrote seq=1 time=08:04:45 err=0
!   writer: wrote seq=2 time=08:04:46 err=0
!   ... (one record per second, err=0 every time) ...
!   writer: wrote seq=13 time=08:04:57 err=0
!   writer: closed at 08:04:57 -- done, 13 records
!
! The writer's own behavior was flawless in every run, exactly like the
! 68k pass: every WRITE succeeded immediately with err=0, landed on the
! expected one-second boundary, CLOSE completed cleanly. All findings
! are on the reader side -- see dogfood-eoflock-reader-6809.bas and the
! shared report.
PROCEDURE eofwr
DIM path: BYTE
DIM i: INTEGER
DIM t, lastt: STRING[8]
DIM line: STRING[24]
CREATE #path, "/DD/EOFTEST/eoflock.dat": WRITE
PRINT #2, "writer: created path="; path; " err="; ERR; " time="; RIGHT$(DATE$,8)
lastt = RIGHT$(DATE$,8)
i = 0
100 t = RIGHT$(DATE$,8)
IF t = lastt THEN 100
lastt = t
i = i + 1
line = t + " seq=" + STR$(i)
WRITE #path, line
PRINT #2, "writer: wrote seq="; i; " time="; t; " err="; ERR
IF i < 13 THEN 100
CLOSE #path
PRINT #2, "writer: closed at "; RIGHT$(DATE$,8); " -- done, 13 records"
END
