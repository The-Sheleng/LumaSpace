# DPI and coordinate-space contract

LumaSpace enables Per-Monitor DPI Awareness V2 before constructing
`QApplication`. Native overlay bounds from Win32 are therefore physical pixels.
Qt Widgets remain in Qt logical coordinates and must cross into native geometry
only through an explicit per-monitor DPI mapping; callers must not apply Qt's
device-pixel ratio a second time.

The coordinate types are intentionally non-interchangeable:

- `WorldPoint`: persistent double-precision scene coordinates.
- `VirtualDesktopPhysicalPoint`: signed physical position in the Windows virtual desktop.
- `MonitorLocalPhysicalPoint`: physical position relative to a stable monitor ID.
- `EditingViewportPoint`: editor-local coordinate; identity-only until M4 adds a camera.
- `QtLogicalPoint`: Qt UI coordinate converted with the selected monitor's X/Y DPI.

Desktop projection is an identity mapping: one world unit is one physical pixel.
Conversion to an integer pixel uses nearest rounding with exact half values away
from zero. Non-finite values and values beyond the exact-integer range of a
double are rejected instead of overflowing.

Mappings are atomically replaced by monitor ID after DPI or topology changes;
world coordinates never change. Fractional Qt logical positions are preserved as
doubles until rasterization. Rotated monitors use the physical bounds Windows
reports after rotation; this layer does not rotate content axes independently.
Mixed-DPI seam alignment and a negative-origin display still require the physical
two-monitor check deferred to A.1.
