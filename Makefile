CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic -I include

# Source files
SRC_DIR  := src
SRCS     := $(wildcard $(SRC_DIR)/*.cpp)

# Example files
EXAMPLE_DIR := examples
EXAMPLES    := $(wildcard $(EXAMPLE_DIR)/*.cpp)

# Output
BUILD_DIR := build
TARGET    := $(BUILD_DIR)/lob_demo

.PHONY: all clean run

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SRCS) $(EXAMPLES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS) $(EXAMPLES)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

# Development: compile with debug symbols and sanitizers
debug: CXXFLAGS := -std=c++17 -g -O0 -Wall -Wextra -Wpedantic -I include -fsanitize=address,undefined
debug: $(TARGET)

# Header dependency tracking (for development)
%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM -MT '$(BUILD_DIR)/$*.o' $< > $@
