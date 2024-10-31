CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror

LIBTCP = ../../libtcp/libtcp.a
LIBHTTP = ../../libhttp/libhttp.a
INCLUDE = -I../../libhttp/include -I../../libtcp/include

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	LIBS = -lpthread
endif

.PHONY: all clean

all: proxy client
clean:
	rm -f proxy
	rm -f client
	rm -f *.o
	rm -rf *.dSYM

client: http_client.cpp $(LIBTCP) $(LIBHTTP)
	$(CXX) $(CXXFLAGS) -o client $(INCLUDE) http_client.cpp $(LIBHTTP) $(LIBTCP) $(LIBS)

proxy: http_proxy.cpp $(LIBTCP) $(LIBHTTP)
	$(CXX) $(CXXFLAGS) -o proxy $(INCLUDE) http_proxy.cpp $(LIBHTTP) $(LIBTCP) $(LIBS)

$(LIBTCP):
	$(MAKE) -C ../../libtcp MODE=static

$(LIBHTTP): $(LIBTCP)
	$(MAKE) -C ../../libhttp MODE=static