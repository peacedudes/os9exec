# Conformance claims

Every test in this suite asserts one documented Microware behaviour. This
table records, for each test, the document and section the expectation
comes from -- not a description we wrote ourselves, and not our own
curated error-code notes. Facts only; no manual text is copied verbatim.

| Test | Citation | Expected behaviour |
|------|----------|---------------------|
| t01  | Microware Training and Education, "OS-9 System Overview" course text, section "OS-9 Overview", Table 5 ("What if you try to ..."), row "open a pipe that doesn't exist?"; corroborated by the OS-9 System Programmers Manual (6809), Appendix B.1 "OS-9 Error Codes", entry for E$PNNF (hex $D8, decimal 216) | The training text states, as the baseline case it compares pipe-opening behaviour against, that opening a file that does not exist returns error E$PNNF. The Programmers Manual's error table independently defines E$PNNF as the code returned when the pathlist given to an open request cannot be located. Together they establish that `I$Open` of a pathlist with no corresponding file reports E$PNNF (216), not silent success and not a different error code. |
