CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror

LIBTCP ?= ../libtcp/libtcp.a
LIBTCP_INC ?= ../libtcp/include
INCLUDE = -I$(LIBTCP_INC)

PREFIX ?= /usr/local
DESTDIR ?=

.PHONY: all clean install

all: echos echo

clean:
	rm -f echos echo
	rm -f *.o
	rm -rf *.dSYM

echos: src/echo_server.cpp $(LIBTCP)
	$(CXX) $(CXXFLAGS) -o echos $(INCLUDE) src/echo_server.cpp $(LIBTCP)

echo: src/echo_client.cpp $(LIBTCP)
	$(CXX) $(CXXFLAGS) -o echo $(INCLUDE) src/echo_client.cpp $(LIBTCP)

install: echos echo
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 echos $(DESTDIR)$(PREFIX)/bin/echos
	install -m 755 echo $(DESTDIR)$(PREFIX)/bin/echo

$(LIBTCP):
	$(MAKE) -C ../libtcp MODE=static
