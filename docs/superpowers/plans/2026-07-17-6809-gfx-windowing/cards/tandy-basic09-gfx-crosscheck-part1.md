# GFX/GFX2 Cross-Check Extraction: Tandy BASIC09 Manual OCR (Lines 4200-6600)

This is a second OCR pass of the GFX/GFX2 graphics appendix, used to cross-verify function signatures against the Level 2 Operating System Manual version. Focus is on OCR discrepancies and signature ambiguities.

---

## GFX Module (Medium-Resolution Graphics)

```
--- CARD ---
id:        gfx-module-overview
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX module overview
claim:     GFX is the medium-resolution graphics module for Color Computer 3. Must be in execution directory or resident in memory when called by BASIC09. Can be loaded via LOAD command or auto-loaded by BASIC09 on first graphics function call.
context:   Tandy OCR line 4225-4232 matches Level 2 manual structure; both sources confirm module-on-demand loading behavior
source:    BASIC09 Reference Manual (Tandy), line 4225-4232
--- END ---
```

```
--- CARD ---
id:        gfx-format-0-vs-format-1
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX screen formats and resolution
claim:     Format 0: 256×192 pixels, 2 colors on screen at once. Format 1: 128×192 pixels, 4 colors on screen at once. Coordinate 0,0 is at lower-left corner.
context:   Tandy OCR lines 4300-4306 agrees with Level 2 manual; both confirm these exact resolutions and color counts
source:    BASIC09 Reference Manual (Tandy), lines 4300-4306
--- END ---
```

```
--- CARD ---
id:        gfx-draw-pointer-initial-position
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX draw pointer default location
claim:     When a graphics screen is established, the draw pointer is located at coordinates 0,0. Functions like LINE automatically update draw pointer position to the end coordinates of the line drawn.
context:   Tandy OCR lines 4370-4382; this cross-confirms with Level 2 manual's draw-pointer semantics
source:    BASIC09 Reference Manual (Tandy), lines 4370-4382
--- END ---
```

```
--- CARD ---
id:        gfx-alpha-function
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX ALPHA function
claim:     ALPHA switches from graphics screen back to alphanumeric (text) screen. Current graphics screen remains intact in memory. Parameters: None.
context:   Tandy OCR line 4389 syntax is garbled ("RUN GEXC ALPHA")" with mangled quotes; Level 2 manual version should clarify clean syntax. Function name is clear (ALPHA) despite OCR noise.
source:    BASIC09 Reference Manual (Tandy), lines 4387-4402
--- END ---
```

```
--- CARD ---
id:        gfx-circle-function-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX CIRCLE function signature
claim:     CIRCLE syntax: RUN GFX("CIRCLE"[, xcor, ycor], radius[, color]). Draws circle with given radius at optional X,Y center or at current draw pointer position. Optional color parameter uses current foreground color if omitted.
context:   Tandy OCR line 4469 reads "RUN GEXCS'CIRCLE"[,.xcor,»ycor),radius [,@otor}])" with OCR corruption: GEXCS→GFX, quotes mangled, @otor→color. Syntax structure matches expectation but quotes are unreadable. Level 2 manual's version should disambiguate quote/bracket style.
source:    BASIC09 Reference Manual (Tandy), lines 4467-4502
--- END ---
```

```
--- CARD ---
id:        gfx-circle-coordinate-ranges
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX CIRCLE coordinate bounds
claim:     X-coordinate range: 0-255. Y-coordinate range: 0-191. Specifying coordinates outside these ranges causes an error.
context:   Tandy OCR lines 4486-4488 clearly state these bounds; Level 2 manual should confirm identical ranges
source:    BASIC09 Reference Manual (Tandy), lines 4486-4488
--- END ---
```

```
--- CARD ---
id:        gfx-clear-function
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX CLEAR function
claim:     CLEAR clears the current graphics screen. If color not specified, sets entire screen to current background color. Also sets graphics cursor to coordinates 0,0 (lower-left corner).
context:   Tandy OCR line 4572 syntax: "RUN GEFXAC'CLEAR"{,color})" — GEFXAC should be GFX, quotes mangled. Function behavior is clear and should match Level 2 manual.
source:    BASIC09 Reference Manual (Tandy), lines 4570-4591
--- END ---
```

