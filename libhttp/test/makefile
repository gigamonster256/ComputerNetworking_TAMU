CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror

LIBTCP = ../../libtcp/libtcp.a
LIBHTTP = ../../libhttp/libhttp.a
INCLUDE = -I../../libhttp/include -I../../libtcp/include

all: client date header
clean:
	rm -f client date header
	rm -f *.o
	rm -rf *.dSYM

client: client.cpp $(LIBTCP) $(LIBHTTP)
	$(CXX) $(CXXFLAGS) -o client $(INCLUDE) client.cpp $(LIBHTTP)  $(LIBTCP)

date: date.cpp $(LIBTCP) $(LIBHTTP)
	$(CXX) $(CXXFLAGS) -o date $(INCLUDE) date.cpp $(LIBHTTP) $(LIBTCP)

header: header.cpp $(LIBTCP) $(LIBHTTP)
	$(CXX) $(CXXFLAGS) -o header $(INCLUDE) header.cpp $(LIBHTTP) $(LIBTCP)

$(LIBTCP):
	$(MAKE) -C ../../libtcp MODE=static

$(LIBHTTP): $(LIBTCP)
	$(MAKE) -C ../../libhttp MODE=static