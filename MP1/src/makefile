CXX ?= g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror -fsanitize=address -fno-omit-frame-pointer -g

all: echos echo
clean :
	rm -f echos
	rm -f echo
	rm -f *.o
	rm -rf *.dSYM

echos: server.cpp common.hpp tcp_client.o tcp_server.o
	$(CXX) $(CXXFLAGS) -o echos server.cpp tcp_client.o tcp_server.o

echo: client.cpp common.hpp tcp_client.o 
	$(CXX) $(CXXFLAGS) -o echo client.cpp tcp_client.o

tcp_server.o: tcp_server.cpp
	$(CXX) $(CXXFLAGS) -c tcp_server.cpp

tcp_client.o: tcp_client.cpp
	$(CXX) $(CXXFLAGS) -c tcp_client.cpp


