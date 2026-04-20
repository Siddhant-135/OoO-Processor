# Portable Makefile for Linux/macOS

SHELL := /bin/sh

# Toolchain defaults can be overridden, e.g. make CXX=clang++
CXX ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
LDFLAGS ?=

# Driver source defaults to local src/main.cpp, but supports autograder override:
# make compile FILE=<grader_main.cpp>
DRIVER := $(if $(FILE),$(FILE),src/main.cpp)

# Core simulator sources (exclude non-core drivers/tools)
SRC_DIR := src
SOURCES := $(shell find $(SRC_DIR) -name "*.cpp")
CORE_SOURCES := $(filter-out $(DRIVER) src/parser/parser_dump.cpp,$(SOURCES))
INCLUDES := -Isrc -Isrc/BranchPredictor -Isrc/decode_units -Isrc/execute_units -Isrc/parser

.PHONY: compile run execute parse clean

# ==========================================
# make compile [FILE=<filename.cpp>]
# ==========================================
compile:
	@if [ ! -f "$(DRIVER)" ]; then \
		echo "Error: driver file '$(DRIVER)' not found."; \
		exit 1; \
	fi
	@echo "Compiling simulator for $(DRIVER)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) "$(DRIVER)" $(CORE_SOURCES) $(LDFLAGS) -o main
	@echo "Build successful: ./main"

# ==========================================
# make run FILE=<filename.s>
# ==========================================
# Preprocesses the input assembly file in-place.
# If compiler.py is absent, falls back to no-op preprocessing.
run:
	@if [ -z "$(FILE)" ]; then \
		echo "Error: FILE is required. Usage: make run FILE=<filename.s>"; \
		exit 1; \
	fi
	@if [ ! -f "$(FILE)" ]; then \
		echo "Error: FILE '$(FILE)' not found."; \
		exit 1; \
	fi
	@echo "Preprocessing $(FILE)..."
	@if [ -f compiler.py ]; then \
		python3 compiler.py "$(FILE)"; \
	else \
		touch "$(FILE)"; \
		echo "No compiler.py found; applied no-op preprocessing."; \
	fi
	@echo "Preprocessing complete."

# ==========================================
# make execute FILE=<filename.s> [CYCLES=N] [VERBOSE=1]
# ==========================================
# Runs as: ./main <filename.s> [-cycles N] [-verbose]
execute:
	@if [ ! -x ./main ]; then \
		echo "Error: ./main not found. Run 'make compile' first."; \
		exit 1; \
	fi
	@if [ -z "$(FILE)" ]; then \
		echo "Error: FILE is required. Usage: make execute FILE=<filename.s> [CYCLES=N] [VERBOSE=1]"; \
		exit 1; \
	fi
	@if [ ! -f "$(FILE)" ]; then \
		echo "Error: input file '$(FILE)' not found."; \
		exit 1; \
	fi
	@echo "Running ./main $(FILE)$(if $(CYCLES), -cycles $(CYCLES),)$(if $(VERBOSE), -verbose,)..."
	./main "$(FILE)" $(if $(CYCLES),-cycles "$(CYCLES)",) $(if $(VERBOSE),-verbose,)

# ==========================================
# make parse FILE=<filename.txt>
# ==========================================
# Parse and print decoded instructions for quick parser verification.
parse:
	@if [ -z "$(FILE)" ]; then \
		echo "Error: FILE is required. Usage: make parse FILE=<filename.txt>"; \
		exit 1; \
	fi
	@if [ ! -f "$(FILE)" ]; then \
		echo "Error: FILE '$(FILE)' not found."; \
		exit 1; \
	fi
	@echo "Parsing $(FILE)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) src/parser/parser_dump.cpp src/parser/Parser.cpp $(LDFLAGS) -o parser_dump
	./parser_dump "$(FILE)"

# ==========================================
# make test
# ==========================================
# Runs the simulator on all test cases in the test_cases directory.
test: compile
	@echo "Running all test cases in test_cases/..."
	@for file in test_cases/code*.txt; do \
		echo "----------------------------------------"; \
		echo "Testing $$file..."; \
		./main "$$file"; \
		echo "----------------------------------------"; \
	done

clean:
	$(RM) main parser_dump
