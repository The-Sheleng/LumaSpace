# Phase A progress

The Phase A task package is executed in the authoritative order recorded in its
`01_TASK_INDEX.md`. Each row records local implementation evidence; reviewer and
remote CI acceptance remain separate where required.

| Task | State | Commit | Evidence |
|---|---|---|---|
| M1.1 Pure application mode state machine | IMPLEMENTED—VERIFYING | This task commit | Clean Debug/Release builds; 2/2 CTest tests passed in each; tray Exit confirmed with no visible overlay or residual process. |
| M1.2 Typed Action Registry | IMPLEMENTED—VERIFYING | This task commit | Clean Debug/Release builds; 3/3 CTest tests passed in each; tray and Settings lifecycle confirmed. |
| M1.3 Global Hotkeys & input semantics | IMPLEMENTED—VERIFYING | This task commit | Clean Debug/Release builds; 4/4 CTest tests passed; real foreground textbox retained focus/input across all mode hotkeys and Settings; registrations released on Exit. |
| M1.4 Lifecycle-to-mode transition coordination | IMPLEMENTED—VERIFYING | This task commit | Fresh Debug/Release builds; 5/5 CTest tests passed; real F10 request rejected and logged without changing focus, creating a window, or capturing input. |
| M1.5 M1 integration, tests & acceptance gate | IMPLEMENTED—VERIFYING | This task commit | Fresh Debug/Release builds; 6/6 CTest tests passed; real tray Settings/close/Exit/relaunch matrix passed; isolated Notepad retained focus and exact input across F9/F10/F11; zero residual processes. |
| M2.1 Win32 transparent HWND overlay proof | IMPLEMENTED—VERIFYING | This task commit | Primary-monitor per-pixel layered HWND integrated; windowed and borderless probes confirmed exact input, click-through, focus preservation, expected native styles, visible marker pixels, and destruction on Disabled. |
| M2.2 Per-monitor surfaces & topology changes | IMPLEMENTED—VERIFYING | Pending checkpoint commit | Synthetic side-by-side/negative/stacked/primary-swap/disconnect/zero-display cases covered; live one-monitor path testable, but physical two-monitor detach/attach is BLOCKED by available hardware. |
