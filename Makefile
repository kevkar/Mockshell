all:
	flex json.l
	bison -d json.y
	g++ -o json main.cpp lex.yy.c json.tab.c

