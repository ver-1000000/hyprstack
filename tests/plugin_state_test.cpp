#include "hyprstack/plugin_state.hpp"

#include "catch_amalgamated.hpp"

TEST_CASE("PluginState builds active workspace snapshot from observed windows") {
    hyprstack::PluginState state;

    state.sync(
        {
            {.workspaceId = 1, .workspaceName = "1", .address = "0x1", .className = "ghostty", .title = "one", .focused = false, .historyIndex = 0},
            {.workspaceId = 1, .workspaceName = "1", .address = "0x2", .className = "thunar", .title = "two", .focused = true, .historyIndex = 1},
            {.workspaceId = 2, .workspaceName = "2", .address = "0x3", .className = "firefox", .title = "three", .focused = false, .historyIndex = 2},
        },
        {
            {.id = 1, .name = "1"},
            {.id = 2, .name = "2"},
        },
        1
    );

    const auto snapshot = state.snapshotForWorkspace();

    REQUIRE(snapshot.workspace.has_value());
    REQUIRE(snapshot.workspace->id == 1);
    REQUIRE(snapshot.stack.windows().size() == 2);
    REQUIRE(snapshot.stack.current() == std::optional<std::string>{"0x2"});
    REQUIRE(snapshot.stack.last() == std::optional<std::string>{"0x1"});
}

