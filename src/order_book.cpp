#include "../include/lob/order_book.hpp"
#include <optional>

namespace lob {

OrderBook::OrderBook() : next_order_id_(1) {}

std::vector<Fill> OrderBook::match_order(std::shared_ptr<Order> incoming) {
    std::vector<Fill> fills;

    if (incoming->side == Side::BUY) {
        while (!incoming->is_filled() && !asks_.empty()) {
            auto it = asks_.begin();
            Price ask_price = it->first;
            PriceLevel& ask_level = it->second;
            
            if (incoming->price < ask_price) break;
            
            while (!incoming->is_filled() && !ask_level.is_empty()) {
                auto resting = ask_level.front();
                Quantity fill_qty = std::min(incoming->remaining_quantity, 
                                             resting->remaining_quantity);
                
                fills.push_back(Fill{
                    incoming->id,
                    resting->id,
                    ask_price,
                    fill_qty
                });
                
                incoming->fill(fill_qty);
                resting->fill(fill_qty);
                ask_level.update_quantity(-static_cast<int64_t>(fill_qty));
                
                if (resting->is_filled()) {
                    ask_level.pop_front();
                    orders_.erase(resting->id);
                }
            }
            
            if (ask_level.is_empty()) {
                asks_.erase(ask_price);
            }
        }
    } else {
        while (!incoming->is_filled() && !bids_.empty()) {
            auto it = bids_.begin();
            Price bid_price = it->first;
            PriceLevel& bid_level = it->second;
            
            if (incoming->price > bid_price) break;
            
            while (!incoming->is_filled() && !bid_level.is_empty()) {
                auto resting = bid_level.front();
                Quantity fill_qty = std::min(incoming->remaining_quantity, 
                                             resting->remaining_quantity);
                
                fills.push_back(Fill{
                    resting->id,
                    incoming->id,
                    bid_price,
                    fill_qty
                });
                
                incoming->fill(fill_qty);
                resting->fill(fill_qty);
                bid_level.update_quantity(-static_cast<int64_t>(fill_qty));
                
                if (resting->is_filled()) {
                    bid_level.pop_front();
                    orders_.erase(resting->id);
                }
            }
            
            if (bid_level.is_empty()) {
                bids_.erase(bid_price);
            }
        }
    }

    return fills;
}

OrderBook::AddResult OrderBook::add_order(Price price, Quantity quantity, Side side) {
    auto order = std::make_shared<Order>(next_order_id_++, price, quantity, side);
    
    std::vector<Fill> fills = match_order(order);
    
    if (!order->is_filled()) {
        if (side == Side::BUY) {
            auto it = bids_.find(price);
            if (it == bids_.end()) {
                it = bids_.emplace(price, PriceLevel(price)).first;
            }
            it->second.add_order(order);
        } else {
            auto it = asks_.find(price);
            if (it == asks_.end()) {
                it = asks_.emplace(price, PriceLevel(price)).first;
            }
            it->second.add_order(order);
        }
        orders_[order->id] = order;
    }
    
    return AddResult{order->id, std::move(fills), order->remaining_quantity};
}

bool OrderBook::cancel_order(OrderId order_id) {
    auto it = orders_.find(order_id);
    if (it == orders_.end()) {
        return false;
    }

    auto order = it->second;

    if (order->side == Side::BUY) {
        auto level_it = bids_.find(order->price);
        if (level_it != bids_.end()) {
            level_it->second.remove_order(order);
            if (level_it->second.is_empty()) {
                bids_.erase(level_it);
            }
        }
    } else {
        auto level_it = asks_.find(order->price);
        if (level_it != asks_.end()) {
            level_it->second.remove_order(order);
            if (level_it->second.is_empty()) {
                asks_.erase(level_it);
            }
        }
    }

    orders_.erase(it);
    return true;
}

bool OrderBook::modify_order(OrderId order_id, Quantity new_quantity) {
    auto it = orders_.find(order_id);
    if (it == orders_.end()) {
        return false;
    }

    auto order = it->second;
    int64_t qty_diff = static_cast<int64_t>(new_quantity) - 
                       static_cast<int64_t>(order->remaining_quantity);

    if (order->side == Side::BUY) {
        auto level_it = bids_.find(order->price);
        if (level_it != bids_.end()) {
            level_it->second.update_quantity(qty_diff);
        }
    } else {
        auto level_it = asks_.find(order->price);
        if (level_it != asks_.end()) {
            level_it->second.update_quantity(qty_diff);
        }
    }

    order->remaining_quantity = new_quantity;
    order->quantity = new_quantity;
    return true;
}

std::optional<Price> OrderBook::get_best_bid() const {
    if (bids_.empty()) return std::nullopt;
    return bids_.begin()->first;
}

std::optional<Price> OrderBook::get_best_ask() const {
    if (asks_.empty()) return std::nullopt;
    return asks_.begin()->first;
}

std::optional<Price> OrderBook::get_spread() const {
    auto bid = get_best_bid();
    auto ask = get_best_ask();
    if (!bid || !ask) return std::nullopt;
    return *ask - *bid;
}

std::optional<Price> OrderBook::get_mid_price() const {
    auto bid = get_best_bid();
    auto ask = get_best_ask();
    if (!bid || !ask) return std::nullopt;
    return (*bid + *ask) / 2.0;
}

Quantity OrderBook::get_bid_quantity_at_top() const {
    if (bids_.empty()) return 0;
    return bids_.begin()->second.total_quantity;
}

Quantity OrderBook::get_ask_quantity_at_top() const {
    if (asks_.empty()) return 0;
    return asks_.begin()->second.total_quantity;
}

OrderBook::BookSnapshot OrderBook::get_snapshot(size_t depth) const {
    BookSnapshot snapshot;
    
    size_t count = 0;
    for (auto it = bids_.begin(); it != bids_.end() && count < depth; ++it, ++count) {
        snapshot.bids.push_back({
            it->first,
            it->second.total_quantity,
            it->second.order_count()
        });
    }
    
    count = 0;
    for (auto it = asks_.begin(); it != asks_.end() && count < depth; ++it, ++count) {
        snapshot.asks.push_back({
            it->first,
            it->second.total_quantity,
            it->second.order_count()
        });
    }
    
    return snapshot;
}

}
