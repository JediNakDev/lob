#ifndef LOB_ORDER_BOOK_HPP
#define LOB_ORDER_BOOK_HPP

#include "price_level.hpp"
#include <map>
#include <unordered_map>
#include <vector>
#include <optional>
#include <functional>

namespace lob {

class OrderBook {
public:
    struct AddResult {
        OrderId order_id;
        std::vector<Fill> fills;
        Quantity remaining_quantity;
    };

private:
    std::map<Price, PriceLevel, std::greater<Price>> bids_;
    std::map<Price, PriceLevel> asks_;
    std::unordered_map<OrderId, std::shared_ptr<Order>> orders_;
    OrderId next_order_id_;

    std::vector<Fill> match_order(std::shared_ptr<Order> incoming);

public:
    OrderBook();

    [[nodiscard]] AddResult add_order(Price price, Quantity quantity, Side side);
    [[nodiscard]] bool cancel_order(OrderId order_id);
    [[nodiscard]] bool modify_order(OrderId order_id, Quantity new_quantity);

    [[nodiscard]] std::optional<Price> get_best_bid() const;
    [[nodiscard]] std::optional<Price> get_best_ask() const;
    [[nodiscard]] std::optional<Price> get_spread() const;
    [[nodiscard]] std::optional<Price> get_mid_price() const;

    [[nodiscard]] Quantity get_bid_quantity_at_top() const;
    [[nodiscard]] Quantity get_ask_quantity_at_top() const;

    [[nodiscard]] size_t get_bid_levels() const noexcept { return bids_.size(); }
    [[nodiscard]] size_t get_ask_levels() const noexcept { return asks_.size(); }
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
