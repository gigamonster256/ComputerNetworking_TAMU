CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror

LIBUDP ?= ../libudp/libudp.a
LIBUDP_INC ?= ../libudp/include
LIBTFTP ?= ../libtftp/libtftp.a
LIBTFTP_INC ?= ../libtftp/include
INCLUDE = -I$(LIBUDP_INC) -I$(LIBTFTP_INC)

PREFIX ?= /usr/local
DESTDIR ?=

.PHONY: all clean install

all: server

clean:
	rm -f server
	rm -f *.o
	rm -rf *.dSYM

server: src/tftp_server.cpp $(LIBUDP) $(LIBTFTP) src/netascii.hpp
	$(CXX) $(CXXFLAGS) -o server $(INCLUDE) src/tftp_server.cpp $(LIBTFTP) $(LIBUDP)

install: server
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 server $(DESTDIR)$(PREFIX)/bin/tftp_server

$(LIBUDP):
	$(MAKE) -C ../libudp MODE=static

$(LIBTFTP):
	$(MAKE) -C ../libtftp MODE=static
