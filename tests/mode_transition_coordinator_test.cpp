#include "app/mode_transition_coordinator.h"

#include <QTest>

#include <string>
#include <vector>

class FakeModePlatformAdapter final : public lumaspace::IModePlatformAdapter {
public:
    lumaspace::ModePlatformResult preflightResult{};
    lumaspace::ModePlatformResult enterResult{};
    lumaspace::ModePlatformResult exitResult{};
    lumaspace::ModePlatformResult rollbackExitResult{};
    bool useRollbackExitResult = false;
    bool shutdownCalled = false;
    std::function<void()> onPreflight;
    std::vector<std::string> calls;

    lumaspace::ModePlatformResult preflight(lumaspace::AppMode target) override
    {
        calls.push_back("preflight:" + std::string(lumaspace::appModeName(target)));
        if (onPreflight) {
            onPreflight();
        }
        return preflightResult;
    }

    lumaspace::ModePlatformResult enter(lumaspace::AppMode target) override
    {
        calls.push_back("enter:" + std::string(lumaspace::appModeName(target)));
        return enterResult;
    }

    lumaspace::ModePlatformResult exit(lumaspace::AppMode mode) override
    {
        calls.push_back("exit:" + std::string(lumaspace::appModeName(mode)));
        if (useRollbackExitResult && calls.size() >= 4) {
            return rollbackExitResult;
        }
        return exitResult;
    }

    void shutdown() noexcept override
    {
        shutdownCalled = true;
        calls.push_back("shutdown");
    }
};

