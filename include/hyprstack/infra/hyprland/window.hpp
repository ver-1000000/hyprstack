#pragma once

#ifdef HYPRSTACK_HYPRLAND_057
#include <hyprland/src/desktop/view/window/Window.hpp>
#else
#include <hyprland/src/desktop/view/Window.hpp>
#endif

namespace hyprstack {

inline bool windowMapped(const PHLWINDOW& window) {
#ifdef HYPRSTACK_HYPRLAND_057
    return window->mapped();
#else
    return window->m_isMapped;
#endif
}

} // namespace hyprstack
