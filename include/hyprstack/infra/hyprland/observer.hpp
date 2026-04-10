#pragma once

#include "hyprstack/application/observed_state.hpp"

namespace hyprstack {

class HyprlandObserver {
  public:
    [[nodiscard]] ObservedSnapshot snapshot() const;
};

} // namespace hyprstack
