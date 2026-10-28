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
    REQUIRE(snapshot.workspace->id == hyprstack::WorkspaceId{1});
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

TEST_CASE("Named and special workspaces keep independent stacks across swaps and moves") {
    hyprstack::StackStore store;
    hyprstack::StateSyncService service;
    const hyprstack::WorkspaceId named = std::string{"notes"};
    const hyprstack::WorkspaceId special = std::string{"special:notes"};
    const hyprstack::WorkspaceId large = int64_t{4294967295};
    hyprstack::ObservedSnapshot observed{
        .windows = {
            {.workspaceId = named, .workspaceName = "Notes", .address = "0x1", .className = "terminal", .title = "one", .focused = true, .historyIndex = 1},
            {.workspaceId = named, .workspaceName = "Notes", .address = "0x2", .className = "terminal", .title = "two", .historyIndex = 0},
            {.workspaceId = special, .workspaceName = "Notes", .address = "0x3", .className = "terminal", .title = "three", .historyIndex = 0},
        },
        .workspaces = {{.id = named, .name = "Notes"}, {.id = special, .name = "Notes"}, {.id = large, .name = "Large"}},
        .activeWorkspaceId = named,
    };
    service.sync(store, observed);
    REQUIRE(store.swapCurrentWithNext());
    service.sync(store, observed);
    REQUIRE(store.snapshotForWorkspace(named).stack.windows()[0].address == "0x2");
    REQUIRE(store.snapshotForWorkspace(special).stack.windows().size() == 1);
    REQUIRE(store.snapshotForWorkspace(large).workspace->id == large);

    observed.windows[0].workspaceId = special;
    observed.activeWorkspaceId = special;
    service.sync(store, observed);
    REQUIRE(store.snapshotForWorkspace(named).stack.windows().size() == 1);
    REQUIRE(store.snapshotForWorkspace().stack.windows().size() == 2);
    REQUIRE(store.snapshotForWorkspace().stack.current() == "0x1");
}
