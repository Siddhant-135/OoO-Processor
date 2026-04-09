# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall
SOURCES = $(filter-out $(FILE), $(wildcard *.cpp))

# ==========================================
# make compile FILE=<filename.cpp>
# ==========================================
# This target should compile your files with the provided 
# main.cpp. The main.cpp will always #include "Processor.h" 
# and will have its own main() function.
compile:
	@echo "Compiling simulator:"
	$(CXX) $(CXXFLAGS) $(FILE) $(SOURCES) -o main
	@echo "Build successful, 'main' created."

# ==========================================
# make run FILE=<filename.s>
# ==========================================
# Update this target to run whatever script or 
# program you wrote to preprocess the assembly labels. 
# Example below assumes a Python script named 'compiler.py'.
run:
	@echo "Preprocessing $(FILE)..."
	python3 compiler.py $(FILE)
	@echo "Preprocessing complete."

# ==========================================
# make parse FILE=<filename.txt>
# ==========================================
# Parse and print decoded instructions for quick parser verification.
parse:
	@echo "Parsing $(FILE)..."
	$(CXX) $(CXXFLAGS) parser_dump.cpp Parser.cpp -o parser_dump
	./parser_dump $(FILE)
