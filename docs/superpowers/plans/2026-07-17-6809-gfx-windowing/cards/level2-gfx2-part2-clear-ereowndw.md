# GFX2 Functions: CIRCLE through FILL

Fact cards extracted from OS-9 Level 2 Operating System Manual, GFX2 subroutine reference (lines 21600-22900).

---

## CIRCLE

```
--- CARD ---
id:        gfx2-circle-basic
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-circle
claim:     CIRCLE draws a circle by specifying a radius; if coordinates omitted, uses current draw pointer position as center.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21615
--- END ---
```

```
--- CARD ---
id:        gfx2-circle-coordinates
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-circle
claim:     CIRCLE center coordinates (xcor, ycor) have ranges X: 0-639, Y: 0-191; if specified, overrides current draw pointer as center point.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21627-21629
--- END ---
```

```
--- CARD ---
id:        gfx2-circle-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-circle
claim:     CIRCLE syntax: RUN GFX2(path, "CIRCLE" [,xcor,ycor,radius]) — path required, coordinates optional, radius required if coordinates specified.
context:   OCR: curved quote rendering unclear but reconstructed from context
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21610-21611
--- END ---
```

---

## CLEAR

```
--- CARD ---
id:        gfx2-clear-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-clear
claim:     CLEAR erases the current working area of a window without changing draw pointer location.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21676-21677
--- END ---
```

```
--- CARD ---
id:        gfx2-clear-cursor-reset
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-clear
claim:     CLEAR sets both text cursor and graphics cursor to upper left corner of the window, but draw pointer position is preserved.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21677-21679
--- END ---
```

```
--- CARD ---
id:        gfx2-clear-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-clear
claim:     CLEAR syntax: RUN GFX2(path, "CLEAR") — path parameter specifies the window to clear.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21673
--- END ---
```

---

## COLOR

```
--- CARD ---
id:        gfx2-color-changes
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-color
claim:     COLOR changes any combination of foreground, background, or border colors using palette register numbers; does not affect draw pointer.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21706-21708
--- END ---
```

```
--- CARD ---
id:        gfx2-color-border-global
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     gfx2-color
claim:     Changing border color for any window on a screen changes border color for ALL windows on that same screen.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21723-21725
--- END ---
```

```
--- CARD ---
id:        gfx2-color-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-color
claim:     COLOR syntax: RUN GFX2(path, "COLOR", foreground [, background [, border]]) — all parameters except path are register numbers from palette.
context:   OCR: some quote characters render as TM or other artifacts; reconstructed from syntax pattern
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21701-21702
--- END ---
```

---

## CRRTN (Carriage Return)

```
--- CARD ---
id:        gfx2-crrtn-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-crrtn
claim:     CRRTN sends carriage return to window: cursor moves down one line and to extreme left edge.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21783-21785
--- END ---
```

```
--- CARD ---
id:        gfx2-crrtn-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-crrtn
claim:     CRRTN syntax: RUN GFX2(path, "CRRTN") — path specifies window receiving carriage return.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21780
--- END ---
```

---

## CURDWN (Cursor Down)

```
--- CARD ---
id:        gfx2-curdwn-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-curdwn
claim:     CURDWN moves cursor down one text line; X-coordinate (column position) remains unchanged.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21810-21811
--- END ---
```

```
--- CARD ---
id:        gfx2-curdwn-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-curdwn
claim:     CURDWN syntax: RUN GFX2(path, "CURDWN") — path specifies the target window.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21807
--- END ---
```

---

## CURHOME (Cursor Home)

```
--- CARD ---
id:        gfx2-curhome-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-curhome
claim:     CURHOME moves text cursor to top-left corner of the screen (absolute position).
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21836-21837
--- END ---
```

```
--- CARD ---
id:        gfx2-curhome-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-curhome
claim:     CURHOME syntax: RUN GFX2(path, "CURHOME") — path specifies the window.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21833
--- END ---
```

