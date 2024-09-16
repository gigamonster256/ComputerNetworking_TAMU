CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -pedantic -O3

MODE ?= shared
# MODE = static

ifeq ($(MODE), static)
	LIBEXT = a
else ifeq ($(MODE), shared)
	LIBEXT = so
endif

# libSBCP
LIBSBCPDIR = src
LIBSBCPINCLUDE = -Iinclude
LIBSBCPSRCS = messages.cpp sbcp.cpp
LIBSBCPSRCS := $(addprefix $(LIBSBCPDIR)/, $(LIBSBCPSRCS))
LIBSBCPOBJS = $(LIBSBCPSRCS:.cpp=.o)
LIBSBCPBASE = libsbcp
LIBSBCP = $(LIBSBCPBASE).$(LIBEXT)

.PHONY: all clean 
all: $(LIBSBCP)

clean:
	rm -f libsbcp.a
	rm -f libsbcp.so
	rm -f $(LIBSBCPDIR)/*.o

$(LIBSBCPBASE).a: $(LIBSBCPOBJS)
	ar rcs $@ $(LIBSBCPOBJS)

$(LIBSBCPBASE).so : $(LIBSBCPOBJS)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(LIBSBCPOBJS)

$(LIBSBCPDIR)/%.o: $(LIBSBCPDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(LIBSBCPINCLUDE) $(INCLUDES) -fPIC -c -o $@ $<




