#include "order_book.hpp"
#include "utils/logger.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>

void OrderBook::add_order(const Order& order) {
    if (!order.is_limit()) {
        LOG_ERROR("Cannot add market order to order book directly");
        return;
    }
    
    double price = order.price;
    
    if (order.is_buy()) {
        buy_orders[price].push_back(order);
        order_locations[order.order_id] = {price, true};
        LOG_DEBUG("Added buy order " + order.to_string() + " to price level " + std::to_string(price));
    } else {
        sell_orders[price].push_back(order);
        order_locations[order.order_id] = {price, false};
        LOG_DEBUG("Added sell order " + order.to_string() + " to price level " + std::to_string(price));
    }
}

bool OrderBook::cancel_order(uint64_t order_id) {
    auto it = order_locations.find(order_id);
    if (it == order_locations.end()) {
        LOG_DEBUG("Order " + std::to_string(order_id) + " not found for cancellation");
        return false;
    }
    
    double price = it->second.first;
    bool is_buy = it->second.second;
    
    remove_order_from_level(order_id, price, is_buy);
    order_locations.erase(it);
    clean_empty_levels();
    
    LOG_INFO("Cancelled order " + std::to_string(order_id));
    return true;
}

bool OrderBook::modify_order(uint64_t order_id, int new_quantity) {
    auto it = order_locations.find(order_id);
    if (it == order_locations.end()) {
        LOG_DEBUG("Order " + std::to_string(order_id) + " not found for modification");
        return false;
    }
    
    if (new_quantity <= 0) {
        LOG_ERROR("Invalid quantity for modification: " + std::to_string(new_quantity));
        return false;
    }
    
    double price = it->second.first;
    bool is_buy = it->second.second;
    
    // Find and modify the order
    auto& orders = is_buy ? buy_orders[price] : sell_orders[price];
    
    for (auto& order : orders) {
        if (order.order_id == order_id) {
            int old_quantity = order.quantity;
            order.quantity = new_quantity;
            LOG_INFO("Modified order " + std::to_string(order_id) + 
                    " quantity from " + std::to_string(old_quantity) + 
                    " to " + std::to_string(new_quantity));
            return true;
        }
    }
    
    return false;
}

TopOfBook OrderBook::get_top_of_book() const {
    TopOfBook tob;
    
    if (!buy_orders.empty()) {
        auto best_buy_it = buy_orders.begin();
        tob.best_bid = best_buy_it->first;
        tob.bid_quantity = calculate_level_quantity(best_buy_it->second);
    }
    
    if (!sell_orders.empty()) {
        auto best_sell_it = sell_orders.begin();
        tob.best_ask = best_sell_it->first;
        tob.ask_quantity = calculate_level_quantity(best_sell_it->second);
    }
    
    return tob;
}

std::vector<PriceLevel> OrderBook::get_bid_levels(int depth) const {
    std::vector<PriceLevel> levels;
    
    int count = 0;
    for (const auto& [price, orders] : buy_orders) {
        if (count >= depth || orders.empty()) break;
        
        levels.push_back({
            price,
            calculate_level_quantity(orders),
            static_cast<int>(orders.size())
        });
        count++;
    }
    
    return levels;
}

std::vector<PriceLevel> OrderBook::get_ask_levels(int depth) const {
    std::vector<PriceLevel> levels;
    
    int count = 0;
    for (const auto& [price, orders] : sell_orders) {
        if (count >= depth || orders.empty()) break;
        
        levels.push_back({
            price,
            calculate_level_quantity(orders),
            static_cast<int>(orders.size())
        });
        count++;
    }
    
    return levels;
}

void OrderBook::print_book(int depth) const {
    std::cout << "\n=== ORDER BOOK ===" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    
    // Print ask levels (sells) in reverse order
    auto ask_levels = get_ask_levels(depth);
    std::reverse(ask_levels.begin(), ask_levels.end());
    
    for (const auto& level : ask_levels) {
        std::cout << std::setw(12) << "" 
                  << std::setw(8) << level.total_quantity 
                  << " @ " << std::setw(8) << level.price 
                  << " SELL (" << level.order_count << " orders)" << std::endl;
    }
    
    std::cout << std::setfill('-') << std::setw(50) << "" << std::setfill(' ') << std::endl;
    
    // Print bid levels (buys)
    auto bid_levels = get_bid_levels(depth);
    for (const auto& level : bid_levels) {
        std::cout << "BUY (" << level.order_count << " orders) " 
                  << std::setw(8) << level.price 
                  << " @ " << std::setw(8) << level.total_quantity << std::endl;
    }
    
    TopOfBook tob = get_top_of_book();
    if (tob.best_bid && tob.best_ask) {
        double spread = *tob.best_ask - *tob.best_bid;
        std::cout << "\nSpread: " << spread << " (" 
                  << std::setprecision(4) << (spread / *tob.best_bid * 100) 
                  << "%)" << std::endl;
    }
    std::cout << "==================\n" << std::endl;
}

size_t OrderBook::total_orders() const {
    size_t count = 0;
    for (const auto& [price, orders] : buy_orders) {
        count += orders.size();
    }
    for (const auto& [price, orders] : sell_orders) {
        count += orders.size();
    }
    return count;
}

bool OrderBook::empty() const {
    return buy_orders.empty() && sell_orders.empty();
}

void OrderBook::remove_order_from_level(uint64_t order_id, double price, bool is_buy) {
    auto& orders = is_buy ? buy_orders[price] : sell_orders[price];
    
    orders.erase(
        std::remove_if(orders.begin(), orders.end(),
            [order_id](const Order& order) { return order.order_id == order_id; }),
        orders.end()
    );
}

void OrderBook::clean_empty_levels() {
    // Clean empty buy levels
    for (auto it = buy_orders.begin(); it != buy_orders.end();) {
        if (it->second.empty()) {
            it = buy_orders.erase(it);
        } else {
            ++it;
        }
    }
    
    // Clean empty sell levels
    for (auto it = sell_orders.begin(); it != sell_orders.end();) {
        if (it->second.empty()) {
            it = sell_orders.erase(it);
        } else {
            ++it;
        }
    }
}

int OrderBook::calculate_level_quantity(const std::deque<Order>& orders) const {
    int total = 0;
    for (const auto& order : orders) {
        total += order.quantity;
    }
    return total;
}
