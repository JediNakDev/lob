#include <lob/order_book.hpp>
#include <algorithm>

namespace lob {

OrderBook::OrderBook() 
    : buy_tree_(nullptr)
    , sell_tree_(nullptr)
    , highest_buy_(nullptr)
    , lowest_sell_(nullptr)
    , next_order_id_(1) 
{}

OrderBook::~OrderBook() = default;

// ============================================================================
// BST Helper Functions
// ============================================================================

PriceLevel* OrderBook::find_min(PriceLevel* node) const {
    if (!node) return nullptr;
    while (node->left_child) {
        node = node->left_child;
    }
    return node;
}

PriceLevel* OrderBook::find_max(PriceLevel* node) const {
    if (!node) return nullptr;
    while (node->right_child) {
        node = node->right_child;
    }
    return node;
}

// Find in-order successor (next higher price)
PriceLevel* OrderBook::find_successor(PriceLevel* node) const {
    if (!node) return nullptr;
    
    // If right subtree exists, successor is the minimum in right subtree
    if (node->right_child) {
        return find_min(node->right_child);
    }
    
    // Otherwise, go up until we find a node that is a left child
    PriceLevel* parent = node->parent;
    while (parent && node == parent->right_child) {
        node = parent;
        parent = parent->parent;
    }
    return parent;
}

// Find in-order predecessor (next lower price)
PriceLevel* OrderBook::find_predecessor(PriceLevel* node) const {
    if (!node) return nullptr;
    
    // If left subtree exists, predecessor is the maximum in left subtree
    if (node->left_child) {
        return find_max(node->left_child);
    }
    
    // Otherwise, go up until we find a node that is a right child
    PriceLevel* parent = node->parent;
    while (parent && node == parent->left_child) {
        node = parent;
        parent = parent->parent;
    }
    return parent;
}

// Replace subtree rooted at u with subtree rooted at v (bid tree)
void OrderBook::transplant_bid(PriceLevel* u, PriceLevel* v) {
    if (!u->parent) {
        buy_tree_ = v;
    } else if (u->is_left_child()) {
        u->parent->left_child = v;
    } else {
        u->parent->right_child = v;
    }
    if (v) {
        v->parent = u->parent;
    }
}

// Replace subtree rooted at u with subtree rooted at v (ask tree)
void OrderBook::transplant_ask(PriceLevel* u, PriceLevel* v) {
    if (!u->parent) {
        sell_tree_ = v;
    } else if (u->is_left_child()) {
        u->parent->left_child = v;
    } else {
        u->parent->right_child = v;
    }
    if (v) {
        v->parent = u->parent;
    }
}

// ============================================================================
// BST Insert Operations - O(log M)
// ============================================================================

void OrderBook::insert_bid_level(PriceLevel* level) {
    level->parent = nullptr;
    level->left_child = nullptr;
    level->right_child = nullptr;
    
    PriceLevel* parent = nullptr;
    PriceLevel* current = buy_tree_;
    
    // Find insertion point
    while (current) {
        parent = current;
        if (level->price < current->price) {
            current = current->left_child;
        } else {
            current = current->right_child;
        }
    }
    
    level->parent = parent;
    
    if (!parent) {
        // Tree was empty
        buy_tree_ = level;
    } else if (level->price < parent->price) {
        parent->left_child = level;
    } else {
        parent->right_child = level;
    }
    
    // Update highest_buy_ (best bid) - O(1) check
    if (!highest_buy_ || level->price > highest_buy_->price) {
        highest_buy_ = level;
    }
}

void OrderBook::insert_ask_level(PriceLevel* level) {
    level->parent = nullptr;
    level->left_child = nullptr;
    level->right_child = nullptr;
    
    PriceLevel* parent = nullptr;
    PriceLevel* current = sell_tree_;
    
    // Find insertion point
    while (current) {
        parent = current;
        if (level->price < current->price) {
            current = current->left_child;
        } else {
            current = current->right_child;
        }
    }
    
    level->parent = parent;
    
    if (!parent) {
        // Tree was empty
        sell_tree_ = level;
    } else if (level->price < parent->price) {
        parent->left_child = level;
    } else {
        parent->right_child = level;
    }
    
    // Update lowest_sell_ (best ask) - O(1) check
    if (!lowest_sell_ || level->price < lowest_sell_->price) {
        lowest_sell_ = level;
    }
}

// ============================================================================
// BST Remove Operations - O(log M)
// ============================================================================

void OrderBook::remove_bid_level(PriceLevel* level) {
    // Update highest_buy_ before removal
    if (level == highest_buy_) {
        highest_buy_ = find_predecessor(level);
    }
    
    // Standard BST deletion
    if (!level->left_child) {
        transplant_bid(level, level->right_child);
    } else if (!level->right_child) {
        transplant_bid(level, level->left_child);
    } else {
        // Node has two children - find successor
        PriceLevel* successor = find_min(level->right_child);
        if (successor->parent != level) {
            transplant_bid(successor, successor->right_child);
            successor->right_child = level->right_child;
            successor->right_child->parent = successor;
        }
        transplant_bid(level, successor);
        successor->left_child = level->left_child;
        successor->left_child->parent = successor;
    }
    
    // Clear pointers
    level->parent = nullptr;
    level->left_child = nullptr;
    level->right_child = nullptr;
}

void OrderBook::remove_ask_level(PriceLevel* level) {
    // Update lowest_sell_ before removal
    if (level == lowest_sell_) {
        lowest_sell_ = find_successor(level);
    }
    
    // Standard BST deletion
    if (!level->left_child) {
        transplant_ask(level, level->right_child);
    } else if (!level->right_child) {
        transplant_ask(level, level->left_child);
    } else {
        // Node has two children - find successor
        PriceLevel* successor = find_min(level->right_child);
        if (successor->parent != level) {
            transplant_ask(successor, successor->right_child);
            successor->right_child = level->right_child;
            successor->right_child->parent = successor;
        }
        transplant_ask(level, successor);
        successor->left_child = level->left_child;
        successor->left_child->parent = successor;
    }
    
    // Clear pointers
    level->parent = nullptr;
    level->left_child = nullptr;
    level->right_child = nullptr;
}

// ============================================================================
// Order Operations
// ============================================================================

void OrderBook::add_order_to_book(Order* order) {
    if (order->side == Side::BUY) {
        auto it = bid_levels_.find(order->price);
        if (it == bid_levels_.end()) {
            // New price level - O(log M) insertion
            auto level = std::make_unique<PriceLevel>(order->price);
            PriceLevel* level_ptr = level.get();
            bid_levels_[order->price] = std::move(level);
            insert_bid_level(level_ptr);
            level_ptr->add_order(order);  // O(1)
        } else {
            // Existing level - O(1)
            it->second->add_order(order);
        }
    } else {
        auto it = ask_levels_.find(order->price);
        if (it == ask_levels_.end()) {
            // New price level - O(log M) insertion
            auto level = std::make_unique<PriceLevel>(order->price);
            PriceLevel* level_ptr = level.get();
            ask_levels_[order->price] = std::move(level);
            insert_ask_level(level_ptr);
            level_ptr->add_order(order);  // O(1)
        } else {
            // Existing level - O(1)
            it->second->add_order(order);
        }
    }
}

void OrderBook::remove_order_from_book(Order* order) {
    PriceLevel* level = order->parent_level;
    if (!level) return;
    
    level->remove_order(order);  // O(1)
    
    if (level->is_empty()) {
        if (order->side == Side::BUY) {
            remove_bid_level(level);  // O(log M)
            bid_levels_.erase(level->price);
        } else {
            remove_ask_level(level);  // O(log M)
            ask_levels_.erase(level->price);
        }
    }
}

// ============================================================================
// Matching Engine - O(1) per fill
// ============================================================================

std::vector<Fill> OrderBook::match_order(Order* incoming) {
    std::vector<Fill> fills;

    if (incoming->side == Side::BUY) {
        while (!incoming->is_filled() && lowest_sell_) {
            PriceLevel* ask_level = lowest_sell_;
            
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
        while (!incoming->is_filled() && highest_buy_) {
            PriceLevel* bid_level = highest_buy_;
            
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

// ============================================================================
// Public API
// ============================================================================

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

// O(1) - cached pointer
std::optional<Price> OrderBook::get_best_bid() const {
    if (!highest_buy_) return std::nullopt;
    return highest_buy_->price;
}

// O(1) - cached pointer
std::optional<Price> OrderBook::get_best_ask() const {
    if (!lowest_sell_) return std::nullopt;
    return lowest_sell_->price;
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
    if (!highest_buy_) return 0;
    return highest_buy_->total_volume;
}

Quantity OrderBook::get_ask_quantity_at_top() const {
    if (!lowest_sell_) return 0;
    return lowest_sell_->total_volume;
}

// ============================================================================
// Snapshot - uses in-order BST traversal
// ============================================================================

OrderBook::BookSnapshot OrderBook::get_snapshot(size_t depth) const {
    BookSnapshot snapshot;
    
    // Bids: start from highest (best) and go down
    size_t count = 0;
    PriceLevel* level = highest_buy_;
    while (level && count < depth) {
        snapshot.bids.push_back({
            level->price,
            level->total_volume,
            level->order_count()
        });
        level = find_predecessor(level);
        ++count;
    }
    
    // Asks: start from lowest (best) and go up
    count = 0;
    level = lowest_sell_;
    while (level && count < depth) {
        snapshot.asks.push_back({
            level->price,
            level->total_volume,
            level->order_count()
        });
        level = find_successor(level);
        ++count;
    }
    
    return snapshot;
}

}
