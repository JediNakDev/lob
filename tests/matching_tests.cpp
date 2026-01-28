#include "matching_tests.hpp"
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

void test_aggressive_buy_matches_asks() {
    OrderBook book;
    
    (void)book.add_order(101.0, 100, Side::SELL);
    (void)book.add_order(102.0, 100, Side::SELL);
    
    auto result = book.add_order(101.0, 50, Side::BUY);
    
    assert(result.fills.size() == 1);
    assert(result.fills[0].quantity == 50);
    assert(approx_eq(result.fills[0].price, 101.0));
    assert(result.remaining_quantity == 0);
    assert(book.get_ask_quantity_at_top() == 50);
}

void test_aggressive_sell_matches_bids() {
    OrderBook book;
    
    (void)book.add_order(100.0, 100, Side::BUY);
    (void)book.add_order(99.0, 100, Side::BUY);
    
    auto result = book.add_order(100.0, 50, Side::SELL);
    
    assert(result.fills.size() == 1);
    assert(result.fills[0].quantity == 50);
    assert(approx_eq(result.fills[0].price, 100.0));
    assert(result.remaining_quantity == 0);
    assert(book.get_bid_quantity_at_top() == 50);
}

void test_partial_fill_rests_on_book() {
    OrderBook book;
    
    (void)book.add_order(101.0, 30, Side::SELL);
    
    auto result = book.add_order(101.0, 50, Side::BUY);
    
    assert(result.fills.size() == 1);
    assert(result.fills[0].quantity == 30);
    assert(result.remaining_quantity == 20);
    assert(book.get_total_orders() == 1);
    assert(book.get_bid_quantity_at_top() == 20);
}

void test_sweep_multiple_price_levels() {
    OrderBook book;
    
    (void)book.add_order(101.0, 50, Side::SELL);
    (void)book.add_order(102.0, 50, Side::SELL);
    (void)book.add_order(103.0, 50, Side::SELL);
    
    auto result = book.add_order(103.0, 120, Side::BUY);
    
    assert(result.fills.size() == 3);
    assert(result.fills[0].quantity == 50);
    assert(approx_eq(result.fills[0].price, 101.0));
    assert(result.fills[1].quantity == 50);
    assert(approx_eq(result.fills[1].price, 102.0));
    assert(result.fills[2].quantity == 20);
    assert(approx_eq(result.fills[2].price, 103.0));
    assert(result.remaining_quantity == 0);
}

void test_fifo_matching_order() {
    OrderBook book;
    
    auto r1 = book.add_order(100.0, 50, Side::BUY);
    (void)book.add_order(100.0, 50, Side::BUY);
    
    auto result = book.add_order(100.0, 30, Side::SELL);
    
    assert(result.fills.size() == 1);
    assert(result.fills[0].buy_order_id == r1.order_id);
    assert(book.get_bid_quantity_at_top() == 70);
}

void test_price_priority() {
    OrderBook book;
    
    (void)book.add_order(99.0, 50, Side::BUY);
    (void)book.add_order(100.0, 50, Side::BUY);
    (void)book.add_order(98.0, 50, Side::BUY);
    
    auto result = book.add_order(98.0, 30, Side::SELL);
    
    assert(result.fills.size() == 1);
    assert(approx_eq(result.fills[0].price, 100.0));
}

void test_no_cross_when_price_doesnt_match() {
    OrderBook book;
    
    (void)book.add_order(100.0, 50, Side::BUY);
    
    auto result = book.add_order(101.0, 50, Side::SELL);
    
    assert(result.fills.empty());
    assert(result.remaining_quantity == 50);
    assert(book.get_total_orders() == 2);
}

void run_matching_tests() {
    std::cout << "[Matching Tests]\n";
    RUN_TEST(test_aggressive_buy_matches_asks);
    RUN_TEST(test_aggressive_sell_matches_bids);
    RUN_TEST(test_partial_fill_rests_on_book);
    RUN_TEST(test_sweep_multiple_price_levels);
    RUN_TEST(test_fifo_matching_order);
    RUN_TEST(test_price_priority);
    RUN_TEST(test_no_cross_when_price_doesnt_match);
    std::cout << "\n";
}
