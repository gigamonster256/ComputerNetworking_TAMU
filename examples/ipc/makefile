CXX ?= c++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -pedantic

SRCDIR := src
PROGS := 01_pipe 02_fifo 03_mq 04_shm 05_sem 06_socketpair

.PHONY: all clean

all: $(PROGS)

01_pipe: $(SRCDIR)/01_pipe.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

02_fifo: $(SRCDIR)/02_fifo.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

03_mq: $(SRCDIR)/03_mq.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< -lrt

04_shm: $(SRCDIR)/04_shm.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< -lrt

05_sem: $(SRCDIR)/05_sem.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< -lrt -pthread

06_socketpair: $(SRCDIR)/06_socketpair.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(PROGS)
	rm -rf *.dSYM
