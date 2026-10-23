#include "actions/mode_actions.h"

namespace lumaspace {
namespace {

ActionDefinition makeModeAction(const ActionId& id, std::string label,
                                std::string description, AppMode target,
                                AppModeController& controller)
{
    return ActionDefinition{
        id,
        std::move(label),
        std::move(description),
        ActionPayloadType::None,
        [&controller] { return controller.isRunning(); },
        [&controller, target](const ActionPayload&) {
            const TransitionResult result = controller.requestTransition(
                target, TransitionContext{TransitionSource::Action});
            return result.status == TransitionStatus::Succeeded ||
                           result.status == TransitionStatus::NoOp
                       ? ActionHandlerStatus::Succeeded
                       : ActionHandlerStatus::Failed;
        },
    };
}

}  // namespace

bool registerModeActions(ActionRegistry& registry, AppModeController& controller)
{
    const auto disabled = registry.registerAction(makeModeAction(
        kModeDisabledAction, "Disabled", "Switch LumaSpace to Disabled mode.",
        AppMode::Disabled, controller));
    if (disabled != ActionRegistrationStatus::Registered) {
        return false;
    }
    const auto overlay = registry.registerAction(makeModeAction(
        kModeOverlayAction, "Overlay", "Switch LumaSpace to Overlay mode.",
        AppMode::Overlay, controller));
    if (overlay != ActionRegistrationStatus::Registered) {
        static_cast<void>(registry.unregisterAction(kModeDisabledAction));
        return false;
    }

    const auto editing = registry.registerAction(makeModeAction(
        kModeEditingAction, "Editing", "Switch LumaSpace to Editing mode.",
        AppMode::Editing, controller));
    if (editing != ActionRegistrationStatus::Registered) {
        static_cast<void>(registry.unregisterAction(kModeOverlayAction));
        static_cast<void>(registry.unregisterAction(kModeDisabledAction));
        return false;
    }
    return true;
}

}  // namespace lumaspace
