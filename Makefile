DEBUG ?= 0

CC = gcc
CFLAGS = -g -Wall -Werror -O2 \
-DDEBUG=$(DEBUG) \
-I$(CURDIR)/core/cgb/include -I$(CURDIR)/frontend/sdl/include -I$(CURDIR)/utils/include \
-pthread

LD = gcc
LDFLAGS = -g -lSDL3 -pthread

OBJS = \
utils/timer.o \
main.o

HEADERS = \
utils/include/timer.h

.PHONY: all clean core/cgb/libcore.a frontend/sdl/libgraphics.a

all: wem

wem: $(OBJS) core/cgb/libcore.a frontend/sdl/libgraphics.a
	$(LD) $(LDFLAGS) $^ -o $@

clean:
	rm -f $(OBJS) wem
	$(MAKE) -C core/cgb clean
	$(MAKE) -C frontend/sdl clean

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

core/cgb/libcore.a:
	$(MAKE) -C core/cgb libcore.a CC=$(CC) CFLAGS="$(CFLAGS)"

frontend/sdl/libgraphics.a:
	$(MAKE) -C frontend/sdl libgraphics.a CC=$(CC) CFLAGS="$(CFLAGS)"