%{
#include "ns_parser.tab.h"
%}
%option noyywrap
%x capture_string
%x capture_variable


%%


	/* Ignore whitespace */

[ \t\r]+						{}


	/* Store all characters found between quotes */

["]								{ BEGIN(capture_string); }
<capture_string>[^"]*			{
									yylval.sval = (char*)malloc(strlen(yytext));
									strncpy(yylval.sval, yytext, strlen(yytext));
									return STRING; 
								}
<capture_string>["]				{ BEGIN(INITIAL); }


	/* Store variable name if found between ${...} */

$\{								{ BEGIN(capture_variable); }
<capture_variable>[A-Za-z]*		{
									yylval.sval = (char*)malloc(strlen(yytext));
									strncpy(yylval.sval, yytext, strlen(yytext));
									return VARIABLE; 
								}
<capture_variable>[}]			{ BEGIN(INITIAL); }


	/* Exit yyparse() upon reaching end of line */

\n								{ return 0; }


	/* Solitary metacharacters */
[\<\>\|\\\&]			{ yylval.sval = yytext; return META; }				



	/* Partial list of (case insensitive) built-in commands (just stored as strings for now)  */

(?i:alias)						{
									char text[] = "alias";
									yylval.sval = (char*)malloc(strlen(text));
									strncpy(yylval.sval, text, strlen(text));
									return COMMAND;
								}

(?i:unalias)					{
									char text[] = "unalias";
									yylval.sval = (char*)malloc(strlen(text));
									strncpy(yylval.sval, text, strlen(text));
									return COMMAND;
								}

(?i:setenv)						{
									char text[] = "setenv";
									yylval.sval = (char*)malloc(strlen(text));
									strncpy(yylval.sval, text, strlen(text));
									return COMMAND;
								}

(?i:unsetenv)					{
									char text[] = "unsetenv";
									yylval.sval = (char*)malloc(strlen(text));
									strncpy(yylval.sval, text, strlen(text));
									return COMMAND;
								}

(?i:printenv)					{
									char text[] = "printenv";
									yylval.sval = (char*)malloc(strlen(text));
									strncpy(yylval.sval, text, strlen(text));
									return COMMAND;
								}

(?i:cd)					{
									char text[] = "cd";
									yylval.sval = (char*)malloc(strlen(text));
									strncpy(yylval.sval, text, strlen(text));
									return COMMAND;
								}


	/* Exit nutshell */

(?i:bye)						{ exit(0); }


	/* Anything else that isn't a special command or character  */

[a-zA-Z0-9]+					{ 
									yylval.sval = (char*)malloc(strlen(yytext));
									strncpy(yylval.sval, yytext, strlen(yytext));
									return WORD; 
								}
