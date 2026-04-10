#pragma once

#include "hyprstack/application/stack_store.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace hyprstack {

enum class StackSwapDirection {
    Next,
    Prev,
};

struct StackDispatchResolution {
    std::optional<std::string> address;
    std::string                error;
};

[[nodiscard]] StackDispatchResolution successResult(std::string address);
[[nodiscard]] StackDispatchResolution errorResult(std::string error);
[[nodiscard]] StackDispatchResolution resolveStackFocusTarget(const WorkspaceStack& stack, std::string_view arg);
[[nodiscard]] StackDispatchResolution resolveStackSwapTarget(const WorkspaceStack& stack, std::string_view arg);
[[nodiscard]] std::optional<StackSwapDirection> parseStackSwapDirection(std::string_view arg);
[[nodiscard]] bool applyStackSwap(StackStore& store, StackSwapDirection direction, std::optional<int> workspaceId);
[[nodiscard]] std::string stackSwapFailureMessage(StackSwapDirection direction);

} // namespace hyprstack
