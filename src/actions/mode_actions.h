#pragma once

#include "actions/action_registry.h"
#include "app/mode_transition_coordinator.h"

namespace lumaspace {

inline const ActionId kModeDisabledAction{"app.mode.disabled"};
inline const ActionId kModeOverlayAction{"app.mode.overlay"};
inline const ActionId kModeEditingAction{"app.mode.editing"};

[[nodiscard]] bool registerModeActions(ActionRegistry& registry,
                                       AppModeController& controller);
[[nodiscard]] bool registerModeActions(ActionRegistry& registry,
                                       ModeTransitionCoordinator& coordinator);

}  // namespace lumaspace
