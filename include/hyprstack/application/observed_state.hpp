#pragma once

#include "hyprstack/domain/stack_types.hpp"

#include <optional>
#include <string>
#include <vector>

namespace hyprstack {

struct ObservedWindow {
    WorkspaceId           workspaceId;
    std::string           workspaceName;
    std::string           address;
    std::string           className;
    std::string           title;
    bool                  focused = false;
    std::optional<size_t> historyIndex;
};

struct ObservedWorkspace {
    WorkspaceId id;
    std::string name;
};

struct ObservedSnapshot {
    std::vector<ObservedWindow>    windows;
    std::vector<ObservedWorkspace> workspaces;
    std::optional<WorkspaceId>     activeWorkspaceId;
};

} // namespace hyprstack
