CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -fsanitize=address -fno-omit-frame-pointer -g

# turn off asan if running on test server
# for some reason the program crashes some times when loading the
# libasan library... possibly because the container is unprivileged?
# it could also be that my code is just bad (something in TCPServer)?
ifeq ($(shell hostname), ecen602)
	CXXFLAGS = -std=c++17 -Wall -Wextra -Werror
endif

LIBS = -L../../libtcp -ltcp
LIBTCP = ../../libtcp/libtcp.a
INCLUDE = -I../../include

all: echos echo

clean :
	rm -f echos
	rm -f echo
	rm -f *.o
	rm -rf *.dSYM

echos: echo_server.cpp $(LIBTCP)
	$(CXX) $(CXXFLAGS) -o echos $(LIBS) $(INCLUDE) echo_server.cpp

echo: echo_client.cpp $(LIBTCP)
	$(CXX) $(CXXFLAGS) -o echo $(LIBS) $(INCLUDE) echo_client.cpp

$(LIBTCP):
	$(MAKE) -C ../../libtcp MODE=static


