#include "hyprstack/stack_dispatcher.hpp"

#include <algorithm>
#include <cctype>

namespace hyprstack {

StackDispatchResolution successResult(std::string address) {
    return {
        .address = std::move(address),
        .error   = "",
    };
}

StackDispatchResolution errorResult(std::string error) {
    return {
        .address = std::nullopt,
        .error   = std::move(error),
    };
}

namespace {

std::string trim(std::string_view input) {
    auto first = input.begin();
    auto last  = input.end();

    while (first != last && std::isspace(static_cast<unsigned char>(*first)))
        ++first;

    while (first != last && std::isspace(static_cast<unsigned char>(*(last - 1))))
        --last;

    return std::string(first, last);
}

std::optional<size_t> currentIndex(const WorkspaceStack& stack) {
    if (!stack.current())
        return std::nullopt;

    const auto& windows = stack.windows();

    for (size_t i = 0; i < windows.size(); ++i) {
        if (windows[i].address == *stack.current())
            return i;
    }

    return std::nullopt;
}

StackDispatchResolution resolveNext(const WorkspaceStack& stack) {
    const auto& windows = stack.windows();

    if (windows.empty())
        return errorResult("stack is empty");

    const auto index = currentIndex(stack).value_or(windows.size() - 1);
    return successResult(windows[(index + 1) % windows.size()].address);
}

StackDispatchResolution resolvePrev(const WorkspaceStack& stack) {
    const auto& windows = stack.windows();

    if (windows.empty())
        return errorResult("stack is empty");

    const auto index = currentIndex(stack).value_or(0);
    return successResult(windows[(index + windows.size() - 1) % windows.size()].address);
}

StackDispatchResolution resolveLast(const WorkspaceStack& stack) {
    if (!stack.last())
        return errorResult("no last window in stack");

    return successResult(*stack.last());
}

} // namespace

StackDispatchResolution resolveStackFocusTarget(const WorkspaceStack& stack, const std::string_view arg) {
    const auto action = trim(arg);

    if (action == "next")
        return resolveNext(stack);

    if (action == "prev")
        return resolvePrev(stack);

    if (action == "last")
        return resolveLast(stack);

    return errorResult("unknown stackfocus subcommand: " + action);
}

StackDispatchResolution resolveStackSwapTarget(const WorkspaceStack& stack, const std::string_view arg) {
    const auto action = trim(arg);

    if (action == "next")
        return resolveNext(stack);

    if (action == "prev")
        return resolvePrev(stack);

    return errorResult("unknown stackswap subcommand: " + action);
}

} // namespace hyprstack
