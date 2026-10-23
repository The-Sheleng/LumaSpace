# LumaSpace — Functional Specification v1.0

## User workflow
App starts in system tray, no account/server. Modes switched with configurable global shortcuts:
1. Disabled — nothing visible, normal PC, shortcuts remain active.
2. Overlay — existing annotations visible, all mouse input passes to underlying apps.
3. Editing — frozen desktop screenshot as background, movable toolbar, editable infinite canvas, pan/zoom both screenshot and document; toolbar does not zoom. Return to mode 1/2 restores live desktop and unzoomed world-to-desktop projection.

## MVP behavior
Pen: adjustable RGBA color, opacity, width, saved named presets. Select: move/resize/delete objects, locked layers not editable. Hand: pan; pointer-centered wheel zoom; temporary Space-to-pan. Paste from clipboard: image creates image object, text creates editable sticky note; prioritize actual bitmap content when both exist. Sticky notes: Unicode text, background, resize/move. Images: resize/move, preserve source asset. Layers: reorder, visibility, opacity, lock. Undo/redo for document mutations. Floating toolbar can be dragged anywhere and must not block mode switch. Export: transparent PNG of content bounds or crop to selected monitor; define size/memory limit. Save/open project, autosave/recovery. Settings: shortcuts, action activation policy, tool properties/presets, autosave.

## Input contracts
Disabled and Overlay do not intercept normal mouse input. Editing captures mouse only in editor, not through hidden overlay. Hotkey action eligibility EditingOnly/OverlayAndEditing/Always independent of Press/Toggle/Hold trigger. Reject conflicting shortcuts. Mode switches never discard edits. On capture failure remain in previous mode and show error. All editor dialogs must allow keyboard navigation.

## Multi-monitor
Unified world space, per-monitor windows, mixed DPI, negative desktop coordinates, hotplug. An object spanning displays remains one object. On disconnect keep object coordinates; do not silently relocate. Snapshot tiles aligned to desktop coordinates. Window-only sharing may not show overlays; document as known limitation.

## Post-MVP
M9 spotlight/focus rectangle/line focus; M10 arrows/shapes; M11 keystrokes/cursor trail/timer; M12 multiple canvases/workspaces; M13 window/monitor/viewport anchors; M14 magnifier/freeze/compare; M15 macros; M16 app-specific profiles; M17 OBS pipeline; M18 plugin SDK; M19 installer/updater/diagnostics.

## Pending UX defaults to approve during implementation
Default hotkeys and direct vs cycling mode transitions; zoom min/max; toolbar position persistence; screenshot refresh action; sticky note visual style. Do not let Codex silently lock these into irreversible architecture.
