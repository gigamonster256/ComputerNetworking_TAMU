CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror

LIBTCP ?= ../libtcp/libtcp.a
LIBTCP_INC ?= ../libtcp/include
LIBHTTP ?= ../libhttp/libhttp.a
LIBHTTP_INC ?= ../libhttp/include
INCLUDE = -I$(LIBHTTP_INC) -I$(LIBTCP_INC)

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	LIBS = -lpthread
endif

PREFIX ?= /usr/local
DESTDIR ?=

.PHONY: all clean install

all: proxy client

clean:
	rm -f proxy client server
	rm -f *.o
	rm -rf *.dSYM
	rm -f *.http

client: src/http_client.cpp $(LIBTCP) $(LIBHTTP)
	$(CXX) $(CXXFLAGS) -o client $(INCLUDE) src/http_client.cpp $(LIBHTTP) $(LIBTCP) $(LIBS)

proxy: src/http_proxy.cpp $(LIBTCP) $(LIBHTTP)
	$(CXX) $(CXXFLAGS) -o proxy $(INCLUDE) src/http_proxy.cpp $(LIBHTTP) $(LIBTCP) $(LIBS)

server: src/http_server.cpp $(LIBTCP) $(LIBHTTP)
	$(CXX) $(CXXFLAGS) -o server $(INCLUDE) src/http_server.cpp $(LIBHTTP) $(LIBTCP) $(LIBS)

install: proxy client server
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 proxy $(DESTDIR)$(PREFIX)/bin/http_proxy
	install -m 755 client $(DESTDIR)$(PREFIX)/bin/http_client
	install -m 755 server $(DESTDIR)$(PREFIX)/bin/http_server

$(LIBTCP):
	$(MAKE) -C ../libtcp MODE=static

$(LIBHTTP): $(LIBTCP)
	$(MAKE) -C ../libhttp MODE=static
