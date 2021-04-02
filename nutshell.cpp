#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include "ns_parser.tab.h"
#include "command.h"
#include "tables.h"
#include <unordered_map>

using namespace std;

extern int yyparse();

char* PATH;
char* HOME;

std::vector<Command> command_table;
std::vector<std::string> built_in_cmds;
map<string,string> variableMap;
map<string,string> aliasTable;
vector<Alias> aliases;

void shell_init();

int main()
{
	shell_init();

	while(1) 
	{
		char *userName=getenv("USER");
		printf("%s>> ",userName);
		yyparse();

		// Print and clear contents of the command table
		for(int i = 0; i < command_table.size(); ++i) {
			//print_command(command_table[i]);
			execute_command(command_table[i]);
		}
		command_table.clear();

	}

	return 0;
}

void shell_init() 
{
	HOME = getenv("HOME");
	PATH = getenv("PATH");
	variableMap["HOME"] = HOME;
	variableMap["PATH"] = PATH;

	built_in_cmds.push_back("setenv");
	built_in_cmds.push_back("printenv");
	built_in_cmds.push_back("unsetenv");
	built_in_cmds.push_back("cd");
}