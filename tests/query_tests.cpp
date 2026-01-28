#include "query_tests.hpp"
#include "test_framework.hpp"
#include "../include/lob/order_book.hpp"
#include <cassert>
#include <cmath>

using namespace lob;

namespace {
    bool approx_eq(double a, double b, double eps = 0.0001) {
        return std::fabs(a - b) < eps;
    }
}

void test_best_bid_ask() {
    OrderBook book;
    
    (void)book.add_order(100.0, 50, Side::BUY);
    (void)book.add_order(99.0, 50, Side::BUY);
    (void)book.add_order(101.0, 50, Side::SELL);
    (void)book.add_order(102.0, 50, Side::SELL);
    
    assert(book.get_best_bid().has_value());
    assert(approx_eq(*book.get_best_bid(), 100.0));
    assert(book.get_best_ask().has_value());
    assert(approx_eq(*book.get_best_ask(), 101.0));
}

void test_spread_and_mid_price() {
    OrderBook book;
    
    (void)book.add_order(100.0, 50, Side::BUY);
    (void)book.add_order(102.0, 50, Side::SELL);
    
    assert(book.get_spread().has_value());
    assert(approx_eq(*book.get_spread(), 2.0));
    assert(book.get_mid_price().has_value());
    assert(approx_eq(*book.get_mid_price(), 101.0));
}

void test_empty_book_returns_nullopt() {
    OrderBook book;
    
    assert(!book.get_best_bid().has_value());
    assert(!book.get_best_ask().has_value());
    assert(!book.get_spread().has_value());
    assert(!book.get_mid_price().has_value());
}

void test_snapshot() {
    OrderBook book;
    
    (void)book.add_order(100.0, 50, Side::BUY);
    (void)book.add_order(99.0, 60, Side::BUY);
    (void)book.add_order(101.0, 70, Side::SELL);
    (void)book.add_order(102.0, 80, Side::SELL);
    
    auto snapshot = book.get_snapshot(5);
    
    assert(snapshot.bids.size() == 2);
    assert(snapshot.asks.size() == 2);
    assert(approx_eq(snapshot.bids[0].price, 100.0));
    assert(snapshot.bids[0].quantity == 50);
    assert(approx_eq(snapshot.asks[0].price, 101.0));
    assert(snapshot.asks[0].quantity == 70);
}

void run_query_tests() {
    std::cout << "[Query Tests]\n";
    RUN_TEST(test_best_bid_ask);
    RUN_TEST(test_spread_and_mid_price);
    RUN_TEST(test_empty_book_returns_nullopt);
    RUN_TEST(test_snapshot);
    std::cout << "\n";
}
