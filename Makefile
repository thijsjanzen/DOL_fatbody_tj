CFLAGS = -Wall -Werror -Wextra -std=c++17 -ffast-math -O3
SRC = main.cpp simulation.cpp parameters.cpp

all: 
	$(CXX) $(SRC) $(CFLAGS) -o dol_fatbody_tj 
