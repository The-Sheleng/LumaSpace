# Codex task M0 — Project Bootstrap

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

## Context
Repository is new. Read ARCHITECTURE.md, SPECIFICATION.md, ROADMAP.md and AGENTS.md. M0 only: do not build overlay, document renderer, screen capture, hotkey hooks, or editor yet.

## Goal
Create a reproducibly buildable Windows x64 C++20/Qt 6 Widgets application with tray lifecycle, logging, a minimal test, and a maintainable directory skeleton. Use MSVC, CMake and Qt dynamic linkage.

## Scope
1. `CMakeLists.txt`, `CMakePresets.json` (or documented equivalent), `.gitignore`, `README.md` with Windows prerequisites and exact configure/build/test commands.
2. `src/main.cpp` with QApplication, single-instance behavior (Qt local server/shared memory or equivalent with error handling), system tray icon/menu with Show Settings and Exit. Settings window may be placeholder but functional. Quit must cleanly release resources.
3. Module directories or lightweight targets: app, actions, document, interaction, viewport, render, capture, platform/windows, ui, persistence. Do not invent heavyweight abstractions before their milestone.
4. Logging initialization and clean error reporting for missing system tray availability.
5. Qt Test smoke test of pure startup-independent code; CTest integration.
6. Windows CI workflow building and running tests (if credentials/environment do not allow running CI, add workflow and report unverified).
7. Include `THIRD_PARTY_NOTICES.md` placeholder explaining Qt license review and dynamic deployment obligation; do not assert compliance is complete.

## Acceptance criteria
- Fresh Windows machine with documented prerequisites: configure, compile Release/Debug, run tests.
- App starts with tray icon; Settings opens and closes; Exit terminates process without ghost icon; second launch does not create a second instance.
- No overlay windows, screen capture or global hooks yet.
- Repo has no checked-in build artifacts or proprietary Qt binaries.
- Build instructions are reproducible and match CI.

## Verification
Run Windows CMake configure/build/ctest commands and report exact results. Manually verify tray/second-instance/exit. If Windows execution unavailable, mark these checks BLOCKED/UNVERIFIED; do not fake success. Provide changed file list, known limitations, commit hash or UNCOMMITTED, working tree status, and next task M1.1.

## Constraints
Do not change architecture baseline. Do not implement M1 or later. Ask for clarification only if blocked by a fundamental decision; otherwise choose small reversible defaults and document them.

## Naming acceptance checks
- Main window/tray tooltip/Settings title display `LumaSpace`.
- CMake target is exactly `LumaSpace`; resulting Windows executable is `LumaSpace.exe`.
- Initialize QCoreApplication organization/application names to `LumaSpace` before creating settings.
- Use `lumaspace` namespace for authored application code.
- The proposed reverse-DNS identifier is metadata only; do not assume trademark or domain ownership.
