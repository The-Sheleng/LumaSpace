#pragma once

#include "actions/action_registry.h"

#include <cstdint>
#include <string>
#include <vector>

namespace lumaspace {

enum class TriggerMode {
    Press,
    Toggle,
    Hold,
};

enum class ActivationPolicy {
    EditingOnly,
    OverlayAndEditing,
    Always,
};

enum class HotkeyModifier : std::uint32_t {
    None = 0,
    Alt = 1U << 0U,
    Control = 1U << 1U,
    Shift = 1U << 2U,
    Windows = 1U << 3U,
};

constexpr HotkeyModifier operator|(HotkeyModifier left, HotkeyModifier right) noexcept
{
    return static_cast<HotkeyModifier>(static_cast<std::uint32_t>(left) |
                                       static_cast<std::uint32_t>(right));
}

constexpr bool hasModifier(HotkeyModifier value, HotkeyModifier modifier) noexcept
{
    return (static_cast<std::uint32_t>(value) & static_cast<std::uint32_t>(modifier)) != 0;
}

struct HotkeyCombination {
    HotkeyModifier modifiers = HotkeyModifier::None;
    std::uint32_t virtualKey = 0;

    auto operator<=>(const HotkeyCombination&) const = default;
};

struct HotkeyBinding {
    int registrationId = 0;
    HotkeyCombination combination;
    ActionId actionId{"invalid"};
    TriggerMode triggerMode = TriggerMode::Press;
    ActivationPolicy activationPolicy = ActivationPolicy::Always;
};

enum class HotkeyValidationStatus {
    Valid,
    InvalidId,
    InvalidKey,
    MissingModifier,
    UnsupportedTrigger,
    DuplicateId,
    DuplicateCombination,
};

[[nodiscard]] HotkeyValidationStatus validateHotkeyBindings(
    const std::vector<HotkeyBinding>& bindings) noexcept;
[[nodiscard]] bool isActionEligible(ActivationPolicy policy, AppMode mode) noexcept;
[[nodiscard]] std::vector<HotkeyBinding> defaultModeHotkeys();
[[nodiscard]] std::string hotkeyDisplayName(const HotkeyCombination& combination);

}  // namespace lumaspace
