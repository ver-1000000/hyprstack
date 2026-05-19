#include "hyprstack/infra/hyprland/executor.hpp"
#include "hyprstack/plugin/plugin_app.hpp"

#include <hyprland/src/plugins/PluginAPI.hpp>

#include <memory>
#include <stdexcept>
#include <string>

inline HANDLE                         G_HANDLE                = nullptr;
inline SP<SHyprCtlCommand>            G_HYPRCTL_COMMAND       = nullptr;
inline std::unique_ptr<hyprstack::PluginApp> G_APP           = nullptr;
inline bool                           G_STACKFOCUS_REGISTERED = false;
inline bool                           G_STACKSWAP_REGISTERED  = false;

namespace {

std::string onHyprCtl([[maybe_unused]] const eHyprCtlOutputFormat format, const std::string args) {
    if (!G_APP)
        return "hyprstack unavailable";

    return G_APP->handleHyprCtl(args);
}

SDispatchResult onStackFocus(const std::string& args) {
    if (!G_APP)
        return hyprstack::makeDispatchError("plugin app is unavailable");

    return G_APP->handleStackFocus(args);
}

SDispatchResult onStackSwap(const std::string& args) {
    if (!G_APP)
        return hyprstack::makeDispatchError("plugin app is unavailable");

    return G_APP->handleStackSwap(args);
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

    G_APP = std::make_unique<hyprstack::PluginApp>();

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

    G_STACKFOCUS_REGISTERED = HyprlandAPI::addDispatcherV2(G_HANDLE, "stackfocus", onStackFocus);

    if (!G_STACKFOCUS_REGISTERED)
        throw std::runtime_error("[hyprstack] Failed to register stackfocus dispatcher");

    G_STACKSWAP_REGISTERED = HyprlandAPI::addDispatcherV2(G_HANDLE, "stackswap", onStackSwap);

    if (!G_STACKSWAP_REGISTERED)
        throw std::runtime_error("[hyprstack] Failed to register stackswap dispatcher");

    G_APP->bindEvents();

    return {
        .name        = "hyprstack",
        .description = "Workspace-local stable window stack semantics for Hyprland",
        .author      = "ver-1000000",
        .version     = "0.55.2.1",
    };
}

APICALL EXPORT void PLUGIN_EXIT() {
    if (G_APP)
        G_APP->reset();

    if (G_HANDLE && G_STACKFOCUS_REGISTERED) {
        HyprlandAPI::removeDispatcher(G_HANDLE, "stackfocus");
        G_STACKFOCUS_REGISTERED = false;
    }

    if (G_HANDLE && G_STACKSWAP_REGISTERED) {
        HyprlandAPI::removeDispatcher(G_HANDLE, "stackswap");
        G_STACKSWAP_REGISTERED = false;
    }

    if (G_HANDLE && G_HYPRCTL_COMMAND) {
        HyprlandAPI::unregisterHyprCtlCommand(G_HANDLE, G_HYPRCTL_COMMAND);
        G_HYPRCTL_COMMAND.reset();
    }

    G_APP.reset();
    G_HANDLE = nullptr;
}
