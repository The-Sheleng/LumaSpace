#include "actions/mode_actions.h"

namespace lumaspace {
namespace {

using ModeRequest = std::function<bool(AppMode)>;

ActionDefinition makeModeAction(const ActionId& id, std::string label,
                                std::string description, AppMode target,
                                std::function<bool()> isEnabled, ModeRequest request)
{
    return ActionDefinition{
        id,
        std::move(label),
        std::move(description),
        ActionPayloadType::None,
        std::move(isEnabled),
        [request = std::move(request), target](const ActionPayload&) {
            return request(target) ? ActionHandlerStatus::Succeeded
                                   : ActionHandlerStatus::Failed;
        },
    };
}

}  // namespace

bool registerModeActionsImpl(ActionRegistry& registry, std::function<bool()> isEnabled,
                             ModeRequest request)
{
    const auto disabled = registry.registerAction(makeModeAction(
        kModeDisabledAction, "Disabled", "Switch LumaSpace to Disabled mode.",
        AppMode::Disabled, isEnabled, request));
    if (disabled != ActionRegistrationStatus::Registered) {
        return false;
    }
    const auto overlay = registry.registerAction(makeModeAction(
        kModeOverlayAction, "Overlay", "Switch LumaSpace to Overlay mode.",
        AppMode::Overlay, isEnabled, request));
    if (overlay != ActionRegistrationStatus::Registered) {
        static_cast<void>(registry.unregisterAction(kModeDisabledAction));
        return false;
    }

    const auto editing = registry.registerAction(makeModeAction(
        kModeEditingAction, "Editing", "Switch LumaSpace to Editing mode.",
        AppMode::Editing, std::move(isEnabled), std::move(request)));
    if (editing != ActionRegistrationStatus::Registered) {
        static_cast<void>(registry.unregisterAction(kModeOverlayAction));
        static_cast<void>(registry.unregisterAction(kModeDisabledAction));
        return false;
    }
    return true;
}

bool registerModeActions(ActionRegistry& registry, AppModeController& controller)
{
    return registerModeActionsImpl(
        registry, [&controller] { return controller.isRunning(); },
        [&controller](AppMode target) {
            const TransitionResult result = controller.requestTransition(
                target, TransitionContext{TransitionSource::Action});
            return result.status == TransitionStatus::Succeeded ||
                   result.status == TransitionStatus::NoOp;
        });
}

bool registerModeActions(ActionRegistry& registry,
                         ModeTransitionCoordinator& coordinator)
{
    return registerModeActionsImpl(
        registry, [&coordinator] { return coordinator.isRunning(); },
        [&coordinator](AppMode target) {
            return coordinator.requestTransition(target, TransitionSource::Action)
                .succeeded();
        });
}

}  // namespace lumaspace
