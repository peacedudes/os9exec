# GFX2 Reference: GPLOAD through UNDLNOFF

This fact-card file covers GFX2 subroutine functions GPLOAD through UNDLNOFF (alphabetically) from the OS-9 Level 2 Operating System Manual, Chapter 9.

---

## GPLOAD

--- CARD ---
id:        gfx2-gpload-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-gpload
claim:     GPLOAD loads a buffer with image data that PUTBLK (or PUT) can use for window displays.
context:   Manual uses both "PUTBLK" in function description and "PUT" in cross-reference; they are synonymous in this context.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 22966
--- END ---

--- CARD ---
id:        gfx2-gpload-buffer-creation
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-gpload
claim:     If the Get/Put buffer does not already exist, GPLOAD creates it. If it already exists, the size of new data loaded must not exceed the existing buffer size.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 22968
--- END ---

--- CARD ---
id:        gfx2-gpload-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-gpload
claim:     GPLOAD syntax: RUN GFX2("GPLOAD", group, buffer, format, xdim, ydim, size)
context:   OCR: original shows corruption in "RUN GFX2(*GPLOAD"" but corrected from context and parameter documentation.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 22960
--- END ---

--- CARD ---
id:        gfx2-gpload-param-group
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-gpload
claim:     GPLOAD parameter 'group': integer in range 1-199 to let you group buffers. Buffer group numbers 0 and 200-255 are reserved for OS-9 system use.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 22973
--- END ---

--- CARD ---
id:        gfx2-gpload-param-buffer
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-gpload
claim:     GPLOAD parameter 'buffer': integer in range 1-255 that you assign to identify the buffer you create.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 22976
--- END ---

--- CARD ---
id:        gfx2-gpload-param-format
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-gpload
claim:     GPLOAD parameter 'format': the type code of the screen format (see Table 9.4 for valid codes).
context:   Cross-reference to external table; exact format codes not extracted here.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 22979
--- END ---

--- CARD ---
id:        gfx2-gpload-param-dimensions
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-gpload
claim:     GPLOAD parameters 'xdim' and 'ydim': the X (horizontal) and Y (vertical) dimensions of the stored block, respectively.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 22982-22985
--- END ---

--- CARD ---
id:        gfx2-gpload-param-size
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-gpload
claim:     GPLOAD parameter 'size': the size of the buffer in bytes. Buffer size can be 1 to 8 kilobytes, depending on available memory.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 22987
--- END ---

---

## INSLIN

--- CARD ---
id:        gfx2-inslin-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-inslin
claim:     INSLIN (insert line) moves the window lines at and below the cursor down one line.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23012
--- END ---

--- CARD ---
id:        gfx2-inslin-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-inslin
claim:     INSLIN syntax: RUN GFX2({path,}"INSLIN")
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23009
--- END ---

--- CARD ---
id:        gfx2-inslin-param-path
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-inslin
claim:     INSLIN parameter 'path': the route to the window in which you want a blank line inserted.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23018
--- END ---

--- CARD ---
id:        gfx2-inslin-paired-dellin
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     gfx2-inslin
claim:     INSLIN is typically paired with DELLIN (delete line); the manual shows a sample procedure using both to create animated mouth movement.
context:   DELLIN itself is not documented in this range, only referenced via cross-example at line 23030 and line 23069. See paired function extraction for DELLIN details.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 23030, 23069
--- END ---

---

## KILLBUFF

--- CARD ---
id:        gfx2-killbuff-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-killbuff
claim:     KILLBUFF deallocates the indicated Get/Put buffer, freeing its memory.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23086
--- END ---

--- CARD ---
id:        gfx2-killbuff-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-killbuff
claim:     KILLBUFF syntax: RUN GFX2("KILLBUFF", group, buffer)
context:   OCR: original shows "KILLEUFE" which is mangled OCR for "KILLBUFF". Corrected via cross-reference to parameter names and example at line 23105.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23084
--- END ---

--- CARD ---
id:        gfx2-killbuff-param-group
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-killbuff
claim:     KILLBUFF parameter 'group': the group number of the buffer to deallocate, in range 1-199. Buffer group numbers 0 and 200-255 are reserved for OS-9 system use.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23094
--- END ---

