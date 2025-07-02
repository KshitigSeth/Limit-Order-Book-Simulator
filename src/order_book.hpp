#pragma once

#include "order.hpp"
#include <map>
#include <deque>
#include <vector>
#include <optional>
#include <unordered_map>

struct TopOfBook {
    std::optional<double> best_bid;
    std::optional<double> best_ask;
    std::optional<int> bid_quantity;
    std::optional<int> ask_quantity;
};

struct PriceLevel {
    double price;
    int total_quantity;
    int order_count;
};

class OrderBook {
public:
    OrderBook() = default;
    
    // Core order management
    void add_order(const Order& order);
    bool cancel_order(uint64_t order_id);
    bool modify_order(uint64_t order_id, int new_quantity);
    
    // Book information
    TopOfBook get_top_of_book() const;
    std::vector<PriceLevel> get_bid_levels(int depth = 5) const;
    std::vector<PriceLevel> get_ask_levels(int depth = 5) const;
    
    // Display
    void print_book(int depth = 5) const;
    
    // Internal access for matching engine
    std::map<double, std::deque<Order>, std::greater<double>>& get_buy_orders() { return buy_orders; }
    std::map<double, std::deque<Order>>& get_sell_orders() { return sell_orders; }
    
    const std::map<double, std::deque<Order>, std::greater<double>>& get_buy_orders() const { return buy_orders; }
    const std::map<double, std::deque<Order>>& get_sell_orders() const { return sell_orders; }
    
    // Statistics
    size_t total_orders() const;
    bool empty() const;
    
private:
    // Buy orders: price -> queue of orders (sorted descending by price)
    std::map<double, std::deque<Order>, std::greater<double>> buy_orders;
    
    // Sell orders: price -> queue of orders (sorted ascending by price)
    std::map<double, std::deque<Order>> sell_orders;
    
    // Order ID to location mapping for fast cancellation
    std::unordered_map<uint64_t, std::pair<double, bool>> order_locations; // price, is_buy
    
    // Helper methods
    void remove_order_from_level(uint64_t order_id, double price, bool is_buy);
    void clean_empty_levels();
    int calculate_level_quantity(const std::deque<Order>& orders) const;
};
