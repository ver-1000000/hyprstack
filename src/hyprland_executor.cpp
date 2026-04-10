#include "hyprstack/hyprland_executor.hpp"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/state/FocusState.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/event/EventBus.hpp>
#include <hyprland/src/layout/LayoutManager.hpp>
#include <hyprland/src/render/Renderer.hpp>

#include <algorithm>

namespace hyprstack {

namespace {

PHLWINDOW focusedWindow() {
    return Desktop::focusState()->window();
}

PHLWINDOW resolveWindowByAddress(const std::string& address) {
    return g_pCompositor->getWindowByRegex("address:" + address);
}

SDispatchResult resolveSwapWindows(const std::string& targetAddress, PHLWINDOW& focused, PHLWINDOW& target) {
    focused = focusedWindow();

    if (!focused)
        return makeDispatchError("focused window is unavailable");

    target = resolveWindowByAddress(targetAddress);

    if (!target || target == focused)
        return makeDispatchError("target window is unavailable");

    return {};
}

SDispatchResult swapLayoutTargets(const std::string& targetAddress) {
    PHLWINDOW focused;
    PHLWINDOW target;
    const auto resolution = resolveSwapWindows(targetAddress, focused, target);

    if (!resolution.success)
        return resolution;

    if (focused->isFullscreen())
        return makeDispatchError("can't swap fullscreen window");

    if (!g_layoutManager)
        return makeDispatchError("layout manager is unavailable");

    g_layoutManager->switchTargets(focused->layoutTarget(), target->layoutTarget(), true);
    focused->warpCursor();
    return {};
}

SDispatchResult swapCompositorOrder(const std::string& targetAddress) {
    PHLWINDOW focused;
    PHLWINDOW target;
    const auto resolution = resolveSwapWindows(targetAddress, focused, target);

    if (!resolution.success)
        return resolution;

    const auto focusedIt = std::ranges::find(g_pCompositor->m_windows, focused);
    const auto targetIt  = std::ranges::find(g_pCompositor->m_windows, target);

    if (focusedIt == g_pCompositor->m_windows.end() || targetIt == g_pCompositor->m_windows.end())
        return makeDispatchError("window is unavailable in compositor order");

    // Waybar taskbar order follows Hyprland's foreign toplevel advertisement order.
    // In practice that order tracks g_pCompositor->m_windows, so stackswap updates it too.
    std::iter_swap(focusedIt, targetIt);

    if (focused->m_isMapped)
        g_pHyprRenderer->damageMonitor(focused->m_monitor.lock());

    if (target->m_isMapped)
        g_pHyprRenderer->damageMonitor(target->m_monitor.lock());

    return {};
}

SDispatchResult refreshForeignToplevelForWorkspace(const int workspaceId) {
    std::vector<PHLWINDOW> windows;

    for (const auto& window : g_pCompositor->m_windows) {
        if (!window || !window->m_isMapped || !valid(window->m_workspace))
            continue;

        if (window->m_workspace->m_id != workspaceId)
            continue;

        windows.push_back(window);
    }

    // Foreign toplevel clients do not observe compositor-order changes live, so
    // we re-emit close/open for the workspace windows to force re-advertisement.
    for (const auto& window : windows)
        Event::bus()->m_events.window.close.emit(window);

    for (const auto& window : windows)
        Event::bus()->m_events.window.open.emit(window);

    return {};
}

} // namespace

SDispatchResult makeDispatchError(std::string error) {
    return {
        .success = false,
        .error   = std::move(error),
    };
}

std::optional<StackSwapAction> resolveSwapAction(const std::string_view action) {
    if (action == "next") {
        return StackSwapAction{
            .operation      = &PluginState::swapCurrentWithNext,
            .failureMessage = "stackswap next requires a focused window and at least two windows",
        };
    }

    if (action == "prev") {
        return StackSwapAction{
            .operation      = &PluginState::swapCurrentWithPrev,
            .failureMessage = "stackswap prev requires a focused window and at least two windows",
        };
    }

    return std::nullopt;
}

SDispatchResult focusWindowAddress(const std::string& address) {
    if (!g_pKeybindManager)
        return makeDispatchError("hypr keybind manager is unavailable");

    const auto dispatcher = g_pKeybindManager->m_dispatchers.find("focuswindow");

    if (dispatcher == g_pKeybindManager->m_dispatchers.end())
        return makeDispatchError("focuswindow dispatcher is unavailable");

    const auto focusResult = dispatcher->second("address:" + address);

    if (!focusResult.success)
        return focusResult;

    const auto zOrderDispatcher = g_pKeybindManager->m_dispatchers.find("alterzorder");

    if (zOrderDispatcher == g_pKeybindManager->m_dispatchers.end())
        return focusResult;

    const auto zOrderResult = zOrderDispatcher->second("top,address:" + address);

    if (!zOrderResult.success)
        return zOrderResult;

    return focusResult;
}

SDispatchResult executeStackSwap(
    const std::string& address, const std::optional<int> workspaceId, PluginState& state, const PluginStateSwapOperation operation,
    const std::string_view failureMessage
) {
    const auto swapResult = swapLayoutTargets(address);

    if (!swapResult.success)
        return swapResult;

    const auto compositorResult = swapCompositorOrder(address);

    if (!compositorResult.success)
        return compositorResult;

    if (!(state.*operation)(workspaceId))
        return makeDispatchError(std::string(failureMessage));

    if (!workspaceId)
        return {};

    return refreshForeignToplevelForWorkspace(*workspaceId);
}

} // namespace hyprstack
