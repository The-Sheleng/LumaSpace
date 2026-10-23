# Codex development contract — LumaSpace

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
Read ARCHITECTURE.md, SPECIFICATION.md, ROADMAP.md before implementation. The user approves architectural changes; Codex must not silently change technology, module boundaries, coordinate system, mode semantics or MVP scope.

## Workflow
Implement ONE assigned task only. Inspect repository first. State assumptions, minimal change plan and affected files. Keep changes narrowly scoped; do not implement future milestones or speculative plugin APIs. Follow C++20, CMake, Qt 6, MSVC and Win32 compatibility. Prefer explicit ownership/RAII, no global mutable state except justified OS lifecycle state. Document Win32 handles and thread affinity. Qt UI only on main thread. Never use admin rights or network telemetry.

## Verification
Run available configure/build/test commands; never claim success if not run. If on Linux and Windows build is unavailable, explicitly report UNVERIFIED Windows compilation and give exact Windows commands. Add unit tests for new pure logic. For Windows UI behaviors give manual reproduction steps and expected results. Avoid committing binaries/build artifacts. Update relevant docs when behavior changes. Do not commit unless explicitly requested by task or repository policy.

## Report format (8–12 lines)
Task ID / status; summary; changed files; architectural decisions; build command/result; tests command/result; manual checks; known risks; commit hash or UNCOMMITTED; working tree state; next recommended task. Status DONE only after independent acceptance; Codex reports IMPLEMENTED/VERIFYING, not self-certified DONE.

## Forbidden shortcuts
No screenshot baked into scene. No zoom transformation persisted as object geometry. No click-blocking overlay in Disabled/Overlay. No unconditional keyboard hooks. No single giant virtual-desktop texture. No Qt commercial-only module without explicit licensing review. No broad rewrite for a narrow task.
