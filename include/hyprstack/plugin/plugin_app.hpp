#pragma once

#include "hyprstack/application/stack_store.hpp"
#include "hyprstack/application/state_sync.hpp"
#include "hyprstack/infra/hyprland/event_bridge.hpp"
#include "hyprstack/infra/hyprland/executor.hpp"
#include "hyprstack/infra/hyprland/observer.hpp"

#include <hyprland/src/managers/KeybindManager.hpp>

#include <string>

namespace hyprstack {

class PluginApp {
  public:
    void bindEvents();
    void reset();

    [[nodiscard]] std::string handleHyprCtl(const std::string& args);
    [[nodiscard]] SDispatchResult handleStackFocus(const std::string& args);
    [[nodiscard]] SDispatchResult handleStackSwap(const std::string& args);

  private:
    void ensureStateSynced();
    [[nodiscard]] QuerySnapshot currentSnapshot();

    StackStore         m_store;
    StateSyncService   m_syncService;
    HyprlandObserver   m_observer;
    HyprlandExecutor   m_executor;
    HyprlandEventBridge m_eventBridge;
    bool               m_dirty = true;
};

} // namespace hyprstack
