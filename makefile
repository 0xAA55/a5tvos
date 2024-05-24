
CROSS_COMPILE ?= arm-linux-

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)gcc
CFLAGS ?= -march=armv5t
CFLAGS += -flto -O3 -fPIC -static
CXXSTD ?= -std=c++2a
CXXFLAGS += $(CFLAGS) $(CXXSTD)
LDLIBS += -lstdc++ -lm
LDFLAGS += $(CFLAGS)

OBJS+=main.o
OBJS+=graphics.o
OBJS+=font.o
OBJS+=utf.o

all: tvos

tvos: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

clean:
	rm -f *.o tvos

.PHONY: all clean
