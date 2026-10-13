CC = gcc
CFLAGS = -g -Wall -Werror -O0

LD = gcc
LDFLAGS = -g

OBJS = \
core/cgb/cpu.o \
core/cgb/instructions.o \
main.o

HEADERS = \
core/cgb/cpu.h \
core/cgb/instructions.h

.PHONY: all clean

all: wem

wem: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

clean:
	rm $(OBJS) wem

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@