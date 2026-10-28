#pragma once

#include "hyprstack/domain/stack_types.hpp"

#ifdef HYPRSTACK_HYPRLAND_057
#include <hyprland/src/workspace/HLWorkspace.hpp>
#else
#include <hyprland/src/desktop/Workspace.hpp>
#endif

namespace hyprstack {

inline WorkspaceId workspaceId(const PHLWORKSPACE& workspace) {
#ifdef HYPRSTACK_HYPRLAND_057
    if (const auto id = workspace->numberedID())
        return int64_t{*id};
    return workspace->addressableName();
#else
    return int64_t{workspace->m_id};
#endif
}

inline std::string workspaceName(const PHLWORKSPACE& workspace) {
#ifdef HYPRSTACK_HYPRLAND_057
    return workspace->displayName();
#else
    return workspace->m_name;
#endif
}

} // namespace hyprstack
