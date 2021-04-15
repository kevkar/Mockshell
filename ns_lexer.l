%option noyywrap
%{
#include <string>
#include "ns_parser.tab.h"
#include "global.h"
#include <dirent.h>
#include "unistd.h"
#include <algorithm>

int countForAlias = 0;
bool tryingToUnset = false;

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

bool hasQuestionMark(char* word)
{
	std::string s(word);
	for (int i = 0; i < s.length(); i++)
		if (s[i] == '?')
			return true;	
	return false;
}

bool hasAsterisk(char* word)
{
	std::string s(word);
	for (int i = 0; i < s.length(); i++)
		if (s[i] == '*')
			return true;	
	return false;
}

//gets files from cwd only
std::vector<std::string> getFiles()
{
	std::vector<std::string> res;

	char currentDirectory[PATH_MAX];
	getcwd(currentDirectory,sizeof(currentDirectory));

	if (auto dir = opendir(currentDirectory)) {
    	while (auto f = readdir(dir)) {
			if (f->d_name[0] == '.') continue;
			res.push_back(f->d_name);
   		 }
   		 closedir(dir);
	}

	return res;
}

// matches if ? is in place of character
bool isMatchQuestion(std::string word, std::string s)
{
	if (word.length() != s.length()) return false;
	int index;
	for (int i = 0; i < s.size(); i++)
		if (s[i] == '?')
		{
			index = i;
			break;
		}
			
	int before = index - 1;
	int after = index + 1;

	for (int i = 0; i < before; i++)
	{
		if (word[i] != s[i])
			return false;
	}

	int length = std::min(s.length(),word.length());
	for (int i = after; i < length; i++)
	{
		if (word[i] != s[i])
			return false;
	}
	return true;
}

// removes wildcard character if there are no matches
char* removeWildcardChar(char* word)
{
	int index = 0;
	std::string res = "";
	std::string s(word);
	for (int i = 0; i < s.length();i++)
		if (s[i] == '?' || s[i] == '*')
		{
			index = i;
			break;
		}

	for (int i = 0; i < index; i++)
		res += s[i];
	
	for (int i = index + 1; i < s.length(); i++)
		res += s[i];
	
	return strdup(res.c_str());
}


char* transformQuestionWildcard(char* word)
{
	std::string res = "";
	std::string s(word);
	
	std::vector<std::string> fileNames = getFiles();

	for (int i = 0; i < fileNames.size(); i++)
	{
		if (isMatchQuestion(fileNames[i],s))
		{
			res += fileNames[i];
			res += " ";
		}

	}

	if (res == "")
		return removeWildcardChar(word);
	
	return strdup(res.c_str());
}

// matches if everything before or after * matches
bool matchAsterisk(std::string word, std::string s)
{
	if (s[0] == '*' && s[s.length() -1] == '*')
		return true;
	if (s[0] == '*')
	{
		int lastIndex = word.length() - 1;
		for (int i = s.length() -1; i > 0; i--)
		{
			if (s[i] != word[lastIndex])
				return false;
			lastIndex--;
		}
	}

	if (s[s.length() -1] == '*')
	{
		for (int i = 0; i < s.length() -1; i++)
		{
			if (s[i] != word[i])
				return false;
		}
	}
	return true;
}

char* transformAsteriskWildcard(char* word)
{
	std::string res = "";
	std::string s(word);

	std::vector<std::string> fileNames = getFiles();

	for (int i = 0; i < fileNames.size(); i++)
	{
		if (matchAsterisk(fileNames[i],s))
		{
			res += fileNames[i];
			res += " ";
		}
	}

	if (res == "")
		return removeWildcardChar(word);

	return strdup(res.c_str());
}


%}

%x capture_string

%%

	/* Ignore whitespace */

[ ]+						{}

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
[\\?:\*\-\{\}\.\~\$\(\)\/a-zA-Z0-9\_]+	{ 
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
									else if (hasQuestionMark(yytext))
									{
										yytext = transformQuestionWildcard(yytext);
										std::cout << "YYTEXT IS: " << yytext << std::endl;
										char* yycopy = yytext;																			
										for (int i = strlen(yytext) - 1; i >= 0; --i )
												unput( yycopy[i] );										                              	
                          	 			free( yycopy );
									} else	if (hasAsterisk(yytext))
									{
										yytext = transformAsteriskWildcard(yytext);
										char* yycopy = yytext;																			
										for (int i = strlen(yytext) - 1; i >= 0; --i )
												unput( yycopy[i] );										                              	
                          	 			free( yycopy );
										
									}	else										
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

