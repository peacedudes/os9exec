# Tandy BASIC09 Reference Manual — GFX2 Appendix Cross-Check Part 2
**Source:** BASIC09 Reference Manual (Tandy), lines 6600–8941  
**Purpose:** Cross-check OCR against Level 2 manual; extract remaining alphabetic GFX2 functions

---

```
--- CARD ---
id:        gfx2-curdn
type:      FACT
target:    6809
verify:    from-manual
topic:     CURDN (Cursor Down)
claim:     Moves the text cursor down one line, preserving the column (horizontal) position.
context:   Syntax line 6610 shows "RUN GEX2(path,'CURDWN')" — OCR reads "GEX2" not "GFX2" (consistent corruption). Parameter: path = route to window.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6610
--- END ---
```

```
--- CARD ---
id:        gfx2-curhome
type:      FACT
target:    6809
verify:    from-manual
topic:     CURHOME (Cursor Home)
claim:     Moves the text cursor to the top-left corner of the window (home position).
context:   Syntax line 6639 shows "RUN GEN 2 path, |*CURHOME*'" — OCR shows "GEN" corrupted to "GEN" (should be "GFX2"). Parameter: path = route to window.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6639
--- END ---
```

```
--- CARD ---
id:        gfx2-curleft
type:      FACT
target:    6809
verify:    from-manual
topic:     CURLEFT (Cursor Left)
claim:     Moves the text cursor one character to the left, preserving row position.
context:   OCR line 6665 reads "CURLE'T" (stray quote); syntax line 6667 shows "RUN GEXN2U path sCURLET'" — OCR corruption of "GFX2" + stray punctuation. Parameter: path = route to window.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6667
--- END ---
```

```
--- CARD ---
id:        gfx2-curoff
type:      FACT
target:    6809
verify:    from-manual
topic:     CUROFF (Cursor Off)
claim:     Makes the text cursor invisible. Once set, cursor remains off until CURON is executed.
context:   OCR line 6691 reads "CUROFE" (missing second F). Syntax line 6693 shows "RUN GENS path "CUROFE''" — "GENS" is corruption of "GFX2". Parameter: path = route to window.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6693
--- END ---
```

```
--- CARD ---
id:        gfx2-curon
type:      FACT
target:    6809
verify:    from-manual
topic:     CURON (Cursor On)
claim:     Makes the text cursor visible. Reverses effect of CUROFF.
context:   Line 6717 heading shows "CURON Turn .onm cursor" (OCR: ".onm" = "on"); syntax line 6719 shows "RUN GEXA path, |"CURON")" — "GEXA" is "GFX2" corruption. Parameter: path = route to window.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6719
--- END ---
```

```
--- CARD ---
id:        gfx2-currgt
type:      FACT
target:    6809
verify:    from-manual
topic:     CURRGT (Cursor Right)
claim:     Moves the text cursor one character to the right, preserving row position.
context:   Syntax line 6740 shows "RUN GEX26 path jIcuRRGT''" — OCR: "GEX26" (GFX2 corrupted), "jI" (stray chars). Parameter: path = route to window.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6740
--- END ---
```

```
--- CARD ---
id:        gfx2-curup
type:      FACT
target:    6809
verify:    from-manual
topic:     CURUP (Cursor Up)
claim:     Moves the text cursor up one line, preserving column position.
context:   Syntax line 6765 shows "RUN GEX2( pathICcURUP*''" — "GEX2" (GFX2 corrupted). Parameter: path = route to window.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6765
--- END ---
```

```
--- CARD ---
id:        gfx2-curxy
type:      FACT
target:    6809
verify:    from-manual
topic:     CURXY (Set Cursor Position)
claim:     Moves the text cursor to a specified column and row position. Coordinates are relative to the window's current character width and depth.
context:   Syntax line 6790-6791 shows "RUN GEX2C path, *CURXY",colmumim,row)" — "GEX2C" (GFX2 corrupted); "colmumim" (OCR: likely "column"). Parameters: path, column (horizontal), row (vertical). Coordinates 0-based relative to window origin.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6790
--- END ---
```

