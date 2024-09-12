CXX ?= g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror -fsanitize=address -fno-omit-frame-pointer

# libtcp
LIBTCPDIR = src/tcp
LIBTCPINCLUDES = -Iincludes/tcp
LIBTCPSRCS = server.cpp client.cpp
LIBTCPSRCS := $(addprefix $(LIBTCPDIR)/, $(LIBTCPSRCS))
LIBTCPOBJS = $(LIBTCPSRCS:.cpp=.o)

# libhttp
LIBHTTPDIR = src/http
LIBHTTPINCLUDES = -Iincludes/http
LIBHTTPSRCS = date.cpp error.cpp header.cpp client.cpp
LIBHTTPSRCS := $(addprefix $(LIBHTTPDIR)/, $(LIBHTTPSRCS))
LIBHTTPOBJS = $(LIBHTTPSRCS:.cpp=.o)

# tests
TESTDIR = test
TESTSRCS = date.cpp header.cpp client.cpp
TESTS = $(TESTSRCS:.cpp=.out)
TESTSRCS := $(addprefix $(TESTDIR)/, $(TESTSRCS))

INCLUDES = -Iincludes
LIBS = -Llibs -ltcp -lhttp

.PHONY: all clean test clean_libtcp clean_libhttp clean_tests
all: libs/libtcp.so libs/libhttp.so

test: all $(TESTS) 
	for test in $(TESTS); do ./$$test; done

$(TESTDIR)/%.out: $(TESTDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LIBS)

clean: clean_libtcp clean_libhttp clean_tests

clean_libtcp:
	rm -f libs/libtcp.so
	rm -f $(LIBTCPDIR)/*.o

clean_libhttp:
	rm -f libs/libhttp.so
	rm -f $(LIBHTTPDIR)/*.o

clean_tests:
	rm -f $(TESTDIR)/*.out

libs/libtcp.so: $(LIBTCPOBJS)
	mkdir -p libs
	$(CXX) $(CXXFLAGS) -shared -o $@ $(LIBTCPOBJS)

$(LIBTCPDIR)/%.o: $(LIBTCPDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(LIBTCPINCLUDES) -fPIC -c -o $@ $<

libs/libhttp.so: $(LIBHTTPOBJS) libs
	mkdir -p libs
	$(CXX) $(CXXFLAGS) -shared -o $@ $(LIBHTTPOBJS) -Llibs -ltcp

$(LIBHTTPDIR)/%.o: $(LIBHTTPDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(LIBHTTPINCLUDES) $(INCLUDES) -fPIC -c -o $@ $<

