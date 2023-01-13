PREFIX = /usr/local

ICONS = 1
CFLAGS = -Iinclude -Wall -Wextra -pedantic -DICONS=${ICONS}

NCURSES = ncurses`pkg-config --exists ncursesw && echo w`
TINFO = `[ "$$(uname)" = Darwin ] && echo -ltinfo`
LDFLAGS = -l${NCURSES} ${TINFO}
