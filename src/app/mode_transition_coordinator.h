#pragma once

#include "app/app_mode.h"

#include <functional>

namespace lumaspace {

enum class ModePlatformStatus {
    Succeeded,
    NotImplemented,
    Rejected,
    Failed,
};

struct ModePlatformResult {
    ModePlatformStatus status = ModePlatformStatus::Succeeded;
};

class IModePlatformAdapter {
public:
    virtual ~IModePlatformAdapter() = default;

    [[nodiscard]] virtual ModePlatformResult preflight(AppMode target) = 0;
    [[nodiscard]] virtual ModePlatformResult enter(AppMode target) = 0;
    [[nodiscard]] virtual ModePlatformResult exit(AppMode mode) = 0;
    virtual void shutdown() noexcept = 0;
};

enum class CoordinatedTransitionStatus {
    Succeeded,
    NoOp,
    Rejected,
    Failed,
};

enum class CoordinatedTransitionReason {
    None,
    ControllerStopped,
    TransitionInProgress,
    TargetNotImplemented,
    PreflightRejected,
    PreflightFailed,
    EnterFailed,
    ExitFailed,
    CommitFailed,
    RollbackFailed,
};

struct CoordinatedTransitionResult {
    AppMode from = AppMode::Disabled;
    AppMode requested = AppMode::Disabled;
    CoordinatedTransitionStatus status = CoordinatedTransitionStatus::NoOp;
    CoordinatedTransitionReason reason = CoordinatedTransitionReason::None;

    [[nodiscard]] bool succeeded() const noexcept
    {
        return status == CoordinatedTransitionStatus::Succeeded ||
               status == CoordinatedTransitionStatus::NoOp;
    }
};

class ModeTransitionCoordinator final {
public:
    using FailureObserver = std::function<void(const CoordinatedTransitionResult&)>;

    ModeTransitionCoordinator(AppModeController& controller,
                              IModePlatformAdapter& platformAdapter,
                              FailureObserver failureObserver = {});

    [[nodiscard]] CoordinatedTransitionResult requestTransition(
        AppMode target, TransitionSource source);
    [[nodiscard]] AppMode currentMode() const noexcept;
    [[nodiscard]] bool isRunning() const noexcept;
    void shutdown() noexcept;

private:
    void completeShutdown() noexcept;
    [[nodiscard]] CoordinatedTransitionResult fail(
        AppMode from, AppMode target, CoordinatedTransitionStatus status,
        CoordinatedTransitionReason reason) const;

    AppModeController& controller_;
    IModePlatformAdapter& platformAdapter_;
    FailureObserver failureObserver_;
    bool running_ = true;
    bool transitionInProgress_ = false;
    bool shutdownCompleted_ = false;
};

class UnsupportedModePlatformAdapter final : public IModePlatformAdapter {
public:
    [[nodiscard]] ModePlatformResult preflight(AppMode target) override;
    [[nodiscard]] ModePlatformResult enter(AppMode target) override;
    [[nodiscard]] ModePlatformResult exit(AppMode mode) override;
    void shutdown() noexcept override;
};

}  // namespace lumaspace
