PREFIX = /usr/local

ICONS = 1
CFLAGS += -Iinclude -Wall -Wextra -pedantic -DICONS=${ICONS}

LDFLAGS = `pkg-config --exists ncursesw && pkg-config --libs ncursesw || pkg-config --libs ncurses`
