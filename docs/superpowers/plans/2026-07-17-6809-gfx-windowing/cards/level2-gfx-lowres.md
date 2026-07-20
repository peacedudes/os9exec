# GFX Level-2 Medium-Resolution Graphics — Fact Cards

Source: OS-9 Level 2 Operating System Manual, lines 19300-20240

---

## Screen Modes & Initialization

--- CARD ---
id:        gfx-mode-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-mode
claim:     MODE function switches screen from alphanumeric (text) to graphics mode and must be called before any other graphics function can be used.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 20049-20074
--- END ---

--- CARD ---
id:        gfx-mode-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-mode
claim:     MODE syntax is: RUN GFX("MODE", format, color), where format is 0 or 1, and color is a color code.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 20051
--- END ---

--- CARD ---
id:        gfx-mode-format-0
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-mode-formats
claim:     Format 0 provides 256 horizontal points by 192 vertical points with a maximum of 2 colors on screen at once.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19425-19426
--- END ---

--- CARD ---
id:        gfx-mode-format-1
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-mode-formats
claim:     Format 1 provides 128 horizontal points by 192 vertical points with a maximum of 4 colors on screen at once.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19428-19429
--- END ---

--- CARD ---
id:        gfx-mode-memory-allocation
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-mode
claim:     Calling MODE allocates a 6-kilobyte block of memory for graphics; if insufficient memory is available, OS-9 returns an error message.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 20057-20059
--- END ---

--- CARD ---
id:        gfx-coordinate-system
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-coordinates
claim:     Graphics coordinate system has 0,0 at the lower left corner of the screen; all points on the grid are positive.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19430-19432
--- END ---

--- CARD ---
id:        gfx-draw-pointer-initial
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-draw-pointer
claim:     When a graphics screen is established, the draw pointer (invisible graphics cursor) is initially located at coordinates 0,0.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19504-19505
--- END ---

--- CARD ---
id:        gfx-draw-pointer-tracking
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-draw-pointer
claim:     Some graphics functions automatically change the draw pointer location; for instance, the LINE function moves the draw pointer from beginning coordinates to ending coordinates.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19505-19508
--- END ---

---

## Alphanumeric & Cleanup

--- CARD ---
id:        gfx-alpha-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-alpha
claim:     ALPHA function switches from graphics screen to alphanumeric (text) screen; the graphics screen remains intact in memory.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19527-19529
--- END ---

--- CARD ---
id:        gfx-alpha-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-alpha
claim:     ALPHA syntax is: RUN GFX("ALPHA") with no parameters.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19524
--- END ---

--- CARD ---
id:        gfx-quit-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-quit
claim:     QUIT function switches screen to alphanumeric (text) mode and deallocates graphics memory.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 20220-20221
--- END ---

--- CARD ---
id:        gfx-quit-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-quit
claim:     QUIT syntax is: RUN GFX("QUIT") with no parameters.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 20217
--- END ---

---

## Screen Clearing & Color

--- CARD ---
id:        gfx-clear-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-clear
claim:     CLEAR function clears the current graphics screen and sets the graphics cursor at coordinates 0,0 (lower left corner).
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19717-19720
--- END ---

--- CARD ---
id:        gfx-clear-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-clear
claim:     CLEAR syntax is: RUN GFX("CLEAR"[,color]), where color parameter is optional.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19714
--- END ---

--- CARD ---
id:        gfx-clear-color-behavior
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-clear
claim:     If color is not specified in CLEAR, the function sets the entire screen to the current background color; if color is specified, it clears to that color.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19717-19718
--- END ---

--- CARD ---
id:        gfx-color-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-color
claim:     COLOR function changes the foreground color and may change the color set, but does not change the graphics format or cursor position.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19747-19749
--- END ---

--- CARD ---
id:        gfx-color-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-color
claim:     COLOR syntax is: RUN GFX("COLOR", color), where color is a color code.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19744
--- END ---

