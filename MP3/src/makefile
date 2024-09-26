CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror

LIBUDP = ../../libudp/libudp.a
LIBTFTP = ../../libtftp/libtftp.a
INCLUDE = -I../../libudp/include -I../../libtftp/include

all: server client
clean:
	rm -f server
	rm -f client
	rm -f *.o
	rm -rf *.dSYM

client: client.cpp $(LIBUDP) $(LIBTFTP)
	$(CXX) $(CXXFLAGS) -o client $(INCLUDE) client.cpp $(LIBTFTP) $(LIBUDP)

server: server.cpp $(LIBUDP) $(LIBTFTP)
	$(CXX) $(CXXFLAGS) -o server $(INCLUDE) server.cpp $(LIBTFTP) $(LIBUDP)

$(LIBUDP):
	$(MAKE) -C ../../libudp MODE=static

$(LIBTFTP):
	$(MAKE) -C ../../libtftp MODE=static