```
--- CARD ---
id:        gfx-color-function
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX COLOR function
claim:     COLOR changes the foreground color (and possibly the color set). Does not change graphics format or cursor position. Parameter: color code.
context:   Tandy OCR line 4602 syntax: "RUN GEXAC'COLOR",colar)" — GEXAC→GFX, colar→color. Function effect matches Level 2 manual semantics.
source:    BASIC09 Reference Manual (Tandy), lines 4600-4620
--- END ---
```

```
--- CARD ---
id:        gfx-gcolor-function
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX GCOLOR function (read pixel)
claim:     GCOLOR reads a pixel's color and stores result. Parameters: names of variables in which to store optional X- and Y-coordinates. Purpose is to read current pixel color at graphics cursor or specified coordinates.
context:   Tandy OCR line 4260 reference table entry is sparse; this is GFX, not GFX2, and is read-only. Level 2 manual should provide full syntax details.
source:    BASIC09 Reference Manual (Tandy), line 4260
--- END ---
```

```
--- CARD ---
id:        gfx-gloc-function
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX GLOC function (graphics screen memory location)
claim:     GLOC determines graphics screen memory location and returns address in specified variable. Enables use of PEEK/POKE for advanced graphics not available in GFX module (e.g., screen fill, graphics save to disk).
context:   Tandy OCR line 4673 syntax: "RUN GEAC'SGLOC" storage)" — GEAC should be GFX, SGLOC→GLOC; also odd quote/paren pattern. Level 2 manual should clarify clean signature.
source:    BASIC09 Reference Manual (Tandy), lines 4671-4700
--- END ---
```

```
--- CARD ---
id:        gfx-gloc-memory-requirement
type:      FACT
target:    6809
verify:    from-manual
topic:     GLOC memory overhead for PEEK/POKE
claim:     OS-9 Level 2 maps display screens into program address space before PEEK/POKE can operate on display. Requires at least 8 kilobytes free memory in user address space. Program and data memory must not exceed 56 kilobytes total.
context:   Tandy OCR lines 4683-4687; this is a hard constraint documented in both versions
source:    BASIC09 Reference Manual (Tandy), lines 4683-4687
--- END ---
```

```
--- CARD ---
id:        gfx-joystick-function-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX JOYSTICK function (read stick/fire status)
claim:     JOYSTICK determines status of fire button and X,Y position of joystick handle. Syntax parameters: stick (0=right, 1=left), fire (variable to hold button state), xcor/ycor (variables for X,Y coordinates). Coordinate range is 0-63 for both axes.
context:   Tandy OCR line 4753 syntax: "RUN GEX(C'JIOY STE" stick, fire,xcor, yCcor)" — GEX→GFX, "JIOY STE"→"JOYSTICK", quotes mangled, yCcor has stray capital. Level 2 version should clarify.
source:    BASIC09 Reference Manual (Tandy), lines 4751-4782
--- END ---
```

```
--- CARD ---
id:        gfx-line-function-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX LINE function
claim:     LINE draws a line in current or specified foreground color in two ways: (1) from current draw position to specified X,Y-coordinates, OR (2) from specified beginning X,Y-coordinates to ending X,Y-coordinates. Syntax: RUN GFX("LINE"[, xcor1, ycor1], xcor2, ycor2[, color]).
context:   Tandy OCR line 4816 syntax reads "RUN GEXCLINE" 1, xcori,yocort),xcor2,ycor2 {,cofor})" — GEXCLINE→GFX("LINE", quotes mangled, yocort→ycor1, cofor→color. This example OCR is particularly garbled. Level 2 manual's version is needed to resolve.
source:    BASIC09 Reference Manual (Tandy), lines 4814-4847
--- END ---
```

