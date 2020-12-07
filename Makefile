CC = gcc
CFLAGS = -lreadline

all: cshell.a

cshell.a: cshell.o
	     $(CC) $(CFLAGS) cshell.c -o cshell

clean:
		 rm -rf *.o *.a