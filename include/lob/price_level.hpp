#ifndef LOB_PRICE_LEVEL_HPP
#define LOB_PRICE_LEVEL_HPP

#include "order.hpp"
#include <list>
#include <memory>

namespace lob {

class PriceLevel {
public:
    Price price;
    std::list<std::shared_ptr<Order>> orders;
    Quantity total_quantity;

    explicit PriceLevel(Price price) noexcept
        : price(price)
        , total_quantity(0) 
    {}

    void add_order(std::shared_ptr<Order> order) {
        orders.push_back(order);
        total_quantity += order->remaining_quantity;
    }

    void remove_order(const std::shared_ptr<Order>& order) {
        total_quantity -= order->remaining_quantity;
        orders.remove(order);
    }

    [[nodiscard]] std::shared_ptr<Order> front() const {
        return orders.empty() ? nullptr : orders.front();
    }

    void pop_front() {
        if (!orders.empty()) {
            total_quantity -= orders.front()->remaining_quantity;
            orders.pop_front();
        }
    }

    [[nodiscard]] bool is_empty() const noexcept { 
        return orders.empty(); 
    }

    [[nodiscard]] size_t order_count() const noexcept { 
        return orders.size(); 
    }

    void update_quantity(int64_t delta) noexcept {
        int64_t new_qty = static_cast<int64_t>(total_quantity) + delta;
        total_quantity = (new_qty < 0) ? 0 : static_cast<Quantity>(new_qty);
    }
};

}

#endif
