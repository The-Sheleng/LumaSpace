#pragma once

namespace lumaspace {

enum class DpiAwarenessResult {
    Enabled,
    AlreadyEnabled,
    Failed,
};

[[nodiscard]] DpiAwarenessResult enablePerMonitorDpiAwarenessV2() noexcept;

}  // namespace lumaspace
