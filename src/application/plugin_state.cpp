#include "hyprstack/plugin_state.hpp"

namespace hyprstack {

void PluginState::sync(
    const std::vector<ObservedWindow>& windows, const std::vector<ObservedWorkspace>& workspaces, const std::optional<int> activeWorkspaceId
) {
    m_syncService.sync(m_store, ObservedSnapshot{
                                    .windows           = windows,
                                    .workspaces        = workspaces,
                                    .activeWorkspaceId = activeWorkspaceId,
                                });
}

QuerySnapshot PluginState::snapshotForWorkspace(const std::optional<int> workspaceId) const {
    return m_store.snapshotForWorkspace(workspaceId);
}

bool PluginState::swapCurrentWithNext(const std::optional<int> workspaceId) {
    return m_store.swapCurrentWithNext(workspaceId);
}

bool PluginState::swapCurrentWithPrev(const std::optional<int> workspaceId) {
    return m_store.swapCurrentWithPrev(workspaceId);
}

} // namespace hyprstack
