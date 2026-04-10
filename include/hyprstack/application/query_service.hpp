#pragma once

#include "hyprstack/application/stack_store.hpp"

#include <string>
#include <vector>

namespace hyprstack {

std::string handleQueryCommand(const QuerySnapshot& snapshot, const std::vector<std::string>& args);
std::string handleQueryCommand(const std::vector<std::string>& args);

} // namespace hyprstack