```
--- CARD ---
id:        gfx-mode-function-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX MODE function (switch to graphics)
claim:     MODE switches screen from alphanumeric (text) to graphics, selecting screen format and color code. Must be run before any other graphics function. BASIC09 allocates 6-kilobyte block of memory for graphics. Returns error if memory unavailable.
context:   Tandy OCR line 4882 syntax: "RUN GEXAC MODE", format, Color" — GEXAC→GFX, "MODE" quote mangled. Parameters: format (0 or 1), color (code for screen color set). Behavior matches Level 2 manual.
source:    BASIC09 Reference Manual (Tandy), lines 4880-4911
--- END ---
```

```
--- CARD ---
id:        gfx-move-function-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX MOVE function (position draw pointer)
claim:     MOVE repositions invisible graphics cursor to specified location on screen. Does not change display. Syntax: RUN GFX("MOVE", xcor, ycor). Parameters: xcor, ycor (coordinates for cursor).
context:   Tandy OCR line 4917 syntax: "RUN GEXC'MON E"', xcor, ycor)" — GEXC→GFX, "MON E"→"MOVE", quote/space pattern is corrupted. Clear intent but OCR is messy. Level 2 version needed for clean reference.
source:    BASIC09 Reference Manual (Tandy), lines 4915-4934
--- END ---
```

```
--- CARD ---
id:        gfx-point-function-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX POINT function (set pixel color)
claim:     POINT displays a dot at specified coordinates. If color specified, sets pixel to that color. Otherwise, uses foreground color. Syntax: RUN GFX("POINT", xcor, ycor[, color]).
context:   Tandy OCR line 4965 syntax: "RUN GEXCPOITN °? oxeor, vcorl, color))" — GEXCPOITN→GFX("POINT", garbled stray characters (°?), oxeor→xcor, vcorl→ycor. Function logic is clear but OCR is corrupted. Level 2 reference needed.
source:    BASIC09 Reference Manual (Tandy), lines 4963-4987
--- END ---
```

```
--- CARD ---
id:        gfx-quit-function
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX QUIT function (deallocate graphics)
claim:     QUIT switches screen to alphanumeric (text) mode and deallocates graphics memory. Parameters: None.
context:   Tandy OCR line 5034 syntax: "RUN GERAC'QUIT'')" — GERAC→GFX, quotes are stray double-quote. Function behavior matches Level 2 manual.
source:    BASIC09 Reference Manual (Tandy), lines 5032-5046
--- END ---
```

---

## GFX2 Module (High-Resolution Graphics)

```
--- CARD ---
id:        gfx2-module-overview
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 module capabilities
claim:     GFX2 is the high-resolution graphics module. Enables: 64 colors (selectable palette with 16 default colors), border colors, color patterns, custom graphics cursors, logic functions (AND/OR/NOR), automatic scaling on/off, outline and filled boxes, ellipses and arcs, area fill, GET/PUT screen sections, character fonts (bold, transparent, proportional), cursor control, line erase, screen erase, reverse/normal video, underline, multi-window support, graphics mixed with text on same screen.
context:   Tandy OCR lines 5050-5097 comprehensive listing matches Level 2 manual's feature enumeration
source:    BASIC09 Reference Manual (Tandy), lines 5050-5097
--- END ---
```

```
--- CARD ---
id:        gfx2-window-formats-table
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 WCREATE format codes and screen specs
claim:     Format codes 01-02 are text screens (40×24 and 80×24 chars). Format codes 05-08 are graphics screens with resolutions: 05=640×192 2-color, 06=320×192 4-color, 07=640×192 4-color, 08=320×192 16-color. Memory requirements vary: text 4000-16000 bytes, graphics 16000-32000 bytes depending on resolution and color count.
context:   Tandy OCR lines 5186-5209 contain OCR corruption: "ot"→"01", "o2"→"02", "os"→"05", "oc"→"06", "o7"→"07", which alter the format codes significantly. Level 2 manual must be used to verify correct format-code assignments. This is a CRITICAL OCR discrepancy.
source:    BASIC09 Reference Manual (Tandy), lines 5186-5209
--- END ---
```