---

## CURLFT (Cursor Left)

```
--- CARD ---
id:        gfx2-curlft-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-curlft
claim:     CURLFT moves cursor one character to the left.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21865
--- END ---
```

```
--- CARD ---
id:        gfx2-curlft-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-curlft
claim:     CURLFT syntax: RUN GFX2(path, "CURLFT") — path specifies the target window.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21861-21862
--- END ---
```

---

## CUROFF (Cursor Off)

```
--- CARD ---
id:        gfx2-curoff-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-curoff
claim:     CUROFF makes the text cursor invisible.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21891
--- END ---
```

```
--- CARD ---
id:        gfx2-curoff-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-curoff
claim:     CUROFF syntax: RUN GFX2(path, "CUROFF") — path specifies the window.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21888
--- END ---
```

---

## CURON (Cursor On)

```
--- CARD ---
id:        gfx2-curon-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-curon
claim:     CURON makes the text cursor visible.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21917
--- END ---
```

```
--- CARD ---
id:        gfx2-curon-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-curon
claim:     CURON syntax: RUN GFX2(path, "CURON") — path specifies the window.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21914
--- END ---
```

---

## CURRGT (Cursor Right)

```
--- CARD ---
id:        gfx2-currgt-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-currgt
claim:     CURRGT moves cursor one character to the right.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21943
--- END ---
```

```
--- CARD ---
id:        gfx2-currgt-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-currgt
claim:     CURRGT syntax: RUN GFX2(path, "CURRGT") — path specifies the target window.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21940
--- END ---
```

---

## CURUP (Cursor Up)

```
--- CARD ---
id:        gfx2-curup-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-curup
claim:     CURUP moves cursor up one line.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21970
--- END ---
```

```
--- CARD ---
id:        gfx2-curup-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-curup
claim:     CURUP syntax: RUN GFX2(path, "CURUP") — path specifies the window.
context:   OCR: first parameter shows "paih" but reconstructed as "path" from context
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21967
--- END ---
```

---

## CURXY (Cursor X/Y Position)

```
--- CARD ---
id:        gfx2-curxy-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-curxy
claim:     CURXY moves cursor to specified column and row; coordinates are relative to window's character width and depth, not absolute screen coordinates.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21995-21997
--- END ---
```

```
--- CARD ---
id:        gfx2-curxy-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-curxy
claim:     CURXY syntax: RUN GFX2(path, "CURXY", column, row) — path and both coordinates required.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 21992
--- END ---
```

---

## CWAREA (Change Working Area)

```
--- CARD ---
id:        gfx2-cwarea-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-cwarea
claim:     CWAREA restricts output in window to specified area; new area must be same size or smaller than previous working area.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22025-22027
--- END ---
```

```
--- CARD ---
id:        gfx2-cwarea-scaling
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-cwarea
claim:     When CWAREA changes a window's working area, OS-9 scales graphics and text coordinates to new proportions; text characters remain same size.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22027-22029
--- END ---
```

```
--- CARD ---
id:        gfx2-cwarea-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-cwarea
claim:     CWAREA syntax: RUN GFX2(path, "CWAREA", xcor, ycor, sizex, sizey) — xcor/ycor are upper-left corner of new area (relative to original window), sizex/sizey are column/line counts.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22021-22022
--- END ---
```

---

## DEFBUFF (Define Buffer)

```
--- CARD ---
id:        gfx2-defbuff-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-defbuff
claim:     DEFBUFF defines a buffer for GET/PUT operations; remains allocated until KILLBUFF is called.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22107-22120
--- END ---
```

```
--- CARD ---
id:        gfx2-defbuff-group-allocation
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-defbuff
claim:     Each DEFBUFF group allocates 8 kilobytes; 30 bytes used for overhead, leaving 8162 bytes free for buffer data within the group.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22110-22112
--- END ---
```

