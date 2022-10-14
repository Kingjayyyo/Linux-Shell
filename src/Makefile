# provide a makefile with a target called d8sh that 
# creates the executable d8sh

CC = gcc
CFLAGS =  -ansi -Wall -g -O0 -Wwrite-strings -Wshadow -pedantic-errors \
		  -fstack-protector-all 

# Our programs (executables)
PROGS = d8sh

.PHONY: all clean

all: $(PROGS)

d8sh: lexer.o parser.tab.o executor.o d8sh.o command.h
	$(CC) -o d8sh d8sh.o lexer.o parser.tab.o executor.o -lreadline

d8sh.o: d8sh.c executor.o lexer.o parser.tab.o command.h
	$(CC) $(CFLAGS) -c d8sh.c -lreadline

lexer.o: lexer.c lexer.h
	$(CC) $(CFLAGS) -c lexer.c

executor.o: executor.c executor.h command.h
	$(CC) $(CFLAGS) -c executor.c

parser.tab.o: parser.tab.c parser.tab.h
	$(CC) $(CFLAGS) -c parser.tab.c

clean:
	rm	-f	*.o	$(PROGS)	a.out