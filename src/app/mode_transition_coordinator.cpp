#include "app/mode_transition_coordinator.h"

#include <utility>

namespace lumaspace {
namespace {

CoordinatedTransitionReason preflightReason(ModePlatformStatus status) noexcept
{
    switch (status) {
    case ModePlatformStatus::NotImplemented:
        return CoordinatedTransitionReason::TargetNotImplemented;
    case ModePlatformStatus::Rejected:
        return CoordinatedTransitionReason::PreflightRejected;
    case ModePlatformStatus::Failed:
        return CoordinatedTransitionReason::PreflightFailed;
    case ModePlatformStatus::Succeeded:
        return CoordinatedTransitionReason::None;
    }
    return CoordinatedTransitionReason::PreflightFailed;
}

}  // namespace

ModeTransitionCoordinator::ModeTransitionCoordinator(
    AppModeController& controller, IModePlatformAdapter& platformAdapter,
    FailureObserver failureObserver)
    : controller_(controller),
      platformAdapter_(platformAdapter),
      failureObserver_(std::move(failureObserver))
{
}

CoordinatedTransitionResult ModeTransitionCoordinator::requestTransition(
    AppMode target, TransitionSource source)
{
    const AppMode previous = controller_.currentMode();
    if (!running_ || !controller_.isRunning()) {
        return fail(previous, target, CoordinatedTransitionStatus::Rejected,
                    CoordinatedTransitionReason::ControllerStopped);
    }
    if (transitionInProgress_) {
        return fail(previous, target, CoordinatedTransitionStatus::Rejected,
                    CoordinatedTransitionReason::TransitionInProgress);
    }
    if (target == previous) {
        return {previous, target, CoordinatedTransitionStatus::NoOp,
                CoordinatedTransitionReason::None};
    }

    transitionInProgress_ = true;
    const ModePlatformResult preflight = platformAdapter_.preflight(target);
    if (!running_) {
        transitionInProgress_ = false;
        completeShutdown();
        return {previous, target, CoordinatedTransitionStatus::Rejected,
                CoordinatedTransitionReason::ControllerStopped};
    }
    if (preflight.status != ModePlatformStatus::Succeeded) {
        transitionInProgress_ = false;
        const CoordinatedTransitionReason reason = preflightReason(preflight.status);
        return fail(previous, target,
                    preflight.status == ModePlatformStatus::Failed
                        ? CoordinatedTransitionStatus::Failed
                        : CoordinatedTransitionStatus::Rejected,
                    reason);
    }

    if (platformAdapter_.enter(target).status != ModePlatformStatus::Succeeded) {
        transitionInProgress_ = false;
        return fail(previous, target, CoordinatedTransitionStatus::Failed,
                    CoordinatedTransitionReason::EnterFailed);
    }
    if (!running_) {
        static_cast<void>(platformAdapter_.exit(target));
        transitionInProgress_ = false;
        completeShutdown();
        return {previous, target, CoordinatedTransitionStatus::Rejected,
                CoordinatedTransitionReason::ControllerStopped};
    }

    if (platformAdapter_.exit(previous).status != ModePlatformStatus::Succeeded) {
        const bool rolledBack =
            platformAdapter_.exit(target).status == ModePlatformStatus::Succeeded;
        transitionInProgress_ = false;
        return fail(previous, target, CoordinatedTransitionStatus::Failed,
                    rolledBack ? CoordinatedTransitionReason::ExitFailed
                               : CoordinatedTransitionReason::RollbackFailed);
    }
    if (!running_) {
        static_cast<void>(platformAdapter_.exit(target));
        transitionInProgress_ = false;
        completeShutdown();
        return {previous, target, CoordinatedTransitionStatus::Rejected,
                CoordinatedTransitionReason::ControllerStopped};
    }

    const TransitionResult commit = controller_.requestTransition(
        target, TransitionContext{source}, TransitionPrecondition::Satisfied);
    if (commit.status != TransitionStatus::Succeeded) {
        const bool targetReleased =
            platformAdapter_.exit(target).status == ModePlatformStatus::Succeeded;
        const bool previousRestored =
            platformAdapter_.enter(previous).status == ModePlatformStatus::Succeeded;
        transitionInProgress_ = false;
        return fail(previous, target, CoordinatedTransitionStatus::Failed,
                    targetReleased && previousRestored
                        ? CoordinatedTransitionReason::CommitFailed
                        : CoordinatedTransitionReason::RollbackFailed);
    }

    transitionInProgress_ = false;
    return {previous, target, CoordinatedTransitionStatus::Succeeded,
            CoordinatedTransitionReason::None};
}

AppMode ModeTransitionCoordinator::currentMode() const noexcept
{
    return controller_.currentMode();
}

bool ModeTransitionCoordinator::isRunning() const noexcept
{
    return running_ && controller_.isRunning();
}

void ModeTransitionCoordinator::shutdown() noexcept
{
    if (!running_) {
        return;
    }
    running_ = false;
    if (transitionInProgress_) {
        return;
    }
    completeShutdown();
}

void ModeTransitionCoordinator::completeShutdown() noexcept
{
    if (shutdownCompleted_) {
        return;
    }
    shutdownCompleted_ = true;
    const AppMode current = controller_.currentMode();
    if (current != AppMode::Disabled) {
        static_cast<void>(platformAdapter_.exit(current));
    }
    platformAdapter_.shutdown();
    controller_.shutdown();
    failureObserver_ = {};
}

CoordinatedTransitionResult ModeTransitionCoordinator::fail(
    AppMode from, AppMode target, CoordinatedTransitionStatus status,
    CoordinatedTransitionReason reason) const
{
    const CoordinatedTransitionResult result{from, target, status, reason};
    if (failureObserver_) {
        failureObserver_(result);
    }
    return result;
}

ModePlatformResult UnsupportedModePlatformAdapter::preflight(AppMode target)
{
    return {target == AppMode::Disabled ? ModePlatformStatus::Succeeded
                                        : ModePlatformStatus::NotImplemented};
}

ModePlatformResult UnsupportedModePlatformAdapter::enter(AppMode target)
{
    return {target == AppMode::Disabled ? ModePlatformStatus::Succeeded
                                        : ModePlatformStatus::NotImplemented};
}

ModePlatformResult UnsupportedModePlatformAdapter::exit(AppMode)
{
    return {ModePlatformStatus::Succeeded};
}

void UnsupportedModePlatformAdapter::shutdown() noexcept
{
}

}  // namespace lumaspace
