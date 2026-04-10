#include "hyprstack/workspace_stack.hpp"

#include "catch_amalgamated.hpp"

namespace {

hyprstack::StackWindow makeWindow(std::string address, std::string className = "test.app", std::string title = "title") {
    return {
        .address   = std::move(address),
        .className = std::move(className),
        .title     = std::move(title),
    };
}

} // namespace

TEST_CASE("addWindow appends windows in stable order") {
    hyprstack::WorkspaceStack stack;

    stack.addWindow(makeWindow("0x1"));
    stack.addWindow(makeWindow("0x2"));

    REQUIRE(stack.windows().size() == 2);
    REQUIRE(stack.windows()[0].address == "0x1");
    REQUIRE(stack.windows()[1].address == "0x2");
}

TEST_CASE("focusWindow tracks current and last without changing stable order") {
    hyprstack::WorkspaceStack stack;

    stack.addWindow(makeWindow("0x1"));
    stack.addWindow(makeWindow("0x2"));

    stack.focusWindow(makeWindow("0x2"));
    stack.focusWindow(makeWindow("0x1"));

    REQUIRE(stack.windows().size() == 2);
    REQUIRE(stack.windows()[0].address == "0x1");
    REQUIRE(stack.windows()[1].address == "0x2");
    REQUIRE(stack.current() == std::optional<std::string>{"0x1"});
    REQUIRE(stack.last() == std::optional<std::string>{"0x2"});
}

TEST_CASE("focusWindow on unknown address appends then focuses") {
    hyprstack::WorkspaceStack stack;

    stack.focusWindow(makeWindow("0x1"));
    stack.focusWindow(makeWindow("0x2"));

    REQUIRE(stack.windows().size() == 2);
    REQUIRE(stack.windows()[1].address == "0x2");
    REQUIRE(stack.current() == std::optional<std::string>{"0x2"});
    REQUIRE(stack.last() == std::optional<std::string>{"0x1"});
}

TEST_CASE("addWindow updates metadata without duplicating address") {
    hyprstack::WorkspaceStack stack;

    stack.addWindow(makeWindow("0x1", "old.class", "old title"));
    stack.addWindow(makeWindow("0x1", "new.class", "new title"));

    REQUIRE(stack.windows().size() == 1);
    REQUIRE(stack.windows()[0].className == "new.class");
    REQUIRE(stack.windows()[0].title == "new title");
}

TEST_CASE("around returns stable neighbors and last-focused independently") {
    hyprstack::WorkspaceStack stack;

    stack.addWindow(makeWindow("0x1"));
    stack.addWindow(makeWindow("0x2"));
    stack.addWindow(makeWindow("0x3"));

    stack.focusWindow(makeWindow("0x2"));
    stack.focusWindow(makeWindow("0x3"));
    stack.focusWindow(makeWindow("0x2"));

    const auto around = stack.around();

    REQUIRE(around.current == std::optional<std::string>{"0x2"});
    REQUIRE(around.prev == std::optional<std::string>{"0x1"});
    REQUIRE(around.next == std::optional<std::string>{"0x3"});
    REQUIRE(around.last == std::optional<std::string>{"0x3"});
}

TEST_CASE("removeWindow removes current and last when they point to removed window") {
    hyprstack::WorkspaceStack stack;

    stack.focusWindow(makeWindow("0x1"));
    stack.focusWindow(makeWindow("0x2"));

    stack.removeWindow("0x2");

    REQUIRE(stack.windows().size() == 1);
    REQUIRE(stack.windows()[0].address == "0x1");
    REQUIRE_FALSE(stack.current().has_value());
    REQUIRE(stack.last() == std::optional<std::string>{"0x1"});

    stack.removeWindow("0x1");

    REQUIRE(stack.windows().empty());
    REQUIRE_FALSE(stack.current().has_value());
    REQUIRE_FALSE(stack.last().has_value());
}

TEST_CASE("focusWindow clears last when refocusing the only remaining window") {
    hyprstack::WorkspaceStack stack;

    stack.focusWindow(makeWindow("0x1"));
    stack.focusWindow(makeWindow("0x2"));
    stack.removeWindow("0x2");
    stack.focusWindow(makeWindow("0x1"));

    REQUIRE(stack.windows().size() == 1);
    REQUIRE(stack.current() == std::optional<std::string>{"0x1"});
    REQUIRE_FALSE(stack.last().has_value());
}

TEST_CASE("swapCurrentWithNext swaps stable order and wraps at the end") {
    hyprstack::WorkspaceStack stack;

    stack.addWindow(makeWindow("0x1"));
    stack.addWindow(makeWindow("0x2"));
    stack.addWindow(makeWindow("0x3"));
    stack.focusWindow(makeWindow("0x2"));

    REQUIRE(stack.swapCurrentWithNext());
    REQUIRE(stack.windows()[0].address == "0x1");
    REQUIRE(stack.windows()[1].address == "0x3");
    REQUIRE(stack.windows()[2].address == "0x2");
    REQUIRE(stack.current() == std::optional<std::string>{"0x2"});

    stack.focusWindow(makeWindow("0x2"));
    REQUIRE(stack.swapCurrentWithNext());
    REQUIRE(stack.windows()[0].address == "0x2");
    REQUIRE(stack.windows()[1].address == "0x3");
    REQUIRE(stack.windows()[2].address == "0x1");
}

TEST_CASE("swapCurrentWithPrev swaps stable order and wraps at the beginning") {
    hyprstack::WorkspaceStack stack;

    stack.addWindow(makeWindow("0x1"));
    stack.addWindow(makeWindow("0x2"));
    stack.addWindow(makeWindow("0x3"));
    stack.focusWindow(makeWindow("0x1"));

    REQUIRE(stack.swapCurrentWithPrev());
    REQUIRE(stack.windows()[0].address == "0x3");
    REQUIRE(stack.windows()[1].address == "0x2");
    REQUIRE(stack.windows()[2].address == "0x1");
    REQUIRE(stack.current() == std::optional<std::string>{"0x1"});
}
