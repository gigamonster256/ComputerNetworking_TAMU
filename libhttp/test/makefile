CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -g

LIBTCP = ../../libtcp/libtcp.a
LIBHTTP = ../../libhttp/libhttp.a
INCLUDE = -I../../libhttp/include -I../../libtcp/include

all: client
clean:
	rm -f client
	rm -f *.o
	rm -rf *.dSYM

client: client.cpp $(LIBTCP) $(LIBHTTP)
	$(CXX) $(CXXFLAGS) -o client $(INCLUDE) client.cpp $(LIBTCP) $(LIBHTTP)

$(LIBTCP):
	$(MAKE) -C ../../libtcp MODE=static

$(LIBHTTP): $(LIBTCP)
	$(MAKE) -C ../../libhttp MODE=static