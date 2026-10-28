#include "hyprstack/infra/hyprland/observer.hpp"

#include "hyprstack/infra/hyprland/workspace.hpp"
#include "hyprstack/infra/hyprland/window.hpp"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/history/WindowHistoryTracker.hpp>
#include <hyprland/src/desktop/state/FocusState.hpp>
#include <hyprland/src/desktop/state/WindowState.hpp>
#include <hyprland/src/output/Monitor.hpp>
#include <hyprland/src/state/MonitorState.hpp>
#include <hyprland/src/state/WorkspaceState.hpp>

#include <format>

namespace hyprstack {

namespace {

std::string formatAddress(const PHLWINDOW& window) {
    return std::format("0x{:x}", reinterpret_cast<uintptr_t>(window.get()));
}

std::optional<WorkspaceId> activeWorkspaceId() {
    if (const auto focusedWindow = Desktop::focusState()->window(); focusedWindow && valid(focusedWindow->m_workspace))
        return workspaceId(focusedWindow->m_workspace);

    if (const auto monitor = Desktop::focusState()->monitor()) {
        if (valid(monitor->m_activeSpecialWorkspace))
            return workspaceId(monitor->m_activeSpecialWorkspace);
        if (valid(monitor->m_activeWorkspace))
            return workspaceId(monitor->m_activeWorkspace);
    }

    for (const auto& monitor : State::monitorState()->monitors()) {
        if (monitor && valid(monitor->m_activeWorkspace))
            return workspaceId(monitor->m_activeWorkspace);
    }

    return std::nullopt;
}

std::vector<ObservedWindow> observeWindows() {
    std::vector<ObservedWindow> observed;
    const auto focusedWindow = Desktop::focusState()->window();
    const auto history       = Desktop::History::windowTracker()->fullHistory();

    for (const auto& window : Desktop::windowState()->windows()) {
        if (!window || !windowMapped(window) || !valid(window->m_workspace))
            continue;

        std::optional<size_t> historyIndex;
        for (size_t i = 0; i < history.size(); ++i) {
            if (const auto historyWindow = history[i].lock(); historyWindow && historyWindow == window) {
                historyIndex = i;
                break;
            }
        }

        observed.push_back({
            .workspaceId   = workspaceId(window->m_workspace),
            .workspaceName = workspaceName(window->m_workspace),
            .address       = formatAddress(window),
#ifdef HYPRSTACK_HYPRLAND_057
            .className     = window->metadata().appID(),
            .title         = window->metadata().title(),
#else
            .className     = window->m_class,
            .title         = window->m_title,
#endif
            .focused       = focusedWindow && window == focusedWindow,
            .historyIndex  = historyIndex,
        });
    }

    return observed;
}

std::vector<ObservedWorkspace> observeWorkspaces() {
    std::vector<ObservedWorkspace> observed;

    for (const auto& workspaceRef : State::workspaceState()->workspaceRefs()) {
        const auto workspace = workspaceRef.lock();

        if (!valid(workspace))
            continue;

        observed.push_back({
            .id   = workspaceId(workspace),
            .name = workspaceName(workspace),
        });
    }

    return observed;
}

} // namespace

ObservedSnapshot HyprlandObserver::snapshot() const {
    return {
        .windows           = observeWindows(),
        .workspaces        = observeWorkspaces(),
        .activeWorkspaceId = activeWorkspaceId(),
    };
}

} // namespace hyprstack
