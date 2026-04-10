PLUGIN := hyprstack.so
PLUGIN_SRC := src/main.cpp src/query_command.cpp
TEST_BIN := tests-runner
TEST_SRC := tests/query_command_test.cpp src/query_command.cpp vendor/catch2/catch_amalgamated.cpp

CXX ?= c++
COMMON_CXXFLAGS += -std=c++23 -Wall -Wextra -Wpedantic
PLUGIN_CXXFLAGS += $(COMMON_CXXFLAGS) -fPIC
PLUGIN_CXXFLAGS += -Iinclude
PLUGIN_CXXFLAGS += $(shell pkg-config --cflags hyprland)
TEST_CXXFLAGS += $(COMMON_CXXFLAGS) -Iinclude -Ivendor/catch2
PLUGIN_LDFLAGS += -shared
PLUGIN_LDLIBS += $(shell pkg-config --libs hyprland)

all: $(PLUGIN)

$(PLUGIN): $(PLUGIN_SRC)
	$(CXX) $(PLUGIN_CXXFLAGS) $(PLUGIN_LDFLAGS) -o $@ $^ $(PLUGIN_LDLIBS)

$(TEST_BIN): $(TEST_SRC)
	$(CXX) $(TEST_CXXFLAGS) -o $@ $^

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(PLUGIN) $(TEST_BIN)

.PHONY: all test clean