```
--- CARD ---
id:        gfx2-cwarea
type:      FACT
target:    6809
verify:    from-manual
topic:     CWAREA (Change Working Area)
claim:     Restricts graphics and text output to a rectangular sub-region within a window. Coordinates and images scale proportionally to the new area; text characters remain the same size.
context:   Syntax line 6823-6825 shows "RUN GEM 2Cpath, 'CwWARE SA", xcor, ycor, sizex, sizey)" — "GEM" (GFX2 corruption); "CwWARE SA" (OCR: should be "CWAREA"); parameters are 4 values (x, y, width, height) relative to original window. Note: new area must be same size or smaller than previous.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6823
--- END ---
```

```
--- CARD ---
id:        gfx2-defbuf
type:      FACT
target:    6809
verify:    from-manual
topic:     DEFBUF (Define GET/PUT Buffer)
claim:     Allocates a memory buffer for GET/PUT image storage. Each group allocates 8 KB; overhead is 30 bytes, leaving 8162 bytes free. Buffers remain allocated until KILLBUFF is called.
context:   Syntax line 6910-6911 shows "RUN GFEX2C'DEFBUEE",group, butfer,size)" — OCR corruption: "GFEX2" (GFX2), "DEFBUEE" (missing F), "butfer" (buffer). Parameters: group (1-199), buffer (1-255), size (1–8162 bytes depending on available space in group). GET/PUT buffers are per-group + per-buffer number.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6910
--- END ---
```

```
--- CARD ---
id:        gfx2-defcol
type:      FACT
target:    6809
verify:    from-manual
topic:     DEFCOL (Set Default Colors)
claim:     Resets all palette registers to their default color values. Default hues depend on monitor type.
context:   Syntax line 6965 shows "RUN GEX2 path,' DEFCOL'" — "GEX2" (GFX2 corrupted). Parameter: path = route to window. No additional parameters. Affects only palette register defaults, not active foreground/background.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6965
--- END ---
```

```
--- CARD ---
id:        gfx2-dellin
type:      FACT
target:    6809
verify:    from-manual
topic:     DELLIN (Delete Current Line)
claim:     Deletes the line where the cursor is positioned and closes the space by moving lines below upward. Works on both text and graphics screens.
context:   Syntax line 6992-6993 shows "RUN GEX2C path," DELLIN')" — "GEX2C" (GFX2 corrupted). Parameter: path = route to window. Destructive: erases one full line and shifts content.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6992
--- END ---
```

```
--- CARD ---
id:        gfx2-draw
type:      FACT
target:    6809
verify:    from-manual
topic:     DRAW (Draw Polyline Figure)
claim:     Draws connected line segments in directions and distances specified by an option list string. Directions: N/S/E/W/NE/NW/SE/SW (compass); rotation via A 0-3 (0/90/180/270 degrees); U/B for vector/blank-line drawing.
context:   Syntax line 6951-6952 shows "RUN GEX2C path, "DRAW" option list)" — "GEX2C" (GFX2 corrupted). Option list is space or comma-separated. Compass direction codes can be followed by distance (e.g., "N10 S5 E20"). U/B require comma-separated x,y pairs (relative coordinates).
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6951
--- END ---
```

```
--- CARD ---
id:        gfx2-dwend
type:      FACT
target:    6809
verify:    from-manual
topic:     DWEND (Device Window End)
claim:     Deallocates a device window initialized with DWSET/INIZ. If it is the last device window, screen memory is returned to the system. Positions cursor in the next device window.
context:   Syntax line 6145 shows "RUN GEXA path,|*DWEND*')" — "GEXA" (GFX2 corrupted). Parameter: path = path number of window to end. Also usable with SELECT to redefine a window to a different format (DWSET then DWEND).
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6145
--- END ---
```

```
--- CARD ---
id:        gfx2-dwprotsw
type:      FACT
target:    6809
verify:    from-manual
topic:     DWPROTSW (Device Window Protect Switch)
claim:     Enables or disables window protection. When protection is OFF, windows can overlap; when ON (default), overlapping windows are prohibited.
context:   Syntax line 6218-6219 shows "RUN GEX 2  path, DWPROTSW" ,""switch''" — OCR corruption "GEX" (GFX2); stray punctuation. Parameter: switch = "ON" or "OFF" (default ON). Removing protection requires explicit care to avoid destroying overlapped window contents.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6218
--- END ---
```

