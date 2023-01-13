.POSIX:

include config.mk

BIN = cscroll
SRC = src/commands.c src/dir.c src/hash.c src/io.c src/main.c \
      src/opts.c src/type.c src/var.c
OBJ = ${SRC:.c=.o}

CC ?= cc

all: ${BIN}

${BIN}: ${OBJ}
	${CC} ${CFLAGS} ${OBJ} -o $@ ${LDFLAGS}

%.o:
	${CC} -c ${CFLAGS} $<

clean:
	rm -f ${BIN} ${OBJ}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	install -Dm755 ${BIN} ${DESTDIR}${PREFIX}/bin/${BIN}

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/${BIN}

.PHONY: all clean install uninstall
