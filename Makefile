CC = gcc
CFLAGS = -lreadline

all: cshell.a

cshell.a: cshell.o
	     $(CC) cshell.c -o cshell $(CFLAGS)

clean:
		 rm -rf *.o *.a cshell