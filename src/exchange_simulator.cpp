#include "exchange_simulator.hpp"
#include "utils/logger.hpp"
#include <random>
#include <thread>
#include <chrono>

ExchangeSimulator::ExchangeSimulator() : engine([this](const Fill& fill) {
    this->on_fill(fill);
}) {
    LOG_INFO("ExchangeSimulator initialized");
}

void ExchangeSimulator::run_simulation(int duration_seconds, int orders_per_second) {
    LOG_INFO("Starting simulation: " + std::to_string(duration_seconds) + 
             " seconds, " + std::to_string(orders_per_second) + " orders/sec");
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Price distribution around $100
    std::uniform_real_distribution<> price_dist(95.0, 105.0);
    std::uniform_int_distribution<> quantity_dist(10, 1000);
    std::uniform_int_distribution<> side_dist(0, 1); // 0 = BUY, 1 = SELL
    std::uniform_int_distribution<> type_dist(0, 9); // 90% limit, 10% market
    
    uint64_t order_id = 1;
    auto start_time = std::chrono::steady_clock::now();
    auto simulation_end = start_time + std::chrono::seconds(duration_seconds);
    
    int tick = 0;
    while (std::chrono::steady_clock::now() < simulation_end) {
        tick++;
        std::cout << "\n=== TICK " << tick << " ===" << std::endl;
        
        // Generate orders for this tick
        for (int i = 0; i < orders_per_second; ++i) {
            Order order = generate_random_order(order_id++, price_dist, quantity_dist, 
                                              side_dist, type_dist, gen);
            
            std::cout << "Submitting: " << order.to_string() << std::endl;
            auto fills = engine.process_order(order);
            
            if (!fills.empty()) {
                std::cout << "Generated " << fills.size() << " fills:" << std::endl;
                for (const auto& fill : fills) {
                    std::cout << "  " << fill.to_string() << std::endl;
                }
            }
        }
        
        // Print order book state
        engine.get_order_book().print_book(3);
        
        // Print statistics
        print_statistics();
        
        // Sleep to simulate real-time
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
    LOG_INFO("Simulation completed");
}

void ExchangeSimulator::run_interactive_mode() {
    std::cout << "\n=== INTERACTIVE MODE ===" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  ADD <SIDE> <TYPE> <PRICE> <QUANTITY> - Add order" << std::endl;
    std::cout << "  CANCEL <ORDER_ID> - Cancel order" << std::endl;
    std::cout << "  MODIFY <ORDER_ID> <QUANTITY> - Modify order quantity" << std::endl;
    std::cout << "  BOOK - Show order book" << std::endl;
    std::cout << "  STATS - Show statistics" << std::endl;
    std::cout << "  QUIT - Exit" << std::endl;
    std::cout << "\nExample: ADD BUY LIMIT 100.50 200\n" << std::endl;
    
    std::string command;
    uint64_t order_id = 1;
    
    while (std::getline(std::cin, command)) {
        if (command.empty()) continue;
        
        std::istringstream iss(command);
        std::string cmd;
        iss >> cmd;
        
        if (cmd == "QUIT" || cmd == "quit" || cmd == "q") {
            break;
        } else if (cmd == "ADD" || cmd == "add") {
            handle_add_command(iss, order_id++);
        } else if (cmd == "CANCEL" || cmd == "cancel") {
            handle_cancel_command(iss);
        } else if (cmd == "MODIFY" || cmd == "modify") {
            handle_modify_command(iss);
        } else if (cmd == "BOOK" || cmd == "book") {
            engine.get_order_book().print_book();
        } else if (cmd == "STATS" || cmd == "stats") {
            print_statistics();
        } else {
            std::cout << "Unknown command: " << cmd << std::endl;
        }
    }
}

void ExchangeSimulator::handle_add_command(std::istringstream& iss, uint64_t order_id) {
    std::string side, type;
    double price = 0.0;
    int quantity;
    
    if (!(iss >> side >> type >> price >> quantity)) {
        std::cout << "Invalid ADD command format" << std::endl;
        return;
    }
    
    try {
        Order order;
        if (type == "LIMIT" || type == "limit") {
            order = Order::create_limit_order(order_id, price, quantity, side);
        } else if (type == "MARKET" || type == "market") {
            order = Order::create_market_order(order_id, quantity, side);
        } else {
            std::cout << "Invalid order type: " << type << std::endl;
            return;
        }
        
        std::cout << "Adding order: " << order.to_string() << std::endl;
        auto fills = engine.process_order(order);
        
        if (!fills.empty()) {
            std::cout << "Generated " << fills.size() << " fills:" << std::endl;
            for (const auto& fill : fills) {
                std::cout << "  " << fill.to_string() << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cout << "Error creating order: " << e.what() << std::endl;
    }
}

void ExchangeSimulator::handle_cancel_command(std::istringstream& iss) {
    uint64_t order_id;
    if (!(iss >> order_id)) {
        std::cout << "Invalid CANCEL command format" << std::endl;
        return;
    }
    
    if (engine.cancel_order(order_id)) {
        std::cout << "Order " << order_id << " cancelled successfully" << std::endl;
    } else {
        std::cout << "Order " << order_id << " not found" << std::endl;
    }
}

void ExchangeSimulator::handle_modify_command(std::istringstream& iss) {
    uint64_t order_id;
    int new_quantity;
    
    if (!(iss >> order_id >> new_quantity)) {
        std::cout << "Invalid MODIFY command format" << std::endl;
        return;
    }
    
    if (engine.modify_order(order_id, new_quantity)) {
        std::cout << "Order " << order_id << " modified successfully" << std::endl;
    } else {
        std::cout << "Order " << order_id << " not found or invalid quantity" << std::endl;
    }
}

void ExchangeSimulator::print_statistics() const {
    std::cout << "\n=== STATISTICS ===" << std::endl;
    std::cout << "Total Fills: " << engine.total_fills() << std::endl;
    std::cout << "Total Volume: $" << std::fixed << std::setprecision(2) 
              << engine.total_volume() << std::endl;
    std::cout << "Orders in Book: " << engine.get_order_book().total_orders() << std::endl;
    
    TopOfBook tob = engine.get_order_book().get_top_of_book();
    if (tob.best_bid && tob.best_ask) {
        std::cout << "Best Bid: " << *tob.best_bid << " (" << *tob.bid_quantity << ")" << std::endl;
        std::cout << "Best Ask: " << *tob.best_ask << " (" << *tob.ask_quantity << ")" << std::endl;
        std::cout << "Spread: " << (*tob.best_ask - *tob.best_bid) << std::endl;
    } else {
        std::cout << "No top of book available" << std::endl;
    }
    std::cout << "==================\n" << std::endl;
}

void ExchangeSimulator::on_fill(const Fill& fill) {
    // This callback is called whenever a fill occurs
    // Can be used for real-time processing, logging, etc.
    LOG_INFO("Fill executed: " + fill.to_string());
}

Order ExchangeSimulator::generate_random_order(uint64_t order_id,
    std::uniform_real_distribution<>& price_dist,
    std::uniform_int_distribution<>& quantity_dist,
    std::uniform_int_distribution<>& side_dist,
    std::uniform_int_distribution<>& type_dist,
    std::mt19937& gen) {
    
    std::string side = (side_dist(gen) == 0) ? "BUY" : "SELL";
    bool is_market = (type_dist(gen) == 0); // 10% chance for market order
    
    if (is_market) {
        return Order::create_market_order(order_id, quantity_dist(gen), side);
    } else {
        return Order::create_limit_order(order_id, price_dist(gen), quantity_dist(gen), side);
    }
}
