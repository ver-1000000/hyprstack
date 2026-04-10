#pragma once

#include "hyprstack/application/observed_state.hpp"
#include "hyprstack/application/stack_store.hpp"

namespace hyprstack {

class StateSyncService {
  public:
    void sync(StackStore& store, const ObservedSnapshot& snapshot) const;
};

} // namespace hyprstack
