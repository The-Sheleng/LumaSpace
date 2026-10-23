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
