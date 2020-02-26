CC := gcc
CFLAGS := -g -Wall
targets := oss prime

all: $(targets)

oss: oss.c
	$(CC) $(CFLAGS) -o $@ $<
prime: prime.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm $(targets)
