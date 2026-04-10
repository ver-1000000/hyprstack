#include "hyprstack/plugin_state.hpp"

#include <algorithm>
#include <ranges>

namespace hyprstack {

void PluginState::sync(
    const std::vector<ObservedWindow>& windows, const std::vector<ObservedWorkspace>& workspaces, const std::optional<int> activeWorkspaceId
) {
    m_activeWorkspaceId = activeWorkspaceId;

    for (const auto& workspace : workspaces) {
        auto* existing = findWorkspace(workspace.id);

        if (!existing) {
            m_workspaces.push_back(WorkspaceState{
                .workspace =
                    {
                        .id   = workspace.id,
                        .name = workspace.name,
                    },
                .stack = {},
            });
            continue;
        }

        existing->workspace.name = workspace.name;
    }

    for (const auto& window : windows) {
        auto* workspace = findWorkspace(window.workspaceId);

        if (!workspace) {
            m_workspaces.push_back(WorkspaceState{
                .workspace =
                    {
                        .id   = window.workspaceId,
                        .name = window.workspaceName,
                    },
                .stack = {},
            });
            workspace = &m_workspaces.back();
        } else {
            workspace->workspace.name = window.workspaceName;
        }

        workspace->stack.addWindow({
            .address   = window.address,
            .className = window.className,
            .title     = window.title,
        });

        if (window.focused) {
            workspace->stack.focusWindow({
                .address   = window.address,
                .className = window.className,
                .title     = window.title,
            });
        }
    }

    for (auto& workspace : m_workspaces) {
        std::vector<ObservedWindow> workspaceWindows;
        std::vector<std::string> presentAddresses;

        for (const auto& window : windows) {
            if (window.workspaceId == workspace.workspace.id) {
                presentAddresses.push_back(window.address);
                workspaceWindows.push_back(window);
            }
        }

        std::vector<std::string> missingAddresses;

        for (const auto& window : workspace.stack.windows()) {
            if (!std::ranges::contains(presentAddresses, window.address))
                missingAddresses.push_back(window.address);
        }

        for (const auto& address : missingAddresses)
            workspace.stack.removeWindow(address);

        std::ranges::sort(workspaceWindows, [](const ObservedWindow& lhs, const ObservedWindow& rhs) {
            return lhs.historyIndex < rhs.historyIndex;
        });

        for (const auto& window : workspaceWindows) {
            if (!window.historyIndex)
                continue;

            workspace.stack.focusWindow({
                .address   = window.address,
                .className = window.className,
                .title     = window.title,
            });
        }
    }

    std::vector<int> observedWorkspaceIds;
    observedWorkspaceIds.reserve(workspaces.size());

    for (const auto& workspace : workspaces)
        observedWorkspaceIds.push_back(workspace.id);

    std::erase_if(m_workspaces, [&observedWorkspaceIds](const WorkspaceState& workspace) {
        return workspace.stack.windows().empty() && !std::ranges::contains(observedWorkspaceIds, workspace.workspace.id);
    });
}

QuerySnapshot PluginState::snapshotForWorkspace(const std::optional<int> workspaceId) const {
    const auto targetWorkspaceId = workspaceId.or_else([this]() { return m_activeWorkspaceId; });

    if (!targetWorkspaceId)
        return {};

    const auto* workspace = findWorkspace(*targetWorkspaceId);

    if (!workspace)
        return {};

    return {
        .workspace = workspace->workspace,
        .stack     = workspace->stack,
    };
}

bool PluginState::swapCurrentWithNext(const std::optional<int> workspaceId) {
    const auto targetWorkspaceId = workspaceId.or_else([this]() { return m_activeWorkspaceId; });

    if (!targetWorkspaceId)
        return false;

    auto* workspace = findWorkspace(*targetWorkspaceId);

    if (!workspace)
        return false;

    return workspace->stack.swapCurrentWithNext();
}

bool PluginState::swapCurrentWithPrev(const std::optional<int> workspaceId) {
    const auto targetWorkspaceId = workspaceId.or_else([this]() { return m_activeWorkspaceId; });

    if (!targetWorkspaceId)
        return false;

    auto* workspace = findWorkspace(*targetWorkspaceId);

    if (!workspace)
        return false;

    return workspace->stack.swapCurrentWithPrev();
}

PluginState::WorkspaceState* PluginState::findWorkspace(const int workspaceId) {
    const auto iter = std::ranges::find(m_workspaces, workspaceId, [](const WorkspaceState& workspace) { return workspace.workspace.id; });

    if (iter == m_workspaces.end())
        return nullptr;

    return &(*iter);
}

const PluginState::WorkspaceState* PluginState::findWorkspace(const int workspaceId) const {
    const auto iter = std::ranges::find(m_workspaces, workspaceId, [](const WorkspaceState& workspace) { return workspace.workspace.id; });

    if (iter == m_workspaces.end())
        return nullptr;

    return &(*iter);
}

} // namespace hyprstack
