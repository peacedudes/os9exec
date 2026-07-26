# Conformance claims

Every test in this suite asserts one documented Microware behaviour. This
table records, for each test, the document and section the expectation
comes from -- not a description we wrote ourselves, and not our own
curated error-code notes. Facts only; no manual text is copied verbatim.

| Test | Citation | Expected behaviour |
|------|----------|---------------------|
| t01  | OS-9 System Programmers Manual (6809): Chapter 11 introduction (before 11.1), page 73; section 11.3.11 (I$Open); Appendix B.1, error-code table entry for E$PNNF. Microware Training and Education, "OS-9 System Overview" course text, pages 44-45, Table 5. | I$Open's own error list (11.3.11) does not include E$PNNF, but the manual's Chapter 11 introduction explains that an error not in a call's own list was probably returned by another system call the main call made -- here, the file-manager lookup I$Open dispatches to. The Appendix B.1 table independently defines E$PNNF as the code for a pathlist that cannot be located, and the training text states, as the baseline case it compares pipe-opening behaviour against, that opening a file that does not exist returns that same error. Together they establish that `I$Open` of a pathlist with no corresponding file reports E$PNNF (216), not silent success and not a different error code. |
