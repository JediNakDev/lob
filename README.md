# Limit Order Book

A high-performance limit order book implementation in modern C++17.

## Features

- **Price-Time Priority**: Orders are matched by best price first, then by arrival time (FIFO)
- **Efficient Data Structures**:
  - `std::map` for price levels (O(log P) operations, automatic sorting)
  - `std::list` for order queues (O(1) FIFO operations)
  - `std::unordered_map` for order lookup (O(1) average)
- **Clean API**: Returns structured results instead of printing; I/O separated from business logic
- **Modern C++**: Uses `std::optional`, `[[nodiscard]]`, namespaces, and proper const-correctness

## Project Structure

```
lob/
├── Makefile
├── README.md
├── include/
│   └── lob/
│       ├── types.hpp        # Core types: Side, Fill, type aliases
│       ├── order.hpp        # Order class
│       ├── price_level.hpp  # PriceLevel class (FIFO queue at a price)
│       └── order_book.hpp   # OrderBook class (the main interface)
├── src/
│   └── order_book.cpp       # OrderBook implementation
└── examples/
    └── main.cpp             # Demo/test application
```

## Building

```bash
make        # Build the demo
make run    # Build and run
make clean  # Clean build artifacts
make debug  # Build with debug symbols and sanitizers
```

## Time Complexity

| Operation       | Complexity | Notes |
|-----------------|------------|-------|
| `add_order`     | O(log P + M) | P = price levels, M = matched orders |
| `cancel_order`  | O(log P + N) | N = orders at that price level |
| `modify_order`  | O(log P) | |
| `get_best_bid`  | O(1) | |
| `get_best_ask`  | O(1) | |
| `get_snapshot`  | O(D) | D = depth requested |

## Usage Example

```cpp
#include "lob/order_book.hpp"

int main() {
    lob::OrderBook book;
    
    // Add orders
    auto result = book.add_order(100.0, 50, lob::Side::BUY);
    // result.order_id    - the assigned order ID
    // result.fills       - vector of fills if any matching occurred
    // result.remaining   - quantity still resting on book
    
    book.add_order(101.0, 100, lob::Side::SELL);
    
    // Query the book
    if (auto bid = book.get_best_bid()) {
        std::cout << "Best bid: " << *bid << "\n";
    }
    
    // Get a snapshot of top 5 levels
    auto snapshot = book.get_snapshot(5);
    
    // Cancel an order
    bool cancelled = book.cancel_order(result.order_id);
    
    return 0;
}
```

## Design Decisions

### Why `std::map` for price levels?

- Automatically maintains sorted order (bids descending, asks ascending)
- O(log P) insert/delete/lookup is acceptable for most use cases
- Alternatives considered:
  - `std::unordered_map` + sorted vector: faster lookup but complex maintenance
  - Skip list: similar complexity, more implementation effort
  - For ultra-low-latency (sub-microsecond), a fixed-size array with price bucketing would be preferred

### Why `std::list` for order queues?

- O(1) insertion at back (new orders)
- O(1) removal from front (fills)
- Stable iterators (important for order lookup)
- Trade-off: O(n) for mid-queue cancellation, but this is rare in practice

### Why `double` for prices?

Production systems typically use fixed-point arithmetic (e.g., `int64_t` with implicit decimals) to avoid floating-point precision issues. This implementation uses `double` for simplicity, with a type alias (`Price`) to make future changes straightforward.

### Separation of I/O

The order book returns structured results (`AddResult`, `Fill`, `BookSnapshot`) instead of printing directly. This:
- Makes the code testable
- Allows different output formats (JSON, binary, etc.)
- Follows the single responsibility principle

## Potential Improvements

1. **Fixed-point prices**: Replace `double` with an integer-based fixed-point type
2. **Order types**: Add support for market orders, stop orders, IOC, FOK, etc.
3. **Thread safety**: Add mutex or lock-free structures for concurrent access
4. **Memory pool**: Use custom allocators for `Order` objects to reduce allocation overhead
5. **Intrusive containers**: Use Boost.Intrusive or custom intrusive lists to eliminate pointer overhead
6. **Benchmarking**: Add performance benchmarks with realistic order flow

## License

MIT
