# Limit Order Book Simulator

A C++ implementation of a limit order book matching engine that replicates core exchange functionality. This project implements price-time priority matching, order management, and market depth visualization for educational and research purposes.

## Overview

This simulator models how electronic exchanges process and match trading orders. The limit order book maintains separate queues for buy and sell orders, executing matches according to standard market rules.

### Features

- Price-time priority order matching
- Support for limit and market orders
- Order cancellation and modification
- Real-time order book visualization
- Command-line interface for order entry
- Automated simulation mode

## Architecture

The system consists of several key components:

- **Order**: Basic order structure with validation
- **OrderBook**: Maintains sorted price levels and order queues
- **MatchingEngine**: Processes orders and executes matches
- **ExchangeSimulator**: Provides user interface and simulation control

## Building

### Requirements

- C++17 compatible compiler
- Make

### Compilation

```bash
# Build the project
make

# Build debug version
make debug

# Clean build artifacts
make clean
```

## Usage

### Interactive Mode

```bash
./lob_simulator
```

Available commands:
- `ADD <SIDE> <TYPE> <PRICE> <QUANTITY>` - Submit new order
- `CANCEL <ORDER_ID>` - Cancel existing order
- `MODIFY <ORDER_ID> <NEW_QUANTITY>` - Modify order quantity
- `BOOK` - Display current order book
- `STATS` - Show trading statistics
- `QUIT` - Exit

Example session:
```
ADD BUY LIMIT 100.50 200
ADD SELL LIMIT 101.00 150
BOOK
ADD SELL MARKET 0 50
STATS
QUIT
```

### Simulation Mode

```bash
./lob_simulator simulation
```

Runs automated simulation with random order flow.

## Testing

The project includes unit tests covering:
- Order validation and creation
- Order book operations
- Matching engine logic
- Edge cases and error conditions

Tests are located in `tests/test_order_book.cpp` and use the Catch2 framework.

## Performance

### Algorithmic Complexity

| Operation | Time Complexity |
|-----------|----------------|
| Add Order | O(log n) |
| Cancel Order | O(log n) |
| Modify Order | O(1) to O(n) |
| Match Order | O(k log n) |

Where n is the number of price levels and k is the number of matched levels.

### Data Structures

- Orders are stored in deques at each price level for FIFO processing
- Price levels use std::map for ordered access
- Order lookup uses unordered_map for O(1) cancellation

## Configuration

### Logging

Adjust logging level in main.cpp:
```cpp
Logger::current_level = LogLevel::DEBUG;  // Verbose output
Logger::current_level = LogLevel::INFO;   // Normal operation
Logger::current_level = LogLevel::ERROR;  // Errors only
```

### Order Book Display

Modify depth in book display:
```cpp
order_book.print_book(10);  // Show top 10 levels
```

## Implementation Details

### Order Matching

The engine implements price-time priority:
1. Orders match at best available prices first
2. Within each price level, orders match in FIFO order
3. Partially filled orders remain in the book

### Data Structures

- Buy orders: std::map with descending price order
- Sell orders: std::map with ascending price order
- Order queues: std::deque for efficient front/back operations
- Order lookup: std::unordered_map for fast cancellation