```
--- CARD ---
id:        gfx2-palette-structure
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 palette registers and default colors
claim:     GFX2 has 64 total colors available. Palette holds 16 colors at once. Default palette registers: 00=Black, 01=Red, 02=Green, 03=Yellow, 04=Blue, 05=Magenta, 06=Cyan, 07=White, 08-0F=varies by screen format and register function. Register 0 controls border color. Registers 02 and 03 control background and foreground.
context:   Tandy OCR lines 5221-5243 palette table (Table 9.7) has scattered OCR errors (0L→01, oso→08, ao→0A, etc.) but structure and default mappings are recognizable. Level 2 manual should provide authoritative register mapping.
source:    BASIC09 Reference Manual (Tandy), lines 5221-5243
--- END ---
```

```
--- CARD ---
id:        gfx2-syntax-general-form
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 function call syntax (general form)
claim:     GFX2 functions are called: RUN GFX2([PATH,] "FUNCTION", PARAMETER, ...). PATH is optional variable naming window for function. FUNCTION is the high-resolution task name. PARAMETER is essential or optional value affecting function performance.
context:   Tandy OCR line 5719 syntax reads "RUN -GFX SCL PATHI LM FUNCT ION. , PAR ANE TERE ,..... 119" — heavily garbled: should read roughly "RUN GFX2([PATH,] "FUNCTION", PARAMETER, ...)". Exact syntax template needs Level 2 manual clarification.
source:    BASIC09 Reference Manual (Tandy), lines 5715-5726
--- END ---
```

```
--- CARD ---
id:        gfx2-arc-function-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 ARC function
claim:     ARC draws an arc with specified X- and Y-radii. Syntax: RUN GFX2(path, "ARC"[, xradius, yradius, xcor1, ycor1, xcor2, ycor2]). If same radius for X and Y, draws circular arc; otherwise elliptical. Begins drawing from point closest to first coordinate set, ends at point closest to second coordinate set. X-coordinates range 0-639, Y-coordinates 0-191.
context:   Tandy OCR line 5951 syntax reads "RUN GEN2 path, "SARC" lax zal, xrad, vrad, xcorl,yoorl, xcor2, ycor2)" — GEN2→GFX2, "SARC"→"ARC", lax/zal→mzx/mzy (center coordinates), vrad→yrad, yoorl→ycor1. Syntax structure is recognizable but OCR introduces spurious parameters. Level 2 version needed.
source:    BASIC09 Reference Manual (Tandy), lines 5949-5995
--- END ---
```

```
--- CARD ---
id:        gfx2-bar-function-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 BAR function (filled rectangle)
claim:     BAR fills rectangular area defined by diagonal from first coordinate set to second. Syntax: RUN GFX2(path, "BAR", xcor1, ycor1, xcor2, ycor2). If omitted, beginning coordinates use current draw pointer position. X-coordinates 0-639, Y-coordinates 0-191.
context:   Tandy OCR line 5020 syntax reads "RUN GPRS path, "BAR" xeori,ycordi), xCore, weor2)" — GPRS→GFX2, xeori→xcor1, ycordi→ycor1, xCore→xcor2, weor2→ycor2. Noticeable OCR corruption but function intent is clear.
source:    BASIC09 Reference Manual (Tandy), lines 6018-6086
--- END ---
```

```
--- CARD ---
id:        gfx2-bell-function
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 BELL function (terminal beep)
claim:     BELL rings terminal bell (produces beep through speaker). Syntax: RUN GFX2("BELL"). Parameters: None.
context:   Tandy OCR line 6102 syntax: "RUN GEXR2C'BELL'')" — GEXR2→GFX2, quote/paren mangled. Function is straightforward and should match Level 2 manual exactly.
source:    BASIC09 Reference Manual (Tandy), lines 6100-6115
--- END ---
```

```
--- CARD ---
id:        gfx2-blnkon-blnkoff-functions
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 BLNKON and BLNKOFF functions (character blink control)
claim:     BLNKON causes subsequent characters to blink on hardware text screens (/W1-/W7). BLNKOFF cancels blink; already-blinking characters continue. Blink does not operate on graphics windows. Syntax: RUN GFX2(path, "BLNKON") and RUN GFX2(path, "BLNKOFF"). Parameters: path (window route).
context:   Tandy OCR lines 6128-6129 syntax reads "RUN GEX 20 path,' BLNKON"'>" and "RUN GEN 2 path,|**"BLNKOFF'))" — GEX/GEN→GFX2, quote/bracket combinations are corrupted. Function names BLNKON/BLNKOFF are clear despite OCR noise.
source:    BASIC09 Reference Manual (Tandy), lines 6125-6152
--- END ---
```

