# Makefile for Limit Order Book Simulator

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -Isrc
DEBUG_FLAGS = -g -O0 -DDEBUG
RELEASE_FLAGS = -O2 -DNDEBUG

# Source files
SRC_DIR = src
SOURCES = $(SRC_DIR)/order.cpp $(SRC_DIR)/order_book.cpp \
          $(SRC_DIR)/matching_engine.cpp $(SRC_DIR)/exchange_simulator.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = lob_simulator

# Default target
all: $(TARGET)

# Link executable
$(TARGET): $(OBJECTS) main.o
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) -o $@ $^

# Debug build
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: clean $(TARGET)

# Compile source files
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) main.o $(TARGET) test_runner

# Install (copy to /usr/local/bin)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Check syntax without linking
check:
	$(CXX) $(CXXFLAGS) -fsyntax-only $(SOURCES) main.cpp

# Build and run tests
test: test_runner
	./test_runner

test_runner: tests/test_order_book.cpp $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ $^

# Show help
help:
	@echo "Limit Order Book Simulator Build System"
	@echo "======================================="
	@echo "Targets:"
	@echo "  all     - Build optimized executable (default)"
	@echo "  debug   - Build with debug symbols"
	@echo "  test    - Build and run test suite"
	@echo "  clean   - Remove build artifacts"
	@echo "  check   - Syntax check only"
	@echo "  install - Install to /usr/local/bin"
	@echo "  help    - Show this message"

.PHONY: all debug clean install check test help