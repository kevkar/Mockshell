%{
#include <stdio.h>
#include <vector>
#include "command.h"
#include "global.h"
#include <cstring>
#include <unistd.h>
#include "limits.h"
#include <map>
#include <algorithm>

int yylex();
int yyerror(const char* s);
void cd(std::string directory);
void changeDirectoryToHome();
std::string tilde_parse(std::string word);
std::string dot_dot_parse(std::string word);
std::string one_dot_parse(std::string word);
std::string do_parse(std::string word);

extern std::map<std::string,std::string> variableMap;
extern std::map<std::string,std::string> aliasTable;

void setEnvironmentVariable(std::string variable, std::string value);
void printEnvVariables();
void unsetEnvironmentVariable(std::string variable);

void printAliases();
void setAlias(std::string variable, std::string value);
char* transform_alias(char* name);
bool wouldMakeInfiniteLoop(char* name, int depth);
void unAlias(std::string key);


%}

%union
{
	char* str;
};

%token <str> WORD
%token GT GTGT LT PIPE AMP ERR_FILE ERR_STDOUT CD SETENV UNSETENV PRINTENV ALIAS UNALIAS

%%
goal:
	command_line;


command:
	ALIAS					{
								printAliases();
							}
	
	| ALIAS WORD WORD		{
								setAlias($2,$3);
							}

	| UNALIAS WORD			{
								unAlias($2);
							}
	
	| SETENV	WORD WORD	{
								setEnvironmentVariable($2,$3);
							}

	| UNSETENV	WORD		{
								unsetEnvironmentVariable($2);
							}

	| PRINTENV				{
								printEnvVariables();
							}

	| CD 					{
								changeDirectoryToHome();
							}

	| CD  WORD 				{
								std::string word = $2;
								std::string parsedString = do_parse(word);
								cd(parsedString);
							}

	| WORD					{ 	
								Command* new_cmd = new Command();
								char* word = $1;
								new_cmd->command_name = word;
								cmd_tbl->command[cmd_tbl->num_cmds] = new_cmd;
								cmd_tbl->num_cmds += 1;
							}
	| command WORD			{	
								Command* cmd = cmd_tbl->command[cmd_tbl->num_cmds - 1];
								char* word = $2;
								cmd->args[cmd->num_args] = word;
								//if (strcmp(cmd->command_name,"cd") == 0) cd(cmd->args[0]);
								cmd->num_args += 1;
							}
	;


command_list:
	command_list PIPE command
	| command
	;

io_redirect:
	GT WORD					{	cmd_tbl->output = $2;
							}	
	| GTGT WORD				{	cmd_tbl->output = $2;
								cmd_tbl->append_output = true;
							}
	| LT WORD				{	cmd_tbl->input = $2;
							}
	| ERR_FILE WORD			{	cmd_tbl->err_file = $2;
							}
	| ERR_STDOUT			{	cmd_tbl->err_stdout = true;
							}
	| AMP					{	cmd_tbl->wait_for_exec = false;
							}
	;

io_redirect_list:
	io_redirect_list io_redirect
	| /* empty */
	;

command_line:
	command_list io_redirect_list
	;

%%

int yyerror(const char *s) {
	fprintf(stderr, "Error: %s\n", s);
	return 0;
}

void printAliases()
{
	if (aliasTable.size() == 0)
	{
		std::cout << "No aliases to show!" << std::endl;
		return;
	}
		
	std::cout << "List of aliases" << std::endl;
	for (auto iter = aliasTable.begin(); iter != aliasTable.end(); iter++)
		std::cout  << iter->first << "=" << iter->second << std::endl;
}

void setAlias(std::string key, std::string value)
{
	aliasTable[key] = value;
	std::cout << "key is: " << key << std::endl;
	std::cout << "value is: " << value << std::endl;
	if (wouldMakeInfiniteLoop(strdup(value.c_str()),10))
	{
		std::cout << "Adding alias would make infinite loop!!!" << std::endl;
		aliasTable.erase(key);
		return;
	}

	std::string nested_command(transform_alias(strdup(value.c_str())));
		
}

