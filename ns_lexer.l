%option noyywrap

%{
#include "ns_parser.tab.h"
#include <string>
#include "global.h"

int countForAlias = 0;

bool isAlias(std::string word)
{
	for (auto iter = aliasTable.begin(); iter != aliasTable.end(); iter++)
		if (iter->first == word)
			return true;
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

	std::cout << "env variable found!" << std::endl;

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

(?i:bye)						{ 	exit(0); }

"alias"							{	countForAlias++;
									return ALIAS;}

"unalias"						{	countForAlias++;
									return UNALIAS;}

"setenv"						{    countForAlias++;
									return SETENV; }

"unsetenv"						{ 	countForAlias++;
									return UNSETENV; }

"printenv"						{ return PRINTENV; }

"cd"							{ countForAlias++;
									return CD; }

">>"							{ 	return GTGT; }

">"								{	return GT; }

"<"								{	return LT; }		

"|"								{	return PIPE; }

"&"								{ 	return AMP; }

"2>&1"							{	return ERR_STDOUT; }

"2>"							{ 	return ERR_FILE; }


	/* Any character that is not a META character can be part of a word */
[\-\{\}\.\~\$\(\)\/a-zA-Z0-9]+	{ 
									if (isAlias(yytext) && countForAlias == 0)
									{
										char* yycopy = strdup(aliasTable[yytext].c_str());
										for ( int i = strlen(aliasTable[yytext].c_str()) - 1; i >= 0; --i )
                               				unput( yycopy[i] );
                          	 			free( yycopy );
										
									} else if (has_valid_env_variable(yytext))
									{	
										char* yycopy = strdup(transform_env_variable(yytext));
										for (int i = strlen(transform_env_variable(yytext)) - 1; i >= 0; --i )
                               				unput( yycopy[i] );
                          	 			free( yycopy );
																		
									}	else
									{
										yylval.str = strdup(yytext);
										return WORD;
									}							
								}


	/* Exit yyparse() upon reaching end of line */

\n								{ 	countForAlias = 0;
									return 0; }

