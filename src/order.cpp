#include "order.hpp"
#include <sstream>
#include <stdexcept>

Order::Order(uint64_t id, double p, int qty, const std::string& s, const std::string& t)
    : order_id(id), price(p), quantity(qty), side(s), type(t), timestamp(get_current_timestamp()) {
    
    // Validate side
    if (side != "BUY" && side != "SELL") {
        throw std::invalid_argument("Order side must be 'BUY' or 'SELL'");
    }
    
    // Validate type
    if (type != "LIMIT" && type != "MARKET") {
        throw std::invalid_argument("Order type must be 'LIMIT' or 'MARKET'");
    }
    
    // Validate quantity
    if (quantity <= 0) {
        throw std::invalid_argument("Order quantity must be positive");
    }
    
    // Validate price for limit orders
    if (type == "LIMIT" && price <= 0.0) {
        throw std::invalid_argument("Limit order price must be positive");
    }
}

Order Order::create_limit_order(uint64_t id, double price, int quantity, const std::string& side) {
    return Order(id, price, quantity, side, "LIMIT");
}

Order Order::create_market_order(uint64_t id, int quantity, const std::string& side) {
    return Order(id, 0.0, quantity, side, "MARKET");
}

bool Order::is_buy() const {
    return side == "BUY";
}

bool Order::is_sell() const {
    return side == "SELL";
}

bool Order::is_limit() const {
    return type == "LIMIT";
}

bool Order::is_market() const {
    return type == "MARKET";
}

std::string Order::to_string() const {
    std::stringstream ss;
    ss << "Order[ID=" << order_id 
       << ", " << side << " " << type;
    
    if (is_limit()) {
        ss << " " << quantity << "@" << price;
    } else {
        ss << " " << quantity << "@MARKET";
    }
    
    ss << ", TS=" << timestamp << "]";
    return ss.str();
}

bool Order::operator<(const Order& other) const {
    // First compare by timestamp for time priority
    return timestamp < other.timestamp;
}

bool Order::operator==(const Order& other) const {
    return order_id == other.order_id;
}

uint64_t Order::get_current_timestamp() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}
