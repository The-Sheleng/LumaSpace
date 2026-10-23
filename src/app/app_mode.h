#pragma once

#include <functional>
#include <string_view>

namespace lumaspace {

enum class AppMode {
    Disabled,
    Overlay,
    Editing,
};

[[nodiscard]] constexpr std::string_view appModeName(AppMode mode) noexcept
{
    switch (mode) {
    case AppMode::Disabled:
        return "Disabled";
    case AppMode::Overlay:
        return "Overlay";
    case AppMode::Editing:
        return "Editing";
    }
    return "Unknown";
}

enum class TransitionSource {
    Lifecycle,
    Action,
    Hotkey,
    Test,
};

struct TransitionContext {
    TransitionSource source = TransitionSource::Lifecycle;
};

enum class TransitionPrecondition {
    Satisfied,
    Rejected,
    Failed,
};

enum class TransitionStatus {
    Succeeded,
    NoOp,
    Rejected,
    Failed,
};

enum class TransitionReason {
    None,
    PreconditionRejected,
    PreconditionFailed,
    InvalidTarget,
    ControllerStopped,
    TransitionInProgress,
};

struct TransitionResult {
    AppMode from = AppMode::Disabled;
    AppMode requested = AppMode::Disabled;
    TransitionStatus status = TransitionStatus::NoOp;
    TransitionReason reason = TransitionReason::None;

    [[nodiscard]] bool changed() const noexcept
    {
        return status == TransitionStatus::Succeeded;
    }
};

struct ModeTransition {
    AppMode from;
    AppMode to;
    TransitionContext context;
};

class AppModeController final {
public:
    using Observer = std::function<void(const ModeTransition&)>;

    explicit AppModeController(Observer observer = {});

    AppModeController(const AppModeController&) = delete;
    AppModeController& operator=(const AppModeController&) = delete;

    [[nodiscard]] AppMode currentMode() const noexcept;
    [[nodiscard]] bool isRunning() const noexcept;

    [[nodiscard]] TransitionResult requestTransition(
        AppMode target, TransitionContext context = {},
        TransitionPrecondition precondition = TransitionPrecondition::Satisfied);

    void shutdown() noexcept;

private:
    [[nodiscard]] static bool isValidMode(AppMode mode) noexcept;

    AppMode currentMode_ = AppMode::Disabled;
    Observer observer_;
    bool running_ = true;
    bool transitionInProgress_ = false;
};

}  // namespace lumaspace
