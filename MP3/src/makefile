CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -fsanitize=address -fno-omit-frame-pointer -g

# turn off asan if running on test server
# for some reason the program crashes some times when loading the
# libasan library... possibly because the container is unprivileged?
# it could also be that my code is just bad (something in udpServer)?
ifeq ($(shell hostname), ecen602)
	CXXFLAGS = -std=c++17 -Wall -Wextra -Werror
endif

LIBS = -L../../libs -ludp
INCLUDE = -I../../include

all: server client
clean:
	rm -f server
	rm -f client
	rm -f *.o
	rm -rf *.dSYM

client: client.cpp ../../libs/libudp.a
	$(CXX) $(CXXFLAGS) -o client $(LIBS) $(INCLUDE) client.cpp

server: server.cpp ../../libs/libudp.a
	$(CXX) $(CXXFLAGS) -o server $(LIBS) $(INCLUDE) server.cpp

../../libs/libudp.a:
	$(MAKE) -C ../../ libs/libudp.a

