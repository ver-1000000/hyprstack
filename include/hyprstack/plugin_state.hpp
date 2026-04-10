#pragma once

#include "hyprstack/query_command.hpp"

#include <optional>
#include <string>
#include <vector>

namespace hyprstack {

struct ObservedWindow {
    int         workspaceId;
    std::string workspaceName;
    std::string address;
    std::string className;
    std::string title;
    bool        focused = false;
    std::optional<size_t> historyIndex;
};

struct ObservedWorkspace {
    int         id;
    std::string name;
};

class PluginState {
  public:
    void sync(const std::vector<ObservedWindow>& windows, const std::vector<ObservedWorkspace>& workspaces, std::optional<int> activeWorkspaceId);
    [[nodiscard]] QuerySnapshot snapshotForWorkspace(std::optional<int> workspaceId = std::nullopt) const;

  private:
    struct WorkspaceState {
        WorkspaceRef    workspace;
        WorkspaceStack  stack;
    };

    std::vector<WorkspaceState> m_workspaces;
    std::optional<int>          m_activeWorkspaceId;

    WorkspaceState*       findWorkspace(int workspaceId);
    const WorkspaceState* findWorkspace(int workspaceId) const;
};

} // namespace hyprstack
