PREFIX = /usr/local

ICONS = 1
CFLAGS += -Iinclude -Wall -Wextra -pedantic -DICONS=${ICONS}

LDFLAGS = `pkg-config --libs-only-l ncurses`
