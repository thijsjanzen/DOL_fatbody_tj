CFLAGS = -Wall -Werror -Wextra -std=c++14 -ffast-math -O3
SRC = main.cpp

all: 
	$(CXX) $(SRC) $(CFLAGS) -o dol_fatbody_tj 
