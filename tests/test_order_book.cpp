// Basic test framework for order book functionality
// Run with: make test

#include "order.hpp"
#include "order_book.hpp"
#include "matching_engine.hpp"
#include "utils/logger.hpp"
#include <iostream>
#include <cassert>
#include <stdexcept>

// Define logger static member
LogLevel Logger::current_level = LogLevel::LOG_ERROR;

// Simple test framework
void test_order_creation() {
    std::cout << "Testing order creation...";
    
    // Valid limit order
    Order order1 = Order::create_limit_order(1, 100.50, 200, "BUY");
    assert(order1.order_id == 1);
    assert(order1.price == 100.50);
    assert(order1.quantity == 200);
    assert(order1.side == "BUY");
    assert(order1.type == "LIMIT");
    assert(order1.is_buy());
    assert(order1.is_limit());
    assert(!order1.is_sell());
    assert(!order1.is_market());
    
    // Valid market order
    Order order2 = Order::create_market_order(2, 100, "SELL");
    assert(order2.order_id == 2);
    assert(order2.price == 0.0);
    assert(order2.quantity == 100);
    assert(order2.side == "SELL");
    assert(order2.type == "MARKET");
    assert(order2.is_sell());
    assert(order2.is_market());
    
    // Test invalid parameters
    try {
        Order bad_order(1, 100.0, -10, "BUY", "LIMIT");
        assert(false); // Should not reach here
    } catch (const std::invalid_argument&) {
        // Expected
    }
    
    std::cout << " PASSED\n";
}

void test_orderbook_basic() {
    std::cout << "Testing order book basics...";
    
    OrderBook book;
    assert(book.empty());
    assert(book.total_orders() == 0);
    
    TopOfBook tob = book.get_top_of_book();
    assert(!tob.best_bid.has_value());
    assert(!tob.best_ask.has_value());
    
    // Add buy order
    Order buy_order = Order::create_limit_order(1, 100.50, 200, "BUY");
    book.add_order(buy_order);
    assert(!book.empty());
    assert(book.total_orders() == 1);
    
    tob = book.get_top_of_book();
    assert(tob.best_bid.has_value());
    assert(tob.best_bid.value() == 100.50);
    assert(tob.bid_quantity.value() == 200);
    
    std::cout << " PASSED\n";
}

void test_order_cancellation() {
    std::cout << "Testing order cancellation...";
    
    OrderBook book;
    Order order1 = Order::create_limit_order(1, 100.50, 200, "BUY");
    Order order2 = Order::create_limit_order(2, 100.50, 300, "BUY");
    
    book.add_order(order1);
    book.add_order(order2);
    assert(book.total_orders() == 2);
    
    assert(book.cancel_order(2));
    assert(book.total_orders() == 1);
    
    TopOfBook tob = book.get_top_of_book();
    assert(tob.bid_quantity.value() == 200);
    
    assert(!book.cancel_order(999)); // Non-existent order
    
    std::cout << " PASSED\n";
}

void test_matching_basic() {
    std::cout << "Testing basic matching...";
    
    MatchingEngine engine;
    
    // No match case
    Order buy_order = Order::create_limit_order(1, 100.00, 200, "BUY");
    Order sell_order = Order::create_limit_order(2, 101.00, 150, "SELL");
    
    auto fills1 = engine.process_order(buy_order);
    auto fills2 = engine.process_order(sell_order);
    
    assert(fills1.empty());
    assert(fills2.empty());
    assert(engine.get_order_book().total_orders() == 2);
    
    // Perfect match case
    MatchingEngine engine2;
    Order buy_order2 = Order::create_limit_order(1, 100.50, 200, "BUY");
    Order sell_order2 = Order::create_limit_order(2, 100.50, 200, "SELL");
    
    auto fills3 = engine2.process_order(buy_order2);
    auto fills4 = engine2.process_order(sell_order2);
    
    assert(fills3.empty());
    assert(fills4.size() == 1);
    
    Fill fill = fills4[0];
    assert(fill.buy_order_id == 1);
    assert(fill.sell_order_id == 2);
    assert(fill.price == 100.50);
    assert(fill.quantity == 200);
    
    assert(engine2.get_order_book().empty());
    
    std::cout << " PASSED\n";
}

int main() {
    std::cout << "Running Order Book Tests\n";
    std::cout << "========================\n\n";
    
    try {

        test_order_creation();
        test_orderbook_basic();
        test_order_cancellation();
        test_matching_basic();
        
        std::cout << "\nAll tests passed successfully!\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "\nTest failed: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "\nTest failed with unknown exception\n";
        return 1;
    }
}