class ModeTransitionCoordinatorTest final : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void allCrossStateTransitionsCommitInOrder()
    {
        using lumaspace::AppMode;
        for (const auto& [from, to] : std::vector<std::pair<AppMode, AppMode>>{
                 {AppMode::Disabled, AppMode::Overlay},
                 {AppMode::Disabled, AppMode::Editing},
                 {AppMode::Overlay, AppMode::Disabled},
                 {AppMode::Overlay, AppMode::Editing},
                 {AppMode::Editing, AppMode::Disabled},
                 {AppMode::Editing, AppMode::Overlay}}) {
            lumaspace::AppModeController controller;
            if (from != AppMode::Disabled) {
                QCOMPARE(controller.requestTransition(from).status,
                         lumaspace::TransitionStatus::Succeeded);
            }
            FakeModePlatformAdapter adapter;
            lumaspace::ModeTransitionCoordinator coordinator(controller, adapter);
            const auto result = coordinator.requestTransition(
                to, lumaspace::TransitionSource::Test);
            QVERIFY(result.succeeded());
            QCOMPARE(controller.currentMode(), to);
            QCOMPARE(adapter.calls.size(), std::size_t{3});
            QCOMPARE(adapter.calls[0], "preflight:" + std::string(lumaspace::appModeName(to)));
            QCOMPARE(adapter.calls[1], "enter:" + std::string(lumaspace::appModeName(to)));
            QCOMPARE(adapter.calls[2], "exit:" + std::string(lumaspace::appModeName(from)));
        }
    }

    void successfulTransitionEmitsExactlyOneNotification()
    {
        int notifications = 0;
        lumaspace::AppModeController controller(
            [&notifications](const lumaspace::ModeTransition&) { ++notifications; });
        FakeModePlatformAdapter adapter;
        lumaspace::ModeTransitionCoordinator coordinator(controller, adapter);

        QVERIFY(coordinator
                    .requestTransition(lumaspace::AppMode::Overlay,
                                       lumaspace::TransitionSource::Test)
                    .succeeded());
        QCOMPARE(notifications, 1);
        QCOMPARE(coordinator
                     .requestTransition(lumaspace::AppMode::Overlay,
                                        lumaspace::TransitionSource::Test)
                     .status,
                 lumaspace::CoordinatedTransitionStatus::NoOp);
        QCOMPARE(notifications, 1);
    }

    void unsupportedProductionModeIsRejectedWithoutCommit()
    {
        lumaspace::AppModeController controller;
        lumaspace::UnsupportedModePlatformAdapter adapter;
        int failures = 0;
        lumaspace::ModeTransitionCoordinator coordinator(
            controller, adapter,
            [&failures](const lumaspace::CoordinatedTransitionResult&) { ++failures; });
        const auto result = coordinator.requestTransition(
            lumaspace::AppMode::Overlay, lumaspace::TransitionSource::Hotkey);
        QCOMPARE(result.status, lumaspace::CoordinatedTransitionStatus::Rejected);
        QCOMPARE(result.reason,
                 lumaspace::CoordinatedTransitionReason::TargetNotImplemented);
        QCOMPARE(controller.currentMode(), lumaspace::AppMode::Disabled);
        QCOMPARE(failures, 1);
    }

    void enterFailurePreservesPriorMode()
    {
        lumaspace::AppModeController controller;
        FakeModePlatformAdapter adapter;
        adapter.enterResult = {lumaspace::ModePlatformStatus::Failed};
        lumaspace::ModeTransitionCoordinator coordinator(controller, adapter);
        const auto result = coordinator.requestTransition(
            lumaspace::AppMode::Editing, lumaspace::TransitionSource::Test);
        QCOMPARE(result.reason, lumaspace::CoordinatedTransitionReason::EnterFailed);
        QCOMPARE(controller.currentMode(), lumaspace::AppMode::Disabled);
        QCOMPARE(adapter.calls.size(), std::size_t{2});
    }

    void priorExitFailureReleasesAcquiredTarget()
    {
        lumaspace::AppModeController controller;
        QCOMPARE(controller.requestTransition(lumaspace::AppMode::Overlay).status,
                 lumaspace::TransitionStatus::Succeeded);
        FakeModePlatformAdapter adapter;
        adapter.exitResult = {lumaspace::ModePlatformStatus::Failed};
        adapter.rollbackExitResult = {lumaspace::ModePlatformStatus::Succeeded};
        adapter.useRollbackExitResult = true;
        lumaspace::ModeTransitionCoordinator coordinator(controller, adapter);
        const auto result = coordinator.requestTransition(
            lumaspace::AppMode::Editing, lumaspace::TransitionSource::Test);
        QCOMPARE(result.reason, lumaspace::CoordinatedTransitionReason::ExitFailed);
        QCOMPARE(controller.currentMode(), lumaspace::AppMode::Overlay);
        QCOMPARE(adapter.calls.back(), std::string{"exit:Editing"});
    }

    void repeatedRequestIsNoOpWithoutPlatformCalls()
    {
        lumaspace::AppModeController controller;
        FakeModePlatformAdapter adapter;
        lumaspace::ModeTransitionCoordinator coordinator(controller, adapter);
        const auto result = coordinator.requestTransition(
            lumaspace::AppMode::Disabled, lumaspace::TransitionSource::Test);
        QCOMPARE(result.status, lumaspace::CoordinatedTransitionStatus::NoOp);
        QVERIFY(adapter.calls.empty());
    }

    void shutdownReleasesResourcesAndRejectsFurtherRequests()
    {
        lumaspace::AppModeController controller;
        QCOMPARE(controller.requestTransition(lumaspace::AppMode::Overlay).status,
                 lumaspace::TransitionStatus::Succeeded);
        FakeModePlatformAdapter adapter;
        lumaspace::ModeTransitionCoordinator coordinator(controller, adapter);
        coordinator.shutdown();
        coordinator.shutdown();
        QVERIFY(adapter.shutdownCalled);
        QVERIFY(!coordinator.isRunning());
        QCOMPARE(adapter.calls[0], std::string{"exit:Overlay"});
        QCOMPARE(adapter.calls[1], std::string{"shutdown"});
        const auto result = coordinator.requestTransition(
            lumaspace::AppMode::Disabled, lumaspace::TransitionSource::Test);
        QCOMPARE(result.reason,
                 lumaspace::CoordinatedTransitionReason::ControllerStopped);
    }

    void shutdownDuringTransitionStopsBeforeResourceAcquisition()
    {
        lumaspace::AppModeController controller;
        FakeModePlatformAdapter adapter;
        lumaspace::ModeTransitionCoordinator coordinator(controller, adapter);
        adapter.onPreflight = [&coordinator] { coordinator.shutdown(); };

        const auto result = coordinator.requestTransition(
            lumaspace::AppMode::Overlay, lumaspace::TransitionSource::Test);

        QCOMPARE(result.reason,
                 lumaspace::CoordinatedTransitionReason::ControllerStopped);
        QCOMPARE(controller.currentMode(), lumaspace::AppMode::Disabled);
        QCOMPARE(adapter.calls.size(), std::size_t{2});
        QCOMPARE(adapter.calls[0], std::string{"preflight:Overlay"});
        QCOMPARE(adapter.calls[1], std::string{"shutdown"});
        QVERIFY(adapter.shutdownCalled);
        QVERIFY(!coordinator.isRunning());
    }
};

QTEST_APPLESS_MAIN(ModeTransitionCoordinatorTest)

#include "mode_transition_coordinator_test.moc"
