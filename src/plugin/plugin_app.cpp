#include "hyprstack/plugin/plugin_app.hpp"

#include "hyprstack/application/dispatch_service.hpp"
#include "hyprstack/application/query_service.hpp"
#include "hyprstack/interface/query_parser.hpp"

namespace hyprstack {

void PluginApp::bindEvents() {
    m_eventBridge.bind(m_dirty);
}

void PluginApp::reset() {
    m_eventBridge.reset();
    m_store = {};
    m_dirty = true;
}

std::string PluginApp::handleHyprCtl(const std::string& args) {
    const auto action = splitArgs(args);
    size_t     offset = 0;

    if (!action.empty() && action[0] == "hyprstack")
        offset = 1;

    if (action.size() == offset + 2 && action[offset] == "focus") {
        const auto result = handleStackFocus(action[offset + 1]);
        return result.success ? "ok" : "error: " + result.error;
    }

    if (action.size() == offset + 2 && action[offset] == "swap") {
        const auto result = handleStackSwap(action[offset + 1]);
        return result.success ? "ok" : "error: " + result.error;
    }

    return handleQueryCommand(currentSnapshot(), action);
}

SDispatchResult PluginApp::handleStackFocus(const std::string& args) {
    const auto action = splitArgs(args);

    if (action.size() != 1)
        return makeDispatchError("usage: stackfocus <next|prev|last>");

    const auto resolution = resolveStackFocusTarget(currentSnapshot().stack, action[0]);

    if (!resolution.address)
        return makeDispatchError(std::move(resolution.error));

    return m_executor.focusWindowAddress(*resolution.address);
}

SDispatchResult PluginApp::handleStackSwap(const std::string& args) {
    const auto action = splitArgs(args);

    if (action.size() != 1)
        return makeDispatchError("usage: stackswap <next|prev>");

    const auto direction = parseStackSwapDirection(action[0]);

    if (!direction)
        return makeDispatchError("unknown stackswap subcommand: " + action[0]);

    const auto snapshot   = currentSnapshot();
    const auto resolution = resolveStackSwapTarget(snapshot.stack, action[0]);

    if (!resolution.address)
        return makeDispatchError(std::move(resolution.error));

    return m_executor.executeStackSwap(*resolution.address, snapshot.workspace.transform([](const WorkspaceRef& workspace) { return workspace.id; }), m_store, *direction);
}

void PluginApp::ensureStateSynced() {
    if (!m_dirty)
        return;

    m_syncService.sync(m_store, m_observer.snapshot());
    m_dirty = false;
}

QuerySnapshot PluginApp::currentSnapshot() {
    ensureStateSynced();
    return m_store.snapshotForWorkspace();
}

} // namespace hyprstack
