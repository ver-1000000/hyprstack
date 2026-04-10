#pragma once

#include "hyprstack/plugin_state.hpp"

namespace hyprstack {

void markStateDirty(bool& dirty);
std::optional<int> activeWorkspaceId();
std::vector<ObservedWindow> observeWindows();
std::vector<ObservedWorkspace> observeWorkspaces();
void ensureStateSynced(PluginState& state, bool& dirty);
QuerySnapshot currentSnapshot(PluginState& state, bool& dirty);
std::optional<int> snapshotWorkspaceId(const QuerySnapshot& snapshot);

} // namespace hyprstack
