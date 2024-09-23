CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror

LIBTCP = ../../libtcp/libtcp.a
LIBSBCP = ../../libsbcp/libsbcp.a
INCLUDE = -I../../libsbcp/include -I../../libtcp/include

all: server client
clean:
	rm -f server
	rm -f client
	rm -f *.o
	rm -rf *.dSYM

client: sbcp_client.cpp $(LIBTCP) $(LIBSBCP)
	$(CXX) $(CXXFLAGS) -o client $(INCLUDE) sbcp_client.cpp $(LIBSBCP) $(LIBTCP)

server: sbcp_server.cpp $(LIBTCP) $(LIBSBCP)
	$(CXX) $(CXXFLAGS) -o server $(INCLUDE) sbcp_server.cpp $(LIBSBCP) $(LIBTCP)

$(LIBTCP):
	$(MAKE) -C ../../libtcp MODE=static

$(LIBSBCP):
	$(MAKE) -C ../../libsbcp MODE=static