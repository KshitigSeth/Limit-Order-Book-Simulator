#pragma once

#include "order.hpp"
#include "order_book.hpp"
#include <vector>
#include <functional>

struct Fill {
    uint64_t buy_order_id;
    uint64_t sell_order_id;
    double price;
    int quantity;
    uint64_t timestamp;
    
    std::string to_string() const;
};

using FillCallback = std::function<void(const Fill&)>;

class MatchingEngine {
public:
    explicit MatchingEngine(FillCallback callback = nullptr);
    
    // Core matching functionality
    std::vector<Fill> process_order(const Order& order);
    
    // Order book access
    const OrderBook& get_order_book() const { return order_book; }
    OrderBook& get_order_book() { return order_book; }
    
    // Cancel and modify orders
    bool cancel_order(uint64_t order_id);
    bool modify_order(uint64_t order_id, int new_quantity);
    
    // Statistics
    size_t total_fills() const { return fill_count; }
    double total_volume() const { return total_traded_volume; }
    
private:
    OrderBook order_book;
    FillCallback fill_callback;
    size_t fill_count = 0;
    double total_traded_volume = 0.0;
    
    // Matching algorithms
    std::vector<Fill> match_limit_order(const Order& order);
    std::vector<Fill> match_market_order(const Order& order);
    
    // Helper methods
    Fill create_fill(const Order& aggressive_order, const Order& passive_order, 
                    double fill_price, int fill_quantity);
    void notify_fill(const Fill& fill);
    void update_statistics(const Fill& fill);
    
    // Price-time priority matching
    bool can_match(const Order& buy_order, const Order& sell_order) const;
    double determine_fill_price(const Order& aggressive_order, const Order& passive_order) const;
};
