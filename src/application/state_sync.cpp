#include "hyprstack/application/state_sync.hpp"

#include <algorithm>
#include <ranges>

namespace hyprstack {

namespace {

StackWindow asStackWindow(const ObservedWindow& window) {
    return {
        .address   = window.address,
        .className = window.className,
        .title     = window.title,
    };
}

WorkspaceRef asWorkspaceRef(const ObservedWorkspace& workspace) {
    return {
        .id   = workspace.id,
        .name = workspace.name,
    };
}

WorkspaceRef asWorkspaceRef(const ObservedWindow& window) {
    return {
        .id   = window.workspaceId,
        .name = window.workspaceName,
    };
}

std::vector<int> observedWorkspaceIds(const std::vector<ObservedWorkspace>& workspaces) {
    std::vector<int> workspaceIds;
    workspaceIds.reserve(workspaces.size());

    for (const auto& workspace : workspaces)
        workspaceIds.push_back(workspace.id);

    return workspaceIds;
}

void replayFocusHistory(WorkspaceStack& stack, std::vector<ObservedWindow> workspaceWindows) {
    std::ranges::sort(workspaceWindows, [](const ObservedWindow& lhs, const ObservedWindow& rhs) {
        return lhs.historyIndex < rhs.historyIndex;
    });

    for (const auto& window : workspaceWindows) {
        if (!window.historyIndex)
            continue;

        stack.focusWindow(asStackWindow(window));
    }
}

} // namespace

void StateSyncService::sync(StackStore& store, const ObservedSnapshot& snapshot) const {
    store.setActiveWorkspaceId(snapshot.activeWorkspaceId);

    for (const auto& workspace : snapshot.workspaces)
        store.upsertWorkspace(asWorkspaceRef(workspace));

    for (const auto& window : snapshot.windows) {
        auto& stack = store.ensureWorkspace(asWorkspaceRef(window));
        stack.addWindow(asStackWindow(window));

        if (window.focused)
            stack.focusWindow(asStackWindow(window));
    }

    for (const int workspaceId : store.workspaceIds()) {
        std::vector<ObservedWindow> workspaceWindows;
        std::vector<std::string>    presentAddresses;

        for (const auto& window : snapshot.windows) {
            if (window.workspaceId != workspaceId)
                continue;

            presentAddresses.push_back(window.address);
            workspaceWindows.push_back(window);
        }

        store.removeMissingWindows(workspaceId, presentAddresses);

        auto* stack = store.findWorkspaceStack(workspaceId);

        if (!stack)
            continue;

        replayFocusHistory(*stack, std::move(workspaceWindows));
    }

    store.pruneEmptyWorkspaces(observedWorkspaceIds(snapshot.workspaces));
}

} // namespace hyprstack
