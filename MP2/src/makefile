CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -fsanitize=address -fno-omit-frame-pointer -g

# turn off asan if running on test server
# for some reason the program crashes some times when loading the
# libasan library... possibly because the container is unprivileged?
# it could also be that my code is just bad (something in TCPServer)?
ifeq ($(shell hostname), ecen602)
	CXXFLAGS = -std=c++17 -Wall -Wextra -Werror
endif

LIBS = -L../../libs -ltcp -lsbcp
INCLUDE = -I../../include

all: server client
clean:
	rm -f server
	rm -f client
	rm -f *.o
	rm -rf *.dSYM
	rm -f *_publish
	rm -f *_subscribe

client: sbcp_client.cpp ../../libs/libtcp.a ../../libs/libsbcp.a
	$(CXX) $(CXXFLAGS) -o client $(LIBS) $(INCLUDE) sbcp_client.cpp

server: sbcp_server.cpp ../../libs/libtcp.a ../../libs/libsbcp.a
	$(CXX) $(CXXFLAGS) -o server $(LIBS) $(INCLUDE) sbcp_server.cpp

../../libs/libtcp.a:
	$(MAKE) -C ../../ libs/libtcp.a

../../libs/libsbcp.a:
	$(MAKE) -C ../../ libs/libsbcp.a