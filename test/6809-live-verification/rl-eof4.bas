PROCEDURE rleof4
(* Control for rleof3: dies untrapped on a DIFFERENT error -- opening a
(* file that does not exist, which the kernel answers with 216. If the
(* death banner says 216, runb's untrapped-abort path reports most codes
(* faithfully and mangles specifically the EOF case; if it says 203 the
(* mangling is general and no read error survives an untrapped death.
DIM path: BYTE
PRINT #2, "eof4: about to open a file that is not there"
OPEN #path, "nosuch.zzz": READ
PRINT #2, "eof4: open somehow worked"
CLOSE #path
END
