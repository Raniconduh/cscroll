PREFIX = /usr/local

ICONS ?= 1
CFLAGS += -Iinclude -Wall -Wextra -pedantic -D_XOPEN_SOURCE=700 \
		  -DICONS=${ICONS}

LDFLAGS = `pkg-config --libs ncursesw || pkg-config --libs ncurses`
