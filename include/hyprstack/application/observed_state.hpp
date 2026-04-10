#pragma once

#include <optional>
#include <string>
#include <vector>

namespace hyprstack {

struct ObservedWindow {
    int                   workspaceId;
    std::string           workspaceName;
    std::string           address;
    std::string           className;
    std::string           title;
    bool                  focused = false;
    std::optional<size_t> historyIndex;
};

struct ObservedWorkspace {
    int         id;
    std::string name;
};

struct ObservedSnapshot {
    std::vector<ObservedWindow>    windows;
    std::vector<ObservedWorkspace> workspaces;
    std::optional<int>             activeWorkspaceId;
};

} // namespace hyprstack
