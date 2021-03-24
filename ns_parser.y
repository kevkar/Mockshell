%{
#include <stdio.h>

int yylex();
int yyerror(const char* s);
%}

%union
{
	char *sval;
};

%token <sval> WORD
%token <sval> COMMAND
%token <sval> STRING
%token <sval> VARIABLE
%token <sval> META
%token EOL

%%

sentance:
	/* empty */
	| sentance WORD		{ printf("%s\n", $2); } ;
	| sentance COMMAND	{ printf("*%s*\n", $2); } ;
	| sentance STRING	{ printf("%s\n", $2); } ;
	| sentance VARIABLE	{ printf("__%s__\n", $2); } ;
	| sentance META		{ printf("** %s **\n", $2); } ;
	| sentance EOL
	;

%%

int yyerror(const char *s) {
	fprintf(stderr, "Error: %s\n", s);
	return 0;
}