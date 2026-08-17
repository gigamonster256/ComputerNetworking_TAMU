# Shared build rules for the ECEN 602 helper libraries.
#
# Each lib/makefile includes this fragment after setting:
#   LIBNAME  library base name, e.g. "tcp"
#   SRCS     sources relative to src/, e.g. "client.cpp server.cpp"
#
# Optional per-lib variables:
#   EXTRA_INC     extra -I flags for compiling
#   LDLIBS        extra flags for linking the shared library
#   SHARED_DEPS   extra prerequisites for the shared library
#   SHARED_LDLIBS extra flags for linking the shared library (dependencies)
#   TESTS         test sources relative to test/ (built and run by `make test`)

CXX ?= g++
AR ?= ar
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Werror -pedantic -O3

PREFIX ?= /usr/local
DESTDIR ?=

MODE ?= both

SRCDIR := src
INCLUDEDIR := include
OBJS := $(addprefix $(SRCDIR)/, $(SRCS:.cpp=.o))
STATIC_LIB := lib$(LIBNAME).a
SHARED_LIB := lib$(LIBNAME).so
DEPS := $(OBJS:.o=.d)
TESTEXECS := $(addprefix test/, $(TESTS:.cpp=.out))

ifeq ($(MODE), static)
	TARGETS = $(STATIC_LIB)
else ifeq ($(MODE), shared)
	TARGETS = $(SHARED_LIB)
else
	TARGETS = $(STATIC_LIB) $(SHARED_LIB)
endif

.PHONY: all static shared install clean test
all: $(TARGETS)

static: $(STATIC_LIB)
shared: $(SHARED_LIB)

$(STATIC_LIB): $(OBJS)
	$(AR) rcs $@ $(OBJS)

$(SHARED_LIB): $(OBJS) $(SHARED_DEPS)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(OBJS) $(SHARED_LDLIBS) $(LDLIBS)

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDEDIR) $(EXTRA_INC) -fPIC -MMD -MP -c -o $@ $<

install: $(TARGETS)
	install -d $(DESTDIR)$(PREFIX)/lib $(DESTDIR)$(PREFIX)/include
ifeq ($(findstring $(STATIC_LIB),$(TARGETS)),$(STATIC_LIB))
	install -m 644 $(STATIC_LIB) $(DESTDIR)$(PREFIX)/lib/
endif
ifeq ($(findstring $(SHARED_LIB),$(TARGETS)),$(SHARED_LIB))
	install -m 755 $(SHARED_LIB) $(DESTDIR)$(PREFIX)/lib/
endif
	cp -r $(INCLUDEDIR)/. $(DESTDIR)$(PREFIX)/include/

test: $(TESTEXECS)
	for test in $(TESTEXECS); do ./$$test; done

test/%.out: test/%.cpp $(STATIC_LIB)
	$(CXX) $(CXXFLAGS) -I$(INCLUDEDIR) $(EXTRA_INC) -o $@ $< $(STATIC_LIB) $(LDLIBS)

clean:
	rm -f $(OBJS) $(DEPS) $(STATIC_LIB) $(SHARED_LIB) $(TESTEXECS)

-include $(DEPS)