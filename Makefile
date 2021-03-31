all:
	flex ns_lexer.l
	bison -d ns_parser.y
	g++ -c nutshell.cpp command.cpp
	g++ -o nutshell nutshell.o command.o nutshscanner.lex.o ns_parser.y.o

