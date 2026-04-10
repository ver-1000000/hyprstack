#include "hyprstack/plugin_state.hpp"

#include <algorithm>
#include <ranges>

namespace hyprstack {

void PluginState::sync(const std::vector<ObservedWindow>& windows, const std::optional<int> activeWorkspaceId) {
    m_activeWorkspaceId = activeWorkspaceId;

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
        std::vector<std::string> presentAddresses;

        for (const auto& window : windows) {
            if (window.workspaceId == workspace.workspace.id)
                presentAddresses.push_back(window.address);
        }

        std::vector<std::string> missingAddresses;

        for (const auto& window : workspace.stack.windows()) {
            if (!std::ranges::contains(presentAddresses, window.address))
                missingAddresses.push_back(window.address);
        }

        for (const auto& address : missingAddresses)
            workspace.stack.removeWindow(address);
    }

    std::erase_if(m_workspaces, [](const WorkspaceState& workspace) { return workspace.stack.windows().empty(); });
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
