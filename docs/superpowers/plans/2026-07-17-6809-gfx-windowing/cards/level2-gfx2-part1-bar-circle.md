# GFX2 Function Cards: Part 1 (ARC through CIRCLE)

OS-9 Level 2 BASICO9 high-resolution graphics module GFX2 function documentation.
Source: OS-9 Level 2 Operating System Manual, lines 20240-21599.

---

## Window and Graphics Setup

--- CARD ---
id:        gfx2-path-argument-optional
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-syntax
claim:     The path argument to GFX2 functions is optional; when omitted, the function operates on the current window. Syntax: RUN GFX2([path,]"FUNCTION",params...).
context:   None
source:    OS-9 Level 2 Operating System Manual, line 20937-20944
--- END ---

--- CARD ---
id:        gfx2-coordinate-system
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-coordinates
claim:     Graphics drawing coordinates use a 640x192 grid: X ranges 0-639, Y ranges 0-191. Text cursor coordinates relate to characters per line and lines on screen.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 20859-20862
--- END ---

--- CARD ---
id:        gfx2-draw-pointer-vs-text-cursor
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-draw-pointer
claim:     Graphics functions use two independent cursors: the draw pointer (invisible, for graphics functions) starts at 0,0; the text cursor (visible/invisible) is separate and controlled independently. Many graphics functions begin at the draw pointer position.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 20806-20831
--- END ---

--- CARD ---
id:        gfx2-draw-pointer-tracking
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-draw-pointer
claim:     Some GFX2 functions (like LINE) update the draw pointer to the endpoint after execution; functions like BOX and BAR do NOT reset the draw pointer.
context:   LINE endpoint becomes new draw pointer position; BOX and BAR explicitly do not reset it.
source:    OS-9 Level 2 Operating System Manual, lines 20820-20831, 21524-21528, and 21234-21236
--- END ---

---

## Drawing Functions

--- CARD ---
id:        gfx2-arc-basic
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-arc
claim:     ARC draws an arc at current or specified draw position with specified X and Y radius. If xrad==yrad, draws circular arc; otherwise draws elliptical arc.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 21148-21151
--- END ---

--- CARD ---
id:        gfx2-arc-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-arc
claim:     ARC syntax: RUN GFX2(path,"ARC"[,mx,my],xrad,yrad,xcorl,ycor1,xcor2,ycor2). Center coordinates mx,my are optional; if omitted, uses current draw pointer position.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 21144-21169
--- END ---

--- CARD ---
id:        gfx2-arc-coordinate-line
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-arc
claim:     ARC's xcorl/ycor1 and xcor2/ycor2 parameters define an imaginary line relative to the arc center (center point is 0,0 for these coordinates). ARC draws from point closest to first coordinate set, stops at point closest to second set. Reversing the coordinate order changes which side of the line the arc is drawn on.
context:   Coordinates are relative to center, which is conceptually at 0,0 for the line calculation.
source:    OS-9 Level 2 Operating System Manual, lines 21175-21180
--- END ---

--- CARD ---
id:        gfx2-bar-basic
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-bar
claim:     BAR fills a rectangular area defined by two sets of coordinates (diagonal corners). BAR does not reset the draw pointer.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 21220-21226
--- END ---

--- CARD ---
id:        gfx2-bar-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-bar
claim:     BAR syntax: RUN GFX2([path,]"BAR"[,xcor1,ycor1],xcor2,ycor2). First corner coordinates are optional; if omitted, uses current draw pointer position. Second corner coordinates are required.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 21215-21239
--- END ---

---

## Control Functions

--- CARD ---
id:        gfx2-bell-basic
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-bell
claim:     BELL rings the terminal bell (produces a beep through the speaker). No parameters required.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 21306-21307
--- END ---

--- CARD ---
id:        gfx2-bell-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-bell
claim:     BELL syntax: RUN GFX2("BELL"). Path parameter is not shown; unclear if optional.
context:   Example shows path omitted (line 21314); documentation does not clarify if path is optional for BELL.
source:    OS-9 Level 2 Operating System Manual, lines 21303-21314
--- END ---

--- CARD ---
id:        gfx2-blnkon-blnkoff-basic
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-blink
claim:     BLNKON causes all subsequent characters sent to a hardware window to blink. BLNKOFF cancels blinking for new characters only; characters already blinking continue to blink. Blinking does not operate on graphics windows.
context:   OCR unclear: manual shows "BLNEON" and "BLNEOFF" but context strongly suggests "BLNKON" and "BLNKOFF"; the 'E' and 'K' may be OCR artifacts (curly quote or stray character).
source:    OS-9 Level 2 Operating System Manual, lines 21327-21336
--- END ---

