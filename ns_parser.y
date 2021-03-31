%{
#include <stdio.h>
#include <vector>
#include "command.h"

int yylex();
int yyerror(const char* s);

extern std::vector<Command> command_table;
%}

%union
{
	char* str;
};

%token <str> WORD
%token GT
%token LT

%%

cmd:
	/* empty */
	| WORD				{ 	Command new_command($1);
							command_table.push_back(new_command);
						}
	| cmd WORD			{	command_table[command_table.size()-1].args.push_back($2);
						}
	| cmd GT WORD		{	command_table[command_table.size()-1].output = $3;
						}
	| cmd LT WORD		{	command_table[command_table.size()-1].input = $3;
						}
	;


%%

int yyerror(const char *s) {
	fprintf(stderr, "Error: %s\n", s);
	return 0;
}

