#include "platform/windows/dpi_awareness.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace lumaspace {

DpiAwarenessResult enablePerMonitorDpiAwarenessV2() noexcept
{
    if (SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
        return DpiAwarenessResult::Enabled;
    }
    if (GetLastError() == ERROR_ACCESS_DENIED &&
        AreDpiAwarenessContextsEqual(GetThreadDpiAwarenessContext(),
                                     DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
        return DpiAwarenessResult::AlreadyEnabled;
    }
    return DpiAwarenessResult::Failed;
}

}  // namespace lumaspace
