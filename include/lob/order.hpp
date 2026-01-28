#ifndef LOB_ORDER_HPP
#define LOB_ORDER_HPP

#include "types.hpp"
#include <chrono>

namespace lob {

class Order {
public:
    OrderId id;
    Price price;
    Quantity quantity;
    Quantity remaining_quantity;
    Side side;
    Timestamp timestamp;

    Order(OrderId id, Price price, Quantity quantity, Side side)
        : id(id)
        , price(price)
        , quantity(quantity)
        , remaining_quantity(quantity)
        , side(side)
        , timestamp(static_cast<Timestamp>(
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
