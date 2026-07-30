# Conformance claims — CONF68K

Every test in this suite asserts one documented Microware behaviour. This
table records, for each test, the document and page the expectation comes
from — not a description we wrote ourselves, and not this project's own
curated error-code notes. Facts only; no manual text is copied verbatim.

Unless stated otherwise the source is the **OS-9/68000 Operating System
Technical Manual, version 2.4** (Microware Systems Corporation), read
directly as `os9/txtResources/68k/OS-9_v2.4_Technical_Reference_Manual.txt`
rather than from any summary of it. Its Chapter 1 covers user-state system
calls (`F$…`, paginated `1 - n`), Chapter 2 the I/O system calls (`I$…`,
paginated `2 - n`), and its error-code appendix is paginated
`Error Codes - n`.

A note on what these citations do and do not settle. The manual is the
specification; a running system — including os9exec, and including
NitrOS-9 — is a candidate implementation of it. Where the two disagree the
manual wins and the system has a defect to explain. That is the whole
premise of the suite: a FAIL is a disagreement with the citation in this
table, not with our emulator's behaviour.

| Test | Citation | Expected behaviour |
|------|----------|---------------------|
| t01 | I$Open, POSSIBLE ERRORS, page 2 - 17; error-code appendix entry for E$PNNF, page Error Codes - 5. | I$Open's own error list names E$PNNF, and the appendix defines that code, 216, as the one reported when the specified pathlist cannot be found. No inference is needed: unlike the 6809 manual — where the same claim has to be assembled from the chapter introduction plus the error table, as the 6809 suite's t01 records — the 68k manual lists the error against the call itself. So opening a pathlist with no corresponding file reports 216, neither silent success nor some other code. |
| t02 | I$Read, FUNCTION and POSSIBLE ERRORS, page 2 - 18; error-code appendix entry for E$EOF, page Error Codes - 5. | I$Read's FUNCTION text states that once all data in a file has been read, the next I$Read request returns an end-of-file error, and E$EOF appears in the call's own error list. The appendix assigns 211 to an end-of-file condition met on a read. So a read issued against a file whose content is already exhausted reports 211 — not a zero-byte success. |
| t03 | I$Write, FUNCTION and POSSIBLE ERRORS, page 2 - 30; I$Open access-mode bits, page 2 - 16; error-code appendix entry for E$BMode, page Error Codes - 4. | I$Write's FUNCTION text requires the path to have been opened or created in write or update mode, and names E$BMode in its own error list. I$Open's mode-bit table defines bit 0 as read access, so mode 1 grants read and nothing else. The appendix assigns 203 to an I/O function the file or device is incapable of. Together: writing to a path opened with mode 1 reports 203. |
| t04 | I$Open, FUNCTION text including its NOTE on an all-zero access mode, page 2 - 16. | The manual states directly that a non-directory file may be opened with no mode bits set at all, and gives the purpose: the caller can then examine the file's attributes and size through I$GetStt. It states in the same passage that such a path permits no actual I/O. The first half of that is this test: the open itself must succeed rather than being rejected as a malformed request. |
| t05 | I$Open, FUNCTION text and its all-zero-access-mode NOTE, page 2 - 16; error-code appendix entry for E$BMode, page Error Codes - 4. | The second half of the same passage: a path opened with no mode bits permits no actual I/O, and the appendix defines 203 as the code for an I/O function the file or device cannot perform. So a read on such a path must be refused, and refused with 203. Taken with t04 this pins both ends — a system that rejects the open is wrong, and so is one that allows the read. Worth having as a pair because mode 0 arises by accident as well as by design: POSIX `O_RDONLY` is 0, while OS-9 read access is bit 0 set, so C code ported without adjustment asks for no access at all and looks to the programmer like it asked for read. |
| t06 | I$Read, FUNCTION text, page 2 - 18. | The same FUNCTION paragraph that establishes t02 also states the converse for a read that cannot be fully satisfied: fewer bytes are read than were requested, and this is explicitly not an end-of-file error. So a 64-byte read of a 4-byte file succeeds and reports 4 bytes read. This is the half a test written only around "did the read fail" would miss, which is why it is separate from t02. |
| t07 | I$Seek, INPUT register description and FUNCTION text, page 2 - 20; I$Write, FUNCTION text, page 2 - 30; I$GetStt SS_Size, page 2 - 13. | I$Seek's FUNCTION text states a seek may be made to any position even when the file is not that large, and that subsequent writes automatically expand the file to the required size. I$Write's own text states the same from the writing side: data written past the present end of file expands it. I$GetStt's SS_Size function returns the current file size in d2.l. So seeking to 70000, writing 5 bytes, and asking for the size must give exactly 70005. I$Seek's INPUT description gives the position as d1.l, a full 32-bit byte offset, which is why the seek target is deliberately above 65535: a target inside 16 bits would pass equally well on a system honouring only the low half. |
| t08 | I$Seek, FUNCTION text, page 2 - 20; error-code appendix entry for E$EOF, page Error Codes - 5. | The same I$Seek passage that permits seeking past the end states that reads there return an end-of-file condition, and that a seek by itself does not initiate anything on the media. So a read at position 1000 of a 4-byte file reports 211, and the file is not extended by the attempt. With t07 this establishes the division of labour the manual describes: the seek commits to nothing, the write is what grows the file. |
| t09 | Error-code appendix entry for E$BPNum, page Error Codes - 4; I$Read, POSSIBLE ERRORS, page 2 - 18. | The appendix defines 201 as a path number that was too large or refers to a path that does not exist, and says it can arise whenever a path number is passed to an I/O call. I$Read lists E$BPNum among its own possible errors. So an I$Read quoting a path number that is not open reports 201. |
| t10 | I$Create, FUNCTION text, page 2 - 5; error-code appendix entry for E$CEF, page Error Codes - 5. | I$Create's FUNCTION text states that an error occurs if the pathlist specifies a file name that already exists, and the appendix assigns 218 to an attempt to create a file under a name already present in the directory. So a second create of the same name reports 218 rather than reopening or truncating the existing file. (This is the test that found a real divergence on its first run — see `ourruns.md`.) |
| t11 | I$Seek, FUNCTION text including its NOTE, page 2 - 20. | I$Seek's FUNCTION text states outright that a seek to address zero is the same as a rewind. So after a file has been read to its end, seeking to 0 and reading again returns the file's first byte. The observed value here is that byte rather than an error code, because the failure this test exists to catch — a seek that reports success and leaves the file pointer where it was — produces no error at all. |
| t12 | I$Write, FUNCTION text, page 2 - 30; I$GetStt SS_Size, page 2 - 13. | I$Write's FUNCTION text states that data written past the present end of file automatically expands the file, and SS_Size returns the current size. So six successive 50-byte writes to a new file must leave it reporting 300 bytes. Several writes rather than one deliberately: a system tracking only the largest single write, or only the most recent one, cannot be told apart from a correct one by a single-write test. |

## Claims deliberately not tested here

Recorded so that their absence reads as a decision rather than an oversight.

- **Record locking and the EOF lock.** The richest part of RBF's documented
  behaviour, and four of the 6809 suite's twelve tests. Every one of them
  needs a second process running concurrently with the first, which the
  single-process design used here cannot express. Adding them means a test
  that forks, waits, and reports ERROR rather than guessing when its helper
  does not finish — worth doing, and a larger piece of work than the twelve
  tests above put together.
- **Permission enforcement (E$FNA, 214).** Needs an operator who is neither
  the file's owner nor a member of group 0, so the test can only report SKIP
  when run by the account that built the disk. The 6809 suite's t05 does
  exactly this and is instructive about the cost: most of its citation is
  taken up explaining which identity model applies.
- **F$ID's group/user packing.** The manual (page 1 - 38) gives d1.l as the
  process's group and user number, both word values. That is a genuine 68k
  fact worth having in the suite, but a machine-checkable assertion needs a
  known expected identity, and the suite cannot know which account a
  recipient runs it under.
