#include "../utils/runner.hpp"
#include "../utils/workload.hpp"
#include <lob/order_book.hpp>

using namespace bench;

static void BM_AddOrder(benchmark::State& state) {
    const auto& w = bench::workload();
    lob::OrderBook book;
    size_t idx = 0;

    BenchmarkRunner runner(state, "AddOrder");
    runner.run_with_setup(
        [&] {
            book = lob::OrderBook();
            idx = 0;
        },
        [&](size_t i) {
            const auto& order = w.get(idx++);
            double price = order.side == lob::Side::BUY
                               ? BASE_PRICE - 50 * TICK_SIZE - (i % 50) * TICK_SIZE
                               : BASE_PRICE + 50 * TICK_SIZE + (i % 50) * TICK_SIZE;
            return book.add_order(price, order.quantity, order.side);
        });
}

BENCHMARK(BM_AddOrder)->Unit(benchmark::kNanosecond)->MinTime(3.0);
