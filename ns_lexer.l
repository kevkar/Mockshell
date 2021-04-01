%option noyywrap
%array
%{
#include "ns_parser.tab.h"
#include "command.h"
%}

%x capture_string

%%

	/* Ignore whitespace */

[ \t\r]+						{}

	/* Store all characters found between quotes */

["]								{	BEGIN(capture_string); }
<capture_string>[^"]*			{
									yylval.str = (char*) malloc(strlen(yytext));
									strncpy(yylval.str, yytext, strlen(yytext));
									yylval.str = transform_env_variable(yylval.str);
									printf("string here\n");
									return WORD;
								}
<capture_string>["]				{ 	BEGIN(INITIAL); }



(?i:bye)						{ 	exit(0); }



[\-\{\}\.\~\$\(\)\/a-zA-Z0-9]+	{	
									yylval.str = (char*) malloc(strlen(yytext));
									strncpy(yylval.str, yytext, strlen(yytext));
									yylval.str = transform_env_variable(yylval.str);
									printf("word here\n");
									return WORD; 
								}


	/* Exit yyparse() upon reaching end of line */

\n								{ return 0; }

