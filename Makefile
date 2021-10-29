SOURCES ?= src/*.c
INCLUDEDIR ?= include
DEST ?= cscroll

CFLAGS ?= -Wall -Wextra -pedantic $(shell pkg-config --cflags ncurses)
LIBS ?= $(shell pkg-config --libs ncurses) -ltinfo

RM ?= rm -f

all: cscroll

cscroll: $(SOURCES)
	cc -I$(INCLUDEDIR) -o $(DEST) $(SOURCES) $(CFLAGS) $(LIBS)

debug: $(SOURCES)
	cc -I$(INCLUDEDIR) -DDEBUG -o $(DEST) $(SOURCES) $(CFLAGS) $(LIBS) -g

install: all
	install $(DEST) $(DESTDIR)/$(PREFIX)/bin

uninstall:
	rm -f $(DESTDIR)/$(PREFIX)/bin/$(DEST)

clean:
	$(RM) $(DEST)