```
--- CARD ---
id:        gfx2-boldsw-function-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 BOLDSW function (bold character control)
claim:     BOLDSW controls bold typeface display. Works on graphics screens only. Syntax: RUN GFX2(path, "BOLDSW", switch). Switch parameter is either "ON" (enable bold) or "OFF" (disable bold). Default is regular typeface.
context:   Tandy OCR line 6159 syntax: "RUN GEXA path, I BOLDSW'"',**switch')" — GEXA→GFX2, quote/bracket/pipe characters corrupted. Switch values ("ON"/"OFF") are clear. Level 2 manual should disambiguate signature.
source:    BASIC09 Reference Manual (Tandy), lines 6157-6182
--- END ---
```

```
--- CARD ---
id:        gfx2-border-function-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 BORDER function (set window border color)
claim:     BORDER sets window border color by resetting palette register 0 to specified color code. Syntax: RUN GFX2(path, "BORDER", color). Parameter color can be constant or variable. Affects Register 0 of palette.
context:   Tandy OCR line 6251 syntax: "RUN GEN2C path, "BORDER" ,color)" — GEN2C→GFX2, comma spacing is odd. Function behavior matches Level 2 manual structure.
source:    BASIC09 Reference Manual (Tandy), lines 6249-6272
--- END ---
```

```
--- CARD ---
id:        gfx2-box-function-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 BOX function (draw rectangle outline)
claim:     BOX draws rectangle outline defined by diagonal from first to second coordinate set. Does NOT reset draw pointer. Syntax: RUN GFX2(path, "BOX", xcor1, ycor1, xcor2, ycor2). If beginning coordinates omitted, uses current draw pointer position. X-coordinates 0-639, Y-coordinates 0-191.
context:   Tandy OCR line 6322 syntax: "RUN GEX2Z path, BOX'|, xcori,vcor), XO,YCOrzZ)" — GEX2Z→GFX2, BOX'|→"BOX", xcori→xcor1, vcor→ycor1, XO→xcor2, YCOrzZ→ycor2. Significant OCR corruption but intent recognizable. Level 2 needed.
source:    BASIC09 Reference Manual (Tandy), lines 6320-6354
--- END ---
```

```
--- CARD ---
id:        gfx2-circle-function-syntax-hr
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 CIRCLE function (high-resolution)
claim:     CIRCLE draws circle with specified radius. Syntax: RUN GFX2(path, "CIRCLE"[, xcor, ycor], radius). If coordinates omitted, uses current draw pointer position. X-coordinates 0-639, Y-coordinates 0-191. Radius unit is pixels.
context:   Tandy OCR line 6415 syntax: "RUN GEXN2([ path, |'CIRCLE"[,xeor, yeor), radims)" — GEXN2→GFX2, xeor→xcor, yeor→ycor, radims→radius. OCR corruption is extensive but structure clear.
source:    BASIC09 Reference Manual (Tandy), lines 6413-6444
--- END ---
```

```
--- CARD ---
id:        gfx2-clear-function-hr
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 CLEAR function (high-resolution)
claim:     CLEAR clears current working area of window. Does not change draw pointer location but resets text cursor and graphics cursor to upper-left corner of window. Syntax: RUN GFX2(path, "CLEAR").
context:   Tandy OCR line 6484 syntax: "RUN GEXA path, |'CLEAR'');" — GEXA→GFX2, quote mangled. Function behavior is straightforward.
source:    BASIC09 Reference Manual (Tandy), lines 6482-6501
--- END ---
```

