#include <hyprland/src/desktop/state/FocusState.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/event/EventBus.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>

#include "hyprstack/hyprland_executor.hpp"
#include "hyprstack/hyprland_observer.hpp"
#include "hyprstack/plugin_state.hpp"
#include "hyprstack/query_command.hpp"
#include "hyprstack/stack_dispatcher.hpp"

#include <stdexcept>
#include <string>

inline HANDLE               G_HANDLE          = nullptr;
inline SP<SHyprCtlCommand>  G_HYPRCTL_COMMAND = nullptr;
inline hyprstack::PluginState G_STATE;
inline bool                 G_STATE_DIRTY     = true;
inline bool                 G_STACKFOCUS_REGISTERED = false;
inline bool                 G_STACKSWAP_REGISTERED  = false;

struct SEventListeners {
    CHyprSignalListener windowOpen;
    CHyprSignalListener windowClose;
    CHyprSignalListener windowDestroy;
    CHyprSignalListener windowActive;
    CHyprSignalListener windowTitle;
    CHyprSignalListener windowClass;
    CHyprSignalListener windowMoveToWorkspace;
    CHyprSignalListener workspaceActive;
};

inline SEventListeners G_LISTENERS;

namespace {

void registerEventListeners() {
    auto& events = Event::bus()->m_events;

    G_LISTENERS.windowOpen            = events.window.open.listen([](PHLWINDOW) { hyprstack::markStateDirty(G_STATE_DIRTY); });
    G_LISTENERS.windowClose           = events.window.close.listen([](PHLWINDOW) { hyprstack::markStateDirty(G_STATE_DIRTY); });
    G_LISTENERS.windowDestroy         = events.window.destroy.listen([](PHLWINDOW) { hyprstack::markStateDirty(G_STATE_DIRTY); });
    G_LISTENERS.windowActive          = events.window.active.listen([](PHLWINDOW, Desktop::eFocusReason) { hyprstack::markStateDirty(G_STATE_DIRTY); });
    G_LISTENERS.windowTitle           = events.window.title.listen([](PHLWINDOW) { hyprstack::markStateDirty(G_STATE_DIRTY); });
    G_LISTENERS.windowClass           = events.window.class_.listen([](PHLWINDOW) { hyprstack::markStateDirty(G_STATE_DIRTY); });
    G_LISTENERS.windowMoveToWorkspace = events.window.moveToWorkspace.listen([](PHLWINDOW, PHLWORKSPACE) { hyprstack::markStateDirty(G_STATE_DIRTY); });
    G_LISTENERS.workspaceActive       = events.workspace.active.listen([](PHLWORKSPACE) { hyprstack::markStateDirty(G_STATE_DIRTY); });
}

std::string onHyprCtl([[maybe_unused]] eHyprCtlOutputFormat format, const std::string args) {
    return hyprstack::handleQueryCommand(hyprstack::currentSnapshot(G_STATE, G_STATE_DIRTY), hyprstack::splitArgs(args));
}

SDispatchResult onStackFocus(const std::string& args) {
    const auto resolution = hyprstack::resolveStackFocusTarget(hyprstack::currentSnapshot(G_STATE, G_STATE_DIRTY).stack, args);

    if (!resolution.address)
        return hyprstack::makeDispatchError(std::move(resolution.error));

    return hyprstack::focusWindowAddress(*resolution.address);
}

SDispatchResult onStackSwap(const std::string& args) {
    hyprstack::ensureStateSynced(G_STATE, G_STATE_DIRTY);

    const auto action = hyprstack::splitArgs(args);

    if (action.size() != 1)
        return hyprstack::makeDispatchError("usage: stackswap <next|prev>");

    const auto swapAction = hyprstack::resolveSwapAction(action[0]);

    if (!swapAction)
        return hyprstack::makeDispatchError("unknown stackswap subcommand: " + action[0]);

    const auto snapshot   = hyprstack::currentSnapshot(G_STATE, G_STATE_DIRTY);
    const auto resolution = hyprstack::resolveStackSwapTarget(snapshot.stack, args);

    if (!resolution.address)
        return hyprstack::makeDispatchError(std::move(resolution.error));

    return hyprstack::executeStackSwap(
        *resolution.address, hyprstack::snapshotWorkspaceId(snapshot), G_STATE, swapAction->operation, swapAction->failureMessage
    );
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

    G_STACKFOCUS_REGISTERED = HyprlandAPI::addDispatcherV2(G_HANDLE, "stackfocus", onStackFocus);

    if (!G_STACKFOCUS_REGISTERED)
        throw std::runtime_error("[hyprstack] Failed to register stackfocus dispatcher");

    G_STACKSWAP_REGISTERED = HyprlandAPI::addDispatcherV2(G_HANDLE, "stackswap", onStackSwap);

    if (!G_STACKSWAP_REGISTERED)
        throw std::runtime_error("[hyprstack] Failed to register stackswap dispatcher");

    registerEventListeners();

    return {
        .name        = "hyprstack",
        .description = "Workspace-local stable window stack semantics for Hyprland",
        .author      = "ver-1000000",
        .version     = "0.4.0",
    };
}

APICALL EXPORT void PLUGIN_EXIT() {
    G_LISTENERS = {};

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

    G_STATE       = {};
    G_STATE_DIRTY = true;
    G_HANDLE = nullptr;
}
