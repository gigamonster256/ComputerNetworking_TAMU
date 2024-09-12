CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -pedantic -O3

MODE ?= shared
# MODE = static

ifeq ($(MODE), static)
	LIBEXT = a
else ifeq ($(MODE), shared)
	LIBEXT = so
endif

# libUDP
LIBUDPDIR = src
LIBUDPINCLUDE = -Iinclude
LIBUDPSRCS = client.cpp server.cpp
LIBUDPSRCS := $(addprefix $(LIBUDPDIR)/, $(LIBUDPSRCS))
LIBUDPOBJS = $(LIBUDPSRCS:.cpp=.o)
LIBUDPBASE = libudp
LIBUDP = $(LIBUDPBASE).$(LIBEXT)

.PHONY: all clean 
all: $(LIBUDP)

clean:
	rm -f libtcp.a
	rm -f libtcp.so
	rm -f $(LIBUDPDIR)/*.o

$(LIBUDPBASE).a: $(LIBUDPOBJS)
	ar rcs $@ $(LIBUDPOBJS)

$(LIBUDPBASE).so : $(LIBUDPOBJS)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(LIBUDPOBJS)

$(LIBUDPDIR)/%.o: $(LIBUDPDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(LIBUDPINCLUDE) $(INCLUDES) -fPIC -c -o $@ $<




