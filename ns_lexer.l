%option noyywrap

%{
#include "ns_parser.tab.h"
%}

%x capture_string

%%

	/* Ignore whitespace */

[{WHITESPACE}]+						{}

	/* Store all characters found between quotes */

["]								{	BEGIN(capture_string); }
<capture_string>[^"]*			{
									yylval.str = (char*) malloc(strlen(yytext));
									strncpy(yylval.str, yytext, strlen(yytext));
									return WORD;
								}
<capture_string>["]				{ 	BEGIN(INITIAL); }

(?i:bye)						{ 	exit(0); }

[>]								{	return GT; }

[<]								{	return LT; }		


	/* Any character that is not a META character can be part of a word */
[A-Za-z0-9\-\.]+					{ 
									yylval.str = (char*) malloc(strlen(yytext));
									strncpy(yylval.str, yytext, strlen(yytext));
									return WORD; 
								}


	/* Exit yyparse() upon reaching end of line */

\n								{ return 0; }

