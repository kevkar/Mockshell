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
%token EOL

%%

cmd:
	/* empty */
	| WORD				{   
							printf("WORD UPPER IS: %d\n",WORD);
							Command new_command($1);
							command_table.push_back(new_command);
						}
	| cmd WORD			{	
							printf("WORD LOWER IS: %d\n",WORD);
							command_table[command_table.size()-1].args.push_back($2);
						}
	;


%%

int yyerror(const char *s) {
	fprintf(stderr, "Error: %s\n", s);
	return 0;
}

