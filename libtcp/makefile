LIBNAME := tcp
SRCS := client.cpp server.cpp
TESTS := server.cpp

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
LDLIBS := -lpthread
endif

include ../lib.mk