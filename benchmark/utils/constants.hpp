#pragma once

#include <cstddef>

namespace bench {

constexpr double BASE_PRICE = 100.0;
constexpr double TICK_SIZE = 0.01;
constexpr int PRICE_LEVELS = 100;
constexpr int ORDERS_PER_LEVEL = 10;
constexpr size_t WARMUP_ITERATIONS = 10000;
constexpr size_t BENCHMARK_SAMPLES = 100000;

}
