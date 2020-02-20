CC := gcc
CFLAGS := -g
targets := oss prime

all: $(targets)

oss: oss.c
	$(CC) $(CFLAGS) -o $@ $<
prime: prime.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm $(targets)