char* transform_alias(char* name)
{
	std::string word(name);
	for (auto iter = aliasTable.begin(); iter != aliasTable.end(); iter++)
		if (iter->first == word)
			return transform_alias(strdup(iter->second.c_str()));
	
	return name;
}

bool wouldMakeInfiniteLoop(char* name, int depth)
{
	if (depth == 0)
		return true;

	std::string word(name);
	for (auto iter = aliasTable.begin(); iter != aliasTable.end(); iter++)
		if (iter->first == word)
			return wouldMakeInfiniteLoop(strdup(iter->second.c_str()),depth-1);

	return false;
	
}

void unAlias(std::string key)
{	
	bool found = false;
	for (auto iter = aliasTable.begin(); iter != aliasTable.end(); iter++)
		if (iter->first == key)
			found = true;
	
	if (found == false)
	{
		std::cout << "No alias with name " << key << " found!" << std::endl;
		return;
	}
		
	aliasTable.erase(key);
}

void setEnvironmentVariable(std::string variable, std::string value)
{
	variableMap[variable] = value;
	//setenv(variable,value,0);
}

void printEnvVariables()
{
	for (auto iter = variableMap.begin(); iter != variableMap.end(); iter++ )
		std::cout << iter->first << "=" << iter->second << std::endl;

	//for (char** envVar = environ; *envVar != 0; envVar++)
    //	printf("%s\n", *envVar);    
}

void unsetEnvironmentVariable(std::string variable)
{
	if (variable == "PATH" || variable == "HOME")
	{
		std::cout << "Cannot unset variable " << variable << "!" << std::endl;
		return;
	}

	bool found = false;
	for (auto iter = variableMap.begin(); iter != variableMap.end(); iter++)
		if (iter->first == variable)
			found = true;

	if (found == false)
	{
		std::cout << "No variable with name " << variable << " exists." << std::endl;
		return;
	}

	variableMap.erase(variable);
	//unsetenv(variable);
}

void cd(std::string directory)
{
	const char* dir = directory.c_str();
	if (directory[0] == '/')
	{
		int error = chdir(dir);
		if (error != 0)
			std::cout << "Error: directory " << directory << " not found." << std::endl;
		return;
	}

	char cwd[PATH_MAX];
	getcwd(cwd,sizeof(cwd));
	strcat(cwd,"/");
	const char* newDir = strcat(cwd,dir);
	
	int error = chdir(newDir);
	if (error != 0)
		std::cout << "Error: directory " << newDir << " not found." << std::endl;
}

void changeDirectoryToHome()
{
	char* home = strdup(variableMap["HOME"].c_str());
	chdir(home);
}

std::string tilde_parse(std::string word)
{
	std::string home = variableMap["HOME"];
	word = word.substr(1);
	word = word[0] == '/' ? home + word : home + '/' + word;
	
	return word;
}

std::string dot_dot_parse(std::string word)
{
	char currentDirectory[PATH_MAX];
	getcwd(currentDirectory,sizeof(currentDirectory));

	std::string cwd(currentDirectory);
	std::string s(word);

	std::string slash = "/";

	int index = cwd.find_last_of(slash);
	if (index == 0)
		return strdup(slash.c_str());

	cwd = cwd.substr(0, index);

	if (s.length() > 2)
		cwd += s.substr(2);
	
	return cwd;
}

std::string one_dot_parse(std::string word)
{
	char currentDirectory[PATH_MAX];
	getcwd(currentDirectory,sizeof(currentDirectory));

	std::string cwd(currentDirectory);
	
	if (word.length() == 0)
		word = cwd;
	else
	{
		word = word.substr(1);
		word = cwd + word;
	}
		
	return word;
}

std::string do_parse(std::string word)
{
	if (word[0] == '~')
		word = tilde_parse(word);
	else if (word.length() > 1 && word[0] == '.' && word[1] == '.' )
		word = dot_dot_parse(word);
	else if (word[0] == '.')
		word = one_dot_parse(word);

	return word;
}

