#include "hyprstack/application/state_sync.hpp"

#include "catch_amalgamated.hpp"

TEST_CASE("StateSyncService builds active workspace snapshots in StackStore") {
    hyprstack::StackStore       store;
    hyprstack::StateSyncService service;

    service.sync(
        store,
        {
            .windows =
                {
                    {.workspaceId = 1, .workspaceName = "1", .address = "0x1", .className = "ghostty", .title = "one", .focused = false, .historyIndex = 0},
                    {.workspaceId = 1, .workspaceName = "1", .address = "0x2", .className = "thunar", .title = "two", .focused = true, .historyIndex = 1},
                },
            .workspaces        = {{.id = 1, .name = "1"}},
            .activeWorkspaceId = 1,
        }
    );

    const auto snapshot = store.snapshotForWorkspace();

    REQUIRE(snapshot.workspace.has_value());
    REQUIRE(snapshot.workspace->id == 1);
    REQUIRE(snapshot.stack.windows().size() == 2);
    REQUIRE(snapshot.stack.current() == std::optional<std::string>{"0x2"});
    REQUIRE(snapshot.stack.last() == std::optional<std::string>{"0x1"});
}

TEST_CASE("StateSyncService preserves swapped order across syncs") {
    hyprstack::StackStore       store;
    hyprstack::StateSyncService service;

    const hyprstack::ObservedSnapshot snapshot{
        .windows =
            {
                {.workspaceId = 1, .workspaceName = "1", .address = "0x1", .className = "ghostty", .title = "one", .focused = true, .historyIndex = 1},
                {.workspaceId = 1, .workspaceName = "1", .address = "0x2", .className = "thunar", .title = "two", .focused = false, .historyIndex = 0},
            },
        .workspaces        = {{.id = 1, .name = "1"}},
        .activeWorkspaceId = 1,
    };

    service.sync(store, snapshot);

    REQUIRE(store.swapCurrentWithNext());

    service.sync(store, snapshot);

    const auto updated = store.snapshotForWorkspace(1);

    REQUIRE(updated.stack.windows().size() == 2);
    REQUIRE(updated.stack.windows()[0].address == "0x2");
    REQUIRE(updated.stack.windows()[1].address == "0x1");
    REQUIRE(updated.stack.current() == std::optional<std::string>{"0x1"});
}
