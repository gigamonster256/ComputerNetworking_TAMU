CXX ?= g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror -fsanitize=address -fno-omit-frame-pointer -g

all:
clean:
	rm -f *.o
	rm -rf *.dSYM

sbcp.o: sbcp.cpp sbcp.hpp
	$(CXX) $(CXXFLAGS) -c sbcp.cpp

sbcp_helpers.o: sbcp_helpers.cpp sbcp_helpers.hpp sbcp.hpp
	$(CXX) $(CXXFLAGS) -c sbcp_helpers.cpp 


