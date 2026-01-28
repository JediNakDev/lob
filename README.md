# Limit Order Book

A high-performance limit order book implementation in modern C++17.

## Building

```bash
make        # Build the demo
make test   # Run tests
make benchmark  # Build benchmark suite
make clean  # Clean build artifacts
```

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

The bottleneck operations are `AddOrder` and `MatchOrder`. The next iteration will focus on optimizing these.

See [benchmark/README.md](benchmark/README.md) for methodology.

### Time Complexity

| Operation       | Complexity | Notes |
|-----------------|------------|-------|
| `add_order`     | O(log P + M) | P = price levels, M = matched orders |
| `cancel_order`  | O(log P + N) | N = orders at that price level |
| `modify_order`  | O(log P) | |
| `get_best_bid`  | O(1) | |
| `get_best_ask`  | O(1) | |
| `get_snapshot`  | O(D) | D = depth requested |

### Potential Improvements

1. Fixed-point prices for precision
2. Market orders, IOC, FOK order types
3. Thread safety / lock-free structures
4. Memory pool allocators
5. Intrusive containers
