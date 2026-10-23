#pragma once

#include "app/app_mode.h"

#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <variant>

namespace lumaspace {

class ActionId final {
public:
    explicit ActionId(std::string value);

    [[nodiscard]] const std::string& value() const noexcept;
    [[nodiscard]] bool isValid() const noexcept;

    auto operator<=>(const ActionId&) const = default;

private:
    std::string value_;
};

enum class ActionPayloadType {
    None,
    AppMode,
};

using ActionPayload = std::variant<std::monostate, AppMode>;

enum class ActionHandlerStatus {
    Succeeded,
    Failed,
};

enum class ActionRegistrationStatus {
    Registered,
    DuplicateId,
    InvalidId,
    InvalidMetadata,
    InvalidHandler,
    MutationDuringDispatch,
};

enum class ActionDispatchStatus {
    Succeeded,
    UnknownAction,
    Disabled,
    InvalidPayload,
    HandlerFailed,
    ReentrantDispatch,
};

struct ActionDispatchResult {
    ActionDispatchStatus status;

    [[nodiscard]] bool succeeded() const noexcept
    {
        return status == ActionDispatchStatus::Succeeded;
    }
};

struct ActionDefinition {
    ActionId id;
    std::string label;
    std::string description;
    ActionPayloadType payloadType = ActionPayloadType::None;
    std::function<bool()> isEnabled;
    std::function<ActionHandlerStatus(const ActionPayload&)> handler;
};

class ActionRegistry final {
public:
    [[nodiscard]] ActionRegistrationStatus registerAction(ActionDefinition definition);
    [[nodiscard]] bool unregisterAction(const ActionId& id);
    [[nodiscard]] ActionDispatchResult dispatch(
        const ActionId& id, const ActionPayload& payload = std::monostate{});

    [[nodiscard]] bool contains(const ActionId& id) const;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    [[nodiscard]] static bool payloadMatches(ActionPayloadType expected,
                                             const ActionPayload& payload) noexcept;

    std::map<ActionId, ActionDefinition> actions_;
    bool dispatching_ = false;
};

}  // namespace lumaspace
