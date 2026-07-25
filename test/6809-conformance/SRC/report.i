* report.i -- shared result-line emitter for the 6809 conformance suite.
* Included via "use report.i" (lwasm) after a module's name string and
* before its entry point. Every test in the suite builds one line in
* OUTBUF and calls emit to write it (plus a trailing CR) to path 1.
*
* copys, prdec and dectbl are lifted unchanged from
* test/6809-live-verification/syscall-iseek.a:126-161 (already proven on
* the guest). crwrite from that same file is NOT reused as-is: it hardcoded
* "ldy #60", writing the full 60-byte buffer including whatever trailing
* junk followed the caller's last character. Here it is replaced by emit,
* which computes the byte count as (end address) - (start of OUTBUF), so
* the line written is exactly what the caller assembled -- no padding, no
* truncation. See task-2-report.md Step 3 for the live-guest proof that
* this arithmetic is correct.
*
* Data-area offsets (U-relative), fixed here and used unchanged by every
* test in the suite. OUTBUF is 128 bytes and that size is load-bearing: a
* result line costs ~39 bytes of fixed overhead before its description
* even starts ("RESULT tNN VERDICT  obs=00000 exp=00000  "), and the
* spec's own example lines run to 73 characters. A smaller buffer here
* would let a normal-length description silently overwrite VALUE, then
* DIGIT, ERRB and PATHNUM -- a corrupted PATHNUM means a botched
* I$Close after the line is emitted, on the recipient's machine, long
* after anyone is watching. Keep OUTBUF at least as long as the longest
* line the format can produce.
OUTBUF  equ 0
VALUE   equ 128
DIGIT   equ 130
ERRB    equ 131
PATHNUM equ 132

* copys -- copy B bytes from Y to X (both advanced past the copied data).
copys   lda ,y+
        sta ,x+
        decb
        bne copys
        rts

* emit -- call with X pointing one past the last character written into
* OUTBUF. Appends a CR, computes the line length as (X after CR) minus
* (start of OUTBUF), and writes exactly that many bytes to path 1 via
* I$WritLn. Returns with the call's own carry/register results from swi2.
* On return X is left at OUTBUF's start (I$WritLn's own calling
* convention), NOT at the line end -- a caller building another line
* must re-establish X (e.g. "leax OUTBUF,u") before the next copys/prdec.
emit    lda #$0D
        sta ,x+              terminate the line with CR
        tfr x,d               D = end address
        leax OUTBUF,u          X = start of buffer (I$WritLn wants it here)
        pshs x                 push start
        subd ,s++              D = end - start = byte count
        tfr d,y                Y = byte count
        lda #1                 path 1 = standard output
        swi2
        fcb $8C                I$WritLn
        rts

* prdec -- append VALUE,u at X as 5 decimal digits, zero-padded (e.g.
* 00216). Fixed width is deliberate: it keeps report lines diffable
* column-for-column regardless of the magnitude of the value printed.
prdec   leay dectbl,pcr
        ldb #5
prdout  pshs b
        clr DIGIT,u
prdin   ldd VALUE,u
        subd ,y
        bcs prddn
        std VALUE,u
        inc DIGIT,u
        bra prdin
prddn   lda DIGIT,u
        adda #$30
        sta ,x+
        leay 2,y
        puls b
        decb
        bne prdout
        rts

dectbl  fdb 10000,1000,100,10,1
