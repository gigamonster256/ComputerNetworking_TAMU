LIBNAME := http
SRCS := client.cpp date.cpp error.cpp header.cpp message.cpp request-line.cpp status-line.cpp uri.cpp

# libtcp dependency (overridable, e.g. from nix)
LIBTCP_ROOT ?= ../libtcp
LIBTCP_LIBDIR ?= $(LIBTCP_ROOT)
LIBTCP_INC ?= -I$(LIBTCP_ROOT)/include
EXTRA_INC := $(LIBTCP_INC)
SHARED_DEPS := $(LIBTCP_LIBDIR)/libtcp.so
SHARED_LDLIBS := -L$(LIBTCP_LIBDIR) -ltcp

include ../lib.mk

$(LIBTCP_LIBDIR)/libtcp.so:
	$(MAKE) -C $(LIBTCP_ROOT) MODE=$(MODE)