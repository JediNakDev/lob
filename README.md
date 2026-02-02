# Limit Order Book

A high-performance limit order book implementation in modern C++17.

## Building

```bash
make        # Build the demo
make test   # Run tests
make benchmark  # Build benchmark suite
make clean  # Clean build artifacts
```

## Iteration 1.1.0

Implemented optimizations based on ["How to Build a Fast Limit Order Book"](https://web.archive.org/web/20110219163448/http://howtohft.wordpress.com/2011/02/15/how-to-build-a-fast-limit-order-book/) article.

### Key Changes

- **Integer Prices**: Changed from `double` to `int64_t` (ticks) for faster comparisons and better hashing
- **Intrusive Doubly-Linked Lists**: Orders contain `prev/next` pointers for O(1) removal
- **Hash Maps**: `std::unordered_map` for O(1) price level and order lookup
- **Binary Search Tree**: Price levels organized in BST for O(log M) insertion of new levels
- **Cached Best Bid/Ask**: Direct pointer access (`highest_buy_`/`lowest_sell_`) for O(1) best price queries

### Performance Summary

**AddOrder 47 ns | MatchOrder 117 ns | MixedWorkload 16.6 Mops/s**

#### Significant Improvements vs 1.0.0

| Operation | Mean | Improvement |
|-----------|------|-------------|
| AddOrder | 47 ns | **2.3x faster** (was 109 ns) |
| CancelOrder | 16 ns | **1.4x faster** (was 23 ns) |
| ModifyOrder | 15 ns | **1.6x faster** (was 24 ns) |
| MatchOrder | 117 ns | **1.4x faster** (was 160 ns) |
| MixedWorkload | 42 ns | **1.6x faster** (was 69 ns) |

#### Tail Latency Improvements

| Operation | P99 | P99.99 | vs 1.0.0 |
|-----------|-----|--------|----------|
| AddOrder | 84 ns | 1.8 µs | P99 **9x better** (was 792 ns), P99.99 **19x better** (was 34 µs) |
| MatchOrder | 209 ns | 334 ns | P99.99 **69x better** (was 23 µs) |
| MixedWorkload | 292 ns | 708 ns | P99.99 **28x better** (was 20 µs) |

### Time Complexity

| Operation       | Complexity | Notes |
|-----------------|------------|-------|
| `add_order`     | O(1) / O(log M) | O(1) at existing limit, O(log M) for new price level |
| `cancel_order`  | O(1) | Hash lookup + intrusive list removal |
| `modify_order`  | O(1) | Hash lookup + quantity update |
| `get_best_bid`  | O(1) | Cached pointer |
| `get_best_ask`  | O(1) | Cached pointer |
| `get_snapshot`  | O(D) | D = depth requested |

---

## Iteration 1.0.0

- **Price-Time Priority**: Orders matched by best price first, then arrival time (FIFO)
- **Efficient Data Structures**: `std::map` for price levels, `std::list` for order queues, `std::unordered_map` for O(1) order lookup
- **Clean API**: Structured results with I/O separated from business logic
- **Modern C++17**: `std::optional`, `[[nodiscard]]`, namespaces, const-correctness

### Performance Summary

**AddOrder 109 ns | MatchOrder 160 ns | MixedWorkload 11 Mops/s**

| Benchmark | Mean | P99 | P99.99 |
|-----------|------|-----|--------|
| GetBestBid | 19 ns | 42 ns | 1.4 µs |
| GetBestAsk | 17 ns | 42 ns | 166 ns |
| GetSpread | 18 ns | 42 ns | 167 ns |
| CancelOrder | 23 ns | 125 ns | 459 ns |
| ModifyOrder | 24 ns | 42 ns | 167 ns |
| AddOrder | 109 ns | 792 ns | 34 µs |
| MatchOrder | 160 ns | 292 ns | 23 µs |
| MixedWorkload | 69 ns | 459 ns | 20 µs |

See [benchmark/README.md](benchmark/README.md) for methodology.

### Potential Improvements

1. Memory pool allocators
2. Market orders, IOC, FOK order types
3. Thread safety / lock-free structures
4. SIMD optimizations for batch operations
