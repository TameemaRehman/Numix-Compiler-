# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -O2
DEBUG_FLAGS = -g -DDEBUG
RELEASE_FLAGS = -O3

# Directories
SRCDIR = src
INCDIR = include
BUILDDIR = build
TARGETDIR = bin

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)

# Target
TARGET = $(TARGETDIR)/mathseqc

# Default target
all: release

# Release build
release: CXXFLAGS += $(RELEASE_FLAGS)
release: $(TARGET)

# Debug build
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

# Create target
$(TARGET): $(OBJECTS) | $(TARGETDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "Build complete: $(TARGET)"

# Compile source files
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create directories
$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

$(TARGETDIR):
	@mkdir -p $(TARGETDIR)

# Clean build
clean:
	rm -rf $(BUILDDIR) $(TARGETDIR)

# Run tests
test: debug
	@echo "Running tests..."
	@$(TARGET) test/examples/fibonacci.mathseq -ast

# Run with example
example: debug
	@$(TARGET) test/examples/fibonacci.mathseq -output output.asm

# Install system-wide (optional)
install: release
	cp $(TARGET) /usr/local/bin/mathseqc

# Show help
help:
	@echo "Available targets:"
	@echo "  all/release - Build release version (default)"
	@echo "  debug       - Build debug version with symbols"
	@echo "  clean       - Remove build artifacts"
	@echo "  test        - Run compiler tests"
	@echo "  example     - Compile example program"
	@echo "  install     - Install system-wide (requires sudo)"

.PHONY: all release debug clean test example install help