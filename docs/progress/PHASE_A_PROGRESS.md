# Phase A progress

The Phase A task package is executed in the authoritative order recorded in its
`01_TASK_INDEX.md`. Each row records local implementation evidence; reviewer and
remote CI acceptance remain separate where required.

| Task | State | Commit | Evidence |
|---|---|---|---|
| M1.1 Pure application mode state machine | IMPLEMENTED—VERIFYING | `1dc6527` | Clean Debug/Release builds; 2/2 CTest tests passed in each; tray Exit confirmed with no visible overlay or residual process. |
| M1.2 Typed Action Registry | IMPLEMENTED—VERIFYING | `03404b1` | Clean Debug/Release builds; 3/3 CTest tests passed in each; tray and Settings lifecycle confirmed. |
| M1.3 Global Hotkeys & input semantics | IMPLEMENTED—VERIFYING | `feb42fb` | Clean Debug/Release builds; 4/4 CTest tests passed; real foreground textbox retained focus/input across all mode hotkeys and Settings; registrations released on Exit. |
| M1.4 Lifecycle-to-mode transition coordination | IMPLEMENTED—VERIFYING | `b271630` | Fresh Debug/Release builds; 5/5 CTest tests passed; real F10 request rejected and logged without changing focus, creating a window, or capturing input. |
| M1.5 M1 integration, tests & acceptance gate | IMPLEMENTED—VERIFYING | `ae70423` | Fresh Debug/Release builds; 6/6 CTest tests passed; real tray Settings/close/Exit/relaunch matrix passed; isolated Notepad retained focus and exact input across F9/F10/F11; zero residual processes. |
| M2.1 Win32 transparent HWND overlay proof | IMPLEMENTED—VERIFYING | `871e801` | Fresh Debug/Release builds; 7/7 CTest tests passed; windowed and borderless probes confirmed exact input, click-through, focus preservation, required native styles, visible marker pixels, Alt+Tab exclusion, and destruction on Disabled. |
| M2.2 Per-monitor surfaces & topology changes | IMPLEMENTED—VERIFYING | `2053602` | No unresolved code-level or automated-test failures: fresh Debug/Release builds and 7/7 CTest tests passed. Synthetic side-by-side, negative-origin, stacked, primary-swap, disconnect, zero-display, refresh failure and idempotency cases passed. Live single-monitor bounds, display-change refresh, teardown and diagnostics passed. Physical multi-monitor verification is deferred to A.1 and is not represented by simulation. |
| M2.3 DPI & coordinate conversion foundations | IMPLEMENTED—VERIFYING | This task commit | No unresolved code-level or automated-test failures: fresh Debug/Release builds and 8/8 CTest tests passed. Live single-display evidence confirms Per-Monitor V2, 144 DPI (150%), marker alignment and clean teardown. Mandatory 100% plus mixed-DPI physical seam/focus verification is BLOCKED by the available single 150% display; unlike M2.2, this gate has not been authorized for deferral. |

## Deferred M2.2 physical hardware gate — required at A.1

The following checks remain **PENDING HARDWARE VERIFICATION** and must pass on a
real two-monitor Windows system before Phase A can be marked complete:

1. Place one display left of the primary so its virtual-desktop X origin is negative.
2. Enter Overlay and confirm exactly one correctly bounded diagnostic surface and marker on each physical display, with no overlap or gap at desktop edges.
3. Click controls and type into foreground applications through both overlay surfaces; confirm neither surface receives focus or pointer input.
4. Detach one display while Overlay is active; confirm its surface is removed, the remaining surface stays correctly placed, and desktop input remains safe.
5. Reattach the display while Overlay is active; confirm its surface is recreated once with correct bounds and marker placement.
6. Swap the primary display and repeat seam alignment and click-through checks.
7. Confirm Disabled and Exit remove every overlay HWND and leave no residual process.

Synthetic displays and unit tests are implementation evidence only; they are not
equivalent to these physical multi-monitor observations.

## M2.3 physical DPI gate — current blocker

M2.3 cannot advance to M2.4 until these mandatory observations are available:

1. Verify marker and pointer alignment on a real display configured at 100% scale.
2. On two physical displays with different scale factors, place one left of the primary and verify alignment across the seam without offset or drift.
3. Move the foreground interaction probe between the displays and confirm Overlay preserves keyboard focus and pointer pass-through at both scale factors.

The current machine has one 1920×1080 display at 144 DPI (150%). Numeric tests
for 100/125/150/200% and synthetic negative origins are not substitutes for this
mandatory physical observation.