--- CARD ---
id:        gfx-color-set-behavior
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-color
claim:     BASIC09 divides color codes into color sets; specifying a color code outside the current color set automatically initializes the new set.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19441-19445
--- END ---

---

## Drawing Primitives

--- CARD ---
id:        gfx-point-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-point
claim:     POINT function displays a dot at specified coordinates; if color is not specified, it uses the current foreground color.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 20148-20151
--- END ---

--- CARD ---
id:        gfx-point-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-point
claim:     POINT syntax is: RUN GFX("POINT", xcor, ycor[, color]), where xcor and ycor are required and color is optional.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 20145
--- END ---

--- CARD ---
id:        gfx-line-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-line
claim:     LINE function draws a line either from current draw position to specified end coordinates, or from specified beginning coordinates to specified ending coordinates.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19977-19984
--- END ---

--- CARD ---
id:        gfx-line-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-line
claim:     LINE syntax is: RUN GFX("LINE"[,xcor1,ycor1],xcor2,ycor2[,color]), where xcor1/ycor1 are optional beginning coordinates, xcor2/ycor2 are ending coordinates, and color is optional.
context:   OCR unclear: syntax line has formatting noise but essential parameters are clear
source:    OS-9 Level 2 Operating System Manual, line 19972-19973
--- END ---

--- CARD ---
id:        gfx-line-cursor-update
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-line
claim:     LINE function moves the draw pointer from the beginning coordinates to the end coordinates, affecting subsequent graphics operations.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19507-19508
--- END ---

--- CARD ---
id:        gfx-line-color-default
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-line
claim:     LINE function draws in the current or specified foreground color.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19977
--- END ---

--- CARD ---
id:        gfx-circle-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-circle
claim:     CIRCLE function draws a circle of a given radius; if xcor and ycor are omitted, it uses the current graphics cursor position as the circle's center.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19609-19612
--- END ---

--- CARD ---
id:        gfx-circle-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-circle
claim:     CIRCLE syntax is: RUN GFX("CIRCLE", xcor, ycor, radius[, color]), where xcor, ycor, and radius are shown in the syntax; center coordinates are optional and color is optional.
context:   OCR artifact: syntax shows "CIRCLE"L,xcor,ycorl — the L and l are OCR noise for actual brackets
source:    OS-9 Level 2 Operating System Manual, line 19606
--- END ---

--- CARD ---
id:        gfx-circle-color-default
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-circle
claim:     CIRCLE function uses the current foreground color if color is not specified.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19610
--- END ---

--- CARD ---
id:        gfx-circle-coordinates-range
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-circle
claim:     Specifying CIRCLE coordinates outside the X-coordinate range 0-255 or Y-coordinate range 0-191 causes an error.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19623-19625
--- END ---

---

## Cursor Movement

--- CARD ---
id:        gfx-move-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-move
claim:     MOVE function positions the invisible graphics cursor (draw pointer) to specified coordinates without changing the display.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 20089-20091
--- END ---

--- CARD ---
id:        gfx-move-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-move
claim:     MOVE syntax is: RUN GFX("MOVE", xcor, ycor), where xcor and ycor are the cursor coordinates.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 20086
--- END ---

--- CARD ---
id:        gfx-move-no-display-change
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-move
claim:     MOVE function does not change the display in any way; it only updates the internal draw pointer position.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 20089-20091
--- END ---

---

## Input & Memory Access

--- CARD ---
id:        gfx-joystk-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-joystk
claim:     JOYSTK function determines the status of a specified joystick fire button and the X,Y position of the joystick handle.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19907-19908
--- END ---

--- CARD ---
id:        gfx-joystk-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-joystk
claim:     JOYSTK syntax is: RUN GFX("JOYSTK", stick, fire, xcor, ycor), where stick, fire, xcor, and ycor are required parameters as variable names or values.
context:   OCR artifact: syntax spans lines 19902-19904 with line break inside function name
source:    OS-9 Level 2 Operating System Manual, line 19902-19904
--- END ---

