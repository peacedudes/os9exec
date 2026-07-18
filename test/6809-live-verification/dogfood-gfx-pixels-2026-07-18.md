# Dogfood: first observed pixel output on the CoCo3 (2026-07-18)

Closes the long-standing "a text channel can't observe a screen" blocker that
had kept nearly all of `6809/gfx-windowing.md` at `Manual`. Graphics drawn on
a NitrOS-9 window are now **rendered, captured, and inspected**.

## The working method

Text goes over the socket, pixels come back over a screenshot. Do not drive
text by injecting keystrokes — it is drastically slower and it was the wrong
tool.

```sh
# full speed, sound off, but WITH a window so it can be photographed
NITROS9REPL_GUI=1 NITROS9REPL_EXTRA_XROAR="-no-ratelimit" \
    ./tools/nitros9repl.sh restart

./tools/nitros9repl.sh send 'wcreate /w4 -s=5 0 0 40 24 0 1 1'
./tools/nitros9repl.sh send 'display 1b 32 02 1b 40 00 20 00 20 1b 48 00 c0 00 a0 >/w4'
./tools/nitros9repl.sh send 'display 1b 40 00 a0 00 60 1b 50 00 40 >/w4'

./tools/cocoscreen.sh place 0 0     # reproducible window position
./tools/cocoscreen.sh cycle 4       # CLEAR-cycle screens, capture each
```

The drawing lands on a screen that is not displayed, so it must be cycled to
with CLEAR (host backtick) before it can be captured. `cycle` captures every
screen and prints an md5 per frame, so the graphics screen is found by
inspecting whichever frame differs.

Confirmed working: a yellow box outline and a circle, drawn with windint
`$1B` escape codes on a `wcreate`d 320x192 4-colour window, photographed
through `cocoscreen.sh`.

## RESOLVED — the coordinate space is normalized to the window

The "X is squashed" anomaly first noted below is fully explained. Measurements
on a `wcreate /w4 -s=5 0 0 40 24 0 1 1` window (40 chars = 320 pixels wide,
sitting on a 640-pixel-wide screen):

- Vertical lines drawn at x = 0, 160, 320, 480, 639 land **evenly spaced**
  across the window — a clean linear map, no clipping.
- A 40-character text row spans **exactly the same width** as x=0..639. Since
  40 chars x 8 px = 320 px, the whole 0-639 range maps onto 320 pixels.
- With the scale switch turned **off** (`display 1b 35 00`), the same five
  lines redraw at **literal pixel positions** 1:1 — only x=0 and x=160 remain
  visible, and x=320/480/639 clip at the window's 320-pixel right edge.

**Conclusion (`Live`):** with scaling on (the default), the documented
0-639 x 0-191 coordinate range is a *normalized* space stretched to fit the
**device window's** dimensions — not screen pixels. On a 320-pixel-wide
window every X is effectively halved. With `WScaleSw` ($1b35) off,
coordinates become literal window pixels and drawing clips at the window edge.

This also confirms `Manual` format code `5` = 640x192: the screen measured
640 pixels wide, with the 40-column window occupying its left half.

**Correction owed to `gfx-windowing.md`.** It currently says coordinates "are
screen-relative unless `SCALESW` is off, in which case they become relative to
the window's own working-area origin" — describing the switch as an *origin*
change. The switch demonstrably changes **scaling**. Note the origin half of
that claim is *untested here*: this window sits at 0,0, so window-relative and
screen-relative origins coincide. Retesting with a window at a non-zero
position is the obvious follow-up.

## Second finding — SELECT does not bring a screen forward

`display 1b 21 >/w4` (`WSelect`) produced no display change, and neither did
`display 1b 21 <>/w4`, which puts *all* of the writing process's std paths on
the target window. The screen only comes forward on a CLEAR keypress.
`gfx-windowing.md` says a SELECT from inside the target window "shows
immediately" — not reproduced. (An earlier session's apparent confirmation of
the deferral behavior was invalid, having used `$1b22`/`WOWSet` by mistake;
this pass used the correct `$1b21`.)

## Original observation (superseded by the above)

`gfx-windowing.md` states X range 0-639 / Y range 0-191 for `SETDPTR`,
`CIRCLE` and friends. Observed on a type-5 (320x192, 4-colour) window created
as 40x24 characters:

- A box `SetDPtr(32,32)` -> `Box(192,160)` renders with roughly correct
  **height** but a badly compressed **width**.
- A circle of radius 64 renders as an ellipse whose horizontal radius is
  about half its vertical radius. (Some ellipticity is expected from
  non-square pixels; the measured X:Y ratio is larger than that alone.)
- A box spanning the whole nominal space, `(0,0)` -> `(639,191)`, fills the
  **full height but only about half the width** of the display.

Consistent story: Y behaves as documented, X does not — the drawing is
either clipping at the device window's 320-pixel width or X is being scaled.
**Mechanism not established.** These are measurements off screenshots, not a
controlled sweep, and the numbers above should not be quoted as ratios.

Next step to settle it: draw single points at known X values (0, 159, 160,
319, 320, 639) and see exactly where each lands, and repeat on a 640-wide
screen type to separate "clipped at window width" from "scaled coordinate
space". Also worth testing with `WScaleSw` ($1b35) explicitly on and off,
since a scale switch would explain the behavior directly.

Until that is done, `gfx-windowing.md`'s coordinate-range claims stay
`Manual` — they are now *suspect*, not merely unverified.
