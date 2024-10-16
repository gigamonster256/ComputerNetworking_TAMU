CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror

LIBUDP = ../../libudp/libudp.a
LIBTFTP = ../../libtftp/libtftp.a
INCLUDE = -I../../libudp/include -I../../libtftp/include

all: server
clean:
	rm -f server
	rm -f *.o
	rm -rf *.dSYM

server: tftp_server.cpp $(LIBUDP) $(LIBTFTP) netascii.hpp
	$(CXX) $(CXXFLAGS) -o server $(INCLUDE) tftp_server.cpp $(LIBTFTP) $(LIBUDP)

$(LIBUDP):
	$(MAKE) -C ../../libudp MODE=static

$(LIBTFTP):
	$(MAKE) -C ../../libtftp MODE=static