#ifndef LOB_TYPES_HPP
#define LOB_TYPES_HPP

#include <cstdint>

namespace lob {

enum class Side { BUY, SELL };

using OrderId = uint64_t;
using Quantity = uint64_t;
using Timestamp = uint64_t;

// Use integer price in cents/ticks for faster comparison and hashing
// This avoids floating-point comparison issues and enables better hash performance
using Price = int64_t;

struct Fill {
    OrderId buy_order_id;
    OrderId sell_order_id;
    Price price;
    Quantity quantity;
};

}

#endif
