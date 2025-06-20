# MeMS-Pro Makefile

# Compiler and Flags
CXX := g++
CXXFLAGS := -std=c++17 -I./src -Wall -Wextra -g
LDFLAGS := -pthread
TEST_LDFLAGS := -pthread -lgtest -lgtest_main

# Directories
SRC_DIR := src
TEST_DIR := tests
BIN_DIR := bin

# Source files
SRCS := $(SRC_DIR)/memory_manager.cpp \
        $(SRC_DIR)/slab_allocator.cpp \
        $(SRC_DIR)/main.cpp

BENCH_SRC := $(SRC_DIR)/benchmark.cpp

TEST_SRCS := $(TEST_DIR)/allocator_test.cpp

# Executables
DEMO_EXE := $(BIN_DIR)/memory_test
BENCH_EXE := $(BIN_DIR)/benchmark
TEST_EXE := $(BIN_DIR)/test_runner

# Default target builds everything needed
.PHONY: all demo bench test clean
all: demo bench

# Build the demo (multithreaded run)
demo: $(SRCS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(DEMO_EXE) $(LDFLAGS)

# Build & run the benchmark suite
bench: $(BENCH_SRC) $(SRC_DIR)/memory_manager.cpp $(SRC_DIR)/slab_allocator.cpp
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $(BENCH_EXE) $(LDFLAGS)
	@echo "Yaay .. Running benchmark..."
	./$(BENCH_EXE)

# Build & run unit tests (requires Google Test)
test: $(SRC_DIR)/memory_manager.cpp $(SRC_DIR)/slab_allocator.cpp $(TEST_SRCS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $(TEST_EXE) $(TEST_LDFLAGS)
	./$(TEST_EXE)

# Clean up all build artifacts and logs
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BIN_DIR) *.o *.out memlog.txt benchmark_results.csv


