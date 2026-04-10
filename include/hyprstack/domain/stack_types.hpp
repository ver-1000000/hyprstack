#pragma once

#include <optional>
#include <string>

namespace hyprstack {

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
