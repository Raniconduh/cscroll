PREFIX = /usr/local

ICONS ?= 1
CFLAGS += -Iinclude -Wall -Wextra -pedantic -D_POSIX_C_SOURCE=200112L \
		  -DICONS=${ICONS}

LDFLAGS = `pkg-config --libs ncursesw || pkg-config --libs ncurses`
