#pragma once

#include "hyprstack/application/dispatch_service.hpp"
#include "hyprstack/application/stack_store.hpp"

#include <hyprland/src/SharedDefs.hpp>

#include <optional>
#include <string>

namespace hyprstack {

SDispatchResult makeDispatchError(std::string error);

class HyprlandExecutor {
  public:
    [[nodiscard]] SDispatchResult focusWindowAddress(const std::string& address) const;
    [[nodiscard]] SDispatchResult executeStackSwap(
        const std::string& address, std::optional<WorkspaceId> workspaceId, StackStore& store, StackSwapDirection direction
    ) const;
};

} // namespace hyprstack
