# Simple Makefile
# Using flex 2.5.35
# Using bison 2.3

all: flex-config bison-config nutshparser nutshscanner nutshell nutshell-out
	
flex-config:
	flex ns_lexer.l

bison-config:
	bison -d ns_parser.y

nutshparser:  ns_parser.tab.c 
	g++ -std=c++11 -c ns_parser.tab.c -o ns_parser.y.o

nutshscanner:  lex.yy.c
	g++ -std=c++11 -c lex.yy.c -o nutshscanner.lex.o

nutshell:  nutshell.cpp
	g++ -std=c++11 -c nutshell.cpp command.cpp builtin.cpp

nutshell-out: 
	g++ -std=c++11 -o nutshell nutshell.o command.o builtin.o nutshscanner.lex.o ns_parser.y.o

clean:
	rm nutshell *.o lex.yy.c ns_parser.tab.c ns_parser.tab.h