TEST_CASE("PluginState preserves stable order across syncs and removes missing windows") {
    hyprstack::PluginState state;

    state.sync(
        {
            {.workspaceId = 1, .workspaceName = "1", .address = "0x1", .className = "ghostty", .title = "one", .focused = true, .historyIndex = 1},
            {.workspaceId = 1, .workspaceName = "1", .address = "0x2", .className = "thunar", .title = "two", .focused = false, .historyIndex = 0},
        },
        {
            {.id = 1, .name = "1"},
        },
        1
    );

    state.sync(
        {
            {.workspaceId = 1, .workspaceName = "main", .address = "0x2", .className = "thunar", .title = "two updated", .focused = false, .historyIndex = 1},
            {.workspaceId = 1, .workspaceName = "main", .address = "0x3", .className = "firefox", .title = "three", .focused = true, .historyIndex = 2},
        },
        {
            {.id = 1, .name = "main"},
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
    REQUIRE(snapshot.stack.last() == std::optional<std::string>{"0x2"});
}

TEST_CASE("PluginState moves a window between workspaces") {
    hyprstack::PluginState state;

    state.sync(
        {
            {.workspaceId = 1, .workspaceName = "1", .address = "0x1", .className = "ghostty", .title = "one", .focused = true, .historyIndex = 1},
            {.workspaceId = 1, .workspaceName = "1", .address = "0x2", .className = "thunar", .title = "two", .focused = false, .historyIndex = 0},
        },
        {
            {.id = 1, .name = "1"},
        },
        1
    );

    state.sync(
        {
            {.workspaceId = 1, .workspaceName = "1", .address = "0x1", .className = "ghostty", .title = "one", .focused = true, .historyIndex = 1},
            {.workspaceId = 2, .workspaceName = "2", .address = "0x2", .className = "thunar", .title = "two", .focused = false, .historyIndex = 0},
        },
        {
            {.id = 1, .name = "1"},
            {.id = 2, .name = "2"},
        },
        1
    );

    const auto firstWorkspace = state.snapshotForWorkspace(1);
    const auto secondWorkspace = state.snapshotForWorkspace(2);

    REQUIRE(firstWorkspace.workspace.has_value());
    REQUIRE(firstWorkspace.stack.windows().size() == 1);
    REQUIRE(firstWorkspace.stack.windows()[0].address == "0x1");

    REQUIRE(secondWorkspace.workspace.has_value());
    REQUIRE(secondWorkspace.stack.windows().size() == 1);
    REQUIRE(secondWorkspace.stack.windows()[0].address == "0x2");
}

TEST_CASE("PluginState falls back to explicit workspace when active workspace is absent") {
    hyprstack::PluginState state;

    state.sync(
        {
            {.workspaceId = 7, .workspaceName = "scratch", .address = "0xaa", .className = "ghostty", .title = "shell", .focused = false, .historyIndex = std::nullopt},
        },
        {
            {.id = 7, .name = "scratch"},
        },
        std::nullopt
    );

    const auto activeSnapshot = state.snapshotForWorkspace();
    const auto explicitSnapshot = state.snapshotForWorkspace(7);

    REQUIRE_FALSE(activeSnapshot.workspace.has_value());
    REQUIRE(explicitSnapshot.workspace.has_value());
    REQUIRE(explicitSnapshot.workspace->id == 7);
    REQUIRE(explicitSnapshot.stack.windows().size() == 1);
}

TEST_CASE("PluginState updates workspace name without losing stack order") {
    hyprstack::PluginState state;

    state.sync(
        {
            {.workspaceId = 5, .workspaceName = "old", .address = "0x1", .className = "ghostty", .title = "one", .focused = true, .historyIndex = 0},
            {.workspaceId = 5, .workspaceName = "old", .address = "0x2", .className = "thunar", .title = "two", .focused = false, .historyIndex = std::nullopt},
        },
        {
            {.id = 5, .name = "old"},
        },
        5
    );

    state.sync(
        {
            {.workspaceId = 5, .workspaceName = "new", .address = "0x1", .className = "ghostty", .title = "one", .focused = true, .historyIndex = 0},
            {.workspaceId = 5, .workspaceName = "new", .address = "0x2", .className = "thunar", .title = "two", .focused = false, .historyIndex = std::nullopt},
        },
        {
            {.id = 5, .name = "new"},
        },
        5
    );

    const auto snapshot = state.snapshotForWorkspace(5);

    REQUIRE(snapshot.workspace.has_value());
    REQUIRE(snapshot.workspace->name == "new");
    REQUIRE(snapshot.stack.windows().size() == 2);
    REQUIRE(snapshot.stack.windows()[0].address == "0x1");
    REQUIRE(snapshot.stack.windows()[1].address == "0x2");
}

TEST_CASE("PluginState updates window metadata in place") {
    hyprstack::PluginState state;

    state.sync(
        {
            {.workspaceId = 3, .workspaceName = "3", .address = "0xaa", .className = "ghostty", .title = "old title", .focused = true, .historyIndex = 0},
        },
        {
            {.id = 3, .name = "3"},
        },
        3
    );

    state.sync(
        {
            {.workspaceId = 3, .workspaceName = "3", .address = "0xaa", .className = "ghostty", .title = "new title", .focused = true, .historyIndex = 0},
        },
        {
            {.id = 3, .name = "3"},
        },
        3
    );

    const auto snapshot = state.snapshotForWorkspace(3);

    REQUIRE(snapshot.stack.windows().size() == 1);
    REQUIRE(snapshot.stack.windows()[0].title == "new title");
}

TEST_CASE("PluginState keeps empty active workspace metadata") {
    hyprstack::PluginState state;

    state.sync({}, {{.id = 9, .name = "9"}}, 9);

    const auto snapshot = state.snapshotForWorkspace();

    REQUIRE(snapshot.workspace.has_value());
    REQUIRE(snapshot.workspace->id == 9);
    REQUIRE(snapshot.workspace->name == "9");
    REQUIRE(snapshot.stack.windows().empty());
    REQUIRE_FALSE(snapshot.stack.current().has_value());
    REQUIRE_FALSE(snapshot.stack.last().has_value());
}

TEST_CASE("PluginState clears last when focused window closes and previous window regains focus") {
    hyprstack::PluginState state;

    state.sync(
        {
            {.workspaceId = 1, .workspaceName = "1", .address = "0x1", .className = "ghostty", .title = "one", .focused = false, .historyIndex = 0},
            {.workspaceId = 1, .workspaceName = "1", .address = "0x2", .className = "thunar", .title = "two", .focused = true, .historyIndex = 1},
        },
        {
            {.id = 1, .name = "1"},
        },
        1
    );

    state.sync(
        {
            {.workspaceId = 1, .workspaceName = "1", .address = "0x1", .className = "ghostty", .title = "one", .focused = true, .historyIndex = 0},
        },
        {
            {.id = 1, .name = "1"},
        },
        1
    );

    const auto snapshot = state.snapshotForWorkspace(1);

    REQUIRE(snapshot.stack.windows().size() == 1);
    REQUIRE(snapshot.stack.current() == std::optional<std::string>{"0x1"});
    REQUIRE_FALSE(snapshot.stack.last().has_value());
}

TEST_CASE("PluginState updates current and last when focused window moves to another workspace") {
    hyprstack::PluginState state;

    state.sync(
        {
            {.workspaceId = 1, .workspaceName = "1", .address = "0x1", .className = "ghostty", .title = "one", .focused = false, .historyIndex = 0},
            {.workspaceId = 1, .workspaceName = "1", .address = "0x2", .className = "thunar", .title = "two", .focused = true, .historyIndex = 1},
        },
        {
            {.id = 1, .name = "1"},
            {.id = 2, .name = "2"},
        },
        1
    );

    state.sync(
        {
            {.workspaceId = 1, .workspaceName = "1", .address = "0x1", .className = "ghostty", .title = "one", .focused = false, .historyIndex = 0},
            {.workspaceId = 2, .workspaceName = "2", .address = "0x2", .className = "thunar", .title = "two", .focused = true, .historyIndex = 0},
        },
        {
            {.id = 1, .name = "1"},
            {.id = 2, .name = "2"},
        },
        2
    );

    const auto firstWorkspace  = state.snapshotForWorkspace(1);
    const auto secondWorkspace = state.snapshotForWorkspace(2);

    REQUIRE(firstWorkspace.stack.windows().size() == 1);
    REQUIRE(firstWorkspace.stack.current() == std::optional<std::string>{"0x1"});
    REQUIRE_FALSE(firstWorkspace.stack.last().has_value());

    REQUIRE(secondWorkspace.stack.windows().size() == 1);
    REQUIRE(secondWorkspace.stack.current() == std::optional<std::string>{"0x2"});
    REQUIRE_FALSE(secondWorkspace.stack.last().has_value());
}

TEST_CASE("PluginState preserves swapped stable order across syncs") {
    hyprstack::PluginState state;

    state.sync(
        {
            {.workspaceId = 1, .workspaceName = "1", .address = "0x1", .className = "ghostty", .title = "one", .focused = true, .historyIndex = 1},
            {.workspaceId = 1, .workspaceName = "1", .address = "0x2", .className = "thunar", .title = "two", .focused = false, .historyIndex = 0},
        },
        {
            {.id = 1, .name = "1"},
        },
        1
    );

    REQUIRE(state.swapCurrentWithNext());

    state.sync(
        {
            {.workspaceId = 1, .workspaceName = "1", .address = "0x1", .className = "ghostty", .title = "one", .focused = true, .historyIndex = 1},
            {.workspaceId = 1, .workspaceName = "1", .address = "0x2", .className = "thunar", .title = "two", .focused = false, .historyIndex = 0},
        },
        {
            {.id = 1, .name = "1"},
        },
        1
    );

    const auto snapshot = state.snapshotForWorkspace(1);

    REQUIRE(snapshot.stack.windows().size() == 2);
    REQUIRE(snapshot.stack.windows()[0].address == "0x2");
    REQUIRE(snapshot.stack.windows()[1].address == "0x1");
    REQUIRE(snapshot.stack.current() == std::optional<std::string>{"0x1"});
}
