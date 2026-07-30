PROCEDURE tprobe
DIM d: STRING[40]
d = DATE$
PRINT #2, "date is ["; d; "]"
PRINT #2, "len="; LEN(d)
END
