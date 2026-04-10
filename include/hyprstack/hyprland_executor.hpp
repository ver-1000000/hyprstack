#pragma once

#include "hyprstack/plugin_state.hpp"

#include <hyprland/src/managers/KeybindManager.hpp>

#include <optional>
#include <string>
#include <string_view>

namespace hyprstack {

using PluginStateSwapOperation = bool (PluginState::*)(std::optional<int>);

struct StackSwapAction {
    PluginStateSwapOperation operation;
    std::string              failureMessage;
};

SDispatchResult makeDispatchError(std::string error);
std::optional<StackSwapAction> resolveSwapAction(std::string_view action);
SDispatchResult focusWindowAddress(const std::string& address);
SDispatchResult executeStackSwap(
    const std::string& address, std::optional<int> workspaceId, PluginState& state, PluginStateSwapOperation operation, std::string_view failureMessage
);

} // namespace hyprstack
