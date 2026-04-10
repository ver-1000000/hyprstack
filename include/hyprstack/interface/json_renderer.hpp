#pragma once

#include "hyprstack/application/stack_store.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace hyprstack {

std::string escapeJson(std::string_view input);
std::string renderEmptyStackList();
std::string renderEmptyCurrent();
std::string renderEmptyAround();
std::string renderStackList(const std::optional<WorkspaceRef>& workspace, const WorkspaceStack& stack);
std::string renderCurrent(const std::optional<WorkspaceRef>& workspace, const WorkspaceStack& stack);
std::string renderAround(const std::optional<WorkspaceRef>& workspace, const WorkspaceStack& stack);
std::string renderHelp();

} // namespace hyprstack
