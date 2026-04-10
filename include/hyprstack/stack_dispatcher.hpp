#pragma once

#include "hyprstack/workspace_stack.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace hyprstack {

struct StackDispatchResolution {
    std::optional<std::string> address;
    std::string                error;
};

[[nodiscard]] StackDispatchResolution successResult(std::string address);
[[nodiscard]] StackDispatchResolution errorResult(std::string error);
[[nodiscard]] StackDispatchResolution resolveStackFocusTarget(const WorkspaceStack& stack, std::string_view arg);
[[nodiscard]] StackDispatchResolution resolveStackSwapTarget(const WorkspaceStack& stack, std::string_view arg);

} // namespace hyprstack
