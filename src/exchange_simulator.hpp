#pragma once

#include "matching_engine.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>

class ExchangeSimulator {
public:
    ExchangeSimulator();
    
    // Simulation modes
    void run_simulation(int duration_seconds = 10, int orders_per_second = 2);
    void run_interactive_mode();
    
    // Access to matching engine
    MatchingEngine& get_engine() { return engine; }
    const MatchingEngine& get_engine() const { return engine; }
    
private:
    MatchingEngine engine;
    
    // Command handlers
    void handle_add_command(std::istringstream& iss, uint64_t order_id);
    void handle_cancel_command(std::istringstream& iss);
    void handle_modify_command(std::istringstream& iss);
    
    // Utility methods
    void print_statistics() const;
    void on_fill(const Fill& fill);
    
    // Random order generation
    Order generate_random_order(uint64_t order_id,
        std::uniform_real_distribution<>& price_dist,
        std::uniform_int_distribution<>& quantity_dist,
        std::uniform_int_distribution<>& side_dist,
        std::uniform_int_distribution<>& type_dist,
        std::mt19937& gen);
};
