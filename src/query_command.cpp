#include "hyprstack/query_command.hpp"

#include <format>
#include <sstream>

namespace hyprstack {

namespace {

std::string renderWorkspace(const std::optional<WorkspaceRef>& workspace) {
    if (!workspace)
        return "null";

    return std::format(
        R"json({{
    "id": {},
    "name": "{}"
  }})json",
        workspace->id, escapeJson(workspace->name)
    );
}

std::string renderOptionalAddress(const std::optional<std::string>& address) {
    if (!address)
        return "null";

    return std::format(R"json("{}")json", escapeJson(*address));
}

} // namespace

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
    return renderStackList(std::nullopt, WorkspaceStack{});
}

std::string renderEmptyCurrent() {
    return renderCurrent(std::nullopt, WorkspaceStack{});
}

std::string renderEmptyAround() {
    return renderAround(std::nullopt, WorkspaceStack{});
}

std::string renderStackList(const std::optional<WorkspaceRef>& workspace, const WorkspaceStack& stack) {
    std::ostringstream windows;

    for (size_t i = 0; i < stack.windows().size(); ++i) {
        const auto& window = stack.windows()[i];

        if (i != 0)
            windows << ",\n";

        windows << std::format(
            R"json(    {{
      "index": {},
      "address": "{}",
      "class": "{}",
      "title": "{}"
    }})json",
            i, escapeJson(window.address), escapeJson(window.className), escapeJson(window.title)
        );
    }

    return std::format(
        R"json({{
  "workspace": {},
  "current": {},
  "last": {},
  "windows": [
{}
  ]
}})json",
        renderWorkspace(workspace), renderOptionalAddress(stack.current()), renderOptionalAddress(stack.last()), windows.str()
    );
}

std::string renderCurrent(const std::optional<WorkspaceRef>& workspace, const WorkspaceStack& stack) {
    return std::format(
        R"json({{
  "workspace": {},
  "current": {},
  "last": {}
}})json",
        renderWorkspace(workspace), renderOptionalAddress(stack.current()), renderOptionalAddress(stack.last())
    );
}

std::string renderAround(const std::optional<WorkspaceRef>& workspace, const WorkspaceStack& stack) {
    const auto around = stack.around();

    return std::format(
        R"json({{
  "workspace": {},
  "current": {},
  "prev": {},
  "next": {},
  "last": {}
}})json",
        renderWorkspace(workspace), renderOptionalAddress(around.current), renderOptionalAddress(around.prev),
        renderOptionalAddress(around.next), renderOptionalAddress(around.last)
    );
}

std::string renderHelp() {
    return R"help(usage: hyprstack stack <list|current|around>

returns:
    stack list       -> stable stack snapshot for the current workspace
    stack current    -> current and last window for the current workspace
    stack around     -> prev/next/last around the current window

notes:
    query api v0 is implemented for the active workspace
    responses follow docs/query-api-v0.md
    direct hyprctl invocation is not wired yet in the local environment
)help";
}

std::string handleQueryCommand(const QuerySnapshot& snapshot, const std::vector<std::string>& args) {
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
        return renderStackList(snapshot.workspace, snapshot.stack);

    if (args[offset + 1] == "current")
        return renderCurrent(snapshot.workspace, snapshot.stack);

    if (args[offset + 1] == "around")
        return renderAround(snapshot.workspace, snapshot.stack);

    return "unknown hyprstack subcommand: " + escapeJson(args[offset + 1]);
}

std::string handleQueryCommand(const std::vector<std::string>& args) {
    return handleQueryCommand(QuerySnapshot{}, args);
}

} // namespace hyprstack
