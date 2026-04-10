#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace hyprstack {

std::string escapeJson(std::string_view input);
std::vector<std::string> splitArgs(std::string_view input);
std::string renderEmptyStackList();
std::string renderEmptyCurrent();
std::string renderEmptyAround();
std::string renderHelp();
std::string handleQueryCommand(const std::vector<std::string>& args);

} // namespace hyprstack
