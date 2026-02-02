#ifndef LOB_ORDER_BOOK_HPP
#define LOB_ORDER_BOOK_HPP

#include "price_level.hpp"
#include <unordered_map>
#include <vector>
#include <optional>
#include <memory>

namespace lob {

/**
 * OrderBook - Limit Order Book using BST for price levels.
 * 
 * Structure:
 * - Separate BST for buy and sell sides (buy_tree_, sell_tree_)
 * - Hash maps for O(1) lookup by price and order ID
 * - Cached pointers to best bid (highest_buy_) and best ask (lowest_sell_)
 * 
 * Performance:
 * - Add order (existing level): O(1)
 * - Add order (new level): O(log M) where M = number of price levels
 * - Cancel order: O(1)
 * - Execute order: O(1)
 * - GetBestBid/Ask: O(1)
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
    // Order storage - keyed by order ID for O(1) lookup
    std::unordered_map<OrderId, std::unique_ptr<Order>> orders_;
    
    // Price level storage - keyed by price for O(1) lookup
    std::unordered_map<Price, std::unique_ptr<PriceLevel>> bid_levels_;
    std::unordered_map<Price, std::unique_ptr<PriceLevel>> ask_levels_;
    
    // BST roots for sorted price levels
    PriceLevel* buy_tree_;      // Root of buy-side BST
    PriceLevel* sell_tree_;     // Root of sell-side BST
    
    // Cached best prices for O(1) access
    PriceLevel* highest_buy_;   // Best bid (max price in buy tree)
    PriceLevel* lowest_sell_;   // Best ask (min price in sell tree)
    
    OrderId next_order_id_;
    
    // BST operations for bid side (sorted by price, higher = better)
    void insert_bid_level(PriceLevel* level);
    void remove_bid_level(PriceLevel* level);
    
    // BST operations for ask side (sorted by price, lower = better)
    void insert_ask_level(PriceLevel* level);
    void remove_ask_level(PriceLevel* level);
    
    // BST helpers
    PriceLevel* find_min(PriceLevel* node) const;
    PriceLevel* find_max(PriceLevel* node) const;
    PriceLevel* find_successor(PriceLevel* node) const;
    PriceLevel* find_predecessor(PriceLevel* node) const;
    void transplant_bid(PriceLevel* u, PriceLevel* v);
    void transplant_ask(PriceLevel* u, PriceLevel* v);
    
    // Order book operations
    std::vector<Fill> match_order(Order* incoming);
    void add_order_to_book(Order* order);
    void remove_order_from_book(Order* order);

public:
    OrderBook();
    ~OrderBook();
    
    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;
    OrderBook(OrderBook&&) = default;
    OrderBook& operator=(OrderBook&&) = default;

    [[nodiscard]] AddResult add_order(Price price, Quantity quantity, Side side);
    [[nodiscard]] bool cancel_order(OrderId order_id);
    [[nodiscard]] bool modify_order(OrderId order_id, Quantity new_quantity);

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
