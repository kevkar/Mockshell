# Simple Makefile
# Using flex 2.5.35
# Using bison 2.3

CC=/usr/bin/cc

all: flex-config bison-config nutshparser nutshscanner nutshell nutshell-out
	
flex-config:
	flex ns_lexer.l

bison-config:
	bison -d ns_parser.y

nutshparser:  ns_parser.tab.c 
	$(CC) -c ns_parser.tab.c -o ns_parser.y.o

nutshscanner:  lex.yy.c
	$(CC) -c lex.yy.c -o nutshscanner.lex.o

nutshell:  nutshell.c
	$(CC) -g -c nutshell.c -o nutshell.o

nutshell-out: 
	$(CC) -o nutshell nutshell.o nutshscanner.lex.o ns_parser.y.o -ll -lm -lfl

clean:
	rm nutshell *.o lex.yy.c ns_parser.tab.c ns_parser.tab.h