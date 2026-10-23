#include "actions/action_registry.h"
#include "actions/mode_actions.h"

#include <QTest>

class ActionRegistryTest final : public QObject {
    Q_OBJECT

private:
    static lumaspace::ActionDefinition definition(
        std::string id, std::function<lumaspace::ActionHandlerStatus(
                            const lumaspace::ActionPayload&)> handler,
        lumaspace::ActionPayloadType payloadType = lumaspace::ActionPayloadType::None,
        std::function<bool()> enabled = {})
    {
        return {lumaspace::ActionId{std::move(id)}, "Test action", "Used by tests.",
                payloadType, std::move(enabled), std::move(handler)};
    }

private Q_SLOTS:
    void registrationAndDuplicateIds()
    {
        lumaspace::ActionRegistry registry;
        const auto handler = [](const lumaspace::ActionPayload&) {
            return lumaspace::ActionHandlerStatus::Succeeded;
        };
        QCOMPARE(registry.registerAction(definition("test.action", handler)),
                 lumaspace::ActionRegistrationStatus::Registered);
        QCOMPARE(registry.registerAction(definition("test.action", handler)),
                 lumaspace::ActionRegistrationStatus::DuplicateId);
        QCOMPARE(registry.size(), std::size_t{1});
        QVERIFY(registry.contains(lumaspace::ActionId{"test.action"}));
    }

    void invalidIdsAndMetadataAreRejected()
    {
        lumaspace::ActionRegistry registry;
        const auto handler = [](const lumaspace::ActionPayload&) {
            return lumaspace::ActionHandlerStatus::Succeeded;
        };
        QCOMPARE(registry.registerAction(definition("Bad Action", handler)),
                 lumaspace::ActionRegistrationStatus::InvalidId);
        auto missingLabel = definition("test.no-label", handler);
        missingLabel.label.clear();
        QCOMPARE(registry.registerAction(std::move(missingLabel)),
                 lumaspace::ActionRegistrationStatus::InvalidMetadata);
    }

    void unknownDisabledAndBadPayloadAreExplicit()
    {
        lumaspace::ActionRegistry registry;
        QCOMPARE(registry.dispatch(lumaspace::ActionId{"unknown.action"}).status,
                 lumaspace::ActionDispatchStatus::UnknownAction);

        QVERIFY(registry.registerAction(definition(
                    "test.disabled",
                    [](const lumaspace::ActionPayload&) {
                        return lumaspace::ActionHandlerStatus::Succeeded;
                    },
                    lumaspace::ActionPayloadType::None, [] { return false; })) ==
                lumaspace::ActionRegistrationStatus::Registered);
        QCOMPARE(registry.dispatch(lumaspace::ActionId{"test.disabled"}).status,
                 lumaspace::ActionDispatchStatus::Disabled);

        QVERIFY(registry.registerAction(definition(
                    "test.typed",
                    [](const lumaspace::ActionPayload&) {
                        return lumaspace::ActionHandlerStatus::Succeeded;
                    },
                    lumaspace::ActionPayloadType::AppMode)) ==
                lumaspace::ActionRegistrationStatus::Registered);
        QCOMPARE(registry.dispatch(lumaspace::ActionId{"test.typed"}).status,
                 lumaspace::ActionDispatchStatus::InvalidPayload);
    }

    void handlerDispatchesExactlyOnce()
    {
        lumaspace::ActionRegistry registry;
        int calls = 0;
        QVERIFY(registry.registerAction(definition(
                    "test.once", [&calls](const lumaspace::ActionPayload&) {
                        ++calls;
                        return lumaspace::ActionHandlerStatus::Succeeded;
                    })) == lumaspace::ActionRegistrationStatus::Registered);
        const auto result = registry.dispatch(lumaspace::ActionId{"test.once"});
        QVERIFY(result.succeeded());
        QCOMPARE(calls, 1);
    }

    void throwingHandlerFailsWithoutPoisoningRegistry()
    {
        lumaspace::ActionRegistry registry;
        QVERIFY(registry.registerAction(definition(
                    "test.throws", [](const lumaspace::ActionPayload&) ->
                                       lumaspace::ActionHandlerStatus {
                        throw 42;
                    })) == lumaspace::ActionRegistrationStatus::Registered);
        QCOMPARE(registry.dispatch(lumaspace::ActionId{"test.throws"}).status,
                 lumaspace::ActionDispatchStatus::HandlerFailed);
        QVERIFY(registry.unregisterAction(lumaspace::ActionId{"test.throws"}));
    }

    void reentrantDispatchAndMutationAreRejected()
    {
        lumaspace::ActionRegistry registry;
        lumaspace::ActionDispatchStatus nestedStatus =
            lumaspace::ActionDispatchStatus::Succeeded;
        bool mutationSucceeded = true;
        QVERIFY(registry.registerAction(definition(
                    "test.reentrant", [&registry, &nestedStatus, &mutationSucceeded](
                                          const lumaspace::ActionPayload&) {
                        nestedStatus = registry.dispatch(
                            lumaspace::ActionId{"test.reentrant"}).status;
                        mutationSucceeded = registry.unregisterAction(
                            lumaspace::ActionId{"test.reentrant"});
                        return lumaspace::ActionHandlerStatus::Succeeded;
                    })) == lumaspace::ActionRegistrationStatus::Registered);
        QVERIFY(registry.dispatch(lumaspace::ActionId{"test.reentrant"}).succeeded());
        QCOMPARE(nestedStatus, lumaspace::ActionDispatchStatus::ReentrantDispatch);
        QVERIFY(!mutationSucceeded);
        QVERIFY(registry.contains(lumaspace::ActionId{"test.reentrant"}));
    }

    void modeActionsDriveController()
    {
        lumaspace::AppModeController controller;
        lumaspace::ActionRegistry registry;
        QVERIFY(lumaspace::registerModeActions(registry, controller));
        QCOMPARE(registry.size(), std::size_t{3});

        QVERIFY(registry.dispatch(lumaspace::kModeOverlayAction).succeeded());
        QCOMPARE(controller.currentMode(), lumaspace::AppMode::Overlay);
        QVERIFY(registry.dispatch(lumaspace::kModeEditingAction).succeeded());
        QCOMPARE(controller.currentMode(), lumaspace::AppMode::Editing);
        QVERIFY(registry.dispatch(lumaspace::kModeDisabledAction).succeeded());
        QCOMPARE(controller.currentMode(), lumaspace::AppMode::Disabled);

        controller.shutdown();
        QCOMPARE(registry.dispatch(lumaspace::kModeOverlayAction).status,
                 lumaspace::ActionDispatchStatus::Disabled);
    }
};

QTEST_APPLESS_MAIN(ActionRegistryTest)

#include "action_registry_test.moc"
