#pragma once

#include <string>
#include <chrono>

class Order {
public:
    uint64_t order_id;
    double price;
    int quantity;
    std::string side;      // "BUY" or "SELL"
    std::string type;      // "LIMIT" or "MARKET"
    uint64_t timestamp;
    
    // Constructor
    Order(uint64_t id, double p, int qty, const std::string& s, const std::string& t);
    
    // Default constructor
    Order() = default;
    
    // Copy constructor and assignment operator
    Order(const Order& other) = default;
    Order& operator=(const Order& other) = default;
    
    // Factory methods
    static Order create_limit_order(uint64_t id, double price, int quantity, const std::string& side);
    static Order create_market_order(uint64_t id, int quantity, const std::string& side);
    
    // Utility methods
    bool is_buy() const;
    bool is_sell() const;
    bool is_limit() const;
    bool is_market() const;
    
    // String representation
    std::string to_string() const;
    
    // Comparison operators for sorting
    bool operator<(const Order& other) const;
    bool operator==(const Order& other) const;
    
private:
    static uint64_t get_current_timestamp();
};
