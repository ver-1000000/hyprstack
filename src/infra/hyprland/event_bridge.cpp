#include "hyprstack/infra/hyprland/event_bridge.hpp"

#include <hyprland/src/desktop/state/FocusState.hpp>

namespace hyprstack {

void HyprlandEventBridge::bind(bool& dirty) {
    auto& events = Event::bus()->m_events;

    m_listeners.windowOpen            = events.window.open.listen([&dirty](PHLWINDOW) { dirty = true; });
    m_listeners.windowClose           = events.window.close.listen([&dirty](PHLWINDOW) { dirty = true; });
    m_listeners.windowDestroy         = events.window.destroy.listen([&dirty] { dirty = true; });
    m_listeners.windowActive          = events.window.active.listen([&dirty](PHLWINDOW, Desktop::eFocusReason) { dirty = true; });
    m_listeners.windowTitle           = events.window.title.listen([&dirty](PHLWINDOW) { dirty = true; });
    m_listeners.windowClass           = events.window.class_.listen([&dirty](PHLWINDOW) { dirty = true; });
    m_listeners.windowMoveToWorkspace = events.window.moveToWorkspace.listen([&dirty](PHLWINDOW, PHLWORKSPACE) { dirty = true; });
    m_listeners.workspaceActive       = events.workspace.active.listen([&dirty](PHLWORKSPACE) { dirty = true; });
    m_listeners.specialWorkspaceActive = events.workspace.specialActive.listen([&dirty](PHLWORKSPACE, PHLMONITOR) { dirty = true; });
    m_listeners.monitorFocused         = events.monitor.focused.listen([&dirty](PHLMONITOR) { dirty = true; });
#ifdef HYPRSTACK_HYPRLAND_057
    m_listeners.workspaceRenamed       = events.workspace.renamed.listen([&dirty](PHLWORKSPACEREF) { dirty = true; });
    m_listeners.propsRefreshed         = events.config.props_refreshed.listen([&dirty](bool) { dirty = true; });
#endif
}

void HyprlandEventBridge::reset() {
    m_listeners = {};
}

} // namespace hyprstack
