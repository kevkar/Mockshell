%option noyywrap
%{
#include <string>
#include "ns_parser.tab.h"
#include "global.h"

int countForAlias = 0;

bool is_alias(std::string word)
{
	for (auto iter = aliasTable.begin(); iter != aliasTable.end(); iter++)
		if (iter->first == word)
			return true;
	return false;
}

bool escape_key_pressed(std::string input)
{
	for (auto &ch : input)
	{	
		if((int)ch == 27)
			return true;
	}

	return false;
}

bool has_valid_env_variable(std::string word)
{
	std::string s(word);
	if (s.find("$") != std::string::npos &&
		s.find("{") != std::string::npos &&
		s.find("}") != std::string::npos)
		return true;

	return false;
}

bool envVariableExists(std::string word)
{
	for (auto iter = variableMap.begin(); iter != variableMap.end(); iter++)
		if (iter->first == word)
			return true;
	return false;
}

char* transform_env_variable(char* word)
{
	if (!has_valid_env_variable(word)) return word;

	std::string s(word);

	int n = s.length();
	std::string res = "";
	std::string dollarBracket("${");
	std::string closeBracket("}");

	int startIndex = s.find(dollarBracket);
	int endIndex = s.find(closeBracket);

	std::string begin = s.substr(0,startIndex);
	std::string middle = s.substr(startIndex+2,endIndex - startIndex - 2);
	const char* var = strdup(middle.c_str());

	if (!envVariableExists(var))
	{
		std::cout << "Missing environment variable" << std::endl;
		return word;
	}

	std::string value = variableMap[var];

	std::string end = s.substr(endIndex + 1,n - endIndex);
	
	res += begin + value + end;

	return strdup(res.c_str());
}


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

(?i:bye)						{ 	printf("Goodbye!\n");
									exit(0); }

"alias"							{	yylval.str = strdup(yytext);
									countForAlias++;
									return ALIAS;
								}

"unalias"						{	yylval.str = strdup(yytext);
									countForAlias++;
									return UNALIAS;
								}

"setenv"						{   yylval.str = strdup(yytext); 
									countForAlias++;
									return SETENV;
								}

"unsetenv"						{ 	yylval.str = strdup(yytext);
									countForAlias++;
									return UNSETENV;
								}

"printenv"						{ 	yylval.str = strdup(yytext);
									return PRINTENV; }

"cd"							{ 	yylval.str = strdup(yytext);
									countForAlias++;
									return CD;
								}

">>"							{ 	return GTGT; }

">"								{	return GT; }

"<"								{	return LT; }		

"|"								{	return PIPE; }

"&"								{ 	return AMP; }

"2>&1"							{	return ERR_STDOUT; }

"2>"							{ 	return ERR_FILE; }


	/* Any character that is not a META character can be part of a word */
[\-\{\}\.\~\$\(\)\/a-zA-Z0-9]+	{ 
									if (is_alias(yytext) && countForAlias == 0)
									{
										char* yycopy = strdup(aliasTable[yytext].c_str());
										for ( int i = strlen(aliasTable[yytext].c_str()) - 1; i >= 0; --i )
                               				unput( yycopy[i] );
                          	 			free( yycopy );
										
									}
									else if (has_valid_env_variable(yytext))
									{	
										char* yycopy = strdup(transform_env_variable(yytext));
										for (int i = strlen(transform_env_variable(yytext)) - 1; i >= 0; --i )
                               				unput( yycopy[i] );
                          	 			free( yycopy );
																		
									}
									else
									{
										yylval.str = strdup(yytext);
										return WORD;
									}							
								}

.								{	if (escape_key_pressed(yytext))
									{
										return ESCAPE;
									}
								}

	/* Exit yyparse() upon reaching end of line */

\n								{ 	countForAlias = 0;
									return 0; }

	/* Exit nutshell when EOF is reached */
<<EOF>>							{  exit(0); }

