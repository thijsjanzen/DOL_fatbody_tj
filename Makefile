CFLAGS = -Wall -Wextra -std=c++17 -ffast-math -O3
SRC = main.cpp

all: 
	$(CXX) $(SRC) $(CFLAGS) -o my_simulation_program