```
--- CARD ---
id:        gfx2-defbuff-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-defbuff
claim:     DEFBUFF syntax: RUN GFX2("DEFBUFF", group, buffer, size) — NO path parameter; group range 1-199, buffer range 1-255, size range 1-8192 bytes.
context:   OCR: "GETPUT" appears mangled; from context clearly means GET/PUT buffers
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22103-22104
--- END ---
```

```
--- CARD ---
id:        gfx2-defbuff-process-id-strategy
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-defbuff
claim:     Recommended practice: use SYSCALL with Get ID (system call 103F OC hex) to obtain process ID number, then use that as DEFBUFF group number to avoid overlapping.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22136-22141
--- END ---
```

---

## DEFCOL (Define Colors / Default Colors)

```
--- CARD ---
id:        gfx2-defcol-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-defcol
claim:     DEFCOL resets palette registers to their default values; actual hues depend on monitor type.
context:   Manual references Table 9.7 for palette details
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22163-22165
--- END ---
```

```
--- CARD ---
id:        gfx2-defcol-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-defcol
claim:     DEFCOL syntax: RUN GFX2(path, "DEFCOL") — path specifies window to restore default palette.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22160
--- END ---
```

---

## DELLIN (Delete Line)

```
--- CARD ---
id:        gfx2-dellin-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-dellin
claim:     DELLIN deletes the line on which cursor rests and closes the space (remaining lines move up).
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22191-22193
--- END ---
```

```
--- CARD ---
id:        gfx2-dellin-graphics-support
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-dellin
claim:     DELLIN operates on both text and graphics screens.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22192-22193
--- END ---
```

```
--- CARD ---
id:        gfx2-dellin-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-dellin
claim:     DELLIN syntax: RUN GFX2(path, "DELLIN") — path specifies window.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22188
--- END ---
```

---

## DRAW (Draw Polyline)

```
--- CARD ---
id:        gfx2-draw-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-draw
claim:     DRAW draws polyline figures following a series of directional instructions and distances specified in an option list.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22249-22250
--- END ---
```

```
--- CARD ---
id:        gfx2-draw-direction-options
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-draw
claim:     DRAW direction options: Nnum (north/up), Snum (south/down), Enum (east/right), Wnum (west/left), NEnum/NWnum/SEnum/SWnum (diagonals); all move num units.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22266-22273
--- END ---
```

```
--- CARD ---
id:        gfx2-draw-rotation-option
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-draw
claim:     DRAW rotation option A: Aval rotates draw axis; values: 0=normal, 1=90 degrees, 2=180 degrees, 3=270 degrees.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22274-22278
--- END ---
```

```
--- CARD ---
id:        gfx2-draw-vector-option
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-draw
claim:     DRAW vector option U: Uxcor,ycor draws relative vector to specified coordinates (relative to current draw pointer); draw pointer location does NOT change.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22279-22283
--- END ---
```

```
--- CARD ---
id:        gfx2-draw-blank-option
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-draw
claim:     DRAW blank option B: Bxcor,ycor moves cursor without drawing (blank line); coordinates relative to current position; offscreen moves hide subsequent lines.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22293-22297
--- END ---
```

```
--- CARD ---
id:        gfx2-draw-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-draw
claim:     DRAW syntax: RUN GFX2(path, "DRAW", option_list) — option_list is string of characters and numbers, separable by spaces or commas; commas required between B and U coordinates.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22245-22253
--- END ---
```

---

## DWEND (Device Window End)

```
--- CARD ---
id:        gfx2-dwend-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-dwend
claim:     DWEND deallocates a device window initialized with DWSET; if last device window on screen, returns screen memory to system.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22345-22348
--- END ---
```

```
--- CARD ---
id:        gfx2-dwend-positioning
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-dwend
claim:     DWEND automatically positions to the next device window (similar to pressing CLEAR) after deallocation.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22348-22349
--- END ---
```

