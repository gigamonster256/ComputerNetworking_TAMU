CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -pedantic -O3

MODE ?= shared
# MODE = static

ifeq ($(MODE), static)
	LIBEXT = a
else ifeq ($(MODE), shared)
	LIBEXT = so
endif

# libhttp
LIBHTTPDIR = src
LIBHTTPINCLUDE = -Iinclude
LIBHTTPSRCS = date.cpp error.cpp header.cpp client.cpp
LIBHTTPSRCS := $(addprefix $(LIBHTTPDIR)/, $(LIBHTTPSRCS))
LIBHTTPOBJS = $(LIBHTTPSRCS:.cpp=.o)
LIBHTTPBASE = libhttp
LIBHTTP = $(LIBHTTPBASE).$(LIBEXT)

# libhttp dependencies
LIBTCPDIR = ../libtcp
LIBTCPINCLUDE = -I../include
LIBTCP = $(LIBTCPDIR)/libtcp.$(LIBEXT)

.PHONY: all clean 
all: $(LIBHTTP)

clean:
	rm -f libhttp.a
	rm -f libhttp.so
	rm -f $(LIBHTTPDIR)/*.o


$(LIBHTTPBASE).a: $(LIBHTTPOBJS)
	ar rcs $@ $(LIBHTTPOBJS)

$(LIBHTTPBASE).so: $(LIBTCP) $(LIBHTTPOBJS)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(LIBHTTPOBJS) -L$(LIBTCPDIR) -ltcp

$(LIBHTTPDIR)/%.o: $(LIBHTTPDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(LIBHTTPINCLUDE) $(LIBTCPINCLUDE) -fPIC -c -o $@ $<

$(LIBTCP):
	$(MAKE) -C $(LIBTCPDIR) MODE=$(MODE)




