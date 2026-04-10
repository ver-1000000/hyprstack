#include "hyprstack/application/query_service.hpp"

#include "hyprstack/interface/json_renderer.hpp"

namespace hyprstack {

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
