CC      ?= cc
PREFIX  ?= /usr/local
CFLAGS  ?= -O2 -Wall -Wextra -Wpedantic -std=c11
LDFLAGS ?=

GIT2_CFLAGS := $(shell pkg-config --cflags libgit2 2>/dev/null)
GIT2_LIBS   := $(shell pkg-config --libs libgit2 2>/dev/null || echo "-lgit2")

SRCS = hs.c prompt.c git.c
OBJS = $(SRCS:.c=.o)

hs: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(GIT2_LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(GIT2_CFLAGS) -c $< -o $@

install: hs
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 hs $(DESTDIR)$(PREFIX)/bin/hs

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/hs

clean:
	rm -f hs $(OBJS)

# Header dependencies
hs.o: hs.c config.h prompt.h git.h
prompt.o: prompt.c prompt.h config.h
git.o: git.c git.h config.h

.PHONY: install uninstall clean
