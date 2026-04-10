#pragma once

#include "hyprstack/application/observed_state.hpp"
#include "hyprstack/application/stack_store.hpp"
#include "hyprstack/application/state_sync.hpp"

#include <optional>

namespace hyprstack {

class PluginState {
  public:
    void sync(const std::vector<ObservedWindow>& windows, const std::vector<ObservedWorkspace>& workspaces, std::optional<int> activeWorkspaceId);
    [[nodiscard]] QuerySnapshot snapshotForWorkspace(std::optional<int> workspaceId = std::nullopt) const;
    [[nodiscard]] bool swapCurrentWithNext(std::optional<int> workspaceId = std::nullopt);
    [[nodiscard]] bool swapCurrentWithPrev(std::optional<int> workspaceId = std::nullopt);

  private:
    StackStore       m_store;
    StateSyncService m_syncService;
};

} // namespace hyprstack
