#ifndef LOB_PRICE_LEVEL_HPP
#define LOB_PRICE_LEVEL_HPP

#include "order.hpp"

namespace lob {

/**
 * PriceLevel following the article's design:
 * - Intrusive doubly-linked list of orders (head/tail pointers)
 * - All operations are O(1)
 * - Parent/child pointers could be added for tree-based implementation
 *   but we use hash map + linked list approach for O(1) operations
 */
class PriceLevel {
public:
    Price price;
    Quantity total_volume;
    size_t order_count_;
    
    // Doubly-linked list head/tail for FIFO order matching
    Order* head_order;
    Order* tail_order;
    
    // For linked list of price levels (enables O(1) best bid/ask updates)
    PriceLevel* prev_level;
    PriceLevel* next_level;

    explicit PriceLevel(Price price_) noexcept
        : price(price_)
        , total_volume(0)
        , order_count_(0)
        , head_order(nullptr)
        , tail_order(nullptr)
        , prev_level(nullptr)
        , next_level(nullptr)
    {}

    // O(1) - Add order to tail of the list
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

    // O(1) - Remove order from anywhere in the list
    void remove_order(Order* order) noexcept {
        // Unlink from doubly-linked list
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

    // O(1) - Get first order (for matching)
    [[nodiscard]] Order* front() const noexcept {
        return head_order;
    }

    // O(1) - Remove first order (after full fill)
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
};

}

#endif
