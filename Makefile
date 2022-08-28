SOURCES ?= src/*.c
HEADERS ?= include/*.h
INCLUDEDIR ?= include
DEST ?= cscroll

ICONS ?= 1

CC ?= cc
CFLAGS += -DICONS=$(ICONS) -Wall -Wextra -pedantic $(shell pkg-config --cflags ncurses)

UNAME := $(shell uname)
$(shell pkg-config --exists ncursesw)
ifeq ($(.SHELLSTATUS),0)
        NCURSES := ncursesw
else
        NCURSES := ncurses
endif

LIBS += $(shell pkg-config --libs $(NCURSES)) -lm
ifneq ($(UNAME), Darwin)
	LIBS += -ltinfo
endif

PREFIX ?= /usr/local

all: cscroll

cscroll: $(SOURCES) $(HEADERS)
	$(CC) -I$(INCLUDEDIR) -o $(DEST) $(SOURCES) $(CFLAGS) $(LIBS)

debug: $(SOURCES) $(HEADERS)
	$(CC) -I$(INCLUDEDIR) -DDEBUG -o $(DEST) $(SOURCES) $(CFLAGS) $(LIBS) -g

install: all
	install -D $(DEST) $(DESTDIR)$(PREFIX)/bin/$(DEST)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(DEST)

clean:
	rm -f $(DEST)

