#include <hyprland/src/plugins/PluginAPI.hpp>

#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

inline HANDLE               G_HANDLE          = nullptr;
inline SP<SHyprCtlCommand>  G_HYPRCTL_COMMAND = nullptr;

namespace {

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

std::string handleStackCommand(const std::vector<std::string>& args) {
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

std::string onHyprCtl([[maybe_unused]] eHyprCtlOutputFormat format, const std::string args) {
    return handleStackCommand(splitArgs(args));
}

} // namespace

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    G_HANDLE = handle;

    const std::string compositorHash = __hyprland_api_get_hash();
    const std::string clientHash     = __hyprland_api_get_client_hash();

    if (compositorHash != clientHash) {
        HyprlandAPI::addNotification(
            G_HANDLE, "[hyprstack] Mismatched Hyprland headers. Refusing to load.", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000
        );
        throw std::runtime_error("[hyprstack] Version mismatch");
    }

    G_HYPRCTL_COMMAND = HyprlandAPI::registerHyprCtlCommand(
        G_HANDLE,
        {
            .name  = "hyprstack",
            .exact = false,
            .fn    = onHyprCtl,
        }
    );

    if (!G_HYPRCTL_COMMAND)
        throw std::runtime_error("[hyprstack] Failed to register hyprctl command");

    HyprlandAPI::addNotification(
        G_HANDLE, "[hyprstack] Loaded Query API v0 skeleton", CHyprColor{0.2, 0.8, 1.0, 1.0}, 3000
    );

    return {
        .name        = "hyprstack",
        .description = "Workspace-local stable window stack semantics for Hyprland",
        .author      = "AKAI",
        .version     = "0.1.1",
    };
}

APICALL EXPORT void PLUGIN_EXIT() {
    if (G_HANDLE && G_HYPRCTL_COMMAND) {
        HyprlandAPI::unregisterHyprCtlCommand(G_HANDLE, G_HYPRCTL_COMMAND);
        G_HYPRCTL_COMMAND.reset();
    }

    G_HANDLE = nullptr;
}
