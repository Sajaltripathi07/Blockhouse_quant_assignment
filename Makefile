# Makefile for MBP-10 Reconstruction Tool
# Optimized for performance with aggressive compiler optimizations

CXX = g++
CXXFLAGS = -std=c++17 -O3 -march=native -flto -DNDEBUG -Wall -Wextra
LDFLAGS = -flto

TARGET = reconstruction_sajal
TEST_TARGET = test_reconstruction
SOURCES = reconstruction_sajal.cpp orderbook.cpp csv_parser.cpp
TEST_SOURCES = test_reconstruction.cpp orderbook.cpp csv_parser.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)
HEADERS = orderbook.h csv_parser.h

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Link the test executable
$(TEST_TARGET): $(TEST_OBJECTS)
	$(CXX) $(TEST_OBJECTS) -o $(TEST_TARGET) $(LDFLAGS)

# Compile source files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(TARGET) $(TEST_TARGET) *.exe test_input.csv

# Test with sample data
test: $(TARGET)
	./$(TARGET) sample_mbo.csv

# Run unit tests
unit-test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Debug build
debug: CXXFLAGS = -std=c++17 -g -O0 -Wall -Wextra -DDEBUG
debug: $(TARGET)

# Profile build
profile: CXXFLAGS = -std=c++17 -O2 -pg -Wall -Wextra
profile: LDFLAGS = -pg
profile: $(TARGET)

# Install dependencies (if needed)
install-deps:
	@echo "No external dependencies required"

# Help
help:
	@echo "Available targets:"
	@echo "  all        - Build optimized release version (default)"
	@echo "  debug      - Build debug version with symbols"
	@echo "  profile    - Build version with profiling support"
	@echo "  test       - Build and test with sample data"
	@echo "  unit-test  - Build and run unit tests"
	@echo "  clean      - Remove build artifacts"
	@echo "  help       - Show this help message"

.PHONY: all clean test debug profile install-deps help
