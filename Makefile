-include ../common_abgabe.mk
CFLAGS  = -std=c17 -pedantic -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700 -Wall -Werror -g
LDFLAGS =
CC		= gcc
.PHONY: all doc clean

all: crawl

doc:
	$(RM) -r html
	doxygen

crawl: crawl.o argumentParser.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $^

clean:
	$(RM) crawl crawl.o
