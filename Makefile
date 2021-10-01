SOURCES ?= src/*.c
INCLUDEDIR ?= include
DEST ?= cscroll

CFLAGS = -Wall -Wextra
LIBS = -lncurses

all: cscroll

cscroll: $(SOURCES)
	cc -I$(INCLUDEDIR) -o $(DEST) $(SOURCES) $(CFLAGS) $(LIBS)

debug: $(SOURCES)
	cc -I$(INCLUDEDIR) -DDEBUG -o $(DEST) $(SOURCES) $(CFLAGS) $(LIBS) -g

clean:
	$(RM) $(DEST)

