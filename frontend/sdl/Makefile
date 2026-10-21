GFX_OBJS = \
frontend.o

GFX_HEADERS = \
include/frontend.h

.PHONY: clean

libgraphics.a: $(GFX_OBJS)
	ar rcs $@ $^

clean:
	rm -f $(GFX_OBJS) libgraphics.a

%.o: %.c $(GFX_HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@