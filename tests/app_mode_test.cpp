#include "app/app_mode.h"

#include <QTest>

#include <array>
#include <vector>

class AppModeTest final : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initialStateAndNames()
    {
        lumaspace::AppModeController controller;
        QCOMPARE(controller.currentMode(), lumaspace::AppMode::Disabled);
        QVERIFY(controller.isRunning());
        QCOMPARE(lumaspace::appModeName(lumaspace::AppMode::Disabled), "Disabled");
        QCOMPARE(lumaspace::appModeName(lumaspace::AppMode::Overlay), "Overlay");
        QCOMPARE(lumaspace::appModeName(lumaspace::AppMode::Editing), "Editing");
    }

    void allCrossStateTransitions()
    {
        using lumaspace::AppMode;
        constexpr std::array pairs{
            std::pair{AppMode::Disabled, AppMode::Overlay},
            std::pair{AppMode::Disabled, AppMode::Editing},
            std::pair{AppMode::Overlay, AppMode::Disabled},
            std::pair{AppMode::Overlay, AppMode::Editing},
            std::pair{AppMode::Editing, AppMode::Disabled},
            std::pair{AppMode::Editing, AppMode::Overlay},
        };

        for (const auto& [from, to] : pairs) {
            lumaspace::AppModeController controller;
            if (from != AppMode::Disabled) {
                QCOMPARE(controller.requestTransition(from).status,
                         lumaspace::TransitionStatus::Succeeded);
            }
            const auto result = controller.requestTransition(to);
            QCOMPARE(result.status, lumaspace::TransitionStatus::Succeeded);
            QCOMPARE(result.from, from);
            QCOMPARE(result.requested, to);
            QVERIFY(result.changed());
            QCOMPARE(controller.currentMode(), to);
        }
    }

    void selfTransitionsAreNoOps()
    {
        using lumaspace::AppMode;
        for (const AppMode mode : {AppMode::Disabled, AppMode::Overlay, AppMode::Editing}) {
            int notifications = 0;
            lumaspace::AppModeController controller(
                [&notifications](const lumaspace::ModeTransition&) { ++notifications; });
            if (mode != AppMode::Disabled) {
                const auto setupResult = controller.requestTransition(mode);
                QCOMPARE(setupResult.status, lumaspace::TransitionStatus::Succeeded);
                notifications = 0;
            }
            const auto result = controller.requestTransition(mode);
            QCOMPARE(result.status, lumaspace::TransitionStatus::NoOp);
            QVERIFY(!result.changed());
            QCOMPARE(notifications, 0);
        }
    }

    void rejectedAndFailedPreconditionsPreserveState()
    {
        int notifications = 0;
        lumaspace::AppModeController controller(
            [&notifications](const lumaspace::ModeTransition&) { ++notifications; });

        const auto rejected = controller.requestTransition(
            lumaspace::AppMode::Overlay, {}, lumaspace::TransitionPrecondition::Rejected);
        QCOMPARE(rejected.status, lumaspace::TransitionStatus::Rejected);
        QCOMPARE(rejected.reason, lumaspace::TransitionReason::PreconditionRejected);

        const auto failed = controller.requestTransition(
            lumaspace::AppMode::Editing, {}, lumaspace::TransitionPrecondition::Failed);
        QCOMPARE(failed.status, lumaspace::TransitionStatus::Failed);
        QCOMPARE(failed.reason, lumaspace::TransitionReason::PreconditionFailed);
        QCOMPARE(controller.currentMode(), lumaspace::AppMode::Disabled);
        QCOMPARE(notifications, 0);
    }

    void successfulChangesNotifyExactlyOnce()
    {
        std::vector<lumaspace::ModeTransition> transitions;
        lumaspace::AppModeController controller(
            [&transitions](const lumaspace::ModeTransition& transition) {
                transitions.push_back(transition);
            });

        const lumaspace::TransitionContext context{lumaspace::TransitionSource::Action};
        const auto result = controller.requestTransition(lumaspace::AppMode::Overlay, context);
        QCOMPARE(result.status, lumaspace::TransitionStatus::Succeeded);
        QCOMPARE(transitions.size(), std::size_t{1});
        QCOMPARE(transitions.front().from, lumaspace::AppMode::Disabled);
        QCOMPARE(transitions.front().to, lumaspace::AppMode::Overlay);
        QCOMPARE(transitions.front().context.source, lumaspace::TransitionSource::Action);
    }

    void shutdownIsOrderlyAndIdempotent()
    {
        lumaspace::AppModeController controller;
        controller.shutdown();
        controller.shutdown();
        QVERIFY(!controller.isRunning());

        const auto result = controller.requestTransition(lumaspace::AppMode::Overlay);
        QCOMPARE(result.status, lumaspace::TransitionStatus::Rejected);
        QCOMPARE(result.reason, lumaspace::TransitionReason::ControllerStopped);
        QCOMPARE(controller.currentMode(), lumaspace::AppMode::Disabled);
    }
};

QTEST_APPLESS_MAIN(AppModeTest)

#include "app_mode_test.moc"
