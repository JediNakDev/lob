#ifndef LOB_PRICE_LEVEL_HPP
#define LOB_PRICE_LEVEL_HPP

#include "order.hpp"

namespace lob {

/**
 * PriceLevel (Limit) - represents a single limit price in the order book.
 * 
 * Structure:
 * - Binary tree node (parent/leftChild/rightChild) for O(log M) level insertion
 * - Doubly linked list of orders (headOrder/tailOrder) for O(1) order operations
 * - Hash map lookup by price gives O(1) access to existing levels
 * 
 * Performance:
 * - Add order to existing level: O(1)
 * - Add first order at new level: O(log M) where M = number of price levels
 * - Cancel order: O(1)
 * - Execute order: O(1)
 * - GetVolumeAtLimit: O(1)
 */
class PriceLevel {
public:
    // Price and aggregate data
    Price price;
    Quantity total_volume;      // Total quantity at this price level
    size_t order_count_;        // Number of orders at this level

    // Binary tree pointers for sorted price levels
    PriceLevel* parent;
    PriceLevel* left_child;     // Lower prices
    PriceLevel* right_child;    // Higher prices

    // Doubly linked list of orders at this price
    Order* head_order;
    Order* tail_order;

    explicit PriceLevel(Price price_) noexcept
        : price(price_)
        , total_volume(0)
        , order_count_(0)
        , parent(nullptr)
        , left_child(nullptr)
        , right_child(nullptr)
        , head_order(nullptr)
        , tail_order(nullptr)
    {}

    // Add order to tail of the order list - O(1)
    void add_order(Order* order) noexcept {
        order->parent_level = this;
        order->prev_order = tail_order;
        order->next_order = nullptr;
        
        if (tail_order) {
            tail_order->next_order = order;
        } else {
            head_order = order;
        }
        tail_order = order;
        
        total_volume += order->remaining_quantity;
        ++order_count_;
    }

    // Remove order from list - O(1)
    void remove_order(Order* order) noexcept {
        if (order->prev_order) {
            order->prev_order->next_order = order->next_order;
        } else {
            head_order = order->next_order;
        }
        
        if (order->next_order) {
            order->next_order->prev_order = order->prev_order;
        } else {
            tail_order = order->prev_order;
        }
        
        total_volume -= order->remaining_quantity;
        --order_count_;
        
        order->prev_order = nullptr;
        order->next_order = nullptr;
        order->parent_level = nullptr;
    }

    [[nodiscard]] Order* front() const noexcept {
        return head_order;
    }

    // Pop front order - O(1) for execute operations
    void pop_front() noexcept {
        if (head_order) {
            Order* old_head = head_order;
            total_volume -= old_head->remaining_quantity;
            --order_count_;
            
            head_order = old_head->next_order;
            if (head_order) {
                head_order->prev_order = nullptr;
            } else {
                tail_order = nullptr;
            }
            
            old_head->prev_order = nullptr;
            old_head->next_order = nullptr;
            old_head->parent_level = nullptr;
        }
    }

    [[nodiscard]] bool is_empty() const noexcept { 
        return head_order == nullptr; 
    }

    [[nodiscard]] size_t order_count() const noexcept { 
        return order_count_; 
    }

    void update_quantity(int64_t delta) noexcept {
        int64_t new_qty = static_cast<int64_t>(total_volume) + delta;
        total_volume = (new_qty < 0) ? 0 : static_cast<Quantity>(new_qty);
    }

    // BST helper: check if this is a left child
    [[nodiscard]] bool is_left_child() const noexcept {
        return parent && parent->left_child == this;
    }

    // BST helper: check if this is a right child
    [[nodiscard]] bool is_right_child() const noexcept {
        return parent && parent->right_child == this;
    }
};

}

#endif
