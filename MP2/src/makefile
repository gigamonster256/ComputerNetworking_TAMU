CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -fsanitize=address -fno-omit-frame-pointer -g

# turn off asan if running on test server
# for some reason the program crashes some times when loading the
# libasan library... possibly because the container is unprivileged?
# it could also be that my code is just bad (something in TCPServer)?
ifeq ($(shell hostname), ecen602)
	CXXFLAGS = -std=c++17 -Wall -Wextra -Werror
endif

LIBS = -L../../libtcp -L../../libsbcp -ltcp -lsbcp
LIBTCP = ../../libtcp/libtcp.a
LIBSBCP = ../../libsbcp/libsbcp.a
INCLUDE = -I../../include

all: server client
clean:
	rm -f server
	rm -f client
	rm -f *.o
	rm -rf *.dSYM

client: sbcp_client.cpp $(LIBTCP) $(LIBSBCP)
	$(CXX) $(CXXFLAGS) -o client $(LIBS) $(INCLUDE) sbcp_client.cpp

server: sbcp_server.cpp $(LIBTCP) $(LIBSBCP)
	$(CXX) $(CXXFLAGS) -o server $(LIBS) $(INCLUDE) sbcp_server.cpp

$(LIBTCP):
	$(MAKE) -C ../../libtcp MODE=static

$(LIBSBCP):
	$(MAKE) -C ../../libsbcp MODE=static