CXX ?= g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror -fsanitize=address -fno-omit-frame-pointer -g

all: server client
clean:
	rm -f server
	rm -f client
	rm -f test
	rm -f *.o
	rm -rf *.dSYM

client: client.cpp sbcp.o sbcp_messages.o tcp_client.o
	$(CXX) $(CXXFLAGS) -o client client.cpp sbcp.o sbcp_messages.o tcp_client.o

server: server.cpp sbcp.o sbcp_messages.o tcp_server.o tcp_client.o
	$(CXX) $(CXXFLAGS) -o server server.cpp sbcp.o sbcp_messages.o tcp_server.o tcp_client.o

test: test.cpp sbcp.o sbcp_messages.o
	$(CXX) $(CXXFLAGS) -o test test.cpp sbcp.o sbcp_messages.o

sbcp.o: sbcp.cpp sbcp.hpp
	$(CXX) $(CXXFLAGS) -c sbcp.cpp

sbcp_messages.o: sbcp_messages.cpp sbcp_messages.hpp
	$(CXX) $(CXXFLAGS) -c sbcp_messages.cpp

tcp_server.o: tcp_server.cpp common.hpp
	$(CXX) $(CXXFLAGS) -c tcp_server.cpp

tcp_client.o: tcp_client.cpp common.hpp
	$(CXX) $(CXXFLAGS) -c tcp_client.cpp

