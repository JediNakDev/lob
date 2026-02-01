#ifndef LOB_ORDER_HPP
#define LOB_ORDER_HPP

#include "types.hpp"
#include <chrono>

namespace lob {

// Forward declaration for parent pointer
class PriceLevel;

/**
 * Order struct following the article's design:
 * - Intrusive doubly-linked list pointers for O(1) removal
 * - Parent pointer to PriceLevel for O(1) quantity updates
 * - All fields packed for cache efficiency
 */
struct Order {
    // Core order data (hot path - keep together for cache locality)
    OrderId id;
    Price price;
    Quantity quantity;
    Quantity remaining_quantity;
    Side side;
    
    // Intrusive doubly-linked list pointers for O(1) cancel
    Order* prev_order;
    Order* next_order;
    
    // Parent pointer for O(1) price level access during cancel
    PriceLevel* parent_level;
    
    // Timestamp (cold path)
    Timestamp entry_time;
    
    Order(OrderId id_, Price price_, Quantity quantity_, Side side_) noexcept
        : id(id_)
        , price(price_)
        , quantity(quantity_)
        , remaining_quantity(quantity_)
        , side(side_)
        , prev_order(nullptr)
        , next_order(nullptr)
        , parent_level(nullptr)
        , entry_time(static_cast<Timestamp>(
              std::chrono::duration_cast<std::chrono::nanoseconds>(
                  std::chrono::steady_clock::now().time_since_epoch()).count()))
    {}

    [[nodiscard]] bool is_filled() const noexcept { 
        return remaining_quantity == 0; 
    }

    void fill(Quantity qty) noexcept {
        remaining_quantity = (qty >= remaining_quantity) ? 0 : remaining_quantity - qty;
    }
};

}

#endif
