CFLAGS = -Wall -Wextra -Wshadow -Wconversion -g $(shell pkg-config --cflags sdl3)
CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
LDLIBS = $(shell pkg-config --libs sdl3) -lm
LDFLAGS = -fsanitize=address,undefined

ray: main.c
	$(CC) $(CFLAGS) $(LDFLAGS) main.c $(LDLIBS) -o $@

run: ray
		MallocNanoZone=0 ./ray

clean:
	rm -f ray

.PHONY: run clean
