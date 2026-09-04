CXX ?= c++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -pedantic

SRCDIR := src
PROGS := 01_basic 02_zombie 03_wait 04_sigchld 05_graceful

.PHONY: all clean

all: $(PROGS)

01_basic: $(SRCDIR)/01_basic.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

02_zombie: $(SRCDIR)/02_zombie.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

03_wait: $(SRCDIR)/03_wait.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

04_sigchld: $(SRCDIR)/04_sigchld.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

05_graceful: $(SRCDIR)/05_graceful.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(PROGS)
	rm -rf *.dSYM
