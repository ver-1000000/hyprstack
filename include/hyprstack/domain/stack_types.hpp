#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>

namespace hyprstack {

// Numbered workspace IDs and addressable names for named/special workspaces.
using WorkspaceId = std::variant<int64_t, std::string>;

struct StackWindow {
    std::string address;
    std::string className;
    std::string title;
};

struct StackAround {
    std::optional<std::string> current;
    std::optional<std::string> prev;
    std::optional<std::string> next;
    std::optional<std::string> last;
};

} // namespace hyprstack
