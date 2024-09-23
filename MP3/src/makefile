CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror

LIBS = -L../../libudp -ludp
LIBUDP = ../../libudb/libudp.a
INCLUDE = -I../../libudp/include

all: server client
clean:
	rm -f server
	rm -f client
	rm -f *.o
	rm -rf *.dSYM

client: client.cpp $(LIBUDP)
	$(CXX) $(CXXFLAGS) -o client $(LIBS) $(INCLUDE) client.cpp

server: server.cpp $(LIBUDP)
	$(CXX) $(CXXFLAGS) -o server $(LIBS) $(INCLUDE) server.cpp

$(LIBUDP):
	$(MAKE) -C ../../libudp MODE=static

