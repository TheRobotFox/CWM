##
# CWM
#
# @file
# @version 0.1
#
CC=gcc
CFLAGS=-lm -fsanitize=address -ggdb

SOURCE:= $(wildcard *.c) \
		 $(wildcard Conscreen/Conscreen*.c) \
		 Conscreen/List/List.c \
		 Conscreen/List/Heap.c

HEADERS:= $(wildcard *.h)\
		  $(wildcard Conscreen/Conscreen_*.h)\
		  $(wildcard Conscreen/List/*.h)

OBJECTS:= $(SOURCE:.c=.o)


app: $(OBJECTS) $(HEADERS)
	$(CC) -o $@ $(OBJECTS) $(CFLAGS)

clean:
	rm -rf $(OBJECTS) app
# end
