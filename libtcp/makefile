CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -pedantic -O3

MODE ?= shared
# MODE = static

ifeq ($(MODE), static)
	LIBEXT = a
else ifeq ($(MODE), shared)
	LIBEXT = so
endif

# libTCP
LIBTCPDIR = src
LIBTCPINCLUDE = -Iinclude
LIBTCPSRCS = client.cpp server.cpp
LIBTCPSRCS := $(addprefix $(LIBTCPDIR)/, $(LIBTCPSRCS))
LIBTCPOBJS = $(LIBTCPSRCS:.cpp=.o)
LIBTCPBASE = libtcp
LIBTCP = $(LIBTCPBASE).$(LIBEXT)


UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	LIBS = -lpthread
endif


TESTDIR = test
TESTSRCS = server.cpp
TESTSRCS := $(addprefix $(TESTDIR)/, $(TESTSRCS))
TESTEXECS = $(TESTSRCS:.cpp=.out)

.PHONY: all clean test
all: $(LIBTCP)

clean:
	rm -f libtcp.a
	rm -f libtcp.so
	rm -f $(LIBTCPDIR)/*.o
	rm -f $(TESTDIR)/*.out

test: $(TESTEXECS)
	for test in $(TESTEXECS); do ./$$test; done

$(TESTDIR)/%: $(TESTDIR)/%.cpp $(LIBTCP)
	$(CXX) $(CXXFLAGS) $(LIBTCPINCLUDE) -o $@ $< $(LIBTCP)

$(LIBTCPBASE).a: $(LIBTCPOBJS)
	ar rcs $@ $(LIBTCPOBJS)

$(LIBTCPBASE).so : $(LIBTCPOBJS)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(LIBTCPOBJS)

$(LIBTCPDIR)/%.o: $(LIBTCPDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(LIBTCPINCLUDE) $(LIBS) -fPIC -c -o $@ $<