```
--- CARD ---
id:        gfx2-color-function-syntax-hr
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 COLOR function (set screen colors, high-resolution)
claim:     COLOR changes foreground, background, and/or border colors. Does not change draw pointer position. Syntax: RUN GFX2(path, "COLOR", foreground[, background[, border]]). Parameters are palette register numbers. All three are optional, allowing partial color updates.
context:   Tandy OCR line 6509 syntax: "RUN GEXR2 pathy'COoOLOor"', foregroundl,back ground, borecder))" — GEXR2→GFX2, foregroundl→foreground, back ground→background (space inserted), borecder→border. OCR corrupted but intent is clear.
source:    BASIC09 Reference Manual (Tandy), lines 6507-6539
--- END ---
```

```
--- CARD ---
id:        gfx2-crritn-function
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 CRRITN function (carriage return)
claim:     CRRITN performs carriage return: moves text cursor down one line and to extreme left of window. Syntax: RUN GFX2(path, "CRRITN").
context:   Tandy OCR line 6588 syntax: "RUN GFX2C path, 'CRRITN'')" — quote/paren mangling present. Function name CRRITN is clear, behavior matches text-mode terminal carriage-return semantics.
source:    BASIC09 Reference Manual (Tandy), lines 6585-6604
--- END ---
```

```
--- CARD ---
id:        gfx2-cursor-movement-group
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 cursor movement functions (CURHOME, CURXY, CURDWN, CURUP, CURRGT, CURLET)
claim:     Cursor movement functions: CURHOME (move to 0,0 top-left), CURXY(col,row) (move to specific position), CURDWN (down one line), CURUP (up one line), CURRGT (right one char), CURLET (left one char). All work on text cursor. CURXY coordinates are relative to window's character dimensions, not pixels.
context:   Tandy OCR has pervasive corruptions: CURDWN missing function name header (line 6610), CURHOME: "cursd6r" (line 6645) should be "cursor", CURLET line 6667 syntax "RUN GEXN2U path sCURLET'" — GEXN2U→GFX2, sCURLET→"CURLET", CURRGT line 6741 "RUN GEX26" → GFX2. Despite heavy OCR noise, function names and movement semantics are recoverable. Level 2 version needed for clean reference.
source:    BASIC09 Reference Manual (Tandy), lines 6610-6778
--- END ---
```

```
--- CARD ---
id:        gfx2-curofe-vs-curoff-ambiguity
type:      OPEN-QUESTION
target:    6809
verify:    from-manual
topic:     GFX2 cursor visibility functions (CUROFE or CUROFF?)
claim:     Tandy OCR line 6691 shows "CUROFE" as function name for "Turn off cursor". This is almost certainly OCR misread of "CUROFF" (with 'F' and 'E' confused). Level 2 manual needed to confirm correct spelling.
context:   OCR unclear: line 6691 "CUROFE Turn off cursor" vs. line 6717 "CURON Turn .onm cursor" (with "onm" corrupted from "on"). Parallel form suggests CUROFF is correct, not CUROFE.
source:    BASIC09 Reference Manual (Tandy), lines 6691-6733
--- END ---
```

```
--- CARD ---
id:        gfx2-curon-function
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 CURON function (make text cursor visible)
claim:     CURON makes text cursor visible. Syntax: RUN GFX2(path, "CURON"). Inverse operation to CUROFF/CUROFE.
context:   Tandy OCR line 6719 syntax: "RUN GEXA path, |"CURON")" — GEXA→GFX2, quote/pipe corrupted. Function name and purpose are clear.
source:    BASIC09 Reference Manual (Tandy), lines 6717-6733
--- END ---
```

```
--- CARD ---
id:        gfx2-cwarea-function-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 CWAREA function (change working area)
claim:     CWAREA restricts output in window to specified area. New area must be same or smaller than previous working area. OS-9 scales graphics and text coordinates and images to new proportions; text character size unchanged. Syntax: RUN GFX2(path, "CWAREA", xcor, ycor, sizex, sizey). Parameters: xcor/ycor (upper-left corner in char units relative to original window), sizex/sizey (new working area char width/height).
context:   Tandy OCR line 6823 syntax: "RUN GEM 2Cpath, 'CwWARE SA", xcor, ycor, sizex, sizey)" — GEM→GFX, CwWARE SA→"CWAREA". Scaling behavior is documented and should match Level 2 manual.
source:    BASIC09 Reference Manual (Tandy), lines 6821-6884
--- END ---
```