--- CARD ---
id:        gfx-joystk-stick-parameter
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-joystk
claim:     JOYSTK stick parameter takes 0 for the right joystick or 1 for the left joystick.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19915-19917
--- END ---

--- CARD ---
id:        gfx-joystk-fire-parameter
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-joystk
claim:     JOYSTK fire parameter is a variable in which JOYSTK returns the button status; fire can be byte, integer, or boolean type, with non-zero or TRUE indicating the button is pressed.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19919-19923
--- END ---

--- CARD ---
id:        gfx-joystk-coordinate-range
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-joystk
claim:     JOYSTK xcor and ycor parameters return joystick handle position in the range 0-63; parameters can be byte or integer type variables.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19925-19928
--- END ---

--- CARD ---
id:        gfx-joystk-hardware-compatibility
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-joystk
claim:     JOYSTK function works only with a standard joystick or mouse, not with the high-resolution mouse adapter.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19909-19910
--- END ---

--- CARD ---
id:        gfx-gloc-function
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-gloc
claim:     GLOC function determines the location of the graphics screen in memory and returns the address in the specified variable.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19821-19822
--- END ---

--- CARD ---
id:        gfx-gloc-syntax
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-gloc
claim:     GLOC syntax is: RUN GFX("GLOC", storage), where storage is a variable name.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19818
--- END ---

--- CARD ---
id:        gfx-gloc-storage-parameter
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-gloc
claim:     GLOC storage parameter is an integer or byte type variable in which GLOC stores the memory address of the graphics screen.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19837-19839
--- END ---

--- CARD ---
id:        gfx-gloc-peek-poke-access
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-gloc
claim:     After obtaining the graphics screen address via GLOC, BASIC09 programs can use PEEK and POKE to perform special graphics functions not available in the GFX module, such as filling a portion of the screen with a color or saving a graphics screen to disk.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19823-19826
--- END ---

--- CARD ---
id:        gfx-gloc-memory-requirements
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-gloc
claim:     OS-9 Level Two maps display screens into a program's address space before PEEK and POKE can operate on a display screen; this requires at least 8 kilobytes of free memory in the user's address space.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19828-19831
--- END ---

--- CARD ---
id:        gfx-gloc-memory-constraints
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-gloc
claim:     Program and data memory requirements in BASIC09 programs using GLOC must not exceed 56 kilobytes.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19831-19832
--- END ---

---

## Module Loading & General Notes

--- CARD ---
id:        gfx-module-loading
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-loading
claim:     GFX module must be in the execution directory or resident in memory when called by BASIC09; it can be preloaded using the LOAD command or loaded automatically when first graphics function is called.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19341-19346
--- END ---

--- CARD ---
id:        gfx-module-residency
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-loading
claim:     Once GFX is loaded, it resides in memory until removed using the OS-9 UNLINK command or the BASIC09 KILL command.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19346-19347
--- END ---

--- CARD ---
id:        gfx-function-parameter-passing
type:      FACT
target:    6809
verify:    from-manual
topic:     gfx-general
claim:     GFX functions are passed as parameters with the RUN statement; for example, RUN GFX("CLEAR") clears the current graphics screen.
context:   None
source:    OS-9 Level 2 Operating System Manual, line 19357-19361
--- END ---

---

## Undocumented/Incomplete in Range

--- CARD ---
id:        gfx-gcolr-missing-section
type:      OPEN-QUESTION
target:    6809
verify:    from-manual
topic:     gfx-gcolr
claim:     The manual's quick reference lists GCOLR as a function that "reads a pixel's color" and takes "names of variables in which to store optional X- and Y-coordinates," but no detailed syntax or parameter section appears in lines 19300-20240.
context:   OCR note: function name appears as "GCOLR" in line 19382; unclear if this is OCR artifact for "GCOLOR". No detailed section found between COLOR (line 19742-19806) and GLOC (line 19816), where it would be expected.
source:    OS-9 Level 2 Operating System Manual, line 19382-19384 (quick reference only)
--- END ---

