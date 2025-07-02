#include "src/exchange_simulator.hpp"
#include "src/utils/logger.hpp"
#include <iostream>
#include <string>

// Define the static member to avoid multiple definitions
LogLevel Logger::current_level = LogLevel::LOG_INFO;

void print_usage() {
    std::cout << "\nLimit Order Book Simulator\n";
    std::cout << "==========================\n";
    std::cout << "Usage: ./lob_simulator [mode]\n\n";
    std::cout << "Modes:\n";
    std::cout << "  interactive  - Interactive command line mode (default)\n";
    std::cout << "  simulation   - Run automated simulation\n";
    std::cout << "  help         - Show this help message\n\n";
    std::cout << "Interactive Commands:\n";
    std::cout << "  ADD <SIDE> <TYPE> <PRICE> <QUANTITY>\n";
    std::cout << "    Example: ADD BUY LIMIT 100.50 200\n";
    std::cout << "    Example: ADD SELL MARKET 0 100\n";
    std::cout << "  CANCEL <ORDER_ID>\n";
    std::cout << "  MODIFY <ORDER_ID> <NEW_QUANTITY>\n";
    std::cout << "  BOOK    - Show order book\n";
    std::cout << "  STATS   - Show statistics\n";
    std::cout << "  QUIT    - Exit\n\n";
}

int main(int argc, char* argv[]) {
    // Set logging level
    Logger::current_level = LogLevel::LOG_INFO;
    
    std::string mode = "interactive";
    if (argc > 1) {
        mode = argv[1];
    }
    
    if (mode == "help" || mode == "--help" || mode == "-h") {
        print_usage();
        return 0;
    }
    
    try {
        ExchangeSimulator simulator;
        
        if (mode == "simulation") {
            std::cout << "Starting automated simulation...\n" << std::endl;
            simulator.run_simulation(10, 3); // 10 seconds, 3 orders per second
        } else if (mode == "interactive") {
            print_usage();
            simulator.run_interactive_mode();
        } else {
            std::cerr << "Unknown mode: " << mode << std::endl;
            print_usage();
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
