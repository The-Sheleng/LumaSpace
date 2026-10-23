# LumaSpace — Roadmap v1.0
Each milestone requires reproducible build/test evidence and, for platform features, real Windows manual verification. Estimates are active working days, not commitments.

| ID | Goal | Acceptance gate | Estimate |
|---|---|---|---|
| M0 | CMake, Qt executable, tray, logging, CI, skeleton | Fresh clone builds; tray open/exit; CI green | 1–2d |
| M1 | AppMode state machine, ActionRegistry, hotkeys | All transitions verified; no input leakage | 2–3d |
| M2 | Per-monitor transparent HWNDs, click-through, DPI | Dual monitor mixed DPI/negative origin works | 3–5d |
| M3 | Document model, layers, IDs, Direct2D, transforms | Model and coordinate tests; correct rendering | 4–6d |
| M4 | Snapshot Editing, toolbar, pan/zoom, selection | Frozen background and scene align; exit restores desktop | 3–5d |
| M5 | Pen, presets, move/delete, undo/redo | Stroke and gesture transaction tests | 4–6d |
| M6 | Paste images, sticky notes, editing/transforms | Clipboard and persistence-ready objects | 3–5d |
| M7 | Save/load, autosave, recovery, PNG export | Round-trip and recovery/export tests | 3–5d |
| M8 | Settings, shortcuts, policies, packaging, regression | Usable release build tested in real lesson | 4–7d |

MVP: M0–M8, roughly 27–44 active days before unknown debugging.

| ID | Post-MVP scope |
|---|---|
| M9 | Spotlight, focus rectangle, line focus |
| M10 | Shape tools, arrows, circles, text |
| M11 | Keystroke visualizer, cursor trail, timer |
| M12 | Multi-workspace and multi-canvas UI |
| M13 | Desktop/monitor/window/viewport anchors |
| M14 | Magnifier, freeze frame, before/after |
| M15 | Macros, temporary tools, advanced presets |
| M16 | Presentation profiles, active-app detection |
| M17 | OBS output and capture/composition pipeline |
| M18 | Versioned plugin SDK |
| M19 | Installer, updater, crash diagnostics, accessibility |

## Slices
M0.1 repo/build/skeleton; M0.2 tray/logging; M0.3 CI/test/docs.
M1.1 state machine; M1.2 registry; M1.3 hotkeys; M1.4 transitions; M1.5 tests.
M2.1 overlay HWND proof; M2.2 multi-monitor; M2.3 DPI mapping; M2.4 focus/input regression.
M3.1 math; M3.2 scene/layers; M3.3 renderer; M3.4 hit testing/tests.
M4.1 capture; M4.2 editor surfaces; M4.3 camera; M4.4 toolbar.
M5.1 strokes; M5.2 presets; M5.3 select/move; M5.4 command history.
M6.1 clipboard; M6.2 images; M6.3 notes; M6.4 transforms.
M7.1 schema/assets; M7.2 save/load; M7.3 recovery; M7.4 export.
M8.1 settings UI; M8.2 hotkey editor; M8.3 policies; M8.4 release QA.

## Task status
READY -> IN_PROGRESS -> VERIFYING -> DONE; BLOCKED when dependencies prevent progress. No DONE based only on Codex claims. Commit hash, dirty state, build command/output, tests and manual evidence required. Only one active critical-path task.
