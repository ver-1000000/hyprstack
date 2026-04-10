BUILD_DIR ?= build
CMAKE ?= cmake
VERSION_FILE ?= src/plugin/entrypoint.cpp
PLUGIN_VERSION := $(shell sed -n 's/.*\.version *= *"\([^"]*\)".*/\1/p' $(VERSION_FILE) | head -n 1)
PLUGIN_TAG := v$(PLUGIN_VERSION)

all: $(BUILD_DIR)/CMakeCache.txt
	$(CMAKE) --build $(BUILD_DIR) --target hyprstack_plugin

dev-plugin: all
	cp -f $(BUILD_DIR)/hyprstack.so $(BUILD_DIR)/hyprstack-dev.so

$(BUILD_DIR)/CMakeCache.txt: CMakeLists.txt
	$(CMAKE) -S . -B $(BUILD_DIR)

test: $(BUILD_DIR)/CMakeCache.txt
	$(CMAKE) --build $(BUILD_DIR) --target tests_runner
	./$(BUILD_DIR)/tests-runner

print-tag:
	@test -n "$(PLUGIN_VERSION)"
	@printf '%s\n' '$(PLUGIN_TAG)'

release-tag:
	@test -n "$(PLUGIN_VERSION)"
	@git diff --quiet
	@git diff --cached --quiet
	@! git rev-parse -q --verify "refs/tags/$(PLUGIN_TAG)" >/dev/null
	@! git ls-remote --exit-code --tags origin "refs/tags/$(PLUGIN_TAG)" >/dev/null 2>&1
	@git tag -a "$(PLUGIN_TAG)" -m "$(PLUGIN_TAG)"
	@git push origin "$(PLUGIN_TAG)"

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all dev-plugin test print-tag release-tag clean
