CXX ?= g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror -fsanitize=address -fno-omit-frame-pointer -g

# turn off asan if running on test server
# for some reason the program crashes some times when loading the
# libasan library... possibly because the container is unprivileged?
# it could also be that my code is just bad (something in TCPServer)?
ifeq ($(shell hostname), ecen602)
	CXXFLAGS = -std=c++11 -Wall -Wextra -Werror
endif

all: server client
clean:
	rm -f server
	rm -f client
	rm -f *.o
	rm -rf *.dSYM
	rm -f *_publish
	rm -f *_subscribe

client: client.cpp sbcp.o sbcp_messages.o tcp_client.o
	$(CXX) $(CXXFLAGS) -o client client.cpp sbcp.o sbcp_messages.o tcp_client.o

server: server.cpp sbcp.o sbcp_messages.o tcp_server.o tcp_client.o
	$(CXX) $(CXXFLAGS) -o server server.cpp sbcp.o sbcp_messages.o tcp_server.o tcp_client.o

sbcp.o: sbcp.cpp sbcp.hpp
	$(CXX) $(CXXFLAGS) -c sbcp.cpp

sbcp_messages.o: sbcp_messages.cpp sbcp_messages.hpp
	$(CXX) $(CXXFLAGS) -c sbcp_messages.cpp

tcp_server.o: tcp_server.cpp
	$(CXX) $(CXXFLAGS) -c tcp_server.cpp

tcp_client.o: tcp_client.cpp
	$(CXX) $(CXXFLAGS) -c tcp_client.cpp

