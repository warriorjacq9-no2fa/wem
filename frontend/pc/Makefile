GFX_OBJS = \
graphics.o

GFX_HEADERS = \
include/graphics.h

.PHONY: clean

libgraphics.a: $(GFX_OBJS)
	ar rcs $@ $^

clean:
	rm -f $(GFX_OBJS) libgraphics.a

%.o: %.c $(GFX_HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@