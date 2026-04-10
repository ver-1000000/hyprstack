#include "hyprstack/interface/query_parser.hpp"

#include <sstream>

namespace hyprstack {

std::vector<std::string> splitArgs(const std::string_view input) {
    std::istringstream       stream(std::string{input});
    std::vector<std::string> parts;
    std::string              part;

    while (stream >> part)
        parts.push_back(part);

    return parts;
}

} // namespace hyprstack
