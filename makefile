CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -pedantic -O3

MODE = static
# MODE = shared

ifeq ($(MODE), static)
	LIBEXT = a
else ifeq ($(MODE), shared)
	LIBEXT = so
endif

# libtcp
LIBTCPDIR = libtcp/src
LIBTCPINCLUDE = -Ilibtcp/include
LIBTCPSRCS = server.cpp client.cpp
LIBTCPSRCS := $(addprefix $(LIBTCPDIR)/, $(LIBTCPSRCS))
LIBTCPOBJS = $(LIBTCPSRCS:.cpp=.o)
LIBTCPBASE = libs/libtcp
LIBTCP = $(LIBTCPBASE).$(LIBEXT)

# libsbcp
LIBSBCPDIR = libsbcp/src
LIBSBCPINCLUDE = -Ilibsbcp/include
LIBSBCPSRCS = sbcp.cpp messages.cpp
LIBSBCPSRCS := $(addprefix $(LIBSBCPDIR)/, $(LIBSBCPSRCS))
LIBSBCPOBJS = $(LIBSBCPSRCS:.cpp=.o)
LIBSBCPBASE = libs/libsbcp
LIBSBCP = $(LIBSBCPBASE).$(LIBEXT)

# libudp
LIBUDPDIR = libudp/src
LIBUDPINCLUDE = -Ilibudp/include
LIBUDPSRCS = server.cpp client.cpp
LIBUDPSRCS := $(addprefix $(LIBUDPDIR)/, $(LIBUDPSRCS))
LIBUDPOBJS = $(LIBUDPSRCS:.cpp=.o)
LIBUDPBASE = libs/libudp
LIBUDP = $(LIBUDPBASE).$(LIBEXT)

# libhttp
LIBHTTPDIR = libhttp/src
LIBHTTPINCLUDE = -Ilibhttp/include -Iinclude
LIBHTTPSRCS = date.cpp error.cpp header.cpp client.cpp
LIBHTTPSRCS := $(addprefix $(LIBHTTPDIR)/, $(LIBHTTPSRCS))
LIBHTTPOBJS = $(LIBHTTPSRCS:.cpp=.o)
LIBHTTPBASE = libs/libhttp
LIBHTTP = $(LIBHTTPBASE).$(LIBEXT)

.PHONY: all clean clean_libtcp clean_libsbcp clean_lubudp clean_libhttp
all: $(LIBTCP) $(LIBSBCP) $(LIBUDP) $(LIBHTTP)

clean: clean_libtcp clean_libsbcp clean_libudp clean_libhttp
	make -C MP1/src clean
	make -C MP2/src clean
	make -C MP3/src clean

clean_libtcp:
	rm -f libs/libtcp.a
	rm -f libs/libtcp.so
	rm -f $(LIBTCPDIR)/*.o

clean_libsbcp:
	rm -f libs/libsbcp.a
	rm -f libs/libsbcp.so
	rm -f $(LIBSBCPDIR)/*.o

clean_libudp:
	rm -f libs/libudp.a
	rm -f libs/libudp.so
	rm -f $(LIBUDPDIR)/*.o

clean_libhttp:
	rm -f libs/libhttp.a
	rm -f libs/libhttp.so
	rm -f $(LIBHTTPDIR)/*.o

$(LIBTCPBASE).a: $(LIBTCPOBJS)
	mkdir -p libs
	ar rcs $@ $^

$(LIBTCPBASE).so: $(LIBTCPOBJS)
	mkdir -p libs
	$(CXX) $(CXXFLAGS) -shared -o $@ $(LIBTCPOBJS)

$(LIBTCPDIR)/%.o: $(LIBTCPDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(LIBTCPINCLUDE) -fPIC -c -o $@ $<

$(LIBSBCPBASE).a: $(LIBSBCPOBJS)
	mkdir -p libs
	ar rcs $@ $^

$(LIBSBCPBASE).so: $(LIBSBCPOBJS)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(LIBSBCPOBJS)

$(LIBSBCPDIR)/%.o: $(LIBSBCPDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(LIBSBCPINCLUDE) -fPIC -c -o $@ $<

$(LIBUDPBASE).a: $(LIBUDPOBJS)
	mkdir -p libs
	ar rcs $@ $^

$(LIBUDPBASE).so: $(LIBUDPOBJS)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(LIBUDPOBJS)

$(LIBUDPDIR)/%.o: $(LIBUDPDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(LIBUDPINCLUDE) -fPIC -c -o $@ $<

$(LIBHTTPBASE).a: libs/libtcp.a $(LIBHTTPOBJS)
	mkdir -p libs
	ar rcs $@ $(LIBHTTPOBJS)

$(LIBHTTPBASE).so : libs/libtcp.so $(LIBHTTPOBJS)
	mkdir -p libs
	$(CXX) $(CXXFLAGS) -shared -o $@ $(LIBHTTPOBJS) -Llibs -ltcp

$(LIBHTTPDIR)/%.o: $(LIBHTTPDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(LIBHTTPINCLUDE) $(INCLUDES) -fPIC -c -o $@ $<




