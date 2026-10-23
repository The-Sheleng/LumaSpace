# LumaSpace

LumaSpace is a Windows 10/11 x64 desktop application. M0 contains only the Qt
application lifecycle, a tray menu, a placeholder Settings window, logging,
single-instance enforcement, and the future module skeleton.

The proposed application identifier is `com.lumaspace.app`. It is provisional
metadata and does not imply ownership of a trademark or domain. The reserved
`.lumaspace` document extension is not implemented in M0.

## Windows prerequisites

- Windows 10 or 11 x64
- Visual Studio 2022 with **Desktop development with C++**, the MSVC v143 x64/x86
  build tools, and an x64 Windows SDK. A newer Visual Studio installation must
  also have the optional v143 component because the presets deliberately select
  the `Visual Studio 17 2022` generator used by CI.
- CMake 3.25 or newer
- Qt 6.8.3 for MSVC 2022 x64, installed as shared (dynamic) libraries, including
  Core, Gui, Widgets, and Test. This matches the Windows CI workflow.

Set `QT_ROOT_DIR` to the Qt kit root containing `lib/cmake/Qt6`, for example:

```powershell
$env:QT_ROOT_DIR = "C:\Qt\6.8.3\msvc2022_64"
```

If these components are absent, use Visual Studio Installer's **Modify** action
to add the workload/toolset, and Qt Online Installer's **Add or remove
components** action to add **Qt 6.8.3 > MSVC 2022 64-bit**. These are explicit
machine-level installation steps; the project does not install or alter them.

## Configure, build, and test

Run from a Visual Studio 2022 Developer PowerShell:

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug

cmake --preset windows-release
cmake --build --preset windows-release
ctest --preset windows-release
```

Run the application from `build/windows-debug/Debug/LumaSpace.exe` or
`build/windows-release/Release/LumaSpace.exe`. A development run needs the Qt
DLLs on `PATH`; deployment must use the appropriately licensed dynamic Qt DLLs
(normally prepared with `windeployqt`) rather than copying them into this repo.

## M0 manual checks

1. Start `LumaSpace.exe`; confirm the LumaSpace tray icon appears.
2. Choose **Show Settings**; confirm a window titled **LumaSpace Settings** opens,
   can be closed, and can be reopened from the tray.
3. Launch the executable again; confirm the second process exits and only one
   tray icon remains.
4. Choose **Exit**; confirm the process exits and its tray icon disappears.

Qt log messages are appended to `LumaSpace.log` in Qt's per-user
`AppLocalDataLocation` and mirrored to standard error when one is attached.
Missing tray availability is also shown as a blocking error dialog, after which
the process exits.

## Provisional M1 shortcuts

The following temporary, code-defined global shortcuts use `RegisterHotKey` and
the Press trigger. They are intended for technical verification until the M8
settings UI; conflicts cause an actionable startup error and all registrations
are rolled back.

- Ctrl+Alt+Shift+F9: Disabled
- Ctrl+Alt+Shift+F10: request Overlay
- Ctrl+Alt+Shift+F11: request Editing

Toggle and Hold are not supported in this slice. LumaSpace installs no low-level
keyboard hook, and normal keys remain owned by the foreground application.
Until M2 supplies native overlay resources and M4 supplies editing capture/input
resources, the production adapter rejects the F10 and F11 requests as
`NotImplemented` and records the reason in `LumaSpace.log`; it does not claim a
logical mode that is not visibly active. The transaction contract and future
adapter responsibilities are documented in
[`docs/mode-transition-coordination.md`](docs/mode-transition-coordination.md).
