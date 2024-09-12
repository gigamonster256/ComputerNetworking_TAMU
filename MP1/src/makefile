CXX ?= g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror -fsanitize=address -fno-omit-frame-pointer -g
PORT ?= 8080
SERVER_ADDR ?= 127.0.0.1

# turn off asan if running on test server
# for some reason the program crashes some times when loading the
# libasan library... possibly because the container is unprivileged?
# it could also be that my code is just bad (something in TCPServer)?
ifeq ($(shell hostname), ecen602)
	CXXFLAGS = -std=c++11 -Wall -Wextra -Werror
endif

.PHONY: all clean echo echos

all: tcp_server.o tcp_client.o
	$(CXX) $(CXXFLAGS) -o echos server.cpp tcp_client.o tcp_server.o
	$(CXX) $(CXXFLAGS) -o echo client.cpp tcp_client.o

clean :
	rm -f echos
	rm -f echo
	rm -f *.o
	rm -rf *.dSYM

echos: server.cpp tcp_client.o tcp_server.o
	$(CXX) $(CXXFLAGS) -o echos server.cpp tcp_client.o tcp_server.o
	./echos $(PORT)

echo: client.cpp tcp_client.o 
	$(CXX) $(CXXFLAGS) -o echo client.cpp tcp_client.o
	./echo $(SERVER_ADDR) $(PORT)

tcp_server.o: tcp_server.cpp
	$(CXX) $(CXXFLAGS) -c tcp_server.cpp

tcp_client.o: tcp_client.cpp
	$(CXX) $(CXXFLAGS) -c tcp_client.cpp


