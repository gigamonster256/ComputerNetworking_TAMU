CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror

LIBTCP ?= ../libtcp/libtcp.a
LIBTCP_INC ?= ../libtcp/include
LIBSBCP ?= ../libsbcp/libsbcp.a
LIBSBCP_INC ?= ../libsbcp/include
INCLUDE = -I$(LIBSBCP_INC) -I$(LIBTCP_INC)

PREFIX ?= /usr/local
DESTDIR ?=

.PHONY: all clean install

all: server client

clean:
	rm -f server client
	rm -f *.o
	rm -rf *.dSYM

client: src/sbcp_client.cpp $(LIBTCP) $(LIBSBCP)
	$(CXX) $(CXXFLAGS) -o client $(INCLUDE) src/sbcp_client.cpp $(LIBSBCP) $(LIBTCP)

server: src/sbcp_server.cpp $(LIBTCP) $(LIBSBCP)
	$(CXX) $(CXXFLAGS) -o server $(INCLUDE) src/sbcp_server.cpp $(LIBSBCP) $(LIBTCP)

install: server client
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 server $(DESTDIR)$(PREFIX)/bin/sbcp_server
	install -m 755 client $(DESTDIR)$(PREFIX)/bin/sbcp_client

$(LIBTCP):
	$(MAKE) -C ../libtcp MODE=static

$(LIBSBCP):
	$(MAKE) -C ../libsbcp MODE=static
