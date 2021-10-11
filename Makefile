SOURCES ?= src/*.c
INCLUDEDIR ?= include
DEST ?= cscroll

CFLAGS ?= -Wall -Wextra -pedantic
LIBS ?= -lncurses

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

