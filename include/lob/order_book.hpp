#ifndef LOB_ORDER_BOOK_HPP
#define LOB_ORDER_BOOK_HPP

#include "price_level.hpp"
#include <unordered_map>
#include <vector>
#include <optional>
#include <memory>

namespace lob {

/**
 * High-performance Limit Order Book following the article's design:
 * 
 * Data Structures:
 * - Hash map of Orders keyed by OrderId -> O(1) lookup for cancel/modify
 * - Hash map of PriceLevels keyed by Price -> O(1) lookup for add
 * - Cached best_bid/best_ask pointers -> O(1) for GetBestBid/Offer
 * - Intrusive linked lists within PriceLevels -> O(1) for order operations
 * 
 * Complexity:
 * - Add:    O(1) for orders at existing limits (O(log M) amortized for new limits due to best tracking)
 * - Cancel: O(1)
 * - Execute: O(1)
 * - GetBestBid/Offer: O(1)
 * - GetVolumeAtLimit: O(1)
 */
class OrderBook {
public:
    struct AddResult {
        OrderId order_id;
        std::vector<Fill> fills;
        Quantity remaining_quantity;
    };

private:
    // Hash map for O(1) order lookup by ID
    std::unordered_map<OrderId, std::unique_ptr<Order>> orders_;
    
    // Hash map for O(1) price level lookup
    std::unordered_map<Price, std::unique_ptr<PriceLevel>> bid_levels_;
    std::unordered_map<Price, std::unique_ptr<PriceLevel>> ask_levels_;
    
    // Cached best bid/ask for O(1) access
    // These are kept sorted via linked list of price levels
    PriceLevel* best_bid_;
    PriceLevel* best_ask_;
    
    OrderId next_order_id_;
    
    // Internal helpers
    std::vector<Fill> match_order(Order* incoming);
    void add_order_to_book(Order* order);
    void remove_order_from_book(Order* order);
    void insert_bid_level(PriceLevel* level);
    void insert_ask_level(PriceLevel* level);
    void remove_bid_level(PriceLevel* level);
    void remove_ask_level(PriceLevel* level);

public:
    OrderBook();
    ~OrderBook();
    
    // Prevent copying (unique_ptr members)
    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;
    
    // Allow moving
    OrderBook(OrderBook&&) = default;
    OrderBook& operator=(OrderBook&&) = default;

    [[nodiscard]] AddResult add_order(Price price, Quantity quantity, Side side);
    [[nodiscard]] bool cancel_order(OrderId order_id);
    [[nodiscard]] bool modify_order(OrderId order_id, Quantity new_quantity);

    // O(1) - direct pointer access
    [[nodiscard]] std::optional<Price> get_best_bid() const;
    [[nodiscard]] std::optional<Price> get_best_ask() const;
    [[nodiscard]] std::optional<Price> get_spread() const;
    [[nodiscard]] std::optional<Price> get_mid_price() const;

    [[nodiscard]] Quantity get_bid_quantity_at_top() const;
    [[nodiscard]] Quantity get_ask_quantity_at_top() const;

    [[nodiscard]] size_t get_bid_levels() const noexcept { return bid_levels_.size(); }
    [[nodiscard]] size_t get_ask_levels() const noexcept { return ask_levels_.size(); }
    [[nodiscard]] size_t get_total_orders() const noexcept { return orders_.size(); }

    struct BookSnapshot {
        struct Level {
            Price price;
            Quantity quantity;
            size_t order_count;
        };
        std::vector<Level> bids;
        std::vector<Level> asks;
    };
    
    [[nodiscard]] BookSnapshot get_snapshot(size_t depth = 5) const;
};

}

#endif
