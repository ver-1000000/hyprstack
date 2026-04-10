#include "hyprstack/stack_dispatcher.hpp"

#include "catch_amalgamated.hpp"

namespace {

hyprstack::StackWindow makeWindow(std::string address) {
    return {
        .address   = std::move(address),
        .className = "test.app",
        .title     = "title",
    };
}

} // namespace

TEST_CASE("resolveStackFocusTarget resolves last focused window") {
    hyprstack::WorkspaceStack stack;

    stack.focusWindow(makeWindow("0x1"));
    stack.focusWindow(makeWindow("0x2"));

    const auto resolution = hyprstack::resolveStackFocusTarget(stack, "last");

    REQUIRE(resolution.address == std::optional<std::string>{"0x1"});
    REQUIRE(resolution.error.empty());
}

TEST_CASE("resolveStackFocusTarget cycles next across stable order") {
    hyprstack::WorkspaceStack stack;

    stack.addWindow(makeWindow("0x1"));
    stack.addWindow(makeWindow("0x2"));
    stack.addWindow(makeWindow("0x3"));
    stack.focusWindow(makeWindow("0x3"));

    const auto resolution = hyprstack::resolveStackFocusTarget(stack, "next");

    REQUIRE(resolution.address == std::optional<std::string>{"0x1"});
}

TEST_CASE("resolveStackFocusTarget cycles prev across stable order") {
    hyprstack::WorkspaceStack stack;

    stack.addWindow(makeWindow("0x1"));
    stack.addWindow(makeWindow("0x2"));
    stack.addWindow(makeWindow("0x3"));
    stack.focusWindow(makeWindow("0x1"));

    const auto resolution = hyprstack::resolveStackFocusTarget(stack, "prev");

    REQUIRE(resolution.address == std::optional<std::string>{"0x3"});
}

TEST_CASE("resolveStackFocusTarget falls back when current is absent") {
    hyprstack::WorkspaceStack stack;

    stack.addWindow(makeWindow("0x1"));
    stack.addWindow(makeWindow("0x2"));

    const auto nextResolution = hyprstack::resolveStackFocusTarget(stack, "next");
    const auto prevResolution = hyprstack::resolveStackFocusTarget(stack, "prev");

    REQUIRE(nextResolution.address == std::optional<std::string>{"0x1"});
    REQUIRE(prevResolution.address == std::optional<std::string>{"0x2"});
}

TEST_CASE("resolveStackFocusTarget reports empty stack and unknown subcommand") {
    hyprstack::WorkspaceStack stack;

    const auto emptyResolution   = hyprstack::resolveStackFocusTarget(stack, "next");
    const auto unknownResolution = hyprstack::resolveStackFocusTarget(stack, "swap");

    REQUIRE_FALSE(emptyResolution.address.has_value());
    REQUIRE(emptyResolution.error == "stack is empty");
    REQUIRE_FALSE(unknownResolution.address.has_value());
    REQUIRE(unknownResolution.error == "unknown stackfocus subcommand: swap");
}

TEST_CASE("resolveStackSwapTarget resolves next and prev across stable order") {
    hyprstack::WorkspaceStack stack;

    stack.addWindow(makeWindow("0x1"));
    stack.addWindow(makeWindow("0x2"));
    stack.addWindow(makeWindow("0x3"));
    stack.focusWindow(makeWindow("0x2"));

    const auto nextResolution = hyprstack::resolveStackSwapTarget(stack, "next");
    const auto prevResolution = hyprstack::resolveStackSwapTarget(stack, "prev");

    REQUIRE(nextResolution.address == std::optional<std::string>{"0x3"});
    REQUIRE(nextResolution.error.empty());
    REQUIRE(prevResolution.address == std::optional<std::string>{"0x1"});
    REQUIRE(prevResolution.error.empty());
}

TEST_CASE("resolveStackSwapTarget reports empty stack and unknown subcommand") {
    hyprstack::WorkspaceStack stack;

    const auto emptyResolution   = hyprstack::resolveStackSwapTarget(stack, "next");
    const auto unknownResolution = hyprstack::resolveStackSwapTarget(stack, "last");

    REQUIRE_FALSE(emptyResolution.address.has_value());
    REQUIRE(emptyResolution.error == "stack is empty");
    REQUIRE_FALSE(unknownResolution.address.has_value());
    REQUIRE(unknownResolution.error == "unknown stackswap subcommand: last");
}
