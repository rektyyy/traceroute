CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra

traceroute: main.o send.o recieve.o
	$(CC) $(CFLAGS) -o traceroute $^

clean:
	rm -rf *.o

distclean:
	rm -rf *.o traceroute