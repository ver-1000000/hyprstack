PLUGIN := hyprstack.so
PLUGIN_DEV := hyprstack-dev.so
PLUGIN_SRC := src/hyprland_executor.cpp src/hyprland_observer.cpp src/main.cpp src/plugin_state.cpp src/query_command.cpp src/stack_dispatcher.cpp src/workspace_stack.cpp
TEST_BIN := tests-runner
TEST_SRC := tests/plugin_state_test.cpp tests/query_command_test.cpp tests/stack_dispatcher_test.cpp tests/workspace_stack_test.cpp src/plugin_state.cpp src/query_command.cpp src/stack_dispatcher.cpp src/workspace_stack.cpp vendor/catch2/catch_amalgamated.cpp

CXX ?= c++
COMMON_CXXFLAGS += -std=c++23 -Wall -Wextra -Wpedantic
PLUGIN_CXXFLAGS += $(COMMON_CXXFLAGS) -fPIC
PLUGIN_CXXFLAGS += -Iinclude
PLUGIN_CXXFLAGS += $(shell pkg-config --cflags hyprland)
TEST_CXXFLAGS += $(COMMON_CXXFLAGS) -Iinclude -Ivendor/catch2
PLUGIN_LDFLAGS += -shared
PLUGIN_LDLIBS += $(shell pkg-config --libs hyprland)

all: $(PLUGIN)

dev-plugin: $(PLUGIN)
	cp -f $(PLUGIN) $(PLUGIN_DEV)

$(PLUGIN): $(PLUGIN_SRC)
	$(CXX) $(PLUGIN_CXXFLAGS) $(PLUGIN_LDFLAGS) -o $@ $^ $(PLUGIN_LDLIBS)

$(TEST_BIN): $(TEST_SRC)
	$(CXX) $(TEST_CXXFLAGS) -o $@ $^

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(PLUGIN) $(PLUGIN_DEV) $(TEST_BIN)

.PHONY: all dev-plugin test clean
