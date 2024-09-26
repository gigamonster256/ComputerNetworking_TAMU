CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -pedantic -O3

MODE ?= shared
# MODE = static

ifeq ($(MODE), static)
	LIBEXT = a
else ifeq ($(MODE), shared)
	LIBEXT = so
endif

# libTFTP
LIBTFTPDIR = src
LIBTFTPINCLUDE = -Iinclude
LIBTFTPSRCS = packets.cpp tftp.cpp error.cpp
LIBTFTPSRCS := $(addprefix $(LIBTFTPDIR)/, $(LIBTFTPSRCS))
LIBTFTPOBJS = $(LIBTFTPSRCS:.cpp=.o)
LIBTFTPBASE = libtftp
LIBTFTP = $(LIBTFTPBASE).$(LIBEXT)

.PHONY: all clean 
all: $(LIBTFTP)

clean:
	rm -f libtftp.a
	rm -f libtftp.so
	rm -f $(LIBTFTPDIR)/*.o

$(LIBTFTPBASE).a: $(LIBTFTPOBJS)
	ar rcs $@ $(LIBTFTPOBJS)

$(LIBTFTPBASE).so : $(LIBTFTPOBJS)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(LIBTFTPOBJS)

$(LIBTFTPDIR)/%.o: $(LIBTFTPDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(LIBTFTPINCLUDE) $(INCLUDES) -fPIC -c -o $@ $<