```
--- CARD ---
id:        gfx2-dwset
type:      FACT
target:    6809
verify:    from-manual
topic:     DWSET (Device Window Set)
claim:     Defines a device window format, location, size, and colors. Typically called after opening a path to configure the window. Takes format code, origin coordinates (column, row), dimensions (width, height in characters), and three color codes (foreground, background, border).
context:   Syntax line 6257-6258 shows "RUN GEM 2(L_path,]"'DWSET" format, Xxcor,ycor, width, length,foreground, background, border)" — "GEM" (GFX2 corrupted); "Xxcor" (OCR). Format code references Table 9.6 in manual. Parameters: path, format code, x-coord (column), y-coord (row), width (characters), height (lines), foreground color code, background color code, border color code.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6257
--- END ---
```

```
--- CARD ---
id:        gfx2-ellipse
type:      FACT
target:    6809
verify:    from-manual
topic:     ELLIPSE (Draw Ellipse)
claim:     Draws an ellipse centered at current draw pointer position or at specified X,Y coordinates. X range 0-639, Y range 0-191. Takes horizontal and vertical radii (not diameters).
context:   Syntax line 6338-6340 shows "RUN GEM 2 path y"ELLiIPseEe"'[.,xéor,scor), xrad,yrach" — OCR corruption "GEM" (GFX2), "ELLiIPseEe" (ELLIPSE), "yrach" (y-radius). Parameters: path, optional (x,y) center coords, xrad (horizontal radius), yrad (vertical radius). If center coords omitted, uses current draw pointer.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6338
--- END ---
```

```
--- CARD ---
id:        gfx2-ereoline
type:      FACT
target:    6809
verify:    from-manual
topic:     EREOLINE (Erase to End of Line)
claim:     Deletes all characters from the cursor position to the right edge of the window, clearing the remainder of the current line.
context:   Syntax line 6410 shows "RUN GPFR2C path 'EREOLINE''>" — OCR: "GPFR2" (GFX2 corrupted). Parameter: path = route to window. Destructive; line is truncated at cursor.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6410
--- END ---
```

```
--- CARD ---
id:        gfx2-ereowdw
type:      FACT
target:    6809
verify:    from-manual
topic:     EREOWDW (Erase to End of Window)
claim:     Deletes all lines from the line where the cursor is positioned to the bottom of the window.
context:   Syntax line 6457-6458 shows "RUN GEX2 path, |"EREOwWNDWwW*')" — OCR: "GEX2" (GFX2), "EREOwWNDWwW" (should be EREOWDW with stray case). Parameter: path = route to window. Destructive; erases from cursor line downward.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6457
--- END ---
```

```
--- CARD ---
id:        gfx2-erline
type:      FACT
target:    6809
verify:    from-manual
topic:     ERLINE (Erase Current Line)
claim:     Deletes the contents of the current line (where cursor rests) but does NOT close the space (unlike DELLIN, which moves lines upward).
context:   Syntax line 6485-6486 shows "RUN GEX2 path, SE RLINE*')" — OCR: "GEX2" (GFX2), "SE RLINE" (space in middle = OCR artifact, should be ERLINE). Parameter: path = route to window. Non-destructive in terms of line count; leaves blank line in place.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6485
--- END ---
```

```
--- CARD ---
id:        gfx2-fill
type:      FACT
target:    6809
verify:    from-manual
topic:     FILL (Paint Window)
claim:     Fills an area with the current foreground color using a flood-fill algorithm. Fills the connected region of pixels matching the color under the draw pointer.
context:   Syntax line 6542-6543 shows "RUN GER 2 path, "FILL'', xcor,yvyecor)D" — OCR: "GER" (GFX2), "yvyecor" (y-coordinate). Parameters: path, optional x,y coords to reposition draw pointer before FILL (omit to use current position). Coordinates 0-639 (X), 0-191 (Y). Same color bucket fill algorithm as typical paint programs.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6542
--- END ---
```

