#include "matching_engine.hpp"
#include "utils/logger.hpp"
#include <sstream>
#include <algorithm>
#include <limits>

std::string Fill::to_string() const {
    std::stringstream ss;
    ss << "Fill[BuyID=" << buy_order_id 
       << ", SellID=" << sell_order_id
       << ", Price=" << price
       << ", Qty=" << quantity
       << ", TS=" << timestamp << "]";
    return ss.str();
}

MatchingEngine::MatchingEngine(FillCallback callback) 
    : fill_callback(callback) {
    LOG_INFO("MatchingEngine initialized");
}

std::vector<Fill> MatchingEngine::process_order(const Order& order) {
    LOG_INFO("Processing order: " + order.to_string());
    
    std::vector<Fill> fills;
    
    if (order.is_limit()) {
        fills = match_limit_order(order);
    } else if (order.is_market()) {
        fills = match_market_order(order);
    } else {
        LOG_ERROR("Unknown order type: " + order.type);
        return fills;
    }
    
    // Notify about fills
    for (const auto& fill : fills) {
        notify_fill(fill);
        update_statistics(fill);
    }
    
    LOG_INFO("Generated " + std::to_string(fills.size()) + " fills");
    return fills;
}

bool MatchingEngine::cancel_order(uint64_t order_id) {
    return order_book.cancel_order(order_id);
}

bool MatchingEngine::modify_order(uint64_t order_id, int new_quantity) {
    return order_book.modify_order(order_id, new_quantity);
}

