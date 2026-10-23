#include "actions/mode_actions.h"
#include "platform/windows/hotkey_service.h"

#include <QTest>

#include <windows.h>

class HotkeyTest final : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void validationRejectsInvalidAndUnsupportedBindings()
    {
        auto bindings = lumaspace::defaultModeHotkeys();
        QCOMPARE(lumaspace::validateHotkeyBindings(bindings),
                 lumaspace::HotkeyValidationStatus::Valid);

        bindings.front().triggerMode = lumaspace::TriggerMode::Hold;
        QCOMPARE(lumaspace::validateHotkeyBindings(bindings),
                 lumaspace::HotkeyValidationStatus::UnsupportedTrigger);
        bindings = lumaspace::defaultModeHotkeys();
        bindings[1].registrationId = bindings[0].registrationId;
        QCOMPARE(lumaspace::validateHotkeyBindings(bindings),
                 lumaspace::HotkeyValidationStatus::DuplicateId);
        bindings = lumaspace::defaultModeHotkeys();
        bindings[1].combination = bindings[0].combination;
        QCOMPARE(lumaspace::validateHotkeyBindings(bindings),
                 lumaspace::HotkeyValidationStatus::DuplicateCombination);
    }

    void activationPoliciesAreIndependentOfTriggerMode()
    {
        using enum lumaspace::AppMode;
        QVERIFY(!lumaspace::isActionEligible(lumaspace::ActivationPolicy::EditingOnly,
                                             Overlay));
        QVERIFY(lumaspace::isActionEligible(lumaspace::ActivationPolicy::EditingOnly,
                                            Editing));
        QVERIFY(!lumaspace::isActionEligible(
            lumaspace::ActivationPolicy::OverlayAndEditing, Disabled));
        QVERIFY(lumaspace::isActionEligible(
            lumaspace::ActivationPolicy::OverlayAndEditing, Overlay));
        QVERIFY(lumaspace::isActionEligible(lumaspace::ActivationPolicy::Always,
                                            Disabled));
    }

    void displayNamesAreActionable()
    {
        const auto bindings = lumaspace::defaultModeHotkeys();
        QCOMPARE(lumaspace::hotkeyDisplayName(bindings[0].combination),
                 std::string{"Ctrl+Alt+Shift+F9"});
    }

    void registerDispatchAndUnregisterIntegration()
    {
        lumaspace::AppModeController controller;
        lumaspace::ActionRegistry registry;
        QVERIFY(lumaspace::registerModeActions(registry, controller));
        lumaspace::HotkeyService service(
            registry, [&controller] { return controller.currentMode(); });

        constexpr lumaspace::HotkeyModifier modifiers =
            lumaspace::HotkeyModifier::Control | lumaspace::HotkeyModifier::Alt |
            lumaspace::HotkeyModifier::Shift;
        const std::vector bindings{
            lumaspace::HotkeyBinding{91, {modifiers, VK_F24},
                                     lumaspace::kModeOverlayAction,
                                     lumaspace::TriggerMode::Press,
                                     lumaspace::ActivationPolicy::Always},
        };
        const auto result = service.start(bindings);
        QVERIFY2(result.succeeded(),
                 lumaspace::describeHotkeyServiceFailure(result, bindings).c_str());
        QVERIFY(service.isRunning());
        QVERIFY(service.nativeWindowHandle() != nullptr);

        SendMessageW(static_cast<HWND>(service.nativeWindowHandle()), WM_HOTKEY, 91, 0);
        QCOMPARE(controller.currentMode(), lumaspace::AppMode::Overlay);
        service.stop();
        QVERIFY(!service.isRunning());

        QVERIFY(RegisterHotKey(nullptr, 91, MOD_CONTROL | MOD_ALT | MOD_SHIFT | MOD_NOREPEAT,
                               VK_F24));
        QVERIFY(UnregisterHotKey(nullptr, 91));
    }

    void registrationConflictIsDiagnosableAndRollsBack()
    {
        lumaspace::AppModeController controller;
        lumaspace::ActionRegistry registry;
        QVERIFY(lumaspace::registerModeActions(registry, controller));
        lumaspace::HotkeyService owner(
            registry, [&controller] { return controller.currentMode(); });
        lumaspace::HotkeyService contender(
            registry, [&controller] { return controller.currentMode(); });
        constexpr lumaspace::HotkeyModifier modifiers =
            lumaspace::HotkeyModifier::Control | lumaspace::HotkeyModifier::Alt |
            lumaspace::HotkeyModifier::Shift;
        const std::vector bindings{
            lumaspace::HotkeyBinding{92, {modifiers, VK_F23},
                                     lumaspace::kModeOverlayAction,
                                     lumaspace::TriggerMode::Press,
                                     lumaspace::ActivationPolicy::Always},
        };

        QVERIFY(owner.start(bindings).succeeded());
        const auto conflict = contender.start(bindings);
        QCOMPARE(conflict.status, lumaspace::HotkeyServiceStatus::RegistrationFailed);
        QVERIFY(!contender.isRunning());
        const std::string diagnostic =
            lumaspace::describeHotkeyServiceFailure(conflict, bindings);
        QVERIFY(diagnostic.find("Ctrl+Alt+Shift+F23") != std::string::npos);
        QVERIFY(diagnostic.find("already in use") != std::string::npos);

        owner.stop();
        QVERIFY(contender.start(bindings).succeeded());
        contender.stop();
    }
};

QTEST_APPLESS_MAIN(HotkeyTest)

#include "hotkey_test.moc"
