#include "platform/windows/hotkey_types.h"

#include "actions/mode_actions.h"

#include <set>

namespace lumaspace {

HotkeyValidationStatus validateHotkeyBindings(
    const std::vector<HotkeyBinding>& bindings) noexcept
{
    std::set<int> ids;
    std::set<HotkeyCombination> combinations;
    for (const HotkeyBinding& binding : bindings) {
        if (binding.registrationId <= 0) {
            return HotkeyValidationStatus::InvalidId;
        }
        if (binding.combination.virtualKey == 0 || binding.combination.virtualKey > 0xFEU) {
            return HotkeyValidationStatus::InvalidKey;
        }
        if (binding.combination.modifiers == HotkeyModifier::None) {
            return HotkeyValidationStatus::MissingModifier;
        }
        if (binding.triggerMode != TriggerMode::Press) {
            return HotkeyValidationStatus::UnsupportedTrigger;
        }
        if (!ids.insert(binding.registrationId).second) {
            return HotkeyValidationStatus::DuplicateId;
        }
        if (!combinations.insert(binding.combination).second) {
            return HotkeyValidationStatus::DuplicateCombination;
        }
    }
    return HotkeyValidationStatus::Valid;
}

bool isActionEligible(ActivationPolicy policy, AppMode mode) noexcept
{
    switch (policy) {
    case ActivationPolicy::EditingOnly:
        return mode == AppMode::Editing;
    case ActivationPolicy::OverlayAndEditing:
        return mode == AppMode::Overlay || mode == AppMode::Editing;
    case ActivationPolicy::Always:
        return true;
    }
    return false;
}

std::vector<HotkeyBinding> defaultModeHotkeys()
{
    constexpr HotkeyModifier modifiers =
        HotkeyModifier::Control | HotkeyModifier::Alt | HotkeyModifier::Shift;
    return {
        {1, {modifiers, 0x78U}, kModeDisabledAction, TriggerMode::Press,
         ActivationPolicy::Always},  // F9
        {2, {modifiers, 0x79U}, kModeOverlayAction, TriggerMode::Press,
         ActivationPolicy::Always},  // F10
        {3, {modifiers, 0x7AU}, kModeEditingAction, TriggerMode::Press,
         ActivationPolicy::Always},  // F11
    };
}

std::string hotkeyDisplayName(const HotkeyCombination& combination)
{
    std::string result;
    const auto append = [&result](std::string_view value) {
        if (!result.empty()) {
            result += '+';
        }
        result += value;
    };
    if (hasModifier(combination.modifiers, HotkeyModifier::Control)) {
        append("Ctrl");
    }
    if (hasModifier(combination.modifiers, HotkeyModifier::Alt)) {
        append("Alt");
    }
    if (hasModifier(combination.modifiers, HotkeyModifier::Shift)) {
        append("Shift");
    }
    if (hasModifier(combination.modifiers, HotkeyModifier::Windows)) {
        append("Win");
    }
    if (combination.virtualKey >= 0x70U && combination.virtualKey <= 0x87U) {
        append("F" + std::to_string(combination.virtualKey - 0x6FU));
    } else {
        append("VK_" + std::to_string(combination.virtualKey));
    }
    return result;
}

}  // namespace lumaspace
