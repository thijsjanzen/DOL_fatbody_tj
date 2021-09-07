CFLAGS = -Wall -Wextra -Werror -std=c++17 -ffast-math -O3
SRC = main.cpp

all: 
	$(CXX) $(SRC) $(CFLAGS) -o dol_fatbody_tj 
