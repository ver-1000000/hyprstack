BUILD_DIR ?= build
CMAKE ?= cmake

all: $(BUILD_DIR)/CMakeCache.txt
	$(CMAKE) --build $(BUILD_DIR) --target hyprstack_plugin

dev-plugin: all
	cp -f $(BUILD_DIR)/hyprstack.so $(BUILD_DIR)/hyprstack-dev.so

$(BUILD_DIR)/CMakeCache.txt: CMakeLists.txt
	$(CMAKE) -S . -B $(BUILD_DIR)

test: $(BUILD_DIR)/CMakeCache.txt
	$(CMAKE) --build $(BUILD_DIR) --target tests_runner
	./$(BUILD_DIR)/tests-runner

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all dev-plugin test clean
