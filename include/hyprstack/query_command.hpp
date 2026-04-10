#pragma once

#include "hyprstack/workspace_stack.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace hyprstack {

struct WorkspaceRef {
    int         id;
    std::string name;
};

struct QuerySnapshot {
    std::optional<WorkspaceRef> workspace;
    WorkspaceStack              stack;
};

std::string escapeJson(std::string_view input);
std::vector<std::string> splitArgs(std::string_view input);
std::string renderEmptyStackList();
std::string renderEmptyCurrent();
std::string renderEmptyAround();
std::string renderStackList(const std::optional<WorkspaceRef>& workspace, const WorkspaceStack& stack);
std::string renderCurrent(const std::optional<WorkspaceRef>& workspace, const WorkspaceStack& stack);
std::string renderAround(const std::optional<WorkspaceRef>& workspace, const WorkspaceStack& stack);
std::string renderHelp();
std::string handleQueryCommand(const QuerySnapshot& snapshot, const std::vector<std::string>& args);
std::string handleQueryCommand(const std::vector<std::string>& args);

} // namespace hyprstack
