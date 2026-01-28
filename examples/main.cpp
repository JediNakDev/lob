#include "lob/order_book.hpp"
#include <iostream>
#include <iomanip>

using namespace lob;

void print_fill(const Fill& fill) {
    std::cout << "    FILL: " << fill.quantity << " @ " 
              << std::fixed << std::setprecision(2) << fill.price
              << " (Buy #" << fill.buy_order_id 
              << " <-> Sell #" << fill.sell_order_id << ")\n";
}

void print_book(const OrderBook& book) {
    auto snapshot = book.get_snapshot(5);
    
    std::cout << "\n╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║                      ORDER BOOK                          ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";
    
    std::cout << "║ ASKS (Sell Orders)                                       ║\n";
    std::cout << "╟──────────────────────────────────────────────────────────╢\n";
    
    for (auto it = snapshot.asks.rbegin(); it != snapshot.asks.rend(); ++it) {
        std::cout << "║  Price: " << std::fixed << std::setprecision(2) 
                  << std::setw(8) << it->price
                  << " | Qty: " << std::setw(6) << it->quantity
                  << " | Orders: " << std::setw(3) << it->order_count
                  << "              ║\n";
    }
    
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";
    
    auto spread = book.get_spread();
    auto mid = book.get_mid_price();
    auto best_bid = book.get_best_bid();
    auto best_ask = book.get_best_ask();
    
    std::cout << "║ Spread: " << std::setw(6) << (spread ? *spread : 0.0)
              << " | Mid: " << std::setw(8) << (mid ? *mid : 0.0)
              << " | Bid: " << std::setw(8) << (best_bid ? *best_bid : 0.0)
              << " | Ask: " << std::setw(8) << (best_ask ? *best_ask : 0.0)
              << " ║\n";
    
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";
    
    std::cout << "║ BIDS (Buy Orders)                                        ║\n";
    std::cout << "╟──────────────────────────────────────────────────────────╢\n";
    
    for (const auto& level : snapshot.bids) {
        std::cout << "║  Price: " << std::fixed << std::setprecision(2) 
                  << std::setw(8) << level.price
                  << " | Qty: " << std::setw(6) << level.quantity
                  << " | Orders: " << std::setw(3) << level.order_count
                  << "              ║\n";
    }
    
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    std::cout << "Total Orders: " << book.get_total_orders() 
              << " | Bid Levels: " << book.get_bid_levels()
              << " | Ask Levels: " << book.get_ask_levels() << "\n\n";
}

OrderId add_and_print(OrderBook& book, Price price, Quantity qty, Side side) {
    std::cout << "ADD: " << (side == Side::BUY ? "BUY" : "SELL")
              << " " << qty << " @ " << std::fixed << std::setprecision(2) << price << "\n";
    
    auto result = book.add_order(price, qty, side);
    
    for (const auto& fill : result.fills) {
        print_fill(fill);
    }
    
    if (result.remaining_quantity == 0) {
        std::cout << "  -> Fully filled (Order #" << result.order_id << ")\n";
    } else {
        std::cout << "  -> Resting (Order #" << result.order_id 
                  << ", remaining: " << result.remaining_quantity << ")\n";
    }
    
    return result.order_id;
}

int main() {
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "           LIMIT ORDER BOOK - DEMONSTRATION                    \n";
    std::cout << "═══════════════════════════════════════════════════════════════\n\n";

    OrderBook book;

    std::cout << ">>> TEST 1: Adding resting orders to build the book\n";
    std::cout << "───────────────────────────────────────────────────────────────\n";
    
    add_and_print(book, 100.00, 100, Side::BUY);
    add_and_print(book, 99.50, 150, Side::BUY);
    add_and_print(book, 99.00, 200, Side::BUY);
    add_and_print(book, 100.00, 50, Side::BUY);
    
    add_and_print(book, 101.00, 100, Side::SELL);
    add_and_print(book, 101.50, 150, Side::SELL);
    add_and_print(book, 102.00, 200, Side::SELL);
    add_and_print(book, 101.00, 75, Side::SELL);
    
    print_book(book);

    std::cout << ">>> TEST 2: Aggressive BUY order that crosses the spread\n";
    std::cout << "───────────────────────────────────────────────────────────────\n";
    add_and_print(book, 101.50, 120, Side::BUY);
    print_book(book);

    std::cout << ">>> TEST 3: Aggressive SELL order that crosses the spread\n";
    std::cout << "───────────────────────────────────────────────────────────────\n";
    add_and_print(book, 99.50, 80, Side::SELL);
    print_book(book);

    std::cout << ">>> TEST 4: Cancel order #3\n";
    std::cout << "───────────────────────────────────────────────────────────────\n";
    bool cancelled = book.cancel_order(3);
    std::cout << "Cancel order #3: " << (cancelled ? "SUCCESS" : "FAILED") << "\n";
    print_book(book);

    std::cout << ">>> TEST 5: Modify order #2 quantity to 300\n";
    std::cout << "───────────────────────────────────────────────────────────────\n";
    bool modified = book.modify_order(2, 300);
    std::cout << "Modify order #2: " << (modified ? "SUCCESS" : "FAILED") << "\n";
    print_book(book);

    std::cout << ">>> TEST 6: Large aggressive order sweeping multiple levels\n";
    std::cout << "───────────────────────────────────────────────────────────────\n";
    add_and_print(book, 95.00, 1000, Side::SELL);
    print_book(book);

    std::cout << ">>> TEST 7: Rebuilding the book\n";
    std::cout << "───────────────────────────────────────────────────────────────\n";
    add_and_print(book, 98.00, 500, Side::BUY);
    add_and_print(book, 97.50, 300, Side::BUY);
    add_and_print(book, 97.00, 400, Side::BUY);
    add_and_print(book, 103.00, 500, Side::SELL);
    add_and_print(book, 103.50, 300, Side::SELL);
    add_and_print(book, 104.00, 400, Side::SELL);
    print_book(book);

    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "                    DEMONSTRATION COMPLETE                     \n";
    std::cout << "═══════════════════════════════════════════════════════════════\n";

    return 0;
}
