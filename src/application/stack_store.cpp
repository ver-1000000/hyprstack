#include "hyprstack/application/stack_store.hpp"

#include <algorithm>
#include <ranges>

namespace hyprstack {

void StackStore::setActiveWorkspaceId(const std::optional<WorkspaceId> workspaceId) {
    m_activeWorkspaceId = workspaceId;
}

void StackStore::upsertWorkspace(const WorkspaceRef& workspace) {
    if (auto* existing = findEntry(workspace.id)) {
        existing->workspace = workspace;
        return;
    }

    m_workspaces.push_back(WorkspaceEntry{
        .workspace = workspace,
        .stack     = {},
    });
}

WorkspaceStack& StackStore::ensureWorkspace(const WorkspaceRef& workspace) {
    upsertWorkspace(workspace);
    return findEntry(workspace.id)->stack;
}

WorkspaceStack* StackStore::findWorkspaceStack(const WorkspaceId workspaceId) {
    auto* entry = findEntry(workspaceId);

    if (!entry)
        return nullptr;

    return &entry->stack;
}

const WorkspaceStack* StackStore::findWorkspaceStack(const WorkspaceId workspaceId) const {
    const auto* entry = findEntry(workspaceId);

    if (!entry)
        return nullptr;

    return &entry->stack;
}

std::optional<WorkspaceRef> StackStore::findWorkspaceRef(const WorkspaceId workspaceId) const {
    const auto* entry = findEntry(workspaceId);

    if (!entry)
        return std::nullopt;

    return entry->workspace;
}

void StackStore::removeMissingWindows(const WorkspaceId workspaceId, const std::vector<std::string>& presentAddresses) {
    auto* stack = findWorkspaceStack(workspaceId);

    if (!stack)
        return;

    std::vector<std::string> missingAddresses;

    for (const auto& window : stack->windows()) {
        if (!std::ranges::contains(presentAddresses, window.address))
            missingAddresses.push_back(window.address);
    }

    for (const auto& address : missingAddresses)
        stack->removeWindow(address);
}

void StackStore::pruneEmptyWorkspaces(const std::vector<WorkspaceId>& observedWorkspaceIds) {
    std::erase_if(m_workspaces, [&observedWorkspaceIds](const WorkspaceEntry& entry) {
        return entry.stack.windows().empty() && !std::ranges::contains(observedWorkspaceIds, entry.workspace.id);
    });
}

std::vector<WorkspaceId> StackStore::workspaceIds() const {
    std::vector<WorkspaceId> workspaceIds;
    workspaceIds.reserve(m_workspaces.size());

    for (const auto& entry : m_workspaces)
        workspaceIds.push_back(entry.workspace.id);

    return workspaceIds;
}

QuerySnapshot StackStore::snapshotForWorkspace(const std::optional<WorkspaceId> workspaceId) const {
    const auto targetWorkspaceId = resolveWorkspaceId(workspaceId);

    if (!targetWorkspaceId)
        return {};

    const auto* entry = findEntry(*targetWorkspaceId);

    if (!entry)
        return {};

    return {
        .workspace = entry->workspace,
        .stack     = entry->stack,
    };
}

bool StackStore::swapCurrentWithNext(const std::optional<WorkspaceId> workspaceId) {
    const auto targetWorkspaceId = resolveWorkspaceId(workspaceId);

    if (!targetWorkspaceId)
        return false;

    auto* stack = findWorkspaceStack(*targetWorkspaceId);

    if (!stack)
        return false;

    return stack->swapCurrentWithNext();
}

bool StackStore::swapCurrentWithPrev(const std::optional<WorkspaceId> workspaceId) {
    const auto targetWorkspaceId = resolveWorkspaceId(workspaceId);

    if (!targetWorkspaceId)
        return false;

    auto* stack = findWorkspaceStack(*targetWorkspaceId);

    if (!stack)
        return false;

    return stack->swapCurrentWithPrev();
}

StackStore::WorkspaceEntry* StackStore::findEntry(const WorkspaceId workspaceId) {
    const auto iter = std::ranges::find(m_workspaces, workspaceId, [](const WorkspaceEntry& entry) { return entry.workspace.id; });

    if (iter == m_workspaces.end())
        return nullptr;

    return &(*iter);
}

const StackStore::WorkspaceEntry* StackStore::findEntry(const WorkspaceId workspaceId) const {
    const auto iter = std::ranges::find(m_workspaces, workspaceId, [](const WorkspaceEntry& entry) { return entry.workspace.id; });

    if (iter == m_workspaces.end())
        return nullptr;

    return &(*iter);
}

std::optional<WorkspaceId> StackStore::resolveWorkspaceId(const std::optional<WorkspaceId> workspaceId) const {
    return workspaceId.or_else([this]() { return m_activeWorkspaceId; });
}

} // namespace hyprstack
