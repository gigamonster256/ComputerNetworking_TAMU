CXX ?= g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror -fsanitize=address -fno-omit-frame-pointer -g

# turn off asan if running on test server
# for some reason the program crashes some times when loading the
# libasan library... possibly because the container is unprivileged?
# it could also be that my code is just bad (something in udpServer)?
ifeq ($(shell hostname), ecen602)
	CXXFLAGS = -std=c++11 -Wall -Wextra -Werror
endif

all: server client
clean:
	rm -f server
	rm -f client
	rm -f *.o
	rm -rf *.dSYM

client: client.cpp udp_client.o
	$(CXX) $(CXXFLAGS) -o client client.cpp udp_client.o

server: server.cpp udp_server.o udp_client.o
	$(CXX) $(CXXFLAGS) -o server server.cpp udp_server.o udp_client.o

udp_server.o: udp_server.cpp udp_common.hpp
	$(CXX) $(CXXFLAGS) -c udp_server.cpp

udp_client.o: udp_client.cpp udp_common.hpp
	$(CXX) $(CXXFLAGS) -c udp_client.cpp

