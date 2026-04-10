#include "hyprstack/query_command.hpp"

#include "catch_amalgamated.hpp"

TEST_CASE("splitArgs splits space separated tokens") {
    const auto args = hyprstack::splitArgs("hyprstack stack list");

    REQUIRE(args == std::vector<std::string>{"hyprstack", "stack", "list"});
}

TEST_CASE("handleQueryCommand accepts request with plugin prefix") {
    const auto response = hyprstack::handleQueryCommand({"hyprstack", "stack", "list"});

    REQUIRE(response == hyprstack::renderEmptyStackList());
}

TEST_CASE("handleQueryCommand accepts request without plugin prefix") {
    const auto response = hyprstack::handleQueryCommand({"stack", "current"});

    REQUIRE(response == hyprstack::renderEmptyCurrent());
}

TEST_CASE("handleQueryCommand returns around response") {
    const auto response = hyprstack::handleQueryCommand({"hyprstack", "stack", "around"});

    REQUIRE(response == hyprstack::renderEmptyAround());
}

TEST_CASE("handleQueryCommand returns help for incomplete request") {
    const auto response = hyprstack::handleQueryCommand({"hyprstack"});

    REQUIRE(response == hyprstack::renderHelp());
}

TEST_CASE("handleQueryCommand returns escaped error for unknown subcommand") {
    const auto response = hyprstack::handleQueryCommand({"hyprstack", "stack", "no\"pe"});

    REQUIRE(response == "unknown hyprstack subcommand: no\\\"pe");
}

TEST_CASE("handleQueryCommand renders snapshot-backed list response") {
    hyprstack::QuerySnapshot snapshot;
    snapshot.workspace = hyprstack::WorkspaceRef{.id = 3, .name = "code"};
    snapshot.stack.focusWindow({
        .address   = "0xa",
        .className = "ghostty",
        .title     = "editor",
    });

    const auto response = hyprstack::handleQueryCommand(snapshot, {"hyprstack", "stack", "list"});

    REQUIRE(response == R"json({
  "workspace": {
    "id": 3,
    "name": "code"
  },
  "current": "0xa",
  "last": null,
  "windows": [
    {
      "index": 0,
      "address": "0xa",
      "class": "ghostty",
      "title": "editor"
    }
  ]
})json");
}

TEST_CASE("renderStackList renders workspace and windows") {
    hyprstack::WorkspaceStack stack;
    stack.focusWindow({
        .address   = "0x1",
        .className = "ghostty",
        .title     = "tmux",
    });
    stack.addWindow({
        .address   = "0x2",
        .className = "thunar",
        .title     = "home",
    });

    const auto response = hyprstack::renderStackList(hyprstack::WorkspaceRef{.id = 1, .name = "1"}, stack);

    REQUIRE(response == R"json({
  "workspace": {
    "id": 1,
    "name": "1"
  },
  "current": "0x1",
  "last": null,
  "windows": [
    {
      "index": 0,
      "address": "0x1",
      "class": "ghostty",
      "title": "tmux"
    },
    {
      "index": 1,
      "address": "0x2",
      "class": "thunar",
      "title": "home"
    }
  ]
})json");
}

TEST_CASE("renderAround renders current stable neighbors") {
    hyprstack::WorkspaceStack stack;
    stack.addWindow({.address = "0x1", .className = "a", .title = "a"});
    stack.addWindow({.address = "0x2", .className = "b", .title = "b"});
    stack.addWindow({.address = "0x3", .className = "c", .title = "c"});
    stack.focusWindow({.address = "0x2", .className = "b", .title = "b"});
    stack.focusWindow({.address = "0x3", .className = "c", .title = "c"});
    stack.focusWindow({.address = "0x2", .className = "b", .title = "b"});

    const auto response = hyprstack::renderAround(hyprstack::WorkspaceRef{.id = 2, .name = "web"}, stack);

    REQUIRE(response == R"json({
  "workspace": {
    "id": 2,
    "name": "web"
  },
  "current": "0x2",
  "prev": "0x1",
  "next": "0x3",
  "last": "0x3"
})json");
}
