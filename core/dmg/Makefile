CORE_OBJS = \
cpu.o \
instructions.o \
memmap.o \
ppu.o \
apu.o \
tmr.o \
core.o

CORE_HEADERS = \
include/core.h \
cpu.h \
instructions.h \
memmap.h \
ppu.h \
apu.h \
tmr.h

.PHONY: clean

libcore.a: $(CORE_OBJS)
	ar rcs $@ $^

clean:
	rm -f $(CORE_OBJS) libcore.a

%.o: %.c $(CORE_HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@