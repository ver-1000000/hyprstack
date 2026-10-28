#pragma once

#include <hyprland/src/event/EventBus.hpp>

namespace hyprstack {

class HyprlandEventBridge {
  public:
    void bind(bool& dirty);
    void reset();

  private:
    struct ListenerSet {
        CHyprSignalListener windowOpen;
        CHyprSignalListener windowClose;
        CHyprSignalListener windowDestroy;
        CHyprSignalListener windowActive;
        CHyprSignalListener windowTitle;
        CHyprSignalListener windowClass;
        CHyprSignalListener windowMoveToWorkspace;
        CHyprSignalListener workspaceActive;
        CHyprSignalListener specialWorkspaceActive;
        CHyprSignalListener monitorFocused;
#ifdef HYPRSTACK_HYPRLAND_057
        CHyprSignalListener workspaceRenamed;
        CHyprSignalListener propsRefreshed;
#endif
    };

    ListenerSet m_listeners;
};

} // namespace hyprstack
