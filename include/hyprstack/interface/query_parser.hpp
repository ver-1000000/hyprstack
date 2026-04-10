#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace hyprstack {

std::vector<std::string> splitArgs(std::string_view input);

} // namespace hyprstack