```
--- CARD ---
id:        gfx2-defbuf-function-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 DEFBUF function (define GET/PUT buffer)
claim:     DEFBUF allocates a buffer for GET/PUT graphics operations. Syntax: RUN GFX2("DEFBUF", group, buffer, size). Group range 1-199, buffer range 1-255, size 1-8192 bytes (per-group). Each group allocates 8KB; 30 bytes overhead leaves ~8162 bytes per group for buffers. Buffer remains allocated until KILLBUFF called.
context:   Tandy OCR line 6910 syntax: "RUN GFEX2C'DEFBUEE",group, butfer,size)" — GFEX2→GFX2, DEFBUEE→DEFBUF, butfer→buffer. OCR is messy but parameters are clear. Level 2 version needed.
source:    BASIC09 Reference Manual (Tandy), lines 6907-6954
--- END ---
```

```
--- CARD ---
id:        gfx2-defcol-function
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 DEFCOL function (restore default palette colors)
claim:     DEFCOL resets all palette registers back to their default color values. Monitor type determines actual hues. Syntax: RUN GFX2(path, "DEFCOL").
context:   Tandy OCR line 6965 syntax: "RUN GEX2 path,' DEFCOL'" — GEX2→GFX2, quote mangled. Function is simple and should match Level 2 manual.
source:    BASIC09 Reference Manual (Tandy), lines 6963-6981
--- END ---
```

```
--- CARD ---
id:        gfx2-dellin-function
type:      FACT
target:    6809
verify:    from-manual
topic:     GFX2 DELLIN function (delete current text line)
claim:     DELLIN deletes the line at graphics cursor position. Syntax: RUN GFX2(path, "DELLIN"). This is a text-mode function, not graphics-mode.
context:   Tandy OCR line 6992 shows start of DELLIN entry but text is cut off at line 6995 in the provided range. Enough is visible to confirm function name and basic syntax structure.
source:    BASIC09 Reference Manual (Tandy), lines 6990-6995
--- END ---
```

---

## OCR Discrepancy Summary

### Critical Ambiguities Requiring Level 2 Manual Cross-Check:

1. **Format Code Corruption (Line 5186-5209):** Tandy OCR shows "ot", "o2", "os", "oc", "o7" which should likely be "01", "02", "05", "06", "07" or similar — this fundamentally alters format-code assignments for WCREATE. This is the single most important cross-check needed.

2. **Function Name Corruptions:** Throughout, "GFX"/"GFX2" rendered as "GEXC", "GEX", "GEN2", "GEXR2", "GEXA", "GEM", etc. Pattern is clear (E/F, X/X confusion), but exact function names in worked examples need Level 2 verification.

3. **Quote and Bracket Styling:** Mangled throughout. Tandy OCR shows curly quotes, stray pipes, odd paren/bracket combinations. Level 2 manual should clarify canonical parameter-list formatting (parentheses vs. brackets, quote style).

4. **CUROFE vs. CUROFF Ambiguity (Line 6691):** Tandy shows "CUROFE" for cursor-off function. Likely OCR misread of "CUROFF", but needs confirmation.

5. **Function Signature Clarity:** Many function syntaxes are corrupted in original OCR. Tandy examples like ARC (line 5951), BAR (line 6020), CIRCLE (line 6415), CURXY (line 6790) all have parameter-name corruption that makes direct signature comparison difficult.

### Functions Observed But Not Fully Extracted (Due to Line-Range Boundary):

- DELLIN (text line deletion) — entry starts at line 6990, cut off
- Additional G-Z functions beyond DELLIN in the alphabetic reference (not included in 4200-6600 range)

### Tandy-Specific OCR Patterns Observed:

- `E` ↔ `F` confusion (GEXC for GFX, GEXA for GFX2)
- `O` ↔ `0` confusion (ot for 01, o2 for 02, etc.)
- `l`/`1`/`I` confusion scattered throughout
- Curly quotes rendered as stray characters or `TM` sequences
- Stray pipes `|` and unusual bracket/paren combinations
- Space insertions in multi-word terms ("back ground" for "background", "CwWARE SA" for "CWAREA")

