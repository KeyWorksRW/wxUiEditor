# NanoSVG (vendored)

This is the NanoSVG copy bundled with wxWidgets 3.2 / 3.3.3 (`3rdparty/nanosvg`), vendored
here so wxUiEditor's Mockup can reproduce how a wxWidgets 3.2 application rasterizes SVG.

Upstream NanoSVG is by Mikko Mononen (zlib licence — see the header of `src/nanosvg.h`).

**Do not update** this to a newer NanoSVG fork. Newer forks (the ones bundled with recent
wxWidgets releases) support CSS `<style>` class rules, which wxWidgets 3.2 does not, so the
Mockup preview would no longer match what a wxWidgets 3.2 build draws.

Files:

- `src/nanosvg.h`
- `src/nanosvgrast.h`
