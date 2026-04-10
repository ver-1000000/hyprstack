#pragma once

#include "hyprstack/domain/stack_types.hpp"

#include <optional>
#include <string>
#include <vector>

namespace hyprstack {

class WorkspaceStack {
  public:
    void addWindow(const StackWindow& window);
    void focusWindow(const StackWindow& window);
    void removeWindow(const std::string& address);
    bool swapCurrentWithNext();
    bool swapCurrentWithPrev();

    [[nodiscard]] const std::vector<StackWindow>& windows() const;
    [[nodiscard]] std::optional<std::string> current() const;
    [[nodiscard]] std::optional<std::string> last() const;
    [[nodiscard]] StackAround around() const;

  private:
    std::vector<StackWindow>   m_windows;
    std::optional<std::string> m_current;
    std::optional<std::string> m_last;

    StackWindow*       findWindow(const std::string& address);
    const StackWindow* findWindow(const std::string& address) const;
    size_t             indexOf(const std::string& address) const;
};

} // namespace hyprstack
