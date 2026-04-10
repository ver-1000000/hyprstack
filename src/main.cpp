#include <hyprland/src/plugins/PluginAPI.hpp>

#include "hyprstack/query_command.hpp"

#include <stdexcept>
#include <string>

inline HANDLE               G_HANDLE          = nullptr;
inline SP<SHyprCtlCommand>  G_HYPRCTL_COMMAND = nullptr;

namespace {

std::string onHyprCtl([[maybe_unused]] eHyprCtlOutputFormat format, const std::string args) {
    return hyprstack::handleQueryCommand(hyprstack::splitArgs(args));
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