```
--- CARD ---
id:        gfx2-dwend-redefine-pattern
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-dwend
claim:     DWEND can be used with DWSET to redefine a device window to a different type.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22350-22351
--- END ---
```

```
--- CARD ---
id:        gfx2-dwend-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-dwend
claim:     DWEND syntax: RUN GFX2(path, "DWEND") — path can be constant or variable, specifies window to end.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22342
--- END ---
```

---

## DWPROTSW (Device Window Protection Switch)

```
--- CARD ---
id:        gfx2-dwprotsw-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-dwprotsw
claim:     DWPROTSW unprotects one device window and allows other windows to be placed on top of it; normally OS-9 Color Computer 3 uses protected windowing that forbids overlap.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22418-22426
--- END ---
```

```
--- CARD ---
id:        gfx2-dwprotsw-gotcha
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     gfx2-dwprotsw
claim:     Removing window protection with DWPROTSW can destroy contents of unprotected window; use with care.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22425-22426
--- END ---
```

```
--- CARD ---
id:        gfx2-dwprotsw-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-dwprotsw
claim:     DWPROTSW syntax: RUN GFX2(path, "DWPROTSW", switch) — switch is OFF (unprotect) or ON (protect, default).
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22415
--- END ---
```

---

## DWSET (Device Window Set)

```
--- CARD ---
id:        gfx2-dwset-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-dwset
claim:     DWSET defines a device window by specifying format, location, size, and colors; normally used after opening a path to a window.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22459-22461
--- END ---
```

```
--- CARD ---
id:        gfx2-dwset-format-codes
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-dwset
claim:     DWSET format parameter selects screen type codes (see Table 9.6 in manual for available formats).
context:   Manual references external table; format codes not enumerated in this section
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22468-22470
--- END ---
```

```
--- CARD ---
id:        gfx2-dwset-coordinates
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-dwset
claim:     DWSET xcor, ycor parameters specify upper-left corner of screen in character columns and rows (not pixel coordinates).
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22472-22474
--- END ---
```

```
--- CARD ---
id:        gfx2-dwset-dimensions
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-dwset
claim:     DWSET width parameter is window width in characters; length parameter is window depth in lines.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22476-22477
--- END ---
```

```
--- CARD ---
id:        gfx2-dwset-colors
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-dwset
claim:     DWSET foreground, background, border parameters are codes (palette register numbers) specifying window's initial color scheme.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22478-22481
--- END ---
```

```
--- CARD ---
id:        gfx2-dwset-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-dwset
claim:     DWSET syntax: RUN GFX2(path, "DWSET", format, xcor, ycor, width, length, foreground, background, border) — all parameters required.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22454-22455
--- END ---
```

---

## ELLIPSE

```
--- CARD ---
id:        gfx2-ellipse-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-ellipse
claim:     ELLIPSE draws an ellipse; center can be at current draw pointer position or at specified X,Y coordinates.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22542-22543
--- END ---
```

```
--- CARD ---
id:        gfx2-ellipse-coordinates
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-ellipse
claim:     ELLIPSE coordinate ranges: X: 0-639, Y: 0-191; if coordinates omitted, uses current draw pointer position.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22544-22555
--- END ---
```

```
--- CARD ---
id:        gfx2-ellipse-radii
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-ellipse
claim:     ELLIPSE xrad, yrad parameters specify the ellipse's length radius and height radius respectively.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22557
--- END ---
```

```
--- CARD ---
id:        gfx2-ellipse-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-ellipse
claim:     ELLIPSE syntax: RUN GFX2(path, "ELLIPSE" [,xcor,ycor], xrad, yrad) — coordinates optional, radii required.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22537-22538
--- END ---
```

---

## EREOLINE (Erase to End of Line)

```
--- CARD ---
id:        gfx2-ereoline-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-ereoline
claim:     EREOLINE deletes the portion of the current line from cursor position to the right edge of window.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22613-22614
--- END ---
```

