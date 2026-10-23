#pragma once

#include "platform/windows/hotkey_types.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace lumaspace {

enum class HotkeyServiceStatus {
    Started,
    AlreadyStarted,
    InvalidBindings,
    WindowCreationFailed,
    RegistrationFailed,
};

struct HotkeyServiceResult {
    HotkeyServiceStatus status = HotkeyServiceStatus::Started;
    HotkeyValidationStatus validation = HotkeyValidationStatus::Valid;
    int failedRegistrationId = 0;
    unsigned long systemError = 0;

    [[nodiscard]] bool succeeded() const noexcept
    {
        return status == HotkeyServiceStatus::Started;
    }
};

class HotkeyService final {
public:
    using ModeProvider = std::function<AppMode()>;

    HotkeyService(ActionRegistry& registry, ModeProvider modeProvider);
    ~HotkeyService();

    HotkeyService(const HotkeyService&) = delete;
    HotkeyService& operator=(const HotkeyService&) = delete;

    [[nodiscard]] HotkeyServiceResult start(std::vector<HotkeyBinding> bindings);
    void stop() noexcept;
    [[nodiscard]] bool isRunning() const noexcept;
    [[nodiscard]] void* nativeWindowHandle() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

[[nodiscard]] std::string describeHotkeyServiceFailure(const HotkeyServiceResult& result,
                                                       const std::vector<HotkeyBinding>& bindings);

}  // namespace lumaspace
