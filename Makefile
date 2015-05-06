
# Fallback to gcc when $CC is not in $PATH.
CC:=$(shell sh -c 'type $(CC) >/dev/null 2>/dev/null && echo $(CC) || echo gcc')
OPTIMIZATION?=-O3
WARNINGS=-Wall -W -Wstrict-prototypes -Wwrite-strings
DEBUG?= -g -ggdb
#
REAL_CFLAGS=$(OPTIMIZATION) -fPIC -std=c99 $(CFLAGS) $(DEBUG) 
#REAL_CFLAGS=$(OPTIMIZATION) -fPIC -std=c99 -pedantic $(CFLAGS) $(WARNINGS) $(DEBUG) $(ARCH)
REAL_LDFLAGS=$(LDFLAGS) $(ARCH)

all:	test-libev test2-libev example-libev

test-libev: test-libev.c 
	$(CC) -o $@ $(REAL_CFLAGS) $(REAL_LDFLAGS) -I. $< -lev 

test2-libev: test2-libev.c
	$(CC) -o $@ $(REAL_CFLAGS) $(REAL_LDFLAGS) -I. $< -lev

example-libev: example-libev.c
example-libev: example-libev.c  
	$(CC) -o $@ $(REAL_CFLAGS) $(REAL_LDFLAGS) -I /usr/local -I. $< -lev -lhiredis

clean:
	rm test-libev
	rm test2-libev
	rm example-libev

