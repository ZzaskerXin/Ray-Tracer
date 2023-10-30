# Makefile for compiling HW1B.cpp and running with a user-specified input file

# Compiler and compiler flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall

# Source file and executable name
SRC = raytracer1c.cpp
EXE = raytracer1c

# Default target (the target that gets built when you run 'make' with no arguments)
all: $(EXE)

# Rule to build the executable
$(EXE): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(EXE) $(SRC)

# Rule to run the program with a user-specified input file
run: $(EXE)
	./$(EXE)

# Clean up temporary and generated files
clean:
	rm -f $(EXE)

# .PHONY tells make that these targets don't correspond to actual files
.PHONY: all run clean