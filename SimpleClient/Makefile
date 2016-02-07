CC=gcc
LD=gcc
CFLAGS=-c -Wall -Werror -O2
LDFLAGS=-lrt

CLIENT=client

all: $(CLIENT)

$(CLIENT): $(CLIENT).o
	$(CC) $(LDFLAGS) $^  -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(CLIENT).o