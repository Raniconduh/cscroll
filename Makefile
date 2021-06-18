SOURCES ?= scroll.c
DEST ?= cscroll

CFLAGS = -Wall -Wextra
LIBS = -lncurses

all: cscroll

cscroll: $(SOURCES)
	cc -o $(DEST) $(SOURCES) $(CFLAGS) $(LIBS)

debug: $(SOURCES)
	cc -DDEBUG -o $(DEST) $(SOURCES) $(CFLAGS) $(LIBS) -g

clean:
	$(RM) $(DEST)