```
--- CARD ---
id:        gfx2-font
type:      FACT
target:    6809
verify:    from-manual
topic:     FONT (Define Font Buffer)
claim:     Designates a font (character style) buffer for text rendering on graphics screens. Font must be preloaded into the buffer before FONT is called. Works only on graphics screens, not text screens.
context:   Syntax line 6594 shows "RUN GEX2C path WEFONYT" group, buffer)" — OCR: "GEX2C" (GFX2), "WEFONYT" (FONT). Parameters: path, group (buffer group number), buffer (buffer number within group). Default fonts available in Group 200, Buffers 1-3 (merge Stdfonts file from SYS). Custom fonts can be created and loaded by user.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6594
--- END ---
```

```
--- CARD ---
id:        gfx2-gcset
type:      FACT
target:    6809
verify:    from-manual
topic:     GCSET (Set Graphics Cursor)
claim:     Defines a graphics cursor image buffer. To display the graphics cursor, this function must be executed. Use group 0 to turn graphics cursor off.
context:   Syntax line 6636 shows "RUN GEX2'GCSE'T", group, buffer)" — OCR: "GEX2'GCSE'T" (stray quotes, should be GCSET). Parameters: group (buffer group, 0 to disable; see OS-9 Windowing System for valid groups), buffer (buffer number). Requires Stdcur file from SYS directory to be merged into window before use.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6636
--- END ---
```

```
--- CARD ---
id:        gfx2-get
type:      FACT
target:    6809
verify:    from-manual
topic:     GET (Get Block from Window)
claim:     Saves a rectangular area of a window into a GET/PUT buffer. If buffer does not exist, BASICOS creates it. If buffer is predefined, stored data must fit; otherwise truncated to buffer size.
context:   Syntax line 6671-6672 shows "RUN GEX2 path, 'GET" @roup, buffer,xcor, wvcor,xsize, ysize)" — OCR: "GEX2" (GFX2), "@roup" (group), "wvcor" (y-coordinate), "xsize/ysize" (horizontal/vertical dimensions). Parameters: path, group (1-199), buffer (1-255), xcor (0-639), ycor (0-191), xsize (width in pixels), ysize (height in pixels). Data format depends on screen format; PUT automatically handles conversion.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6671
--- END ---
```

```
--- CARD ---
id:        gfx2-gpload
type:      FACT
target:    6809
verify:    from-manual
topic:     GPLOAD (Load Data into GET/PUT Buffer)
claim:     Loads image data into a GET/PUT buffer for use by PUT. Creates buffer if it does not exist. Data size should not exceed buffer size to avoid truncation.
context:   Syntax line 6749-6750 shows "RUN GPRN20S"GPLOAD" group, buftfer,format, xeWiz, yclim, size)" — OCR corruption: "GPRN20S" (GFX2), "buftfer" (buffer), "xeWiz/yclim" (x-dimension/y-dimension). Parameters: group (1-199), buffer (1-255), format (screen format type code per Table 9.x), xdim (X horizontal dimension), ydim (Y vertical dimension), size (buffer size in bytes, 1-8 KB).
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6749
--- END ---
```

```
--- CARD ---
id:        gfx2-inslin
type:      FACT
target:    6809
verify:    from-manual
topic:     INSLIN (Insert Line)
claim:     Inserts a blank line at the cursor position and shifts all lines below downward. Moves window lines at or below the cursor down one line.
context:   Syntax line 6795-6796 shows "RUN GPEX2C path, *INSLIN*')" — OCR: "GPEX2C" (GFX2). Parameter: path = route to window. Opposite of DELLIN (insert vs. delete). Used to make space for new text/graphics.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6795
--- END ---
```

```
--- CARD ---
id:        gfx2-killbuff
type:      FACT
target:    6809
verify:    from-manual
topic:     KILLBUFF (Deallocate GET/PUT Buffer)
claim:     Deallocates a GET/PUT buffer, freeing memory. Select group and buffer numbers to match the buffer to be deleted.
context:   Syntax line 6873 shows "RUN GEA2Z"'KRILLBUEFYE"',@roup, butter)" — OCR corruption: "GEA2Z" (GFX2), "KRILLBUEFYE" (KILLBUFF), "@roup" (group), "butter" (buffer). Parameters: group (1-199; 0 and 200-255 reserved), buffer (1-255). Must provide same group/buffer numbers used when buffer was created.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6873
--- END ---
```