```
--- CARD ---
id:        gfx2-ereoline-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-ereoline
claim:     EREOLINE syntax: RUN GFX2(path, "EREOLINE") — path specifies window.
context:   OCR: function name shows as "EREOL INE" with space; confirmed as EREOLINE from context
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22610
--- END ---
```

---

## EREOWNDW (Erase to End of Window)

```
--- CARD ---
id:        gfx2-ereowndw-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-ereowndw
claim:     EREOWNDW deletes all lines from cursor position to bottom of window.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22658-22659
--- END ---
```

```
--- CARD ---
id:        gfx2-ereowndw-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-ereowndw
claim:     EREOWNDW syntax: RUN GFX2(path, "EREOWNDW") — path specifies window.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22655
--- END ---
```

---

## ERLINE (Erase Line without Space Closure)

```
--- CARD ---
id:        gfx2-erline-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-erline
claim:     ERLINE deletes current line (cursor on) WITHOUT closing the space (distinguishing feature from DELLIN).
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22686-22687
--- END ---
```

```
--- CARD ---
id:        gfx2-erline-vs-dellin
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-erline
claim:     ERLINE differs from DELLIN: ERLINE does not close space after deletion, DELLIN does close space.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22686-22687
--- END ---
```

```
--- CARD ---
id:        gfx2-erline-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-erline
claim:     ERLINE syntax: RUN GFX2(path, "ERLINE") — path specifies window.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22683
--- END ---
```

---

## FILL (Fill/Paint Area)

```
--- CARD ---
id:        gfx2-fill-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-fill
claim:     FILL paints an area with current foreground color, filling all connected pixels of the same color as the pixel under draw pointer.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22739-22741
--- END ---
```

```
--- CARD ---
id:        gfx2-fill-coordinates
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-fill
claim:     FILL xcor, ycor parameters are optional; if specified, they reposition draw pointer before FILL begins; if omitted, uses current draw position.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22749-22752
--- END ---
```

```
--- CARD ---
id:        gfx2-fill-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-fill
claim:     FILL syntax: RUN GFX2(path, "FILL" [,xcor,ycor]) — path required, coordinates optional.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22736
--- END ---
```

---

## FONT (Define Font Buffer)

```
--- CARD ---
id:        gfx2-font-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-font
claim:     FONT defines a buffer from which BASICO9 retrieves character font (style) for graphics screen; font must be loaded into buffer before FONT is called.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22798-22808
--- END ---
```

```
--- CARD ---
id:        gfx2-font-graphics-only
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-font
claim:     FONT works only on graphics screens, not text screens.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22804
--- END ---
```

```
--- CARD ---
id:        gfx2-font-standard-fonts
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-font
claim:     Standard fonts available: Sidfonts file in SYS directory contains three fonts in Group 200 at Buffers 1, 2, and 3; custom fonts can also be created.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22801-22803
--- END ---
```

```
--- CARD ---
id:        gfx2-font-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-font
claim:     FONT syntax: RUN GFX2(path, "FONT", group, buffer) — path required, group is buffer group number, buffer is buffer number containing font.
context:   OCR: syntax line broken across multiple lines with formatting noise; reconstructed from context
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22793-22795
--- END ---
```

---

## GCSET (Graphics Cursor Set)

```
--- CARD ---
id:        gfx2-gcset-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-gcset
claim:     GCSET defines a buffer from which BASICO9 retrieves graphics cursor, allowing definition of custom cursor for graphics operations.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22836-22837
--- END ---
```

```
--- CARD ---
id:        gfx2-gcset-no-path
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-gcset
claim:     GCSET syntax does NOT include path parameter: RUN GFX2("GCSET", group, buffer) — unique among graphics functions.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22832-22833
--- END ---
```

```
--- CARD ---
id:        gfx2-gcset-off-with-zero
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-gcset
claim:     GCSET group number 0 turns graphics cursor off; group 0 is special case for disabling cursor.
context:   OCR: group number shown as © in source, resolved as 0 from context (cursor off feature)
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22838-22839
--- END ---
```

