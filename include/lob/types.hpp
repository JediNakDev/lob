#ifndef LOB_TYPES_HPP
#define LOB_TYPES_HPP

#include <cstdint>

namespace lob {

enum class Side { BUY, SELL };

using OrderId = uint64_t;
using Quantity = uint64_t;
using Timestamp = uint64_t;
using Price = double;

struct Fill {
    OrderId buy_order_id;
    OrderId sell_order_id;
    Price price;
    Quantity quantity;
};

}

#endif
