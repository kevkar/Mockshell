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
									yylval.str = strdup(yytext);
									return WORD;
								}
<capture_string>["]				{ 	BEGIN(INITIAL); }

(?i:bye)						{ 	exit(0); }


">>"							{ 	return GTGT; }

">"								{	return GT; }

"<"								{	return LT; }		

"|"								{	return PIPE; }

"&"								{ 	return AMP; }

"2>&1"							{	return ERR_STDOUT; }

"2>"							{ 	return ERR_FILE; }


	/* Any character that is not a META character can be part of a word */
[A-Za-z0-9\-\.\/]+					{ 
									yylval.str = strdup(yytext);
									return WORD;
								}


	/* Exit yyparse() upon reaching end of line */

\n								{ return 0; }

	/* Exit nutshell when EOF is reached */
<<EOF>>							{  exit(0); }
