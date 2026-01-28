CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic -I include -I .

SRC_DIR   := src
SRCS      := $(wildcard $(SRC_DIR)/*.cpp)

TEST_DIR  := tests
TESTS     := $(wildcard $(TEST_DIR)/*.cpp)

EXAMPLE_DIR := examples
EXAMPLES    := $(wildcard $(EXAMPLE_DIR)/*.cpp)

BUILD_DIR := build
TARGET    := $(BUILD_DIR)/run_tests

.PHONY: all clean test

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SRCS) $(TESTS) $(EXAMPLES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS) $(TESTS) $(EXAMPLES)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

debug: CXXFLAGS := -std=c++17 -g -O0 -Wall -Wextra -Wpedantic -I include -I . -fsanitize=address,undefined
debug: clean $(TARGET)
