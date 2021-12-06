SOURCES ?= src/*.c
INCLUDEDIR ?= include
DEST ?= cscroll

CC ?= cc
CFLAGS ?= -Wall -Wextra -pedantic $(shell pkg-config --cflags ncurses)
LIBS ?= $(shell pkg-config --libs ncurses) -ltinfo

PREFIX ?= /usr/local

all: cscroll

cscroll: $(SOURCES)
	$(CC) -I$(INCLUDEDIR) -o $(DEST) $(SOURCES) $(CFLAGS) $(LIBS)

debug: $(SOURCES)
	$(CC) -I$(INCLUDEDIR) -DDEBUG -o $(DEST) $(SOURCES) $(CFLAGS) $(LIBS) -g

install: all
	install -D $(DEST) $(DESTDIR)$(PREFIX)/bin/$(DEST)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(DEST)

clean:
	rm -f $(DEST)

