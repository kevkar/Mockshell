# Simple Makefile
# Using flex 2.5.35
# Using bison 2.3

all: flex-config bison-config nutshparser nutshscanner nutshell nutshell-out
	
flex-config:
	flex ns_lexer.l

bison-config:
	bison -d ns_parser.y

nutshparser:  ns_parser.tab.c 
	g++ -c ns_parser.tab.c -o ns_parser.y.o

nutshscanner:  lex.yy.c
	g++ -c lex.yy.c -o nutshscanner.lex.o

nutshell:  nutshell.cpp
	g++ -c nutshell.cpp command.cpp

nutshell-out: 
	g++ -o nutshell nutshell.o command.o nutshscanner.lex.o ns_parser.y.o

clean:
	rm nutshell *.o lex.yy.c ns_parser.tab.c ns_parser.tab.h