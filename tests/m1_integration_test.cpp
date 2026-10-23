#include "actions/mode_actions.h"

#include <QTest>

#include <vector>

class IntegrationPlatformAdapter final : public lumaspace::IModePlatformAdapter {
public:
    lumaspace::AppMode rejectedMode = static_cast<lumaspace::AppMode>(-1);
    std::vector<lumaspace::AppMode> entered;
    std::vector<lumaspace::AppMode> exited;

    lumaspace::ModePlatformResult preflight(lumaspace::AppMode target) override
    {
        return {target == rejectedMode ? lumaspace::ModePlatformStatus::Rejected
                                       : lumaspace::ModePlatformStatus::Succeeded};
    }

    lumaspace::ModePlatformResult enter(lumaspace::AppMode target) override
    {
        entered.push_back(target);
        return {};
    }

    lumaspace::ModePlatformResult exit(lumaspace::AppMode mode) override
    {
        exited.push_back(mode);
        return {};
    }

    void shutdown() noexcept override {}
};

class M1IntegrationTest final : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void completeActionTransitionTable()
    {
        using lumaspace::AppMode;
        lumaspace::AppModeController controller;
        IntegrationPlatformAdapter platform;
        lumaspace::ModeTransitionCoordinator coordinator(controller, platform);
        lumaspace::ActionRegistry registry;
        QVERIFY(lumaspace::registerModeActions(registry, coordinator));

        const std::vector transitions{
            std::pair{lumaspace::kModeOverlayAction, AppMode::Overlay},
            std::pair{lumaspace::kModeEditingAction, AppMode::Editing},
            std::pair{lumaspace::kModeDisabledAction, AppMode::Disabled},
            std::pair{lumaspace::kModeEditingAction, AppMode::Editing},
            std::pair{lumaspace::kModeOverlayAction, AppMode::Overlay},
            std::pair{lumaspace::kModeDisabledAction, AppMode::Disabled},
        };
        for (const auto& [action, expected] : transitions) {
            QCOMPARE(registry.dispatch(action).status,
                     lumaspace::ActionDispatchStatus::Succeeded);
            QCOMPARE(coordinator.currentMode(), expected);
        }
        QCOMPARE(platform.entered.size(), transitions.size());
        QCOMPARE(platform.exited.size(), transitions.size());
    }

    void noOpAndRejectedActionPreserveConfirmedMode()
    {
        lumaspace::AppModeController controller;
        IntegrationPlatformAdapter platform;
        lumaspace::ModeTransitionCoordinator coordinator(controller, platform);
        lumaspace::ActionRegistry registry;
        QVERIFY(lumaspace::registerModeActions(registry, coordinator));

        QCOMPARE(registry.dispatch(lumaspace::kModeDisabledAction).status,
                 lumaspace::ActionDispatchStatus::Succeeded);
        QVERIFY(platform.entered.empty());
        platform.rejectedMode = lumaspace::AppMode::Editing;
        QCOMPARE(registry.dispatch(lumaspace::kModeEditingAction).status,
                 lumaspace::ActionDispatchStatus::HandlerFailed);
        QCOMPARE(coordinator.currentMode(), lumaspace::AppMode::Disabled);
        QVERIFY(platform.entered.empty());
    }

    void stoppedCoordinatorDisablesModeActions()
    {
        lumaspace::AppModeController controller;
        IntegrationPlatformAdapter platform;
        lumaspace::ModeTransitionCoordinator coordinator(controller, platform);
        lumaspace::ActionRegistry registry;
        QVERIFY(lumaspace::registerModeActions(registry, coordinator));
        coordinator.shutdown();
        QCOMPARE(registry.dispatch(lumaspace::kModeOverlayAction).status,
                 lumaspace::ActionDispatchStatus::Disabled);
    }
};

QTEST_APPLESS_MAIN(M1IntegrationTest)

#include "m1_integration_test.moc"
