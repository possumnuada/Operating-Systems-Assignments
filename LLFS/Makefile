VPATH = io disk apps
CC = gcc
CFLAGS = -Wall -Werror
DEPS = Disk_Library.h
OBJ = File.o Disk_Library.o

File: $(OBJ)
	gcc $(CFLAGS) -o $@ $^

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<
