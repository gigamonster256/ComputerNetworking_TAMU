CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror

LIBTCP = ../../libtcp/libtcp.a
LIBHTTP = ../../libhttp/libhttp.a
INCLUDE = -I../../libhttp/include

all: proxy
clean:
	rm -f proxy
	rm -f *.o
	rm -rf *.dSYM

proxy: http_proxy.cpp $(LIBTCP) $(LIBHTTP)
	$(CXX) $(CXXFLAGS) -o proxy $(INCLUDE) http_proxy.cpp $(LIBTCP) $(LIBHTTP)

$(LIBTCP):
	$(MAKE) -C ../../libtcp MODE=static

$(LIBHTTP): $(LIBTCP)
	$(MAKE) -C ../../libhttp MODE=static