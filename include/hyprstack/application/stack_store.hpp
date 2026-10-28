#pragma once

#include "hyprstack/domain/workspace_stack.hpp"

#include <optional>
#include <string>
#include <vector>

namespace hyprstack {

struct WorkspaceRef {
    WorkspaceId id;
    std::string name;
};

struct QuerySnapshot {
    std::optional<WorkspaceRef> workspace;
    WorkspaceStack              stack;
};

class StackStore {
  public:
    void setActiveWorkspaceId(std::optional<WorkspaceId> workspaceId);
    void upsertWorkspace(const WorkspaceRef& workspace);
    WorkspaceStack& ensureWorkspace(const WorkspaceRef& workspace);
    WorkspaceStack* findWorkspaceStack(WorkspaceId workspaceId);
    const WorkspaceStack* findWorkspaceStack(WorkspaceId workspaceId) const;
    std::optional<WorkspaceRef> findWorkspaceRef(WorkspaceId workspaceId) const;
    void removeMissingWindows(WorkspaceId workspaceId, const std::vector<std::string>& presentAddresses);
    void pruneEmptyWorkspaces(const std::vector<WorkspaceId>& observedWorkspaceIds);
    [[nodiscard]] std::vector<WorkspaceId> workspaceIds() const;
    [[nodiscard]] QuerySnapshot snapshotForWorkspace(std::optional<WorkspaceId> workspaceId = std::nullopt) const;
    bool swapCurrentWithNext(std::optional<WorkspaceId> workspaceId = std::nullopt);
    bool swapCurrentWithPrev(std::optional<WorkspaceId> workspaceId = std::nullopt);

  private:
    struct WorkspaceEntry {
        WorkspaceRef   workspace;
        WorkspaceStack stack;
    };

    std::vector<WorkspaceEntry> m_workspaces;
    std::optional<WorkspaceId>  m_activeWorkspaceId;

    WorkspaceEntry*             findEntry(WorkspaceId workspaceId);
    const WorkspaceEntry*       findEntry(WorkspaceId workspaceId) const;
    [[nodiscard]] std::optional<WorkspaceId> resolveWorkspaceId(std::optional<WorkspaceId> workspaceId) const;
};

} // namespace hyprstack