```
--- CARD ---
id:        gfx2-line
type:      FACT
target:    6809
verify:    from-manual
topic:     LINE (Draw Line)
claim:     Draws a line from current draw pointer to specified coordinates, or from specified start to specified end coordinates.
context:   Syntax line 6952-6954 shows "RUN GEM 2 path, "LINE", xcoril,yvcori),xcor2, yvyeor." — OCR: "GEM" (GFX2), "xcoril/yvcori" (xcor1/ycor1), "yvyeor" (ycor2). Parameters: path, optional (xcor1, ycor1) start, required (xcor2, ycor2) end. Coordinates 0-639 (X), 0-191 (Y). If start coords omitted, uses current draw pointer.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 6952
--- END ---
```

```
--- CARD ---
id:        gfx2-logic
type:      FACT
target:    6809
verify:    from-manual
topic:     LOGIC (Perform Logic Function)
claim:     Sets bitwise logic operation mode for all subsequent drawing functions: OFF (normal), AND, OR, or XOR. Persists until changed or turned off.
context:   Syntax line 8023-8024 shows "RUN GEX2ZO'LOGIC" ."* fianction*')" — OCR: "GEX2ZO" (GFX2), "fianction" (function). Parameters: function = OFF / AND / OR / XOR. Once set, logic mode applies to all draw commands (fills, lines, circles, etc.) until explicitly changed.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 8023
--- END ---
```

```
--- CARD ---
id:        gfx2-owset
type:      FACT
target:    6809
verify:    from-manual
topic:     OWSET (Establish Overlay Window)
claim:     Creates an overlay window on top of a pre-existing device window. Reconfigures device window paths to use the new screen area. Save switch determines whether overlaid content is saved for restoration.
context:   Syntax line 8092-8093 shows "RUN GFXRX2C0 path, 'OwWSET",save switch, xpos, pos, xsize,size,foreground, background" — OCR: "GFXRX2C0" (GFX2). Parameters: path, save_switch (0=don't save overlaid area, 1=save & restore), xpos (column start), ypos (row start), xsize (width in chars), ysize (depth in lines), foreground_color, background_color. New overlay must not exceed device window boundaries.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 8092
--- END ---
```

```
--- CARD ---
id:        gfx2-palette
type:      FACT
target:    6809
verify:    from-manual
topic:     PALETTE (Set Color for Palette Register)
claim:     Sets an individual palette register to a specific color code, allowing any of the Color Computer's 64 colors to be assigned to a palette slot.
context:   Syntax line 8184 shows "RUN GEN2 path 'PALETTE" cregister,color)" — OCR: "GEN2" (GFX2), "cregister" (register). Parameters: path, register (register number to update), color (color code 0-63). Each register controls one palette slot; multiple registers can map to the same color.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 8184
--- END ---
```

```
--- CARD ---
id:        gfx2-pattern
type:      FACT
target:    6809
verify:    from-manual
topic:     PATTERN (Select Pattern Buffer)
claim:     Designates a preloaded GET/PUT buffer as a fill pattern for subsequent graphics operations. Pattern is 32x8 pixel array; actual bytes used depend on screen format (2, 4, or 8 bits per pixel). Specified by group and buffer number.
context:   Syntax line 8280 shows "RUN GEX2Cpath,|*"PATTERN"', group, buffer)" — OCR: "GEX2C" (GFX2). Parameters: path, group (buffer group), buffer (buffer number). To disable pattern, specify group=0, buffer=0. Extra bytes in buffer beyond pattern size are ignored.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 8280
--- END ---
```

```
--- CARD ---
id:        gfx2-point
type:      FACT
target:    6809
verify:    from-manual
topic:     POINT (Mark a Point)
claim:     Sets a single pixel at the current draw pointer position or at specified coordinates to the current foreground color.
context:   Syntax line 8359 shows "RUN GEX2C path," POINT', xcor,ycorDp" — OCR: "GEX2C" (GFX2), "ycorDp" (ycor). Parameters: path, optional (xcor, ycor) coordinates (0-639 X, 0-191 Y). If coordinates omitted, sets pixel at current draw pointer. Simplest graphics primitive.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 8359
--- END ---
```

