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
