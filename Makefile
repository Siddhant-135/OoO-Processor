# Portable Makefile for Linux/macOS

SHELL := /bin/sh

# Toolchain defaults can be overridden, e.g. make CXX=clang++
CXX ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
LDFLAGS ?=

# Driver source defaults to local main.cpp, but supports autograder override:
# make compile FILE=<grader_main.cpp>
DRIVER := $(if $(FILE),$(FILE),main.cpp)

# Core simulator sources (exclude non-core drivers/tools)
CORE_SOURCES := $(filter-out $(DRIVER) main.cpp parser_dump.cpp,$(wildcard *.cpp))

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
	$(CXX) $(CXXFLAGS) "$(DRIVER)" $(CORE_SOURCES) $(LDFLAGS) -o main
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
# make execute FILE=<filename.s> [CYCLES=N]
# ==========================================
# Runs as: ./main <filename.s> [-cycles N]
execute:
	@if [ ! -x ./main ]; then \
		echo "Error: ./main not found. Run 'make compile' first."; \
		exit 1; \
	fi
	@if [ -z "$(FILE)" ]; then \
		echo "Error: FILE is required. Usage: make execute FILE=<filename.s> [CYCLES=N]"; \
		exit 1; \
	fi
	@if [ ! -f "$(FILE)" ]; then \
		echo "Error: input file '$(FILE)' not found."; \
		exit 1; \
	fi
	@echo "Running ./main $(FILE)$(if $(CYCLES), -cycles $(CYCLES),)..."
	@if [ -n "$(CYCLES)" ]; then \
		./main "$(FILE)" -cycles "$(CYCLES)"; \
	else \
		./main "$(FILE)"; \
	fi

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
	$(CXX) $(CXXFLAGS) parser_dump.cpp Parser.cpp $(LDFLAGS) -o parser_dump
	./parser_dump "$(FILE)"

clean:
	$(RM) main parser_dump