```
--- CARD ---
id:        gfx2-gcset-prerequisites
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-gcset
claim:     GCSET requires: (1) merge of Stdcur file from SYS directory to window, (2) GCSET command execution to display graphics cursor.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22840-22841
--- END ---
```

---

## GET (Get Block from Window)

```
--- CARD ---
id:        gfx2-get-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-get
claim:     GET saves a window area to a Get/Put buffer; use PUT to restore the saved image to the window.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22873-22874
--- END ---
```

```
--- CARD ---
id:        gfx2-get-buffer-creation
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-get
claim:     GET automatically creates buffer if not previously defined with DEFBUFF; if data exceeds predefined buffer size, GET truncates data to buffer size.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22874-22878
--- END ---
```

```
--- CARD ---
id:        gfx2-get-data-size-constraint
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-get
claim:     GET data must be same size or smaller than buffer; if larger, BASICO9 truncates data to buffer size.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22876-22878
--- END ---
```

```
--- CARD ---
id:        gfx2-get-coordinates
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-get
claim:     GET xcor, ycor specify upper-left corner of window image to save; X range 0-639, Y range 0-191.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22888-22891
--- END ---
```

```
--- CARD ---
id:        gfx2-get-size-parameters
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-get
claim:     GET xsize is horizontal size of window section to save; ysize is vertical size.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22893-22897
--- END ---
```

```
--- CARD ---
id:        gfx2-get-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-get
claim:     GET syntax: RUN GFX2(path, "GET", group, buffer, xcor, ycor, xsize, ysize) — all parameters required; group range 1-199, buffer range 1-255.
context:   None
source:    OS-9 Level 2 Operating System Manual, GFX2 section, line 22869-22870
--- END ---
```

---

# Summary

This document contains 86 fact cards extracted from the OS-9 Level 2 Operating System Manual, covering GFX2 subroutine functions from CIRCLE through GET (lines 21600-22900).

## Functions Completely Extracted (28 total)
- CIRCLE (21608-21666)
- CLEAR (21671-21696)
- COLOR (21699-21770)
- CRRTN (21780-21802)
- CURDWN (21807-21827)
- CURHOME (21833-21855)
- CURLFT (21859-21882)
- CUROFF (21886-21907)
- CURON (21912-21933)
- CURRGT (21938-21960)
- CURUP (21965-21983)
- CURXY (21987-22015)
- CWAREA (22019-22097)
- DEFBUFF (22101-22152)
- DEFCOL (22158-22180)
- DELLIN (22186-22240)
- DRAW (22243-22337)
- DWEND (22340-22410)
- DWPROTSW (22413-22448)
- DWSET (22452-22525)
- ELLIPSE (22535-22599)
- EREOLINE (22608-22650)
- EREOWNDW (22653-22677)
- ERLINE (22681-22730)
- FILL (22734-22785)
- FONT (22791-22826)
- GCSET (22830-22862)
- GET (22867-22901)

## Functions Unable to Extract Cleanly
None — all 28 functions in the 21600-22900 range were successfully extracted and documented despite OCR artifacts.

## OCR Issues Encountered and Resolved
1. **Quote character mangling**: Curved quotes rendered as `TM`, `C`, `"`, `€`, `¢` — reconstructed from syntax context
2. **Letter confusion**: `l`/`1`/`I` and `O`/`0` — used frequency analysis and parameter order to resolve
3. **Function name spacing**: "EREOL INE" split across lines — rejoined from heading context
4. **Typos in examples**: E.g., "CERCLE" vs "CIRCLE" — verified against main function header and confirmed as OCR artifact
5. **Stray characters in code listings**: E.g., `GFX2€`, `GFX2¢` — these appear in program examples (not extracted verbatim) and are OCR noise

All fact cards are formatted as atomic, single-claim units and avoid verbatim code reproduction as instructed.
