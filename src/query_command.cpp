#include "hyprstack/query_command.hpp"

#include <sstream>

namespace hyprstack {

std::string escapeJson(const std::string_view input) {
    std::string escaped;
    escaped.reserve(input.size());

    for (const char ch : input) {
        switch (ch) {
            case '\\': escaped += "\\\\"; break;
            case '"': escaped += "\\\""; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += ch; break;
        }
    }

    return escaped;
}

std::vector<std::string> splitArgs(const std::string_view input) {
    std::istringstream      stream(std::string{input});
    std::vector<std::string> parts;
    std::string              part;

    while (stream >> part)
        parts.push_back(part);

    return parts;
}

std::string renderEmptyStackList() {
    return R"json({
  "workspace": null,
  "current": null,
  "last": null,
  "windows": []
})json";
}

std::string renderEmptyCurrent() {
    return R"json({
  "workspace": null,
  "current": null,
  "last": null
})json";
}

std::string renderEmptyAround() {
    return R"json({
  "workspace": null,
  "current": null,
  "prev": null,
  "next": null,
  "last": null
})json";
}

std::string renderHelp() {
    return R"help(usage: hyprstack stack <list|current|around>

returns:
    stack list       -> stable stack snapshot for the current workspace
    stack current    -> current and last window for the current workspace
    stack around     -> prev/next/last around the current window

notes:
    this is the v0 skeleton implementation
    window tracking is not implemented yet
    all commands currently return the fixed JSON shape from docs/query-api-v0.md
)help";
}

std::string handleQueryCommand(const std::vector<std::string>& args) {
    if (args.empty())
        return renderHelp();

    size_t offset = 0;

    if (args[0] == "hyprstack")
        offset = 1;

    if (args.size() < offset + 2)
        return renderHelp();

    if (args[offset] != "stack")
        return renderHelp();

    if (args[offset + 1] == "list")
        return renderEmptyStackList();

    if (args[offset + 1] == "current")
        return renderEmptyCurrent();

    if (args[offset + 1] == "around")
        return renderEmptyAround();

    return "unknown hyprstack subcommand: " + escapeJson(args[offset + 1]);
}

} // namespace hyprstack
