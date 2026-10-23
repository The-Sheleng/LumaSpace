#include "app/app_mode.h"

#include <utility>

namespace lumaspace {

AppModeController::AppModeController(Observer observer)
    : observer_(std::move(observer))
{
}

AppMode AppModeController::currentMode() const noexcept
{
    return currentMode_;
}

bool AppModeController::isRunning() const noexcept
{
    return running_;
}

TransitionResult AppModeController::requestTransition(AppMode target,
                                                      TransitionContext context,
                                                      TransitionPrecondition precondition)
{
    const AppMode previous = currentMode_;
    if (!running_) {
        return {previous, target, TransitionStatus::Rejected,
                TransitionReason::ControllerStopped};
    }
    if (!isValidMode(target)) {
        return {previous, target, TransitionStatus::Rejected,
                TransitionReason::InvalidTarget};
    }
    if (transitionInProgress_) {
        return {previous, target, TransitionStatus::Rejected,
                TransitionReason::TransitionInProgress};
    }
    if (target == previous) {
        return {previous, target, TransitionStatus::NoOp, TransitionReason::None};
    }
    if (precondition == TransitionPrecondition::Rejected) {
        return {previous, target, TransitionStatus::Rejected,
                TransitionReason::PreconditionRejected};
    }
    if (precondition == TransitionPrecondition::Failed) {
        return {previous, target, TransitionStatus::Failed,
                TransitionReason::PreconditionFailed};
    }

    transitionInProgress_ = true;
    currentMode_ = target;
    if (observer_) {
        observer_(ModeTransition{previous, target, context});
    }
    transitionInProgress_ = false;
    return {previous, target, TransitionStatus::Succeeded, TransitionReason::None};
}

void AppModeController::shutdown() noexcept
{
    running_ = false;
    observer_ = {};
}

bool AppModeController::isValidMode(AppMode mode) noexcept
{
    switch (mode) {
    case AppMode::Disabled:
    case AppMode::Overlay:
    case AppMode::Editing:
        return true;
    }
    return false;
}

}  // namespace lumaspace
