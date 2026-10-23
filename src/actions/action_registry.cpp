#include "actions/action_registry.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace lumaspace {

ActionId::ActionId(std::string value)
    : value_(std::move(value))
{
}

const std::string& ActionId::value() const noexcept
{
    return value_;
}

bool ActionId::isValid() const noexcept
{
    if (value_.empty() || value_.front() == '.' || value_.back() == '.') {
        return false;
    }

    bool previousWasDot = false;
    for (const unsigned char character : value_) {
        const bool isDot = character == '.';
        const bool isAllowed = isDot || character == '-' || character == '_' ||
                               std::islower(character) != 0 || std::isdigit(character) != 0;
        if (!isAllowed || (isDot && previousWasDot)) {
            return false;
        }
        previousWasDot = isDot;
    }
    return true;
}

ActionRegistrationStatus ActionRegistry::registerAction(ActionDefinition definition)
{
    if (dispatching_) {
        return ActionRegistrationStatus::MutationDuringDispatch;
    }
    if (!definition.id.isValid()) {
        return ActionRegistrationStatus::InvalidId;
    }
    if (definition.label.empty() || definition.description.empty()) {
        return ActionRegistrationStatus::InvalidMetadata;
    }
    if (!definition.handler) {
        return ActionRegistrationStatus::InvalidHandler;
    }
    if (actions_.contains(definition.id)) {
        return ActionRegistrationStatus::DuplicateId;
    }

    actions_.emplace(definition.id, std::move(definition));
    return ActionRegistrationStatus::Registered;
}

bool ActionRegistry::unregisterAction(const ActionId& id)
{
    if (dispatching_) {
        return false;
    }
    return actions_.erase(id) == 1;
}

ActionDispatchResult ActionRegistry::dispatch(const ActionId& id, const ActionPayload& payload)
{
    if (dispatching_) {
        return {ActionDispatchStatus::ReentrantDispatch};
    }

    const auto iterator = actions_.find(id);
    if (iterator == actions_.end()) {
        return {ActionDispatchStatus::UnknownAction};
    }

    const ActionDefinition& definition = iterator->second;
    if (!payloadMatches(definition.payloadType, payload)) {
        return {ActionDispatchStatus::InvalidPayload};
    }
    dispatching_ = true;
    try {
        if (definition.isEnabled && !definition.isEnabled()) {
            dispatching_ = false;
            return {ActionDispatchStatus::Disabled};
        }
        const ActionHandlerStatus handlerStatus = definition.handler(payload);
        dispatching_ = false;
        return {handlerStatus == ActionHandlerStatus::Succeeded
                    ? ActionDispatchStatus::Succeeded
                    : ActionDispatchStatus::HandlerFailed};
    } catch (...) {
        dispatching_ = false;
        return {ActionDispatchStatus::HandlerFailed};
    }
}

bool ActionRegistry::contains(const ActionId& id) const
{
    return actions_.contains(id);
}

std::size_t ActionRegistry::size() const noexcept
{
    return actions_.size();
}

bool ActionRegistry::payloadMatches(ActionPayloadType expected,
                                    const ActionPayload& payload) noexcept
{
    switch (expected) {
    case ActionPayloadType::None:
        return std::holds_alternative<std::monostate>(payload);
    case ActionPayloadType::AppMode:
        return std::holds_alternative<AppMode>(payload);
    }
    return false;
}

}  // namespace lumaspace
