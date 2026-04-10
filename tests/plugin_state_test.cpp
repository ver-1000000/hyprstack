#include "hyprstack/plugin_state.hpp"

#include "catch_amalgamated.hpp"

TEST_CASE("PluginState builds active workspace snapshot from observed windows") {
    hyprstack::PluginState state;

    state.sync(
        {
            {.workspaceId = 1, .workspaceName = "1", .address = "0x1", .className = "ghostty", .title = "one", .focused = false},
            {.workspaceId = 1, .workspaceName = "1", .address = "0x2", .className = "thunar", .title = "two", .focused = true},
            {.workspaceId = 2, .workspaceName = "2", .address = "0x3", .className = "firefox", .title = "three", .focused = false},
        },
        1
    );

    const auto snapshot = state.snapshotForWorkspace();

    REQUIRE(snapshot.workspace.has_value());
    REQUIRE(snapshot.workspace->id == 1);
    REQUIRE(snapshot.stack.windows().size() == 2);
    REQUIRE(snapshot.stack.current() == std::optional<std::string>{"0x2"});
}

TEST_CASE("PluginState preserves stable order across syncs and removes missing windows") {
    hyprstack::PluginState state;

    state.sync(
        {
            {.workspaceId = 1, .workspaceName = "1", .address = "0x1", .className = "ghostty", .title = "one", .focused = true},
            {.workspaceId = 1, .workspaceName = "1", .address = "0x2", .className = "thunar", .title = "two", .focused = false},
        },
        1
    );

    state.sync(
        {
            {.workspaceId = 1, .workspaceName = "main", .address = "0x2", .className = "thunar", .title = "two updated", .focused = false},
            {.workspaceId = 1, .workspaceName = "main", .address = "0x3", .className = "firefox", .title = "three", .focused = true},
        },
        1
    );

    const auto snapshot = state.snapshotForWorkspace(1);

    REQUIRE(snapshot.workspace.has_value());
    REQUIRE(snapshot.workspace->name == "main");
    REQUIRE(snapshot.stack.windows().size() == 2);
    REQUIRE(snapshot.stack.windows()[0].address == "0x2");
    REQUIRE(snapshot.stack.windows()[1].address == "0x3");
    REQUIRE(snapshot.stack.windows()[0].title == "two updated");
    REQUIRE(snapshot.stack.current() == std::optional<std::string>{"0x3"});
    REQUIRE_FALSE(snapshot.stack.last().has_value());
}