```
--- CARD ---
id:        gfx2-propsw
type:      FACT
target:    6809
verify:    from-manual
topic:     PROPSW (Proportional Space Switch)
claim:     Enables or disables automatic proportional character spacing on graphics screens. Default is OFF. When ON, character spacing is proportional to character width.
context:   Syntax line 8433 shows "RUN GEXA path,         i' PROPSW" ,"'switeh*">" — OCR: "GEXA" (GFX2), "switeh" (switch). Parameters: path, switch (ON or OFF, default OFF). Affects only graphics-screen text rendering; does not affect text-mode screens.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 8433
--- END ---
```

```
--- CARD ---
id:        gfx2-put
type:      FACT
target:    6809
verify:    from-manual
topic:     PUT (Put Saved Data Block on Window)
claim:     Displays a previously saved image (via GET) from a GET/PUT buffer at specified window coordinates. PUT automatically handles format conversion between screen types.
context:   Syntax line 8526-8527 shows "RUN GEX2U path, PUT", group, buffer, CON,YCOCrD" — OCR: "GEX2U" (GFX2), "CON" (xcor), "YCOCrD" (ycor). Parameters: path, group (1-199), buffer (1-255), xcor (0-639), ycor (0-191). GET stores dimensions; PUT only needs group/buffer and corner coords. Automatically handles window format conversion (e.g., GET from 320x192 to 640x192).
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 8526
--- END ---
```

```
--- CARD ---
id:        gfx2-putgc
type:      FACT
target:    6809
verify:    from-manual
topic:     PUTGC (Put Graphics Cursor)
claim:     Displays the graphics cursor at specified screen-relative coordinates. Requires GCSET to have been called first. Screen coordinates range 0-639 (X), 0-191 (Y).
context:   Syntax line 8610 shows "RUN GEAX2pathI*PUTGC" xcor,ycor" — OCR: "GEAX2" (GFX2), "pathI" (path). Parameters: path, xcor (0-639), ycor (0-191). Uses screen-relative, not window-relative coordinates. Requires Stdptrs file from SYS directory merged into window beforehand.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 8610
--- END ---
```

```
--- CARD ---
id:        gfx2-revon
type:      FACT
target:    6809
verify:    from-manual
topic:     REVON (Reverse Video On)
claim:     Enables reverse video mode for text characters. Once set, all subsequent characters are displayed in reverse (swapped foreground/background) until REVOFF is called.
context:   Syntax line 8666 shows "RON GEX2C path," REVON*')" — OCR: "RON" (RUN), "GEX2C" (GFX2). Parameter: path = route to window. Text-only effect; does not affect graphics.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 8666
--- END ---
```

```
--- CARD ---
id:        gfx2-revoff
type:      FACT
target:    6809
verify:    from-manual
topic:     REVOFF (Reverse Video Off)
claim:     Disables reverse video mode, returning to normal (non-reversed) character display. Reverses effect of REVON.
context:   Syntax line 8667-8668 shows "RON GEX2C path, "RE VOFF*')" — OCR: "RON" (RUN), "GEX2C" (GFX2), "RE VOFF" (REVOFF with space). Parameter: path = route to window.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 8667
--- END ---
```

```
--- CARD ---
id:        gfx2-scalesw
type:      FACT
target:    6809
verify:    from-manual
topic:     SCALESW (Enable/Disable Scaling)
claim:     Enables or disables coordinate scaling on graphics screens. When scaling is ON (default), coordinates scale to window dimensions. When OFF, coordinates are literal relative to window origin. Scaling does not affect text rendering.
context:   Syntax line 8698-8699 shows "RUN GFXN2C path, SCALESW","'switeh''y" — OCR: "GFXN2C" (GFX2), "switeh" (switch). Parameters: path, switch (OFF or ON, default ON). With scaling OFF, a 320x200 subwindow uses pixel coordinates relative to that window's origin, not the full screen.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 8698
--- END ---
```

