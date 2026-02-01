#include <lob/order_book.hpp>
#include <algorithm>

namespace lob {

OrderBook::OrderBook() 
    : best_bid_(nullptr)
    , best_ask_(nullptr)
    , next_order_id_(1) 
{}

OrderBook::~OrderBook() = default;

// Insert a bid level into the sorted linked list (highest price first)
void OrderBook::insert_bid_level(PriceLevel* level) {
    if (!best_bid_) {
        // First bid level
        best_bid_ = level;
        level->prev_level = nullptr;
        level->next_level = nullptr;
        return;
    }
    
    // Find insertion point (bids sorted high to low)
    if (level->price > best_bid_->price) {
        // New best bid
        level->next_level = best_bid_;
        level->prev_level = nullptr;
        best_bid_->prev_level = level;
        best_bid_ = level;
        return;
    }
    
    // Insert in sorted position (traverse from best)
    PriceLevel* current = best_bid_;
    while (current->next_level && current->next_level->price > level->price) {
        current = current->next_level;
    }
    
    level->next_level = current->next_level;
    level->prev_level = current;
    if (current->next_level) {
        current->next_level->prev_level = level;
    }
    current->next_level = level;
}

// Insert an ask level into the sorted linked list (lowest price first)
void OrderBook::insert_ask_level(PriceLevel* level) {
    if (!best_ask_) {
        // First ask level
        best_ask_ = level;
        level->prev_level = nullptr;
        level->next_level = nullptr;
        return;
    }
    
    // Find insertion point (asks sorted low to high)
    if (level->price < best_ask_->price) {
        // New best ask
        level->next_level = best_ask_;
        level->prev_level = nullptr;
        best_ask_->prev_level = level;
        best_ask_ = level;
        return;
    }
    
    // Insert in sorted position (traverse from best)
    PriceLevel* current = best_ask_;
    while (current->next_level && current->next_level->price < level->price) {
        current = current->next_level;
    }
    
    level->next_level = current->next_level;
    level->prev_level = current;
    if (current->next_level) {
        current->next_level->prev_level = level;
    }
    current->next_level = level;
}

// O(1) - Remove bid level from linked list
void OrderBook::remove_bid_level(PriceLevel* level) {
    if (level->prev_level) {
        level->prev_level->next_level = level->next_level;
    } else {
        // This was best_bid_
        best_bid_ = level->next_level;
    }
    
    if (level->next_level) {
        level->next_level->prev_level = level->prev_level;
    }
    
    level->prev_level = nullptr;
    level->next_level = nullptr;
}

// O(1) - Remove ask level from linked list
void OrderBook::remove_ask_level(PriceLevel* level) {
    if (level->prev_level) {
        level->prev_level->next_level = level->next_level;
    } else {
        // This was best_ask_
        best_ask_ = level->next_level;
    }
    
    if (level->next_level) {
        level->next_level->prev_level = level->prev_level;
    }
    
    level->prev_level = nullptr;
    level->next_level = nullptr;
}

void OrderBook::add_order_to_book(Order* order) {
    if (order->side == Side::BUY) {
        auto it = bid_levels_.find(order->price);
        if (it == bid_levels_.end()) {
            // O(log M) amortized - create new price level and insert in sorted position
            auto level = std::make_unique<PriceLevel>(order->price);
            PriceLevel* level_ptr = level.get();
            bid_levels_[order->price] = std::move(level);
            insert_bid_level(level_ptr);
            level_ptr->add_order(order);
        } else {
            // O(1) - add to existing level
            it->second->add_order(order);
        }
    } else {
        auto it = ask_levels_.find(order->price);
        if (it == ask_levels_.end()) {
            // O(log M) amortized - create new price level
            auto level = std::make_unique<PriceLevel>(order->price);
            PriceLevel* level_ptr = level.get();
            ask_levels_[order->price] = std::move(level);
            insert_ask_level(level_ptr);
            level_ptr->add_order(order);
        } else {
            // O(1) - add to existing level
            it->second->add_order(order);
        }
    }
}

void OrderBook::remove_order_from_book(Order* order) {
    PriceLevel* level = order->parent_level;
    if (!level) return;
    
    // O(1) - remove from intrusive linked list
    level->remove_order(order);
    
    // Remove empty price level
    if (level->is_empty()) {
        if (order->side == Side::BUY) {
            remove_bid_level(level);
            bid_levels_.erase(level->price);
        } else {
            remove_ask_level(level);
            ask_levels_.erase(level->price);
        }
    }
}

std::vector<Fill> OrderBook::match_order(Order* incoming) {
    std::vector<Fill> fills;

    if (incoming->side == Side::BUY) {
        // Match against asks (lowest first)
        while (!incoming->is_filled() && best_ask_) {
            PriceLevel* ask_level = best_ask_;
            
            if (incoming->price < ask_level->price) break;
            
            while (!incoming->is_filled() && !ask_level->is_empty()) {
                Order* resting = ask_level->front();
                Quantity fill_qty = std::min(incoming->remaining_quantity, 
                                             resting->remaining_quantity);
                
                fills.push_back(Fill{
                    incoming->id,
                    resting->id,
                    ask_level->price,
                    fill_qty
                });
                
                incoming->fill(fill_qty);
                resting->fill(fill_qty);
                ask_level->update_quantity(-static_cast<int64_t>(fill_qty));
                
                if (resting->is_filled()) {
                    OrderId resting_id = resting->id;
                    ask_level->pop_front();
                    orders_.erase(resting_id);
                }
            }
            
            if (ask_level->is_empty()) {
                Price level_price = ask_level->price;
                remove_ask_level(ask_level);
                ask_levels_.erase(level_price);
            }
        }
    } else {
        // Match against bids (highest first)
        while (!incoming->is_filled() && best_bid_) {
            PriceLevel* bid_level = best_bid_;
            
            if (incoming->price > bid_level->price) break;
            
            while (!incoming->is_filled() && !bid_level->is_empty()) {
                Order* resting = bid_level->front();
                Quantity fill_qty = std::min(incoming->remaining_quantity, 
                                             resting->remaining_quantity);
                
                fills.push_back(Fill{
                    resting->id,
                    incoming->id,
                    bid_level->price,
                    fill_qty
                });
                
                incoming->fill(fill_qty);
                resting->fill(fill_qty);
                bid_level->update_quantity(-static_cast<int64_t>(fill_qty));
                
                if (resting->is_filled()) {
                    OrderId resting_id = resting->id;
                    bid_level->pop_front();
                    orders_.erase(resting_id);
                }
            }
            
            if (bid_level->is_empty()) {
                Price level_price = bid_level->price;
                remove_bid_level(bid_level);
                bid_levels_.erase(level_price);
            }
        }
    }

    return fills;
}

OrderBook::AddResult OrderBook::add_order(Price price, Quantity quantity, Side side) {
    auto order = std::make_unique<Order>(next_order_id_++, price, quantity, side);
    Order* order_ptr = order.get();
    
    std::vector<Fill> fills = match_order(order_ptr);
    
    if (!order_ptr->is_filled()) {
        orders_[order_ptr->id] = std::move(order);
        add_order_to_book(order_ptr);
    }
    
    return AddResult{order_ptr->id, std::move(fills), order_ptr->remaining_quantity};
}

bool OrderBook::cancel_order(OrderId order_id) {
    auto it = orders_.find(order_id);
    if (it == orders_.end()) {
        return false;
    }

    Order* order = it->second.get();
    
    // O(1) - remove from book using intrusive linked list
    remove_order_from_book(order);
    
    orders_.erase(it);
    return true;
}

bool OrderBook::modify_order(OrderId order_id, Quantity new_quantity) {
    auto it = orders_.find(order_id);
    if (it == orders_.end()) {
        return false;
    }

    Order* order = it->second.get();
    PriceLevel* level = order->parent_level;
    
    if (level) {
        int64_t qty_diff = static_cast<int64_t>(new_quantity) - 
                           static_cast<int64_t>(order->remaining_quantity);
        level->update_quantity(qty_diff);
    }

    order->remaining_quantity = new_quantity;
    order->quantity = new_quantity;
    return true;
}

// O(1) - direct pointer access
std::optional<Price> OrderBook::get_best_bid() const {
    if (!best_bid_) return std::nullopt;
    return best_bid_->price;
}

// O(1) - direct pointer access
std::optional<Price> OrderBook::get_best_ask() const {
    if (!best_ask_) return std::nullopt;
    return best_ask_->price;
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
    return (*bid + *ask) / 2;
}

Quantity OrderBook::get_bid_quantity_at_top() const {
    if (!best_bid_) return 0;
    return best_bid_->total_volume;
}

Quantity OrderBook::get_ask_quantity_at_top() const {
    if (!best_ask_) return 0;
    return best_ask_->total_volume;
}

OrderBook::BookSnapshot OrderBook::get_snapshot(size_t depth) const {
    BookSnapshot snapshot;
    
    // Traverse bid linked list (already sorted high to low)
    size_t count = 0;
    PriceLevel* level = best_bid_;
    while (level && count < depth) {
        snapshot.bids.push_back({
            level->price,
            level->total_volume,
            level->order_count()
        });
        level = level->next_level;
        ++count;
    }
    
    // Traverse ask linked list (already sorted low to high)
    count = 0;
    level = best_ask_;
    while (level && count < depth) {
        snapshot.asks.push_back({
            level->price,
            level->total_volume,
            level->order_count()
        });
        level = level->next_level;
        ++count;
    }
    
    return snapshot;
}

}
