#include "hyprstack/infra/hyprland/observer.hpp"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/history/WindowHistoryTracker.hpp>
#include <hyprland/src/desktop/state/FocusState.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/helpers/Monitor.hpp>

#include <format>

namespace hyprstack {

namespace {

std::string formatAddress(const PHLWINDOW& window) {
    return std::format("0x{:x}", reinterpret_cast<uintptr_t>(window.get()));
}

std::optional<int> activeWorkspaceId() {
    if (const auto focusedWindow = Desktop::focusState()->window(); focusedWindow && valid(focusedWindow->m_workspace))
        return static_cast<int>(focusedWindow->m_workspace->m_id);

    for (const auto& monitor : g_pCompositor->m_monitors) {
        if (monitor && valid(monitor->m_activeWorkspace))
            return static_cast<int>(monitor->m_activeWorkspace->m_id);
    }

    return std::nullopt;
}

std::vector<ObservedWindow> observeWindows() {
    std::vector<ObservedWindow> observed;
    const auto focusedWindow = Desktop::focusState()->window();
    const auto history       = Desktop::History::windowTracker()->fullHistory();

    for (const auto& window : g_pCompositor->m_windows) {
        if (!window || !window->m_isMapped || !valid(window->m_workspace))
            continue;

        std::optional<size_t> historyIndex;
        for (size_t i = 0; i < history.size(); ++i) {
            if (const auto historyWindow = history[i].lock(); historyWindow && historyWindow == window) {
                historyIndex = i;
                break;
            }
        }

        observed.push_back({
            .workspaceId   = static_cast<int>(window->m_workspace->m_id),
            .workspaceName = window->m_workspace->m_name,
            .address       = formatAddress(window),
            .className     = window->m_class,
            .title         = window->m_title,
            .focused       = focusedWindow && window == focusedWindow,
            .historyIndex  = historyIndex,
        });
    }

    return observed;
}

std::vector<ObservedWorkspace> observeWorkspaces() {
    std::vector<ObservedWorkspace> observed;

    for (const auto& workspaceRef : g_pCompositor->getWorkspaces()) {
        const auto workspace = workspaceRef.lock();

        if (!valid(workspace))
            continue;

        observed.push_back({
            .id   = static_cast<int>(workspace->m_id),
            .name = workspace->m_name,
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