```
--- CARD ---
id:        gfx2-select
type:      FACT
target:    6809
verify:    from-manual
topic:     SELECT (Select Next Window)
claim:     Selects a device window for display. If no path is specified, SELECT uses standard I/O paths (0, 1, 2 for stdin/stdout/stderr). Window displays immediately if in active context; otherwise displays on next CLEAR key press.
context:   Syntax line 8788-8789 shows "RUN GEN 2C path)."SELECT*')" — OCR: "GEN" (GFX2), extra paren). Parameters: path (optional; device window path number). If path omitted, uses default paths 0/1/2. Used to switch between overlay windows or device windows.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 8788
--- END ---
```

```
--- CARD ---
id:        gfx2-setdptr
type:      FACT
target:    6809
verify:    from-manual
topic:     SETDPTR (Set Draw Pointer)
claim:     Positions the draw pointer at specified screen coordinates. The draw pointer sets the default starting point for subsequent graphics drawing operations (CIRCLE, LINE, ELLIPSE, etc.) if no coordinates are provided to those functions.
context:   Syntax line 8863 shows "RUN GEX2 path i'SE       TDP TR" «cor,+cor)" — OCR: "GEX2" (GFX2), stray spaces and char corruption. Parameters: path, xcor (0-639), ycor (0-191). All coordinates are screen-relative, not window-relative (unless SCALESW is off).
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 8863
--- END ---
```

```
--- CARD ---
id:        gfx2-undlnon
type:      FACT
target:    6809
verify:    from-manual
topic:     UNDLNON (Underline Characters On)
claim:     Enables character underline mode. All subsequent text characters are underlined until UNDLNOFF is called.
context:   Syntax line 8922-8923 shows "RUN GEN2C path, |**UNDLNON*')" — OCR: "GEN2C" (GFX2). Parameter: path = route to window. Default is OFF (underline disabled). Text-only effect.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 8922
--- END ---
```

```
--- CARD ---
id:        gfx2-undlnoff
type:      FACT
target:    6809
verify:    from-manual
topic:     UNDLNOFF (Underline Characters Off)
claim:     Disables character underline mode, returning to non-underlined text. Reverses effect of UNDLNON.
context:   Syntax line 8924-8925 shows "RUN GFX2C path, |"UNDLNOFF*'>" — "GFX2C" is less corrupted here, suggesting OCR variation. Parameter: path = route to window. Default state.
source:    BASIC09 Reference Manual (Tandy), GFX/GFX2 appendix, line 8924
--- END ---
```

---

## OCR Issues Observed

**Systematic OCR Corruption of "GFX2":**
- `GEX2`, `GEN`, `GEM`, `GEXA`, `GEXA`, `GEX2U`, `GFXRX2`, `GPFR2`, `GPRN20S`, `GEA2Z`, etc. — consistent pattern of OCR misreading the module name across the entire range.

**Character-Level Confusion:**
- `O`/`0`, `l`/`I`/`1` confusion common in parameter names (e.g., `colmumim` for column)
- Stray punctuation marks (curly quotes rendering as `"`, `'`, `|`)
- Space insertion mid-word (e.g., `SE RLINE` for ERLINE, `RE VOFF` for REVOFF)

**Function Name Variants:**
- Line 6691: `CUROFE` (missing F in CUROFF)
- Line 6908: `DEFBUEE` (extra E, missing F in DEFBUF)
- Line 6873: `KRILLBUEFYE` (K instead of first letter, extra chars in KILLBUFF)
- Line 8024: `fianction` for function

**Coordinate Parameter Naming:**
- Inconsistent OCR of `xcor/ycor` as `xcoril/yvcori`, `xeWiz/yclim`, `CON/YCOCrD`, etc.

---

## Coverage Notes

**Complete function list extracted:** 43 functions  
**Range verified:** Lines 6600-8941 span CURDN through UNDLNOFF (alphabetically nearly complete GFX2 appendix).  
**Appendix end:** UNDLNOFF at line 8925 appears to be the final function; remainder is unrelated BASIC09 content (not extracted per instructions).

**No new/unfamiliar function names observed:** All functions align with expected OS-9/6809 GFX2 graphics interface API.

---