std::vector<Fill> MatchingEngine::match_limit_order(const Order& order) {
    std::vector<Fill> fills;
    Order remaining_order = order;
    
    if (order.is_buy()) {
        auto& sell_orders = order_book.get_sell_orders();
        
        for (auto it = sell_orders.begin(); 
             it != sell_orders.end() && remaining_order.quantity > 0;) {
            
            double ask_price = it->first;
            auto& sell_queue = it->second;
            
            // Check if we can match at this price level
            if (remaining_order.price < ask_price) {
                break; // No more matching possible
            }
            
            // Match orders at this price level (FIFO)
            while (!sell_queue.empty() && remaining_order.quantity > 0) {
                Order& passive_order = sell_queue.front();
                
                int fill_quantity = std::min(remaining_order.quantity, passive_order.quantity);
                double fill_price = determine_fill_price(remaining_order, passive_order);
                
                Fill fill = create_fill(remaining_order, passive_order, fill_price, fill_quantity);
                fills.push_back(fill);
                
                // Update quantities
                remaining_order.quantity -= fill_quantity;
                passive_order.quantity -= fill_quantity;
                
                // Remove fully filled orders
                if (passive_order.quantity == 0) {
                    LOG_DEBUG("Removing fully filled passive sell order " + 
                             std::to_string(passive_order.order_id));
                    sell_queue.pop_front();
                }
            }
            
            // Remove empty price levels
            if (sell_queue.empty()) {
                it = sell_orders.erase(it);
            } else {
                ++it;
            }
        }
    } else { // Sell order
        auto& buy_orders = order_book.get_buy_orders();
        
        for (auto it = buy_orders.begin(); 
             it != buy_orders.end() && remaining_order.quantity > 0;) {
            
            double bid_price = it->first;
            auto& buy_queue = it->second;
            
            // Check if we can match at this price level
            if (remaining_order.price > bid_price) {
                break; // No more matching possible
            }
            
            // Match orders at this price level (FIFO)
            while (!buy_queue.empty() && remaining_order.quantity > 0) {
                Order& passive_order = buy_queue.front();
                
                int fill_quantity = std::min(remaining_order.quantity, passive_order.quantity);
                double fill_price = determine_fill_price(remaining_order, passive_order);
                
                Fill fill = create_fill(passive_order, remaining_order, fill_price, fill_quantity);
                fills.push_back(fill);
                
                // Update quantities
                remaining_order.quantity -= fill_quantity;
                passive_order.quantity -= fill_quantity;
                
                // Remove fully filled orders
                if (passive_order.quantity == 0) {
                    LOG_DEBUG("Removing fully filled passive buy order " + 
                             std::to_string(passive_order.order_id));
                    buy_queue.pop_front();
                }
            }
            
            // Remove empty price levels
            if (buy_queue.empty()) {
                it = buy_orders.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    // Add remaining quantity to book if any
    if (remaining_order.quantity > 0) {
        LOG_DEBUG("Adding remaining quantity " + std::to_string(remaining_order.quantity) + 
                 " to order book");
        order_book.add_order(remaining_order);
    }
    
    return fills;
}

std::vector<Fill> MatchingEngine::match_market_order(const Order& order) {
    std::vector<Fill> fills;
    Order remaining_order = order;
    
    if (order.is_buy()) {
        auto& sell_orders = order_book.get_sell_orders();
        
        for (auto it = sell_orders.begin(); 
             it != sell_orders.end() && remaining_order.quantity > 0;) {
            
            auto& sell_queue = it->second;
            
            // Match orders at this price level (FIFO)
            while (!sell_queue.empty() && remaining_order.quantity > 0) {
                Order& passive_order = sell_queue.front();
                
                int fill_quantity = std::min(remaining_order.quantity, passive_order.quantity);
                double fill_price = passive_order.price; // Market order takes the passive price
                
                Fill fill = create_fill(remaining_order, passive_order, fill_price, fill_quantity);
                fills.push_back(fill);
                
                // Update quantities
                remaining_order.quantity -= fill_quantity;
                passive_order.quantity -= fill_quantity;
                
                // Remove fully filled orders
                if (passive_order.quantity == 0) {
                    sell_queue.pop_front();
                }
            }
            
            // Remove empty price levels
            if (sell_queue.empty()) {
                it = sell_orders.erase(it);
            } else {
                ++it;
            }
        }
    } else { // Market sell order
        auto& buy_orders = order_book.get_buy_orders();
        
        for (auto it = buy_orders.begin(); 
             it != buy_orders.end() && remaining_order.quantity > 0;) {
            
            auto& buy_queue = it->second;
            
            // Match orders at this price level (FIFO)
            while (!buy_queue.empty() && remaining_order.quantity > 0) {
                Order& passive_order = buy_queue.front();
                
                int fill_quantity = std::min(remaining_order.quantity, passive_order.quantity);
                double fill_price = passive_order.price; // Market order takes the passive price
                
                Fill fill = create_fill(passive_order, remaining_order, fill_price, fill_quantity);
                fills.push_back(fill);
                
                // Update quantities
                remaining_order.quantity -= fill_quantity;
                passive_order.quantity -= fill_quantity;
                
                // Remove fully filled orders
                if (passive_order.quantity == 0) {
                    buy_queue.pop_front();
                }
            }
            
            // Remove empty price levels
            if (buy_queue.empty()) {
                it = buy_orders.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    // Market orders that can't be filled are rejected
    if (remaining_order.quantity > 0) {
        LOG_ERROR("Market order " + std::to_string(order.order_id) + 
                 " partially rejected - remaining quantity: " + 
                 std::to_string(remaining_order.quantity));
    }
    
    return fills;
}

Fill MatchingEngine::create_fill(const Order& aggressive_order, const Order& passive_order,
                                double fill_price, int fill_quantity) {
    Fill fill;
    fill.buy_order_id = aggressive_order.is_buy() ? aggressive_order.order_id : passive_order.order_id;
    fill.sell_order_id = aggressive_order.is_sell() ? aggressive_order.order_id : passive_order.order_id;
    fill.price = fill_price;
    fill.quantity = fill_quantity;
    fill.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    return fill;
}

void MatchingEngine::notify_fill(const Fill& fill) {
    if (fill_callback) {
        fill_callback(fill);
    }
}

void MatchingEngine::update_statistics(const Fill& fill) {
    fill_count++;
    total_traded_volume += fill.price * fill.quantity;
}

bool MatchingEngine::can_match(const Order& buy_order, const Order& sell_order) const {
    return buy_order.price >= sell_order.price;
}

double MatchingEngine::determine_fill_price(const Order& /* aggressive_order */, 
                                           const Order& passive_order) const {
    // Price-time priority: passive order price takes precedence
    return passive_order.price;
}
