CC = gcc
CFLAGS = -g -Wall -Werror -O0

CORE_OBJS = \
cpu.o \
instructions.o \
memmap.o

CORE_HEADERS = \
include/core.h \
cpu.h \
instructions.h \
memmap.h

.PHONY: clean

libcore.a: $(CORE_OBJS)
	ar rcs $@ $^

clean:
	rm -f $(CORE_OBJS) libcore.a

%.o: %.c $(CORE_HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@