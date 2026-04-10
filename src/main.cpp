#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/history/WindowHistoryTracker.hpp>
#include <hyprland/src/desktop/state/FocusState.hpp>
#include <hyprland/src/event/EventBus.hpp>
#include <hyprland/src/helpers/Monitor.hpp>
#include <hyprland/src/managers/KeybindManager.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>

#include "hyprstack/plugin_state.hpp"
#include "hyprstack/query_command.hpp"
#include "hyprstack/stack_dispatcher.hpp"

#include <format>
#include <stdexcept>
#include <string>
#include <vector>

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

void markStateDirty() {
    G_STATE_DIRTY = true;
}

std::string formatAddress(const PHLWINDOW& window) {
    return std::format("0x{:x}", reinterpret_cast<uintptr_t>(window.get()));
}

std::optional<int> activeWorkspaceId() {
    if (const auto focusedWindow = Desktop::focusState()->window(); focusedWindow && valid(focusedWindow->m_workspace))
        return static_cast<int>(focusedWindow->m_workspace->m_id);

    for (const auto& monitor : g_pCompositor->m_monitors) {
        if (monitor && valid(monitor->m_activeWorkspace))
            return static_cast<int>(monitor->m_activeWorkspace->m_id);
    }

    return std::nullopt;
}

std::vector<hyprstack::ObservedWindow> observeWindows() {
    std::vector<hyprstack::ObservedWindow> observed;
    const auto focusedWindow = Desktop::focusState()->window();
    const auto history       = Desktop::History::windowTracker()->fullHistory();

    for (const auto& window : g_pCompositor->m_windows) {
        if (!window || !window->m_isMapped || !valid(window->m_workspace))
            continue;

        std::optional<size_t> historyIndex;
        for (size_t i = 0; i < history.size(); ++i) {
            if (const auto historyWindow = history[i].lock(); historyWindow && historyWindow == window) {
                historyIndex = i;
                break;
            }
        }

        observed.push_back({
            .workspaceId   = static_cast<int>(window->m_workspace->m_id),
            .workspaceName = window->m_workspace->m_name,
            .address       = formatAddress(window),
            .className     = window->m_class,
            .title         = window->m_title,
            .focused       = focusedWindow && window == focusedWindow,
            .historyIndex  = historyIndex,
        });
    }

    return observed;
}

std::vector<hyprstack::ObservedWorkspace> observeWorkspaces() {
    std::vector<hyprstack::ObservedWorkspace> observed;

    for (const auto& workspaceRef : g_pCompositor->getWorkspaces()) {
        const auto workspace = workspaceRef.lock();

        if (!valid(workspace))
            continue;

        observed.push_back({
            .id   = static_cast<int>(workspace->m_id),
            .name = workspace->m_name,
        });
    }

    return observed;
}

hyprstack::QuerySnapshot currentSnapshot() {
    if (G_STATE_DIRTY) {
        G_STATE.sync(observeWindows(), observeWorkspaces(), activeWorkspaceId());
        G_STATE_DIRTY = false;
    }

    return G_STATE.snapshotForWorkspace();
}

void registerEventListeners() {
    auto& events = Event::bus()->m_events;

    G_LISTENERS.windowOpen            = events.window.open.listen([](PHLWINDOW) { markStateDirty(); });
    G_LISTENERS.windowClose           = events.window.close.listen([](PHLWINDOW) { markStateDirty(); });
    G_LISTENERS.windowDestroy         = events.window.destroy.listen([](PHLWINDOW) { markStateDirty(); });
    G_LISTENERS.windowActive          = events.window.active.listen([](PHLWINDOW, Desktop::eFocusReason) { markStateDirty(); });
    G_LISTENERS.windowTitle           = events.window.title.listen([](PHLWINDOW) { markStateDirty(); });
    G_LISTENERS.windowClass           = events.window.class_.listen([](PHLWINDOW) { markStateDirty(); });
    G_LISTENERS.windowMoveToWorkspace = events.window.moveToWorkspace.listen([](PHLWINDOW, PHLWORKSPACE) { markStateDirty(); });
    G_LISTENERS.workspaceActive       = events.workspace.active.listen([](PHLWORKSPACE) { markStateDirty(); });
}

std::string onHyprCtl([[maybe_unused]] eHyprCtlOutputFormat format, const std::string args) {
    return hyprstack::handleQueryCommand(currentSnapshot(), hyprstack::splitArgs(args));
}

SDispatchResult onStackFocus(const std::string& args) {
    const auto resolution = hyprstack::resolveStackFocusTarget(currentSnapshot().stack, args);

    if (!resolution.address)
        return {
            .success = false,
            .error   = resolution.error,
        };

    if (!g_pKeybindManager)
        return {
            .success = false,
            .error   = "hypr keybind manager is unavailable",
        };

    const auto dispatcher = g_pKeybindManager->m_dispatchers.find("focuswindow");

    if (dispatcher == g_pKeybindManager->m_dispatchers.end())
        return {
            .success = false,
            .error   = "focuswindow dispatcher is unavailable",
        };

    return dispatcher->second("address:" + *resolution.address);
}

SDispatchResult onStackSwap(const std::string& args) {
    const auto action = hyprstack::splitArgs(args);

    if (action.size() != 1)
        return {
            .success = false,
            .error   = "usage: stackswap <next|prev>",
        };

    if (action[0] == "next") {
        if (!G_STATE.swapCurrentWithNext()) {
            return {
                .success = false,
                .error   = "stackswap next requires a focused window and at least two windows",
            };
        }

        return {};
    }

    if (action[0] == "prev") {
        if (!G_STATE.swapCurrentWithPrev()) {
            return {
                .success = false,
                .error   = "stackswap prev requires a focused window and at least two windows",
            };
        }

        return {};
    }

    return {
        .success = false,
        .error   = "unknown stackswap subcommand: " + action[0],
    };
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

    HyprlandAPI::addNotification(
        G_HANDLE, "[hyprstack] Loaded Query API v0", CHyprColor{0.2, 0.8, 1.0, 1.0}, 3000
    );

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
