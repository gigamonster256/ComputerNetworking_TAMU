CXX ?= g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror -fsanitize=address -fno-omit-frame-pointer -g

# turn off asan if running on test server
# for some reason the program crashes some times when loading the
# libasan library... possibly because the container is unprivileged?
# it could also be that my code is just bad (something in TCPServer)?
ifeq ($(shell hostname), ecen602)
	CXXFLAGS = -std=c++11 -Wall -Wextra -Werror
endif

all: echos echo
clean :
	rm -f echos
	rm -f echo
	rm -f *.o
	rm -rf *.dSYM

echos: server.cpp tcp_client.o tcp_server.o
	$(CXX) $(CXXFLAGS) -o echos server.cpp tcp_client.o tcp_server.o

echo: client.cpp tcp_client.o 
	$(CXX) $(CXXFLAGS) -o echo client.cpp tcp_client.o

tcp_server.o: tcp_server.cpp
	$(CXX) $(CXXFLAGS) -c tcp_server.cpp

tcp_client.o: tcp_client.cpp
	$(CXX) $(CXXFLAGS) -c tcp_client.cpp


