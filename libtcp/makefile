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

.PHONY: all clean 
all: $(LIBTCP)

clean:
	rm -f libtcp.a
	rm -f libtcp.so
	rm -f $(LIBTCPDIR)/*.o

$(LIBTCPBASE).a: $(LIBTCPOBJS)
	ar rcs $@ $(LIBTCPOBJS)

$(LIBTCPBASE).so : $(LIBTCPOBJS)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(LIBTCPOBJS)

$(LIBTCPDIR)/%.o: $(LIBTCPDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(LIBTCPINCLUDE) $(INCLUDES) -fPIC -c -o $@ $<




