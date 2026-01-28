CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic -I include -I .

# Google Benchmark paths (Homebrew on macOS, system default on Linux)
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    BENCHMARK_PREFIX := $(shell brew --prefix google-benchmark 2>/dev/null || echo "/opt/homebrew/opt/google-benchmark")
    BENCH_INCLUDES := -I$(BENCHMARK_PREFIX)/include
    BENCH_LDFLAGS := -L$(BENCHMARK_PREFIX)/lib
else
    BENCH_INCLUDES :=
    BENCH_LDFLAGS :=
endif

# Benchmark-specific flags for maximum performance
BENCH_CXXFLAGS := -std=c++17 -O3 -march=native -mtune=native -flto \
                  -fno-omit-frame-pointer -Wall -Wextra -I include -I . -I benchmark $(BENCH_INCLUDES)

SRC_DIR   := src
SRCS      := $(wildcard $(SRC_DIR)/*.cpp)

TEST_DIR  := tests
TESTS     := $(wildcard $(TEST_DIR)/*.cpp)

EXAMPLE_DIR := examples
EXAMPLES    := $(wildcard $(EXAMPLE_DIR)/*.cpp)

BENCH_DIR := benchmark
BENCH_SRCS := $(BENCH_DIR)/main.cpp $(wildcard $(BENCH_DIR)/cases/*.cpp)

BUILD_DIR := build
TARGET    := $(BUILD_DIR)/run_tests
BENCH_TARGET := $(BUILD_DIR)/benchmark

.PHONY: all clean test benchmark benchmark-run benchmark-json

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SRCS) $(TESTS) $(EXAMPLES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS) $(TESTS) $(EXAMPLES)

test: $(TARGET)
	./$(TARGET)

# ============================================================================
# Benchmark Targets
# ============================================================================

# Build benchmark with maximum optimizations
$(BENCH_TARGET): $(SRCS) $(BENCH_SRCS) | $(BUILD_DIR)
	$(CXX) $(BENCH_CXXFLAGS) -o $@ $(SRCS) $(BENCH_SRCS) $(BENCH_LDFLAGS) -lbenchmark -lpthread

benchmark: $(BENCH_TARGET)
	@echo "Benchmark built successfully: $(BENCH_TARGET)"
	@echo ""
	@echo "=== CPU Pinning Instructions ==="
	@echo ""
	@echo "Linux (recommended):"
	@echo "  sudo taskset -c 0 ./$(BENCH_TARGET)"
	@echo "  # or with isolated CPU:"
	@echo "  sudo taskset -c <isolated_core> ./$(BENCH_TARGET)"
	@echo ""
	@echo "macOS:"
	@echo "  ./$(BENCH_TARGET) --core=0"
	@echo ""
	@echo "Run 'make benchmark-run' to execute directly"

# Run benchmark with CPU pinning (Linux)
benchmark-run: $(BENCH_TARGET)
ifeq ($(shell uname),Linux)
	@echo "Running with CPU pinning on Core 0..."
	taskset -c 0 ./$(BENCH_TARGET)
else
	@echo "Running benchmark (CPU pinning handled internally on macOS)..."
	./$(BENCH_TARGET)
endif

# Export benchmark results to JSON for analysis
benchmark-json: $(BENCH_TARGET)
	./$(BENCH_TARGET) --benchmark_format=json --benchmark_out=$(BUILD_DIR)/benchmark_results.json
	@echo "Results saved to $(BUILD_DIR)/benchmark_results.json"

# Run specific benchmark by filter
benchmark-filter: $(BENCH_TARGET)
	@if [ -z "$(FILTER)" ]; then \
		echo "Usage: make benchmark-filter FILTER=<pattern>"; \
		echo "Example: make benchmark-filter FILTER=AddOrder"; \
	else \
		./$(BENCH_TARGET) --benchmark_filter=$(FILTER); \
	fi

clean:
	rm -rf $(BUILD_DIR)

debug: CXXFLAGS := -std=c++17 -g -O0 -Wall -Wextra -Wpedantic -I include -I . -fsanitize=address,undefined
debug: clean $(TARGET)

# Debug build for benchmark (with symbols but optimized)
benchmark-debug: BENCH_CXXFLAGS := -std=c++17 -O2 -g -Wall -Wextra -I include -I . $(BENCH_INCLUDES)
benchmark-debug: clean $(BENCH_TARGET)