--- CARD ---
id:        gfx2-blnkon-blnkoff-hardware-only
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-blink
claim:     BLNKON and BLNKOFF only operate on hardware screens (predefined device windows /W1 through /W7). They have no effect on graphics windows.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 21331-21336
--- END ---

--- CARD ---
id:        gfx2-blnkon-blnkoff-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-blink
claim:     BLNKON syntax: RUN GFX2([path,]"BLNKON"). BLNKOFF syntax: RUN GFX2([path,]"BLNKOFF"). Path parameter appears optional.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 21327-21328
--- END ---

--- CARD ---
id:        gfx2-boldsw-basic
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-bold
claim:     BOLDSW switches bold character display on or off. Default typeface is regular. BOLDSW only works on graphics screens, not hardware text screens.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 21365-21367
--- END ---

--- CARD ---
id:        gfx2-boldsw-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-bold
claim:     BOLDSW syntax: RUN GFX2([path,]"BOLDSW","switch"). Switch parameter is either "ON" or "OFF". Path parameter is optional.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 21362, 21375-21378
--- END ---

--- CARD ---
id:        gfx2-border-basic
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-border
claim:     BORDER resets the palette register that controls window border color (Register 0) to a specified color code.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 21452-21454
--- END ---

--- CARD ---
id:        gfx2-border-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-border
claim:     BORDER syntax: RUN GFX2([path,]"BORDER",color). Color parameter is one of the current palette colors; can be a constant or variable. Path parameter is optional.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 21449, 21463-21464
--- END ---

--- CARD ---
id:        gfx2-box-basic
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-box
claim:     BOX draws a rectangle outline defined by two sets of coordinates (diagonal corners). BOX does not reset the draw pointer.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 21524-21526
--- END ---

--- CARD ---
id:        gfx2-box-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-box
claim:     BOX syntax: RUN GFX2([path,]"BOX"[,xcor1,ycor1],xcor2,ycor2). First corner coordinates are optional; if omitted, uses current draw pointer position. Second corner coordinates are required.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 21519-21539
--- END ---

---

## High-Level Concepts

--- CARD ---
id:        gfx2-memory-requirements
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-windows
claim:     Graphics windows require different amounts of memory depending on format and resolution. Format 5 (640x192, 2 colors) requires 16000 bytes; Format 6 (320x192, 4 colors) requires 16000 bytes; Format 7 (640x192, 4 colors) requires 32000 bytes; Format 8 (320x192, 16 colors) requires 32000 bytes.
context:   From Table 9.6; memory allocation is fixed per format code.
source:    OS-9 Level 2 Operating System Manual, lines 20384-20407
--- END ---

--- CARD ---
id:        gfx2-palette-default-16-colors
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-palette
claim:     OS-9 provides 64 available colors total, but the Color Computer 3 palette hardware can hold only 16 colors at once. The palette has 16 registers (0-15), each holding a color value 0-63.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 20412-20414
--- END ---

--- CARD ---
id:        gfx2-palette-register-meanings
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-palette
claim:     Palette registers have fixed meanings: Register 0 = border color, Register 2 = background color (foreground in some contexts), Register 3 = screen foreground color. Palette registers map to 16 default colors with register-specific meanings.
context:   From Table 9.7; register 0 explicitly named border register.
source:    OS-9 Level 2 Operating System Manual, lines 20428-20443
--- END ---

--- CARD ---
id:        gfx2-dwset-window-setup
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-dwset
claim:     DWSET (Define Window Set) establishes a window and sets its location on screen, size, background color, foreground color, and border color. Must be called before using a window.
context:   Documented in quick reference at line 20964-20966; detailed syntax shown in sample procedure at line 20716.
source:    OS-9 Level 2 Operating System Manual, lines 20964-20966, 20716
--- END ---

--- CARD ---
id:        gfx2-select-window
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-select
claim:     SELECT chooses which window is displayed. Must be called after creating a window to make it active for drawing or text operations.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 20978, 20717
--- END ---

--- CARD ---
id:        gfx2-owset-overlay-window
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-owset
claim:     OWSET (Overlay Window Set) establishes an overlay window on an existing device window. An overlay is a new screen for an existing window, can be same size or smaller than the parent window. The programmer can choose whether to save the contents of the screen covered by the overlay; if saved, the previous screen is redisplayed when the overlay ends.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 20968-20974, 20750-20762
--- END ---

