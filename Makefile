PREFIX?=/usr/X11R6
CFLAGS?=-O2 -pedantic -Wall

all:
	$(CC) $(CFLAGS) -I$(PREFIX)/include tinywm.c -L$(PREFIX)/lib -lX11 -o tokai

clean:
	rm -f tokai
