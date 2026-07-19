PROCEDURE eofvis
! Smallest test that shows the RBF cross-path visibility bug, and the
! narrowest one: ONE process, two paths to one file, no concurrency and
! no timing at all. The reader path is opened while the file is still
! empty -- exactly the situation a reader following a live writer is in
! -- and then the writer appends a record. A correct RBF lets the reader
! see it; it is the same file, and the write has completed.
!
! Before the fix this printed "FAIL err=211" (E$EOF): the reader path
! kept the FD sector it cached at open, so it believed the file was
! still empty for the rest of its life, no matter what any other path
! wrote. Three separate things had to be true for it to see the record:
! the writer had to publish the new file size in the FD (os9exec only
! did that at CLOSE), the writer had to flush its own sector buffer
! (per-path in os9exec, per-drive in real RBF), and the reader had to
! re-read the FD rather than trust its open-time copy.
!
! Deliberately NOT the ghost lock: a single process cannot demonstrate
! that half. A lone process sleeping at end-of-file is the only thing
! that could ever wake itself, which is a genuine deadlock -- see
! dogfood-eoflock-writer.bas / -reader-readonly.bas for the two-process
! version, and dogfood-eoflock-deadlock.bas for the self-conflict case.
!
! Runs on a real RBF image (/h1), not a host-native dir: host-native
! paths have no FD of their own, so this bug cannot exist there and the
! test would pass for the wrong reason.
! Note ON ERROR GOTO 0 as the handler's first act: without it an error
! raised inside the handler re-enters it, and the real first failure is
! lost behind an endless retry of the second one.
DIM w, r: BYTE
DIM line: STRING[20]
ON ERROR GOTO 800
CREATE #w, "vis.dat": WRITE
PRINT "eofvis: created writer err="; ERR
OPEN #r, "vis.dat": READ
PRINT "eofvis: opened reader err="; ERR
WRITE #w, "VISIBLE"
PRINT "eofvis: wrote err="; ERR
READ #r, line
PRINT "eofvis: OK read="; line
GOTO 900
800 ON ERROR GOTO 0
PRINT "eofvis: FAIL err="; ERR
900 END
