BIN = cscroll
SRC = src/hashmap.c src/config.c src/dir.c src/ui.c src/main.c
OBJ = ${SRC:.c=.o}

CC ?= cc

CFLAGS += -Iinclude -Wall -Wextra -O2
LDFLAGS += -lncurses

all: ${BIN}

${BIN}: ${OBJ}
	${CC} ${CFLAGS} ${OBJ} -o $@ ${LDFLAGS}

src/.c.o:
	${CC} -c ${CFLAGS} $<

clean:
	rm -f ${BIN} ${OBJ}

.PHONY: all clean
