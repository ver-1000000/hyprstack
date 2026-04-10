#pragma once

#include "hyprstack/domain/workspace_stack.hpp"

#include <optional>
#include <string>
#include <vector>

namespace hyprstack {

struct WorkspaceRef {
    int         id;
    std::string name;
};

struct QuerySnapshot {
    std::optional<WorkspaceRef> workspace;
    WorkspaceStack              stack;
};

class StackStore {
  public:
    void setActiveWorkspaceId(std::optional<int> workspaceId);
    void upsertWorkspace(const WorkspaceRef& workspace);
    WorkspaceStack& ensureWorkspace(const WorkspaceRef& workspace);
    WorkspaceStack* findWorkspaceStack(int workspaceId);
    const WorkspaceStack* findWorkspaceStack(int workspaceId) const;
    std::optional<WorkspaceRef> findWorkspaceRef(int workspaceId) const;
    void removeMissingWindows(int workspaceId, const std::vector<std::string>& presentAddresses);
    void pruneEmptyWorkspaces(const std::vector<int>& observedWorkspaceIds);
    [[nodiscard]] std::vector<int> workspaceIds() const;
    [[nodiscard]] QuerySnapshot snapshotForWorkspace(std::optional<int> workspaceId = std::nullopt) const;
    bool swapCurrentWithNext(std::optional<int> workspaceId = std::nullopt);
    bool swapCurrentWithPrev(std::optional<int> workspaceId = std::nullopt);

  private:
    struct WorkspaceEntry {
        WorkspaceRef   workspace;
        WorkspaceStack stack;
    };

    std::vector<WorkspaceEntry> m_workspaces;
    std::optional<int>          m_activeWorkspaceId;

    WorkspaceEntry*             findEntry(int workspaceId);
    const WorkspaceEntry*       findEntry(int workspaceId) const;
    [[nodiscard]] std::optional<int> resolveWorkspaceId(std::optional<int> workspaceId) const;
};

} // namespace hyprstack
