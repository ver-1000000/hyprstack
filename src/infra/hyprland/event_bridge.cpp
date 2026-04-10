#include "hyprstack/infra/hyprland/event_bridge.hpp"

#include <hyprland/src/desktop/state/FocusState.hpp>

namespace hyprstack {

void HyprlandEventBridge::bind(bool& dirty) {
    auto& events = Event::bus()->m_events;

    m_listeners.windowOpen            = events.window.open.listen([&dirty](PHLWINDOW) { dirty = true; });
    m_listeners.windowClose           = events.window.close.listen([&dirty](PHLWINDOW) { dirty = true; });
    m_listeners.windowDestroy         = events.window.destroy.listen([&dirty](PHLWINDOW) { dirty = true; });
    m_listeners.windowActive          = events.window.active.listen([&dirty](PHLWINDOW, Desktop::eFocusReason) { dirty = true; });
    m_listeners.windowTitle           = events.window.title.listen([&dirty](PHLWINDOW) { dirty = true; });
    m_listeners.windowClass           = events.window.class_.listen([&dirty](PHLWINDOW) { dirty = true; });
    m_listeners.windowMoveToWorkspace = events.window.moveToWorkspace.listen([&dirty](PHLWINDOW, PHLWORKSPACE) { dirty = true; });
    m_listeners.workspaceActive       = events.workspace.active.listen([&dirty](PHLWORKSPACE) { dirty = true; });
}

void HyprlandEventBridge::reset() {
    m_listeners = {};
}

} // namespace hyprstack