--- CARD ---
id:        gfx2-killbuff-param-buffer
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-killbuff
claim:     KILLBUFF parameter 'buffer': the buffer number to deallocate, in range 1-255.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23099
--- END ---

--- CARD ---
id:        gfx2-killbuff-cross-reference
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-killbuff
claim:     KILLBUFF cross-references DEFBUFF, PUT, GET, and GPLOAD for related Get/Put buffer operations.
context:   Manual directs: "For more information on Get/Put buffers, see DEFBUFF, PUT, GET, and GPLOAD."
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23089
--- END ---

---

## LINE

--- CARD ---
id:        gfx2-line-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-line
claim:     LINE draws a line in one of two ways: (1) from the current draw pointer to specified X,Y coordinates, or (2) from specified beginning X,Y coordinates to specified ending X,Y coordinates.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23168
--- END ---

--- CARD ---
id:        gfx2-line-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-line
claim:     LINE syntax: RUN GFX2({path,}"LINE"[,xcor1,ycor1,xcor2,ycor2])
context:   OCR: original shows "LINE"(" with unmatched paren; corrected syntax shows xcor1,ycor1 as optional (two-endpoint mode) with xcor2,ycor2 required as ending point(s).
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23163
--- END ---

--- CARD ---
id:        gfx2-line-param-path
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-line
claim:     LINE parameter 'path': the route to the window in which you want to draw a line.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23178
--- END ---

--- CARD ---
id:        gfx2-line-param-coordinates
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-line
claim:     LINE parameters 'xcor1' and 'ycor1': optional beginning X and Y coordinates. Parameters 'xcor2' and 'ycor2': ending X and Y coordinates.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 23181-23183
--- END ---

--- CARD ---
id:        gfx2-line-coordinate-ranges
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-line
claim:     LINE coordinate ranges: X-coordinates 0-639, Y-coordinates 0-191 (inferred from other graphics functions; manual does not explicitly list coordinate ranges for LINE).
context:   OCR unclear: manual does not provide explicit range bounds for LINE. Ranges extrapolated from POINT (lines 23581-23582) and other graphics functions which consistently use same coordinate space.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23163
--- END ---

---

## LOGIC

--- CARD ---
id:        gfx2-logic-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-logic
claim:     LOGIC causes BASICO9 to perform the specified logic function on all data bits used by subsequent drawing functions. Once set, the logic function remains in effect until you turn LOGIC off.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23235
--- END ---

--- CARD ---
id:        gfx2-logic-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-logic
claim:     LOGIC syntax: RUN GFX2("LOGIC", "function")
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23232
--- END ---

--- CARD ---
id:        gfx2-logic-function-values
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-logic
claim:     LOGIC parameter 'function' accepts: OFF (no logic), AND (performs AND logic), OR (performs OR logic), XOR (performs XOR logic).
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 23243-23247
--- END ---

--- CARD ---
id:        gfx2-logic-state-persistence
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     gfx2-logic
claim:     Once LOGIC is set to a non-OFF mode, it affects all subsequent drawing operations until explicitly turned OFF. The sample demonstrates color-change effects via XOR over a multicolored background.
context:   Sample program (line 23259-23288) shows XOR mode changing colors of overlapping horizontal bar across vertical bars. This is not a local per-draw mode; it's global state.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23238
--- END ---

---

## OWSET

--- CARD ---
id:        gfx2-owset-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-owset
claim:     OWSET (establish overlay window) creates an overlay window on a previously existing device window and reconfigures the current device window paths to use a new area of the screen.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23305
--- END ---

--- CARD ---
id:        gfx2-owset-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-owset
claim:     OWSET syntax: RUN GFX2({path,}"OWSET", save_switch, xpos, ypos, xsize, ysize, foreground, background)
context:   OCR: original shows "RUN GFX2{ipath,|" which appears to be corruption for "{path,}". Corrected from parameter documentation.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23300
--- END ---

--- CARD ---
id:        gfx2-owset-param-path
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-owset
claim:     OWSET parameter 'path': the route to the window in which you want to set an overlay.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23312
--- END ---

--- CARD ---
id:        gfx2-owset-param-save-switch
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-owset
claim:     OWSET parameter 'save_switch': either 0 or 1. A value of 0 tells BASICO9 not to save the overlaid area. A value of 1 tells BASICO9 to save the overlaid area and restore it when the new window closes.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23315
--- END ---

--- CARD ---
id:        gfx2-owset-param-position
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-owset
claim:     OWSET parameters 'xpos' and 'ypos': the character column and row in which to start the new window (upper left corner).
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 23320-23324
--- END ---

--- CARD ---
id:        gfx2-owset-param-size
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-owset
claim:     OWSET parameters 'xsize' and 'ysize': the width of the new window in characters and the depth in rows, respectively.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 23326-23327
--- END ---

--- CARD ---
id:        gfx2-owset-param-colors
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-owset
claim:     OWSET parameters 'foreground' and 'background': the foreground and background colors of the new window.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 23328-23329
--- END ---

--- CARD ---
id:        gfx2-owset-paired-owend
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     gfx2-owset
claim:     OWSET is paired with OWEND (overlay window end); the sample shows creating progressively smaller overlay windows and then closing them with OWEND calls.
context:   OWEND is not documented in this range but is referenced in sample code at lines 23374. See paired function extraction for OWEND.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23374
--- END ---

---

## PALETTE

--- CARD ---
id:        gfx2-palette-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-palette
claim:     PALETTE sets palette colors, letting you install any of the Color Computer's 64 colors in the palette for use with text and graphics.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23391
--- END ---

--- CARD ---
id:        gfx2-palette-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-palette
claim:     PALETTE syntax: RUN GFX2({path,}"PALETTE", register, color)
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23388
--- END ---

--- CARD ---
id:        gfx2-palette-param-path
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-palette
claim:     PALETTE parameter 'path': the route to the window where you want to change palette colors.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 23398-23400
--- END ---

--- CARD ---
id:        gfx2-palette-param-register
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-palette
claim:     PALETTE parameter 'register': the number of the palette register in which you want to install a new color. Valid range not explicitly specified in manual.
context:   Manual mentions 64 total available colors but does not specify register numbering range (0-63, 1-64, or other).
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23402
--- END ---

--- CARD ---
id:        gfx2-palette-param-color
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-palette
claim:     PALETTE parameter 'color': the code of the new color you want to install.
context:   Manual does not define color code values or list them; cross-reference to Table 9.4 is likely needed.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23405
--- END ---

--- CARD ---
id:        gfx2-palette-cologr-interaction
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     gfx2-palette
claim:     The sample program also uses COLGR to set colors; PALETTE and COLOR/COLGR are complementary mechanisms for palette/color management.
context:   Sample shows COLGR calls (lines 23422, 23427, 23443, 23452, 23455) mixed with PALETTE usage (line 23468). Exact relationship not fully clarified.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 23422-23468
--- END ---

---

## PATTERN

--- CARD ---
id:        gfx2-pattern-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-pattern
claim:     PATTERN selects the contents of a preloaded Get/Put buffer as a pattern for graphics functions. BASICO9 uses the selected pattern with all draw commands until you change it or turn off the pattern function.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23490
--- END ---

--- CARD ---
id:        gfx2-pattern-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-pattern
claim:     PATTERN syntax: RUN GFX2({path,}"PATTERN", group, buffer)
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23486
--- END ---

--- CARD ---
id:        gfx2-pattern-param-path
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-pattern
claim:     PATTERN parameter 'path': the route to the window in which you want to use a new graphics pattern.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23515
--- END ---

--- CARD ---
id:        gfx2-pattern-param-group-buffer
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-pattern
claim:     PATTERN parameters 'group' and 'buffer': select which preloaded Get/Put buffer to use as the pattern. Specifying group=0 and buffer=0 turns off the pattern function.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 23509-23510, 23518-23522
--- END ---

--- CARD ---
id:        gfx2-pattern-size-by-format
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-pattern
claim:     PATTERN buffer size requirements depend on screen format: Mode 02=32 bytes (1 bit per pel), Mode 04=64 bytes (2 bits per pel), Mode 16=128 bytes (4 bits per pel).
context:   Table shows Color Mode, Pattern Array Size, Bits Per Pel. The pattern array is a 32x8 pel representation. If buffer is larger than required, PATTERN ignores extra bytes.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 23495-23510
--- END ---

--- CARD ---
id:        gfx2-pattern-considers-color-mode
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-pattern
claim:     The pattern array takes the current color mode into consideration to define the number of bits per pel and pels per byte.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23504
--- END ---

---

## POINT

--- CARD ---
id:        gfx2-point-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-point
claim:     POINT sets the pixel at the current draw pointer position or at the specified coordinates to the current foreground color. If you do not specify coordinates, POINT sets the pixel at the draw pointer.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23569
--- END ---

--- CARD ---
id:        gfx2-point-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-point
claim:     POINT syntax: RUN GFX2({path,}"POINT"[,xcor,ycor])
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23566
--- END ---

--- CARD ---
id:        gfx2-point-param-path
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-point
claim:     POINT parameter 'path': the route to the window in which you want to turn on the specified pixels.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23577
--- END ---

--- CARD ---
id:        gfx2-point-param-coordinates
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-point
claim:     POINT parameters 'xcor' and 'ycor': optional coordinates for the POINT function. X-coordinates are in range 0-639. Y-coordinates are in range 0-191.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 23580-23582
--- END ---

---

## PROPSW

--- CARD ---
id:        gfx2-propsw-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-propsw
claim:     PROPSW (proportional space switch) enables or disables the automatic proportional spacing of characters on graphic screens.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23640
--- END ---

--- CARD ---
id:        gfx2-propsw-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-propsw
claim:     PROPSW syntax: RUN GFX2({path,}"PROPSW", "switch")
context:   OCR: original shows "RUN GFX2({path,|" which appears to be corruption. Corrected from parameter documentation.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23637
--- END ---

--- CARD ---
id:        gfx2-propsw-param-path
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-propsw
claim:     PROPSW parameter 'path': the route to the window in which you want to use proportional character spacing.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23646
--- END ---

--- CARD ---
id:        gfx2-propsw-param-switch
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-propsw
claim:     PROPSW parameter 'switch': either "OFF" to turn proportional spacing off, or "ON" to turn proportional spacing on. The default setting is OFF.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 23649-23651
--- END ---

---

## PUT

--- CARD ---
id:        gfx2-put-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-put
claim:     PUT places the image in the specified Get/Put buffer on the window. PUT requires only the group and buffer numbers and the window coordinates for the upper left corner of the image.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23736
--- END ---

--- CARD ---
id:        gfx2-put-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-put
claim:     PUT syntax: RUN GFX2({path,}"PUT", group, buffer, xcor, ycor)
context:   OCR: original shows corrupted syntax line missing the function name. Corrected from parameter documentation.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23731
--- END ---

--- CARD ---
id:        gfx2-put-retrieves-dimensions
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-put
claim:     The GET function saves the dimensions of the block in the buffer. PUT automatically handles window format conversion based on stored dimensions.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23739
--- END ---

--- CARD ---
id:        gfx2-put-param-path
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-put
claim:     PUT parameter 'path': the route to the window where you want to place a pre-saved image.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23746
--- END ---

--- CARD ---
id:        gfx2-put-param-group-buffer
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-put
claim:     PUT parameters 'group' and 'buffer': the group number and buffer number in which the window data was saved by GET.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 23749-23753
--- END ---

--- CARD ---
id:        gfx2-put-param-coordinates
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-put
claim:     PUT parameters 'xcor' and 'ycor': the X and Y coordinates of the upper left corner of the window position. X-coordinates range 0-639. Y-coordinates range 0-191.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 23755-23758
--- END ---

--- CARD ---
id:        gfx2-put-animation-erase
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     gfx2-put
claim:     PUT's placement of a new image erases the previous image in the same buffer location when used repeatedly, creating animation.
context:   Sample program demonstrates this by repeatedly calling PUT with incremented coordinates in a loop (line 23806-23810), giving impression of animation.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23777
--- END ---

---

## PUTGC

--- CARD ---
id:        gfx2-putgc-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-putgc
claim:     PUTGC (put graphics cursor) places and displays the graphics cursor at the specified location.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23831
--- END ---

--- CARD ---
id:        gfx2-putgc-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-putgc
claim:     PUTGC syntax: RUN GFX2({path,}"PUTGC", xcor, ycor)
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23828
--- END ---

--- CARD ---
id:        gfx2-putgc-screen-relative-not-window-relative
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     gfx2-putgc
claim:     PUTGC uses screen-relative coordinates, not window-relative coordinates. Horizontal range 0-639. Vertical range 0-191.
context:   This is a key difference from other drawing functions which may use window-relative coords. Manual explicitly distinguishes this.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23832
--- END ---

--- CARD ---
id:        gfx2-putgc-param-path
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-putgc
claim:     PUTGC parameter 'path': the route to the window where you want to display a graphics cursor.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23839
--- END ---

--- CARD ---
id:        gfx2-putgc-param-coordinates
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-putgc
claim:     PUTGC parameters 'xcor' and 'ycor': the screen coordinates for the cursor location. X-coordinates range 0-639. Y-coordinates range 0-191.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 23842-23844
--- END ---

--- CARD ---
id:        gfx2-putgc-stdptrs-cursors
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     gfx2-putgc
claim:     Graphic cursors are stored in group 202 (Stdptrs). The sample shows that you must merge the Stdptrs file from SYS directory into your window before using PUTGC with different cursor variants.
context:   Sample procedure (line 23863-23874) uses GOSET to select different cursor forms from group 202 and iterates through them. Cross-reference to GOSET function; exact GOSET behavior not in this extraction range.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 23854-23874
--- END ---

---

## REVON and REVOFF

--- CARD ---
id:        gfx2-revon-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-revon
claim:     REVON (reverse video on) enables reverse video characters.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23890
--- END ---

--- CARD ---
id:        gfx2-revoff-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-revoff
claim:     REVOFF (reverse video off) disables reverse video characters.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23890
--- END ---

--- CARD ---
id:        gfx2-revon-revoff-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-revon
claim:     REVON syntax: RUN GFX2({path,}"REVON"). REVOFF syntax: RUN GFX2({path,}"REVOFF")
context:   OCR: original shows "RUN G¥X2" where ¥ is corrupted Y. Corrected to GFX2.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 23885-23886
--- END ---

--- CARD ---
id:        gfx2-revon-revoff-state-persistence
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     gfx2-revon
claim:     Once reverse video is set by REVON, it remains in effect until you execute REVOFF. Reverse video is not a per-draw-call mode but a global state.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23891
--- END ---

--- CARD ---
id:        gfx2-revon-revoff-param-path
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-revon
claim:     REVON and REVOFF parameter 'path': the route to the window in which you want to display reverse video.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23897
--- END ---

---

## SCALESW

--- CARD ---
id:        gfx2-scalesw-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-scalesw
claim:     SCALESW (scaling switch) enables or disables scaling when drawing on variously formatted windows. Scaling in windows is normally on.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23919
--- END ---

--- CARD ---
id:        gfx2-scalesw-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-scalesw
claim:     SCALESW syntax: RUN GFX2({path,}"SCALESW", "switch")
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23916
--- END ---

--- CARD ---
id:        gfx2-scalesw-off-mode
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-scalesw
claim:     When scaling is OFF, coordinates are relative to the window origin coordinates, not screen coordinates.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23921
--- END ---

--- CARD ---
id:        gfx2-scalesw-no-text-effect
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-scalesw
claim:     Scaling does not affect text. The scaling mode applies only to graphics drawing.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23922
--- END ---

--- CARD ---
id:        gfx2-scalesw-param-path
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-scalesw
claim:     SCALESW parameter 'path': the route to the window where you want to turn scaling off or on.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23927
--- END ---

--- CARD ---
id:        gfx2-scalesw-param-switch
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-scalesw
claim:     SCALESW parameter 'switch': either "OFF" (disable scaling) or "ON" (enable scaling).
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23930
--- END ---

---

## SELECT

--- CARD ---
id:        gfx2-select-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-select
claim:     SELECT causes a window to display if the procedure is operating in the active window. If the procedure is not in the active window, the newly selected window displays when you press [CLEAR].
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23006
--- END ---

--- CARD ---
id:        gfx2-select-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-select
claim:     SELECT syntax: RUN GFX2([path,]"SELECT")
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23003
--- END ---

--- CARD ---
id:        gfx2-select-default-paths
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-select
claim:     If you do not specify a path, BASICO9 selects the device using the standard input, standard output, and standard error paths (paths 0, 1, and 2).
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23009
--- END ---

--- CARD ---
id:        gfx2-select-param-path
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-select
claim:     SELECT parameter 'path': the path to the window to select. This parameter is optional.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23016
--- END ---

--- CARD ---
id:        gfx2-select-paired-dwset
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     gfx2-select
claim:     SELECT is typically used with DWSET (establish device window) and DWEND (device window end) to open a path to a new window, configure it, select it, draw, and then close it.
context:   Sample procedure (line 24030-24063) shows this pattern: OPEN path to /W3, DWSET to configure, SELECT to display, draw content, then DWEND and CLOSE.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 24037-24062
--- END ---

---

## SETDPTR

--- CARD ---
id:        gfx2-setdptr-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-setdptr
claim:     SETDPTR (set draw pointer) places the draw pointer at the specified coordinates. The draw pointer selects the beginning point of the next graphics draw function (such as CIRCLE, LINE, BOX) if you do not supply other coordinates.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 24078
--- END ---

--- CARD ---
id:        gfx2-setdptr-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-setdptr
claim:     SETDPTR syntax: RUN GFX2({path,}"SETDPTR", xcor, ycor)
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 24075
--- END ---

--- CARD ---
id:        gfx2-setdptr-param-path
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-setdptr
claim:     SETDPTR parameter 'path': the route to the screen where you want to set the draw pointer.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 24086
--- END ---

--- CARD ---
id:        gfx2-setdptr-param-coordinates
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-setdptr
claim:     SETDPTR parameters 'xcor' and 'ycor': the screen coordinates for the draw pointer location. X-coordinates range 0-639. Y-coordinates range 0-191.
context:   OCR: original shows "0-194" for Y-coordinates which appears to be OCR error; corrected to 0-191 based on consistent coordinate space across all graphics functions.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 24089
--- END ---

--- CARD ---
id:        gfx2-setdptr-enables-pointer-based-drawing
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-setdptr
claim:     After SETDPTR, graphics functions that do not supply explicit coordinates use the set draw pointer as their starting point.
context:   Example: CIRCLE with no coordinates draws centered at the pointer; LINE with only ending coords draws from pointer to endpoint.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 24080
--- END ---

---

## UNDLNON and UNDLNOFF

--- CARD ---
id:        gfx2-undlnon-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-undlnon
claim:     UNDLNON (underline on) enables character underline. After UNDLNON, all characters displayed are underlined.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23141
--- END ---

--- CARD ---
id:        gfx2-undlnoff-purpose
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-undlnoff
claim:     UNDLNOFF (underline off) disables character underline.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23141
--- END ---

--- CARD ---
id:        gfx2-undlnon-undlnoff-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-undlnon
claim:     UNDLNON syntax: RUN GFX2({path,}"UNDLNON"). UNDLNOFF syntax: RUN GFX2({path,}"UNDLNOFF")
context:   OCR: original shows "RUN GFX2¢" with ¢ being corrupted character. Corrected to GFX2.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, lines 24136-24137
--- END ---

--- CARD ---
id:        gfx2-undlnon-undlnoff-default
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-undlnon
claim:     The default state is UNDLNOFF (underline disabled).
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23143
--- END ---

--- CARD ---
id:        gfx2-undlnon-undlnoff-state-persistence
type:      GOTCHA
target:    6809
verify:    from-manual
topic:     gfx2-undlnon
claim:     Once UNDLNON is executed, underline remains in effect on all subsequent characters until UNDLNOFF is executed.
context:   Like REVON/REVOFF, this is a global state affecting all future character output, not a per-call mode.
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23142
--- END ---

--- CARD ---
id:        gfx2-undlnon-undlnoff-param-path
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx2-undlnon
claim:     UNDLNON and UNDLNOFF parameter 'path': the route to the window where you want to use underline characters.
context:   None
source:    OS-9 Level 2 Operating System Manual, Chapter 9, line 23148
--- END ---

---

## End of Range

Manual transitions to Chapter 10: BASICO9 Quick Reference at line 24161.
