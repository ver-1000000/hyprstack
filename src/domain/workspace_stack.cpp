#include "hyprstack/domain/workspace_stack.hpp"

#include <algorithm>
#include <cstddef>

namespace hyprstack {

namespace {

std::optional<std::string> atIndex(const std::vector<StackWindow>& windows, const std::ptrdiff_t index) {
    if (index < 0 || static_cast<size_t>(index) >= windows.size())
        return std::nullopt;

    return windows[static_cast<size_t>(index)].address;
}

} // namespace

void WorkspaceStack::addWindow(const StackWindow& window) {
    if (auto* existing = findWindow(window.address)) {
        existing->className = window.className;
        existing->title     = window.title;
        return;
    }

    m_windows.push_back(window);
}

void WorkspaceStack::focusWindow(const StackWindow& window) {
    addWindow(window);

    if (m_current && *m_current != window.address)
        m_last = m_current;

    m_current = window.address;

    if (m_last && *m_last == *m_current)
        m_last.reset();
}

void WorkspaceStack::removeWindow(const std::string& address) {
    std::erase_if(m_windows, [&address](const StackWindow& window) { return window.address == address; });

    if (m_current && *m_current == address)
        m_current.reset();

    if (m_last && *m_last == address)
        m_last.reset();
}

bool WorkspaceStack::swapCurrentWithNext() {
    if (!m_current || m_windows.size() < 2)
        return false;

    const auto currentIndex = indexOf(*m_current);

    if (currentIndex >= m_windows.size())
        return false;

    const auto nextIndex = (currentIndex + 1) % m_windows.size();
    std::swap(m_windows[currentIndex], m_windows[nextIndex]);
    return true;
}

bool WorkspaceStack::swapCurrentWithPrev() {
    if (!m_current || m_windows.size() < 2)
        return false;

    const auto currentIndex = indexOf(*m_current);

    if (currentIndex >= m_windows.size())
        return false;

    const auto prevIndex = (currentIndex + m_windows.size() - 1) % m_windows.size();
    std::swap(m_windows[currentIndex], m_windows[prevIndex]);
    return true;
}

const std::vector<StackWindow>& WorkspaceStack::windows() const {
    return m_windows;
}

std::optional<std::string> WorkspaceStack::current() const {
    return m_current;
}

std::optional<std::string> WorkspaceStack::last() const {
    return m_last;
}

StackAround WorkspaceStack::around() const {
    StackAround result{
        .current = m_current,
        .prev    = std::nullopt,
        .next    = std::nullopt,
        .last    = m_last,
    };

    if (!m_current)
        return result;

    const auto index = static_cast<std::ptrdiff_t>(indexOf(*m_current));
    result.prev      = atIndex(m_windows, index - 1);
    result.next      = atIndex(m_windows, index + 1);

    return result;
}

StackWindow* WorkspaceStack::findWindow(const std::string& address) {
    const auto iter = std::ranges::find(m_windows, address, &StackWindow::address);

    if (iter == m_windows.end())
        return nullptr;

    return &(*iter);
}

const StackWindow* WorkspaceStack::findWindow(const std::string& address) const {
    const auto iter = std::ranges::find(m_windows, address, &StackWindow::address);

    if (iter == m_windows.end())
        return nullptr;

    return &(*iter);
}

size_t WorkspaceStack::indexOf(const std::string& address) const {
    const auto iter = std::ranges::find(m_windows, address, &StackWindow::address);

    if (iter == m_windows.end())
        return m_windows.size();

    return static_cast<size_t>(std::distance(m_windows.begin(), iter));
}

} // namespace hyprstack
