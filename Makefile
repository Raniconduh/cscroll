SOURCES ?= scroll.c
DEST ?= cscroll

CFLAGS = -Wall -Wextra
LIBS = -lncurses

all: cscroll

cscroll:
	cc -o $(DEST) $(SOURCES) $(CFLAGS) $(LIBS)

debug:
	cc -o $(DEST) $(SOURCES) $(CFLAGS) $(LIBS) -g

clean:
	$(RM) $(DEST)

