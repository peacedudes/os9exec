PROCEDURE lostinit
! Dogfood task 2026-07-18: lost-update race, RBF automatic record-locking
! design intent (os9-systems-dev's file-managers.md "Record Locking",
! case 1 -- firsthand design intent from the original Microware
! designer, per user memory): a database-style read-modify-write cycle
! should be safe under concurrent access with ZERO explicit
! application-level locking calls, because a Read in update mode
! automatically locks the record just read, and the following Write on
! that same path automatically releases it. This trio tests exactly
! that: two racer processes (dogfood-lostupdate-incrementer.bas)
! increment a shared counter via unprotected read-modify-write; if
! locking works, no increment is ever lost.
!
! This file: one-shot setup helper. Creates the shared counter file
! fresh, with a binary INTEGER count field (offset 0, 4 bytes on 68k --
! os9-68k-basic-cheatsheet.md) followed by four more INTEGER filler
! fields -- the filler pushes the counter's byte range away from the
! file's true EOF so every racer's WRITE lands mid-file, not at EOF,
! keeping this test on RBF's ordinary per-record lock path (Read locks /
! Write releases) rather than mixing in the separate whole-file EOF-lock
! mechanism (already found broken for a different case/scenario -- see
! dogfood-report-eoflock-2026-07-18.md, unrelated to what this test
! checks).
!
! NOTE: an earlier draft of this file used a TYPE record with a STRING
! field for the filler instead of extra INTEGER fields, and appeared to
! crash BASIC09 outright. That was a false lead: it was the same missing
! "math" trap handler issue described below (any numeric operation --
! not just STRING fields -- triggers the same crash before math is
! loaded), not a real STRING-in-TYPE bug. Left as all-INTEGER here since
! that's what was actually run, not because STRING-in-TYPE is suspect.
!
! Real RBF disk image target: /h1/CLAUDETEST/counter.dat (not a
! host-native mount -- see using-os9exec-repl.md on why that distinction
! matters for RBF locking tests). Run this once before each race;
! CREATE fails with E_CEF (Creating Existing File) if counter.dat
! already exists from a prior run -- DEL it first.
!
! GOTCHA hit and worked around during this pass, not a bug in this
! program: on a freshly booted os9exec session, BASIC09's first
! numeric-variable statement needs to F$TLink the "math" trap handler
! module (TRAP #15); if "math" isn't actually resident, BASIC09 prints
! its own "**** Can't install trap handler ****" banner and exits
! (E_PNNF) -- already documented in this skill's basic09/pack-and-runb.md
! "Can't install trap handler triage" item 1, fix is `load math`. NEW
! finding this pass (added to pack-and-runb.md): the account's own
! /h0/startup line `load -s cio csl math` does NOT actually make
! cio/math resident on this build -- `mdir` confirms only `csl` ends up
! loaded after boot -- while a plain `load math` (no `-s`) from the
! shell works every time. Root cause of the `-s` flag's silent failure
! not chased further (out of scope for this pass); workaround is a
! manual `load math` (and `load cio` if needed) before running any
! BASIC09 program that does real arithmetic.
!
! Literal verified output (68k, os9exec, via tools/os9repl.sh -r,
! account "claude", chx=/dd/CMDS, after `load math` workaround):
!   B:run lostinit
!   lostinit: created counter.dat, count=0
!   Ready
TYPE rectype = count: INTEGER; f1: INTEGER; f2: INTEGER; f3: INTEGER; f4: INTEGER
DIM path: BYTE
DIM rec: rectype
CREATE #path, "/h1/CLAUDETEST/counter.dat": UPDATE
rec.count = 0
SEEK #path, 0
PUT #path, rec
CLOSE #path
PRINT #2, "lostinit: created counter.dat, count=0"
END
