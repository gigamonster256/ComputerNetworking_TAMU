CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror

LIBTCP = ../../libtcp/libtcp.a
INCLUDE = -I../../libtcp/include

all: echos echo

clean :
	rm -f echos
	rm -f echo
	rm -f *.o
	rm -rf *.dSYM

echos: echo_server.cpp $(LIBTCP)
	$(CXX) $(CXXFLAGS) -o echos $(INCLUDE) echo_server.cpp $(LIBTCP)

echo: echo_client.cpp $(LIBTCP)
	$(CXX) $(CXXFLAGS) -o echo $(INCLUDE) echo_client.cpp $(LIBTCP)

$(LIBTCP):
	$(MAKE) -C ../../libtcp MODE=static


