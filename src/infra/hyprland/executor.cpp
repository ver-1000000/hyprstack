#include "hyprstack/infra/hyprland/executor.hpp"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/state/FocusState.hpp>
#include <hyprland/src/desktop/state/WindowState.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/event/EventBus.hpp>
#include <hyprland/src/layout/LayoutManager.hpp>
#include <hyprland/src/managers/fullscreen/FullscreenController.hpp>
#include <hyprland/src/render/Renderer.hpp>

#include <algorithm>

namespace hyprstack {

namespace {

PHLWINDOW focusedWindow() {
    return Desktop::focusState()->window();
}

PHLWINDOW resolveWindowByAddress(const std::string& address) {
    const auto& windows = Desktop::windowState()->windows();
    const auto  found   = std::ranges::find_if(windows, [&address](const auto& window) {
        return window && std::format("0x{:x}", reinterpret_cast<uintptr_t>(window.get())) == address;
    });

    return found == windows.end() ? nullptr : *found;
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

    if (Fullscreen::controller()->isFullscreen(focused))
        return makeDispatchError("can't swap fullscreen window");

    if (!g_layoutManager)
        return makeDispatchError("layout manager is unavailable");

    g_layoutManager->switchTargets(focused->layoutTarget(), target->layoutTarget(), true);
    focused->warpCursor();
    return {};
}

SDispatchResult swapCompositorOrder(const std::string&) {
    // Hyprland 0.56 keeps the window order private. Layout target swapping above
    // is the user-visible ordering change; preserve that without mutating internals.
    return {};
}

SDispatchResult refreshForeignToplevelForWorkspace(const int workspaceId) {
    std::vector<PHLWINDOW> windows;

    for (const auto& window : Desktop::windowState()->windows()) {
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

SDispatchResult HyprlandExecutor::focusWindowAddress(const std::string& address) const {
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

SDispatchResult HyprlandExecutor::executeStackSwap(
    const std::string& address, const std::optional<int> workspaceId, StackStore& store, const StackSwapDirection direction
) const {
    const auto swapResult = swapLayoutTargets(address);

    if (!swapResult.success)
        return swapResult;

    const auto compositorResult = swapCompositorOrder(address);

    if (!compositorResult.success)
        return compositorResult;

    if (!applyStackSwap(store, direction, workspaceId))
        return makeDispatchError(stackSwapFailureMessage(direction));

    if (!workspaceId)
        return {};

    return refreshForeignToplevelForWorkspace(*workspaceId);
}

} // namespace hyprstack