--- CARD ---
id:        gfx2-owend-deallocate-overlay
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-owend
claim:     OWEND deallocates a specified overlay window, potentially restoring the previous screen if it was saved.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 20976
--- END ---

--- CARD ---
id:        gfx2-general-drawing-sequence
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     gfx2-windows
claim:     To use a graphics window from BASIC09: (1) DIM a variable for the path number, (2) OPEN path to the window, (3) SELECT the new window, (4) send commands/data/text through the path, (5) CLOSE the path, (6) SELECT back to original window. Steps 5-6 can be skipped if remaining in the new window.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 20680-20706
--- END ---

--- CARD ---
id:        gfx2-window-persistence
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     gfx2-windows
claim:     A window created with OPEN will disappear automatically when the path is closed. To create a persistent window that survives path closure, precede OPEN with SHELL "INIZ /window" to initialize the device independently.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 20730-20736
--- END ---

--- CARD ---
id:        gfx2-fonts-built-in
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-font
claim:     OS-9 has built-in fonts (character sets) installed in Group 206, Buffers 1, 2, and 3. Users can create custom fonts and instruct BASICO9 to use them via the FONT function.
context:   Buffer 3 contains a special character set for borders/graphics; Buffer 2 contains regular text; Buffer 1 also available.
source:    OS-9 Level 2 Operating System Manual, lines 20867-20879
--- END ---

--- CARD ---
id:        gfx2-font-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-font
claim:     FONT syntax is RUN GFX2([path,]"FONT",groupnum,buffer). Specifies which group and buffer contains the character set to use.
context:   Example at line 20886 shows "FONT", 288, 33 (OCR: 288 might be 2B8 or similar; 33 looks like buffer number).
source:    OS-9 Level 2 Operating System Manual, line 20886
--- END ---

--- CARD ---
id:        gfx2-curxy-text-cursor-position
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-curxy
claim:     CURXY positions the text cursor at specified coordinates. CurHome positions the text cursor at 0,0.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 21079-21081
--- END ---

--- CARD ---
id:        gfx2-setdptr-draw-pointer-position
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-setdptr
claim:     SETDPTR moves the draw pointer to new coordinates. Essential for positioning before executing graphics functions that use the draw pointer.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 21044
--- END ---

--- CARD ---
id:        gfx2-curoff-curon-graphics-cursor
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-cursor-visibility
claim:     CUROFF turns the graphics cursor invisible. CURON turns it visible. The graphics cursor is used with joystick or mouse operations and provides a graphics-application pointer.
context:   None
source:    OS-9 Level 2 Operating System Manual, lines 20833-20836, 21088-21089
--- END ---

---

## Missing / Incomplete Functions

The following functions are listed in the quick reference but NOT documented in detail within this range:

- CIRCLE (listed in quick reference at line 21008, but detailed documentation not in this range)
- Other drawing functions: POINT, LINE, ELLIPSE, FILL, CLEAR
- Get/Put commands: GET, PUT, DEFBUFF, GPLOAD, KILLBUFF
- Configuring commands: COLOR, DEFCOL, PALETTE, PATTERN, LOGIC, GCSET, SCALESW, PUTGC, DRAW
- Other text/cursor commands: ERLINE, EREOELINE, CURRGT, CURLFT, CURUP, CURDWN
- Other font commands: TCHARW, PROPSW, EREOWINDW, CRRTN, REVON, REVOFF, UNDINOM, UNDINOFF, INSLIN, DELLIN
- Window management: DWEND, CWAAREA, DWPROTECTSW

These will require reading beyond line 21599 or from an earlier/later section of the manual.

---

## OCR Correction Notes

1. **BLNKON/BLNKOFF vs BLNEON/BLNEOFF**: Lines 21327-21328 show syntax as "BLNEON" and "BLNEOFF", but the function names in line 21324-21325 describe them as "Character blink on" and "Character blink off". The manual likely intends "BLNKON" and "BLNKOFF"; the 'E' may be OCR noise from a curly quote or formatting artifact.

2. **Coordinate notation**: The manual uses both "xcor1" and "xcorl" (lowercase 'L' vs digit '1') inconsistently, likely OCR artifacts.

3. **Stray characters**: Lines show occasional stray `€`, `¢`, `TM`, or `C` characters where curly quotes or special formatting appears in the original.

