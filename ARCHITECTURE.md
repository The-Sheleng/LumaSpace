# LumaSpace — Architecture v1.0

## Product identity (locked for M0)
- Product/display name: `LumaSpace`.
- Repository/root directory: `LumaSpace`.
- CMake project and primary executable target: `LumaSpace` (`LumaSpace.exe` on Windows).
- C++ namespace for new application code: `lumaspace` (Qt-generated code may follow Qt conventions).
- Qt organization name: `LumaSpace`; application name: `LumaSpace` (settings under LumaSpace).
- Proposed reverse-DNS application identifier: `com.lumaspace.app` (provisional until publisher/domain ownership is confirmed).
- Future native document extension: `.lumaspace` (reserved; not implemented in M0).
- Do not use the previous working title in code, UI, paths, build targets or new documentation.
- Branding/trademark/domain availability has not been checked; do not claim exclusivity.
Status: approved design baseline; not implemented. Platform: Windows 10/11 x64.

## Technology
C++20, MSVC, CMake, Qt 6 Core/Gui/Widgets (dynamic linkage, check LGPL per module), Win32, Direct2D/DirectWrite, D3D11 + Windows Graphics Capture for snapshots when necessary, Qt Test, JSON + binary assets. Windows CI. No Unreal, Electron or service backend.

## Modules and ownership
- `app`: single instance, tray, lifecycle, mode controller.
- `actions`: registry, binding, trigger semantics, activation policy, shortcut conflict detection.
- `document`: Workspace > Canvas > ordered Layers > SceneObjects; ObjectId, commands, model.
- `interaction`: selection, tools, pointer events, temporary tool stack, gesture transactions.
- `viewport`: monitor topology, camera, coordinate transforms.
- `render`: scene rendering, per-monitor surfaces, offscreen export.
- `capture`: frozen desktop snapshots, capture lifecycle, exclusion of own overlays.
- `platform/windows`: native windows, DPI, hotkeys, clipboard, focus.
- `ui`: Qt settings, tray and floating movable toolbar.
- `persistence`: versioned project, autosave, recovery, asset storage.
Dependency rule: document model has no dependency on Win32, Qt Widgets, or rendering APIs. UI calls actions; actions mutate document through commands. Renderer reads model, does not own it. Keep IRenderBackend seam but do not build speculative multi-backend engine.

## Modes
`Disabled`: process and hotkeys running, overlay hidden, no input interception.
`Overlay`: document shown on click-through topmost windows, normal desktop usable.
`Editing`: freeze desktop background, show editable scene and movable toolbar, intercept editor input, pan/zoom scene AND background together. Toolbar remains fixed in UI space.
Transition into Editing: capture desktop WITHOUT own overlays (temporary hide/capture/show acceptable initially), fail safely back to previous mode. Exit Editing: finish/cancel gesture, release input, discard snapshot, restore identity desktop projection. Never mutate document coordinates on mode switch. Escape cancels gesture; mode switching is a separate action.
`ActivationPolicy`: EditingOnly / OverlayAndEditing / Always. This determines action eligibility, NOT continuous activation. `TriggerMode`: Toggle / Press / Hold. Hold needs key-up tracking beyond RegisterHotKey; use narrowly scoped Windows hook if necessary. Separate transient effects from document tools.

## Coordinates and multi-monitor
WorldSpace (double precision persistent scene); VirtualDesktopPhysical (signed physical pixel coordinates); MonitorLocalPhysical; EditingViewport; Qt UI logical space. Centralized conversions. Desktop projection is 1:1 world-to-virtual-desktop physical pixels; editing camera only affects editor viewport. Set Per-Monitor DPI Awareness V2 before Qt initializes. One overlay HWND per monitor; monitor-aligned snapshot tiles, never one enormous texture. Test negative origins, 100/150% DPI, rotated/disconnected displays and primary monitor changes. Preserve object world coordinates when monitor topology changes; later add Bring Offscreen Content Into View.

## Scene model
Stable object IDs, layer IDs, transforms, bounds, visibility. Layer order, lock, opacity and visibility. MVP types: Stroke (points, RGBA, width), Image (asset ref, transform), StickyNote (Unicode text, background, size). Renderer dispatches by object type, not virtual platform-dependent Render() on model. Hit-testing in world coordinates. Clipboard image -> ImageObject; plain text -> StickyNote. Selection decorations and presentation effects transient.
Undo/redo through command dispatcher; one drag or stroke is one history transaction. Presets are tool configuration, not document objects. Workspace and multiple canvases in schema, UI may initially expose one.

## Rendering/capture/export
Native transparent topmost click-through HWNDs for passive overlay; editing windows receive pointer input. Direct2D/DirectWrite renderer on native surfaces; Qt Widgets only for UI. Prove HWND/Direct2D integration early; do not mix QPainter and Direct2D on one surface. Event-driven redraw/dirty regions. Snapshot is ephemeral, not stored in project. Offscreen export independent of live desktop: transparent PNG content bounds or selected monitor crop. Guard empty scene and excessive pixel/memory allocations. Sharing an individual app window may omit overlays; validate display vs window capture separately.

## Persistence, safety and testing
Project format with schemaVersion, scene.json, assets; atomic save and debounced autosave to separate recovery location. Unknown schema versions never overwrite originals. Clipboard read only on explicit paste; no automatic screenshot persistence, telemetry or admin privileges. Restore focus on exit from Editing where possible. Tests: state transition table, transforms round-trip, undo/redo, serialization, asset loading. Manual Windows: mixed-DPI dual monitors, Alt+Tab, screen sharing, full-screen/borderless, monitor hotplug. Targets (not guarantees): near-zero idle rendering, <1% idle CPU on representative machine, responsive drawing, 10k lightweight objects.

## Architecture decisions requiring ADR
Renderer/backend replacement, coordinate contract, capture strategy, persistence container, Qt licensing/module choice, platform expansion, OBS composition pipeline. No Codex unilateral redesign.
