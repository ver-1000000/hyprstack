PLUGIN := hyprstack.so
SRC := src/main.cpp

CXX ?= c++
CXXFLAGS += -std=c++23 -Wall -Wextra -Wpedantic -fPIC
CXXFLAGS += $(shell pkg-config --cflags hyprland)
LDFLAGS += -shared
LDLIBS += $(shell pkg-config --libs hyprland)

all: $(PLUGIN)

$(PLUGIN): $(SRC)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(PLUGIN)

.PHONY: all clean
