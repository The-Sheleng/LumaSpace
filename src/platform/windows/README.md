# platform/windows

Reserved for Win32 integration. Native handles will be owned with RAII and used
only on their documented threads when this module is implemented.

The M1 hotkey service owns a message-only HWND and all `RegisterHotKey`
registrations on the Qt GUI thread. Destruction must occur on that same thread.
It supports `Press` only; `Toggle` and `Hold` are explicit unsupported contracts.
No low-level keyboard hook is installed.
